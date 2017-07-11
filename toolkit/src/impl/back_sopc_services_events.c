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
#include "sopc_sc_events.h"
#include "internal_msg.h"
#include "internal_channel_endpoint.h"
#include "io_dispatch_mgr.h"

void SOPC_ServicesEventDispatcher(int32_t  scEvent, 
                                  uint32_t id, 
                                  void*    params, 
                                  int32_t  auxParam)
{
  printf("Services event dispatcher: event='%d', id='%d', params='%p', auxParam='%d'\n", scEvent, id, params, auxParam);
  SOPC_Services_Event event = (SOPC_Services_Event) scEvent;
  Internal_Channel_Or_Endpoint* chOrEnd = NULL;
  switch(event){
  /* SC to Services events */
  case EP_SC_CONNECTED:
    printf("EP_SC_CONNECTED\n");
    // id ==  endpoint configuration index
    // params = SOPC_SecureChannel_Config*
    // auxParam == secure channel Id
    // => B model entry point to add
    break;
  case EP_CLOSED:
    printf("EP_CLOSED\n");
    // id == endpoint configuration index
    // params = NULL
    // auxParam == status
    // => B model entry point to add
    break;
  case SC_CONNECTED:
    printf("SC_CONNECTED\n");
    // id == connection Id
    // params = SC config (for secure channel Id only ?) ?
    // auxParam == secure channel configuration index
    // => B model entry point to add
    io_dispatch_mgr__channel_connected_event(auxParam);
    break;
  case SC_CONNECTION_TIMEOUT:
    printf("SC_CONNECTION_TIMEOUT\n");
    // id == secure channel configuration index
    // => B model entry point to add
    break;
  case SC_DISCONNECTED:
    printf("SC_DISCONNECTED\n");
    // id == secure channel Id
    // auxParam = connection Id
    // => B model entry point to add
    // secure_channel_lost call !
    break;
  case SC_SERVICE_RCV_MSG:
    printf("SC_SERVICE_RCV_MSG\n");
    // id ==  TMP: secure channel config idx (To be replaced by => connection Id)
    // params = message content (TBD)
    // auxParam == context
    // => B model entry point to add
    assert(NULL != params);
    if(((SOPC_Toolkit_Msg*) params)->isRequest == FALSE){
      channel_mgr_bs__get_valid_channel((constants__t_channel_config_idx_i) id,
                                        (constants__t_channel_i*) &chOrEnd);
    }else{
      // TODO: tmp, must be retrieved by id => model shall take id in receive msg instead of struct
      chOrEnd = calloc(1, sizeof(Internal_Channel_Or_Endpoint));
      chOrEnd->id = id;
      chOrEnd->isChannel = FALSE;
    }
    if(chOrEnd->isChannel == FALSE){
      chOrEnd->context = (struct SOPC_RequestContext*) ((SOPC_Toolkit_Msg*) params)->optContext;
    }
    io_dispatch_mgr__receive_msg((constants__t_channel_i) chOrEnd,
                                 (constants__t_msg_i) params); // SOPC_Toolkit_Msg
    free(chOrEnd);
    break;

    /* App to Services events */
  case SE_OPEN_ENDPOINT:
    printf("SE_OPEN_ENDPOINT\n");
    // id ==  endpoint configuration index
    // => B model entry point to add
    break;
  case SE_CLOSE_ENDPOINT:
    printf("SE_CLOSE_ENDPOINT\n");
    // id ==  endpoint configuration index
    // => B model entry point to add
    break;
  case SE_ACTIVATE_SESSION:
    printf("SE_ACTIVATE_SESSION\n");
    // id == secure channel configuration
    // params = TODO: user authentication
    break;
  case SE_SEND_SESSION_REQUEST:
    printf("SE_SEND_SESSION_REQUEST\n");
    // id == session id
    // params = TODO: user authentication
    break;
  case SE_CLOSE_SESSION:
    printf("SE_CLOSE_SESSION\n");
    // id == session id
    break;
  default:
    assert(FALSE);
  }
}

