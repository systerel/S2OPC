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
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "sopc_services_events.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_user_app_itf.h"
#include "sopc_sc_events.h"
#include "io_dispatch_mgr.h"

void SOPC_ServicesEventDispatcher(int32_t  scEvent, 
                                  uint32_t id, 
                                  void*    params, 
                                  uint32_t auxParam)
{
  //printf("Services event dispatcher: event='%d', id='%d', params='%p', auxParam='%d'\n", scEvent, id, params, auxParam);
  SOPC_Services_Event event = (SOPC_Services_Event) scEvent;
  SOPC_StatusCode status = STATUS_OK;
  SOPC_Endpoint_Config* epConfig = NULL;
  constants__t_StatusCode_i sCode = constants__e_sc_ok;
  bool bres = false;
  switch(event){
  /* SC to Services events */
  case SC_TO_SE_EP_SC_CONNECTED:
    //printf("SC_TO_SE_EP_SC_CONNECTED\n");
    // id ==  endpoint configuration index
    // params = SOPC_SecureChannel_Config*
    // auxParam == connection Id
    // => B model entry point to add

    // Note: temporarly only accept SC config idx == EP config idx
    assert(((SOPC_SecureChannel_ConnectedConfig*) params)->configIdx ==
           SOPC_ToolkitClient_AddSecureChannelConfig (((SOPC_SecureChannel_ConnectedConfig*) params)->config));
    io_dispatch_mgr__server_channel_connected_event(id, 
                                                    ((SOPC_SecureChannel_ConnectedConfig*) params)->configIdx, 
                                                    auxParam);
    break;
  case SC_TO_SE_EP_CLOSED:
    //printf("SC_TO_SE_EPCLOSED\n");
    // id == endpoint configuration index
    // params = NULL
    // auxParam == status
    // => B model entry point to add
    status = SOPC_EventDispatcherManager_AddEvent(applicationEventDispatcherMgr,
                                                  SOPC_AppEvent_ComEvent_Create(SE_CLOSED_ENDPOINT),
                                                  id,                                        
                                                  NULL,
                                                  auxParam,
                                                  "Endpoint is closed !");
    break;
  case SC_TO_SE_SC_CONNECTED:
    //printf("SC_TO_SE_SC_CONNECTED\n");
    // id == connection Id
    // params = SC config (for secure channel Id only ?) ? => TMP: NULL for now
    // auxParam == secure channel configuration index
    // => B model entry point to add
    io_dispatch_mgr__client_channel_connected_event(auxParam, id);
    break;
  case SC_TO_SE_SC_CONNECTION_TIMEOUT:
    //printf("SC_TO_SE_SC_CONNECTION_TIMEOUT\n");
    // id == secure channel configuration index
    // => B model entry point to add
    // TODO: treat with a timer reconnection attempt (limited attempts ?) => no change of state (connecting) until all attempts terminated
    io_dispatch_mgr__client_secure_channel_timeout(id);
    break;
  case SC_TO_SE_SC_DISCONNECTED:
    //printf("SC_TO_SE_SC_DISCONNECTED\n");
    // id == connection Id ==> TMP: secure channel config idx
    // auxParam = status
    // => B model entry point to add
    // secure_channel_lost call !
    io_dispatch_mgr__secure_channel_lost((constants__t_channel_i) id);
    break;
  case SE_TO_SE_SC_ALL_DISCONNECTED:
    //printf("SE_TO_SE_SC_ALL_DISCONNECTED\n");
    // Call directly toolkit configuration callback
    SOPC_Internal_AllClientSecureChannelsDisconnected();
    break;

  case SE_TO_SE_ACTIVATE_ORPHANED_SESSION:
    //printf("SE_TO_SE_ACTIVATE_ORPHANED_SESSION\n");
    io_dispatch_mgr__internal_client_activate_orphaned_session((constants__t_session_i) id,
                                                               (constants__t_channel_config_idx_i) auxParam);
    break;
  case SE_TO_SE_ACTIVATE_SESSION:
    //printf("SE_TO_SE_ACTIVATE_SESSION\n");
    io_dispatch_mgr__client_reactivate_session_new_user((constants__t_session_i) id,
                                                        (constants__t_user_i) auxParam,
                                                        &sCode);
    if(sCode != constants__e_sc_ok){
      // TODO: log error
    }
    break;
  case SE_TO_SE_CREATE_SESSION:
    //printf("SE_TO_SE_CREATE_SESSION\n");
    io_dispatch_mgr__internal_client_create_session((constants__t_session_i) id,
                                                    (constants__t_channel_config_idx_i) auxParam);
    break;

  case SC_TO_SE_SC_SERVICE_RCV_MSG:
    //printf("SC_TO_SE_SC_SERVICE_RCV_MSG\n");
    // id ==  TMP: secure channel config idx (To be replaced by => connection Id)
    // params = message content (TBD)
    // auxParam == context
    // => B model entry point to add
    assert(NULL != params);
    SOPC_SecureChannel_OpcUaMsg* msg = (SOPC_SecureChannel_OpcUaMsg*) params;
    io_dispatch_mgr__receive_msg_buffer((constants__t_channel_i) id,
                                        (constants__t_byte_buffer_i) msg->msgBuffer,
                                        (constants__t_request_context_i) msg->optContext);
    // OpcUaMsg container could be freed
    free(msg);
    // params is freed by services manager
    break;

    /* App to Services events */
  case APP_TO_SE_OPEN_ENDPOINT:
    //printf("APP_TO_SE_OPEN_ENDPOINT\n");
    // id ==  endpoint configuration index
    // => B model entry point to add
    epConfig = SOPC_ToolkitServer_GetEndpointConfig(id);
    if(NULL == epConfig){
      status = STATUS_INVALID_PARAMETERS;
    }else{
      status = SOPC_EventDispatcherManager_AddEvent(scEventDispatcherMgr,
                                                    SE_TO_SC_EP_OPEN,
                                                    id,
                                                    epConfig,
                                                    0,
                                                    "Endpoint opening !");
    }
    break;
  case APP_TO_SE_CLOSE_ENDPOINT:
    //printf("APP_TO_SE_CLOSE_ENDPOINT\n");
    // id ==  endpoint configuration index
    // => B model entry point to add
    epConfig = SOPC_ToolkitServer_GetEndpointConfig(id);
    if(NULL == epConfig){
      status = STATUS_INVALID_PARAMETERS;
    }else{
      status = SOPC_EventDispatcherManager_AddEvent(scEventDispatcherMgr,
                                                    SE_TO_SC_EP_CLOSE,
                                                    id,
                                                    epConfig,
                                                    0,
                                                    "Endpoint closing !");
    }
    break;
  case APP_TO_SE_ACTIVATE_SESSION:
    //printf("APP_TO_SE_ACTIVATE_SESSION\n");
    // id == secure channel configuration
    // params = TODO: user authentication
    // auxParam = TMP: user integer
    io_dispatch_mgr__client_activate_new_session(id,
                                                 auxParam,
                                                 &bres);
    if(bres == false){
      // TODO: log
    }
    break;
  case APP_TO_SE_SEND_SESSION_REQUEST:
    //printf("APP_TO_SE_SEND_SESSION_REQUEST\n");
    // id == session id
    // params = request
    io_dispatch_mgr__client_send_service_request(id,
                                                 params,
                                                 &sCode);
    if(sCode != constants__e_sc_ok){
      status = STATUS_NOK;
    }
    break;
  case APP_TO_SE_CLOSE_SESSION:
    //printf("APP_TO_SE_CLOSE_SESSION\n");
    // id == session id
    io_dispatch_mgr__client_send_close_session_request(id,
                                                       &sCode);
    if(sCode != constants__e_sc_ok){
      printf("WARNING: Problem while closing the session on client demand\n");
    }
    break;
  case APP_TO_SE_CLOSE_ALL_CONNECTIONS:
    //printf("APP_TO_SE_CLOSE_ALL_CONNECTIONS\n");
    io_dispatch_mgr__close_all_active_connections(&bres);
    if(bres == false){
      // All connections considered closed: simulate new service event
      SOPC_ServicesEventDispatcher(SE_TO_SE_SC_ALL_DISCONNECTED,
                                   id,
                                   params,
                                   auxParam);
    }
    break;
  default:
    assert(false);
  }
  assert(STATUS_OK == status);
}

void SOPC_Services_EnqueueEvent(SOPC_Services_Event seEvent,
                                uint32_t            id,
                                void*               params,
                                uint32_t            auxParam,
                                const char*         reason){
    if(servicesEventDispatcherMgr != NULL){
        SOPC_EventDispatcherManager_AddEvent(servicesEventDispatcherMgr,
                                             seEvent,
                                             id,
                                             params,
                                             auxParam,
                                             reason);
    }
}
