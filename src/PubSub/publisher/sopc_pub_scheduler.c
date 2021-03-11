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
#include <errno.h>
#include <inttypes.h>

#include "sopc_atomic.h"
#include "sopc_crypto_provider.h"
#include "sopc_dataset_layer.h"
#include "sopc_event_handler.h"
#include "sopc_event_timer_manager.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_mem_alloc.h"
#include "sopc_mqtt_transport_layer.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_constants.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_pubsub_local_sks.h"
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

// Specific callback for MQTT message. No treatment for PUBLISHER.
static void on_mqtt_message_received(MqttTransportHandle* pCtx, /* Transport context handle */
                                     uint8_t* data,             /* Data received */
                                     uint16_t size,             /* Size of data received, in bytes. */
                                     void* pUserContext);       /* User context, used as pub sub connection */

// Clear a Transport MQTT context. Implements SOPC_PubScheduler_TransportCtx_Clear
static void SOPC_PubScheduler_CtxMqtt_Clear(SOPC_PubScheduler_TransportCtx* ctx);

// Send an MQTT message. Implements SOPC_PubScheduler_TransportCtx_Send
static void SOPC_PubScheduler_CtxMqtt_Send(SOPC_PubScheduler_TransportCtx* ctx, SOPC_Buffer* buffer);

/* Context of a UDP Multicast Message */
struct SOPC_PubScheduler_TransportCtx
{
    SOPC_Socket_AddressInfo* multicastAddr;
    Socket sock;

    SOPC_PubScheduler_TransportCtx_Clear fctClear;
    SOPC_PubScheduler_TransportCtx_Send fctSend;

    // specific to SOPC_PubSubProtocol_MQTT
    MqttTransportHandle* mqttHandle;
};

typedef struct SOPC_PubScheduler_MessageCtx
{
    SOPC_WriterGroup* group;
    SOPC_Dataset_NetworkMessage* message;
    SOPC_PubScheduler_TransportCtx* transport;
    SOPC_PubSub_SecurityType* security;
    uint32_t rt_publisher_msg_id;
} SOPC_PubScheduler_MessageCtx;

typedef struct SOPC_PubScheduler_MessageCtx_Array
{
    uint64_t length;                     // Size of this array is SOPC_PubScheduler_Nb_Message
    uint64_t current;                    // Nb of messages already initialized. Monotonic.
    SOPC_PubScheduler_MessageCtx* array; // MessageCtx: array of context for each message
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

    //  SOPC_LocalSKS_Keys *keys;
    // A strictly monotonically increasing sequence number for a SecurityTokenId and PublisherId combination.
    uint32_t sequenceNumber;

    // Thread variable monitoring

    Thread handleThreadVarMonitoring;
    bool bQuitVarMonitoring;

    // Thread which call beat heart function of RT Publisher

    SOPC_RT_Publisher* pRTPublisher; // RT Publisher

    Thread handleThreadHeartBeat; // Handle thread simulates IRQ. This thread call HeartBeat publisher function.
    bool bQuitBeatHeart;          // Quit control of thread which simulates IRQ

} pubSchedulerCtx = {.isStarted = false,
                     .processingStartStop = false,
                     .config = NULL,
                     .sourceConfig = NULL,
                     .nbConnection = 0,
                     .transport = NULL,
                     .messages.length = 0,
                     .messages.current = 0,
                     .messages.array = NULL,
                     .sequenceNumber = 1,
                     .pRTPublisher = NULL,
                     .bQuitBeatHeart = true,
                     .handleThreadHeartBeat = (Thread) NULL,
                     .bQuitVarMonitoring = true,
                     .handleThreadVarMonitoring = (Thread) NULL};

// This callback of thread var monitoring is porting from code of old event_timer callback
static void* SOPC_RT_Publisher_VarMonitoringCallback(void* arg);

