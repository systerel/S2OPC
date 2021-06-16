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
#include "sopc_threads.h"
#include "sopc_udp_sockets.h"

/* TODO: use the correct log system */
#define log_error(...) printf(__VA_ARGS__)
#define log_warning(...) printf(__VA_ARGS__)
#define log_info(...) printf(__VA_ARGS__)
#define log_debug(...) printf(__VA_ARGS__)

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

/* TODO: this type is local to this file and its name may be shortened */
typedef struct SOPC_PubScheduler_MessageCtx
{
    SOPC_WriterGroup* group; /* TODO: There's seem to be a problem as there may be multiple DSM but only one group */
    SOPC_Dataset_NetworkMessage* message;
    SOPC_PubScheduler_TransportCtx* transport;
    SOPC_PubSub_SecurityType* security;
    SOPC_RealTime* next_timeout; /**< Next expiration absolute date */
    double publishingInterval;
    bool warned; /**< Have we warned about expired messages yet? */
} SOPC_PubScheduler_MessageCtx;

/* TODO: use SOPC_Array, which already does that, and uses size_t */
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

/* Finds the message with the smallest next_timeout */
static SOPC_PubScheduler_MessageCtx* MessageCtxArray_FindMostExpired(void);

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
    SOPC_PubScheduler_MessageCtx_Array messages;

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

