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
#include "sopc_atomic.h"
#include "sopc_logger.h"

#include "MQTTAsync.h"

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

    char clientId[SOPC_MAX_LENGTH_INT32_TO_STRING]; /* Identifier passed to mqtt server */

    bool isSubscriber; /* True if client is subscriber */
    SubscriberContext subContext;

    int nbReconnectTries; /* Store number of connection and reconnection tries */

    void* pUser; /* User Context */
};

/* callback used by paho library */

void cb_subscribe_on_connexion_success(void* context, MQTTAsync_successData* response);
int cb_msg_arrived(void* context, char* topic, int topicLen, MQTTAsync_message* message);

/* mqtt layer helpers */
int set_subscriber_options(MqttContextClient* contextClient, uint16_t nbSubTopic, const char** subTopic);

int32_t get_unique_id(void);

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

SOPC_ReturnStatus SOPC_MQTT_Initialize_Client(MqttContextClient* contextClient,
                                              const char* uri,
                                              const char* username,
                                              const char* password,
                                              const char** subTopic,
                                              uint16_t nbSubTopic,
                                              FctMessageReceived* cbMessageReceived,
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
    memset(contextClient->clientId, 0, SOPC_MAX_LENGTH_INT32_TO_STRING);

    int32_t clientId = get_unique_id();

    int result = set_subscriber_options(contextClient, nbSubTopic, subTopic);
    if (0 != result)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        snprintf(contextClient->clientId, SOPC_MAX_LENGTH_INT32_TO_STRING - 1, "%d", clientId);
        int MQTTAsyncResult =
            MQTTAsync_create(&contextClient->client, uri, contextClient->clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
        if (MQTTAsyncResult != 0)
        {
            status = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            MQTTAsync_setCallbacks(contextClient->client, contextClient, NULL, cb_msg_arrived, cbDeliveryComplete);

            options.keepAliveInterval = MQTT_LIB_KEEPALIVE;
            options.connectTimeout = MQTT_LIB_CONNECTION_TIMEOUT;
            options.onFailure = NULL;
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
                status = SOPC_STATUS_NOK;
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
    MQTTAsync_disconnectOptions options = MQTTAsync_disconnectOptions_initializer;
    MQTTAsync_disconnect(contextClient->client, &options);
    SOPC_Free(contextClient);
}

/* Callback called on Succeed connection, if client is subscriber then subscribe to topics */
void cb_subscribe_on_connexion_success(void* context, MQTTAsync_successData* response)
{
    SOPC_UNUSED_ARG(response);
    MqttContextClient* contextClient = (MqttContextClient*) context;
    if (contextClient->isSubscriber)
    {
        MQTTAsync_subscribeMany(contextClient->client, contextClient->subContext.nbTopic,
                                contextClient->subContext.topic, contextClient->subContext.qos, NULL);
    }
}

int cb_msg_arrived(void* context, char* topic, int topicLen, MQTTAsync_message* message)
{
    SOPC_UNUSED_ARG(topic);
    SOPC_UNUSED_ARG(topicLen);
    MqttContextClient* contextClient = (MqttContextClient*) context;
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_PUBSUB, "Mqtt client %s received a message and transmit to callback",
                           contextClient->clientId);
    contextClient->subContext.cbMessageArrived((uint8_t*) message->payload, (uint16_t) message->payloadlen,
                                               contextClient->pUser);
    return true;
}

/* Set topic to subscribe if nbSubTopic superior to 0 */
int set_subscriber_options(MqttContextClient* contextClient, uint16_t nbSubTopic, const char** subTopic)
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

int32_t get_unique_id(void)
{
    static int32_t unique_client_id = 1;
    int32_t random = (int32_t) SOPC_TimeReference_GetCurrent();
    unique_client_id += random;
    return SOPC_Atomic_Int_Add(&unique_client_id, 1);
}

#else

SOPC_ReturnStatus SOPC_MQTT_Send_Message(MqttContextClient* contextClient, const char* topic, SOPC_Buffer message)
{
    SOPC_UNUSED_ARG(contextClient);
    SOPC_UNUSED_ARG(topic);
    SOPC_UNUSED_ARG(message);
    return SOPC_STATUS_NOT_SUPPORTED;
}

SOPC_ReturnStatus SOPC_MQTT_Initialize_Client(MqttContextClient* contextClient,
                                              const char* uri,
                                              const char* username,
                                              const char* password,
                                              const char** subTopic,
                                              uint16_t nbSubTopic,
                                              FctMessageReceived* cbMessageReceived,
                                              void* userContext)
{
    SOPC_UNUSED_ARG(contextClient);
    SOPC_UNUSED_ARG(uri);
    SOPC_UNUSED_ARG(username);
    SOPC_UNUSED_ARG(password);
    SOPC_UNUSED_ARG(subTopic);
    SOPC_UNUSED_ARG(nbSubTopic);
    SOPC_UNUSED_ARG(cbMessageReceived);
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
#endif
