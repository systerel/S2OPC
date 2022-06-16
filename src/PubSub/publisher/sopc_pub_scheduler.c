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

#include "p_time.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_crypto_provider.h"
#include "sopc_dataset_layer.h"
#include "sopc_eth_sockets.h"
#include "sopc_event_handler.h"
#include "sopc_event_timer_manager.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mqtt_transport_layer.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_constants.h"
#include "sopc_pubsub_helpers.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_pubsub_protocol.h"
#include "sopc_raw_sockets.h"
#include "sopc_threads.h"
#include "sopc_udp_sockets.h"

/* Transport context. One per connection */
typedef struct SOPC_PubScheduler_TransportCtx SOPC_PubScheduler_TransportCtx;

// Function to clear a transport context. To be implemented for each protocol
typedef void SOPC_PubScheduler_TransportCtx_Clear(SOPC_PubScheduler_TransportCtx*);

// Function to send a message. To be implemented for each protocol
typedef void SOPC_PubScheduler_TransportCtx_Send(SOPC_PubScheduler_TransportCtx*, SOPC_Buffer*);

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

// Clear a Transport Ethernet context. Implements SOPC_PubScheduler_TransportCtx_Clear
static void SOPC_PubScheduler_CtxEth_Clear(SOPC_PubScheduler_TransportCtx* ctx);

// Send an UADP NetworkMessage in an Ethernet II frame. Implements SOPC_PubScheduler_TransportCtx_Send
static void SOPC_PubScheduler_CtxEth_Send(SOPC_PubScheduler_TransportCtx* ctx, SOPC_Buffer* buffer);

/* Context of a UDP Multicast Message */
struct SOPC_PubScheduler_TransportCtx
{
    SOPC_Socket_AddressInfo* udpAddr;
    SOPC_ETH_Socket_SendAddressInfo* ethAddr;
    Socket sock;

    SOPC_PubScheduler_TransportCtx_Clear* pFctClear;
    SOPC_PubScheduler_TransportCtx_Send* pFctSend;

    // specific to SOPC_PubSubProtocol_MQTT
    MqttTransportHandle* mqttHandle;
};

typedef struct MessageCtx
{
    SOPC_WriterGroup* group; /* TODO: There's seem to be a problem as there may be multiple DSM but only one group */
    SOPC_Dataset_NetworkMessage* message;
    SOPC_PubScheduler_TransportCtx* transport;
    SOPC_PubSub_SecurityType* security;
    SOPC_RealTime* next_timeout; /**< Next expiration absolute date */
    uint64_t publishingIntervalUs;
    int32_t publishingOffsetUs; /**< Negative = not used */
    bool warned;                /**< Have we warned about expired messages yet? */
} MessageCtx;

/* TODO: use SOPC_Array, which already does that, and uses size_t */
typedef struct MessageCtx_Array
{
    uint64_t length;   // Size of this array is SOPC_PubScheduler_Nb_Message
    uint64_t current;  // Nb of messages already initialized. Monotonic.
    MessageCtx* array; // MessageCtx: array of context for each message
} MessageCtx_Array;

// Total of message
static uint64_t SOPC_PubScheduler_Nb_Message(SOPC_PubSubConfiguration* config);

// Allocation of message context array (size used returned by _Nb_Message)
static bool MessageCtx_Array_Initialize(SOPC_PubSubConfiguration* config);

// Deallocation of message context array
static void MessageCtx_Array_Clear(void);

// Traverse allocated message array to initialize transport context associated to message. Increment "current" field.
static bool MessageCtx_Array_Init_Next(SOPC_PubScheduler_TransportCtx* ctx, SOPC_WriterGroup* group);

/* Finds the message with the smallest next_timeout */
static MessageCtx* MessageCtxArray_FindMostExpired(void);

static void SOPC_PubScheduler_Context_Clear(void);

