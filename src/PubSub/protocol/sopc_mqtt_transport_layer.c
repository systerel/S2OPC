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

//#include "S2OPC_TLS_Task.h" // for cert,key&ca

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

    char clientId[SOPC_MAX_LENGTH_UINT64_TO_STRING]; /* Identifier passed to mqtt server */

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

uint64_t get_unique_client_id(void);

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
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Client connect request wasn't accepted");
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

bool SOPC_MQTT_Client_Is_Connected(MqttContextClient* contextClient)
{
    if (NULL == contextClient)
    {
        return false;
    }
    if (MQTTAsync_isConnected(contextClient->client))
    {
        return true;
    }
    return false;
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

/* Generate a random ID to avoid conflictual client IDs connecting to Mqtt servers */
uint64_t get_unique_client_id(void)
{
    static uint64_t unique_clientId = 1;
    unique_clientId += SOPC_TimeReference_GetCurrent();
    return unique_clientId;
}

#else

#ifndef USE_CORE_MQTT
#define USE_CORE_MQTT 0
#endif
#if USE_CORE_MQTT == 1

#include "S2OPC_mqtt_agent_task.h"
#include "core_mqtt.h"
#include "lwip/sockets.h"
#include "cache.h"
#include "sopc_sub_scheduler.h"

struct sopc_mbedtls_mqtt
{
    mbedtls_net_context server_fd;

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_x509_crt ca;
    mbedtls_x509_crt cert_cli;
    mbedtls_pk_context pk_key;
#endif
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_config conf;
    mbedtls_ssl_context ssl;
} ssl_ctx;

static int sopc_createsock (int domain, int type, int protocol, Socket* sock)
{
	*sock = socket(domain, type, protocol);
	return 0;
}

static int sopc_connectsock (Socket sock, SOPC_Socket_AddressInfo* SA)
{
    int connectStatus = -1;
    connectStatus = connect(sock, SA->ai_addr, SA->ai_addrlen);
    return connectStatus;
}

static int32_t sopc_sendsock (NetworkContext_t * pNetworkContext, const void * pdata, size_t bytesToSend )
{
	ssize_t res = 0;
    res = send(pNetworkContext->sock, pdata, bytesToSend, 0);
    return res;
}

static int32_t sopc_recvsock (NetworkContext_t * pNetworkContext, void * pdata, size_t dataSize )
{
	ssize_t sReadCount = 0;
	sReadCount = recv(pNetworkContext->sock, pdata, dataSize, 0);
    return sReadCount;
}

static int32_t mbedtls_net_send_tls (NetworkContext_t* pSsl, const void * pdata, size_t bytesToSend )
{
    int32_t res = mbedtls_ssl_write(pSsl->xSslCtx, pdata, bytesToSend);
    return res;
}

static int32_t mbedtls_net_recv_tls (NetworkContext_t* pSsl, void * pdata, size_t bytesToSend )
{
    int32_t res = mbedtls_ssl_read((pSsl->xSslCtx), pdata, bytesToSend);
    return res;
}

static int mbedtls_ssl_send(void *pNetworkContext, const unsigned char *pdata, size_t dataSize)
{
    mbedtls_net_context* pctx;
    pctx = (mbedtls_net_context*) pNetworkContext;
    int sReadCount = send(pctx->fd, pdata, dataSize, 0);
    return sReadCount;
}

static int mbedtls_ssl_recv(void *pNetworkContext, unsigned char *pdata, size_t dataSize)
{
    mbedtls_net_context* pctx;
    pctx = (mbedtls_net_context*) pNetworkContext;
    int sReadCount = recv(pctx->fd, pdata, dataSize, 0);
    return sReadCount;
}

/* Target callback */
bool set_target_variable2(OpcUa_WriteValue* nodesToWrite, int32_t nbValues)
{
    bool ok = true;

    for (int i = 0; i < nbValues; i++)
    {
        OpcUa_WriteValue* wv = &nodesToWrite[i];
        SOPC_DataValue* dv = &wv->Value;

        /* Print Variant from nodes that have been received */
        // BEGIN
        if (SOPC_VariantArrayType_SingleValue == dv->Value.ArrayType)
        {
            switch (dv->Value.BuiltInTypeId)
            {
            case SOPC_Int16_Id:
                LogInfo("New value of dv->Value.Value.Int16 = %d\n", dv->Value.Value.Int16);
                break;
            case SOPC_UInt16_Id:
                LogInfo("New value of dv->Value.Value.Uint16 = %d\n", dv->Value.Value.Uint16);
                break;
            case SOPC_Int32_Id:
                LogInfo("New value of dv->Value.Value.Int32 = %d\n", dv->Value.Value.Int32);
                break;
            case SOPC_UInt32_Id:
                LogInfo("New value of dv->Value.Value.Uint32 = %d\n", dv->Value.Value.Uint32);
                break;
            case SOPC_Int64_Id:
                LogInfo("New value of dv->Value.Value.Int64 = %ld\n", dv->Value.Value.Int64);
                break;
            case SOPC_UInt64_Id:
                LogInfo("New value of dv->Value.Value.Uint64 = %ld\n", dv->Value.Value.Uint64);
                break;
            case SOPC_String_Id:
                /* Variant is a SOPC_String. You can add some text to Variant.Value.String */
                // BEGIN
                SOPC_Variant_Print_U5(&dv->Value);
                // END
                break;
            default:
                SOPC_Variant_Print_U5(&dv->Value);
                break;
            }
        }
        // END
        /* As we have ownership of the wv, clear it */
        OpcUa_WriteValue_Clear(wv);
    }

    SOPC_Free(nodesToWrite);

    return ok;
}

/* callback used by core_mqtt library */
static void cb_msg_arrived_core_mqtt ( struct MQTTContext * pContext,
        							struct MQTTPacketInfo * pPacketInfo,
									struct MQTTDeserializedInfo * pDeserializedInfo )
{
    SOPC_ReturnStatus result = SOPC_STATUS_NOK;
	LogInfo("MQTT CB !!");
	if (MQTT_PACKET_TYPE_PUBLISH == pPacketInfo->type || 0x32 == pPacketInfo->type || 0x34 == pPacketInfo->type) //32 -> QoS = 1 | 34 -> QoS = 2
	{
		uint16_t topicLength = pDeserializedInfo->pPublishInfo->topicNameLength;
		char pMqttTopic[topicLength + 1];
		for (int i = 0; i < topicLength; i++)
		{
			pMqttTopic[i] = pDeserializedInfo->pPublishInfo->pTopicName[i];
		}
		pMqttTopic[topicLength] = '\0';

		LogInfo("Topic = %s", &pMqttTopic);
		LogInfo("Payload = %s", pDeserializedInfo->pPublishInfo->pPayload);


		//Decode
		SOPC_Buffer * buffer = SOPC_Buffer_Create(pDeserializedInfo->pPublishInfo->payloadLength);
		memcpy(buffer->data,pDeserializedInfo->pPublishInfo->pPayload,pDeserializedInfo->pPublishInfo->payloadLength);
		buffer->length = (uint32_t) pDeserializedInfo->pPublishInfo->payloadLength;
	    SOPC_SubTargetVariableConfig* targetConfig = NULL;
	    targetConfig = SOPC_SubTargetVariableConfig_Create(&set_target_variable2); //&Cache_SetTargetVariables (put in cache) //&set_target_variable2 (print)
		//on_mqtt_message_received(buffer->data, buffer->length,pContext->transportInterface.pNetworkContext->Connection_interface);
	    result = SOPC_Reader_Read_UADP(pContext->transportInterface.pNetworkContext->Connection_interface, buffer,
        							   targetConfig,
									   SOPC_SubScheduler_Get_Security_Infos,
									   SOPC_SubScheduler_Is_Writer_SN_Newer);
	    if (SOPC_STATUS_OK != result)
	    {
	        LogWarn("Decryption failed with the SOPC return status : %d", result);
	    }
        //Delete
		SOPC_Buffer_Delete(buffer);
		buffer = NULL;
	}
}



static uint32_t ulGlobalEntryTimeMs;
static uint32_t sopc_prvGetTimeMs( void )
{
    uint32_t ulTimeMs = 0UL;
    /* Determine the elapsed time in the application */
    ulTimeMs = ( uint32_t ) ( xTaskGetTickCount() * portTICK_PERIOD_MS ) - ulGlobalEntryTimeMs;
    return ulTimeMs;
}

SOPC_ReturnStatus SOPC_MQTT_Send_Message(MqttContextClient* contextClient, const char* topic, SOPC_Buffer message)
{
    MQTTStatus_t statusM = MQTTIllegalState;
	MQTTPublishInfo_t publishInfo;
	uint16_t packetId = 1; //needed for QoS > 0 but can be static (just != 0)
							// Be careful with packetId collision

	publishInfo.qos = MQTTQoS0;
	if (MQTTQoS0 == publishInfo.qos)
	{
		publishInfo.dup = false;
	}

	publishInfo.pTopicName = topic;
	publishInfo.topicNameLength = (uint16_t) strlen(topic);
	publishInfo.pPayload = message.data;
	publishInfo.payloadLength = message.length;

	packetId = MQTT_GetPacketId(contextClient);
	LogDebug("PacketId = %d",packetId);

	statusM = MQTT_Publish(contextClient, &publishInfo, packetId);

	if ( (MQTTSuccess == statusM) & (publishInfo.qos > MQTTQoS0) )
	{
		statusM = MQTT_ReceiveLoop(contextClient, 100);
		LogDebug("ACK !");
	}

	if (MQTTSuccess != statusM)
	{
	    SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Status Mqtt Error : %d", statusM);
		return SOPC_STATUS_NOK;
	}
	SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "publish message send !");
 /*
	if (packetId > 10)
	{
		SOPC_MQTT_Release_Client(contextClient);
	}
*/
    return SOPC_STATUS_OK;
}

