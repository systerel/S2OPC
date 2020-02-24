/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>

#include "sopc_atomic.h"
#include "sopc_dataset_layer.h"
#include "sopc_event_handler.h"
#include "sopc_event_timer_manager.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_mem_alloc.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_constants.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_pubsub_protocol.h"
#include "sopc_raw_sockets.h"
#include "sopc_rt_publisher.h"
#include "sopc_threads.h"
#include "sopc_udp_sockets.h"

/* Transport context. One per connection */
typedef struct SOPC_PubScheduler_TransportCtx SOPC_PubScheduler_TransportCtx;

// Function to clear a transport context. To be implemented for each protocol
typedef void (*SOPC_PubScheduler_TransportCtx_Clear)(SOPC_PubScheduler_TransportCtx*);

// Function to send a message. To be implemented for each protocol
typedef void (*SOPC_PubScheduler_TransportCtx_Send)(SOPC_PubScheduler_TransportCtx*, SOPC_Buffer*);

// Clear a Transport UDP context. Implements SOPC_PubScheduler_TransportCtx_Clear
static void SOPC_PubScheduler_CtxUdp_Clear(SOPC_PubScheduler_TransportCtx* ctx);

// Send an UDP message. Implements SOPC_PubScheduler_TransportCtx_Send
static void SOPC_PubScheduler_CtxUdp_Send(SOPC_PubScheduler_TransportCtx* ctx, SOPC_Buffer* buffer);

/* Context of a UDP Multicast Message */
struct SOPC_PubScheduler_TransportCtx
{
    SOPC_Socket_AddressInfo* multicastAddr;
    Socket sock;

    SOPC_PubScheduler_TransportCtx_Clear fctClear;
    SOPC_PubScheduler_TransportCtx_Send fctSend;
};

typedef struct SOPC_PubScheduler_MessageCtx
{
    // do not delete
    SOPC_WriterGroup* group;
    SOPC_Dataset_NetworkMessage* message;

    SOPC_PubScheduler_TransportCtx* transport;

    SOPC_Buffer* postEncodeBuffer[2];
    uint32_t postEncodeBufferCurrent;

    uint32_t rt_publisher_msg_id;
} SOPC_PubScheduler_MessageCtx;

typedef struct SOPC_PubScheduler_MessageCtx_Array
{
    uint64_t length;
    uint64_t current;
    SOPC_PubScheduler_MessageCtx* array;
} SOPC_PubScheduler_MessageCtx_Array;

// Total of message
static uint64_t SOPC_PubScheduler_Nb_Message(SOPC_PubSubConfiguration* config);

// Allocation of message context array (size used returned by _Nb_Message)
static bool SOPC_PubScheduler_MessageCtx_Array_Initialize(SOPC_PubSubConfiguration* config);

// Deallocation of message context array
static void SOPC_PubScheduler_MessageCtx_Array_Clear(void);

// Traverse allocated message array to initialize transport context associated to message. Increment "current" field.
static bool SOPC_PubScheduler_MessageCtx_Array_Init_Next(SOPC_PubScheduler_TransportCtx* ctx, SOPC_WriterGroup* group);

// Get last well initialized message context.
static SOPC_PubScheduler_MessageCtx* SOPC_PubScheduler_MessageCtx_Get_Last(void);

// Clear pub scheduler context
static void SOPC_PubScheduler_Context_Clear(void);

// Return data set writer use to build opcua frame to send from a writer group of a message context.
static const SOPC_DataSetWriter* SOPC_PubScheduler_Group_Get_Unique_Writer(const SOPC_WriterGroup* group);

// Allocation and initialization of a transport context. ctx is the field of message context.
static bool SOPC_PubScheduler_Connection_Get_Transport(uint32_t index,
                                                       SOPC_PubSubConnection* connection,
                                                       SOPC_PubScheduler_TransportCtx** ctx);
static struct
{
    SOPC_Looper* looper;
    SOPC_EventHandler* handler;
    /* Managing start / stop phase */
    int32_t isStarted;
    int32_t processingStartStop;

    /* Input parameters */
    SOPC_PubSubConfiguration* config;
    SOPC_PubSourceVariableConfig* sourceConfig;

    /* Internal context */
    // Size of transport are number of connection
    uint32_t nbConnection;
    SOPC_PubScheduler_MessageCtx_Array messages;

    // Global Transport context

