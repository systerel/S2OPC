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

#include <errno.h>
#include <inttypes.h>

#include "p_time.h"
#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_crypto_provider.h"
#include "sopc_dataset_layer.h"
#include "sopc_etf_sockets.h"
#include "sopc_eth_sockets.h"
#include "sopc_event_handler.h"
#include "sopc_event_timer_manager.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_missing_c99.h"
#include "sopc_mqtt_transport_layer.h"
#include "sopc_mutexes.h"
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

// Clear a Transport UDP context. Implements SOPC_PubScheduler_TransportCtx_Clear
static void SOPC_PubScheduler_CtxUdpEtf_Clear(SOPC_PubScheduler_TransportCtx* ctx);

// Send and UDP message in ETF mode. Implement SOPC_PubScheduler_TransportCtx_Send
static void SOPC_PubScheduler_CtxUdpEtf_Send(SOPC_PubScheduler_TransportCtx* ctx, SOPC_Buffer* buffer);

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
    MqttContextClient* mqttClient;
    const char* mqttTopic;

    /* Is publisher in acyclic mode. If yes message will not be considered when looking for most expire one */
    bool isAcyclic;

    /* Etf mode properties */
    bool isEtf;
    uint64_t deltaUs;
    uint64_t txtime;
};

typedef struct MessageCtx
{
    SOPC_WriterGroup* group; /* TODO: There's seem to be a problem as there may be multiple DSM but only one group */
    SOPC_Dataset_NetworkMessage* message;
    SOPC_Dataset_NetworkMessage* messageKeepAlive;
    uint16_t writerMessageSequence; /* TODO add a context by dataSetWriter in writer group when several possible */
    SOPC_PubScheduler_TransportCtx* transport;
    SOPC_PubSub_SecurityType* security;
    SOPC_RealTime* next_timeout; /**< Next expiration absolute date */
    uint64_t publishingIntervalUs;
    int32_t publishingOffsetUs; /**< Negative = not used */
    const char* mqttTopic;
    bool warned; /**< Have we warned about expired messages yet? */
    uint64_t keepAliveTimeUs;
    uint8_t soPriority;
    uint64_t txtime;
} MessageCtx;

/* TODO: use SOPC_Array, which already does that, and uses size_t */
typedef struct MessageCtx_Array
{
    uint64_t length;    // Size of this array is SOPC_PubScheduler_Nb_Message
    uint64_t current;   // Nb of messages already initialized. Monotonic.
    MessageCtx* array;  // MessageCtx: array of context for each message
    Mutex acyclicMutex; // Mutex used for acyclic send
} MessageCtx_Array;

// Total of message
static uint64_t SOPC_PubScheduler_Nb_Message(SOPC_PubSubConfiguration* config);

// Allocation of message context array (size used returned by _Nb_Message)
static bool MessageCtx_Array_Initialize(SOPC_PubSubConfiguration* config);

// Deallocation of message context array
static void MessageCtx_Array_Clear(void);

// Traverse allocated message array to initialize transport context associated to message. Increment "current" field.
static bool MessageCtx_Array_Init_Next(SOPC_PubScheduler_TransportCtx* ctx,
                                       SOPC_Conf_PublisherId pubId,
                                       SOPC_WriterGroup* group);

/* Finds the message with the smallest next_timeout
   return NULL if every publisher is acyclic */
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

    //  SOPC_KeyBunch_Keys *keys;
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

/**
 * @brief Send keep alive message for acyclic publisher
 *
 */
static void send_keepAlive_message(MessageCtx* context);

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
                SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB,
                                      "Transport context freed for connection #%" PRIu32 "(publisher).",
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
    // TODO SOPC_KeyBunch_Keys_Delete(pubSchedulerCtx.keys);
    /* Don't reset the sequenceNumber on Stop(). But for now, there is no other place to reset it */
    // pubSchedulerCtx.sequenceNumber = 1;
}

static bool MessageCtx_Array_Initialize(SOPC_PubSubConfiguration* config)
{
    bool result = true;
    const uint64_t length = SOPC_PubScheduler_Nb_Message(config);
    pubSchedulerCtx.messages.current = 0;
    pubSchedulerCtx.messages.array = SOPC_Calloc((size_t) length, sizeof(MessageCtx));
    if (NULL == pubSchedulerCtx.messages.array)
    {
        result = false;
    }
    if (result)
    {
        pubSchedulerCtx.messages.length = length;
        result = SOPC_STATUS_OK == Mutex_Initialization(&pubSchedulerCtx.messages.acyclicMutex);
    }
    return result;
}

