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
#include "sopc_pubsub_helpers.h"
#include "sopc_raw_sockets.h"
#include "sopc_threads.h"
#include "sopc_udp_sockets.h"

/* Event handler to send message */
void SOPC_PubScheduler_EventHandler_Callback(SOPC_EventHandler* handler,
                                             int32_t event,
                                             uint32_t eltId,
                                             uintptr_t params,
                                             uintptr_t auxParam);

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
    // delete timer event at clear
    uint32_t timerId;
    // do not delete
    SOPC_WriterGroup* group;
    SOPC_Dataset_NetworkMessage* message;

    SOPC_PubScheduler_TransportCtx* transport;

} SOPC_PubScheduler_MessageCtx;

typedef struct SOPC_PubScheduler_MessageCtx_Array
{
    uint64_t length;
    uint64_t current;
    SOPC_PubScheduler_MessageCtx* array;
} SOPC_PubScheduler_MessageCtx_Array;

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

} pubSchedulerCtx = {.isStarted = false,
                     .processingStartStop = false,
                     .config = NULL,
                     .sourceConfig = NULL,
                     .nbConnection = 0,
                     .transport = NULL,
                     .messages.length = 0,
                     .messages.current = 0,
                     .messages.array = NULL};

static uint64_t SOPC_PubScheduler_Nb_Message(SOPC_PubSubConfiguration* config);

/*
 * MessageCtx: array of context for each message
 * Size of this array is SOPC_PubScheduler_Nb_Message
 */
static bool SOPC_PubScheduler_MessageCtx_Array_Initialize(SOPC_PubSubConfiguration* config);
static void SOPC_PubScheduler_MessageCtx_Array_Clear(void);
static bool SOPC_PubScheduler_MessageCtx_Array_Init_Next(SOPC_PubScheduler_TransportCtx* ctx, SOPC_WriterGroup* group);
// Get the last initialise MessageCtx
static SOPC_PubScheduler_MessageCtx* SOPC_PubScheduler_MessageCtx_Get_Last(void);
static void SOPC_PubScheduler_Context_Clear(void);
static const SOPC_DataSetWriter* SOPC_PubScheduler_Group_Get_Unique_Writer(const SOPC_WriterGroup* group);
static bool SOPC_PubScheduler_Connection_Get_Transport(uint32_t index,
                                                       SOPC_PubSubConnection* connection,
                                                       SOPC_PubScheduler_TransportCtx** ctx);

/**
 * - Delete timers
 * - Delete looper
 * - Delete MessageCtx
 */
static void SOPC_PubScheduler_Context_Clear(void)
{
    // delete looper => delete handler
    SOPC_Looper_Delete(pubSchedulerCtx.looper);
    /* Transport Context */
    for (uint32_t i = 0; i < pubSchedulerCtx.nbConnection; i++)
    {
        pubSchedulerCtx.transport[i].fctClear(&pubSchedulerCtx.transport[i]);
    }
    SOPC_Free(pubSchedulerCtx.transport);

    pubSchedulerCtx.nbConnection = 0;
    SOPC_PubScheduler_MessageCtx_Array_Clear();
    pubSchedulerCtx.handler = NULL;
    pubSchedulerCtx.looper = NULL;
    pubSchedulerCtx.config = NULL;
    pubSchedulerCtx.sourceConfig = NULL;
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
    for (uint64_t i = 0; i < pubSchedulerCtx.messages.current; i++)
    {
        SOPC_Dataset_LL_NetworkMessage_Delete(pubSchedulerCtx.messages.array[i].message);
        SOPC_EventTimer_Cancel(pubSchedulerCtx.messages.array[i].timerId);
    }
    pubSchedulerCtx.messages.current = 0;
    pubSchedulerCtx.messages.length = 0;
    SOPC_Free(pubSchedulerCtx.messages.array);
    pubSchedulerCtx.messages.array = NULL;
}

