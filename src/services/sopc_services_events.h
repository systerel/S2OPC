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

#ifndef _sopc_services_events_h
#define _sopc_services_events_h

#include <stdint.h>

typedef enum SOPC_Services_Event {
  /* SC to Services events */
  SC_TO_SE_EP_SC_CONNECTED,
  SC_TO_SE_EP_CLOSED,
  SC_TO_SE_SC_CONNECTED,
  SC_TO_SE_SC_CONNECTION_TIMEOUT,
  SC_TO_SE_SC_DISCONNECTED,
  SC_TO_SE_SC_SERVICE_RCV_MSG,
  /* Services to services events */
  SE_TO_SE_SC_ALL_DISCONNECTED, // special event sent by services mgr itself
  SE_TO_SE_ACTIVATE_ORPHANED_SESSION,
  SE_TO_SE_CREATE_SESSION,
  SE_TO_SE_ACTIVATE_SESSION,
  /* App to Services events */
  APP_TO_SE_OPEN_ENDPOINT,
  APP_TO_SE_CLOSE_ENDPOINT,
  APP_TO_SE_ACTIVATE_SESSION, /* Connect SC + Create Session + Activate session */
  APP_TO_SE_SEND_SESSION_REQUEST, // TODO: manage buffer when session with channel lost ? Or return a send failure in this case
  APP_TO_SE_CLOSE_SESSION,
  APP_TO_SE_CLOSE_ALL_CONNECTIONS, // Automatically called by toolkit clear
  //  SE_SEND_PUBLIC_REQUEST, => discovery services /* Connect SC */

  /* App to Services: local services events */ 
  APP_TO_SE_LOCAL_READ,
  APP_TO_SE_LOCAL_WRITE
} SOPC_Services_Event;

/* API to enqueue an event for services */
void SOPC_Services_EnqueueEvent(SOPC_Services_Event seEvent,
                                uint32_t            id,
                                void*               params,
                                uint32_t            auxParam,
                                const char*         reason);

void SOPC_ServicesEventDispatcher(int32_t  scEvent, 
                                  uint32_t id, 
                                  void*    params, 
                                  uint32_t auxParam);


void SOPC_ApplicationEventDispatcher(int32_t  appEvent, 
                                     uint32_t eventType, 
                                     void*    params, 
                                     uint32_t auxParam);

#endif /* _sopc_services_events_h */
