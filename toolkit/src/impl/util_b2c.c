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
#include <stdbool.h>
#include <string.h>

#include "util_b2c.h"

#include "sopc_types.h"
#include "crypto_profiles.h"

void util_message__get_encodeable_type(const constants__t_msg_type_i message__msg_type,
                                       SOPC_EncodeableType** reqEncType,
                                       SOPC_EncodeableType** respEncType,
                                       t_bool* isRequest) {
  switch(message__msg_type){
  case constants__e_msg_session_create_req:
    *reqEncType = &OpcUa_CreateSessionRequest_EncodeableType;
    *respEncType = &OpcUa_CreateSessionResponse_EncodeableType;
    *isRequest = true;
    break;
  case constants__e_msg_session_create_resp:
    *reqEncType = &OpcUa_CreateSessionRequest_EncodeableType;
    *respEncType = &OpcUa_CreateSessionResponse_EncodeableType;
    break;
  case constants__e_msg_session_activate_req:
    *reqEncType = &OpcUa_ActivateSessionRequest_EncodeableType;
    *respEncType = &OpcUa_ActivateSessionResponse_EncodeableType;
    *isRequest = true;
    break;
  case constants__e_msg_session_activate_resp:
    *reqEncType = &OpcUa_ActivateSessionRequest_EncodeableType;
    *respEncType = &OpcUa_ActivateSessionResponse_EncodeableType;
    break;
  case constants__e_msg_session_close_req:
    *reqEncType = &OpcUa_CloseSessionRequest_EncodeableType;
    *respEncType = &OpcUa_CloseSessionResponse_EncodeableType;
    *isRequest = true;
    break;
  case constants__e_msg_session_close_resp:
    *reqEncType = &OpcUa_CloseSessionRequest_EncodeableType;
    *respEncType = &OpcUa_CloseSessionResponse_EncodeableType;
    break;
  case constants__e_msg_session_read_req:
    *reqEncType = &OpcUa_ReadRequest_EncodeableType;
    *respEncType = &OpcUa_ReadResponse_EncodeableType;
    *isRequest = true;
    break;
  case constants__e_msg_session_read_resp:
    *reqEncType = &OpcUa_ReadRequest_EncodeableType;
    *respEncType = &OpcUa_ReadResponse_EncodeableType;
    break;
  case constants__e_msg_session_write_req:
    *reqEncType = &OpcUa_WriteRequest_EncodeableType;
    *respEncType = &OpcUa_WriteResponse_EncodeableType;
    *isRequest = true;
    break;
  case constants__e_msg_session_write_resp:
    *reqEncType = &OpcUa_WriteRequest_EncodeableType;
    *respEncType = &OpcUa_WriteResponse_EncodeableType;
    break;
  case constants__e_msg_service_fault_resp:
    *reqEncType = NULL;
    *respEncType = &OpcUa_ServiceFault_EncodeableType;
    break;
  case constants__e_msg_get_endpoints_service_req:
    *reqEncType = &OpcUa_GetEndpointsRequest_EncodeableType;
    *respEncType = &OpcUa_GetEndpointsResponse_EncodeableType;
    *isRequest = true;
    break;
  case constants__e_msg_get_endpoints_service_resp:
    *reqEncType = &OpcUa_GetEndpointsRequest_EncodeableType;
    *respEncType = &OpcUa_GetEndpointsResponse_EncodeableType;
    break;
  case constants__e_msg_session_browse_req:
    *reqEncType = &OpcUa_BrowseRequest_EncodeableType;
    *respEncType = &OpcUa_BrowseResponse_EncodeableType;
    *isRequest = true;
    break;
  case constants__e_msg_session_browse_resp:
    *reqEncType = &OpcUa_BrowseRequest_EncodeableType;
    *respEncType = &OpcUa_BrowseResponse_EncodeableType;
    break;
  case constants__e_msg_session_create_subscription_req:
    *reqEncType = &OpcUa_CreateSubscriptionRequest_EncodeableType;
    *respEncType = &OpcUa_CreateSubscriptionResponse_EncodeableType;
    *isRequest = true;
    break;
  case constants__e_msg_session_create_subscription_resp:
    *reqEncType = &OpcUa_CreateSubscriptionRequest_EncodeableType;
    *respEncType = &OpcUa_CreateSubscriptionResponse_EncodeableType;
    break;

  default:
    printf("util_message__get_encodeable_type: not implemented message type required\n");
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
  }else if(encType == &OpcUa_GetEndpointsRequest_EncodeableType){
    *message__msg_type = constants__e_msg_get_endpoints_service_req;
  }else if(encType == &OpcUa_GetEndpointsResponse_EncodeableType){
    *message__msg_type = constants__e_msg_get_endpoints_service_resp;
  }else if(encType == &OpcUa_BrowseRequest_EncodeableType){
    *message__msg_type = constants__e_msg_session_browse_req;
  }else if(encType == &OpcUa_BrowseResponse_EncodeableType){
    *message__msg_type = constants__e_msg_session_browse_resp;
  }else if(encType == &OpcUa_CreateSubscriptionRequest_EncodeableType){
    *message__msg_type = constants__e_msg_session_create_subscription_req;
  }else if(encType == &OpcUa_CreateSubscriptionResponse_EncodeableType){
    *message__msg_type = constants__e_msg_session_create_subscription_resp;
  }else if(encType == &OpcUa_ServiceFault_EncodeableType){
    *message__msg_type = constants__e_msg_service_fault_resp;
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
  case constants__e_sc_bad_nothing_to_do:
    *status = OpcUa_BadNothingToDo;
    break;
  case constants__e_sc_bad_too_many_ops:
    *status = OpcUa_BadTooManyOperations;
    break;
  case constants__e_sc_bad_node_id_unknown:
    *status = OpcUa_BadNodeIdUnknown;
    break;
  case constants__e_sc_bad_node_id_invalid:
    *status = OpcUa_BadNodeIdInvalid;
    break;
  case constants__e_sc_bad_attribute_id_invalid:
    *status = OpcUa_BadAttributeIdInvalid;
    break;
  default:
    printf("util_message__B_to_C_status_code\n");
    exit(1);
  }
}

