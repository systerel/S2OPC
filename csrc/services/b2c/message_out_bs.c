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
#include <stdbool.h>
#include <assert.h>

#include "sopc_encodeable.h"

#include "internal_msg.h"
#include "message_out_bs.h"
#include "util_b2c.h"

#include "constants_bs.h"

#include "sopc_encoder.h"
#include "sopc_toolkit_config_internal.h"
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
   constants__t_msg_header_i * const message_out_bs__nmsg_header,
   constants__t_msg_i * const message_out_bs__nmsg) {
  void* header = NULL;
  void* msg = NULL;
  SOPC_StatusCode status = STATUS_NOK;

  SOPC_EncodeableType* encTyp = NULL;
  SOPC_EncodeableType* reqEncTyp = NULL;
  SOPC_EncodeableType* respEncTyp = NULL;
  t_bool isReq = false; 
  util_message__get_encodeable_type(message_out_bs__msg_type,
                                    &reqEncTyp,
                                    &respEncTyp,
                                    &isReq);

  if(NULL != reqEncTyp && NULL != respEncTyp){
    if(isReq != false){
      encTyp = reqEncTyp;
    }else{
      encTyp = respEncTyp;
    }
  }
  if(NULL != encTyp){
    status = SOPC_Encodeable_Create(encTyp, &msg);
    if(STATUS_OK == status){
      if(isReq == false){
        status = SOPC_Encodeable_Create(&OpcUa_ResponseHeader_EncodeableType, &header);
      }else{
        status = SOPC_Encodeable_Create(&OpcUa_RequestHeader_EncodeableType, &header);
      }
    }else{
      SOPC_Encodeable_Delete(encTyp, &msg);
    }
  }
  if(STATUS_OK == status){
    *message_out_bs__nmsg = (constants__t_msg_i) msg;
    *message_out_bs__nmsg_header = (constants__t_msg_header_i) header;
  }else{
    *message_out_bs__nmsg = constants__c_msg_indet;
  }
}

void message_out_bs__alloc_app_req_msg_header(constants__t_msg_header_i * const message_out_bs__nmsg_header){
  void* header = NULL;
  SOPC_StatusCode status = SOPC_Encodeable_Create(&OpcUa_RequestHeader_EncodeableType, &header);
  if(STATUS_OK == status){
    *message_out_bs__nmsg_header = (constants__t_msg_header_i) header;
  }else{
    *message_out_bs__nmsg_header = constants__c_msg_header_indet;
  }
}


void message_out_bs__alloc_req_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
   constants__t_msg_header_i * const message_out_bs__nmsg_header,
   constants__t_msg_i * const message_out_bs__nmsg){
  util_message_out_bs__alloc_msg(message_out_bs__msg_type,
                                 message_out_bs__nmsg_header,
                                 message_out_bs__nmsg);
}

void message_out_bs__alloc_resp_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
   constants__t_msg_header_i * const message_out_bs__nmsg_header,
   constants__t_msg_i * const message_out_bs__nmsg){
  util_message_out_bs__alloc_msg(message_out_bs__msg_type,
                                 message_out_bs__nmsg_header,
                                 message_out_bs__nmsg);
}

void message_out_bs__bless_msg_out(
   const constants__t_msg_type_i message_out_bs__msg_type,
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_msg_i message_out_bs__msg){
  /* NOTHING TO DO: in B model now message_out_bs__msg = c_msg_out now */
    (void) message_out_bs__msg_type;
    (void) message_out_bs__msg_header;
    (void) message_out_bs__msg;
}

void message_out_bs__dealloc_msg_header_out(
   const constants__t_msg_header_i message_out_bs__msg_header){
// Generated header, parameter not really a const. TODO: Check if message should not be a / in a global variable
SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
  if((*(SOPC_EncodeableType**) message_out_bs__msg_header) == &OpcUa_ResponseHeader_EncodeableType){
    SOPC_Encodeable_Delete(&OpcUa_ResponseHeader_EncodeableType, (void**) &message_out_bs__msg_header);
  }else if((*(SOPC_EncodeableType**) message_out_bs__msg_header) == &OpcUa_RequestHeader_EncodeableType){
    SOPC_Encodeable_Delete(&OpcUa_RequestHeader_EncodeableType, (void**) &message_out_bs__msg_header);
  }else{
    assert(false);
  }
SOPC_GCC_DIAGNOSTIC_RESTORE
}

