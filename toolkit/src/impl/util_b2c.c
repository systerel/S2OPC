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

#include "util_b2c.h"

#include "sopc_types.h"

void util_message__get_encodeable_type(const constants__t_msg_type_i message__msg_type,
                                       SOPC_EncodeableType** reqEncType,
                                       SOPC_EncodeableType** respEncType,
                                       t_bool* isRequest) {
  switch(message__msg_type){
  case constants__e_msg_session_create_req:
    *reqEncType = &OpcUa_CreateSessionRequest_EncodeableType;
    *respEncType = &OpcUa_CreateSessionResponse_EncodeableType;
    *isRequest = (!FALSE);
    break;
  case constants__e_msg_session_create_resp:
    *reqEncType = &OpcUa_CreateSessionRequest_EncodeableType;
    *respEncType = &OpcUa_CreateSessionResponse_EncodeableType;
    break;
  case constants__e_msg_session_activate_req:
    *reqEncType = &OpcUa_ActivateSessionRequest_EncodeableType;
    *respEncType = &OpcUa_ActivateSessionResponse_EncodeableType;
    *isRequest = (!FALSE);
    break;
  case constants__e_msg_session_activate_resp:
    *reqEncType = &OpcUa_ActivateSessionRequest_EncodeableType;
    *respEncType = &OpcUa_ActivateSessionResponse_EncodeableType;
    break;
  case constants__e_msg_session_close_req:
    *reqEncType = &OpcUa_CloseSessionRequest_EncodeableType;
    *respEncType = &OpcUa_CloseSessionResponse_EncodeableType;
    *isRequest = (!FALSE);
    break;
  case constants__e_msg_session_close_resp:
    *reqEncType = &OpcUa_CloseSessionRequest_EncodeableType;
    *respEncType = &OpcUa_CloseSessionResponse_EncodeableType;
    break;
  case constants__e_msg_session_read_req:
    *reqEncType = &OpcUa_ReadRequest_EncodeableType;
    *respEncType = &OpcUa_ReadResponse_EncodeableType;
    break;
  case constants__e_msg_session_read_resp:
    *reqEncType = &OpcUa_ReadRequest_EncodeableType;
    *respEncType = &OpcUa_ReadResponse_EncodeableType;
    break;
  case constants__e_msg_session_write_req:
    *reqEncType = &OpcUa_WriteRequest_EncodeableType;
    *respEncType = &OpcUa_WriteResponse_EncodeableType;
    break;
  case constants__e_msg_session_write_resp:
    *reqEncType = &OpcUa_WriteRequest_EncodeableType;
    *respEncType = &OpcUa_WriteResponse_EncodeableType;
    break;
  case constants__e_msg_public_service_req:
  case constants__e_msg_public_service_resp:
  default:
    printf("message__alloc_msg: not implemented message type required\n");
    exit(1);   
    ;    
  }
}

void util_message__get_message_type(SOPC_EncodeableType* encType,
                                    constants__t_msg_type_i* message__msg_type)
{
  if(encType == &OpcUa_CreateSessionRequest_EncodeableType){
    *message__msg_type = constants__e_msg_session_create_req;
  }else if(encType == &OpcUa_CreateSessionResponse_EncodeableType){
    *message__msg_type = constants__e_msg_session_create_resp;
  }else if(encType == &OpcUa_ActivateSessionRequest_EncodeableType){
    *message__msg_type = constants__e_msg_session_activate_req;
  }else if(encType == &OpcUa_ActivateSessionResponse_EncodeableType){
    *message__msg_type = constants__e_msg_session_activate_resp;
  }else if(encType == &OpcUa_CloseSessionRequest_EncodeableType){
    *message__msg_type = constants__e_msg_session_close_req;
  }else if(encType == &OpcUa_CloseSessionResponse_EncodeableType){
    *message__msg_type = constants__e_msg_session_close_resp;
  }else if(encType == &OpcUa_ReadRequest_EncodeableType){
    *message__msg_type = constants__e_msg_session_read_req;
  }else if(encType == &OpcUa_ReadResponse_EncodeableType){
    *message__msg_type = constants__e_msg_session_read_resp;
  }else if(encType == &OpcUa_WriteRequest_EncodeableType){
    *message__msg_type = constants__e_msg_session_write_req;
  }else if(encType == &OpcUa_WriteResponse_EncodeableType){
    *message__msg_type = constants__e_msg_session_write_resp;
  }else{
    printf("util_message__get_message_type\n");
    exit(1);
  }
}

