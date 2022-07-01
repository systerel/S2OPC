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

/* To use this module, you can follow the following steps (SYNCHRONE API with callback of reception). For more examples,
 * see mqtt_pub_test.c.*/

/* 1) Instantiate a MQTT Manager with SOPC_MQTT_MGR_Create : MqttManagerHandle * pMgr ; SOPC_MQTT_MGR_Create(&pWks);*/
/* 2) Define a reception callback : static void cbReceivedMessage(MqttTransportHandle* pCtx, uint8_t* data, uint16_t
 * size){ return; } */
/* 3) Get a new transport handle with SOPC_MQTT_TRANSPORT_SYNCH_GetHandle : MqttTransportHandle *pHandleSynchroContext =
 * SOPC_MQTT_TRANSPORT_SYNCH_GetHandle(pWks, "tcp://192.168.56.1", cbReceivedMessage); */
/* 4) Send a  message with SOPC_MQTT_TRANSPORT_SYNCH_SendMessage :
 * SOPC_MQTT_TRANSPORT_SYNCH_SendMessage(pHandleSynchroContext, buffer, bufferSizeInBytes, 2000); */
/* 5) On message reception, your callback "cbReceivedMessage" is called. */
/* 6) Release your transport handle with SOPC_MQTT_TRANSPORT_SYNCH_ReleaseHandle :
 * SOPC_MQTT_TRANSPORT_SYNCH_ReleaseHandle(&pHandleSynchroContext).*/

#ifndef SOPC_MQTT
#define SOPC_MQTT

#include "sopc_atomic.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_types.h"

#include <stdio.h>
#include <string.h>

/* Max connexions and scheduler queue size */

#define MQTT_MAX_BUFFER_EVENT (4096)   /* Maximum of buffered events */
#define MQTT_MAX_BUFFER_RCV_MSG (4096) /* Maximum of buffered messages received (sync api if null callback) */
#define MQTT_MAX_TRANSPORT_CONTEXT (4) /* Maximum of broker connections <=> Max number of transport context handle.*/

/* Timing hard coded configuration */

#define MQTT_SCHEDULER_PERIOD_MS (100) /* Scheduler period set to 100 ms */
#define MQTT_STATUS_TIMEOUT_NB_PERIODS \
    (30 * (1000 / MQTT_SCHEDULER_PERIOD_MS)) /* Status timeout of MQTT manager set to 30 s*/
#define MQTT_STATUS_TIMEOUT_SENDING_NB_PERIODS \
    (2 * MQTT_LIB_QOS * (1000 / MQTT_SCHEDULER_PERIOD_MS)) /* Sending timeout set to 2 s */
#define MQTT_STATUS_TIMEOUT_SUBSCRIBING_NB_PERIODS \
    (5 * (1000 / MQTT_SCHEDULER_PERIOD_MS)) /* Subscribing timeout set to 5 s*/
#define MQTT_STATUS_TIMEOUT_CONNECTING_NB_PERIODS \
    (5 * (1000 / MQTT_SCHEDULER_PERIOD_MS)) /* Connecting timeout set to 5 s*/
#define MQTT_STATUS_TIMEOUT_DISCONNECTING_NB_PERIODS \
    (5 * (1000 / MQTT_SCHEDULER_PERIOD_MS))                                   /* Disconnecting timeout set to 5 s*/
#define MQTT_RETRY_TIMEOUT_NB_PERIODS (5 * (1000 / MQTT_SCHEDULER_PERIOD_MS)) /* Retry period = 5ss*/
#define MQTT_CONNECTION_RETRY_ON_ERROR (3)                                    /* Try connection set to 3*/

/* MQTT connection hard coded configuration */

#define MQTT_LIB_QOS (2)                /* QOS of publish, subscribe set to 2*/
#define MQTT_LIB_CONNECTION_TIMEOUT (4) /* Connection lib timeout = 4 s*/
#define MQTT_LIB_KEEPALIVE (4)          /* Connection lost detection set to 4 s*/
#define MQTT_LIB_TOPIC_NAME ("S2OPC")   /* Topic name hard coded.*/
#define MQTT_LIB_MAX_SIZE_TOPIC_NAME (256)
#define MQTT_LIB_MAX_SIZE_URI (256)

/* Others constants*/

#define MQTT_INVALID_TRANSPORT_ASYNC_HANDLE (0xFFFF) /* Invalid transport context identifier */

/* Debugging configuration */

#define DEBUG_SCHEDULER (0)
#define DEBUG_LOOP_ALIVE (0)
#define DEBUG_API (1)
#define DEBUG_CALLBACKS (1)
#define DEBUG_SYNCHRO_CALLBACKS (1)
#define DEBUG_CHANNEL_UNAVAILABLE_ERROR (1)
#define DEBUG_LIB_CALLBACKS (1)
#define DEBUG_TMO_CALLBACKS (1)
#define DEBUG_TMO_TICK (1)
#define DEBUG_SYNCHRO_API_CALLBACKS (1)
#define DEBUG_SYNCHRO_API (1)

/*** Handles definitions ***/

typedef struct T_MQTT_MANAGER_HANDLE MqttManagerHandle;     /* Mqtt manager handle */
typedef struct T_MQTT_TRANSPORT_HANDLE MqttTransportHandle; /* Mqtt transport synchrone context handle */
typedef uint16_t MqttTransportAsyncHandle;                  /* Mqtt transport asynchrone context handle */

/*** Definition of callbacks prototypes ***/

/* Callback called to notify a client status change event (ready, not ready) */