// Allocation and initialization of a transport context. ctx is the field of message context.
static bool SOPC_PubScheduler_Connection_Get_Transport(uint32_t index,
                                                       SOPC_PubSubConnection* connection,
                                                       SOPC_PubScheduler_TransportCtx** ctx);
static struct
{
    /* Managing start / stop phase */
    /* TODO: use an enum, command, sub state, pub state */
    int32_t isStarted;
    int32_t processingStartStop;
    int32_t quit;

    /* Input parameters */
    SOPC_PubSubConfiguration* config;
    SOPC_PubSourceVariableConfig* sourceConfig;

    /* Internal context */
    // Size of transport are number of connection
    uint32_t nbConnection;
    MessageCtx_Array messages;

    // Global Transport context

    // keep transport context to clear them when stop.
    // One per connection
    SOPC_PubScheduler_TransportCtx* transport;

    //  SOPC_LocalSKS_Keys *keys;
    // A strictly monotonically increasing sequence number for a SecurityTokenId and PublisherId combination.
    uint32_t sequenceNumber;

    Thread thPublisher;

} pubSchedulerCtx = {.isStarted = false,
                     .processingStartStop = false,
                     .config = NULL,
                     .sourceConfig = NULL,
                     .nbConnection = 0,
                     .transport = NULL,
                     .messages.length = 0,
                     .messages.current = 0,
                     .messages.array = NULL,
                     .sequenceNumber = 1};

/* This callback implements the main loop of the publisher which fetches data to publish, encode them and send them.
 * It computes when it should wake up and sleeps until then (if not past) */
static void* thread_start_publish(void* arg);

/**
 * Sends a publish message
 * /param context The message context to send
 */
static void MessageCtx_send_publish_message(MessageCtx* context);

// Clear pub scheduler context
static void SOPC_PubScheduler_Context_Clear(void)
{
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.quit, 1);
    SOPC_Thread_Join(pubSchedulerCtx.thPublisher);

    /* Destroy messages and messages array */
    MessageCtx_Array_Clear();

    /* Destroy transport context */
    if (pubSchedulerCtx.transport != NULL)
    {
        for (uint32_t i = 0; i < pubSchedulerCtx.nbConnection; i++)
        {
            if (pubSchedulerCtx.transport[i].pFctClear != NULL)
            {
                pubSchedulerCtx.transport[i].pFctClear(&pubSchedulerCtx.transport[i]);
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Transport context freed for connection #%u (publisher).",
                                      pubSchedulerCtx.nbConnection);
            }
        }

        /* Destroy transport contexts array */
        SOPC_Free(pubSchedulerCtx.transport);
        pubSchedulerCtx.transport = NULL;
    }

    /* Reset Publisher context */
    pubSchedulerCtx.nbConnection = 0;
    pubSchedulerCtx.config = NULL;
    pubSchedulerCtx.sourceConfig = NULL;
    // TODO SOPC_LocalSKS_Keys_Delete(pubSchedulerCtx.keys);
    /* Don't reset the sequenceNumber on Stop(). But for now, there is no other place to reset it */
    // pubSchedulerCtx.sequenceNumber = 1;
}

static bool MessageCtx_Array_Initialize(SOPC_PubSubConfiguration* config)
{
    const uint64_t length = SOPC_PubScheduler_Nb_Message(config);
    pubSchedulerCtx.messages.current = 0;
    pubSchedulerCtx.messages.array = SOPC_Calloc((size_t) length, sizeof(MessageCtx));
    if (NULL == pubSchedulerCtx.messages.array)
    {
        return false;
    }
    pubSchedulerCtx.messages.length = length;
    return true;
}

