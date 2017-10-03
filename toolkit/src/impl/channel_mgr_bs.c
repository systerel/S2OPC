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
#include <stdbool.h>
#include <assert.h>

#include "sopc_secure_channels_api.h"

#include "sopc_types.h"
#include "crypto_profiles.h"

#include "io_dispatch_mgr.h"
#include "channel_mgr_bs.h"

#include "util_b2c.h"

#include "sopc_toolkit_config.h"
#include "sopc_services_events.h"
#include "sopc_event_dispatcher_manager.h"

static struct {
  t_bool isClient;
  t_bool disconnecting;
  uint32_t endpointIdx; // if isClient == false
  uint32_t configIdx;
  uint32_t id; // non indet => connected
} unique_channel = {
  .isClient = false,
  .endpointIdx = constants__c_endpoint_config_idx_indet,
  .configIdx = constants__c_channel_config_idx_indet,
  .id = constants__c_channel_indet,
  .disconnecting = false
};

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void channel_mgr_bs__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void channel_mgr_bs__cli_open_secure_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__config_idx,
   t_bool * const channel_mgr_bs__bres){ 
  SOPC_SecureChannel_Config* config = NULL;
  *channel_mgr_bs__bres = false;
  if(channel_mgr_bs__config_idx != constants__c_channel_config_idx_indet){
    config = SOPC_ToolkitClient_GetSecureChannelConfig(channel_mgr_bs__config_idx);
    if(NULL != config){
        SOPC_SecureChannels_EnqueueEvent(SC_CONNECT,
                                         channel_mgr_bs__config_idx,
                                         NULL,
                                         0);
        unique_channel.isClient = true;
        unique_channel.configIdx = channel_mgr_bs__config_idx;    
        *channel_mgr_bs__bres = true;
    }
  }
}

void channel_mgr_bs__cli_set_connected_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__config_idx,
   const constants__t_channel_i channel_mgr_bs__channel,
   t_bool * const channel_mgr_bs__bres){
  *channel_mgr_bs__bres = false;
  if(channel_mgr_bs__config_idx != constants__c_channel_config_idx_indet &&
     unique_channel.isClient != false &&
     unique_channel.configIdx == (uint32_t) channel_mgr_bs__config_idx){
    unique_channel.id = channel_mgr_bs__channel;
    *channel_mgr_bs__bres = true;
  }
}

void channel_mgr_bs__cli_set_connection_timeout_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__config_idx,
   t_bool * const channel_mgr_bs__bres){
  if(channel_mgr_bs__config_idx != constants__c_channel_config_idx_indet &&
     unique_channel.configIdx == (uint32_t) channel_mgr_bs__config_idx &&
     unique_channel.id == constants__c_channel_indet){
    unique_channel.isClient = false;
    unique_channel.configIdx = constants__c_channel_config_idx_indet;
    *channel_mgr_bs__bres = true;
  }else{
    *channel_mgr_bs__bres = false;
  }
}


void channel_mgr_bs__srv_new_secure_channel(
   const constants__t_endpoint_config_idx_i channel_mgr_bs__endpoint_config_idx,
   const constants__t_channel_config_idx_i channel_mgr_bs__channel_config_idx,
   const constants__t_channel_i channel_mgr_bs__channel,
   t_bool * const channel_mgr_bs__bres){
  *channel_mgr_bs__bres = false;
  if(channel_mgr_bs__endpoint_config_idx != constants__c_endpoint_config_idx_indet &&
     channel_mgr_bs__channel_config_idx != constants__c_channel_config_idx_indet &&
     channel_mgr_bs__channel != constants__c_channel_indet &&
     unique_channel.endpointIdx == constants__c_endpoint_config_idx_indet &&
     unique_channel.id == constants__c_channel_indet &&
     unique_channel.configIdx == constants__c_channel_config_idx_indet &&
     unique_channel.isClient == false){
    unique_channel.configIdx = (uint32_t) channel_mgr_bs__channel_config_idx;
    unique_channel.id = channel_mgr_bs__channel;
    unique_channel.endpointIdx = channel_mgr_bs__endpoint_config_idx;
    *channel_mgr_bs__bres = true;
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
  if(channel_mgr_bs__channel != constants__c_channel_indet &&
     (uint32_t) channel_mgr_bs__channel == unique_channel.id){
    unique_channel.id = constants__c_channel_indet;
    unique_channel.configIdx = constants__c_channel_config_idx_indet;
    unique_channel.endpointIdx = constants__c_endpoint_config_idx_indet;
    if(unique_channel.disconnecting != false){
      unique_channel.disconnecting = false;
      SOPC_EventDispatcherManager_AddEvent(servicesEventDispatcherMgr,
                                           SE_TO_SE_SC_ALL_DISCONNECTED,
                                           0,
                                           NULL,
                                           0,
                                           "Services mgr: notify itself all SC disconnected ");
    }
  }
}

void channel_mgr_bs__send_channel_msg_buffer(
   const constants__t_channel_i channel_mgr_bs__channel,
   const constants__t_byte_buffer_i channel_mgr_bs__buffer,
   const constants__t_request_context_i channel_mgr_bs__request_context) {

  if(channel_mgr_bs__channel == (t_entier4) unique_channel.id){
      SOPC_SecureChannels_EnqueueEvent(SC_SERVICE_SND_MSG,
                                       channel_mgr_bs__channel,
                                       channel_mgr_bs__buffer,
                                       channel_mgr_bs__request_context);

  }

}

void channel_mgr_bs__is_connected_channel(
   const constants__t_channel_i channel_mgr_bs__channel,
   t_bool * const channel_mgr_bs__bres) {
  // Not null channel pointer
  *channel_mgr_bs__bres = 
    (channel_mgr_bs__channel != constants__c_channel_indet &&
     channel_mgr_bs__channel == (t_entier4) unique_channel.id);
}

void channel_mgr_bs__is_disconnecting_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__config_idx,
   t_bool * const channel_mgr_bs__bres){
  *channel_mgr_bs__bres = 
    (channel_mgr_bs__config_idx != constants__c_channel_config_idx_indet &&
     channel_mgr_bs__config_idx == (t_entier4) unique_channel.configIdx &&
     unique_channel.disconnecting != false);
}

