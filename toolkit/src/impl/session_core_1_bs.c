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
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_encodeable.h"
#include "sopc_types.h"
#include "sopc_event_dispatcher_manager.h"
#include "sopc_user_app_itf.h"
#include "crypto_provider.h"
#include "crypto_profiles.h"
#include "key_manager.h"
#include "secret_buffer.h"
#include "sopc_toolkit_config.h"

#include "session_core_1_bs.h"
#include "channel_mgr_bs.h"

#define LENGTH_NONCE 32

typedef struct session {
  constants__t_session_i id;
  constants__t_channel_i session_core_1_bs__channel;
  constants__t_sessionState session_core_1_bs__state;
  constants__t_session_token_i session_core_1_bs__session_token;
  constants__t_user_i session_core_1_bs__user;
  t_bool cli_activated_session;
  SOPC_ByteString NonceServer;
  SOPC_ByteString NonceClient;
  OpcUa_SignatureData SignatureData;
} session;

static session unique_session;
static t_bool unique_session_init = false;
static t_bool unique_session_created = false;

static int token_cpt = 0;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_1_bs__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_1_bs__server_get_session_from_token(
   const constants__t_session_token_i session_core_1_bs__session_token,
   constants__t_session_i * const session_core_1_bs__session) {
  int32_t comparison = 0;
  if(false != unique_session_created){
    SOPC_StatusCode status = SOPC_NodeId_Compare((SOPC_NodeId*)session_core_1_bs__session_token,
                                                 (SOPC_NodeId*)unique_session.session_core_1_bs__session_token,
                                                 &comparison);
    if(STATUS_OK == status && comparison == 0){
      *session_core_1_bs__session = unique_session.id;
    }
  }
}

void session_core_1_bs__client_get_token_from_session(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_session_token_i * const session_core_1_bs__session_token) {
  if(unique_session.id == session_core_1_bs__session){
    *session_core_1_bs__session_token = unique_session.session_core_1_bs__session_token;
  }else{
    *session_core_1_bs__session_token = constants__c_session_token_indet;
  }
}

void session_core_1_bs__server_get_fresh_session_token(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_session_token_i * const session_core_1_bs__token) {
  if(token_cpt + 1 > 0 &&
     unique_session.id == session_core_1_bs__session &&
     unique_session.session_core_1_bs__session_token == constants__c_session_token_indet){
    SOPC_NodeId* token = malloc(sizeof(SOPC_NodeId));
    if(NULL != token){
      SOPC_NodeId_Initialize(token);
      token_cpt++;
      token->IdentifierType = IdentifierType_Numeric;
      token->Data.Numeric = token_cpt;
      *session_core_1_bs__token = token;
      unique_session.session_core_1_bs__session_token = token;
    }else{
      *session_core_1_bs__token = constants__c_session_token_indet;
    }
  }else{
    *session_core_1_bs__token = constants__c_session_token_indet;
  }
}

void session_core_1_bs__server_is_valid_session_token(
   const constants__t_session_token_i session_core_1_bs__token,
   t_bool * const session_core_1_bs__ret) {
  int32_t comparison = 0;
  if(session_core_1_bs__token != constants__c_session_token_indet){
    SOPC_StatusCode status = SOPC_NodeId_Compare((SOPC_NodeId*)session_core_1_bs__token,
                                                 (SOPC_NodeId*)unique_session.session_core_1_bs__session_token,
                                                 &comparison);
    if(STATUS_OK == status && comparison == 0){
      *session_core_1_bs__ret = true;
    }else{
      *session_core_1_bs__ret = false;
    }
  }else{
    *session_core_1_bs__ret = false;
  }
}