    // keep transport context to clear them when stop.
    // One per connection
    SOPC_PubScheduler_TransportCtx* transport;

    // A strictly monotonically increasing sequence number
    uint32_t sequenceNumber;

    // Thread variable monitoring

    Thread handleThreadVarMonitoring;
    volatile bool bQuitVarMonitoring;

    // Thread which call beat heart function of RT Publisher

    SOPC_RT_Publisher* pRTPublisher; // RT Publisher

#if SOPC_PUBSCHEDULER_BEATHEART_FROM_IRQ == 0
    Thread handleThreadBeatHeart; // Handle thread simulates IRQ. This thread call HeartBeat publisher function.
    volatile bool bQuitBeatHeart; // Quit control of thread which simulates IRQ
#endif
} pubSchedulerCtx = {.isStarted = false,
                     .processingStartStop = false,
                     .config = NULL,
                     .sourceConfig = NULL,
                     .nbConnection = 0,
                     .transport = NULL,
                     .messages.length = 0,
                     .messages.current = 0,
                     .messages.array = NULL,
                     .pRTPublisher = NULL,
#if SOPC_PUBSCHEDULER_BEATHEART_FROM_IRQ == 0
                     .bQuitBeatHeart = true,
                     .handleThreadBeatHeart = (Thread) NULL,
#endif
                     .bQuitVarMonitoring = true,
                     .handleThreadVarMonitoring = (Thread) NULL};
// This callback of thread var monitoring is porting from code of old event_timer callback
static void* SOPC_RT_Publisher_VarMonitoringCallback(void* arg);

// This callback increment call RT publisher beat heart with a monotonic counter post incremented
// then sleep during SOPC_TIMER_RESOLUTION_MS ms.
#if SOPC_PUBSCHEDULER_BEATHEART_FROM_IRQ == 0
static void* SOPC_RT_Publisher_ThreadBeatHeartCallback(void* arg);
#endif

// RT Publisher callback functions (start and stop used only for debug, send function is really useful)

// Start callback, called when timer switch from DISABLED to ENABLED
static void SOPC_RT_Publisher_StartPubMsgCallback(uint32_t msgId,      // Message instance identifier
                                                  void* pUserContext); // User context

// Stop callback, called when timer switch from ENABLED to DISABLED
static void SOPC_RT_Publisher_StopPubMsgCallback(uint32_t msgId,      // Message instance identifier
                                                 void* pUserContext); // User context

// Elapsed callback, called when timer reach its configured period
static void SOPC_RT_Publisher_SendPubMsgCallback(uint32_t msgId,     // Message instance identifier
                                                 void* pUserContext, // User context
                                                 void* pData,        // Data published by set data API
                                                 uint32_t size);     // Data size in bytes

// Clear pub scheduler context
// 1) Stop beat heart thread
// 2) Stop variable monitoring thread
// 3) Clear (De initialize) RT Publisher
// 4) Destroy RT Publisher
// 5) Destory all message context

static void SOPC_PubScheduler_Context_Clear(void)
{
    /* Stop beat heart and variable monitoring threads */
#if SOPC_PUBSCHEDULER_BEATHEART_FROM_IRQ == 0
    printf("# Info: Stop beat heart thread\r\n");
    pubSchedulerCtx.bQuitBeatHeart = true;

    if (pubSchedulerCtx.handleThreadBeatHeart != (Thread) NULL)
    {
        SOPC_Thread_Join(pubSchedulerCtx.handleThreadBeatHeart);
        pubSchedulerCtx.handleThreadBeatHeart = (Thread) NULL;
    }
#endif

    printf("# Info: Stop var monitoring thread\r\n");
    pubSchedulerCtx.bQuitVarMonitoring = true;

    if (pubSchedulerCtx.handleThreadVarMonitoring != (Thread) NULL)
    {
        SOPC_Thread_Join(pubSchedulerCtx.handleThreadVarMonitoring);
        pubSchedulerCtx.handleThreadVarMonitoring = (Thread) NULL;
    }

    /* Clear RT publisher */
    printf("# Info: Deinit and destroy rt publisher \r\n");
    SOPC_ReturnStatus result = SOPC_RT_Publisher_DeInitialize(pubSchedulerCtx.pRTPublisher);

    /* This loop is useless in this case, but can be used if beat heart is called from IRQ*/
    while (SOPC_STATUS_INVALID_STATE == result)
    {
        printf("# Info: Error... \r\n");
        result = SOPC_RT_Publisher_DeInitialize(pubSchedulerCtx.pRTPublisher);
        SOPC_Sleep(100);
    }

    /* Destroy RT Publisher.*/

    SOPC_RT_Publisher_Destroy(&pubSchedulerCtx.pRTPublisher);

    /* Destroy messages and messages array */
    SOPC_PubScheduler_MessageCtx_Array_Clear();

    /* Destroy transport context */
    for (uint32_t i = 0; i < pubSchedulerCtx.nbConnection; i++)
    {
        if (pubSchedulerCtx.transport[i].fctClear != NULL)
        {
            pubSchedulerCtx.transport[i].fctClear(&pubSchedulerCtx.transport[i]);
            printf("# Info: transport context destroyed for connection #%d (publisher). \n",
                   pubSchedulerCtx.nbConnection);
        }
    }

    /* Destroy transport contexts array */
    SOPC_Free(pubSchedulerCtx.transport);
    pubSchedulerCtx.nbConnection = 0;
    pubSchedulerCtx.config = NULL;
    pubSchedulerCtx.sourceConfig = NULL;
    pubSchedulerCtx.sequenceNumber = 1;
}