void channel_mgr_bs__get_connected_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__channel_config_idx,
   constants__t_channel_i * const channel_mgr_bs__channel) {
  if(channel_mgr_bs__channel_config_idx != constants__c_channel_config_idx_indet &&
     channel_mgr_bs__channel_config_idx == (t_entier4) unique_channel.configIdx &&
     unique_channel.id != constants__c_channel_indet){
    *channel_mgr_bs__channel = unique_channel.id;
  }else{
    *channel_mgr_bs__channel = constants__c_channel_indet;
  }
}

void channel_mgr_bs__get_channel_info(
   const constants__t_channel_i channel_mgr_bs__channel,
   constants__t_channel_config_idx_i * const channel_mgr_bs__channel_config_idx) {
  if(channel_mgr_bs__channel != constants__c_channel_indet &&
     channel_mgr_bs__channel == (t_entier4) unique_channel.id){
    *channel_mgr_bs__channel_config_idx = unique_channel.configIdx;
  }else{
    *channel_mgr_bs__channel_config_idx = constants__c_channel_config_idx_indet;
  }
}

void channel_mgr_bs__is_client_channel(
   const constants__t_channel_i channel_mgr_bs__channel,
   t_bool * const channel_mgr_bs__bres) {
  if(channel_mgr_bs__channel != constants__c_channel_indet &&
     channel_mgr_bs__channel == (t_entier4) unique_channel.id){
    *channel_mgr_bs__bres = unique_channel.isClient;
  }else{
    assert(false);
  }
}

void channel_mgr_bs__close_all_channel(t_bool * const channel_mgr_bs__bres){
  *channel_mgr_bs__bres = false;
  if(unique_channel.id != constants__c_channel_indet){
    if((!false) == unique_channel.isClient){
        SOPC_SecureChannels_EnqueueEvent(SC_DISCONNECT,
                                         unique_channel.id,
                                         NULL,
                                         0);
      unique_channel.disconnecting = true;
      *channel_mgr_bs__bres = true;
    }
  }
}

void channel_mgr_bs__server_get_endpoint_config(
   const constants__t_channel_i channel_mgr_bs__channel,
   constants__t_endpoint_config_idx_i * const channel_mgr_bs__endpoint_config_idx){
  if(channel_mgr_bs__channel != constants__c_channel_indet &&
     channel_mgr_bs__channel == (t_entier4) unique_channel.id){
    *channel_mgr_bs__endpoint_config_idx = unique_channel.endpointIdx;
  }else{
    assert(false);
  }
}


void channel_mgr_bs__get_SecurityPolicy(
   const constants__t_channel_i channel_mgr_bs__channel,
   constants__t_SecurityPolicy * const channel_mgr_bs__secpol)
{
    SOPC_SecureChannel_Config *pSCCfg = NULL;

    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(unique_channel.configIdx);
    assert(pSCCfg != NULL);

    /* TODO: The following assert is asserted by the PRE of the operation */
    assert(channel_mgr_bs__channel == (t_entier4) unique_channel.id);

    util_channel__SecurityPolicy_C_to_B(pSCCfg->reqSecuPolicyUri, channel_mgr_bs__secpol);
}

