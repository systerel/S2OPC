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

#include "sopc_mqtt_transport_layer.h"
#include "sopc_macros.h"

#ifndef USE_MQTT_PAHO
#define USE_MQTT_PAHO 0
#endif
#if USE_MQTT_PAHO == 1
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_logger.h"
#include "sopc_pubsub_conf.h"

#include "MQTTAsync.h"

#define POLLING_CYCLE_MS 100

typedef struct MQTT_SUBSCRIBER_CONTEXT
{
    char* topic[MQTT_LIB_MAX_NB_TOPIC_NAME]; /* Array of topic to subscribe */
    uint16_t nbTopic;                        /* Number of topic to subscribe */
    int qos[MQTT_LIB_MAX_NB_TOPIC_NAME];     /* Quality of service */

    FctMessageReceived* cbMessageArrived; /* Callback called when message arrived */
} SubscriberContext;

struct MQTT_CONTEXT_CLIENT
{
    MQTTAsync client; /* mqtt client */

    char clientId[SOPC_MAX_LENGTH_UINT64_TO_STRING]; /* Identifier passed to mqtt server */

    bool isSubscriber; /* True if client is subscriber */
    SubscriberContext subContext;

    int nbReconnectTries; /* Store number of connection and reconnection tries */
    MqttClientState* clientState;
    SOPC_PubSub_OnFatalError* cbFatalError;
    void* pUser; /* User Context */
};

/* callback used by paho library */

static void cb_subscribe_on_connexion_success(void* context, MQTTAsync_successData* response);
static void cb_on_connexion_failed(void* context, MQTTAsync_failureData* response);
static void cb_connexion_lost(void* context, char* cause);
static void cb_disconnect_success(void* context, MQTTAsync_successData* response);
static void cb_disconnect_failed(void* context, MQTTAsync_failureData* response);
static int cb_msg_arrived(void* context, char* topic, int topicLen, MQTTAsync_message* message);

/* mqtt layer helpers */
static int set_subscriber_options(MqttContextClient* contextClient, uint16_t nbSubTopic, const char** subTopic);

static uint64_t get_unique_client_id(void);

/************************************************************************/
/*                          Public API                                  */
/************************************************************************/