static bool SOPC_PubScheduler_MessageCtx_Array_Initialize(SOPC_PubSubConfiguration* config)
{
    const uint64_t length = SOPC_PubScheduler_Nb_Message(config);
    pubSchedulerCtx.messages.current = 0;
    pubSchedulerCtx.messages.array = SOPC_Calloc((size_t) length, sizeof(SOPC_PubScheduler_MessageCtx));
    if (NULL == pubSchedulerCtx.messages.array)
    {
        return false;
    }
    pubSchedulerCtx.messages.length = length;
    return true;
}

static void SOPC_PubScheduler_MessageCtx_Array_Clear(void)
{
    if (pubSchedulerCtx.messages.array != NULL)
    {
        /* Destroy message */
        for (uint32_t i = 0; i < pubSchedulerCtx.messages.current; i++)
        {
            printf("# Info: Network message #%d. destroyed \n", i);
            SOPC_Dataset_LL_NetworkMessage_Delete(pubSchedulerCtx.messages.array[i].message);
            pubSchedulerCtx.messages.array[i].message = NULL;

            SOPC_Buffer_Delete(pubSchedulerCtx.messages.array[i].postEncodeBuffer[0]);
            SOPC_Buffer_Delete(pubSchedulerCtx.messages.array[i].postEncodeBuffer[1]);
        }

        /* Destroy messages array */
        SOPC_Free(pubSchedulerCtx.messages.array);
    }
    pubSchedulerCtx.messages.array = NULL;
    pubSchedulerCtx.messages.current = 0;
    pubSchedulerCtx.messages.length = 0;
}

static bool SOPC_PubScheduler_MessageCtx_Array_Init_Next(SOPC_PubScheduler_TransportCtx* ctx, SOPC_WriterGroup* group)
{
    assert(ctx != NULL);
    assert(pubSchedulerCtx.messages.current < pubSchedulerCtx.messages.length);
    bool result = true;

    SOPC_PubScheduler_MessageCtx* context = &(pubSchedulerCtx.messages.array[pubSchedulerCtx.messages.current]);

    context->transport = ctx;
    context->group = group;
    context->message = SOPC_Create_NetworkMessage_From_WriterGroup(group);
    if (NULL == context->message)
    {
        return false;
    }

    if (result)
    {
        context->postEncodeBuffer[0] = SOPC_Buffer_Create(SOPC_PUBSUB_BUFFER_SIZE);
        if (context->postEncodeBuffer[0] == NULL)
        {
            printf("# Error : message can't be created #%d - postEncodeBuffer[0] alloc failed\n",
                   (int32_t) pubSchedulerCtx.messages.current);
            result = false;
        }
    }

    if (result)
    {
        context->postEncodeBuffer[1] = SOPC_Buffer_Create(SOPC_PUBSUB_BUFFER_SIZE);
        if (context->postEncodeBuffer[1] == NULL)
        {
            printf("# Error : message can't be created #%d - postEncodeBuffer[1] alloc failed\n",
                   (int32_t) pubSchedulerCtx.messages.current);
            result = false;
        }
    }

    context->postEncodeBufferCurrent = 2;

    if (!result)
    {
        SOPC_Dataset_LL_NetworkMessage_Delete(context->message);
        context->message = NULL;

        SOPC_Buffer_Delete(context->postEncodeBuffer[0]);
        SOPC_Buffer_Delete(context->postEncodeBuffer[1]);

        context->postEncodeBufferCurrent = 2;

        printf("# Error : message can't be created #%d\n", (int32_t) pubSchedulerCtx.messages.current);
    }
    else
    {
        printf("# Info Publisher : message created #%d\n", (int32_t) pubSchedulerCtx.messages.current);
        // Successfull, free necessary if not further used
        pubSchedulerCtx.messages.current++;
    }
    return result;
}