t_bool util_status_code__C_to_B(SOPC_StatusCode status,
                                constants__t_StatusCode_i* bstatus){
  t_bool success = true;
  switch(status){
  case STATUS_OK:
    *bstatus = constants__e_sc_ok;
    break;
  case STATUS_NOK:
    *bstatus = constants__e_sc_nok;
    break;
  case OpcUa_BadSecureChannelClosed:
    *bstatus = constants__e_sc_bad_secure_channel_closed;
    break;
  case OpcUa_BadSecureChannelIdInvalid:
    *bstatus = constants__e_sc_bad_secure_channel_id_invalid;
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
  case OpcUa_BadNothingToDo:
    *bstatus = constants__e_sc_bad_nothing_to_do;
    break;
  case OpcUa_BadTooManyOperations:
    *bstatus = constants__e_sc_bad_too_many_ops;
    break;
  case OpcUa_BadNodeIdUnknown:
    *bstatus = constants__e_sc_bad_node_id_unknown;
    break;
  case OpcUa_BadNodeIdInvalid:
    *bstatus = constants__e_sc_bad_node_id_invalid;
    break;
  case OpcUa_BadAttributeIdInvalid:
    *bstatus = constants__e_sc_bad_attribute_id_invalid;
    break;
  default:
    success = false;
  }
  return success;
}


