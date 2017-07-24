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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sopc_services_events.h"
#include "sopc_user_app_itf.h"
#include "sopc_sc_events.h"
#include "internal_msg.h"
#include "internal_channel_endpoint.h"
#include "io_dispatch_mgr.h"

// Necessary since not managed by B model
Internal_Channel_Or_Endpoint* tmpEndpointInstance = NULL;

void SOPC_ServicesEventDispatcher(int32_t  scEvent, 
                                  uint32_t id, 
                                  void*    params, 
                                  int32_t  auxParam)
{
  //printf("Services event dispatcher: event='%d', id='%d', params='%p', auxParam='%d'\n", scEvent, id, params, auxParam);
  SOPC_Services_Event event = (SOPC_Services_Event) scEvent;
  switch(event){
  /* SC to Services events */
  case EP_SC_CONNECTED:
    //printf("EP_SC_CONNECTED\n");
    // id ==  endpoint configuration index
    // params = SOPC_SecureChannel_Config*
    // auxParam == connection Id
    // => B model entry point to add
    io_dispatch_mgr__srv_channel_connected_event(id, 
                                                 ((SOPC_SecureChannel_ConnectedConfig*) params)->configIdx, 
                                                 auxParam);
    break;
  case EP_CLOSED:
    //printf("EP_CLOSED\n");
    // id == endpoint configuration index
    // params = NULL
    // auxParam == status
    // => B model entry point to add
    SOPC_EventDispatcherManager_AddEvent(applicationEventDispatcherMgr,
                                         SOPC_AppEvent_ComEvent_Create(SE_CLOSED_ENDPOINT),
                                         id,                                        
                                         NULL,
                                         auxParam,
                                         "Endpoint is closed !");
    break;
  case SC_CONNECTED:
    //printf("SC_CONNECTED\n");
    // id == connection Id
    // params = SC config (for secure channel Id only ?) ? => TMP: NULL for now
    // auxParam == secure channel configuration index
    // => B model entry point to add
    io_dispatch_mgr__cli_channel_connected_event(auxParam, id);
    break;
  case SC_CONNECTION_TIMEOUT:
    //printf("SC_CONNECTION_TIMEOUT\n");
    // id == secure channel configuration index
    // => B model entry point to add
    break;
  case SC_DISCONNECTED:
    //printf("SC_DISCONNECTED\n");
    // id == connection Id ==> TMP: secure channel config idx
    // auxParam = status
    // => B model entry point to add
    // secure_channel_lost call !
    io_dispatch_mgr__secure_channel_lost((constants__t_channel_i) id);
    break;
  case SC_SERVICE_RCV_MSG:
    //printf("SC_SERVICE_RCV_MSG\n");
    // id ==  TMP: secure channel config idx (To be replaced by => connection Id)
    // params = message content (TBD)
    // auxParam == context
    // => B model entry point to add
    assert(NULL != params);
    io_dispatch_mgr__receive_msg((constants__t_channel_i) id,
                                 (constants__t_msg_i) params); // SOPC_Toolkit_Msg
    // params is freed by services manager
    break;

    /* App to Services events */
  case SE_OPEN_ENDPOINT:
    //printf("SE_OPEN_ENDPOINT\n");
    // id ==  endpoint configuration index
    // => B model entry point to add
    break;
  case SE_CLOSE_ENDPOINT:
    //printf("SE_CLOSE_ENDPOINT\n");
    // id ==  endpoint configuration index
    // => B model entry point to add
    break;
  case SE_ACTIVATE_SESSION:
    //printf("SE_ACTIVATE_SESSION\n");
    // id == secure channel configuration
    // params = TODO: user authentication
    break;
  case SE_SEND_SESSION_REQUEST:
    //printf("SE_SEND_SESSION_REQUEST\n");
    // id == session id
    // params = TODO: user authentication
    break;
  case SE_CLOSE_SESSION:
    //printf("SE_CLOSE_SESSION\n");
    // id == session id
    break;
  default:
    assert(FALSE);
  }
}