static SOPC_PubScheduler_MessageCtx* SOPC_PubScheduler_MessageCtx_Get_Last(void)
{
    assert(0 < pubSchedulerCtx.messages.current && pubSchedulerCtx.messages.current <= pubSchedulerCtx.messages.length);
    return &(pubSchedulerCtx.messages.array[pubSchedulerCtx.messages.current - 1]);
}

static uint64_t SOPC_PubScheduler_Nb_Message(SOPC_PubSubConfiguration* config)
{
    uint64_t result = 0;
    const uint32_t nbConnection = SOPC_PubSubConfiguration_Nb_PubConnection(config);
    for (uint32_t i = 0; i < nbConnection; i++)
    {
        const SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, i);
        result = result + SOPC_PubSubConnection_Nb_WriterGroup(connection);
    }
    return result;
}

// RT Publisher callback functions (start and stop used only for debug, send function is really usefull

// Start callback, called when timer switch from DISABLED to ENABLED
static void SOPC_RT_Publisher_StartPubMsgCallback(uint32_t msgId,     // Message instance identifier
                                                  void* pUserContext) // User context
{
    printf("# RT Publisher start callback: Msg id = %u - User context %lu - Started\r\n", msgId,
           (uint64_t) pUserContext);
    return;
}

// Stop callback, called when timer switch from ENABLED to DISABLED
static void SOPC_RT_Publisher_StopPubMsgCallback(uint32_t msgId,     // Message instance identifier
                                                 void* pUserContext) // User context
{
    printf("# RT Publisher stop callback:  Msg id = %u - User context %lu - Stopped\r\n", msgId,
           (uint64_t) pUserContext);
    return;
}

// Elapsed callback, called when timer reach its configured period
static void SOPC_RT_Publisher_SendPubMsgCallback(uint32_t msgId,     // Message instance identifier
                                                 void* pUserContext, // User context
                                                 void* pData,        // Data published by set data API
                                                 uint32_t size)      // Data size in bytes
{
    (void) msgId;

    SOPC_PubScheduler_MessageCtx* pCtx = pUserContext;

    SOPC_Buffer buffer;
    buffer.data = pData;
    buffer.length = size;
    buffer.position = 0;
    buffer.maximum_size = size;
    buffer.initial_size = size;

#if DEBUG_PUBSUB_SCHEDULER_INFO
    printf("# RT Publisher send callback: Msg id = %u - User context %08lx - To encode size = %d!!!\r\n", //
           msgId,                                                                                         //
           (uint64_t) pUserContext,                                                                       //
           size);                                                                                         //
#endif

    if (size > 0 && pCtx != NULL && pCtx->transport != NULL && pCtx->transport->fctSend != NULL)
    {
        SOPC_ReturnStatus status = SOPC_STATUS_OK;
        if (pCtx->postEncodeBufferCurrent < 2)
        {
            pCtx->transport->fctSend(pCtx->transport,                                        //
                                     pCtx->postEncodeBuffer[pCtx->postEncodeBufferCurrent]); //
        }

        pubSchedulerCtx.sequenceNumber++; //
        status = SOPC_Buffer_Copy(pCtx->postEncodeBuffer[(pCtx->postEncodeBufferCurrent + 1) % 2], &buffer);

        if (status == SOPC_STATUS_OK)
        {
            pCtx->postEncodeBufferCurrent = (pCtx->postEncodeBufferCurrent + 1) % 2;
        }
    }

    return;
}

