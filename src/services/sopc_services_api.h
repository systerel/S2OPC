/*
 *  Copyright (C) 2017 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SOPC_SERVICES_API_H
#define _SOPC_SERVICES_API_H

#include <stdint.h>

#include "sopc_user_app_itf.h"

typedef enum SOPC_Services_Event {
  /* SC to Services events */
  SC_TO_SE_EP_SC_CONNECTED,            /* id = endpoint description config index,
                                          params = endpoint connection config index pointer,
                                          auxParams = secure channel connection index
                                       */
  SC_TO_SE_EP_CLOSED,                  /* id = endpoint description config index,
                                       */
  SC_TO_SE_SC_CONNECTED,               /* id = endpoint description config index,
                                          auxParams = secure channel connection index
                                       */
  SC_TO_SE_SC_CONNECTION_TIMEOUT,      /* id = endpoint connection config index
                                       */
  SC_TO_SE_SC_DISCONNECTED,            /* id = secure channel connection index
                                       */
  SC_TO_SE_SC_SERVICE_RCV_MSG,         /* id = secure channel connection index,
                                          params = (SOPC_Buffer*) OPC UA message payload buffer,
                                          auxParam = request Id context (server side only)
                                       */
  /* Services to services events */
  SE_TO_SE_SC_ALL_DISCONNECTED, // special event sent by services mgr itself (no parameters)
  SE_TO_SE_ACTIVATE_ORPHANED_SESSION,  /* id = session id
                                          auxParam = endpoint conneciton config index
                                       */
  SE_TO_SE_CREATE_SESSION,             /* id = session id
                                          auxParam = endpoint conneciton config index
                                       */
  SE_TO_SE_ACTIVATE_SESSION,           /* id = session id
                                          auxParam = user (index ?)
                                       */
  /* App to Services events */
  APP_TO_SE_OPEN_ENDPOINT,             /* id = endpoint description config index
                                       */
  APP_TO_SE_CLOSE_ENDPOINT,            /* id = endpoint description config index
                                        */
  APP_TO_SE_ACTIVATE_SESSION,          /* Connect SC + Create Session + Activate session */
                                       /* id = endpoint connection config index,
                                          auxParam = user (index ?)
                                        */
  APP_TO_SE_SEND_SESSION_REQUEST,       // TODO: manage buffer when session with channel lost ? Or return a send failure in this case
                                        /* id = session id,
                                           params = (OpcUa_<MessageStruct>*) OPC UA message payload structure
                                         */
  APP_TO_SE_CLOSE_SESSION,               /* id = session id
                                         */
  APP_TO_SE_CLOSE_ALL_CONNECTIONS, // Automatically called by toolkit clear (no params)
  //  SE_SEND_PUBLIC_REQUEST, => discovery services /* Connect SC */

  /* App to Services: local services events */ 
  APP_TO_SE_LOCAL_READ, // TBD
  APP_TO_SE_LOCAL_WRITE // TBD
} SOPC_Services_Event;

/* API to enqueue an event for services */
void SOPC_Services_EnqueueEvent(SOPC_Services_Event seEvent,
                                uint32_t            id,
                                void*               params,
                                uint32_t            auxParam);

/* API to enqueue an event for application */
void SOPC_ServicesToApp_EnqueueEvent(SOPC_App_Com_Event  appEvent,
                                     uint32_t            eventType,
                                     void*               params,
                                     uint32_t            auxParam);

/**
 *  \brief Initializes the services and application event dispatcher threads
 */
void SOPC_Services_Initialize();

/**
 *  \brief Notify the toolkit configuration is terminated to could initialize the services B model
 */
void SOPC_Services_ToolkitConfigured();

/**
 *  \brief Notify that the clear function will be called to could terminate
 *         operations using secure channels and sockets services.
 */
void SOPC_Services_PreClear();

/**
 *  \brief Stop and clear the services and application event dispatcher threads
 */
void SOPC_Services_Clear();


#endif /* _SOPC_SERVICES_API_H */