bool util_channel__SecurityPolicy_C_to_B(const char *uri,
                                         constants__t_SecurityPolicy *secpol)
{
    if(NULL == uri || NULL == secpol)
        return false;

    if(strncmp(uri, SecurityPolicy_None_URI, strlen(SecurityPolicy_None_URI)) == 0) {
        *secpol = constants__e_secpol_None;
        return true;
    }
    if(strncmp(uri, SecurityPolicy_Basic256_URI, strlen(SecurityPolicy_Basic256_URI)) == 0) {
        *secpol = constants__e_secpol_B256;
        return true;
    }
    if(strncmp(uri, SecurityPolicy_Basic256Sha256_URI, strlen(SecurityPolicy_Basic256Sha256_URI)) == 0) {
        *secpol = constants__e_secpol_B256S256;
        return true;
    }

    return false;
}


bool util_BrowseDirection__B_to_C(constants__t_BrowseDirection_i bdir,
                                  OpcUa_BrowseDirection *cdir)
{
    if(NULL == cdir)
        return false;

    switch(bdir)
    {
    case constants__e_bd_forward:
        *cdir = OpcUa_BrowseDirection_Forward;
        break;
    case constants__e_bd_inverse:
        *cdir = OpcUa_BrowseDirection_Inverse;
        break;
    case constants__e_bd_both:
        *cdir = OpcUa_BrowseDirection_Both;
        break;
    case constants__e_bd_indet:
    default:
        return false;
    }

    return true;
}


bool util_BrowseDirection__C_to_B(OpcUa_BrowseDirection cdir,
                                  constants__t_BrowseDirection_i *bdir)
{
    if(NULL == bdir)
        return false;

    switch(cdir)
    {
    case OpcUa_BrowseDirection_Forward:
        *bdir = constants__e_bd_forward;
        break;
    case OpcUa_BrowseDirection_Inverse:
        *bdir = constants__e_bd_inverse;
        break;
    case OpcUa_BrowseDirection_Both:
        *bdir = constants__e_bd_both;
        break;
    default:
        return false;
    }

    return true;
}


bool util_NodeClass__B_to_C(constants__t_NodeClass_i bncl,
                            OpcUa_NodeClass *cncl)
{
    if(NULL == cncl)
        return false;

    switch(bncl)
    {
    case constants__e_ncl_Object:
        *cncl = OpcUa_NodeClass_Object;
        break;
    case constants__e_ncl_Variable:
        *cncl = OpcUa_NodeClass_Variable;
        break;
    case constants__e_ncl_Method:
        *cncl = OpcUa_NodeClass_Method;
        break;
    case constants__e_ncl_ObjectType:
        *cncl = OpcUa_NodeClass_ObjectType;
        break;
    case constants__e_ncl_VariableType:
        *cncl = OpcUa_NodeClass_VariableType;
        break;
    case constants__e_ncl_ReferenceType:
        *cncl = OpcUa_NodeClass_ReferenceType;
        break;
    case constants__e_ncl_DataType:
        *cncl = OpcUa_NodeClass_DataType;
        break;
    case constants__e_ncl_View:
        *cncl = OpcUa_NodeClass_View;
        break;
    case constants__c_NodeClass_indet:
    default:
        return false;
    }

    return true;
}


bool util_NodeClass__C_to_B(OpcUa_NodeClass cncl,
                            constants__t_NodeClass_i *bncl)
{
    if(NULL == bncl)
        return false;

    switch(cncl)
    {
    case OpcUa_NodeClass_Object:
        *bncl = constants__e_ncl_Object;
        break;
    case OpcUa_NodeClass_Variable:
        *bncl = constants__e_ncl_Variable;
        break;
    case OpcUa_NodeClass_Method:
        *bncl = constants__e_ncl_Method;
        break;
    case OpcUa_NodeClass_ObjectType:
        *bncl = constants__e_ncl_ObjectType;
        break;
    case OpcUa_NodeClass_VariableType:
        *bncl = constants__e_ncl_VariableType;
        break;
    case OpcUa_NodeClass_ReferenceType:
        *bncl = constants__e_ncl_ReferenceType;
        break;
    case OpcUa_NodeClass_DataType:
        *bncl = constants__e_ncl_DataType;
        break;
    case OpcUa_NodeClass_View:
        *bncl = constants__e_ncl_View;
        break;
    default:
        return false;
    }

    return true;
}