static void MessageCtx_Array_Clear(void)
{
    MessageCtx* arr = pubSchedulerCtx.messages.array;
    if (NULL != arr)
    {
        /* Destroy message */
        for (uint32_t i = 0; i < pubSchedulerCtx.messages.current; i++)
        {
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Network message #%" PRIu32 " freed", i);
            SOPC_Dataset_LL_NetworkMessage_Delete(arr[i].message);
            arr[i].message = NULL;
            SOPC_Dataset_LL_NetworkMessage_Delete(arr[i].messageKeepAlive);
            arr[i].messageKeepAlive = NULL;
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
    Mutex_Clear(&pubSchedulerCtx.messages.acyclicMutex);
}

static bool MessageCtx_Array_Init_Next(SOPC_PubScheduler_TransportCtx* ctx,
                                       SOPC_Conf_PublisherId pubId,
                                       SOPC_WriterGroup* group)
{
    SOPC_ASSERT(ctx != NULL);
    SOPC_ASSERT(pubSchedulerCtx.messages.current < pubSchedulerCtx.messages.length);

    MessageCtx* context = &(pubSchedulerCtx.messages.array[pubSchedulerCtx.messages.current]);

    context->transport = ctx;
    if (NULL != ctx->mqttClient)
    {
        /* If no topic has been set by user a default one is used */
        const char* topic = SOPC_WriterGroup_Get_MqttTopic(group);
        if (NULL == topic)
        {
            SOPC_ASSERT(SOPC_UInteger_PublisherId == pubId.type);
            if (SOPC_WriterGroup_Set_Default_MqttTopic(group, pubId.data.uint, SOPC_WriterGroup_Get_Id(group)))
            {
                context->mqttTopic = SOPC_WriterGroup_Get_MqttTopic(group);
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to set default MQTT topic value");
                return false;
            }
        }
        else
        {
            context->mqttTopic = topic;
        }
    }
    else
    {
        context->mqttTopic = NULL;
    }

    context->group = group;
    SOPC_SecurityMode_Type smode = SOPC_WriterGroup_Get_SecurityMode(group);
    context->warned = false;
    context->message = SOPC_Create_NetworkMessage_From_WriterGroup(group, false);
    context->messageKeepAlive = NULL; // by default NULL and set only if publisher is acyclic
    context->keepAliveTimeUs = 0;     // by default equal to 0 and set only if publisher is acyclic
    context->writerMessageSequence = 1;
    context->next_timeout = SOPC_RealTime_Create(NULL);

    bool result = true;
    if (SOPC_SecurityMode_Sign == smode || SOPC_SecurityMode_SignAndEncrypt == smode)
    {
        context->security = SOPC_Calloc(1, sizeof(SOPC_PubSub_SecurityType));
        result = (NULL != context->security);
    }

    /* If publisher is acyclic we don't need to compute publishing interval */
    if (!ctx->isAcyclic)
    {
        context->publishingIntervalUs = (uint64_t)(SOPC_WriterGroup_Get_PublishingInterval(group) * 1000);
        context->publishingOffsetUs = (int32_t)(SOPC_WriterGroup_Get_PublishingOffset(group) * 1000);
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
        SOPC_RealTime_AddSynchedDuration(context->next_timeout, context->publishingIntervalUs,
                                         context->publishingOffsetUs);
    }
    /* We only use keep alive for acyclic publisher*/
    else
    {
        context->keepAliveTimeUs = (uint64_t)(SOPC_WriterGroup_Get_KeepAlive(group) * 1000);
        SOPC_RealTime_AddSynchedDuration(context->next_timeout, context->keepAliveTimeUs, -1);
        context->messageKeepAlive = SOPC_Create_NetworkMessage_From_WriterGroup(group, true);
    }
    if (NULL == context->message || NULL == context->next_timeout ||
        (NULL != context->messageKeepAlive && !ctx->isAcyclic) ||
        (NULL == context->messageKeepAlive && ctx->isAcyclic) || !result)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Publisher: cannot allocate message context");
        return false;
    }

    if(ctx->isEtf)
    {
        // Interface has been set before no reason to set it again
        result = (SOPC_STATUS_OK == SOPC_UDP_SO_TXTIME_Socket_Option(&ctx->sock, false, SOPC_WriterGroup_Get_SoPriority(group)));
    }

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
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_PUBSUB, "Publisher: message created #%" PRId32,
                               (int32_t) pubSchedulerCtx.messages.current);
        pubSchedulerCtx.messages.current++;
    }
    return result;
}