SOPC_ReturnStatus SOPC_MQTT_Send_Message(MqttContextClient* contextClient, const char* topic, SOPC_Buffer message)
{
    MQTTAsync_message mqttMessage = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions options = MQTTAsync_responseOptions_initializer;
    char mqttTopic[MQTT_LIB_MAX_SIZE_TOPIC_NAME] = {0};

    mqttMessage.payloadlen = (int) message.length;
    mqttMessage.payload = message.data;
    mqttMessage.qos = MQTT_LIB_QOS;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    int n = snprintf(mqttTopic, MQTT_LIB_MAX_SIZE_TOPIC_NAME, "%s", topic);
    if (n < 0 || n >= MQTT_LIB_MAX_SIZE_TOPIC_NAME)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Mqtt client %s topic %s exceed max len topic size %d",
                               contextClient->clientId, topic, MQTT_LIB_MAX_SIZE_TOPIC_NAME);
        status = SOPC_STATUS_NOK;
    }
    else
    {
        if (MQTTAsync_isConnected(contextClient->client))
        {
            int MQTTAsyncResult = MQTTAsync_sendMessage(contextClient->client, mqttTopic, &mqttMessage, &options);

            if (0 != MQTTAsyncResult)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                                       "Mqtt client %s Failed to send message, Paho MQTT library error code %d",
                                       contextClient->clientId, MQTTAsyncResult);
                status = SOPC_STATUS_NOK;
            }
            else
            {
                SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_PUBSUB, "Mqtt client %s send succeed topic=%s",
                                       contextClient->clientId, topic);
            }
        }
        else
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB, "Mqtt client %s Message not send, client not connected",
                                     contextClient->clientId);
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_MQTT_InitializeAndConnect_Client(MqttContextClient* contextClient,
                                                        const char* uri,
                                                        const char* username,
                                                        const char* password,
                                                        const char** subTopic,
                                                        uint16_t nbSubTopic,
                                                        FctMessageReceived* cbMessageReceived,
                                                        SOPC_PubSub_OnFatalError* cbFatalError,
                                                        void* userContext)
{
    MQTTAsync_deliveryComplete* cbDeliveryComplete = NULL;
    MQTTAsync_connectOptions options = MQTTAsync_connectOptions_initializer;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    contextClient->client = NULL;
    contextClient->isSubscriber = false;
    contextClient->nbReconnectTries = 0;
    contextClient->subContext.cbMessageArrived = cbMessageReceived;
    contextClient->subContext.nbTopic = nbSubTopic;
    contextClient->pUser = userContext;
    contextClient->clientState = SOPC_Calloc(1, sizeof(*contextClient->clientState));
    contextClient->cbFatalError = cbFatalError;
    SOPC_ASSERT(NULL != contextClient->clientState);
    *contextClient->clientState = SOPC_MQTT_CLIENT_UNITIALIZED;
    memset(contextClient->clientId, 0, SOPC_MAX_LENGTH_UINT64_TO_STRING);

    uint64_t clientId = get_unique_client_id();

    int result = set_subscriber_options(contextClient, nbSubTopic, subTopic);
    if (0 != result)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        snprintf(contextClient->clientId, SOPC_MAX_LENGTH_UINT64_TO_STRING - 1, "%" PRIu64, clientId);
        int MQTTAsyncResult =
            MQTTAsync_create(&contextClient->client, uri, contextClient->clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
        if (MQTTAsyncResult != 0)
        {
            status = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            *contextClient->clientState = SOPC_MQTT_CLIENT_INITIALIZED;
            MQTTAsync_setCallbacks(contextClient->client, contextClient, cb_connexion_lost, cb_msg_arrived,
                                   cbDeliveryComplete);
            options.keepAliveInterval = MQTT_LIB_KEEPALIVE;
            options.connectTimeout = MQTT_LIB_CONNECTION_TIMEOUT;
            options.onFailure = cb_on_connexion_failed;
            options.onSuccess = cb_subscribe_on_connexion_success;
            options.context = contextClient;
            options.automaticReconnect = true;

            if (NULL != username && NULL != password)
            {
                options.username = username;
                options.password = password;
            }
            MQTTAsyncResult = MQTTAsync_connect(contextClient->client, &options);
            if (0 != MQTTAsyncResult)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Client connect request wasn't accepted");
                status = SOPC_STATUS_NOK;
                *contextClient->clientState = SOPC_MQTT_CLIENT_FAILED_CONNECT;
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_MQTT_Create_Client(MqttContextClient** contextClient)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    *contextClient = SOPC_Calloc(1, sizeof(MqttContextClient));

    if (NULL == *contextClient)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    return status;
}

void SOPC_MQTT_Release_Client(MqttContextClient* contextClient)
{
    SOPC_ASSERT(NULL != contextClient);
    if (NULL != contextClient->clientState)
    {
        MQTTAsync_disconnectOptions options = MQTTAsync_disconnectOptions_initializer;
        options.onSuccess = cb_disconnect_success;
        options.onFailure = cb_disconnect_failed;
        options.context = contextClient;
        int resDisconnect = 0;
        switch (*contextClient->clientState)
        {
        case SOPC_MQTT_CLIENT_UNITIALIZED:
        case SOPC_MQTT_CLIENT_INITIALIZED:
        case SOPC_MQTT_CLIENT_DISCONNECTED:
        case SOPC_MQTT_CLIENT_LOST_CONNECTION:
        case SOPC_MQTT_CLIENT_FAILED_CONNECT:
            break;
        case SOPC_MQTT_CLIENT_CONNECTED:
            resDisconnect = MQTTAsync_disconnect(contextClient->client, &options);
            if (MQTTASYNC_SUCCESS != resDisconnect)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Mqtt client %s failed to request disconnection",
                                       contextClient->clientId);
            }
            else
            {
                while (SOPC_MQTT_CLIENT_DISCONNECTED != *contextClient->clientState)
                {
                    SOPC_Sleep(POLLING_CYCLE_MS);
                }
            }
            break;
        default:
            // should'nt come here
            SOPC_ASSERT(false);
            break;
        }
        MQTTAsync_destroy(&contextClient->client);
        contextClient->client = NULL;
        SOPC_Free(contextClient->clientState);
        contextClient->clientState = NULL;
    }
    SOPC_Free(contextClient);
}