void session_core_1_bs__client_set_session_token(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_session_token_i session_core_1_bs__token) {
  if(unique_session.id == session_core_1_bs__session){
    if(session_core_1_bs__token != unique_session.session_core_1_bs__session_token){
      SOPC_NodeId* token = malloc(sizeof(SOPC_NodeId));
      if(NULL != token){
        SOPC_NodeId_Initialize(token);
        if(STATUS_OK == SOPC_NodeId_Copy(token, (SOPC_NodeId*) session_core_1_bs__token)){
          unique_session.session_core_1_bs__session_token = token;
        }else{
          printf("session_core_1_bs__set_session_token\n");
          exit(1);
        }
      }else{
        printf("session_core_1_bs__set_session_token\n");
        exit(1);
      }
    }else{
      // guarantee by B code
      printf("session_core_1_bs__set_session_token\n");
      exit(1);
    }
  }else{
    printf("session_core_1_bs__set_session_token\n");
    exit(1);
  }
}

void session_core_1_bs__delete_session(
   const constants__t_session_i session_core_1_bs__session) {
  (void) session_core_1_bs__session;
  printf("session_core_1_bs__delete_session\n");
  exit(1);
}

void session_core_1_bs__init_new_session(
   constants__t_session_i * const session_core_1_bs__session){
  if(false == unique_session_init){
    unique_session.id = 1;
    *session_core_1_bs__session = unique_session.id;
    unique_session_init = true;
    unique_session.cli_activated_session = false;
    unique_session.session_core_1_bs__state = constants__e_session_init;
  }else{
    *session_core_1_bs__session = constants__c_session_indet;
  }
}

void session_core_1_bs__create_session(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__channel,
   const constants__t_sessionState session_core_1_bs__state) {
  if(session_core_1_bs__session == unique_session.id && false == unique_session_created){
    unique_session.session_core_1_bs__channel = session_core_1_bs__channel;
    unique_session.session_core_1_bs__state = session_core_1_bs__state;
    unique_session.session_core_1_bs__session_token = constants__c_session_token_indet;
    unique_session.session_core_1_bs__user = constants__c_user_indet;
    unique_session.cli_activated_session = false;
    unique_session_created = true;
  }
}

void session_core_1_bs__create_session_failure(const constants__t_session_i session_core_1_bs__session){
  if(session_core_1_bs__session == unique_session.id && false != unique_session_created){
    unique_session_created = false;
    unique_session.session_core_1_bs__state = constants__e_session_init;
    SOPC_EventDispatcherManager_AddEvent(applicationEventDispatcherMgr,
                                         SOPC_AppEvent_ComEvent_Create(SE_SESSION_ACTIVATION_FAILURE),
                                         0, // TMP unused ? => session idx ?
                                         NULL, // user ?
                                         0, // TBD: status
                                         "Session activation failure notification");
  }
}

void session_core_1_bs__is_valid_session(
   const constants__t_session_i session_core_1_bs__session,
   t_bool * const session_core_1_bs__ret) {
  *session_core_1_bs__ret = session_core_1_bs__session == unique_session.id;
}

void session_core_1_bs__get_session_state(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_sessionState * const session_core_1_bs__state) {
  if(session_core_1_bs__session == unique_session.id){
    *session_core_1_bs__state = unique_session.session_core_1_bs__state;
  }else{
    *session_core_1_bs__state = constants__e_session_closed;
  }
}

void session_core_1_bs__set_session_state(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_sessionState session_core_1_bs__state) {
  t_bool is_client = false;
  constants__t_channel_i channel = constants__c_channel_indet;
  if(unique_session.id == session_core_1_bs__session){
    unique_session.session_core_1_bs__state = session_core_1_bs__state;

    // Check if it is client side
    session_core_1_bs__get_session_channel(session_core_1_bs__session,
                                           &channel);
    channel_mgr_bs__is_client_channel(channel, &is_client);

    if(session_core_1_bs__state == constants__e_session_userActivated){
      if(is_client != false &&
         unique_session.cli_activated_session == false)
      {
        unique_session.cli_activated_session = true;
        SOPC_EventDispatcherManager_AddEvent(applicationEventDispatcherMgr,
                                             SOPC_AppEvent_ComEvent_Create(SE_ACTIVATED_SESSION),
                                             session_core_1_bs__session,
                                             NULL,
                                             0, // unused
                                             "Session activated notification");
      }
    }else if(session_core_1_bs__state == constants__e_session_userActivating ||
             session_core_1_bs__state == constants__e_session_scActivating){
      if(is_client != false &&
         unique_session.cli_activated_session != false){
        // session is in re-activation step
        unique_session.cli_activated_session = false;
        SOPC_EventDispatcherManager_AddEvent(applicationEventDispatcherMgr,
                                             SOPC_AppEvent_ComEvent_Create(SE_SESSION_REACTIVATING),
                                             session_core_1_bs__session,
                                             NULL,
                                             0, // unused
                                             "Session re-activation notification");
      }
    }
  }
}

