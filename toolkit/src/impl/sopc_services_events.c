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
#include "internal_msg.h"
#include "internal_channel_endpoint.h"
#include "io_dispatch_mgr.h"

static Internal_Channel_Or_Endpoint channel_and_context;

void SOPC_ServicesEventDispatcher(int32_t  scEvent, 
                                  uint32_t id, 
                                  void*    params, 
                                  int32_t  auxParam)
{
  SOPC_Services_Event event = (SOPC_Services_Event) scEvent;
  channel_and_context.isChannel = !FALSE;
  channel_and_context.channel = NULL;
  channel_and_context.endpoint = NULL;
  channel_and_context.context = NULL;
  switch(event){
  /* SC to Services events */
  case EP_SC_CONNECTED:
    // id ==  endpoint configuration index
    // params = SOPC_SecureChannel_Config*
    // auxParam == secure channel Id
    // => B model entry point to add
    break;
  case EP_CLOSED:
    // id == endpoint configuration index
    // params = SOPC_SecureChannel_Config*
    // auxParam == endpoint configuration index
    // => B model entry point to add
    break;
  case SC_CONNECTED:
    // id == secure channel Id
    // auxParam == secure channel configuration index
    // => B model entry point to add
    break;
  case SC_CONNECTION_TIMEOUT:
    // id == secure channel configuration index
    // => B model entry point to add
    break;
  case SC_DISCONNECTED:
    // id == secure channel Id
    // => B model entry point to add
    // secure_channel_lost call !
    break;
  case SC_SERVICE_RCV_MSG:
    // id ==  secure channel id
    // params = message content (TBD)
    // auxParam == context
    // => B model entry point to add
    channel_and_context.context = (void*) auxParam; //TODO: wrong type 
    message__message* msg = malloc(sizeof(message__message));
    if(NULL != msg){
      msg->msg = params;
      // Remove next fields ?
      msg->encType = NULL; // msg type => managed by services ?
      msg->respEncType = NULL; // msg type => managed by services ?
      msg->isRequest = (!FALSE);// => managed by services ?

      io_dispatch_mgr__receive_msg((constants__t_channel_i) &channel_and_context,
                                   (constants__t_msg_i) msg);
      free(msg);
    }else{
      printf("Toolkit_BeginService: out of memory \n");
      exit(1);
    }
    break;

/* App to Services events */
  case SE_OPEN_ENDPOINT:
    // id ==  endpoint configuration index
    // => B model entry point to add
    break;
  case SE_CLOSE_ENDPOINT:
    // id ==  endpoint configuration index
    // => B model entry point to add
    break;
  case SE_ACTIVATE_SESSION:
    // id == secure channel configuration
    // params = TODO: user authentication
    break;
  case SE_SEND_SESSION_REQUEST:
    // id == session id
    // params = TODO: user authentication
    break;
  case SE_CLOSE_SESSION:
    // id == session id
    break;
  default:
    assert(FALSE);
  }
}