bool SOPC_MQTT_Client_Is_Connected(MqttContextClient* contextClient)
{
    if (NULL == contextClient)
    {
        return false;
    }
    return MQTTAsync_isConnected(contextClient->client);
}

MqttClientState SOPC_MQTT_Client_Get_State(MqttContextClient* contextClient)
{
    if (NULL == contextClient || NULL == contextClient->clientState)
    {
        return SOPC_MQTT_CLIENT_UNITIALIZED;
    }
    return *contextClient->clientState;
}

/************************************************************************/
/*                          Internal functions                          */
/************************************************************************/

/* Callback called on Succeed connection, if client is subscriber then subscribe to topics */
static void cb_subscribe_on_connexion_success(void* context, MQTTAsync_successData* response)
{
    SOPC_UNUSED_ARG(response);
    SOPC_ASSERT(NULL != context);

    MqttContextClient* contextClient = (MqttContextClient*) context;
    if (contextClient->isSubscriber)
    {
        MQTTAsync_subscribeMany(contextClient->client, contextClient->subContext.nbTopic,
                                contextClient->subContext.topic, contextClient->subContext.qos, NULL);
    }
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_PUBSUB, "Mqtt client %s successfully connected to the server",
                           contextClient->clientId);
    *contextClient->clientState = SOPC_MQTT_CLIENT_CONNECTED;
}

static int cb_msg_arrived(void* context, char* topic, int topicLen, MQTTAsync_message* message)
{
    SOPC_UNUSED_ARG(topic);
    SOPC_UNUSED_ARG(topicLen);
    SOPC_ASSERT(NULL != context);

    MqttContextClient* contextClient = (MqttContextClient*) context;
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_PUBSUB, "Mqtt client %s received a message and transmit to callback",
                           contextClient->clientId);
    contextClient->subContext.cbMessageArrived((uint8_t*) message->payload, (uint16_t) message->payloadlen,
                                               contextClient->pUser);
    return true;
}

static void cb_on_connexion_failed(void* context, MQTTAsync_failureData* response)
{
    SOPC_ASSERT(NULL != context);
    MqttContextClient* contextClient = (MqttContextClient*) context;
    SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Mqtt client %s connection failed, error code %d",
                           contextClient->clientId, response->code);
    *contextClient->clientState = SOPC_MQTT_CLIENT_FAILED_CONNECT;

    // If the connection failed due to a Broker restart, there are high chances
    // that "reconnect" will never succeed and get stuck. Notify the application.
    SOPC_PubSubConnection* connection = (SOPC_PubSubConnection*)contextClient->pUser;
    if (NULL != connection)
    {
        const char* name = SOPC_PubSubConnection_Get_Name(connection);
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_PUBSUB, "MQTT connection '%s' got disconnected.",
                name ? name : "<NULL>");
        SOPC_PubSub_OnFatalError* callback = SOPC_PubSubConfiguration_Get_FatalError_Callback(connection);
        if (NULL != callback)
        {
            (*callback)(connection, "MQTT_CLIENT_FAILED_CONNECT");
        }
    }
}

static void cb_connexion_lost(void* context, char* cause)
{
    SOPC_ASSERT(NULL != context);
    MqttContextClient* contextClient = (MqttContextClient*) context;
    SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Mqtt client %s connection lost, cause %s", contextClient->clientId,
                           cause);
    *contextClient->clientState = SOPC_MQTT_CLIENT_LOST_CONNECTION;
}