// Clear pub scheduler context
static void SOPC_PubScheduler_Context_Clear(void)
{
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.quit, 1);
    SOPC_Thread_Join(pubSchedulerCtx.thPublisher);

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
                log_info("# Info: transport context destroyed for connection #%u (publisher). \n",
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
    /* TODO: have a local variable to store pubSchedulerCtx.messages.array */
    if (pubSchedulerCtx.messages.array != NULL)
    {
        /* Destroy message */
        for (uint32_t i = 0; i < pubSchedulerCtx.messages.current; i++)
        {
            /* TODO: have a local variable to store array[i] */
            printf("# Info: Network message #%u. destroyed \n", i);
            SOPC_Dataset_LL_NetworkMessage_Delete(pubSchedulerCtx.messages.array[i].message);
            pubSchedulerCtx.messages.array[i].message = NULL;
            SOPC_PubSub_Security_Clear(pubSchedulerCtx.messages.array[i].security);
            SOPC_Free(pubSchedulerCtx.messages.array[i].security);
            pubSchedulerCtx.messages.array[i].security = NULL;
            SOPC_RealTime_Delete(&pubSchedulerCtx.messages.array[i].next_timeout);
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

    SOPC_PubScheduler_MessageCtx* context = &(pubSchedulerCtx.messages.array[pubSchedulerCtx.messages.current]);

    context->transport = ctx;
    context->group = group;
    context->publishingInterval = SOPC_WriterGroup_Get_PublishingInterval(group);
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
        log_error("# Error Publisher: cannot allocate message context\n");
        return false;
    }

    /* Compute next timeout */
    SOPC_RealTime_AddDuration(context->next_timeout, context->publishingInterval);

    /* Fill in security context */
    if (SOPC_SecurityMode_Sign == smode || SOPC_SecurityMode_SignAndEncrypt == smode)
    {
        context->security->mode = SOPC_WriterGroup_Get_SecurityMode(group);
        context->security->groupKeys =
            SOPC_LocalSKS_GetSecurityKeys(SOPC_PUBSUB_SKS_DEFAULT_GROUPID, SOPC_PUBSUB_SKS_CURRENT_TOKENID);
        context->security->provider = SOPC_CryptoProvider_CreatePubSub(SOPC_PUBSUB_SECURITY_POLICY);
        if (NULL == context->security->groupKeys || NULL == context->security->provider)
        {
            log_error("# Error Publisher: cannot create security provider\n");
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
        log_debug("# Info Publisher: message created #%d\n", (int32_t) pubSchedulerCtx.messages.current);
        pubSchedulerCtx.messages.current++;
    }
    return result;
}

static SOPC_PubScheduler_MessageCtx* MessageCtxArray_FindMostExpired(void)
{
    SOPC_PubScheduler_MessageCtx_Array* messages = &pubSchedulerCtx.messages;
    SOPC_PubScheduler_MessageCtx* worse = NULL;

    assert(messages->length > 0 && messages->current == messages->length);
    for (size_t i = 0; i < messages->length; ++i)
    {
        SOPC_PubScheduler_MessageCtx* cursor = &messages->array[i];
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

static void display_sched_attr(int policy, struct sched_param* param);
static void display_sched_attr(int policy, struct sched_param* param)
{
    log_info("# Thread current sched policy=%s with priority=%d\n",
             (policy == SCHED_FIFO)
                 ? "SCHED_FIFO"
                 : (policy == SCHED_RR) ? "SCHED_RR" : (policy == SCHED_OTHER) ? "SCHED_OTHER" : "???",
             param->sched_priority);
}

static void* thread_start_publish(void* arg)
{
    (void) arg;

    log_info("# Time-sensitive publisher thread started\n");

    { /* TODO: remove me/refactor me */
        int policy = -1;
        pthread_t tid = pthread_self();
        struct sched_param sp;
        assert(pthread_getschedparam(tid, &policy, &sp) == 0);
        display_sched_attr(policy, &sp);
    }

    SOPC_RealTime* now = SOPC_RealTime_Create(NULL);
    assert(NULL != now);
    bool ok = true;

    while (!SOPC_Atomic_Int_Get(&pubSchedulerCtx.quit))
    {
        /* Wake-up: find which message(s) needs to be sent */
        ok = SOPC_RealTime_GetTime(now);
        assert(ok && "Failed GetTime");

        /* If a message needs to be sent, send it */
        SOPC_PubScheduler_MessageCtx* context = MessageCtxArray_FindMostExpired();
        if (SOPC_RealTime_IsExpired(context->next_timeout, now))
        {
            /* Steps to send a message (TODO: externalize this):
             * - retrieve the NetworkMessage
             * - for each DataSetMessage:
             *   - call GetVariables
             *   - for each DataSetField:
             *     - check type
             *     - encode value
             * Note that there is a problem because the writer group contains the number of DataSetFields,
             *  but we use this info for all the DSMs... -> TODO investigate
             */
            SOPC_Dataset_NetworkMessage* message = context->message;
            assert(NULL != message); /* TODO: ? */

            /* TODO: Problem: single DataSetWriter but maybe-multiple DataSetMessages */
            const SOPC_DataSetWriter* writer = SOPC_PubScheduler_Group_Get_Unique_Writer(context->group);

            size_t ndsm = SOPC_Dataset_LL_NetworkMessage_Nb_DataSetMsg(message);
            assert(1 == ndsm && "Unsupported multi-DSM");

            SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(message, 0);
            uint16_t nbFields = SOPC_Dataset_LL_DataSetMsg_Nb_DataSetField(dsm);

            const SOPC_PublishedDataSet* dataset = SOPC_DataSetWriter_Get_DataSet(writer);
            assert(SOPC_PublishedDataSet_Nb_FieldMetaData(dataset) == nbFields);

            /* TODO: shouldn't this be called for each DSM ? */
            SOPC_DataValue* values = SOPC_PubSourceVariable_GetVariables(pubSchedulerCtx.sourceConfig, dataset);
            assert(NULL != values);

            /* Check value-type compatibility and encode */
            /* TODO: simplify and externalize the type check */
            bool typeCheckingSuccess = true;
            for (size_t i = 0; i < nbFields && typeCheckingSuccess; ++i)
            {
                /* TODO: this function should take a size_t */
                SOPC_FieldMetaData* fieldData = SOPC_PublishedDataSet_Get_FieldMetaData_At(dataset, (uint16_t) i);
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
                        log_error("# Error: incompatible variants returned.\n");
                    }
                    typeCheckingSuccess = false;
                }

                /* TODO: avoid the creation of a Variant to delete it immediately,
                 *  or change the behavior of Set_Variant_At because it is its sole use */
                SOPC_Variant* variant = SOPC_Variant_Create();
                assert(NULL != variant);
                SOPC_Variant_Move(variant, &dv->Value);
                /* TODO: this function should take a size_t */
                SOPC_NetworkMessage_Set_Variant_At(message, 0, (uint16_t) i, variant, fieldData);
            }

            /* Always destroy the created DataValues */
            for (size_t i = 0; i < nbFields; ++i)
            {
                SOPC_DataValue_Clear(&values[i]);
            }
            SOPC_Free(values);

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

                context->transport->fctSend(context->transport, buffer);
                SOPC_Buffer_Delete(buffer);
                buffer = NULL;
            }

            /* Re-schedule this message */
            SOPC_RealTime_AddDuration(context->next_timeout, context->publishingInterval);
            if (SOPC_RealTime_IsExpired(context->next_timeout, now) && !context->warned)
            {
                /* This message next publish cycle was already expired before we encoded the previous one */
                /* TODO: find other message ID, such as the PublisherId */
                log_warning("# Warning: message with writerGroupId %" PRIu16 " could not be sent in time\n",
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

    log_info("# Time-sensitive publisher thread stopped\n");
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

            /* TODO: call with side effect in if statement */
            if (!SOPC_PubScheduler_MessageCtx_Array_Init_Next(transportCtx, group))
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
        log_info("# Scheduler already stopping or stopped\n");
        return;
    }

    log_info("# Stop Scheduler...\n");
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, true); /* TODO: ? -> remove when using an enum */
    SOPC_PubScheduler_Context_Clear();

    SOPC_Atomic_Int_Set(&pubSchedulerCtx.isStarted, false);
    SOPC_Atomic_Int_Set(&pubSchedulerCtx.processingStartStop, false);
    log_info("# Scheduler stopped\n");
}

static const SOPC_DataSetWriter* SOPC_PubScheduler_Group_Get_Unique_Writer(const SOPC_WriterGroup* group)
{
    assert(NULL != group);
    uint8_t nb = SOPC_WriterGroup_Nb_DataSetWriter(group);
    assert(1 == nb);
    return SOPC_WriterGroup_Get_DataSetWriter_At(group, 0);
}

// Get Socket and Multicast Address then save it in context
/* TODO: rename, it does more than a Get */
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
        bool allocSuccess = SOPC_PubSubHelpers_Publisher_ParseMulticastAddress(address, &out_multicastAddr);
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
        pubSchedulerCtx.transport[index].mqttHandle = SOPC_MQTT_TRANSPORT_SYNCH_GetHandle(
            handleMqttManager, &address[strlen(MQTT_PREFIX)], MQTT_LIB_TOPIC_NAME, on_mqtt_message_received, NULL);

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
        log_error("# Error on SOPC_UDP_Socket_SendTo %s ...\n", strerror(errno));
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