static void MessageCtx_Array_Clear(void)
{
    MessageCtx* arr = pubSchedulerCtx.messages.array;
    if (NULL != arr)
    {
        /* Destroy message */
        for (uint32_t i = 0; i < pubSchedulerCtx.messages.current; i++)
        {
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Network message #%u freed", i);
            SOPC_Dataset_LL_NetworkMessage_Delete(arr[i].message);
            arr[i].message = NULL;
            SOPC_PubSub_Security_Clear(arr[i].security);
            SOPC_Free(arr[i].security);
            arr[i].security = NULL;
            SOPC_RealTime_Delete(&arr[i].next_timeout);
        }

        /* Destroy messages array */
        SOPC_Free(arr);
    }
    pubSchedulerCtx.messages.array = NULL;
    pubSchedulerCtx.messages.current = 0;
    pubSchedulerCtx.messages.length = 0;
}

static bool MessageCtx_Array_Init_Next(SOPC_PubScheduler_TransportCtx* ctx, SOPC_WriterGroup* group)
{
    assert(ctx != NULL);
    assert(pubSchedulerCtx.messages.current < pubSchedulerCtx.messages.length);

    MessageCtx* context = &(pubSchedulerCtx.messages.array[pubSchedulerCtx.messages.current]);

    context->transport = ctx;
    context->group = group;
    context->publishingIntervalUs = (uint64_t)(SOPC_WriterGroup_Get_PublishingInterval(group) * 1000);
    context->publishingOffsetUs = (int32_t)(SOPC_WriterGroup_Get_PublishingOffset(group) * 1000);
    SOPC_SecurityMode_Type smode = SOPC_WriterGroup_Get_SecurityMode(group);
    context->warned = false;

    context->message = SOPC_Create_NetworkMessage_From_WriterGroup(group);
    context->next_timeout = SOPC_RealTime_Create(NULL);
    bool result = true;
    if (SOPC_SecurityMode_Sign == smode || SOPC_SecurityMode_SignAndEncrypt == smode)
    {
        context->security = SOPC_Calloc(1, sizeof(SOPC_PubSub_SecurityType));
        result = (NULL != context->security);
    }

    if (NULL == context->message || NULL == context->next_timeout || !result)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Publisher: cannot allocate message context");
        return false;
    }

    /* Compute next timeout.  */
    if (context->publishingOffsetUs >= 0)
    {
        // If publishing offset is not zero, then the publishing period shall be a divisor of 1s (Otherwise,
        // there will be no way to find a common reference)
        SOPC_ASSERT(context->publishingIntervalUs > 0);

        if (((1000 * 1000) % context->publishingIntervalUs) != 0)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                                   "Publisher: When using PublishingOffset, interval must be a divider of 1000");
            return false;
        }
        else if ((uint32_t) context->publishingOffsetUs >= context->publishingIntervalUs)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                                   "Publisher: PublishingOffset cannot be greater than PublishingInterval");
            return false;
        }
    }
    SOPC_RealTime_AddSynchedDuration(context->next_timeout, context->publishingIntervalUs, context->publishingOffsetUs);

    /* Fill in security context */
    if (SOPC_SecurityMode_Sign == smode || SOPC_SecurityMode_SignAndEncrypt == smode)
    {
        context->security->mode = SOPC_WriterGroup_Get_SecurityMode(group);
        context->security->groupKeys =
            SOPC_LocalSKS_GetSecurityKeys(SOPC_PUBSUB_SKS_DEFAULT_GROUPID, SOPC_PUBSUB_SKS_CURRENT_TOKENID);
        context->security->provider = SOPC_CryptoProvider_CreatePubSub(SOPC_PUBSUB_SECURITY_POLICY);
        if (NULL == context->security->groupKeys || NULL == context->security->provider)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Publisher: cannot create security provider");
            result = false; /* TODO: it should be possible to avoid this variable and the partial frees when false */
        }
    }

    if (!result)
    {
        SOPC_Dataset_LL_NetworkMessage_Delete(context->message);
        context->message = NULL;
        SOPC_RealTime_Delete(&context->next_timeout);
        SOPC_PubSub_Security_Clear(context->security);
        SOPC_Free(context->security);
        context->security = NULL;
    }
    else
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_PUBSUB, "Publisher: message created #%d",
                               (int32_t) pubSchedulerCtx.messages.current);
        pubSchedulerCtx.messages.current++;
    }
    return result;
}