#if SOPC_PUBSCHEDULER_BEATHEART_FROM_IRQ == 0
// This callback increment call RT publisher beat heart with a monotonic counter post incremented
// then sleep during SOPC_TIMER_RESOLUTION_MS ms.
static void* SOPC_RT_Publisher_ThreadBeatHeartCallback(void* arg)
{
    (void) arg;

    volatile uint32_t beatHeart = 0;
    SOPC_ReturnStatus resultUpdateThread = SOPC_STATUS_OK;

    printf("# RT Publisher tick thread: Beat heart thread launched !!!\r\n");

    while (!pubSchedulerCtx.bQuitBeatHeart)
    {
        resultUpdateThread = SOPC_RT_Publisher_BeatHeart(pubSchedulerCtx.pRTPublisher, beatHeart++);
        if (resultUpdateThread != SOPC_STATUS_OK)
        {
            printf("# RT Publisher tick thread: SOPC_RT_Publisher_BeatHeart error = %d\r\n", resultUpdateThread);
        }

        SOPC_Sleep(SOPC_TIMER_RESOLUTION_MS);
    }

    printf("# RT Publisher tick thread: Quit Beat heart thread !!!\r\n");
    return NULL;
}
#endif

// This callback of thread var monitoring is porting from code of old event_timer callback
static void* SOPC_RT_Publisher_VarMonitoringCallback(void* arg)
{
    (void) arg;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("# Publisher variables monitoring: thread launched !!!\r\n");

    while (!pubSchedulerCtx.bQuitVarMonitoring)
    {
        for (uint32_t iIterMsg = 0; iIterMsg < pubSchedulerCtx.messages.current; iIterMsg++)
        {
            {
                bool allocSuccess = true;
                bool typeCheckingSuccess = true;
                SOPC_PubScheduler_MessageCtx* ctx = &pubSchedulerCtx.messages.array[iIterMsg];
                SOPC_Dataset_NetworkMessage* message = ctx->message;
                assert(NULL != ctx->message);
                // only one datasetmessage is managed
                assert(1 == SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(message));
                SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(message, 0);
                uint16_t nbFields = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);

                // get datavalue for the unique datasetwriter
                const SOPC_DataSetWriter* writer = SOPC_PubScheduler_Group_Get_Unique_Writer(ctx->group);
                const SOPC_PublishedDataSet* dataset = SOPC_DataSetWriter_Get_DataSet(writer);
                assert(SOPC_PublishedDataSet_Nb_FieldMetaData(dataset) == nbFields);

                // Fill datasetmessage
                SOPC_DataValue* values = SOPC_PubSourceVariable_GetVariables(pubSchedulerCtx.sourceConfig, //
                                                                             dataset);                     //

                if (NULL == values)
                {
                    printf("# Publisher variable monitoring : Warning, request response unexpected values !!!\r\n");
                }
                else
                {
                    // Check compatibility regarding PubSub configuration
                    for (uint16_t i = 0; i < nbFields; i++)
                    {
                        SOPC_FieldMetaData* fieldData = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, i);
                        assert(NULL != fieldData);
                        SOPC_DataValue* dv = &values[i];

                        bool isBad = false;
                        bool isCompatible = SOPC_PubSubHelpers_IsCompatibleVariant(fieldData, &dv->Value, &isBad);

                        // We also want to consider case the value was valid but provided with Bad status, therefore we
                        // do not consider isNullOrBad in the first condition
                        if (isCompatible && 0 != (dv->Status & SOPC_BadStatusMask))
                        {
                            // If status is bad and value has not status type, set value as a Bad status code
                            // (see spec part 14: Table 16: DataSetMessage field representation options)
                            if (SOPC_StatusCode_Id != dv->Value.BuiltInTypeId)
                            {
                                SOPC_Variant_Clear(&dv->Value);
                                dv->Value.BuiltInTypeId = SOPC_StatusCode_Id;
                                dv->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
                                dv->Value.Value.Status = dv->Status;
                            }
                        }
                        else if (!isCompatible || isBad)
                        {
                            // If value is Bad whereas the data value status code is not bad, it is considered as an
                            // error since this is not the type expected for this value from an OpcUa server.

                            if (!isCompatible)
                            {
                                printf(
                                    "# Publisher variables monitoring: Error: incompatible variants between Address "
                                    "Space and Pub config.\r\n");
                            }
                            typeCheckingSuccess = false;
                        }
                    }

                    for (uint16_t dsf_index = 0; dsf_index < nbFields && allocSuccess && typeCheckingSuccess;
                         dsf_index++)
                    {
                        SOPC_FieldMetaData* metadata = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, dsf_index);
                        SOPC_Variant* variant = SOPC_Variant_Create();
                        // TODO: log an error if null
                        allocSuccess = (NULL != variant);
                        if (allocSuccess)
                        {
                            SOPC_Variant_Move(variant, &(values[dsf_index].Value));
                            SOPC_NetworkMessage_Set_Variant_At(message,
                                                               0, // unique datasetmessage
                                                               dsf_index, variant, metadata);
                        }
                    }

                    for (uint16_t dsf_index = 0; dsf_index < nbFields; dsf_index++)
                    {
                        SOPC_DataValue_Clear(&(values[dsf_index]));
                    }
                    SOPC_Free(values);

                    // send message
                    if (allocSuccess && typeCheckingSuccess)
                    {
                        if (allocSuccess)
                        {
                            SOPC_Buffer buffer;

                            memset(&buffer, 0, sizeof(SOPC_Buffer));

                            // Get buffer where message is pre-encoded.
                            status = SOPC_RT_Publisher_GetBuffer(pubSchedulerCtx.pRTPublisher, //
                                                                 ctx->rt_publisher_msg_id,     //
                                                                 &buffer);                     //

                            // If successful, pre-encode message into double buffer
                            if (SOPC_STATUS_OK == status)
                            {
                                SOPC_Buffer* pBuffer = SOPC_UADP_NetworkMessage_Encode(message);

                                if (pBuffer != NULL)
                                {
                                    status = SOPC_Buffer_Copy(&buffer, pBuffer);
                                    SOPC_Buffer_Delete(pBuffer);
                                }

                                // Commit buffer with significant bytes
                                SOPC_RT_Publisher_ReleaseBuffer(pubSchedulerCtx.pRTPublisher, //
                                                                ctx->rt_publisher_msg_id,     //
                                                                &buffer);                     //
                            }
                        }
                    }
                }
            }
        }

        SOPC_Sleep(SOPC_TIMER_RESOLUTION_MS);
    }

    printf("# Publisher variables monitoring: quit variable monitoring thread !!!\r\n");

    return NULL;
}

