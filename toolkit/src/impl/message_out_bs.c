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

#include "internal_msg.h"
#include "message_out_bs.h"
#include "util_b2c.h"

#include "constants_bs.h"

#include "sopc_sc_events.h"
#include "sopc_msg_buffer.h"
#include "sopc_encoder.h"
#include "sopc_stack_csts.h"
#include "sopc_toolkit_config.h"
#include "util_discovery_services.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void message_out_bs__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void util_message_out_bs__alloc_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
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
    msg = calloc(1, sizeof(SOPC_Toolkit_Msg));
    if(NULL != msg){
      status = SOPC_Encodeable_Create(encTyp, &msg->msgStruct);
    }
  }
  if(STATUS_OK == status){
    msg->isRequest = isReq;
    msg->msgType = encTyp;
    *message_out_bs__nmsg = (constants__t_msg_i) msg;
  }else{
    *message_out_bs__nmsg = constants__c_msg_indet;
  }
}

void message_out_bs__alloc_app_req_msg_header(
   const constants__t_msg_type_i message_out_bs__msg_type,
   const constants__t_msg_i message_out_bs__req_msg,
   constants__t_msg_header_i * const message_out_bs__nmsg_header){
  // NOTHING TO DO: until body and header are really separated in implementation of messages
  *message_out_bs__nmsg_header = (constants__t_msg_header_i) message_out_bs__req_msg;
}


void message_out_bs__alloc_req_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
   constants__t_msg_header_i * const message_out_bs__nmsg_header,
   constants__t_msg_i * const message_out_bs__nmsg){
  util_message_out_bs__alloc_msg(message_out_bs__msg_type,
                                 message_out_bs__nmsg);
  *message_out_bs__nmsg_header = *message_out_bs__nmsg;
}

void message_out_bs__alloc_resp_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
   const constants__t_msg_i message_out_bs__req_msg_ctx,
   constants__t_msg_header_i * const message_out_bs__nmsg_header,
   constants__t_msg_i * const message_out_bs__nmsg){
  SOPC_Toolkit_Msg* req_msg = NULL;
  SOPC_Toolkit_Msg* resp_msg = NULL;
  util_message_out_bs__alloc_msg(message_out_bs__msg_type,
                                 message_out_bs__nmsg);
  *message_out_bs__nmsg_header = *message_out_bs__nmsg;
  if(*message_out_bs__nmsg != constants__c_msg_indet){
    req_msg = (SOPC_Toolkit_Msg*) message_out_bs__req_msg_ctx;
    resp_msg = (SOPC_Toolkit_Msg*) *message_out_bs__nmsg;
    // Copy request context into response
    resp_msg->optContext = req_msg->optContext;
    
  }
}

void message_out_bs__bless_msg_out(
   const constants__t_msg_type_i message_out_bs__msg_type,
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_msg_i message_out_bs__msg){
  /* NOTHING TO DO: in B model now message_out_bs__msg = c_msg_out now */
}

void message_out_bs__dealloc_msg_header_out(
   const constants__t_msg_header_i message_out_bs__msg_header){
  message_out_bs__dealloc_msg_out((constants__t_msg_i) message_out_bs__msg_header);
}

void message_out_bs__dealloc_msg_out(
   const constants__t_msg_i message_out_bs__msg) {
  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) message_out_bs__msg;
  if(message_out_bs__msg != constants__c_msg_indet && NULL != msg->msgType){
    if(&OpcUa_ReadResponse_EncodeableType == msg->msgType){
      /* Current implementation share the variants of the address space in the response,
         avoid deallocation of those variants */
      OpcUa_ReadResponse* readMsg = (OpcUa_ReadResponse*) msg->msgStruct;
      if(NULL != readMsg){
        free(readMsg->Results);
        readMsg->Results = NULL;
        readMsg->NoOfResults = 0;
      }
    }
    // TODO: status returned ?
    SOPC_Encodeable_Delete(msg->msgType, &msg->msgStruct);
    msg->msgStruct = NULL;
  }
}