static MessageCtx* MessageCtxArray_FindMostExpired(void)
{
    MessageCtx_Array* messages = &pubSchedulerCtx.messages;
    MessageCtx* worse = NULL;

    assert(messages->length > 0 && messages->current == messages->length);
    for (size_t i = 0; i < messages->length; ++i)
    {
        MessageCtx* cursor = &messages->array[i];
        if (NULL == worse || SOPC_RealTime_IsExpired(cursor->next_timeout, worse->next_timeout))
        {
            worse = cursor;
        }
    }

    return worse;
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

static void MessageCtx_send_publish_message(MessageCtx* context)
{
    /* Steps to send a message
     * - retrieve the NetworkMessage
     * - for each DataSetMessage:
     *   - call GetVariables
     *   - for each DataSetField:
     *     - check type
     *     - encode value
     * Note that there is a problem because the writer group contains the number of DataSetFields,
     *  but we use this info for all the DSMs... -> TODO investigate
     */

    assert(NULL != context);
    SOPC_Dataset_NetworkMessage* message = context->message;
    SOPC_WriterGroup* group = context->group;
    assert(NULL != message && NULL != group);

    size_t nDsm = (size_t) SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(message);
    assert((size_t) SOPC_WriterGroup_Nb_DataSetWriter(group) == nDsm);

    bool typeCheckingSuccess = true;

    for (size_t iDsm = 0; iDsm < nDsm; iDsm++)
    {
        // Note : It has been asserted that there are as many DataSetMessages as DataSetWriters
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(message, (int) iDsm);
        const SOPC_DataSetWriter* writer = SOPC_WriterGroup_Get_DataSetWriter_At(group, (uint8_t) iDsm);

        uint16_t nbFields = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
        const SOPC_PublishedDataSet* dataset = SOPC_DataSetWriter_Get_DataSet(writer);
        assert(SOPC_PublishedDataSet_Nb_FieldMetaData(dataset) == nbFields);

        SOPC_DataValue* values = SOPC_PubSourceVariable_GetVariables(pubSchedulerCtx.sourceConfig, dataset);
        assert(NULL != values);

        /* Check value-type compatibility and encode */
        /* TODO: simplify and externalize the type check */
        for (size_t iField = 0; iField < nbFields && typeCheckingSuccess; ++iField)
        {
            /* TODO: this function should take a size_t */
            SOPC_FieldMetaData* fieldData = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, (uint16_t) iField);
            assert(NULL != fieldData);
            SOPC_DataValue* dv = &values[iField];

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
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_PUBSUB,
                        "GetSourceVariables returned values incompatible with the current PubSub configuration");
                }
                typeCheckingSuccess = false;
            }

            /* TODO: avoid the creation of a Variant to delete it immediately,
             *  or change the behavior of Set_Variant_At because it is its sole use */
            SOPC_Variant* variant = SOPC_Variant_Create();
            assert(NULL != variant);
            SOPC_Variant_Move(variant, &dv->Value);
            /* TODO: this function should take a size_t */
            SOPC_NetworkMessage_Set_Variant_At(message, (uint8_t) iDsm, (uint16_t) iField, variant, fieldData);
        }

        /* Always destroy the created DataValues */
        for (size_t iField = 0; iField < nbFields; ++iField)
        {
            SOPC_DataValue_Clear(&values[iField]);
        }
        SOPC_Free(values);
    }

    /* Finally send it */
    if (typeCheckingSuccess)
    {
        SOPC_PubSub_SecurityType* security = context->security;
        if (NULL != security)
        {
            security->msgNonceRandom = SOPC_PubSub_Security_Random(security->provider);
            assert(NULL != security->msgNonceRandom); /* TODO: Fail sooner, don't call GetVariables */
            security->sequenceNumber = pubSchedulerCtx.sequenceNumber;
            pubSchedulerCtx.sequenceNumber++;
        }
        SOPC_Buffer* buffer = SOPC_UADP_NetworkMessage_Encode(message, security);
        if (NULL != security)
        {
            SOPC_Free(security->msgNonceRandom);
            security->msgNonceRandom = NULL;
        }

        context->transport->pFctSend(context->transport, buffer);
        SOPC_Buffer_Delete(buffer);
        buffer = NULL;
    }
}