typedef void FctClientStatus(MqttTransportAsyncHandle idx, /* Transport context async handle */
                             void* pCtx);                  /* User context */

/* Callback called to notify a get transport context async handle response */

typedef void FctGetHandleResponse(MqttTransportAsyncHandle idx, /* Transport context async handle */
                                  void* pCtx);                  /* User context */

/* Callback called to notify a message reception. */

typedef void FctMessageReceived(MqttTransportAsyncHandle idx, /* Transport context async handle */
                                uint8_t* data,                /* Data received */
                                uint16_t size,                /* Size of data received in bytes */
                                void* pCtx);                  /* User context */

/* Callback called to notify a release transport context handle response */

typedef void FctReleaseHandleResponse(MqttTransportAsyncHandle idx, /* Transport context async handle*/
                                      void* pCtx);                  /* User context */

/* Callback called to notify a message reception. This callback is used by synchrone API.*/

typedef void FctMessageSyncReceived(MqttTransportHandle* pCtx, /* Transport context handle */
                                    uint8_t* data,             /* Data received */
                                    uint16_t size,             /* Size of data received, in bytes. */
                                    void* pUserContext);       /* User context */

/*** MQTT Manager API ***/

/** \brief Create a MQTT manager, which is used to manage several MQTT Transport Context */
SOPC_ReturnStatus SOPC_MQTT_MGR_Create(MqttManagerHandle** ppWks);

/** \brief Destroy a MQTT manager
 *
 * Sets the handle to NULL
 */
SOPC_ReturnStatus SOPC_MQTT_MGR_Destroy(MqttManagerHandle** ppWks);

/*** MQTT Transport Asynchrone API ***/

/** \brief Get a new async handle to use with ASYNC API
 *
 * \param pWks          MQTT Manager Handle
 * \param pUserContext  User context, pass in paramters of callbacks
 * \param uri           Uri of broker
 * \param topicname     Topic name to subscribe
 * \param cbGetHandleSuccess Callback of success of GetTransportAsyncHandle
 * \param cbGetHandleFailure Callback of failure of GetTransportAsyncHandle
 * \param cbClientReady  Callback of status change to READY
 * \param cbClientNotReady Callback of status change to NOT READY
 * \param cbMessageReceived Callback of message reception
 * \param cbReleaseHandle Callback of success of ReleaseHandle
 */
SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_ASYNC_GetHandle(MqttManagerHandle* pWks,
                                                      void* pUserContext,
                                                      const char* uri,
                                                      const char* topicname,
                                                      FctGetHandleResponse* cbGetHandleSuccess,
                                                      FctGetHandleResponse* cbGetHandleFailure,
                                                      FctClientStatus* cbClientReady,
                                                      FctClientStatus* cbClientNotReady,
                                                      FctMessageReceived* cbMessageReceived,
                                                      FctReleaseHandleResponse* cbReleaseHandle);

/** \brief Release transport async handle
 *
 * \param pWks  MQTT Manager Handle
 * \param idx   MQTT Transport Async Handle
 */
SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_ASYNC_ReleaseHandle(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);

/** \brief Send a new message to transport async handle.
 *
 * \param pWks      MQTT Manager Handle
 * \param idx       MQTT Transport Async Handle
 * \param bufferData Data to send
 * \param size      Size of data to send in bytes
 */
SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_ASYNC_SendMessage(MqttManagerHandle* pWks,
                                                        MqttTransportAsyncHandle idx,
                                                        uint8_t* bufferData,
                                                        uint16_t size);

/*** MQTT Transport Synchrone API ***/

/** \brief Return a new transport synchrone handle
 *
 * Note that the returned handle, 1 by new broker connection, is scheduled by the same mqtt manager.
 * So, callback passed as parameter for 2 client is thread safe between 2 clients.
 *
 * \param pWks  MQTT Manager handle
 * \param uri   Uri of broker
 * \param topicName  MQTT topic name used for subscriptions
 * \param getMsg  Callback of message reception. If NULL, SOPC_MQTT_TRANSPORT_SYNCH_ReadMessage
 *                must be used to retrieve SOPC_Buffer data message.
 * \param pUserContext  Passed when calling getMsg
 */
MqttTransportHandle* SOPC_MQTT_TRANSPORT_SYNCH_GetHandle(MqttManagerHandle* pWks,
                                                         const char* uri,
                                                         const char* topicName,
                                                         FctMessageSyncReceived getMsg,
                                                         void* pUserContext);

/** \brief Release a transport synchrone handle */
SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_SYNCH_ReleaseHandle(MqttTransportHandle** ppCtx);

/** \brief Send a message to a transport synchrone handle.
 *
 * \param pCtx          MQTT Transport Synchrone Handle where send message
 * \param bufferData    Data to send
 * \param dataSize      Size of data
 * \param timeoutMs     Send timeout. UINT32_MAX not recommended.
 */
SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_SYNCH_SendMessage(MqttTransportHandle* pCtx,
                                                        uint8_t* bufferData,
                                                        uint16_t dataSize,
                                                        uint32_t timeoutMs);

/** \brief Read a message received by a transport synchrone handle.
 *
 * If callback of reception has been passed to GetHandle, no data is returned by this function.
 *
 * \param pCtx      MQTT Transport Synchrone Handle from where read received messages.
 * \param ppBuffer  If buffer returned, some data has been received.
 * \param timeoutMs Timeout of reception in ms.
 */
SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_SYNCH_ReadMessage(MqttTransportHandle* pCtx,
                                                        SOPC_Buffer** ppBuffer,
                                                        uint32_t timeoutMs);

#endif