void session_core_1_bs__set_session_state_closed(const constants__t_session_i session_core_1_bs__session) {
  if(session_core_1_bs__session == unique_session.id){
    // Manage notification on client side
    constants__t_channel_i channel = constants__c_channel_indet;
    t_bool bres = false;
    session_core_1_bs__get_session_channel(session_core_1_bs__session,
                                           &channel);
    if(channel == constants__c_channel_indet){
      // init session phase: no channel associated (mandatory client side)
      SOPC_EventDispatcherManager_AddEvent(applicationEventDispatcherMgr,
                                           SOPC_AppEvent_ComEvent_Create(SE_SESSION_ACTIVATION_FAILURE),
                                           0, // TMP unused ? => session idx ?
                                           NULL, // user ?
                                           0, // TBD: status
                                           "Session activation failure notification");
    }else{
      channel_mgr_bs__is_client_channel(channel, &bres);
      if(bres != false){
        if(unique_session.cli_activated_session == false){
          SOPC_EventDispatcherManager_AddEvent(applicationEventDispatcherMgr,
                                               SOPC_AppEvent_ComEvent_Create(SE_SESSION_ACTIVATION_FAILURE),
                                               0, // TMP unused ? => session idx ?
                                               NULL, // user ?
                                               0, // TBD: status
                                               "Session activation failure notification");
        }else{
          // Activated session closing
          SOPC_EventDispatcherManager_AddEvent(applicationEventDispatcherMgr,
                                               SOPC_AppEvent_ComEvent_Create(SE_CLOSED_SESSION),
                                               session_core_1_bs__session, // session idx
                                               NULL,
                                               0, // TBD: status
                                               "Session closed notification");
        }
      }
    }

    // Modify state
    unique_session.session_core_1_bs__state = constants__e_session_closed;
    unique_session.session_core_1_bs__user = constants__c_user_indet;
    if(constants__c_session_token_indet != unique_session.session_core_1_bs__session_token){
      SOPC_NodeId* token = unique_session.session_core_1_bs__session_token;
      SOPC_NodeId_Clear(token);
      free(token);
      unique_session.session_core_1_bs__session_token = constants__c_session_token_indet;
    }
    unique_session.session_core_1_bs__channel = constants__c_channel_indet;
  }
}

void session_core_1_bs__set_session_channel(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__channel) {
  // TODO: add constraint that session has no channel in B model ?
  //  => for now the fact it is not is used for setting channel in both cases of activate session (change of user / channel)
  if(session_core_1_bs__session == unique_session.id){
    unique_session.session_core_1_bs__channel = session_core_1_bs__channel;
  }else{
    printf("session_core_1_bs__set_session_channel\n");
    exit(1);
  }
}

void session_core_1_bs__get_session_channel(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_channel_i * const session_core_1_bs__channel) {
  if(session_core_1_bs__session == unique_session.id){
    *session_core_1_bs__channel = unique_session.session_core_1_bs__channel;
  }else{
    // session : s_session guarantee
    printf("session_core_1_bs__get_session_channel\n");
    exit(1);
  }
}

void session_core_1_bs__set_session_orphaned(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__lost_channel,
   const constants__t_channel_i session_core_1_bs__new_channel){
  (void) session_core_1_bs__session;
  (void) session_core_1_bs__lost_channel;
  (void) session_core_1_bs__new_channel;
  printf("session_core_1_bs__set_session_orphaned\n");
  exit(1);
   ;
}

void session_core_1_bs__is_valid_user(
   const constants__t_user_i session_core_1_bs__user,
   t_bool * const session_core_1_bs__ret) {
  assert(session_core_1_bs__user == 1);
  *session_core_1_bs__ret = true;
}