static void cb_disconnect_failed(void* context, MQTTAsync_failureData* response)
{
    SOPC_ASSERT(NULL != context);
    MqttContextClient* contextClient = (MqttContextClient*) context;
    SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB,
                           "Mqtt client %s failed to disconnect, failure code %d. consider client hase disconnected "
                           "anyway and release memory",
                           contextClient->clientId, response->code);
    *contextClient->clientState = SOPC_MQTT_CLIENT_DISCONNECTED;
}
static void cb_disconnect_success(void* context, MQTTAsync_successData* response)
{
    SOPC_UNUSED_ARG(response);
    MqttContextClient* contextClient = (MqttContextClient*) context;
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_PUBSUB, "Mqtt client %s succeed to disconnect", contextClient->clientId);
    *contextClient->clientState = SOPC_MQTT_CLIENT_DISCONNECTED;
}

/* Set topic to subscribe if nbSubTopic superior to 0 */
static int set_subscriber_options(MqttContextClient* contextClient, uint16_t nbSubTopic, const char** subTopic)
{
    if (nbSubTopic > 0)
    {
        contextClient->isSubscriber = true;
        contextClient->subContext.nbTopic = nbSubTopic;
        for (int i = 0; i < nbSubTopic; i++)
        {
            if (NULL != subTopic[i])
            {
                contextClient->subContext.qos[i] = MQTT_LIB_QOS;
                SOPC_GCC_DIAGNOSTIC_IGNORE_DISCARD_QUALIFIER
                contextClient->subContext.topic[i] = subTopic[i];
                SOPC_GCC_DIAGNOSTIC_RESTORE
            }
            else
            {
                return 1;
            }
        }
    }
    return 0;
}

/* Generate a random ID to avoid conflictual client IDs connecting to Mqtt servers */
static uint64_t get_unique_client_id(void)
{
    static uint64_t unique_clientId = 1;
    unique_clientId += SOPC_TimeReference_GetCurrent();
    return unique_clientId;
}

#else // USE_MQTT_PAHO == 1

SOPC_ReturnStatus SOPC_MQTT_Send_Message(MqttContextClient* contextClient, const char* topic, SOPC_Buffer message)
{
    SOPC_UNUSED_ARG(contextClient);
    SOPC_UNUSED_ARG(topic);
    SOPC_UNUSED_ARG(message);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_MQTT_InitializeAndConnect_Client(MqttContextClient* contextClient,
                                                        const char* uri,
                                                        const char* username,
                                                        const char* password,
                                                        const char** subTopic,
                                                        uint16_t nbSubTopic,
                                                        FctMessageReceived* cbMessageReceived,
                                                        SOPC_PubSub_OnFatalError* cbFatalError,
                                                        void* userContext)
{
    SOPC_UNUSED_ARG(contextClient);
    SOPC_UNUSED_ARG(uri);
    SOPC_UNUSED_ARG(username);
    SOPC_UNUSED_ARG(password);
    SOPC_UNUSED_ARG(subTopic);
    SOPC_UNUSED_ARG(nbSubTopic);
    SOPC_UNUSED_ARG(cbMessageReceived);
    SOPC_UNUSED_ARG(cbFatalError);
    SOPC_UNUSED_ARG(userContext);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_MQTT_Create_Client(MqttContextClient** contextClient)
{
    SOPC_UNUSED_ARG(contextClient);
    return SOPC_STATUS_NOT_SUPPORTED;
}

void SOPC_MQTT_Release_Client(MqttContextClient* contextClient)
{
    SOPC_UNUSED_ARG(contextClient);
}

bool SOPC_MQTT_Client_Is_Connected(MqttContextClient* contextClient)
{
    SOPC_UNUSED_ARG(contextClient);
    return false;
}

#endif // USE_MQTT_PAHO == 1
