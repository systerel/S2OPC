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
#include "sopc_pubsub_constants.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_types.h"
#include "p_sockets.h"

/* MQTT connection hard coded configuration */

#define MQTT_LIB_QOS (2)                   /* QOS of publish, subscribe set to 2*/
#define MQTT_LIB_MAX_SIZE_TOPIC_NAME (256) /* Maximum length of a topic */
#define MQTT_LIB_MAX_NB_TOPIC_NAME (256)   /* Maximum subscriber topics that can be handle by client */
#define MQTT_LIB_CONNECTION_TIMEOUT (4)    /* Connection lib timeout = 4 s*/
#define MQTT_LIB_KEEPALIVE (4)             /* Connection lost detection set to 4 s*/

#if USE_CORE_MQTT
typedef struct MQTTContext	 MqttContextClient; /* Mqtt context client */
#else
typedef struct MQTT_CONTEXT_CLIENT MqttContextClient; /* Mqtt context client */
#endif
/* Callback called to notify a message reception. */

typedef void FctMessageReceived(uint8_t* data, /* Data received */
                                uint16_t size, /* Size of data received in bytes */
                                void* pUser);  /* Connexion context */

/**
 * @brief Send message to topic destination with mqtt client, contextClient must be initialized with
 * SOPC_MQTT_Initialize_Client before using this function
 *
 * @param contextClient Context for mqtt library containing mqtt client
 * @param topic Topic destination
 * @param message Buffer with message information
 * @return ::SOPC_STATUS_OK if succeed sending message, ::SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_MQTT_Send_Message(MqttContextClient* contextClient, const char* topic, SOPC_Buffer message);

/**
 * @brief Initialise mqtt client connexion, Shall be called after ::SOPC_MQTT_Create_Client
 * To use mqtt client as a subscriber, number of sub topic must be superior to 0 and Topics to subscribe can't be NULL
 *
 * @param contextClient Context for mqtt library containing mqtt client
 * @param uri Uri to broker endpoint
 * @param username Username identifier optional. Can be NULL
 * @param password Password required if identifier. Can be NULL
 * @param subTopic Array of topics to subscribe, can be NULL
 * @param nbSubTopic Number of topics to subscribe, can be NULL
 * @param cbMessageReceived Callback of message reception
 * @param userContext Connexion context of user
 * @return ::SOPC_STATUS_OK if succeed to initialize mqtt contextClient, ::SOPC_STATUS_NOK if failed to create a mqtt
 * Client
 */
SOPC_ReturnStatus SOPC_MQTT_Initialize_Client(MqttContextClient* contextClient,
                                              const char* uri,
                                              const char* username,
                                              const char* password,
                                              const char** subTopic,
                                              uint16_t nbSubTopic,
                                              FctMessageReceived* cbMessageReceived,
                                              void* userContext);

/**
 * @brief Allocate memory for mqtt context client. Must be freed with ::SOPC_MQTT_Release_Client
 *
 * @param contextClient mqtt client context
 * @return SOPC_ReturnStatus ::SOPC_STATUS_OK if succeed to allocate memory, ::SOPC_STATUS_NOK otherwise
 */
SOPC_ReturnStatus SOPC_MQTT_Create_Client(MqttContextClient** contextClient);

/**
 * @brief Disconnect from server and free mqtt client context pointer
 *
 * @param contextClient mqtt client context
 */
void SOPC_MQTT_Release_Client(MqttContextClient* contextClient);

/**
 * @brief Return if client is connected or not
 *
 * @param contextClient A valid mqtt client context from a successful ::SOPC_MQTT_Create_Client
 * @return true if client is connected
 * @return false if client is not connected or if parameters are invalid
 */
bool SOPC_MQTT_Client_Is_Connected(MqttContextClient* contextClient);

#endif
