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

typedef enum SOPC_Services_Events {
  /* SC to Services events */
  EP_SC_CONNECTED,
  EP_CLOSED,
  SC_CONNECTED,
  SC_CONNECTION_TIMEOUT,
  SC_DISCONNECTED,
  SC_SERVICE_RCV_MSG,
  /* App to Services events */
  SE_OPEN_ENDPOINT,
  SE_CLOSE_ENDPOINT,
  SE_ACTIVATE_SESSION, /* Connect SC + Create Session + Activate session */
  SE_SEND_SESSION_REQUEST, // TODO: manage buffer when session with channel lost ? Or return a send failure in this case
  SE_CLOSE_SESSION,
  //  SE_SEND_PUBLIC_REQUEST, => discovery services /* Connect SC */
} SOPC_Services_Events;

// TODO: move into SC impl !
typedef enum SOPC_SC_Events {
  /* Services to SC events */
  SC_CONNECT,
  SC_DISCONNECT,
  SC_SERVICE_SND_MSG
} SOPC_Services_To_SC_Events;

void SOPC_ServicesEventDispatcher(int32_t  scEvent, 
                                  uint32_t id, 
                                  void*    params, 
                                  int32_t  auxParam);

typedef enum SOPC_App_Com_Events {
  /* Client application events */
  SE_SESSION_ACTIVATION_FAILURE,
  SE_ACTIVATED_SESSION,
  SE_RCV_SESSION_RESPONSE,
  SE_CLOSED_SESSION,
  //  SE_RCV_PUBLIC_RESPONSE, => discovery services
  
  /* Server application events */
  SE_CLOSED_ENDPOINT,
} SOPC_App_Com_Events;

typedef enum SOPC_App_AddSpace_Events {
  /* Server application events */
  AS_READ_EVENT,
  AS_WRITE_EVENT,
} SOPC_App_AddSpace_Events;

#endif /* _sopc_services_events_h */