#if SOPC_PUBSCHEDULER_BEATHEART_FROM_IRQ == 1
SOPC_ReturnStatus SOPC_PubScheduler_BeatHeartFromIRQ(uint32_t tickValue)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    // Verify that scheduler is not already running
    if (true == SOPC_Atomic_Int_Get(&pubSchedulerCtx.isStarted) &&
        false == SOPC_Atomic_Int_Get(&pubSchedulerCtx.processingStartStop))
    {
        result = SOPC_RT_Publisher_BeatHeart(pubSchedulerCtx.pRTPublisher, tickValue);
    }
    else
    {
        result = SOPC_STATUS_INVALID_STATE;
    }

    return result;
}
#endif
bool SOPC_PubScheduler_Start(SOPC_PubSubConfiguration* config, SOPC_PubSourceVariableConfig* sourceConfig)
{
    bool allocSuccess = true;

    SOPC_ReturnStatus resultSOPC = SOPC_STATUS_OK;
    SOPC_PubScheduler_TransportCtx* transportCtx = NULL;

    SOPC_Helper_EndiannessCfg_Initialize(); // TODO: centralize / avoid recompute in S2OPC !

    if (NULL == config || NULL == sourceConfig)
    {
        return false;
    }

    // Verify that scheduler is not already running
    if (true == SOPC_Atomic_Int_Get(&pubSchedulerCtx.isStarted) ||
        true == SOPC_Atomic_Int_Get(&pubSchedulerCtx.processingStartStop))
    {
        return false;
    }
    else
    {
        SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, true);
    }

    pubSchedulerCtx.sequenceNumber = 1;

    // Create Context Array to keep Addresses and Sockets
    const uint32_t nbConnection = SOPC_PubSubConfiguration_Nb_PubConnection(config);
    bool result = nbConnection > 0;

    if (result)
    {
        pubSchedulerCtx.config = config;
        pubSchedulerCtx.sourceConfig = sourceConfig;
        allocSuccess = SOPC_PubScheduler_MessageCtx_Array_Initialize(config);

        pubSchedulerCtx.nbConnection = nbConnection;
        if (allocSuccess)
        {
            pubSchedulerCtx.transport = SOPC_Calloc(nbConnection, sizeof(SOPC_PubScheduler_TransportCtx));
            allocSuccess = (NULL != pubSchedulerCtx.transport);
        }

        // Creation of RT Publisher

        pubSchedulerCtx.pRTPublisher = SOPC_RT_Publisher_Create();
        if (pubSchedulerCtx.pRTPublisher == NULL)
        {
            printf("# Error, can't create rt publisher :(\r\n");
            resultSOPC = SOPC_STATUS_NOK;
            allocSuccess = false;
        }
        else
        {
            printf("# RT publisher created :)\r\n");
        }

        // Creation of RT_Pubslisher Initializer. This will be destroyed after initialization.

        SOPC_RT_Publisher_Initializer* pRTInitializer = NULL;
        if (SOPC_STATUS_OK == resultSOPC)
        {
            pRTInitializer = SOPC_RT_Publisher_Initializer_Create(2048);
            if (NULL == pRTInitializer)
            {
                printf("# Error, can't create rt pub initializer :(\r\n");
                resultSOPC = SOPC_STATUS_NOK;
                allocSuccess = false;
            }
            else
            {
                printf("# RT publisher initializer created :)\r\n");
            }
        }

        // Create the Timer for each Writer Group
        for (uint32_t i = 0; i < nbConnection && allocSuccess; i++)
        {
            SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, i);

            allocSuccess = SOPC_PubScheduler_Connection_Get_Transport(i, connection, &transportCtx);

            const uint16_t nbWriterGroup = SOPC_PubSubConnection_Nb_WriterGroup(connection);
            for (uint16_t j = 0; j < nbWriterGroup && allocSuccess; j++)
            {
                SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, j);
                uint64_t publishingInterval = SOPC_WriterGroup_Get_PublishingInterval(group);
                allocSuccess = SOPC_PubScheduler_MessageCtx_Array_Init_Next(transportCtx, group);
                if (allocSuccess)
                {
                    SOPC_PubScheduler_MessageCtx* msgctx = SOPC_PubScheduler_MessageCtx_Get_Last();

                    // Add a message to rt publisher initializer

                    printf("# RT Publisher initializer : Creation of message with publishing value = %lu\r\n",
                           (uint64_t) publishingInterval);

                    resultSOPC = SOPC_RT_Publisher_Initializer_AddMessage(
                        pRTInitializer, //
                        (uint32_t) publishingInterval > (uint32_t) SOPC_TIMER_RESOLUTION_MS
                            ? (uint32_t) publishingInterval / SOPC_TIMER_RESOLUTION_MS
                            : (uint32_t) SOPC_TIMER_RESOLUTION_MS / SOPC_TIMER_RESOLUTION_MS, // period in ticks
                        0,                                                                    // offset in ticks
                        msgctx,                                                               // Context
                        SOPC_RT_Publisher_StartPubMsgCallback,                                // Not used
                        SOPC_RT_Publisher_SendPubMsgCallback,     // Wrap send callback of transport context
                        SOPC_RT_Publisher_StopPubMsgCallback,     // Not used
                        SOPC_RT_PUBLISHER_MSG_PUB_STATUS_ENABLED, // Publication started
                        &msgctx->rt_publisher_msg_id);            // Message identifier used to update data

                    if (SOPC_STATUS_OK != resultSOPC)
                    {
                        allocSuccess = false;
                        printf("# RT Publisher initializer : Error creation of rt publisher message :(\r\n");
                    }
                    else
                    {
                        printf("# RT Publisher initializer : Creation of rt publisher message handle = %d\r\n",
                               msgctx->rt_publisher_msg_id);
                    }
                }
            }
        }

        // Initalize RT Publisher with initializer

        resultSOPC = SOPC_RT_Publisher_Initialize(pubSchedulerCtx.pRTPublisher, pRTInitializer);

        if (SOPC_STATUS_OK != resultSOPC)
        {
            allocSuccess = false;
            printf("# Error, can't initialize RT Publisher : %d", resultSOPC);
        }
        else
        {
            printf("# RT Publisher well initialized :)");
        }

        // Destroy initializer not further used

        SOPC_RT_Publisher_Initializer_Destroy(&pRTInitializer);