static bool SOPC_PubScheduler_MessageCtx_Array_Init_Next(SOPC_PubScheduler_TransportCtx* ctx, SOPC_WriterGroup* group)
{
    assert(pubSchedulerCtx.messages.current < pubSchedulerCtx.messages.length);

    SOPC_PubScheduler_MessageCtx* context = &(pubSchedulerCtx.messages.array[pubSchedulerCtx.messages.current]);
    pubSchedulerCtx.messages.current++;

    context->transport = ctx;
    context->group = group;
    context->message = SOPC_Create_NetworkMessage_From_WriterGroup(group);
    if (NULL == context->message)
    {
        return false;
    }
    return true;
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

bool SOPC_PubScheduler_Start(SOPC_PubSubConfiguration* config, SOPC_PubSourceVariableConfig* sourceConfig)
{
    bool allocSuccess = true;

    SOPC_PubScheduler_TransportCtx* transportCtx;

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

    // Create Context Array to keep Addresses and Sockets
    const uint32_t nbConnection = SOPC_PubSubConfiguration_Nb_PubConnection(config);
    bool result = nbConnection > 0;

    if (result)
    {
        pubSchedulerCtx.config = config;
        pubSchedulerCtx.sourceConfig = sourceConfig;

        SOPC_EventTimer_Initialize();

        allocSuccess = SOPC_PubScheduler_MessageCtx_Array_Initialize(config);

        // Create the Event Handler
        if (allocSuccess)
        {
            pubSchedulerCtx.looper = SOPC_Looper_Create("PublisherScheduler");
            allocSuccess = (NULL != pubSchedulerCtx.looper);
            if (allocSuccess)
            {
                pubSchedulerCtx.handler =
                    SOPC_EventHandler_Create(pubSchedulerCtx.looper, SOPC_PubScheduler_EventHandler_Callback);
                allocSuccess = (NULL != pubSchedulerCtx.handler);
            }
        }

        pubSchedulerCtx.nbConnection = nbConnection;
        if (allocSuccess)
        {
            pubSchedulerCtx.transport = SOPC_Calloc(nbConnection, sizeof(SOPC_PubScheduler_TransportCtx));
            allocSuccess = (NULL != pubSchedulerCtx.transport);
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
                    SOPC_Event event = {.event = 0, .eltId = 0, .params = (uintptr_t) msgctx};
                    msgctx->timerId =
                        SOPC_EventTimer_CreatePeriodic(pubSchedulerCtx.handler, event, publishingInterval);
                    allocSuccess = (0 != msgctx->timerId);
                }
            }
        }

        if (!allocSuccess)
        {
            SOPC_PubScheduler_Context_Clear();
            result = false;
        }
    }

    SOPC_Atomic_Int_Set(&pubSchedulerCtx.isStarted, result);
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, false);
    return result;
}

void SOPC_PubScheduler_Stop(void)
{
    if (false == SOPC_Atomic_Int_Get(&pubSchedulerCtx.isStarted) ||
        true == SOPC_Atomic_Int_Get(&pubSchedulerCtx.processingStartStop))
    {
        return;
    }
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, true);
    // true because isStarted is false
    assert(pubSchedulerCtx.nbConnection > 0);
    SOPC_PubScheduler_Context_Clear();

    SOPC_Atomic_Int_Set(&pubSchedulerCtx.isStarted, false);
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, false);
}

void SOPC_PubScheduler_Finalize(void)
{
    SOPC_EventTimer_Clear();
}

void SOPC_PubScheduler_EventHandler_Callback(SOPC_EventHandler* handler,
                                             int32_t event,
                                             uint32_t eltId,
                                             uintptr_t params,
                                             uintptr_t auxParam)
{
    // unused variables
    (void) handler;
    (void) event;
    (void) eltId;
    (void) auxParam;
    bool allocSuccess = true;
    bool typeCheckingSuccess = true;
    SOPC_PubScheduler_MessageCtx* ctx = (SOPC_PubScheduler_MessageCtx*) params;
    assert(NULL != ctx);
    SOPC_Dataset_NetworkMessage* message = ctx->message;

    // only one datasetmessage is managed
    assert(1 == SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(message));
    SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(message, 0);
    uint16_t nbFields = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);

    // get datavalue for the unique datasetwriter
    const SOPC_DataSetWriter* writer = SOPC_PubScheduler_Group_Get_Unique_Writer(ctx->group);
    const SOPC_PublishedDataSet* dataset = SOPC_DataSetWriter_Get_DataSet(writer);
    assert(SOPC_PublishedDataSet_Nb_FieldMetaData(dataset) == nbFields);

    // Fill datasetmessage
    SOPC_DataValue* values = SOPC_PubSourceVariable_GetVariables(pubSchedulerCtx.sourceConfig, dataset);
    if (NULL == values)
    {
        return;
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

            // We also want to consider case the value was valid but provided with Bad status, therefore we do not
            // consider isNullOrBad in the first condition
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
                // If value is Bad whereas the data value status code is not bad, it is considered as an error
                // since this is not the type expected for this value from an OpcUa server.

                if (!isCompatible)
                {
                    printf("# Error: incompatible variants between Address Space and Pub config.\n");
                }
                typeCheckingSuccess = false;
            }
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

    // Free DataValue array
    for (uint16_t dsf_index = 0; dsf_index < nbFields; dsf_index++)
    {
        SOPC_DataValue_Clear(&(values[dsf_index]));
    }
    SOPC_Free(values);

    // send message
    if (allocSuccess && typeCheckingSuccess)
    {
        SOPC_Buffer* buffer = SOPC_UADP_NetworkMessage_Encode(message);

        if (NULL != buffer)
        {
            ctx->transport->fctSend(ctx->transport, buffer);
            SOPC_Buffer_Delete(buffer);
        }
    }
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
    Socket out_sock;
    SOPC_Socket_AddressInfo* out_multicastAddr;
    bool allocSuccess = SOPC_PubSubHelpers_Publisher_ParseMulticastAddress(
        SOPC_PubSubConnection_Get_Address(connection), &out_multicastAddr);
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

static void SOPC_PubScheduler_CtxUdp_Clear(SOPC_PubScheduler_TransportCtx* ctx)
{
    SOPC_UDP_SocketAddress_Delete(&(ctx->multicastAddr));
    SOPC_UDP_Socket_Close(&(ctx->sock));
}

static void SOPC_PubScheduler_CtxUdp_Send(SOPC_PubScheduler_TransportCtx* ctx, SOPC_Buffer* buffer)
{
    SOPC_UDP_Socket_SendTo(ctx->sock, ctx->multicastAddr, buffer);
}