void session_core_1_bs__set_session_user(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_user_i session_core_1_bs__user) {
  if(session_core_1_bs__session == unique_session.id){
    unique_session.session_core_1_bs__user = session_core_1_bs__user;
  }
}

void session_core_1_bs__get_session_user(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_user_i * const session_core_1_bs__user) {
  if(session_core_1_bs__session == unique_session.id){
    *session_core_1_bs__user = unique_session.session_core_1_bs__user;
  }
}


void session_core_1_bs__server_create_session_req_do_crypto(
   const constants__t_session_i session_core_1_bs__p_session,
   const constants__t_msg_i session_core_1_bs__p_req_msg,
   const constants__t_endpoint_config_idx_i session_core_1_bs__p_endpoint_config_idx,
   const constants__t_channel_config_idx_i session_core_1_bs__p_channel_config_idx,
   t_bool * const session_core_1_bs__valid,
   constants__t_SignatureData_i * const session_core_1_bs__signature)
{
    CryptoProvider *pProvider = NULL;
    SOPC_Endpoint_Config *pECfg = NULL;
    SOPC_SecureChannel_Config *pSCCfg = NULL;
    session *pSession = NULL;
    SOPC_ByteString *pNonce = NULL;
    OpcUa_SignatureData *pSign = NULL;
    uint8_t *pToSign = NULL;
    uint32_t lenToSign = 0;
    OpcUa_CreateSessionRequest *pReq = (OpcUa_CreateSessionRequest *)session_core_1_bs__p_req_msg;

    *session_core_1_bs__valid = false;
    *session_core_1_bs__signature = constants__c_SignatureData_indet;

    /* Retrieve the security policy and mode */
    /* TODO: this function is denoted CLIENT */
    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig((uint32_t) session_core_1_bs__p_channel_config_idx);
    if(NULL == pSCCfg)
        return;

    /* Retrieve the server certificate */
    pECfg = SOPC_ToolkitServer_GetEndpointConfig((uint32_t) session_core_1_bs__p_endpoint_config_idx);
    if(NULL == pECfg)
        return;

    /* If security policy is not None, generate the nonce and a signature */
    if(strncmp(pSCCfg->reqSecuPolicyUri, SecurityPolicy_None_URI, strlen(SecurityPolicy_None_URI)+1) != 0) /* Including the terminating \0 */
    {
        /* Retrieve ptrs to Nonce and Signature */
        if(session_core_1_bs__p_session != unique_session.id)
            return;
        pSession = &unique_session;
        pNonce = &pSession->NonceServer;
        pSign = &pSession->SignatureData;

        /* Create the CryptoProvider */
        /* TODO: don't create it each time, maybe add it to the session */
        pProvider = CryptoProvider_Create(pSCCfg->reqSecuPolicyUri);

        /* Ask the CryptoProvider for LENGTH_NONCE random bytes */
        SOPC_ByteString_Clear(pNonce);
        pNonce->Length = LENGTH_NONCE;
        pNonce->Data = malloc(sizeof(SOPC_Byte)*pNonce->Length); /* TODO: This should be cleared and freed with session */
        if(NULL == pNonce->Data)
            return;
        if(STATUS_OK != CryptoProvider_GenerateRandomBytes(pProvider, pNonce->Length, &pNonce->Data))
            /* TODO: Should we clean half allocated things? */
            return;

        /* Use the server certificate to sign the client certificate ++ client nonce */
        /* a) Prepare the buffer to sign */
        lenToSign = pReq->ClientCertificate.Length + pReq->ClientNonce.Length;
        pToSign = malloc(sizeof(uint8_t)*lenToSign);
        if(NULL == pToSign)
            return;
        memcpy(pToSign, pReq->ClientCertificate.Data, pReq->ClientCertificate.Length);
        memcpy(pToSign+pReq->ClientCertificate.Length, pReq->ClientNonce.Data, pReq->ClientNonce.Length);
        /* b) Sign and store the signature in pSign */
        SOPC_ByteString_Clear(&pSign->Signature);
        if(STATUS_OK != CryptoProvider_AsymmetricGetLength_Signature(pProvider, pECfg->serverKey, (uint32_t *)&pSign->Signature.Length))
            return;
        pSign->Signature.Data = malloc(sizeof(SOPC_Byte)*pSign->Signature.Length); /* TODO: This should be freed with session */
        if(NULL == pSign->Signature.Data)
            return;
        if(STATUS_OK != CryptoProvider_AsymmetricSign(pProvider,
                                                      pToSign, lenToSign,
                                                      pECfg->serverKey,
                                                      pSign->Signature.Data, pSign->Signature.Length))
            return;
        /* c) Prepare the OpcUa_SignatureData */
        SOPC_String_Delete(&pSign->Algorithm);
        if(STATUS_OK != SOPC_String_CopyFromCString(&pSign->Algorithm, CryptoProvider_AsymmetricGetUri_SignAlgorithm(pProvider)))
            return;

        /* Clean */
        /* TODO: with the many previous returns, you do not always free it */
        free(pToSign);
        CryptoProvider_Free(pProvider);
        pProvider = NULL;
    }

    *session_core_1_bs__valid = true;
}


