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
 *  \file sopc_sockets_api.h
 *
 *  \brief Event oriented API of the Services layer.
 *
 *         This module is in charge of the event dispatcher thread management.
 */

#ifndef SOPC_SERVICES_API_H_
#define SOPC_SERVICES_API_H_

#include <stdint.h>

#include "sopc_event_handler.h"

typedef enum SOPC_Services_Event
{
    /* Services to services events */
    SE_TO_SE_SC_ALL_DISCONNECTED = 0x600, // special event sent by services mgr itself (no parameters)
    SE_TO_SE_ACTIVATE_ORPHANED_SESSION,   /* Client side only:
                                             id = session id
                                             auxParam = (uint32_t) endpoint connection config index
                                          */
    SE_TO_SE_CREATE_SESSION,              /* Client side only:
                                             id = session id
                                             auxParam = (uint32_t) endpoint connection config index
                                          */
    SE_TO_SE_ACTIVATE_SESSION,            /* Client side only:
                                           * id = session id
                                           * params = (user token structure)
                                           */

    SE_TO_SE_SERVER_DATA_CHANGED, /* Server side only:
                                    id = session id
                                    auxParam = (int32_t) session state
                                  */

    SE_TO_SE_SERVER_INACTIVATED_SESSION_PRIO, /* Server side only:
                                                 id = session id
                                                 auxParam = (int32_t) session state
                                               */
    SE_TO_SE_SERVER_SEND_ASYNC_PUB_RESP_PRIO, /* Server side only:
                                                 id = session id
                                                 params = (SOPC_Internal_AsyncSendMsgData*)
                                                 auxParams = (constants_statuscodes_bs__t_StatusCode_i) service result
                                                 code
                                               */

    /* Timer to services events */
    TIMER_SE_EVAL_SESSION_TIMEOUT,  /* Server side only: id = session id */
    TIMER_SE_PUBLISH_CYCLE_TIMEOUT, /* Server side only: id = subscription id */

    /* App to Services events : server side */
    APP_TO_SE_OPEN_ENDPOINT,         /* id = endpoint description config index
                                      */
    APP_TO_SE_CLOSE_ENDPOINT,        /* id = endpoint description config index
                                      */
    APP_TO_SE_LOCAL_SERVICE_REQUEST, /* id = endpoint connection config index,
                                        params = (OpcUa_<MessageStruct>*) OPC UA message payload structure (header
                                         ignored)
                                        auxParam = user application session context
                                      */
    /* App to Services events : client side */
    APP_TO_SE_ACTIVATE_SESSION,       /* Connect SC + Create Session + Activate session */
                                      /* id = endpoint connection config index,
                                       * params = (user token structure)
                                         auxParam = user application session context
                                       */
    APP_TO_SE_SEND_SESSION_REQUEST,   /* id = session id,
                                         params = (OpcUa_<MessageStruct>*) OPC UA message payload structure (header
                                         ignored)
                                         auxParam = user application request context
                                      */
    APP_TO_SE_SEND_DISCOVERY_REQUEST, /* id = endpoint connection config index,
                                         params = (OpcUa_<MessageStruct>*) OPC UA message payload structure (header
                                         ignored)
                                         auxParam = user application request context
                                       */
    APP_TO_SE_CLOSE_SESSION,          // id = session id
    APP_TO_SE_CLOSE_ALL_CONNECTIONS,  // Automatically called by toolkit clear (no params)
} SOPC_Services_Event;

/* API to enqueue an event for services */
void SOPC_Services_EnqueueEvent(SOPC_Services_Event seEvent, uint32_t id, void* params, uintptr_t auxParam);

/**
 *  \brief Initializes the services and application event dispatcher threads
 */
void SOPC_Services_Initialize(SOPC_SetListenerFunc setSecureChannelsListener);

/**
 *  \brief Notify the toolkit configuration is complete to initialize the services B model
 */
void SOPC_Services_ToolkitConfigured(void);

/**
 *  \brief Notify that the clear function will be called to finish
 *         operations using secure channels and sockets services.
 */
void SOPC_Services_PreClear(void);

/**
 *  \brief Stop and clear the services and application event dispatcher threads
 */
void SOPC_Services_Clear(void);

// Internal use only (timers)
SOPC_EventHandler* SOPC_Services_GetEventHandler(void);

#endif /* SOPC_SERVICES_API_H_ */