static MessageCtx* MessageCtxArray_FindMostExpired(void)
{
    MessageCtx_Array* messages = &pubSchedulerCtx.messages;
    MessageCtx* worse = NULL;

    SOPC_ASSERT(messages->length > 0 && messages->current == messages->length);
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

    SOPC_ASSERT(NULL != context);
    SOPC_Dataset_NetworkMessage* message = context->message;
    SOPC_WriterGroup* group = context->group;
    SOPC_ASSERT(NULL != message && NULL != group);

    size_t nDsm = (size_t) SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(message);
    SOPC_ASSERT((size_t) SOPC_WriterGroup_Nb_DataSetWriter(group) == nDsm);

    bool typeCheckingSuccess = true;

    for (size_t iDsm = 0; iDsm < nDsm; iDsm++)
    {
        // Note : It has been asserted that there are as many DataSetMessages as DataSetWriters
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(message, (int) iDsm);
        const SOPC_DataSetWriter* writer = SOPC_WriterGroup_Get_DataSetWriter_At(group, (uint8_t) iDsm);

        // TODO: manage several writers SNs (only 1 writer by group for now)
        SOPC_Dataset_LL_DataSetMsg_Set_SequenceNumber(dsm, context->writerMessageSequence);
        context->writerMessageSequence++;

        uint16_t nbFields = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);
        const SOPC_PublishedDataSet* dataset = SOPC_DataSetWriter_Get_DataSet(writer);
        SOPC_ASSERT(SOPC_PublishedDataSet_Nb_FieldMetaData(dataset) == nbFields);

        SOPC_DataValue* values = SOPC_PubSourceVariable_GetVariables(pubSchedulerCtx.sourceConfig, dataset);
        SOPC_ASSERT(NULL != values);

        /* Check value-type compatibility and encode */
        /* TODO: simplify and externalize the type check */
        for (size_t iField = 0; iField < nbFields && typeCheckingSuccess; ++iField)
        {
            /* TODO: this function should take a size_t */
            SOPC_FieldMetaData* fieldData = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, (uint16_t) iField);
            SOPC_ASSERT(NULL != fieldData);
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
            SOPC_ASSERT(NULL != variant);
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
            SOPC_ASSERT(NULL != security->msgNonceRandom); /* TODO: Fail sooner, don't call GetVariables */
            security->sequenceNumber = pubSchedulerCtx.sequenceNumber;
            pubSchedulerCtx.sequenceNumber++;
        }
        SOPC_Buffer* buffer = SOPC_UADP_NetworkMessage_Encode(message, security);
        if (NULL != security)
        {
            SOPC_Free(security->msgNonceRandom);
            security->msgNonceRandom = NULL;
        }

        context->transport->mqttTopic = context->mqttTopic;
        context->transport->txtime = context->txtime;

        context->transport->pFctSend(context->transport, buffer);
        SOPC_Buffer_Delete(buffer);
        buffer = NULL;
    }
}

static void send_keepAlive_message(MessageCtx* context)
{
    SOPC_PubSub_SecurityType* security = context->security;
    SOPC_Dataset_LL_NetworkMessage* message = context->messageKeepAlive;
    SOPC_WriterGroup* group = context->group;

    size_t nDsm = (size_t) SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(message);
    SOPC_ASSERT((size_t) SOPC_WriterGroup_Nb_DataSetWriter(group) == nDsm);

    for (size_t iDsm = 0; iDsm < nDsm; iDsm++)
    {
        SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(message, (int) iDsm);
        SOPC_Dataset_LL_DataSetMsg_Set_SequenceNumber(dsm, context->writerMessageSequence);
        context->writerMessageSequence++;
    }

    if (NULL != security)
    {
        security->msgNonceRandom = SOPC_PubSub_Security_Random(security->provider);
        SOPC_ASSERT(NULL != security->msgNonceRandom);
        security->sequenceNumber = pubSchedulerCtx.sequenceNumber;
        pubSchedulerCtx.sequenceNumber++;
    }

    SOPC_Buffer* buffer = SOPC_UADP_NetworkMessage_Encode(message, security);
    if (NULL != security)
    {
        SOPC_Free(security->msgNonceRandom);
        security->msgNonceRandom = NULL;
    }

    context->transport->mqttTopic = context->mqttTopic;

    context->transport->pFctSend(context->transport, buffer);
    SOPC_Buffer_Delete(buffer);
    buffer = NULL;
}

