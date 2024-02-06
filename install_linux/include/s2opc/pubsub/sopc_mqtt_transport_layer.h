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

/* This module implements MQTT Transport Protocol */

#ifndef SOPC_MQTT
#define SOPC_MQTT

#include <stdio.h>
#include <string.h>

#include "sopc_atomic.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_constants.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_types.h"

/* MQTT connection hard coded configuration */

#define MQTT_LIB_QOS (2)                   /* QOS of publish, subscribe set to 2*/
#define MQTT_LIB_MAX_SIZE_TOPIC_NAME (256) /* Maximum length of a topic */
#define MQTT_LIB_MAX_NB_TOPIC_NAME (256)   /* Maximum subscriber topics that can be handle by client */
#define MQTT_LIB_CONNECTION_TIMEOUT (4)    /* Connection lib timeout = 4 s*/
#define MQTT_LIB_KEEPALIVE (4)             /* Connection lost detection set to 4 s*/

typedef struct MQTT_CONTEXT_CLIENT MqttContextClient; /* MQTT context client */

typedef enum MQTT_CLIENT_STATE
{
    SOPC_MQTT_CLIENT_UNITIALIZED =
        0, /**< Client isn't initialized with function ::SOPC_MQTT_InitializeAndConnect_Client */
    SOPC_MQTT_CLIENT_INITIALIZED = 1,     /**< Client is successfully initialized with function
                                                ::SOPC_MQTT_InitializeAndConnect_Client */
    SOPC_MQTT_CLIENT_DISCONNECTED = 2,    /**< Client is disconnected */
    SOPC_MQTT_CLIENT_CONNECTED = 3,       /**< Client is successfully connected */
    SOPC_MQTT_CLIENT_FAILED_CONNECT = 4,  /**< Client failed to connect */
    SOPC_MQTT_CLIENT_LOST_CONNECTION = 5, /**< Client lost connection */
} MqttClientState;

/* Callback called to notify a message reception. */

typedef void FctMessageReceived(uint8_t* data, /**< Data received */
                                uint16_t size, /**< Size of data received in bytes */
                                void* pUser);  /**< Connection context */

/**
 * @brief Send message to topic destination with MQTT client, contextClient must be successfully initialized with
 * ::SOPC_MQTT_InitializeAndConnect_Client before using this function
 *
 * @param contextClient Context for MQTT library containing MQTT client
 * @param topic Topic destination
 * @param message Buffer with message information
 * @return ::SOPC_STATUS_OK if succeed sending message, ::SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_MQTT_Send_Message(MqttContextClient* contextClient, const char* topic, SOPC_Buffer message);

/**
 * @brief Initialize MQTT client and connection, Shall be called after ::SOPC_MQTT_Create_Client
 * To use MQTT client as a subscriber, number of sub topic must be superior to 0 and Topics to subscribe can't be NULL
 *
 * @param contextClient Context for MQTT library containing MQTT client
 * @param uri Uri to broker endpoint
 * @param username Username identifier optional. Can be NULL
 * @param password Password required if identifier. Can be NULL
 * @param subTopic Array of topics to subscribe, can be NULL
 * @param nbSubTopic Number of topics to subscribe, can be NULL
 * @param cbMessageReceived Callback of message reception
 * @param cbFatalError Callback for broker disconnection
 * @param userContext Connection context of user (Passed to \a cbFatalError and \a cbMessageReceived)
 * @return ::SOPC_STATUS_OK if succeed to initialize mqtt contextClient, ::SOPC_STATUS_NOK if failed to create a mqtt
 * Client or failed to connect
 */
SOPC_ReturnStatus SOPC_MQTT_InitializeAndConnect_Client(MqttContextClient* contextClient,
                                                        const char* uri,
                                                        const char* username,
                                                        const char* password,
                                                        const char** subTopic,
                                                        uint16_t nbSubTopic,
                                                        FctMessageReceived* cbMessageReceived,
                                                        SOPC_PubSub_OnFatalError* cbFatalError,
                                                        void* userContext);

/**
 * @brief Allocate memory for MQTT context client. Must be freed with ::SOPC_MQTT_Release_Client
 *
 * @param contextClient MQTT client context
 * @return SOPC_ReturnStatus ::SOPC_STATUS_OK if succeed to allocate memory, ::SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_MQTT_Create_Client(MqttContextClient** contextClient);

/**
 * @brief Disconnect from server and free MQTT client context pointer
 *
 * @param contextClient MQTT client context
 */
void SOPC_MQTT_Release_Client(MqttContextClient* contextClient);

/**
 * @brief Return if client is connected or not
 *
 * @param contextClient A valid MQTT client context from a successful ::SOPC_MQTT_Create_Client
 * @return true if client is connected
 * @return false if client is not connected or if parameters are invalid
 */
bool SOPC_MQTT_Client_Is_Connected(MqttContextClient* contextClient);

/**
 * @brief Return state of the client
 *
 * @param contextClient A valid MQTT client context from a successful ::SOPC_MQTT_Create_Client
 * @return ::MqttClientState state of client. Refers to enum description
 */
MqttClientState SOPC_MQTT_Client_Get_State(MqttContextClient* contextClient);

#endif