#if SOPC_PUBSCHEDULER_BEATHEART_FROM_IRQ == 0
        // Creation of beat heart thread which call RT Publisher Beat Heart

        if (allocSuccess)
        {
            pubSchedulerCtx.bQuitBeatHeart = false;
            resultSOPC = SOPC_Thread_Create(&pubSchedulerCtx.handleThreadBeatHeart,    //
                                            SOPC_RT_Publisher_ThreadBeatHeartCallback, //
                                            NULL,                                      //
                                            "PubHeart");                               //

            if (SOPC_STATUS_OK != resultSOPC)
            {
                allocSuccess = false;
                printf("# Error creation of rt publisher beat heart thread\r\n");
            }
        }
#endif

        // Creation of variables monitoring thread

        if (allocSuccess)
        {
            pubSchedulerCtx.bQuitVarMonitoring = false;
            resultSOPC = SOPC_Thread_Create(&pubSchedulerCtx.handleThreadVarMonitoring, //
                                            SOPC_RT_Publisher_VarMonitoringCallback,    //
                                            NULL,                                       //
                                            "PubVar");                                  //

            if (SOPC_STATUS_OK != resultSOPC)
            {
                allocSuccess = false;
                printf("# Error creation of var monitoring thread\r\n");
            }
        }

        if (!allocSuccess)
        {
            SOPC_PubScheduler_Context_Clear();
        }
    }

    SOPC_Atomic_Int_Set(&pubSchedulerCtx.isStarted, allocSuccess);
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, false);
    return allocSuccess;
}