void session_core_1_bs__get_NonceServer(
   const constants__t_session_i session_core_1_bs__p_session,
   constants__t_Nonce_i * const session_core_1_bs__nonce)
{
    *session_core_1_bs__nonce = (constants__t_Nonce_i *)(&unique_session.NonceServer);
}


void session_core_1_bs__client_create_session_req_do_crypto(
   const constants__t_session_i session_core_1_bs__p_session,
   const constants__t_channel_i session_core_1_bs__p_channel,
   const constants__t_channel_config_idx_i session_core_1_bs__p_channel_config_idx,
   t_bool * const session_core_1_bs__valid,
   t_bool * const session_core_1_bs__nonce_needed)
{
    (void) session_core_1_bs__p_channel;
    /* Produce the Nonce when SC:Sec_pol is not "None" */
    CryptoProvider *pProvider = NULL;
    SOPC_SecureChannel_Config *pSCCfg = NULL;
    session *pSession = NULL;
    SOPC_ByteString *pNonce = NULL;

    /* Default answer */
    *session_core_1_bs__valid = false;
    *session_core_1_bs__nonce_needed = false;

    /* Retrieve the security policy */
    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig((uint32_t) session_core_1_bs__p_channel_config_idx);
    if(NULL == pSCCfg)
        return;

    /* If security policy is not None, generate the nonce */
    if(strncmp(pSCCfg->reqSecuPolicyUri, SecurityPolicy_None_URI, strlen(SecurityPolicy_None_URI)+1) != 0) /* Including the terminating \0 */
    {
        *session_core_1_bs__nonce_needed = true;
        /* Retrieve ptrs to Nonce and Signature */
        if(session_core_1_bs__p_session != unique_session.id)
            return;
        pSession = &unique_session;
        pNonce = &pSession->NonceClient;

        /* Create the CryptoProvider */
        /* TODO: don't create it each time, maybe add it to the session */
        pProvider = CryptoProvider_Create(pSCCfg->reqSecuPolicyUri);

        /* Ask the CryptoProvider for LENGTH_NONCE random bytes */
        SOPC_ByteString_Clear(pNonce);
        pNonce->Length = LENGTH_NONCE;
        pNonce->Data = malloc(sizeof(SOPC_Byte)*pNonce->Length);
        if(NULL == pNonce->Data)
            return;
        if(STATUS_OK != CryptoProvider_GenerateRandomBytes(pProvider, pNonce->Length, &pNonce->Data))
            /* TODO: Should we clean half allocated things? */
            return;

        /* Clean */
        /* TODO: with the many previous returns, you do not always free it */
        CryptoProvider_Free(pProvider);
        pProvider = NULL;
    }

    /* Success */
    *session_core_1_bs__valid = true;
}


void session_core_1_bs__get_NonceClient(
   const constants__t_session_i session_core_1_bs__p_session,
   constants__t_Nonce_i * const session_core_1_bs__nonce)
{
    *session_core_1_bs__nonce = (constants__t_Nonce_i *)(&unique_session.NonceServer);
}


