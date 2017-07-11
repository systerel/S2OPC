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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sopc_encodeable.h"

#include "message_out_bs.h"
#include "util_b2c.h"

#include "internal_msg.h"
#include "constants_bs.h"

#include "sopc_sc_events.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void message_out_bs__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void message_out_bs__alloc_msg(
   const constants__t_msg_type message_out_bs__msg_type,
   constants__t_msg_i * const message_out_bs__nmsg) {

  SOPC_Toolkit_Msg* msg = NULL;
  SOPC_StatusCode status = STATUS_NOK;
 
  SOPC_EncodeableType* encTyp = NULL;
  SOPC_EncodeableType* reqEncTyp = NULL;
  SOPC_EncodeableType* respEncTyp = NULL;
  t_bool isReq = FALSE; 
  util_message__get_encodeable_type(message_out_bs__msg_type,
                                    &reqEncTyp,
                                    &respEncTyp,
                                    &isReq);

  if(NULL != reqEncTyp && NULL != respEncTyp){
    if(isReq == (!FALSE)){
      encTyp = reqEncTyp;
    }else{
      encTyp = respEncTyp;
    }
  }
  if(NULL != encTyp){
    msg = malloc(sizeof(SOPC_Toolkit_Msg));
    if(NULL != msg){
      memset(msg, 0, sizeof(SOPC_Toolkit_Msg));
      status = SOPC_Encodeable_Create(encTyp, &msg->msg);
    }
  }
  if(STATUS_OK == status){
    msg->isRequest = isReq;
    msg->encType = encTyp;
    if(isReq == (!FALSE)){
      msg->respEncType = respEncTyp;
    }
    *message_out_bs__nmsg = (constants__t_msg_i) msg;
  }else{
    *message_out_bs__nmsg = constants__c_msg_indet;
  }
}

void message_out_bs__bless_msg_out(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_msg_type message_out_bs__msg_type){
  /* NOTHING TO DO: in B model now message_out_bs__msg = c_msg_out now */
}


void message_out_bs__dealloc_msg_out(
   const constants__t_msg_i message_out_bs__msg) {
  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) message_out_bs__msg;
  if(message_out_bs__msg != constants__c_msg_indet && NULL != msg->encType){
    if(&OpcUa_ReadResponse_EncodeableType == msg->encType){
      /* Current implementation share the variants of the address space in the response,
         avoid deallocation of those variants */
      OpcUa_ReadResponse* readMsg = (OpcUa_ReadResponse*) msg->msg;
      if(NULL != readMsg){
        free(readMsg->Results);
        readMsg->Results = NULL;
        readMsg->NoOfResults = 0;
      }
    }
    // TODO: status returned ?
    SOPC_Encodeable_Delete(msg->encType, &msg->msg);
    free(msg);
  }
}

void message_out_bs__get_msg_out_type(
   const constants__t_msg_i message_out_bs__msg,
   constants__t_msg_type * const message_out_bs__msgtype) {
  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) message_out_bs__msg;
  util_message__get_message_type(msg->encType,
                                 message_out_bs__msgtype);
}

void message_out_bs__is_valid_msg_out(
   const constants__t_msg_i message_out_bs__msg,
   t_bool * const message_out_bs__bres) {
  *message_out_bs__bres = message_out_bs__msg != constants__c_msg_indet;
}

void message_out_bs__msg_out_memory_changed(void) {
  printf("message_out_bs__msg_out_memory_changed\n");
  exit(1);   
   ;
}

void message_out_bs__write_activate_msg_user(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_user_i message_out_bs__user) {
  assert(message_out_bs__user == 1); // anonymous user only for now
}

void message_out_bs__write_create_session_msg_session_token(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_session_token_i message_out_bs__session_token) {
  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) message_out_bs__msg;
  OpcUa_CreateSessionResponse* createSessionResp = (OpcUa_CreateSessionResponse*) msg->msg;
  SOPC_StatusCode status;
  status = SOPC_NodeId_Copy(&createSessionResp->AuthenticationToken, message_out_bs__session_token);
  assert(STATUS_OK == status);
  status = SOPC_NodeId_Copy(&createSessionResp->SessionId, message_out_bs__session_token);
  assert(STATUS_OK == status);
}

void message_out_bs__write_msg_out_header_req_handle(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_request_handle_i message_out_bs__req_handle) {
  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) message_out_bs__msg;
  if((!FALSE) == msg->isRequest){
    message__request_message* req_msg = (message__request_message*) msg->msg;
    req_msg->requestHeader.RequestHandle = message_out_bs__req_handle;
  }else{
    message__response_message* resp_msg = (message__response_message*) msg->msg;
    resp_msg->responseHeader.RequestHandle = message_out_bs__req_handle;
  }
}

void message_out_bs__write_msg_out_header_session_token(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_session_token_i message_out_bs__session_token) {
  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) message_out_bs__msg;
  SOPC_NodeId* authToken = (SOPC_NodeId*) message_out_bs__session_token;
  assert(msg->isRequest == (!FALSE));
  message__request_message* req_msg = (message__request_message*) msg->msg;
  
  if(STATUS_OK != SOPC_NodeId_Copy(&req_msg->requestHeader.AuthenticationToken, authToken)){
    printf("message_out_bs__write_msg_header_session_token\n");
    exit(1);
  }
}

void message_out_bs__write_msg_resp_header_service_status(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_StatusCode_i message_out_bs__status_code) {
  SOPC_StatusCode status = STATUS_NOK;
  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) message_out_bs__msg;
  util_status_code__B_to_C(message_out_bs__status_code,
                           &status);
  message__response_message* resp_msg = (message__response_message*) msg->msg;
  resp_msg->responseHeader.ServiceResult = status;
}

