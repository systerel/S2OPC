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

static int gevent = 0;
static int gconnected = 0;

static int verbose = FALSE;

static Internal_Channel_Or_Endpoint* unique_channel_or_endpoint = NULL;

SOPC_StatusCode util_channel_mgr_bs__Channel_ConnectionEvent_Callback(SOPC_Channel       channel,
                                                                      void*              callbackData,
                                                                      SOPC_Channel_Event event,
                                                                      SOPC_StatusCode    status)
{
  (void) callbackData;
  (void) channel;
  char* cevent = NULL;
  switch(event){
  case SOPC_ChannelEvent_Invalid:
    cevent = "SOPC_ChannelEvent_Invalid";
    break;
  case SOPC_ChannelEvent_Connected:
    cevent = "SOPC_ChannelEvent_Connected";
    gconnected = 1;
    break;
  case SOPC_ChannelEvent_Disconnected:
    cevent = "SOPC_ChannelEvent_Disconnected";
    break;
  }
  printf(">>channel_mgr: Connection event: channel event '%s' and status '%x'\n", cevent, status);
  gevent = 1;
  return 0;
}

SOPC_StatusCode util_channel_mgr_bs__receiveResponse(SOPC_Channel         channel,
                                                  void*                response,
                                                  SOPC_EncodeableType* responseType,
                                                  void*                cbData,
                                                  SOPC_StatusCode      status){
  if(FALSE != verbose){
    printf(">>channel_mgr: Received a response !\n");
  }
  message__message* msg = NULL;
  if(NULL != cbData && NULL != response && NULL != responseType){
    msg = malloc(sizeof(message__message));
    if(NULL != msg){
      msg->msg = response;
      msg->encType = responseType;
      msg->isRequest = FALSE;
      io_dispatch_mgr__receive_msg((constants__t_channel_i) cbData, msg);
      responseType->Clear(response);
      free(response);
      free(msg);
      return STATUS_OK;
    }else{
      return STATUS_NOK;
    }
  }
  return STATUS_INVALID_PARAMETERS;
}

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void channel_mgr_bs__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void channel_mgr_bs__open_secure_channel(
   const constants__t_endpoint_i channel_mgr_bs__endpoint,
   constants__t_channel_i * const channel_mgr_bs__nchannel) {
  assert(channel_mgr_bs__endpoint == constants__c_endpoint_indet);
  // Policy security:
  char* pRequestedSecurityPolicyUri = SecurityPolicy_None_URI;
  OpcUa_MessageSecurityMode messageSecurityMode = OpcUa_MessageSecurityMode_None;

  SOPC_StatusCode status = STATUS_OK;
  
  if(unique_channel_or_endpoint == NULL){
    unique_channel_or_endpoint = malloc(sizeof(Internal_Channel_Or_Endpoint));
    unique_channel_or_endpoint->isChannel = (!FALSE);
    unique_channel_or_endpoint->channel = NULL;
    unique_channel_or_endpoint->endpoint = NULL;
    unique_channel_or_endpoint->context = NULL;

    if(unique_channel_or_endpoint == NULL){
      *channel_mgr_bs__nchannel = constants__c_channel_indet;
      status = STATUS_NOK;
    }

    if(STATUS_OK == status){
      status = SOPC_Channel_Create(&unique_channel_or_endpoint->channel, SOPC_ChannelSerializer_Binary);
    }

    if(STATUS_OK == status){
      status = SOPC_Channel_Connect(unique_channel_or_endpoint->channel,
                                    ENDPOINT_URL,
                                    NULL,                      /* Client Certificate       */
                                    NULL,                     /* Private Key              */
                                    NULL,                      /* Server Certificate       */
                                    NULL,                          /* PKI Config               */
                                    pRequestedSecurityPolicyUri, /* Request secu policy */
                                    5,                            /* Request lifetime */
                                    messageSecurityMode,          /* Message secu mode */
                                    5000,                           /* Network timeout */
                                    util_channel_mgr_bs__Channel_ConnectionEvent_Callback,
                                    unique_channel_or_endpoint);                        /* Connect Callback Data   */
      if(STATUS_OK != status){
        printf(">>channel_mgr: Failed to start connection to server\n");
      }else{
        printf(">>channel_mgr: Establishing connection to server...\n");
      }
    }

    if(gconnected == 0 || STATUS_OK != status){
      SOPC_Channel_Delete(&unique_channel_or_endpoint->channel);
      free(unique_channel_or_endpoint);
      unique_channel_or_endpoint = NULL;
    }else{
      *channel_mgr_bs__nchannel = unique_channel_or_endpoint;
    }

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
  exit(1);
   ;
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

  message__message* msg = (message__message*) channel_mgr_bs__msg;
  Internal_Channel_Or_Endpoint* channelOrEndpoint = (Internal_Channel_Or_Endpoint*) channel_mgr_bs__channel;
  SOPC_StatusCode status = STATUS_OK;
  // Verif on channel state ?
  if(msg->isRequest == (!FALSE) && channelOrEndpoint->isChannel == (!FALSE)){
    status = SOPC_Channel_BeginInvokeService(channelOrEndpoint->channel,
                                             NULL,
                                             msg->msg,
                                             msg->encType,
                                             msg->respEncType,
                                             util_channel_mgr_bs__receiveResponse,
                                             channel_mgr_bs__channel);
  }else if(msg->isRequest == FALSE && channelOrEndpoint->isChannel == FALSE){
    status = SOPC_Endpoint_SendResponse(channelOrEndpoint->endpoint,
                                        msg->encType,
                                        msg->msg,
                                        &channelOrEndpoint->context);
  }else{
    printf("channel_mgr_bs__send_channel_msg\n");
    exit(1);
  }
  if(STATUS_OK == status){
    *channel_mgr_bs__ret = constants__e_sc_ok; 
    if(FALSE != verbose){
      printf(">>channel_mgr: sent a message\n");
    }
  }else{
    *channel_mgr_bs__ret = constants__e_sc_bad_secure_channel_closed; 
  }
}

void channel_mgr_bs__is_valid_channel(
   const constants__t_channel_i channel_mgr_bs__channel,
   t_bool * const channel_mgr_bs__bres) {
  // Not null channel pointer
  *channel_mgr_bs__bres = channel_mgr_bs__channel != 0;
}

void channel_mgr_bs__get_valid_channel(
   const constants__t_endpoint_i channel_mgr_bs__endpoint,
   constants__t_channel_i * const channel_mgr_bs__channel) {
  // Always fail: no channel management
  *channel_mgr_bs__channel = constants__c_channel_indet;
}

void channel_mgr_bs__get_channel_info(
   const constants__t_channel_i channel_mgr_bs__channel,
   constants__t_endpoint_i * const channel_mgr_bs__endpoint) {
  // No endpoint management
  *channel_mgr_bs__endpoint = constants__c_endpoint_indet;
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
        SOPC_Channel_Delete(&unique_channel_or_endpoint->channel);
        free(unique_channel_or_endpoint);
      }
  }
}