void message_out_bs__dealloc_msg_out(
   const constants__t_msg_i message_out_bs__msg) {
  // First field of each message structure is an encodeable type
  SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) message_out_bs__msg;
  if(message_out_bs__msg != constants__c_msg_indet){
    if(&OpcUa_ReadResponse_EncodeableType == encType){
      /* Current implementation share the variants of the address space in the response,
         avoid deallocation of those variants */
      OpcUa_ReadResponse* readMsg = (OpcUa_ReadResponse*) message_out_bs__msg;
      if(NULL != readMsg){
        free(readMsg->Results);
        readMsg->Results = NULL;
        readMsg->NoOfResults = 0;
      }
    }
    // TODO: status returned ?
    // TODO: const parameter modified !

// To could keep generated prototype
// Generated header, parameter not really a const. TODO: Check if message should not be a / in a global variable
SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_Encodeable_Delete(encType, (void*) &message_out_bs__msg);
SOPC_GCC_DIAGNOSTIC_RESTORE
  }
}

void message_out_bs__encode_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_msg_i message_out_bs__msg,
   constants__t_byte_buffer_i * const message_out_bs__buffer){
  *message_out_bs__buffer = constants__c_byte_buffer_indet;
  SOPC_StatusCode status = STATUS_NOK;
  SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) message_out_bs__msg;
  SOPC_EncodeableType* headerType = *(SOPC_EncodeableType**) message_out_bs__msg_header;
  SOPC_Buffer* buffer = SOPC_Buffer_Create(SOPC_MAX_MESSAGE_LENGTH);
  if(NULL != buffer){
    status = STATUS_OK;
  }
  if(STATUS_OK == status){
    status = SOPC_Buffer_SetDataLength(buffer, SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH +
                                               SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH +
                                               SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
  }
  if(STATUS_OK == status){
    // Encodeable type: either msg_type = service fault type or it is the type provided by the msg
    if(message_out_bs__msg_type == constants__e_msg_service_fault_resp){
      encType = &OpcUa_ServiceFault_EncodeableType;
    }
  }
  if(STATUS_OK == status){
    status = SOPC_Buffer_SetPosition(buffer, SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH +
                                             SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH +
                                             SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
  }
  if(STATUS_OK == status){
    status = SOPC_EncodeMsg_Type_Header_Body(buffer, 
                                             encType, 
                                             headerType,
                                             message_out_bs__msg_header,
                                             message_out_bs__msg);
  }
  if(STATUS_OK == status){
    *message_out_bs__buffer = (constants__t_byte_buffer_i) buffer;
  }
}

void message_out_bs__get_msg_out_type(
   const constants__t_msg_i message_out_bs__msg,
   constants__t_msg_type_i * const message_out_bs__msgtype) {
  SOPC_EncodeableType* encType = *(SOPC_EncodeableType**) message_out_bs__msg;
  util_message__get_message_type(encType,
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
    (void) message_out_bs__msg;
    assert(message_out_bs__user == 1); // anonymous user only for now
}

void message_out_bs__write_create_session_req_msg_endpointUrl(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_channel_config_idx_i message_out_bs__channel_config_idx){
  OpcUa_CreateSessionRequest* createSessionReq = (OpcUa_CreateSessionRequest*) message_out_bs__msg;
  SOPC_SecureChannel_Config* chConfig = SOPC_Toolkit_GetSecureChannelConfig(message_out_bs__channel_config_idx);
  assert(STATUS_OK == SOPC_String_CopyFromCString(&createSessionReq->EndpointUrl, chConfig->url));
}

void message_out_bs__write_create_session_req_msg_crypto(
   const constants__t_msg_i message_out_bs__p_req_msg,
   const constants__t_channel_config_idx_i message_out_bs__p_channel_config_idx,
   const constants__t_Nonce_i message_out_bs__p_nonce)
{
    SOPC_SecureChannel_Config *pSCCfg = NULL;
    OpcUa_CreateSessionRequest *pReq = (OpcUa_CreateSessionRequest *)message_out_bs__p_req_msg;
    const SOPC_Certificate *pCrtCli = NULL;

    /* Retrieve the certificate */
    pSCCfg = SOPC_Toolkit_GetSecureChannelConfig((uint32_t) message_out_bs__p_channel_config_idx);
    if(NULL == pSCCfg)
        return;
    pCrtCli = pSCCfg->crt_cli;
    if(NULL == pCrtCli)
        return;

    /* Write the Certificate */
    SOPC_ByteString_Clear(&pReq->ClientCertificate);
    /* TODO: this is a malloc error, this can fail, and the B model should be notified */
    if(STATUS_OK != SOPC_KeyManager_Certificate_CopyDER(pCrtCli, &pReq->ClientCertificate.Data,
                                                   (uint32_t *)&pReq->ClientCertificate.Length))
        return;

    /* Write the nonce */
    SOPC_ByteString_Clear(&pReq->ClientNonce);
    /* TODO: this can also fail because of the malloc */
    if(STATUS_OK != SOPC_ByteString_Copy(&pReq->ClientNonce, (SOPC_ByteString *)message_out_bs__p_nonce))
        return;
}


void message_out_bs__write_create_session_msg_session_token(
   const constants__t_msg_i message_out_bs__msg,
   const constants__t_session_token_i message_out_bs__session_token) {
  OpcUa_CreateSessionResponse* createSessionResp = (OpcUa_CreateSessionResponse*) message_out_bs__msg;
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

    OpcUa_CreateSessionRequest* createSessionReq = (OpcUa_CreateSessionRequest*) message_out_bs__req_msg;

    OpcUa_CreateSessionResponse* createSessionResp = (OpcUa_CreateSessionResponse*) message_out_bs__resp_msg;

    createSessionResp->RevisedSessionTimeout = SOPC_SESSION_TIMEOUT;

    *message_out_bs__ret = SOPC_Discovery_GetEndPointsDescriptions(message_out_bs__endpoint_config_idx,
                                                                   true,
                                                                   &createSessionReq->EndpointUrl,
                                                                   (uint32_t*)&createSessionResp->NoOfServerEndpoints,
                                                                   &createSessionResp->ServerEndpoints);
}


void message_out_bs__write_create_session_resp_msg_crypto(
   const constants__t_msg_i message_out_bs__p_msg,
   const constants__t_channel_config_idx_i message_out_bs__p_channel_config_idx,
   const constants__t_Nonce_i message_out_bs__p_nonce,
   const constants__t_SignatureData_i message_out_bs__p_signature,
   constants__t_StatusCode_i * const message_out_bs__sc)
{
    SOPC_SecureChannel_Config *pSCCfg = NULL;
    const SOPC_Certificate *pCrtSrv = NULL;
    SOPC_StatusCode sc = STATUS_OK;
    OpcUa_CreateSessionResponse *pResp = (OpcUa_CreateSessionResponse *) message_out_bs__p_msg;
    OpcUa_SignatureData *pSig = (OpcUa_SignatureData *)message_out_bs__p_signature;

    /* Retrieve the certificate */
    pSCCfg = SOPC_Toolkit_GetSecureChannelConfig((uint32_t) message_out_bs__p_channel_config_idx);
    if(NULL == pSCCfg)
        sc = STATUS_NOK;
    if(STATUS_OK == sc) {
        pCrtSrv = pSCCfg->crt_srv;
        if(NULL == pCrtSrv)
            sc = STATUS_NOK;
    }

    /* Write the Certificate */
    if(STATUS_OK == sc) {
        SOPC_ByteString_Clear(&pResp->ServerCertificate);
        /* TODO: this is a malloc error, this can fail, and the B model should be notified */
        sc = SOPC_KeyManager_Certificate_CopyDER(pCrtSrv, &pResp->ServerCertificate.Data,
                                            (uint32_t *)&pResp->ServerCertificate.Length);
    }

    /* Copy Nonce */
    /* TODO: should borrow a reference instead of copy */
    if(STATUS_OK == sc)
        sc = SOPC_ByteString_Copy(&pResp->ServerNonce, (SOPC_ByteString *)message_out_bs__p_nonce);

    /* Copy Signature, which is not a built-in, so copy its fields */
    /* TODO: should borrow a reference instead of copy */
    if(STATUS_OK == sc)
        sc = SOPC_String_Copy(&pResp->ServerSignature.Algorithm, &pSig->Algorithm);
    if(STATUS_OK == sc)
        sc = SOPC_ByteString_Copy(&pResp->ServerSignature.Signature, &pSig->Signature);

    if(STATUS_OK == sc)
        *message_out_bs__sc = constants__e_sc_ok;
    else
        *message_out_bs__sc = constants__e_sc_nok;
}


void message_out_bs__write_activate_session_req_msg_crypto(
   const constants__t_msg_i message_out_bs__activate_req_msg,
   const constants__t_SignatureData_i message_out_bs__signature,
   constants__t_StatusCode_i * const message_out_bs__sc)

{
    SOPC_StatusCode sc = STATUS_NOK;
    OpcUa_ActivateSessionRequest *pReq = (OpcUa_ActivateSessionRequest *) message_out_bs__activate_req_msg;
    OpcUa_SignatureData *pSig = (OpcUa_SignatureData *)message_out_bs__signature;

    /* Copy Signature, which is not a built-in, so copy its fields */
    /* TODO: should borrow a reference instead of copy */
    sc = SOPC_String_Copy(&pReq->ClientSignature.Algorithm, &pSig->Algorithm);
    
    if(STATUS_OK == sc)
        sc = SOPC_ByteString_Copy(&pReq->ClientSignature.Signature, &pSig->Signature);

    if(STATUS_OK == sc)
        *message_out_bs__sc = constants__e_sc_ok;
    else
        *message_out_bs__sc = constants__e_sc_nok;    
}

void message_out_bs__write_activate_session_resp_msg_crypto(
   const constants__t_msg_i message_out_bs__activate_resp_msg,
   const constants__t_Nonce_i message_out_bs__nonce)
{
    OpcUa_ActivateSessionResponse *pResp = (OpcUa_ActivateSessionResponse *)message_out_bs__activate_resp_msg;

    /* Write the nonce */
    /* TODO: this can also fail because of the malloc */
    if(STATUS_OK != SOPC_ByteString_Copy(&pResp->ServerNonce, (SOPC_ByteString *)message_out_bs__nonce))
        return;
}

void message_out_bs__write_msg_out_header_req_handle(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_request_handle_i message_out_bs__req_handle) {
  if((*(SOPC_EncodeableType**) message_out_bs__msg_header) == &OpcUa_ResponseHeader_EncodeableType){
    ((OpcUa_ResponseHeader*) message_out_bs__msg_header)->RequestHandle = message_out_bs__req_handle;
  }else if((*(SOPC_EncodeableType**) message_out_bs__msg_header) == &OpcUa_RequestHeader_EncodeableType){
    ((OpcUa_RequestHeader*) message_out_bs__msg_header)->RequestHandle = message_out_bs__req_handle;
  }else{
    assert(false);
  }
}

void message_out_bs__write_msg_out_header_session_token(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_session_token_i message_out_bs__session_token) {
  SOPC_NodeId* authToken = (SOPC_NodeId*) message_out_bs__session_token;
  
  if(STATUS_OK != SOPC_NodeId_Copy(&((OpcUa_RequestHeader*)message_out_bs__msg_header)->AuthenticationToken, 
                                   authToken))
  {
    printf("message_out_bs__write_msg_header_session_token\n");
    exit(1);
  }
}

void message_out_bs__write_msg_resp_header_service_status(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_StatusCode_i message_out_bs__status_code) {
  SOPC_StatusCode status = STATUS_NOK;
  util_status_code__B_to_C(message_out_bs__status_code,
                           &status);
  ((OpcUa_ResponseHeader*) message_out_bs__msg_header)->ServiceResult = status;
}