void SOPC_PubScheduler_Stop(void)
{
    printf("# Try to enter scheduler stop ...\r\n");
    if (false == SOPC_Atomic_Int_Get(&pubSchedulerCtx.isStarted) ||
        true == SOPC_Atomic_Int_Get(&pubSchedulerCtx.processingStartStop))
    {
        printf("# Scheduler already stopping or stopped\r\n");
        return;
    }
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, true);
    // true because isStarted is false
    assert(pubSchedulerCtx.nbConnection > 0);
    SOPC_PubScheduler_Context_Clear();

    SOPC_Atomic_Int_Set(&pubSchedulerCtx.isStarted, false);
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, false);
    printf("# Scheduler stopped ...\r\n");
}

void SOPC_PubScheduler_Finalize(void)
{
    return;
}

static const SOPC_DataSetWriter* SOPC_PubScheduler_Group_Get_Unique_Writer(const SOPC_WriterGroup* group)
{
    assert(NULL != group);
    uint8_t nb = SOPC_WriterGroup_Nb_DataSetWriter(group);
    assert(1 == nb);
    return SOPC_WriterGroup_Get_DataSetWriter_At(group, 0);
}

// Get Socket and Multicast Address then save it in context
static bool SOPC_PubScheduler_Connection_Get_Transport(uint32_t index,
                                                       SOPC_PubSubConnection* connection,
                                                       SOPC_PubScheduler_TransportCtx** ctx)
{
    const char* address = SOPC_PubSubConnection_Get_Address(connection);
    SOPC_PubSubProtocol_Type protocol = SOPC_PubSub_Protocol_From_URI(address);
    switch (protocol)
    {
    case SOPC_PubSubProtocol_UDP:
    {
        Socket out_sock;
        SOPC_Socket_AddressInfo* out_multicastAddr;
        bool allocSuccess = SOPC_PubSubHelpers_Publisher_ParseMulticastAddress(address,             //
                                                                               &out_multicastAddr); //
        if (!allocSuccess)
        {
            return false;
        }
        pubSchedulerCtx.transport[index].multicastAddr = out_multicastAddr;
        allocSuccess = (SOPC_STATUS_OK == SOPC_UDP_Socket_CreateToSend(out_multicastAddr, &out_sock));
        if (!allocSuccess)
        {
            *ctx = NULL;
            return false;
        }
        pubSchedulerCtx.transport[index].sock = out_sock;
        pubSchedulerCtx.transport[index].fctClear = &SOPC_PubScheduler_CtxUdp_Clear;
        pubSchedulerCtx.transport[index].fctSend = &SOPC_PubScheduler_CtxUdp_Send;
        *ctx = &pubSchedulerCtx.transport[index];
        return true;
    }
    break;

    default:
    {
        *ctx = NULL;
        return false;
    }
    break;
    }
}

static void SOPC_PubScheduler_CtxUdp_Clear(SOPC_PubScheduler_TransportCtx* ctx)
{
    SOPC_UDP_SocketAddress_Delete(&(ctx->multicastAddr));
    SOPC_UDP_Socket_Close(&(ctx->sock));
}

static void SOPC_PubScheduler_CtxUdp_Send(SOPC_PubScheduler_TransportCtx* ctx, SOPC_Buffer* buffer)
{
    SOPC_UDP_Socket_SendTo(ctx->sock, ctx->multicastAddr, buffer);
}