// This callback increment call RT publisher beat heart with a monotonic counter post incremented
// then sleep during SOPC_TIMER_RESOLUTION_MS ms.
static void* SOPC_RT_Publisher_ThreadHeartBeatCallback(void* arg);

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
    printf("# Info: Stop beat heart thread\r\n");

    bool newQuitBeatHeart = true;

    __atomic_store(&pubSchedulerCtx.bQuitBeatHeart, &newQuitBeatHeart, __ATOMIC_SEQ_CST);

    if (pubSchedulerCtx.handleThreadHeartBeat != (Thread) NULL)
    {
        SOPC_Thread_Join(pubSchedulerCtx.handleThreadHeartBeat);
        pubSchedulerCtx.handleThreadHeartBeat = (Thread) NULL;
    }

    printf("# Info: Stop var monitoring thread\r\n");

    bool newVarMonitoringStatus = true;
    __atomic_store(&pubSchedulerCtx.bQuitVarMonitoring, &newVarMonitoringStatus, __ATOMIC_SEQ_CST);

    if (pubSchedulerCtx.handleThreadVarMonitoring != (Thread) NULL)
    {
        SOPC_Thread_Join(pubSchedulerCtx.handleThreadVarMonitoring);
        pubSchedulerCtx.handleThreadVarMonitoring = (Thread) NULL;
    }

    /* Clear RT publisher */
    printf("# Info: Deinit and destroy rt publisher \r\n");
    SOPC_ReturnStatus result = SOPC_RT_Publisher_DeInitialize(pubSchedulerCtx.pRTPublisher);

    /* Destroy RT Publisher.*/

    SOPC_RT_Publisher_Destroy(&pubSchedulerCtx.pRTPublisher);

    /* Destroy messages and messages array */
    SOPC_PubScheduler_MessageCtx_Array_Clear();

    /* Destroy transport context */
    if (pubSchedulerCtx.transport != NULL)
    {
        for (uint32_t i = 0; i < pubSchedulerCtx.nbConnection; i++)
        {
            if (pubSchedulerCtx.transport[i].fctClear != NULL)
            {
                pubSchedulerCtx.transport[i].fctClear(&pubSchedulerCtx.transport[i]);
                printf("# Info: transport context destroyed for connection #%u (publisher). \n",
                       pubSchedulerCtx.nbConnection);
            }
        }

        /* Destroy transport contexts array */
        SOPC_Free(pubSchedulerCtx.transport);
        pubSchedulerCtx.transport = NULL;
    }
    pubSchedulerCtx.nbConnection = 0;
    pubSchedulerCtx.config = NULL;
    pubSchedulerCtx.sourceConfig = NULL;
    // TODO SOPC_LocalSKS_Keys_Delete(pubSchedulerCtx.keys);
    /* Don't reset the sequenceNumber on Stop(). But for now, there is no other place to reset it */
    // pubSchedulerCtx.sequenceNumber = 1;
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
            printf("# Info: Network message #%u. destroyed \n", i);
            SOPC_Dataset_LL_NetworkMessage_Delete(pubSchedulerCtx.messages.array[i].message);
            pubSchedulerCtx.messages.array[i].message = NULL;
            SOPC_PubSub_Security_Clear(pubSchedulerCtx.messages.array[i].security);
            SOPC_Free(pubSchedulerCtx.messages.array[i].security);
            pubSchedulerCtx.messages.array[i].security = NULL;
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
        SOPC_SecurityMode_Type smode = SOPC_WriterGroup_Get_SecurityMode(group);
        if (SOPC_SecurityMode_Sign == smode || SOPC_SecurityMode_SignAndEncrypt == smode)
        {
            context->security = SOPC_Calloc(1, sizeof(SOPC_PubSub_SecurityType));
            if (NULL == context->security)
            {
                printf("# Error Publisher : can't alloc security context !\n");
                result = false;
            }

            if (result)
            {
                context->security->mode = SOPC_WriterGroup_Get_SecurityMode(group);
                context->security->groupKeys =
                    SOPC_LocalSKS_GetSecurityKeys(SOPC_PUBSUB_SKS_DEFAULT_GROUPID, SOPC_PUBSUB_SKS_CURRENT_TOKENID);
                context->security->provider = SOPC_CryptoProvider_CreatePubSub(SOPC_PUBSUB_SECURITY_POLICY);
                if (NULL == context->security->groupKeys || NULL == context->security->provider)
                {
                    printf("# Error Publisher : can't create provider !\n");
                    result = false;
                }
            }
        }
    }

    if (!result)
    {
        SOPC_Dataset_LL_NetworkMessage_Delete(context->message);
        context->message = NULL;
        if (context->security != NULL)
        {
            SOPC_PubSub_Security_Clear(context->security);
            SOPC_Free(context->security);
            context->security = NULL;
        }

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

// Elapsed callback, called when timer reach its configured period
static void SOPC_RT_Publisher_SendPubMsgCallback(uint32_t msgId,     // Message instance identifier
                                                 void* pUserContext, // User context
                                                 void* pData,        // Data published by set data API
                                                 uint32_t size)      // Data size in bytes
{
    (void) msgId;

    if (0 == size)
    {
        // No data available on PublishingInterval
        return;
    }

    SOPC_PubScheduler_MessageCtx* pCtx = pUserContext;

    SOPC_Buffer buffer;
    buffer.data = pData;
    buffer.length = size;
    buffer.position = 0;
    buffer.maximum_size = size;
    buffer.initial_size = size;

    //printf("# RT Publisher send callback: Msg id = %u - User context %08lx - To encode size = %d!!!\r\n",
    //       msgId,
    //       (uint64_t) pUserContext,
    //       size);

    pCtx->transport->fctSend(pCtx->transport, &buffer);

    return;
}

// This callback increment call RT publisher beat heart with a monotonic counter post incremented
// then sleep during SOPC_TIMER_RESOLUTION_MS ms.
static void* SOPC_RT_Publisher_ThreadHeartBeatCallback(void* arg)
{
    uintptr_t resMilliSecs = (uintptr_t) arg;

    uint32_t heartBeat = 0;
    SOPC_ReturnStatus resultUpdateThread = SOPC_STATUS_OK;
    SOPC_TimeReference currentRef = SOPC_TimeReference_GetCurrent();
    SOPC_TimeReference targetRef = SOPC_TimeReference_AddMilliseconds(currentRef, resMilliSecs);

    printf("# RT Publisher tick thread: Beat heart thread launched !!!\r\n");

    bool readQuitHeartBeat = false;
    __atomic_load(&pubSchedulerCtx.bQuitBeatHeart, &readQuitHeartBeat, __ATOMIC_SEQ_CST);
    while (!readQuitHeartBeat)
    {
        currentRef = SOPC_TimeReference_GetCurrent();
        while (targetRef <= currentRef)
        {
            // Call heart beat function 1 time per resolution time elapsed since last evaluation
            heartBeat++;
            resultUpdateThread = SOPC_RT_Publisher_HeartBeat(pubSchedulerCtx.pRTPublisher, heartBeat);
            // next target is previous target + resolution time (it might already be elapsed)
            targetRef = SOPC_TimeReference_AddMilliseconds(targetRef, resMilliSecs);
        }
        if (resultUpdateThread != SOPC_STATUS_OK)
        {
            printf("# RT Publisher tick thread: SOPC_RT_Publisher_BeatHeart error = %d\r\n", (int) resultUpdateThread);
        }

        SOPC_Sleep((unsigned int) resMilliSecs);
        __atomic_load(&pubSchedulerCtx.bQuitBeatHeart, &readQuitHeartBeat, __ATOMIC_SEQ_CST);
    }

    printf("# RT Publisher tick thread: Quit Beat heart thread !!!\r\n");
    return NULL;
}

// This callback of thread var monitoring is porting from code of old event_timer callback
static void* SOPC_RT_Publisher_VarMonitoringCallback(void* arg)
{
    uintptr_t resMilliSecs = (uintptr_t) arg;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Choose a reasonable minimum resolution to get variables
    // retrieve minimum publishing interval to manage (if some messages defined)
    if (pubSchedulerCtx.messages.current > 0)
    {
        uint64_t minPubInterval = UINT64_MAX;
        for (uint32_t iIterMsg = 0; iIterMsg < pubSchedulerCtx.messages.current; iIterMsg++)
        {
            SOPC_PubScheduler_MessageCtx* ctx = &pubSchedulerCtx.messages.array[iIterMsg];
            uint64_t pubInterval = SOPC_WriterGroup_Get_PublishingInterval(ctx->group);
            if (minPubInterval > pubInterval)
            {
                minPubInterval = pubInterval;
            }
        }
        // minimum resolution = minimum publish interval / 2
        if (minPubInterval / 2 > resMilliSecs)
        {
            assert(minPubInterval <= UINT32_MAX);
            resMilliSecs = (uint32_t)(minPubInterval / 2);
        }
    }

    printf("# Publisher variables monitoring: thread launched !!!\r\n");

    bool readVarMonitoringStatus = false;
    __atomic_load(&pubSchedulerCtx.bQuitVarMonitoring, &readVarMonitoringStatus, __ATOMIC_SEQ_CST);

    SOPC_TimeReference startTimeRef = SOPC_TimeReference_GetCurrent();
    SOPC_TimeReference prevTimeRef = startTimeRef;

    while (!readVarMonitoringStatus)
    {
        SOPC_TimeReference currentTimeRef = SOPC_TimeReference_GetCurrent();

        for (uint32_t iIterMsg = 0; iIterMsg < pubSchedulerCtx.messages.current; iIterMsg++)
        {
            bool allocSuccess = true;
            bool typeCheckingSuccess = true;
            SOPC_PubScheduler_MessageCtx* ctx = &pubSchedulerCtx.messages.array[iIterMsg];

            // Note: use half interval data update to ensure data up to date for sending on PublishingInterval
            uint64_t halfPubInterval = SOPC_WriterGroup_Get_PublishingInterval(ctx->group) / 2;

            // Check if we should retrieve a new value:
            // - First iteration OR
            // - Number of publications expected is greater than the one on previous evaluation
            if (prevTimeRef != startTimeRef &&
                (currentTimeRef - startTimeRef) / halfPubInterval == (prevTimeRef - startTimeRef) / halfPubInterval)
            {
                continue;
            }

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

                for (uint16_t dsf_index = 0; dsf_index < nbFields && allocSuccess && typeCheckingSuccess; dsf_index++)
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
                    SOPC_PubSub_SecurityType* security = ctx->security;

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
                            if (NULL != security)
                            {
                                security->msgNonceRandom = SOPC_PubSub_Security_Random(security->provider);
                                allocSuccess = (NULL != security->msgNonceRandom);
                                if (allocSuccess)
                                {
                                    security->sequenceNumber = pubSchedulerCtx.sequenceNumber;
                                    pubSchedulerCtx.sequenceNumber++;
                                }
                                else
                                {
                                    status = SOPC_STATUS_NOK;
                                }
                            }

                            if (SOPC_STATUS_OK == status)
                            {
                                SOPC_Buffer* pBuffer = SOPC_UADP_NetworkMessage_Encode(message, security);
                                if (NULL != security)
                                {
                                    SOPC_Free(security->msgNonceRandom);
                                    security->msgNonceRandom = NULL;
                                }
                                if (pBuffer != NULL)
                                {
                                    status = SOPC_Buffer_Copy(&buffer, pBuffer);
                                    SOPC_Buffer_Delete(pBuffer);
                                }
                            }
                            // Commit buffer with significant bytes
                            bool bCancel = false;
                            if (SOPC_STATUS_OK != status)
                            {
                                bCancel = true;
                            }

                            SOPC_RT_Publisher_ReleaseBuffer(pubSchedulerCtx.pRTPublisher, //
                                                            ctx->rt_publisher_msg_id,     //
                                                            &buffer,                      //
                                                            bCancel);                     //
                        }
                    }
                }
            }
        }

        SOPC_Sleep((unsigned int) resMilliSecs);

        __atomic_load(&pubSchedulerCtx.bQuitVarMonitoring, &readVarMonitoringStatus, __ATOMIC_SEQ_CST);
    }

    printf("# Publisher variables monitoring: quit variable monitoring thread !!!\r\n");

    return NULL;
}