void message_out_bs__encode_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_msg_i message_out_bs__msg,
   constants__t_byte_buffer_i * const message_out_bs__buffer){
  *message_out_bs__buffer = constants__c_byte_buffer_indet;
  SOPC_StatusCode status = STATUS_OK;
  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) message_out_bs__msg;
  SOPC_EncodeableType* msgType = NULL;
  if(NULL != msg->msgBuffer){
    status = STATUS_NOK;
  }
  if(STATUS_OK == status){
    msg->msgBuffer = SOPC_Buffer_Create(OPCUA_ENCODER_MAXMESSAGELENGTH);
    if(msg->msgBuffer == NULL){
      status = STATUS_NOK;
    }
  }
  if(STATUS_OK == status){
    status = SOPC_Buffer_SetDataLength(msg->msgBuffer, UA_SECURE_MESSAGE_HEADER_LENGTH + UA_SYMMETRIC_SECURITY_HEADER_LENGTH + UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
  }
  if(STATUS_OK == status){
    // Encodeable type: either msg_type = service fault type or it is the type provided by the msg
    if(message_out_bs__msg_type == constants__e_msg_service_fault_resp){
      msgType = &OpcUa_ServiceFault_EncodeableType;
    }else{
      msgType = msg->msgType;
    }
  }
  if(STATUS_OK == status){
    status = SOPC_Buffer_SetPosition(msg->msgBuffer, UA_SECURE_MESSAGE_HEADER_LENGTH + UA_SYMMETRIC_SECURITY_HEADER_LENGTH + UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
  }
  if(STATUS_OK == status){
    status = SOPC_EncodeMsgTypeAndBody(msg->msgBuffer, msgType, msg->msgStruct);
  }
  if(STATUS_OK == status){
    *message_out_bs__buffer = (constants__t_byte_buffer_i) msg;
  }
}

void message_out_bs__get_msg_out_type(
   const constants__t_msg_i message_out_bs__msg,
   constants__t_msg_type_i * const message_out_bs__msgtype) {
  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) message_out_bs__msg;
  util_message__get_message_type(msg->msgType,
                                 message_out_bs__msgtype);
}

void message_out_bs__is_valid_buffer_out(
   const constants__t_byte_buffer_i message_out_bs__buffer,
   t_bool * const message_out_bs__bres){
  *message_out_bs__bres = message_out_bs__buffer != constants__c_byte_buffer_indet;
}

void message_out_bs__is_valid_msg_out(
   const constants__t_msg_i message_out_bs__msg,
   t_bool * const message_out_bs__bres) {
  *message_out_bs__bres = message_out_bs__msg != constants__c_msg_indet;
}

void message_out_bs__is_valid_msg_out_header(
   const constants__t_msg_header_i message_out_bs__msg_header,
   t_bool * const message_out_bs__bres){
  *message_out_bs__bres = message_out_bs__msg_header != constants__c_msg_header_indet;
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
  OpcUa_CreateSessionResponse* createSessionResp = (OpcUa_CreateSessionResponse*) msg->msgStruct;
  SOPC_StatusCode status;
  status = SOPC_NodeId_Copy(&createSessionResp->AuthenticationToken, message_out_bs__session_token);
  assert(STATUS_OK == status);
  status = SOPC_NodeId_Copy(&createSessionResp->SessionId, message_out_bs__session_token);
  assert(STATUS_OK == status);
}

void message_out_bs__write_create_session_msg_server_endpoints(
   const constants__t_msg_i message_out_bs__req_msg,
   const constants__t_msg_i message_out_bs__resp_msg,
   const constants__t_endpoint_config_idx_i message_out_bs__endpoint_config_idx,
   constants__t_StatusCode_i * const message_out_bs__ret){

	SOPC_Toolkit_Msg* reqMsg = (SOPC_Toolkit_Msg*) message_out_bs__req_msg;
	OpcUa_CreateSessionRequest* createSessionReq = (OpcUa_CreateSessionRequest*) reqMsg->msgStruct;

	SOPC_Toolkit_Msg* respMsg = (SOPC_Toolkit_Msg*) message_out_bs__resp_msg;
	OpcUa_CreateSessionResponse* createSessionResp = (OpcUa_CreateSessionResponse*) respMsg->msgStruct;

	createSessionResp->RevisedSessionTimeout = OPCUA_SESSION_TIMEOUT;

	*message_out_bs__ret = build_endPoints_Descriptions(message_out_bs__endpoint_config_idx,
			&createSessionReq->EndpointUrl,
			(uint32_t*)&createSessionResp->NoOfServerEndpoints,
			&createSessionResp->ServerEndpoints);
}

void message_out_bs__write_msg_out_header_req_handle(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_request_handle_i message_out_bs__req_handle) {
  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) message_out_bs__msg;
  if((!FALSE) == msg->isRequest){
    message__request_message* req_msg = (message__request_message*) msg->msgStruct;
    req_msg->requestHeader.RequestHandle = message_out_bs__req_handle;
  }else{
    message__response_message* resp_msg = (message__response_message*) msg->msgStruct;
    resp_msg->responseHeader.RequestHandle = message_out_bs__req_handle;
  }
}

void message_out_bs__write_msg_out_header_session_token(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_session_token_i message_out_bs__session_token) {
  SOPC_Toolkit_Msg* msg = (SOPC_Toolkit_Msg*) message_out_bs__msg;
  SOPC_NodeId* authToken = (SOPC_NodeId*) message_out_bs__session_token;
  assert(msg->isRequest == (!FALSE));
  message__request_message* req_msg = (message__request_message*) msg->msgStruct;
  
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
  message__response_message* resp_msg = (message__response_message*) msg->msgStruct;
  resp_msg->responseHeader.ServiceResult = status;
}

