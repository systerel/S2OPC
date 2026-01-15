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

/**
 * \file sopc_sockets_api.h
 *
 * \brief Event oriented API of the Services layer.
 *
 *   This module is in charge of the event dispatcher thread management.
 */

#ifndef SOPC_SERVICES_API_H_
#define SOPC_SERVICES_API_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_event_handler.h"
#include "sopc_types.h"

/**
 * Structure used to store old and new values for data changed event
 */
typedef struct SOPC_WriteDataChanged
{
    OpcUa_WriteValue* oldValue;
    OpcUa_WriteValue* newValue;
} SOPC_WriteDataChanged;

/**
 * Structure used to store node change notifications
 */
typedef struct SOPC_NodeChanged
{
    bool added;          /* true if node added, false if node deleted */
    SOPC_NodeId* nodeId; /* NodeId of the node added/deleted */
} SOPC_NodeChanged;

/**
 * Services events input events from application, services layer it-self or timer manager
 */
typedef enum SOPC_Services_Event
{
    /* Services to services events */
    SE_TO_SE_SC_ALL_DISCONNECTED =
        0x600,                          /**< Special event sent by B code to indicate all SC are closed. used to stop
                                           waiting synchronously disconnection of all secure channels before during toolkit
                                           clearing phase.<BR/>
                                           It might concern only SC established as client or both client/server.<BR/>
                                           params = true if for SCs as client only, false if both client and server SCs.
                                         */
    SE_TO_SE_ACTIVATE_ORPHANED_SESSION, /**< Client side only:<BR/>
                                           Re-activate an orphaned session on a new secure channel connection.<BR/>
                                           id = session id<BR/>
                                           auxParam = (uint32_t) secure channel config index
                                        */
    SE_TO_SE_CREATE_SESSION,            /**< Client side only:<BR/>
                                           Create a session on a new secure channel connection.<BR/>
                                           id = session id<BR/>
                                           auxParam = (uint32_t) secure channel config index
                                        */
    SE_TO_SE_ACTIVATE_SESSION,          /**< Client side only:<BR/>
                                         * Activate the session for which creation has succeeded.<BR/>
                                         * id = session id<BR/>
                                         * params = (user token structure)
                                         */

    SE_TO_SE_SERVER_DATA_CHANGED, /**< Server side only:<BR/>
                                    Notifies that address space data have changed, used to generate notification in
                                    subscription mechanism.<BR/>
                                    params = (SOPC_Array*) array of SOPC_WriteDataChanged<BR/>
                                  */

    SE_TO_SE_SERVER_NODE_CHANGED, /**< Server side only:<BR/>
                                     Notifies that address space nodes have changed, used to update
                                     subscription/monitored items state.<BR/>
                                     params = (SOPC_Array*) array of SOPC_NodeChanged<BR/>
                                   */

    SE_TO_SE_SERVER_INACTIVATED_SESSION_PRIO, /**< Server side only:<BR/>
                                                 Notifies that an activated session is not active anymore (closed or
                                                 orphaned), used to update subscription state.<BR/>
                                                 id = session id<BR/>
                                                 auxParam = (int32_t) session state
                                               */
    SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO, /**< Server side only:<BR/>
                                                 Provides an asynchronous publish response to be sent.<BR/>
                                                 id = session id<BR/>
                                                 params = (SOPC_Internal_AsyncSendMsgData*)
                                               */
    SE_TO_SE_SERVER_ASYNC_CLOSE_SUBSCRIPTION, /**< Server side only:<BR/>
                                                   Requests to close a subscription asynchronously.<BR/>
                                                   id = subscription id
                                               */
    /* Timer to services events */
    TIMER_SE_EVAL_SESSION_TIMEOUT,  /**< Server side only:<BR/>
                                       Evaluates if the session has been used during session timeout
                                       duration, otherwise close the session.<BR/>
                                       id = session id */
    TIMER_SE_PUBLISH_CYCLE_TIMEOUT, /**< Server side only: evaluates the publish cycle timeout expiration for the
                                       subscription.<BR/>
                                       id = subscription id */

    /* App to Services events : server side */
    APP_TO_SE_OPEN_ENDPOINT,         /**< Server side only:<BR/>
                                          Requests to open a new endpoint listening for secure channel
                                          connections.<BR/>
                                          id = endpoint description config index
                                      */
    APP_TO_SE_CLOSE_ENDPOINT,        /**< Server side only:<BR/>
                                          Requests to close an opened endpoint listening for connections.<BR/>
                                          id = endpoint description config index
                                      */
    APP_TO_SE_LOCAL_SERVICE_REQUEST, /**< Server side only:<BR/>
                                        Requests to run the provided service request locally on the server (no session
                                        and user considered)<BR/>
                                        id = endpoint description config index<BR/>
                                        params = (OpcUa_<MessageStruct>*) OPC UA message payload structure (header
                                        ignored)<BR/> auxParam = user application session context
                                      */
    APP_TO_SE_TRIGGER_EVENT,         /**< Server side only:<BR/>
                                        Trigger the provided event from the given node notifier with possibly subscription
                                        filtering.
                                        params = (SOPC_Internal_EventContext*) triggered event context
                                      */
    /* App to Services events : client side */
    APP_TO_SE_OPEN_REVERSE_ENDPOINT,  /**< Server side only: <BR/>
                                         Requests to open a new reverse endpoint listening for secure channel
                                         connections.<BR/>
                                         id = reverse endpoint description config index */
    APP_TO_SE_CLOSE_REVERSE_ENDPOINT, /**< Server side only: <BR/>
                                         Requests to close an opened endpoint listening for reverse connections.<BR/>
                                         id = reverse endpoint description config index */
    APP_TO_SE_ACTIVATE_SESSION,       /**< Client side only: <BR/>
                                         Requests to Connect SC, Create and Activate session a session using a given
                                         connection configuration<BR/>
                                         id = secure channel config index<BR/>
                                         params = reverse endpoint connection index or 0 if not a reverse connection<BR/>
                                         auxParam = (SOPC_Internal_SessionAppContext*)
                                       */
    APP_TO_SE_SEND_SESSION_REQUEST,   /**< Client side only:<BR/>
                                         Requests to send a service request on a activated session.<BR/>
                                         id = session id<BR/>
                                         params = (OpcUa_<MessageStruct>*) OPC UA message payload structure (header
                                                  ignored)<BR/>
                                         auxParam = user application request context
                                      */
    APP_TO_SE_SEND_DISCOVERY_REQUEST, /**< Client side only:<BR/>
                                         Requests to send a discovery service request (for which no session is
                                         necessary)<BR/>
                                         id = secure channel config index<BR/>
                                         params = reverse endpoint connection index or 0 if not a reverse
                                         connection<BR/> auxParam = (SOPC_Internal_DiscoveryContext*)
                                       */
    APP_TO_SE_CLOSE_SESSION,          /**< Client side only:<BR/>
                                         requests to close an activated session.<BR/>
                                         id = session id */
    APP_TO_SE_CLOSE_CONNECTION,       /**< Client side only:<BR/>
                                         requests to close an established or pending SC connection.<BR/>
                                         id = secure channel config index<BR/>
                                         auxParam = user application close connection context
                                       */
    APP_TO_SE_CLOSE_ALL_CONNECTIONS,  /**< (internal use only). Request to close all established SC connections<BR/>
                                         It might be used to close only SC established as client or both client/server.
                                         Automatically called by ::SOPC_Toolkit_Clear for both client and server.<BR/>
                                         params = true if for SCs as client only, false if both client and server SCs.
                                       */
    APP_TO_SE_REEVALUATE_SCS,         /**< Re-evaluate the secure channels due to application certificate/key update
                                           (force SC re-establishment)
                                           or PKI application trust list update (peer certificate re-validation necessary).<BR/>
        
                                           params = (bool) flag indicating if it concerns server (true) or client (false)
                                           application secure channels.<BR/>
                                           auxParam = (bool) flag indicating if it concerns application certificate/key update
                                           (true), otherwise the PKI trust list update (false).
                                      */
    APP_TO_SE_EVAL_USR_CRT_SESSIONS,  /**< Server side only:<BR/>
                                           Re-evaluates X509IdentityToken certificates for all active sessions.
                                           If an user certificate is not valid or trusted anymore,
                                           the associated session is closed.<BR/>
                                       */
    APP_TO_SE_UNINITIALIZE_SERVICES,  /**< Server and client sides:<BR/>
                                           Calls io_dispatch_mgr__UNINITIALISATION() function.
                                      */
} SOPC_Services_Event;

/* API to enqueue an event for services */
void SOPC_Services_EnqueueEvent(SOPC_Services_Event seEvent, uint32_t id, uintptr_t params, uintptr_t auxParam);

/** \brief Returns the current pending number of events in Services queue */
uint32_t SOPC_Services_Get_QueueSize(void);

/**
 *  \brief Initializes the services and application event dispatcher threads
 */
void SOPC_Services_Initialize(SOPC_SetListenerFunc* setSecureChannelsListener);

/**
 *  \brief Close all SecureChannels (established as client or both) in a synchronous way.
 *
 *  \param clientOnly  If flag is set, only the Secure Channels established as a client are closed.
 *                     Otherwise both Secure Channels established as a server and as client are closed.
 *
 *  \warning   It is a pre-requisite to call SOPC_Services_CloseAllSCs(false) prior to ::SOPC_Services_Clear
 */
void SOPC_Services_CloseAllSCs(bool clientOnly);

/**
 *  \brief Stop and clear the services and application event dispatcher threads
 */
void SOPC_Services_Clear(void);

// Internal use only (timers)
SOPC_EventHandler* SOPC_Services_GetEventHandler(void);

#endif /* SOPC_SERVICES_API_H_ */