bool SOPC_PubScheduler_Start(SOPC_PubSubConfiguration* config,
                             SOPC_PubSourceVariableConfig* sourceConfig)
{
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

    // Security : init the sequence number
    /* TODO: Don't reset the sequenceNumber here: we might want to publish later and we must keep its value */
    // pubSchedulerCtx.sequenceNumber = 1;
    pubSchedulerCtx.nbConnection = 0;

    // Create Context Array to keep Addresses and Sockets
    const uint32_t nbConnection = SOPC_PubSubConfiguration_Nb_PubConnection(config);
    resultSOPC = nbConnection > 0 ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    if (SOPC_STATUS_OK == resultSOPC)
    {
        pubSchedulerCtx.config = config;
        pubSchedulerCtx.sourceConfig = sourceConfig;
        {
            if (!SOPC_PubScheduler_MessageCtx_Array_Initialize(config))
            {
                resultSOPC = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == resultSOPC)
        {
            pubSchedulerCtx.transport = SOPC_Calloc(nbConnection, sizeof(SOPC_PubScheduler_TransportCtx));
            if (NULL == pubSchedulerCtx.transport)
            {
                resultSOPC = SOPC_STATUS_NOK;
            }
            else
            {
                pubSchedulerCtx.nbConnection = nbConnection;
            }
        }

        if (SOPC_STATUS_OK == resultSOPC)
        {
            // Creation of RT Publisher

            pubSchedulerCtx.pRTPublisher = SOPC_RT_Publisher_Create();
            if (NULL == pubSchedulerCtx.pRTPublisher)
            {
                printf("# Error, can't create rt publisher :(\r\n");
                resultSOPC = SOPC_STATUS_NOK;
            }
            else
            {
                printf("# RT publisher created :)\r\n");
            }
        }

        SOPC_RT_Publisher_Initializer* pRTInitializer = NULL;
        // Creation of RT_Pubslisher Initializer. This will be destroyed after initialization.
        if (SOPC_STATUS_OK == resultSOPC)
        {
            pRTInitializer = SOPC_RT_Publisher_Initializer_Create(2048);
            if (NULL == pRTInitializer)
            {
                printf("# Error, can't create rt pub initializer :(\r\n");
                resultSOPC = SOPC_STATUS_NOK;
            }
            else
            {
                printf("# RT publisher initializer created :)\r\n");
            }
        }

        if (SOPC_STATUS_OK == resultSOPC)
        {
            // Create the Timer for each Writer Group
            for (uint32_t i = 0; (i < nbConnection) && (SOPC_STATUS_OK == resultSOPC); i++)
            {
                SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, i);

                if (!SOPC_PubScheduler_Connection_Get_Transport(i, connection, &transportCtx))
                {
                    resultSOPC = SOPC_STATUS_NOK;
                }
                else
                {
                    const uint16_t nbWriterGroup = SOPC_PubSubConnection_Nb_WriterGroup(connection);
                    for (uint16_t j = 0; (j < nbWriterGroup) && (SOPC_STATUS_OK == resultSOPC); j++)
                    {
                        SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, j);
                        uint64_t publishingInterval = SOPC_WriterGroup_Get_PublishingInterval(group);
                        if (publishingInterval > UINT32_MAX)
                        {
                            resultSOPC = SOPC_STATUS_NOT_SUPPORTED;
                        }
                        else if (!SOPC_PubScheduler_MessageCtx_Array_Init_Next(transportCtx, group))
                        {
                            resultSOPC = SOPC_STATUS_NOK;
                        }
                        else
                        {
                            SOPC_PubScheduler_MessageCtx* msgctx = SOPC_PubScheduler_MessageCtx_Get_Last();

                            // Add a message to rt publisher initializer

                            printf("# RT Publisher initializer : Creation of message with publishing value = %" PRIu64
                                   "\r\n",
                                   publishingInterval);

                            resultSOPC = SOPC_RT_Publisher_Initializer_AddMessage(
                                pRTInitializer, //
                                1,                                  // period in ticks (minimum is 1 tick)
                                0,                                        // offset in ticks
                                msgctx,                                   // Context
                                NULL,    // Not used
                                SOPC_RT_Publisher_SendPubMsgCallback,     // Wrap send callback of transport context
                                NULL,     // Not used
                                SOPC_RT_PUBLISHER_MSG_PUB_STATUS_ENABLED, // Publication started
                                &msgctx->rt_publisher_msg_id);            // Message identifier used to update data

                            if (SOPC_STATUS_OK != resultSOPC)
                            {
                                printf("# RT Publisher initializer : Error creation of rt publisher message :(\r\n");
                            }
                            else
                            {
                                printf("# RT Publisher initializer : Creation of rt publisher message handle = %u\r\n",
                                       msgctx->rt_publisher_msg_id);
                            }
                        }
                    }
                }
            }
        }

        if (SOPC_STATUS_OK == resultSOPC)
        {
            // Initalize RT Publisher with initializer

            resultSOPC = SOPC_RT_Publisher_Initialize(pubSchedulerCtx.pRTPublisher, pRTInitializer);

            if (SOPC_STATUS_OK != resultSOPC)
            {
                printf("# Error, can't initialize RT Publisher : %d", (int) resultSOPC);
            }
            else
            {
                printf("# RT Publisher well initialized\n");
            }
        }

        // Destroy initializer not further used
        SOPC_RT_Publisher_Initializer_Destroy(&pRTInitializer);

        /* Creation of the time-sensitive thread */
        if (SOPC_STATUS_OK == resultSOPC)
        {
            bool newQuitHeartBeat = false;
            __atomic_store(&pubSchedulerCtx.bQuitBeatHeart, &newQuitHeartBeat, __ATOMIC_SEQ_CST);
            resultSOPC =
                SOPC_Thread_Create(&pubSchedulerCtx.handleThreadHeartBeat, SOPC_RT_Publisher_ThreadHeartBeatCallback,
                                   1, "PubHeart");

            if (SOPC_STATUS_OK != resultSOPC)
            {
                printf("# Error creation of rt publisher heart beat thread\r\n");
            }
        }

        // Creation of variables monitoring thread
        if (SOPC_STATUS_OK == resultSOPC)
        {
            bool newVarMonitoringStatus = false;
            __atomic_store(&pubSchedulerCtx.bQuitVarMonitoring, &newVarMonitoringStatus, __ATOMIC_SEQ_CST);

            resultSOPC = SOPC_Thread_Create(&pubSchedulerCtx.handleThreadVarMonitoring, //
                                            SOPC_RT_Publisher_VarMonitoringCallback,    //
                                            1,  //
                                            "PubVar");                                  //

            if (SOPC_STATUS_OK != resultSOPC)
            {
                printf("# Error creation of var monitoring thread\r\n");
            }
        }

        if (SOPC_STATUS_OK != resultSOPC)
        {
            SOPC_PubScheduler_Context_Clear();
        }
    }

    if (SOPC_STATUS_OK != resultSOPC)
    {
        SOPC_Atomic_Int_Set(&pubSchedulerCtx.isStarted, false);
    }
    else
    {
        SOPC_Atomic_Int_Set(&pubSchedulerCtx.isStarted, true);
    }

    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, false);
    return (SOPC_STATUS_OK == resultSOPC);
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
    SOPC_PubScheduler_Context_Clear();

    SOPC_Atomic_Int_Set(&pubSchedulerCtx.isStarted, false);
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, false);
    printf("# Scheduler stopped ...\r\n");
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
    case SOPC_PubSubProtocol_MQTT:
    {
        {
            size_t hostnameLength = 0;
            size_t portIdx = 0;
            size_t portLength = 0;
            if (SOPC_Helper_URI_ParseUri_WithPrefix(MQTT_PREFIX, address, &hostnameLength, &portIdx, &portLength) ==
                false)
            {
                return false;
            }
        }
        MqttManagerHandle* handleMqttManager = SOPC_PubSub_Protocol_GetMqttManagerHandle();
        pubSchedulerCtx.transport[index].mqttHandle =
            SOPC_MQTT_TRANSPORT_SYNCH_GetHandle(handleMqttManager,             //
                                                &address[strlen(MQTT_PREFIX)], //
                                                MQTT_LIB_TOPIC_NAME,           //
                                                on_mqtt_message_received,      //
                                                NULL);                         //

        if (pubSchedulerCtx.transport[index].mqttHandle == NULL)
        {
            return false;
        }

        pubSchedulerCtx.transport[index].fctClear = SOPC_PubScheduler_CtxMqtt_Clear;
        pubSchedulerCtx.transport[index].fctSend = SOPC_PubScheduler_CtxMqtt_Send;
        pubSchedulerCtx.transport[index].multicastAddr = NULL;
        pubSchedulerCtx.transport[index].sock = -1;
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
    SOPC_ReturnStatus result = SOPC_UDP_Socket_SendTo(ctx->sock, ctx->multicastAddr, buffer);
    if (SOPC_STATUS_OK != result)
    {
        // TODO: Some verifications should maybe added...
        printf("# Error on SOPC_UDP_Socket_SendTo %s ...\n", strerror(errno));
    }
}

// Specific callback for MQTT message
static void on_mqtt_message_received(MqttTransportHandle* pCtx, /* Transport context handle */
                                     uint8_t* data,             /* Data received */
                                     uint16_t size,             /* Size of data received, in bytes. */
                                     void* pUserContext)        /* User context, used as pub sub connection */
{
    (void) pCtx;
    (void) data;
    (void) size;
    (void) pUserContext;
    return;
}

static void SOPC_PubScheduler_CtxMqtt_Clear(SOPC_PubScheduler_TransportCtx* ctx)
{
    if (ctx != NULL && ctx->mqttHandle != NULL)
    {
        SOPC_MQTT_TRANSPORT_SYNCH_ReleaseHandle(&(ctx->mqttHandle));
        ctx->mqttHandle = NULL;
    }
}

static void SOPC_PubScheduler_CtxMqtt_Send(SOPC_PubScheduler_TransportCtx* ctx, SOPC_Buffer* buffer)
{
    if (ctx != NULL && ctx->mqttHandle != NULL && buffer != NULL && buffer->data != NULL && buffer->length > 0)
    {
        SOPC_MQTT_TRANSPORT_SYNCH_SendMessage(ctx->mqttHandle, buffer->data, (uint16_t) buffer->length, 0);
    }
}