static void* thread_start_publish(void* arg)
{
    SOPC_UNUSED_ARG(arg);

    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Time-sensitive publisher thread started");
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_RealTime* now = SOPC_RealTime_Create(NULL);
    SOPC_RealTime* nextTimeout = SOPC_RealTime_Create(NULL);
    SOPC_ASSERT(NULL != now);
    SOPC_ASSERT(NULL != nextTimeout);

    bool ok = true;

    status = Mutex_Lock(&pubSchedulerCtx.messages.acyclicMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    while (!SOPC_Atomic_Int_Get(&pubSchedulerCtx.quit))
    {
        /* Wake-up: find which message(s) needs to be sent */
        ok = SOPC_RealTime_GetTime(now);
        SOPC_ASSERT(ok && "Failed GetTime");

        /* If a message needs to be sent, send it */
        MessageCtx* context = MessageCtxArray_FindMostExpired();
        if (SOPC_RealTime_IsExpired(context->next_timeout, now))
        {
            if (context->transport->isAcyclic)
            {
                send_keepAlive_message(context);
                /* Re-schedule keep alive message */
                SOPC_RealTime_AddSynchedDuration(context->next_timeout, context->keepAliveTimeUs, -1);
            }
            else
            {
                MessageCtx_send_publish_message(context);
                /* Re-schedule this message */
                SOPC_RealTime_AddSynchedDuration(context->next_timeout, context->publishingIntervalUs,
                                                 context->publishingOffsetUs);
            }

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
            ok = SOPC_RealTime_Copy(nextTimeout, context->next_timeout);
            SOPC_ASSERT(ok && "Failed Copy");
            status = Mutex_Unlock(&pubSchedulerCtx.messages.acyclicMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
            ok = SOPC_RealTime_SleepUntil(nextTimeout);
            SOPC_ASSERT(ok && "Failed NanoSleep");
            status = Mutex_Lock(&pubSchedulerCtx.messages.acyclicMutex);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
        }
    }
    status = Mutex_Unlock(&pubSchedulerCtx.messages.acyclicMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Time-sensitive publisher thread stopped");
    SOPC_RealTime_Delete(&now);
    SOPC_RealTime_Delete(&nextTimeout);

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
        const SOPC_Conf_PublisherId* pubId = SOPC_PubSubConnection_Get_PublisherId(connection);
        const uint16_t nbWriterGroup = SOPC_PubSubConnection_Nb_WriterGroup(connection);
        if (!SOPC_PubScheduler_Connection_Get_Transport(i, connection, &transportCtx))
        {
            resultSOPC = SOPC_STATUS_NOK;
        }
        for (uint16_t j = 0; SOPC_STATUS_OK == resultSOPC && j < nbWriterGroup; j++)
        {
            SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(connection, j);

            if (!MessageCtx_Array_Init_Next(transportCtx, *pubId, group))
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
        pubSchedulerCtx.transport[index].isEtf = SOPC_PubSubConnection_Get_EtfPublihser(connection);
        if (pubSchedulerCtx.transport[index].isEtf)
        {
            pubSchedulerCtx.transport[index].pFctClear = &SOPC_PubScheduler_CtxUdpEtf_Clear;
            pubSchedulerCtx.transport[index].pFctSend = &SOPC_PubScheduler_CtxUdpEtf_Send;
        }
        else
        {
            pubSchedulerCtx.transport[index].pFctClear = &SOPC_PubScheduler_CtxUdp_Clear;
            pubSchedulerCtx.transport[index].pFctSend = &SOPC_PubScheduler_CtxUdp_Send;
        }
        pubSchedulerCtx.transport[index].sock = outSock;
        pubSchedulerCtx.transport[index].isAcyclic = SOPC_PubSubConnection_Get_AcyclicPublisher(connection);
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
        status = SOPC_MQTT_Create_Client(&pubSchedulerCtx.transport[index].mqttClient);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Not enougth space to allocate mqttClient");
            return false;
        }
        status = SOPC_MQTT_InitializeAndConnect_Client(
            pubSchedulerCtx.transport[index].mqttClient, &address[strlen(MQTT_PREFIX)],
            SOPC_PubSubConnection_Get_MqttUsername(connection), SOPC_PubSubConnection_Get_MqttPassword(connection),
            NULL, 0, NULL, NULL);

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Publisher MQTT configuration failed");
            return false;
        }

        pubSchedulerCtx.transport[index].pFctClear = SOPC_PubScheduler_CtxMqtt_Clear;
        pubSchedulerCtx.transport[index].pFctSend = SOPC_PubScheduler_CtxMqtt_Send;
        pubSchedulerCtx.transport[index].udpAddr = NULL;
        pubSchedulerCtx.transport[index].sock = -1;
        pubSchedulerCtx.transport[index].isAcyclic = SOPC_PubSubConnection_Get_AcyclicPublisher(connection);
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
        pubSchedulerCtx.transport[index].isAcyclic = SOPC_PubSubConnection_Get_AcyclicPublisher(connection);
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

static void SOPC_PubScheduler_CtxUdpEtf_Clear(SOPC_PubScheduler_TransportCtx* ctx)
{
    SOPC_PubScheduler_CtxUdp_Clear(ctx);
}

static void SOPC_PubScheduler_CtxUdpEtf_Send(SOPC_PubScheduler_TransportCtx* ctx, SOPC_Buffer* buffer)
{
    SOPC_ReturnStatus result = SOPC_TX_UDP_send(&ctx->sock, buffer, ctx->txtime, ctx->udpAddr);
    if (SOPC_STATUS_OK != result)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "SOPC_TX_UDP_Send failed");
    }
}

static void SOPC_PubScheduler_CtxMqtt_Clear(SOPC_PubScheduler_TransportCtx* ctx)
{
    SOPC_MQTT_Release_Client(ctx->mqttClient);
}

static void SOPC_PubScheduler_CtxMqtt_Send(SOPC_PubScheduler_TransportCtx* ctx, SOPC_Buffer* buffer)
{
    if (ctx != NULL && ctx->mqttClient != NULL && buffer != NULL && buffer->data != NULL && buffer->length > 0)
    {
        SOPC_ReturnStatus result = SOPC_MQTT_Send_Message(ctx->mqttClient, ctx->mqttTopic, *buffer);
        if (SOPC_STATUS_OK != result)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Failed to send MQTT message");
        }
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

static MessageCtx* MessageCtxArray_GetFromWriterGroupId(uint16_t wgId)
{
    MessageCtx_Array* messages = &pubSchedulerCtx.messages;
    MessageCtx* found = NULL;

    SOPC_ASSERT(messages->length > 0 && messages->current == messages->length);
    for (size_t i = 0; i < messages->length; ++i)
    {
        MessageCtx* cursor = &messages->array[i];
        if (cursor->transport->isAcyclic)
        {
            if (wgId == SOPC_WriterGroup_Get_Id(cursor->group))
            {
                if (NULL == found)
                {
                    found = cursor;
                }
                else
                {
                    SOPC_ASSERT(false && "WriterGroupId shall be unique in configuration");
                }
            }
        }
    }

    return found;
}

bool SOPC_PubScheduler_AcyclicSend(uint16_t writerGroupId)
{
    bool result = true;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    MessageCtx* ctx = MessageCtxArray_GetFromWriterGroupId(writerGroupId);
    if (NULL == ctx)
    {
        return false;
    }
    status = Mutex_Lock(&pubSchedulerCtx.messages.acyclicMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    result = SOPC_RealTime_GetTime(ctx->next_timeout);
    SOPC_ASSERT(result);
    SOPC_RealTime_AddSynchedDuration(ctx->next_timeout, ctx->keepAliveTimeUs, -1);
    MessageCtx_send_publish_message(ctx);
    status = Mutex_Unlock(&pubSchedulerCtx.messages.acyclicMutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    return result;
}