void session_core_1_bs__drop_NonceClient(
   const constants__t_session_i session_core_1_bs__p_session)
{
    SOPC_ByteString_Clear(&unique_session.NonceServer);
}


void session_core_1_bs__client_create_session_check_crypto(
   const constants__t_session_i session_core_1_bs__p_session,
   const constants__t_channel_config_idx_i session_core_1_bs__p_channel_config_idx,
   const constants__t_msg_i session_core_1_bs__p_resp_msg,
   t_bool * const session_core_1_bs__valid)
{
    CryptoProvider *pProvider = NULL;
    const SOPC_SecureChannel_Config *pSCCfg = NULL;
    const session *pSession = NULL;
    const SOPC_ByteString *pNonce = NULL;
    const OpcUa_SignatureData *pSignCandid = NULL;
    uint8_t *pDerCli = NULL;
    uint32_t lenDerCli = 0;
    uint8_t *pToVerify = NULL;
    uint32_t lenToVerify = 0;
    const OpcUa_CreateSessionResponse *pResp = (OpcUa_CreateSessionResponse *)session_core_1_bs__p_resp_msg;
    const Certificate *pCrtCli = NULL, *pCrtSrv = NULL;
    const char *szSignUri = NULL;
    AsymmetricKey *pKeyCrtSrv = NULL;

    /* Default answer */
    *session_core_1_bs__valid = false;

    /* Retrieve the security policy and mode */
    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig((uint32_t) session_core_1_bs__p_channel_config_idx);
    if(NULL == pSCCfg)
        return;

    /* Retrieve the certificates */
    pCrtCli = pSCCfg->crt_cli;
    pCrtSrv = pSCCfg->crt_srv;
    if(NULL == pCrtSrv || NULL == pCrtCli)
        return;

    /* TODO: Verify that the server certificate in the Response is the same as the one stored with the SecureChannel */

    /* Retrieve ptrs to ClientNonce and ServerSignature */
    if(session_core_1_bs__p_session != unique_session.id)
        return;
    pSession = &unique_session;
    pNonce = &pSession->NonceClient;
    pSignCandid = &pResp->ServerSignature;

    /* Build CryptoProvider */
    /* TODO: don't create it each time, maybe add it to the session */
    pProvider = CryptoProvider_Create(pSCCfg->reqSecuPolicyUri);

    /* Verify signature algorithm URI */
    szSignUri = CryptoProvider_AsymmetricGetUri_SignAlgorithm(pProvider);
    pSignCandid = &pResp->ServerSignature;
    if(strncmp((char *)pSignCandid->Algorithm.Data, szSignUri, strlen(szSignUri)+1) == 0)
    {
        /* Build signed data (client certificate ++ client nonce) */
        /* a) Get Der of client certificate */
        if(STATUS_OK == KeyManager_Certificate_CopyDER(pCrtCli, &pDerCli, &lenDerCli))
        {
            /* b) Concat Nonce */
            lenToVerify = lenDerCli + LENGTH_NONCE;
            pToVerify = (uint8_t *)malloc(sizeof(uint8_t)*lenToVerify);
            if(NULL != pToVerify)
            {
                /* TODO: The KeyManager API should be changed to avoid these useless copies */
                memcpy(pToVerify, pDerCli, lenDerCli);
                memcpy(pToVerify+lenDerCli, pNonce->Data, pNonce->Length);

                /* Verify given signature */
                /* a) Retrieve public key from certificate */
                if(STATUS_OK == KeyManager_AsymmetricKey_CreateFromCertificate(pCrtSrv, &pKeyCrtSrv))
                {
                    /* b) Call AsymVerify */
                    if(STATUS_OK == CryptoProvider_AsymmetricVerify(pProvider, pToVerify, lenToVerify,
                                                                    pKeyCrtSrv, pSignCandid->Signature.Data, pSignCandid->Signature.Length))
                        *session_core_1_bs__valid = true;
                }
            }
        }
    }

    /* Clear */
    KeyManager_AsymmetricKey_Free(pKeyCrtSrv);
    free(pToVerify);
    free(pDerCli);
    CryptoProvider_Free(pProvider);
}