static void* thread_start_publish(void* arg)
{
    SOPC_UNUSED_ARG(arg);

    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Time-sensitive publisher thread started");

    SOPC_RealTime* now = SOPC_RealTime_Create(NULL);
    assert(NULL != now);
    bool ok = true;

    while (!SOPC_Atomic_Int_Get(&pubSchedulerCtx.quit))
    {
        /* Wake-up: find which message(s) needs to be sent */
        ok = SOPC_RealTime_GetTime(now);
        assert(ok && "Failed GetTime");

        /* If a message needs to be sent, send it */
        MessageCtx* context = MessageCtxArray_FindMostExpired();
        if (SOPC_RealTime_IsExpired(context->next_timeout, now))
        {
            MessageCtx_send_publish_message(context);

            /* Re-schedule this message */
            SOPC_RealTime_AddSynchedDuration(context->next_timeout, context->publishingIntervalUs,
                                             context->publishingOffsetUs);
            if (SOPC_RealTime_IsExpired(context->next_timeout, now) && !context->warned)
            {
                /* This message next publish cycle was already expired before we encoded the previous one */
                /* TODO: find other message ID, such as the PublisherId */
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB,
                                         "Publisher could not send message with writerGroupId %" PRIu16
                                         " in time (warned once only)",
                                         SOPC_WriterGroup_Get_Id(context->group));
                context->warned = true; /* Avoid being spammed @ 10kHz and being even slower because of this */
            }
        }

        /* Otherwise sleep until there is a message to send */
        else
        {
            ok = SOPC_RealTime_SleepUntil(context->next_timeout) == 0;
            assert(ok && "Failed NanoSleep");
        }
    }

    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Time-sensitive publisher thread stopped");
    SOPC_RealTime_Delete(&now);

    return NULL;
}

