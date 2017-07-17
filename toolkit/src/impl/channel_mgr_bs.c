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

/*------------------------
   Exported Declarations
  ------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sopc_types.h"
#include "crypto_profiles.h"
#include "sopc_channel.h"
#include "sopc_endpoint.h"

#include "io_dispatch_mgr.h"

#include "channel_mgr_bs.h"

#include "internal_channel_endpoint.h"
#include "internal_msg.h"
#include "config_toolkit.h"

#include "sopc_toolkit_config.h"
#include "sopc_event_dispatcher_manager.h"

static Internal_Channel_Or_Endpoint* unique_channel_or_endpoint = NULL;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void channel_mgr_bs__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void channel_mgr_bs__open_secure_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__channel_config_idx,
   constants__t_channel_i * const channel_mgr_bs__nchannel,
   t_bool * const channel_mgr_bs__is_connected) {
  /* TODO: not synchronous anymore ! 
     parameters needed : SC configuration index => same level as endpoint
   */
   if(NULL == unique_channel_or_endpoint){
      SOPC_EventDispatcherManager_AddEvent(scEventDispatcherMgr,
                                           SC_CONNECT,
                                           channel_mgr_bs__channel_config_idx,
                                           (void*) SOPC_ToolkitConfig_GetSecureChannelConfig(channel_mgr_bs__channel_config_idx),
                                           0,
                                           "Services request connection to a SC !");
      // TODO: async response management
      unique_channel_or_endpoint = malloc(sizeof(Internal_Channel_Or_Endpoint));
      unique_channel_or_endpoint->isChannel = (!FALSE);
      unique_channel_or_endpoint->id = channel_mgr_bs__channel_config_idx; // TMP: use SC config index as id
     
      *channel_mgr_bs__nchannel = unique_channel_or_endpoint;
      *channel_mgr_bs__is_connected = FALSE;
   }
}

void channel_mgr_bs__close_secure_channel(
   const constants__t_channel_i channel_mgr_bs__channel) {
  printf("channel_mgr_bs__close_secure_channel\n");
  exit(1);
   ;
}

void channel_mgr_bs__channel_lost(
   const constants__t_channel_i channel_mgr_bs__channel) {
  printf("channel_mgr_bs__channel_lost\n");
  free(unique_channel_or_endpoint);
  unique_channel_or_endpoint = NULL;
}

void channel_mgr_bs__receive_sc_msg(
   const constants__t_channel_i channel_mgr_bs__channel,
   const constants__t_msg_i channel_mgr_bs__msg) {
  printf("channel_mgr_bs__receive_sc_msg\n");
  exit(1);
   ;
}

void channel_mgr_bs__receive_hello_msg(
   const constants__t_msg_i channel_mgr_bs__msg) {
  printf("channel_mgr_bs__receive_hello_msg\n");
  exit(1);
   ;
}

void channel_mgr_bs__receive_channel_msg(
   const constants__t_channel_i channel_mgr_bs__channel,
   const constants__t_msg_i channel_mgr_bs__msg) {
  printf("channel_mgr_bs__receive_channel_msg\n");
  exit(1);
   ;
}

void channel_mgr_bs__send_channel_msg(
   const constants__t_channel_i channel_mgr_bs__channel,
   const constants__t_msg_i channel_mgr_bs__msg,
   constants__t_StatusCode_i * const channel_mgr_bs__ret) {

  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) channel_mgr_bs__msg;
  Internal_Channel_Or_Endpoint* chOrEnd = (Internal_Channel_Or_Endpoint*) channel_mgr_bs__channel;
  
  // Verif on channel state ?
  if(msg->isRequest == (!FALSE)){
    SOPC_EventDispatcherManager_AddEvent(scEventDispatcherMgr,
                                         SC_SERVICE_SND_MSG,
                                         chOrEnd->id,
                                         msg,
                                         chOrEnd->id, // TMP: shall be the sc config index !
                                         "Services sends a message on (client) channel !");
  }else if(msg->isRequest == FALSE){
    if(chOrEnd->isChannel == FALSE){
      msg->optContext = (void*) chOrEnd->context;
    }
    SOPC_EventDispatcherManager_AddEvent(scEventDispatcherMgr,
                                         EP_SC_SERVICE_SND_MSG,
                                         chOrEnd->id, // Shall be endpoint config index
                                         msg,
                                         0,
                                         "Services sends a message on (server) channel !");
  }else{
    printf("channel_mgr_bs__send_channel_msg\n");
    exit(1);
  }
  // TODO: no status to retrieve on asyn operation !
  *channel_mgr_bs__ret = constants__e_sc_ok; 
}

void channel_mgr_bs__is_valid_channel(
   const constants__t_channel_i channel_mgr_bs__channel,
   t_bool * const channel_mgr_bs__bres) {
  // Not null channel pointer
  *channel_mgr_bs__bres = channel_mgr_bs__channel != constants__c_channel_indet;
}

void channel_mgr_bs__get_valid_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__channel_config_idx,
   constants__t_channel_i * const channel_mgr_bs__channel) {
  if(NULL != unique_channel_or_endpoint && 
     channel_mgr_bs__channel_config_idx == (t_entier4) unique_channel_or_endpoint->id){
    *channel_mgr_bs__channel = unique_channel_or_endpoint;
  }else{
    *channel_mgr_bs__channel = constants__c_channel_indet;
  }
  // Always fail: no channel management
}

void channel_mgr_bs__get_channel_info(
   const constants__t_channel_i channel_mgr_bs__channel,
   constants__t_channel_config_idx_i * const channel_mgr_bs__channel_config_idx) {
  // No endpoint management
  *channel_mgr_bs__channel_config_idx = constants__c_channel_config_idx_indet;
}

void channel_mgr_bs__is_client_channel(
   const constants__t_channel_i channel_mgr_bs__channel,
   t_bool * const channel_mgr_bs__bres) {
  Internal_Channel_Or_Endpoint* channel_or_endpoint = (Internal_Channel_Or_Endpoint*) channel_mgr_bs__channel;
  *channel_mgr_bs__bres = channel_or_endpoint->isChannel;
}

void channel_mgr_bs__close_all_channel(void){
  if(unique_channel_or_endpoint != NULL){
    if((!FALSE) == unique_channel_or_endpoint->isChannel){
      SOPC_EventDispatcherManager_AddEvent(scEventDispatcherMgr,
                                           SC_DISCONNECT,
                                           unique_channel_or_endpoint->id, // Shall be endpoint config index
                                           NULL,
                                           0,
                                           "Close all channel !");
      free(unique_channel_or_endpoint);
      unique_channel_or_endpoint = NULL;
    }
  }
}