static void mbedtls_deinit()
{
    mbedtls_ssl_free(&ssl_ctx.ssl);
    mbedtls_ssl_config_free(&ssl_ctx.conf);
    mbedtls_ctr_drbg_free(&ssl_ctx.ctr_drbg);
    mbedtls_entropy_free(&ssl_ctx.entropy);
    // TODO : check that the socket is closed by Lwip
    ssl_ctx.server_fd.fd = -1;
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_x509_crt_free(&ssl_ctx.ca);
    mbedtls_x509_crt_free(&ssl_ctx.cert_cli);
    mbedtls_pk_free(&ssl_ctx.pk_key);
#endif
}

static int mbedtls_init(const char* hostname, SOPC_Socket_AddressInfo* addrs, NetworkContext_t* pNetctx)
{
    int status = status_tls_ok;
    int ret = -1;

    // --- 0. Initialize and setup stuff ---
    mbedtls_ssl_init(&ssl_ctx.ssl);
    mbedtls_ssl_config_init(&ssl_ctx.conf);
    mbedtls_ctr_drbg_init(&ssl_ctx.ctr_drbg);
    mbedtls_entropy_init(&ssl_ctx.entropy);
    ssl_ctx.server_fd.fd = -1;
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_pk_init(&ssl_ctx.pk_key);
    mbedtls_x509_crt_init(&ssl_ctx.ca);
    mbedtls_x509_crt_init(&ssl_ctx.cert_cli);
#endif

    if (status_tls_ok == status)
    {
        ret = mbedtls_ctr_drbg_seed(&ssl_ctx.ctr_drbg, mbedtls_entropy_func, &ssl_ctx.entropy, NULL, 0);
        status = (ret != 0 ? ctr_drbg_seed_failed : status_tls_ok);
    }

    if (status_tls_ok == status)
    {
        ret = mbedtls_ssl_config_defaults(&ssl_ctx.conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
        status = (ret != 0 ? ssl_config_defaults_failed : status_tls_ok);
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    if (status_tls_ok == status)
    {
        ret = mbedtls_pk_parse_key(&ssl_ctx.pk_key, key_cloud, sizeof(key_cloud), NULL, 0, mbedtls_ctr_drbg_random, &ssl_ctx.ctr_drbg);
        status = (ret != 0 ? x509_key_parse_failed : status_tls_ok);
    }

    if (status_tls_ok == status)
    {
        ret = mbedtls_x509_crt_parse(&ssl_ctx.cert_cli, client_cert_cloud, sizeof(client_cert_cloud));
        status = (ret != 0 ? x509_crt_parse_failed : status_tls_ok);
    }

    if (status_tls_ok == status)
    {
        ret = mbedtls_ssl_conf_own_cert(&ssl_ctx.conf, &ssl_ctx.cert_cli, &ssl_ctx.pk_key);
        status = (ret != 0 ? ssl_alloc_failed : status_tls_ok);
    }

    if (status_tls_ok == status)
    {
        ret = mbedtls_x509_crt_parse(&ssl_ctx.ca, ca_cloud, sizeof(ca_cloud));
        status = (ret != 0 ? x509_crt_parse_failed : status_tls_ok);
    }
    mbedtls_ssl_conf_ca_chain(&ssl_ctx.conf, &ssl_ctx.ca, NULL);
    // set to check the peer certificat during handshake
    mbedtls_ssl_conf_authmode(&ssl_ctx.conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_rng(&ssl_ctx.conf, mbedtls_ctr_drbg_random, &ssl_ctx.ctr_drbg);
#endif

    if (status_tls_ok == status)
    {
        ret = mbedtls_ssl_setup(&ssl_ctx.ssl, &ssl_ctx.conf);
        status = (ret != 0 ? ssl_setup_failed : status_tls_ok);
    }

    if (status_tls_ok == status)
    {
        ret = mbedtls_ssl_set_hostname(&ssl_ctx.ssl, hostname);
        status = (ret != 0 ? hostname_failed : status_tls_ok);
    }

    // ---- 1. Start the connection ----
    if (status_tls_ok == status)
    {
        ret = ssl_ctx.server_fd.fd = socket(AF_INET, SOCK_STREAM, 0);
        status = (ret < 0 ? socket_failed : status_tls_ok);
    }

    if (status_tls_ok == status)
    {
        ret = connect(ssl_ctx.server_fd.fd, (const struct sockaddr *) addrs->ai_addr, addrs->ai_addrlen);
        status = (ret < 0 ? connect_failed : status_tls_ok);
    }

    mbedtls_ssl_set_bio(&ssl_ctx.ssl, &ssl_ctx.server_fd, mbedtls_ssl_send, mbedtls_ssl_recv, NULL);

    if (status_tls_ok == status)
    {
        ret = mbedtls_ssl_handshake(&ssl_ctx.ssl);
        status = (ret != 0 ? ssl_handshake_failed : status_tls_ok);
    }

    if (status_tls_ok != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_PUBSUB, "Init Mbed TLS fail with error code : %d", ret);
        mbedtls_deinit();
    }
    else
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_PUBSUB, "Init Mbed TLS Done");
    }

    pNetctx->sock = ssl_ctx.server_fd.fd;
    pNetctx->xSslCtx = &ssl_ctx.ssl;

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
	SOPC_UNUSED_ARG(cbMessageReceived);

	int ret = -1;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
	MQTTStatus_t statusM = MQTTIllegalState;
    NetworkContext_t* xNetworkContext = SOPC_Calloc(1, sizeof(NetworkContext_t));
	TransportInterface_t xTransport = {0}; // not to make an alloc because MQTT_Init just copies the content of the pointer and does not put this structure at the end of its own
	MQTTFixedBuffer_t fixedBuffer = {0}; // same
	uint8_t TLS_flag = 0;

	// -- Set buffer -- //
	uint8_t * pbuffer = SOPC_Calloc(4096, sizeof(uint8_t));
	fixedBuffer.pBuffer = pbuffer;
	fixedBuffer.size = 4096;

	// -- Parse URI -- //
	char* addrUri = SOPC_Malloc(strlen(uri) * sizeof(char));;
	addrUri = strncpy(addrUri,uri,strlen(uri));
	uint16_t len = 0;
	bool separator_bool = true;
	while(separator_bool)
	{
	    separator_bool = (addrUri[len] != ':');
	    len++;
	}
	char* pIP = SOPC_Calloc(len, sizeof(char));
	pIP = strncpy(pIP,uri,len-1);
	const char* pPort = uri+len;

	SOPC_Free(addrUri);
	addrUri = NULL;

	SOPC_Socket_AddressInfo* addrs = NULL;
	status = SOPC_Socket_AddrInfo_Get(pIP, pPort, &addrs);

	// -- socket connection and setup TLS (if secured port) -- //
	if (0 == strcmp(pPort,"8883")) // It's a TLS connection
	{
	    TLS_flag = 1;
	    ret = mbedtls_init(pIP, addrs, xNetworkContext); // Check
	    status = (ret != 0 ? SOPC_STATUS_NOK : SOPC_STATUS_OK);
	}
	else
	{
	    Socket sock1 = -1;
	    xNetworkContext->sock = sock1;
	    sopc_createsock(2, SOCK_STREAM, 0, &xNetworkContext->sock); //@parameter SOCK_STREAM involves tcp on lwip (@param 1 and 3 unused)
	    sopc_connectsock(xNetworkContext->sock, addrs);
	}


	// -- Init Mqtt context -- //
    xNetworkContext->Connection_interface = userContext;
	xTransport.pNetworkContext = xNetworkContext;

	if (1 == TLS_flag)
	{
	    xTransport.recv = mbedtls_net_recv_tls;
	    xTransport.send = mbedtls_net_send_tls;
	}
	else
	{
	    xTransport.recv = sopc_recvsock;
	    xTransport.send = sopc_sendsock;
	}

	statusM = MQTT_Init(contextClient, &xTransport, sopc_prvGetTimeMs, cb_msg_arrived_core_mqtt, &fixedBuffer);
	if (MQTTSuccess != statusM)
	{
		LogDebug("Status Mqtt Error : %d",statusM);
		return SOPC_STATUS_NOK;
	}

	// -- Connect to the borker -- //

	MQTTConnectInfo_t connectInfo = { 0 };
	connectInfo.cleanSession = true;
	connectInfo.pClientIdentifier = "S2OPC_MQTT";
	connectInfo.clientIdentifierLength = (uint16_t) strlen( connectInfo.pClientIdentifier );
	connectInfo.keepAliveSeconds = 60;
	connectInfo.pUserName = username;
	if (NULL == username)
	{
		connectInfo.userNameLength = 0;
	}
	else  {connectInfo.userNameLength = (uint16_t) strlen(username);}
	connectInfo.pPassword = password;
	if (NULL == password)
	{
		connectInfo.passwordLength = 0;
	}
	else  {connectInfo.passwordLength = (uint16_t) strlen(username);}
	bool sessionPresent = false;

	statusM = MQTT_Connect(contextClient, &connectInfo, NULL, 1000, &sessionPresent );

	if (MQTTSuccess != statusM)
	{
		LogDebug("Status Mqtt Error : %d",statusM);
		return SOPC_STATUS_NOK;
	}
	vTaskDelay(2000); // to receiv CONNACK
	LogInfo("Initialisation Done");

	if (nbSubTopic > 0)
	{
		//SUB
		MQTTSubscribeInfo_t subscriptionList[20] = { 0 };
		uint16_t packetId = 0;

		// Set each subscription.
		for( int i = 0; i < nbSubTopic; i++ )
		{
		     subscriptionList[i].qos = MQTTQoS1;
		     // Each subscription needs a topic filter.
		     subscriptionList[i].pTopicFilter = subTopic[i];
		     subscriptionList[i].topicFilterLength = (uint16_t) strlen(subTopic[i]);
		     // TODO :Print all subs
		}

		packetId = MQTT_GetPacketId( contextClient );
		statusM = MQTT_Subscribe( contextClient, &subscriptionList[0], nbSubTopic, packetId );

		if( MQTTSuccess == statusM )
		{
			LogInfo("SUB");
			for (int i = 0; i < 1000; i++)
			{
				MQTT_ReceiveLoop(contextClient, 10000000); // Need to stay in the loop and wait for the messages.
				// TODO : Analyze a better way to get out of this loop
				//vTaskDelay(500);
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
	MQTT_Disconnect(contextClient);

	mbedtls_deinit();
	//Delete
	SOPC_Free(contextClient->transportInterface.pNetworkContext);
	contextClient->transportInterface.pNetworkContext = NULL;
	contextClient->transportInterface.recv = 0;
	contextClient->transportInterface.send = 0;
	SOPC_Free(contextClient->networkBuffer.pBuffer);
	contextClient->networkBuffer.pBuffer = NULL;
	contextClient->networkBuffer.size = 0;
	SOPC_Free(contextClient);
}

bool SOPC_MQTT_Client_Is_Connected(MqttContextClient* contextClient)
{
	return contextClient->connectStatus;
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

bool SOPC_MQTT_Client_Is_Connected(MqttContextClient* contextClient)
{
    SOPC_UNUSED_ARG(contextClient);
    return false;
}

#endif
#endif