bool SOPC_PubScheduler_Start(SOPC_PubSubConfiguration* config,
                             SOPC_PubSourceVariableConfig* sourceConfig,
                             int threadPriority)
{
    SOPC_ReturnStatus resultSOPC = SOPC_STATUS_OK;
    SOPC_PubScheduler_TransportCtx* transportCtx = NULL;

    SOPC_Helper_EndiannessCfg_Initialize(); // TODO: centralize / avoid recompute in S2OPC !

    if (NULL == config || NULL == sourceConfig || threadPriority < 0 || threadPriority > 99)
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

    /* --- Create configurations from configurations --- */

    // Create Context Array to keep Addresses and Sockets
    const uint32_t nbConnection = SOPC_PubSubConfiguration_Nb_PubConnection(config);
    resultSOPC = nbConnection > 0 ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    if (SOPC_STATUS_OK == resultSOPC)
    {
        pubSchedulerCtx.config = config;
        pubSchedulerCtx.sourceConfig = sourceConfig;
        if (!MessageCtx_Array_Initialize(config))
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

    for (uint32_t i = 0; SOPC_STATUS_OK == resultSOPC && i < nbConnection; i++)
    {
        SOPC_PubSubConnection* connection = SOPC_PubSubConfiguration_Get_PubConnection_At(config, i);

        const uint16_t nbWriterGroup = SOPC_PubSubConnection_Nb_WriterGroup(connection);
        if (!SOPC_PubScheduler_Connection_Get_Transport(i, connection, &transportCtx))
        {
            resultSOPC = SOPC_STATUS_NOK;
        }
        for (uint16_t j = 0; SOPC_STATUS_OK == resultSOPC && j < nbWriterGroup; j++)
        {
            SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, j);

            if (!MessageCtx_Array_Init_Next(transportCtx, group))
            {
                resultSOPC = SOPC_STATUS_NOK;
            }
        }
    }

    /* Creation of the thread (time-sensitive or not) */
    if (SOPC_STATUS_OK == resultSOPC)
    {
        SOPC_Atomic_Int_Set(&pubSchedulerCtx.quit, false);
        if (0 == threadPriority)
        {
            resultSOPC = SOPC_Thread_Create(&pubSchedulerCtx.thPublisher, &thread_start_publish, NULL, "Publisher");
        }
        else
        {
            resultSOPC = SOPC_Thread_CreatePrioritized(&pubSchedulerCtx.thPublisher, &thread_start_publish, NULL,
                                                       threadPriority, "Publisher");
        }
    }

    if (SOPC_STATUS_OK != resultSOPC)
    {
        SOPC_PubScheduler_Context_Clear();
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
    if (false == SOPC_Atomic_Int_Get(&pubSchedulerCtx.isStarted) ||
        true == SOPC_Atomic_Int_Get(&pubSchedulerCtx.processingStartStop))
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Pub Scheduler already stopping or stopped");
        return;
    }

    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Stopping Pub Scheduler...");
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, true); /* TODO: ? -> remove when using an enum */
    SOPC_PubScheduler_Context_Clear();

    SOPC_Atomic_Int_Set(&pubSchedulerCtx.isStarted, false);
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, false);
    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Pub Scheduler stopped");
}

// Get Socket and Multicast Address then save it in context
static bool SOPC_PubScheduler_Connection_Get_Transport(uint32_t index,
                                                       SOPC_PubSubConnection* connection,
                                                       SOPC_PubScheduler_TransportCtx** ctx)
{
    const char* address = SOPC_PubSubConnection_Get_Address(connection);
    SOPC_PubSubProtocol_Type protocol = SOPC_PubSub_Protocol_From_URI(address);
    Socket outSock;
    SOPC_Socket_AddressInfo* outUDPaddr = NULL;
    bool allocSuccess = false;
    size_t hostnameLength = 0;
    size_t portIdx = 0;
    size_t portLength = 0;
    MqttManagerHandle* handleMqttManager = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    switch (protocol)
    {
    case SOPC_PubSubProtocol_UDP:
    {
        allocSuccess = SOPC_PubSubHelpers_Publisher_ParseMulticastAddressUDP(address, &outUDPaddr);
        if (!allocSuccess)
        {
            return false;
        }
        pubSchedulerCtx.transport[index].udpAddr = outUDPaddr;
        allocSuccess =
            (SOPC_STATUS_OK == SOPC_UDP_Socket_CreateToSend(
                                   outUDPaddr, SOPC_PubSubConnection_Get_InterfaceName(connection), true, &outSock));
        if (!allocSuccess)
        {
            *ctx = NULL;
            return false;
        }
        pubSchedulerCtx.transport[index].sock = outSock;
        pubSchedulerCtx.transport[index].pFctClear = &SOPC_PubScheduler_CtxUdp_Clear;
        pubSchedulerCtx.transport[index].pFctSend = &SOPC_PubScheduler_CtxUdp_Send;
        *ctx = &pubSchedulerCtx.transport[index];
        return true;
    }
    break;
    case SOPC_PubSubProtocol_MQTT:
    {
        {
            if (SOPC_Helper_URI_ParseUri_WithPrefix(MQTT_PREFIX, address, &hostnameLength, &portIdx, &portLength) ==
                false)
            {
                return false;
            }
        }
        handleMqttManager = SOPC_PubSub_Protocol_GetMqttManagerHandle();
        pubSchedulerCtx.transport[index].mqttHandle = SOPC_MQTT_TRANSPORT_SYNCH_GetHandle(
            handleMqttManager, &address[strlen(MQTT_PREFIX)], MQTT_LIB_TOPIC_NAME, on_mqtt_message_received, NULL);

        if (pubSchedulerCtx.transport[index].mqttHandle == NULL)
        {
            return false;
        }

        pubSchedulerCtx.transport[index].pFctClear = SOPC_PubScheduler_CtxMqtt_Clear;
        pubSchedulerCtx.transport[index].pFctSend = SOPC_PubScheduler_CtxMqtt_Send;
        pubSchedulerCtx.transport[index].udpAddr = NULL;
        pubSchedulerCtx.transport[index].sock = -1;
        *ctx = &pubSchedulerCtx.transport[index];
        return true;
    }
    break;
    case SOPC_PubSubProtocol_ETH:
    {
        status = SOPC_ETH_Socket_CreateSendAddressInfo(SOPC_PubSubConnection_Get_InterfaceName(connection),
                                                       address + strlen(ETH_PREFIX),
                                                       &pubSchedulerCtx.transport[index].ethAddr);

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ETH_Socket_CreateToSend(pubSchedulerCtx.transport[index].ethAddr, true, &outSock);
        }

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                                   "error configuring the Publisher Ethernet socket: check it is run in SUDO mode, "
                                   "check interfaceName is defined (mandatory), "
                                   "check address format is only with hyphens and zero-terminated.");
            SOPC_PubScheduler_CtxEth_Clear(&pubSchedulerCtx.transport[index]);
            return false;
        }

        pubSchedulerCtx.transport[index].sock = outSock;
        pubSchedulerCtx.transport[index].pFctClear = &SOPC_PubScheduler_CtxEth_Clear;
        pubSchedulerCtx.transport[index].pFctSend = &SOPC_PubScheduler_CtxEth_Send;
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
    SOPC_UDP_SocketAddress_Delete(&(ctx->udpAddr));
    SOPC_UDP_Socket_Close(&(ctx->sock));
}

