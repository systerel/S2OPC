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
#include <string.h>
#include <assert.h>

#include "util_b2c.h"

#include "message_out_bs.h"
#include "message_in_bs.h"

#include "internal_msg.h"
#include "constants_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void message_in_bs__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void message_in_bs__dealloc_msg_in(const constants__t_msg_i message_in_bs__msg){
  message_out_bs__dealloc_msg_out(message_in_bs__msg);
}

void message_in_bs__get_msg_in_type(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_msg_type * const message_in_bs__msgtype) {
  message_out_bs__get_msg_out_type(message_in_bs__msg, message_in_bs__msgtype);
}

void message_in_bs__is_valid_msg_in(
   const constants__t_msg_i message_in_bs__msg,
   t_bool * const message_in_bs__bres) {
  message_out_bs__is_valid_msg_out(message_in_bs__msg, message_in_bs__bres);
}

void message_in_bs__msg_in_memory_changed(void) {
  printf("message_in_bs__msgs_memory_changed\n");
  exit(1);   
   ;
}

void message_in_bs__read_activate_msg_user(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_user_i * const message_in_bs__user) {
  (void) message_in_bs__msg;
  (void) message_in_bs__user;
  // TODO: define anonymous user in B ? Still 1 in C implem for anym 
  *message_in_bs__user = 1;
}

void message_in_bs__read_create_session_msg_session_token(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_session_token_i * const message_in_bs__session_token) {
  message__message* msg = (message__message*) message_in_bs__msg;
  OpcUa_CreateSessionResponse* createSessionResp = (OpcUa_CreateSessionResponse*) msg->msg;

  *message_in_bs__session_token = constants__c_session_token_indet;
  *message_in_bs__session_token = &createSessionResp->AuthenticationToken;
}

void message_in_bs__read_msg_header_req_handle(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_request_handle_i * const message_in_bs__handle) {
  message__message* msg = (message__message*) message_in_bs__msg;
  if((!FALSE) == msg->isRequest){
    message__request_message* req_msg = (message__request_message*) msg->msg;
    *message_in_bs__handle = req_msg->requestHeader.RequestHandle;
  }else{
    message__response_message* resp_msg = (message__response_message*) msg->msg;
    *message_in_bs__handle = resp_msg->responseHeader.RequestHandle;
  }
}

void message_in_bs__read_msg_req_header_session_token(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_session_token_i * const message_in_bs__session_token) {
  message__message* msg = (message__message*) message_in_bs__msg;
  // Do only temporary pointer copy
  // TODO: IMPORTANT: if NULL token  => shall return an indet token !
  message__request_message* req_msg = (message__request_message*) msg->msg;
  *message_in_bs__session_token = &req_msg->requestHeader.AuthenticationToken;
}

void message_in_bs__read_msg_resp_header_service_status(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_StatusCode_i * const message_in_bs__status) {
  message__message* msg = (message__message*) message_in_bs__msg;
  message__response_message* resp_msg = (message__response_message*) msg->msg;
  if(FALSE == util_status_code__C_to_B(resp_msg->responseHeader.ServiceResult,
                                       message_in_bs__status)){
    printf("message_in_bs__read_msg_resp_header_service_status\n");
    exit(1);
  }
}