void util_status_code__B_to_C(constants__t_StatusCode_i bstatus,
                              SOPC_StatusCode* status){
  switch(bstatus){
  case constants__e_sc_ok:
    *status = STATUS_OK;
    break;
  case constants__e_sc_nok:
    *status = STATUS_NOK;
    break;
  case constants__e_sc_bad_secure_channel_id_invalid:
    *status = OpcUa_BadSecureChannelIdInvalid;
    break;
  case constants__e_sc_bad_secure_channel_closed:
    *status = OpcUa_BadSecureChannelClosed;
    break;
  case constants__e_sc_bad_connection_closed:
    *status = OpcUa_BadConnectionClosed;
    break;
  case constants__e_sc_bad_invalid_state:
    *status = OpcUa_BadInvalidState;
    break;
  case constants__e_sc_bad_session_id_invalid:
    *status = OpcUa_BadSessionIdInvalid;
    break;
  case constants__e_sc_bad_session_closed:
    *status = OpcUa_BadSessionClosed;
    break;
  case constants__e_sc_bad_identity_token_invalid:
    *status = OpcUa_BadIdentityTokenInvalid;
    break;
  case constants__e_sc_bad_encoding_error:
    *status = OpcUa_BadEncodingError;
    break;
  case constants__e_sc_bad_invalid_argument:
    *status = OpcUa_BadInvalidArgument;
    break;
  case constants__e_sc_bad_unexpected_error:
    *status = OpcUa_BadUnexpectedError;
    break;
  case constants__e_sc_bad_out_of_memory:
    *status = OpcUa_BadOutOfMemory;
    break;
  default:
    printf("util_message__B_to_C_status_code\n");
    exit(1);
  }
}

t_bool util_status_code__C_to_B(SOPC_StatusCode status,
                                constants__t_StatusCode_i* bstatus){
  t_bool success = (!FALSE);
  switch(status){
  case STATUS_OK:
    *bstatus = constants__e_sc_ok;
    break;
  case OpcUa_BadSecureChannelClosed:
    *bstatus = constants__e_sc_bad_secure_channel_closed;
    break;
  case OpcUa_BadConnectionClosed:
    *bstatus = constants__e_sc_bad_connection_closed;
    break;
  case OpcUa_BadInvalidState:
    *bstatus = constants__e_sc_bad_invalid_state;
    break;
  case OpcUa_BadSessionIdInvalid:
    *bstatus = constants__e_sc_bad_session_id_invalid;
    break;
  case OpcUa_BadSessionClosed:
    *bstatus = constants__e_sc_bad_session_closed;
    break;
  case OpcUa_BadIdentityTokenInvalid:
    *bstatus = constants__e_sc_bad_identity_token_invalid;
    break;
  case OpcUa_BadEncodingError:
    *bstatus = constants__e_sc_bad_encoding_error;
    break;
  case OpcUa_BadInvalidArgument:
    *bstatus = constants__e_sc_bad_invalid_argument;
    break;
  case OpcUa_BadUnexpectedError:
    *bstatus = constants__e_sc_bad_unexpected_error;
    break;
  case OpcUa_BadOutOfMemory:
    *bstatus = constants__e_sc_bad_out_of_memory;
    break;
  default:
    success = FALSE;
  }
  return success;
}