static void SOPC_PubScheduler_CtxUdp_Send(SOPC_PubScheduler_TransportCtx* ctx, SOPC_Buffer* buffer)
{
    SOPC_ReturnStatus result = SOPC_UDP_Socket_SendTo(ctx->sock, ctx->udpAddr, buffer);
    if (SOPC_STATUS_OK != result)
    {
        // TODO: Some verifications should maybe added...
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "SOPC_UDP_Socket_SendTo error %s ...", strerror(errno));
    }
}

// Specific callback for MQTT message
static void on_mqtt_message_received(MqttTransportHandle* pCtx, /* Transport context handle */
                                     uint8_t* data,             /* Data received */
                                     uint16_t size,             /* Size of data received, in bytes. */
                                     void* pUserContext)        /* User context, used as pub sub connection */
{
    SOPC_UNUSED_ARG(pCtx);
    SOPC_UNUSED_ARG(data);
    SOPC_UNUSED_ARG(size);
    SOPC_UNUSED_ARG(pUserContext);
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

static void SOPC_PubScheduler_CtxEth_Clear(SOPC_PubScheduler_TransportCtx* ctx)
{
    SOPC_ETH_Socket_Close(&(ctx->sock));
    SOPC_Free(ctx->ethAddr);
    ctx->ethAddr = NULL;
}

static void SOPC_PubScheduler_CtxEth_Send(SOPC_PubScheduler_TransportCtx* ctx, SOPC_Buffer* buffer)
{
    SOPC_ReturnStatus result = SOPC_ETH_Socket_SendTo(ctx->sock, ctx->ethAddr, ETH_ETHERTYPE, buffer);
    if (SOPC_STATUS_OK != result)
    {
        // TODO: Some verifications should maybe added...
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "SOPC_ETH_Socket_SendTo error %s ...", strerror(errno));
    }
}
