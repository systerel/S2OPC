/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*------------------------
   Exported Declarations
  ------------------------*/
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_encodeable.h"
#include "sopc_event_timer_manager.h"
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_secret_buffer.h"
#include "sopc_services_api.h"
#include "sopc_time.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

#include "session_core_bs.h"

#include "channel_mgr_bs.h"

#include "session_core_1.h"

#define LENGTH_NONCE 32

typedef struct SessionData
{
    SOPC_NodeId sessionToken; /* IMPORTANT NOTE: on server side token (numeric value) <=> session index */
    SOPC_ByteString nonceServer;
    SOPC_ByteString nonceClient;           /* TODO: remove ? => no need to be store if returned directly */
    OpcUa_SignatureData signatureData;     /* TODO: remove ? => no need to be stored */
    constants__t_user_i user_server;       /* TODO: remove user management */
    constants__t_user_token_i user_client; /* TODO: remove user management */
} SessionData;

static SessionData sessionDataArray[constants__t_session_i_max + 1]; // index 0 is indet session

static constants__t_application_context_i session_client_app_context[SOPC_MAX_SESSIONS + 1];

static uint32_t session_expiration_timer[SOPC_MAX_SESSIONS + 1];
static uint64_t session_RevisedSessionTimeout[SOPC_MAX_SESSIONS + 1];
static SOPC_TimeReference server_session_latest_msg_receveived[SOPC_MAX_SESSIONS + 1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_bs__INITIALISATION(void)
{
    SessionData* sData = NULL;
    for (int32_t idx = 0; idx <= constants__t_session_i_max; idx++)
    {
        sData = &(sessionDataArray[idx]);
        SOPC_NodeId_Initialize(&sData->sessionToken);
        SOPC_ByteString_Initialize(&sData->nonceClient);
        SOPC_ByteString_Initialize(&sData->nonceServer);
        OpcUa_SignatureData_Initialize(&sData->signatureData);
        sData->user_server = constants__c_user_indet;
        sData->user_client = constants__c_user_token_indet;
    }

    assert(SOPC_MAX_SESSIONS + 1 <= SIZE_MAX / sizeof(constants__t_user_i));
    memset(session_client_app_context, (int) 0, sizeof(constants__t_application_context_i) * (SOPC_MAX_SESSIONS + 1));
    memset(session_expiration_timer, (int) 0, sizeof(uint32_t) * (SOPC_MAX_SESSIONS + 1));
    memset(session_RevisedSessionTimeout, (int) 0, sizeof(uint64_t) * (SOPC_MAX_SESSIONS + 1));
    memset(server_session_latest_msg_receveived, (int) 0, sizeof(SOPC_TimeReference) * (SOPC_MAX_SESSIONS + 1));
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void session_core_bs__notify_set_session_state(const constants__t_session_i session_core_bs__session,
                                               const constants__t_sessionState session_core_bs__prec_state,
                                               const constants__t_sessionState session_core_bs__state,
                                               const t_bool session_core_bs__is_client)
{
    if (session_core_bs__is_client)
    {
        if (session_core_bs__state == constants__e_session_userActivated)
        {
            SOPC_ServicesToApp_EnqueueEvent(SOPC_AppEvent_ComEvent_Create(SE_ACTIVATED_SESSION),
                                            (uint32_t) session_core_bs__session, NULL,
                                            session_client_app_context[session_core_bs__session]);
        }
        else if (session_core_bs__state == constants__e_session_scOrphaned ||
                 ((session_core_bs__state == constants__e_session_userActivating ||
                   session_core_bs__state == constants__e_session_scActivating) &&
                  session_core_bs__prec_state == constants__e_session_userActivated))
        {
            // Session became orphaned OR
            // Already activated session is activating again on a new user or SC

            // if orphaned will be reactivated or closed => notify as reactivating to avoid use of session by
            // application
            SOPC_ServicesToApp_EnqueueEvent(SOPC_AppEvent_ComEvent_Create(SE_SESSION_REACTIVATING),
                                            session_core_bs__session, NULL,
                                            session_client_app_context[session_core_bs__session]);
        }
        else if (session_core_bs__state == constants__e_session_closed)
        {
            if (session_core_bs__prec_state != constants__e_session_closing &&
                session_core_bs__prec_state != constants__e_session_userActivated)
            {
                // If session not in closing state or already activated, it is in activation state regarding user app
                // => notify activation failed
                SOPC_ServicesToApp_EnqueueEvent(
                    SOPC_AppEvent_ComEvent_Create(SE_SESSION_ACTIVATION_FAILURE),
                    session_core_bs__session,                              // session id
                    NULL,                                                  // user ?
                    session_client_app_context[session_core_bs__session]); // user application session context
            }
            else
            {
                // Activated session closing
                SOPC_ServicesToApp_EnqueueEvent(
                    SOPC_AppEvent_ComEvent_Create(SE_CLOSED_SESSION),
                    session_core_bs__session, // session id
                    NULL,
                    session_client_app_context[session_core_bs__session]); // user application session context
            }
        }
    }
}

void session_core_bs__server_get_session_from_token(const constants__t_session_token_i session_core_bs__session_token,
                                                    constants__t_session_i* const session_core_bs__session)
{
    constants__t_session_i result = constants__c_session_indet;
    SOPC_NodeId* requestedToken = session_core_bs__session_token;

    if (requestedToken->IdentifierType == SOPC_IdentifierType_Numeric && requestedToken->Data.Numeric > 0 &&
        requestedToken->Data.Numeric <= INT32_MAX)
    {
        // Note: on server side, token <=> session index
        result = requestedToken->Data.Numeric;
    }

    *session_core_bs__session = result;
}

void session_core_bs__client_get_token_from_session(const constants__t_session_i session_core_bs__session,
                                                    constants__t_session_token_i* const session_core_bs__session_token)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        *session_core_bs__session_token = &(sessionDataArray[session_core_bs__session].sessionToken);
    }
    else
    {
        *session_core_bs__session_token = constants__c_session_token_indet;
    }
}

void session_core_bs__server_get_fresh_session_token(const constants__t_session_i session_core_bs__session,
                                                     constants__t_session_token_i* const session_core_bs__token)
{
    // Important note: on server side, token is session index
    if (constants__c_session_indet != session_core_bs__session)
    {
        // Note: Namespace = 0 for session token ?
        sessionDataArray[session_core_bs__session].sessionToken.IdentifierType = SOPC_IdentifierType_Numeric;
        sessionDataArray[session_core_bs__session].sessionToken.Data.Numeric = session_core_bs__session;
        *session_core_bs__token = &(sessionDataArray[session_core_bs__session].sessionToken);
    }
    else
    {
        *session_core_bs__token = constants__c_session_token_indet;
    }
}

void session_core_bs__server_is_valid_session_token(const constants__t_session_token_i session_core_bs__token,
                                                    t_bool* const session_core_bs__ret)
{
    *session_core_bs__ret = session_core_bs__token != constants__c_session_token_indet;
}

void session_core_bs__client_set_session_token(const constants__t_session_i session_core_bs__session,
                                               const constants__t_session_token_i session_core_bs__token)
{
    SOPC_NodeId* sessionToken = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (constants__c_session_indet != session_core_bs__session &&
        constants__c_session_token_indet != session_core_bs__token)
    {
        sessionToken = &(sessionDataArray[session_core_bs__session].sessionToken);
        status = SOPC_NodeId_Copy(sessionToken, session_core_bs__token);
        if (SOPC_STATUS_OK != status)
        {
            // In case of failure, ensure session token is cleared
            SOPC_NodeId_Clear(sessionToken);
        }
    }
}

void session_core_bs__delete_session_token(const constants__t_session_i session_core_bs__p_session)
{
    SOPC_NodeId_Clear(&sessionDataArray[session_core_bs__p_session].sessionToken);
    SOPC_ExtensionObject_Clear(sessionDataArray[session_core_bs__p_session].user_client);
    free(sessionDataArray[session_core_bs__p_session].user_client);
    sessionDataArray[session_core_bs__p_session].user_client = NULL;
}

void session_core_bs__delete_session_application_context(const constants__t_session_i session_core_bs__p_session)
{
    session_client_app_context[session_core_bs__p_session] = 0;
}

void session_core_bs__is_valid_user(const constants__t_user_i session_core_bs__user, t_bool* const session_core_bs__ret)
{
    assert(session_core_bs__user == 1);
    *session_core_bs__ret = true;
}

void session_core_bs__set_session_user_server(const constants__t_session_i session_core_bs__session,
                                              const constants__t_user_i session_core_bs__p_user)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        sessionDataArray[session_core_bs__session].user_server = session_core_bs__p_user;
    }
}

void session_core_bs__set_session_user_client(const constants__t_session_i session_core_bs__session,
                                              const constants__t_user_token_i session_core_bs__p_user_token)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        sessionDataArray[session_core_bs__session].user_client = session_core_bs__p_user_token;
    }
}

void session_core_bs__get_session_user_server(const constants__t_session_i session_core_bs__session,
                                              constants__t_user_i* const session_core_bs__p_user)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        *session_core_bs__p_user = sessionDataArray[session_core_bs__session].user_server;
    }
    else
    {
        *session_core_bs__p_user = constants__c_user_indet;
    }
}

void session_core_bs__get_session_user_client(const constants__t_session_i session_core_bs__session,
                                              constants__t_user_token_i* const session_core_bs__p_user_token)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        *session_core_bs__p_user_token = sessionDataArray[session_core_bs__session].user_client;
    }
    else
    {
        *session_core_bs__p_user_token = constants__c_user_token_indet;
    }
}

void session_core_bs__server_create_session_req_do_crypto(
    const constants__t_session_i session_core_bs__p_session,
    const constants__t_msg_i session_core_bs__p_req_msg,
    const constants__t_endpoint_config_idx_i session_core_bs__p_endpoint_config_idx,
    const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
    t_bool* const session_core_bs__valid,
    constants__t_SignatureData_i* const session_core_bs__signature)
{
    SOPC_CryptoProvider* pProvider = NULL;
    SOPC_Endpoint_Config* pECfg = NULL;
    SOPC_SecureChannel_Config* pSCCfg = NULL;
    SessionData* pSession = NULL;
    SOPC_ByteString* pNonce = NULL;
    OpcUa_SignatureData* pSign = NULL;
    uint8_t* pToSign = NULL;
    uint32_t lenToSign = 0;
    OpcUa_CreateSessionRequest* pReq = (OpcUa_CreateSessionRequest*) session_core_bs__p_req_msg;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t sigLength = 0;
    SOPC_AsymmetricKey* privateKey = NULL;

    *session_core_bs__valid = false;
    *session_core_bs__signature = constants__c_SignatureData_indet;

    if (constants__c_session_indet == session_core_bs__p_session)
    {
        return;
    }

    pSession = &sessionDataArray[session_core_bs__p_session];

    /* Retrieve the security policy and mode */
    pSCCfg = SOPC_ToolkitServer_GetSecureChannelConfig(session_core_bs__p_channel_config_idx);
    /* Retrieve the server certificate */
    pECfg = SOPC_ToolkitServer_GetEndpointConfig(session_core_bs__p_endpoint_config_idx);

    if (NULL == pSCCfg || NULL == pECfg)
    {
        return;
    }

    /* If security policy is not None, generate the nonce and a signature */
    if (strcmp(pSCCfg->reqSecuPolicyUri, SOPC_SecurityPolicy_None_URI) != 0)
    {
        pNonce = &pSession->nonceServer;
        pSign = &pSession->signatureData;

        /* Create the CryptoProvider */
        /* TODO: don't create it each time, maybe add it to the session */
        pProvider = SOPC_CryptoProvider_Create(pSCCfg->reqSecuPolicyUri);

        /* Ask the CryptoProvider for LENGTH_NONCE random bytes */
        SOPC_ByteString_Clear(pNonce);
        pNonce->Length = LENGTH_NONCE;

        status = SOPC_CryptoProvider_GenerateRandomBytes(pProvider, LENGTH_NONCE, &pNonce->Data);

        /* Use the server private key to sign the client certificate + client nonce */
        /* TODO: check client certificate is the one provided for the Secure Channel */
        /* a) Prepare the buffer to sign */
        if (SOPC_STATUS_OK == status)
        {
            if (pReq->ClientNonce.Length <= 0)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            if (pReq->ClientCertificate.Length >= 0 && pReq->ClientNonce.Length > 0 &&
                (uint64_t) pReq->ClientCertificate.Length + (uint64_t) pReq->ClientNonce.Length <=
                    SIZE_MAX / sizeof(uint8_t))
            {
                lenToSign = (uint32_t) pReq->ClientCertificate.Length + (uint32_t) pReq->ClientNonce.Length;
                pToSign = malloc(sizeof(uint8_t) * (size_t) lenToSign);
            }
            else
            {
                pToSign = NULL;
            }
            if (NULL == pToSign)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            assert(pECfg->serverKey != NULL);
            status = SOPC_KeyManager_SerializedAsymmetricKey_Deserialize(pECfg->serverKey, false, &privateKey);
        }

        if (SOPC_STATUS_OK == status)
        {
            memcpy(pToSign, pReq->ClientCertificate.Data, (size_t) pReq->ClientCertificate.Length);
            memcpy(pToSign + pReq->ClientCertificate.Length, pReq->ClientNonce.Data, (size_t) pReq->ClientNonce.Length);

            /* b) Sign and store the signature in pSign */
            SOPC_ByteString_Clear(&pSign->Signature);
            status = SOPC_CryptoProvider_AsymmetricGetLength_Signature(pProvider, privateKey, &sigLength);
        }

        if (SOPC_STATUS_OK == status)
        {
            assert(sigLength <= INT32_MAX);
            pSign->Signature.Length = (int32_t) sigLength;
            if (pSign->Signature.Length > 0 && (uint64_t) pSign->Signature.Length * sizeof(SOPC_Byte) <= SIZE_MAX)
            {
                /* TODO: This should be freed with session */
                pSign->Signature.Data = malloc(sizeof(SOPC_Byte) * (size_t) pSign->Signature.Length);
            }
            else
            {
                pSign->Signature.Data = NULL;
            }
            if (NULL == pSign->Signature.Data || pSign->Signature.Length <= 0)
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CryptoProvider_AsymmetricSign(pProvider, pToSign, lenToSign, privateKey,
                                                        pSign->Signature.Data, (uint32_t) pSign->Signature.Length);
        }

        SOPC_KeyManager_AsymmetricKey_Free(privateKey);
        privateKey = NULL;

        if (SOPC_STATUS_OK == status)
        {
            /* c) Prepare the OpcUa_SignatureData */
            SOPC_String_Clear(&pSign->Algorithm);

            status = SOPC_String_CopyFromCString(&pSign->Algorithm,
                                                 SOPC_CryptoProvider_AsymmetricGetUri_SignAlgorithm(pProvider));
        }

        if (SOPC_STATUS_OK == status)
        {
            *session_core_bs__signature = pSign;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *session_core_bs__valid = true;
    }

    /* Clean */
    if (NULL != pToSign)
    {
        free(pToSign);
        pToSign = NULL;
    }
    if (NULL != pProvider)
    {
        SOPC_CryptoProvider_Free(pProvider);
        pProvider = NULL;
    }
}

void session_core_bs__clear_Signature(const constants__t_session_i session_core_bs__p_session,
                                      const constants__t_SignatureData_i session_core_bs__p_signature)
{
    // Check same signature since not proved by model
    assert(session_core_bs__p_signature == &sessionDataArray[session_core_bs__p_session].signatureData);
    OpcUa_SignatureData_Clear(&sessionDataArray[session_core_bs__p_session].signatureData);
}

void session_core_bs__client_activate_session_req_do_crypto(
    const constants__t_session_i session_core_bs__session,
    const constants__t_channel_config_idx_i session_core_bs__channel_config_idx,
    const constants__t_Nonce_i session_core_bs__server_nonce,
    t_bool* const session_core_bs__valid,
    constants__t_SignatureData_i* const session_core_bs__signature)
{
    SOPC_CryptoProvider* pProvider = NULL;
    SOPC_SecureChannel_Config* pSCCfg = NULL;
    SessionData* pSession = NULL;
    SOPC_AsymmetricKey* clientKey = NULL;
    SOPC_ByteString* serverNonce = NULL;
    const SOPC_Buffer* serverCert = NULL;
    OpcUa_SignatureData* pSign = NULL;
    uint8_t* pToSign = NULL;
    uint32_t lenToSign = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    *session_core_bs__valid = false;
    *session_core_bs__signature = constants__c_SignatureData_indet;

    if (constants__c_session_indet == session_core_bs__session)
    {
        return;
    }

    pSession = &sessionDataArray[session_core_bs__session];
    /* Retrieve the security policy and mode */
    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(session_core_bs__channel_config_idx);
    if ((NULL == pSCCfg) || (NULL == pSCCfg->crt_srv))
    {
        return;
    }

    /* If security policy is not None, generate the signature */
    if (strcmp(pSCCfg->reqSecuPolicyUri, SOPC_SecurityPolicy_None_URI) != 0) /* Including the terminating \0 */
    {
        /* Retrieve ptr Signature */
        pSign = &pSession->signatureData;

        /* Create the CryptoProvider */
        /* TODO: don't create it each time, maybe add it to the session */
        pProvider = SOPC_CryptoProvider_Create(pSCCfg->reqSecuPolicyUri);

        /* Use the client private key to sign the server certificate + server nonce */
        serverNonce = session_core_bs__server_nonce;
        /* a) Prepare the buffer to sign */
        if (serverNonce->Length <= 0)
        {
            // server Nonce is not present
            status = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            // retrieve expected sender certificate as a ByteString
            serverCert = pSCCfg->crt_srv;
        }

        if (SOPC_STATUS_OK == status && serverCert->length > 0 && serverNonce->Length > 0 &&
            (uint64_t) serverCert->length + (uint64_t) serverNonce->Length <= SIZE_MAX / sizeof(uint8_t))
        {
            lenToSign = (uint32_t) serverCert->length + (uint32_t) serverNonce->Length;
            pToSign = malloc(sizeof(uint8_t) * (size_t) lenToSign);
        }

        if (SOPC_STATUS_OK == status && NULL == pToSign)
        {
            status = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_Deserialize(pSCCfg->key_priv_cli, false, &clientKey);
        }

        if (SOPC_STATUS_OK == status)
        {
            memcpy(pToSign, serverCert->data, (size_t) serverCert->length);
            memcpy(pToSign + serverCert->length, serverNonce->Data, (size_t) serverNonce->Length);
            /* b) Sign and store the signature in pSign */
            SOPC_ByteString_Clear(&pSign->Signature);
            status = SOPC_CryptoProvider_AsymmetricGetLength_Signature(pProvider, clientKey,
                                                                       (uint32_t*) &pSign->Signature.Length);
        }

        if (SOPC_STATUS_OK == status)
        {
            if (pSign->Signature.Length > 0 && (uint64_t) pSign->Signature.Length * sizeof(SOPC_Byte) <= SIZE_MAX)
            {
                pSign->Signature.Data =
                    malloc(sizeof(SOPC_Byte) *
                           (size_t) pSign->Signature.Length); /* TODO: This should not be stored in unique session ? */
            }
            else
            {
                pSign->Signature.Data = NULL;
            }
            if (NULL == pSign->Signature.Data || pSign->Signature.Length <= 0)
            {
                status = SOPC_STATUS_OK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CryptoProvider_AsymmetricSign(pProvider, pToSign, lenToSign, clientKey, pSign->Signature.Data,
                                                        (uint32_t) pSign->Signature.Length);
        }

        SOPC_KeyManager_AsymmetricKey_Free(clientKey);

        /* c) Prepare the OpcUa_SignatureData */
        if (SOPC_STATUS_OK == status)
        {
            SOPC_String_Clear(&pSign->Algorithm);
            status = SOPC_String_CopyFromCString(&pSign->Algorithm,
                                                 SOPC_CryptoProvider_AsymmetricGetUri_SignAlgorithm(pProvider));
        }

        if (SOPC_STATUS_OK == status)
        {
            *session_core_bs__signature = pSign;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *session_core_bs__valid = true;
    }

    /* Clean */
    if (NULL != pToSign)
    {
        free(pToSign);
        pToSign = NULL;
    }
    if (NULL != pProvider)
    {
        SOPC_CryptoProvider_Free(pProvider);
        pProvider = NULL;
    }
}

void session_core_bs__get_NonceServer(const constants__t_session_i session_core_bs__p_session,
                                      constants__t_Nonce_i* const session_core_bs__nonce)
{
    if (constants__c_session_indet != session_core_bs__p_session)
    {
        *session_core_bs__nonce = &(sessionDataArray[session_core_bs__p_session].nonceServer);
    }
    else
    {
        *session_core_bs__nonce = constants__c_Nonce_indet;
    }
}

void session_core_bs__remove_NonceServer(const constants__t_session_i session_core_bs__p_session)
{
    SOPC_ByteString_Clear(&sessionDataArray[session_core_bs__p_session].nonceServer);
}

void session_core_bs__client_create_session_req_do_crypto(
    const constants__t_session_i session_core_bs__p_session,
    const constants__t_channel_i session_core_bs__p_channel,
    const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
    t_bool* const session_core_bs__valid,
    t_bool* const session_core_bs__nonce_needed)
{
    (void) session_core_bs__p_channel;
    /* Produce the Nonce when SC:Sec_pol is not "None" */
    SOPC_CryptoProvider* pProvider = NULL;
    SOPC_SecureChannel_Config* pSCCfg = NULL;
    SessionData* pSession = NULL;
    SOPC_ByteString* pNonce = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    /* Default answer */
    *session_core_bs__valid = false;
    *session_core_bs__nonce_needed = false;

    if (constants__c_session_indet != session_core_bs__p_session)
    {
        pSession = &sessionDataArray[session_core_bs__p_session];

        /* Retrieve the security policy */
        pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(session_core_bs__p_channel_config_idx);
        if (NULL == pSCCfg)
            return;

        /* If security policy is not None, generate the nonce */
        if (strcmp(pSCCfg->reqSecuPolicyUri, SOPC_SecurityPolicy_None_URI) != 0) /* Including the terminating \0 */
        {
            *session_core_bs__nonce_needed = true;
            /* Retrieve ptrs to Nonce and Signature */
            pNonce = &pSession->nonceClient;

            /* Create the CryptoProvider */
            /* TODO: don't create it each time, maybe add it to the session */
            pProvider = SOPC_CryptoProvider_Create(pSCCfg->reqSecuPolicyUri);

            /* Ask the CryptoProvider for LENGTH_NONCE random bytes */
            SOPC_ByteString_Clear(pNonce);
            pNonce->Length = LENGTH_NONCE;

            status = SOPC_CryptoProvider_GenerateRandomBytes(pProvider, LENGTH_NONCE, &pNonce->Data);
            if (SOPC_STATUS_OK != status)
                /* TODO: Should we clean half allocated things? */
                return;

            /* Clean */
            /* TODO: with the many previous returns, you do not always free it */
            SOPC_CryptoProvider_Free(pProvider);
            pProvider = NULL;
        }

        /* Success */
        *session_core_bs__valid = true;
    }
}

void session_core_bs__get_NonceClient(const constants__t_session_i session_core_bs__p_session,
                                      constants__t_Nonce_i* const session_core_bs__nonce)
{
    if (constants__c_session_indet != session_core_bs__p_session)
    {
        *session_core_bs__nonce = &sessionDataArray[session_core_bs__p_session].nonceClient;
    }
    else
    {
        *session_core_bs__nonce = constants__c_Nonce_indet;
    }
}

void session_core_bs__drop_NonceClient(const constants__t_session_i session_core_bs__p_session)
{
    SOPC_ByteString_Clear(&sessionDataArray[session_core_bs__p_session].nonceClient);
}

static SOPC_ReturnStatus check_signature_with_provider(SOPC_CryptoProvider* provider,
                                                       const SOPC_String* requestedSecurityPolicy,
                                                       const SOPC_AsymmetricKey* publicKey,
                                                       const SOPC_Buffer* payload,
                                                       const SOPC_ByteString* nonce,
                                                       const SOPC_String* signature)
{
    /* Verify signature algorithm URI */
    const char* algorithm = SOPC_CryptoProvider_AsymmetricGetUri_SignAlgorithm(provider);

    if (algorithm == NULL ||
        strncmp(algorithm, (const char*) requestedSecurityPolicy->Data, (size_t) requestedSecurityPolicy->Length) !=
            0 ||
        payload->length > (UINT32_MAX - LENGTH_NONCE) || nonce->Length != LENGTH_NONCE)
    {
        return SOPC_STATUS_NOK;
    }

    uint32_t verify_len = payload->length + LENGTH_NONCE;
    uint8_t* verify_payload = calloc(verify_len, sizeof(uint8_t));
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (verify_payload != NULL)
    {
        memcpy(verify_payload, payload->data, payload->length);
        memcpy(verify_payload + payload->length, nonce->Data, LENGTH_NONCE);

        status = SOPC_CryptoProvider_AsymmetricVerify(provider, verify_payload, verify_len, publicKey, signature->Data,
                                                      (uint32_t) signature->Length);
        free(verify_payload);
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    return status;
}

static SOPC_ReturnStatus check_signature(const char* channelSecurityPolicy,
                                         const SOPC_String* requestedSecurityPolicy,
                                         const SOPC_AsymmetricKey* publicKey,
                                         const SOPC_Buffer* payload,
                                         const SOPC_ByteString* nonce,
                                         const SOPC_String* signature)
{
    SOPC_CryptoProvider* provider = SOPC_CryptoProvider_Create(channelSecurityPolicy);

    if (provider == NULL)
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_ReturnStatus status =
        check_signature_with_provider(provider, requestedSecurityPolicy, publicKey, payload, nonce, signature);
    SOPC_CryptoProvider_Free(provider);
    return status;
}

void session_core_bs__client_create_session_check_crypto(
    const constants__t_session_i session_core_bs__p_session,
    const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
    const constants__t_msg_i session_core_bs__p_resp_msg,
    t_bool* const session_core_bs__valid)
{
    const SOPC_SecureChannel_Config* pSCCfg = NULL;
    SessionData* pSession = NULL;
    const OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) session_core_bs__p_resp_msg;
    const OpcUa_SignatureData* pSignCandid = &pResp->ServerSignature;
    SOPC_Certificate* pCrtSrv = NULL;
    SOPC_AsymmetricKey* pKeyCrtSrv = NULL;

    /* Default answer */
    *session_core_bs__valid = false;

    if (constants__c_session_indet == session_core_bs__p_session)
    {
        return;
    }

    pSession = &sessionDataArray[session_core_bs__p_session];

    /* Retrieve the security policy and mode */
    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(session_core_bs__p_channel_config_idx);

    if (NULL == pSCCfg || NULL == pSCCfg->crt_cli || NULL == pSCCfg->crt_srv)
    {
        return;
    }

    /* TODO: Verify that the server certificate in the Response is the same as the one stored with the SecureChannel */

    if (pResp->ServerNonce.Length <= 0 ||
        SOPC_ByteString_Copy(&pSession->nonceServer, &pResp->ServerNonce) != SOPC_STATUS_OK)
    {
        return;
    }

    if (SOPC_KeyManager_SerializedCertificate_Deserialize(pSCCfg->crt_srv, &pCrtSrv) == SOPC_STATUS_OK &&
        SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(pCrtSrv, &pKeyCrtSrv) == SOPC_STATUS_OK &&
        check_signature(pSCCfg->reqSecuPolicyUri, &pSignCandid->Algorithm, pKeyCrtSrv, pSCCfg->crt_cli,
                        &pSession->nonceClient, &pSignCandid->Signature) == SOPC_STATUS_OK)
    {
        *session_core_bs__valid = true;
    }

    SOPC_KeyManager_AsymmetricKey_Free(pKeyCrtSrv);
    SOPC_KeyManager_Certificate_Free(pCrtSrv);
}

void session_core_bs__server_activate_session_check_crypto(
    const constants__t_session_i session_core_bs__session,
    const constants__t_channel_i session_core_bs__channel,
    const constants__t_channel_config_idx_i session_core_bs__channel_config_idx,
    const constants__t_msg_i session_core_bs__activate_req_msg,
    t_bool* const session_core_bs__valid)
{
    (void) session_core_bs__channel;
    const SOPC_SecureChannel_Config* pSCCfg = NULL;
    SOPC_CryptoProvider* provider = NULL;
    SessionData* pSession = NULL;
    SOPC_ByteString* pNonce = NULL;
    const OpcUa_ActivateSessionRequest* pReq = (OpcUa_ActivateSessionRequest*) session_core_bs__activate_req_msg;
    const OpcUa_SignatureData* pSignCandid = &pReq->ClientSignature;
    SOPC_Certificate* pCrtCli = NULL;
    SOPC_AsymmetricKey* pKeyCrtCli = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    /* Default answer */
    *session_core_bs__valid = false;

    if (constants__c_session_indet == session_core_bs__session)
    {
        return;
    }

    pSession = &sessionDataArray[session_core_bs__session];

    /* Retrieve the security policy and mode */
    pSCCfg = SOPC_ToolkitServer_GetSecureChannelConfig(session_core_bs__channel_config_idx);

    if (NULL == pSCCfg || NULL == pSCCfg->crt_cli || NULL == pSCCfg->crt_srv)
    {
        return;
    }

    /* Retrieve ptrs to ServerNonce and ClientSignature */
    pNonce = &pSession->nonceServer;

    if (pNonce == NULL || pNonce->Length <= 0)
    {
        return;
    }

    provider = SOPC_CryptoProvider_Create(pSCCfg->reqSecuPolicyUri);

    if (provider != NULL &&
        SOPC_KeyManager_SerializedCertificate_Deserialize(pSCCfg->crt_cli, &pCrtCli) == SOPC_STATUS_OK &&
        SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(pCrtCli, &pKeyCrtCli) == SOPC_STATUS_OK &&
        check_signature_with_provider(provider, &pSignCandid->Algorithm, pKeyCrtCli, pSCCfg->crt_srv, pNonce,
                                      &pSignCandid->Signature) == SOPC_STATUS_OK)
    {
        *session_core_bs__valid = true;

        // renew the server Nonce
        free(pNonce->Data);
        pNonce->Data = NULL;
        status = SOPC_CryptoProvider_GenerateRandomBytes(provider, (uint32_t) pNonce->Length, &pNonce->Data);
        assert(SOPC_STATUS_OK == status);
    }

    /* Clear */
    SOPC_KeyManager_AsymmetricKey_Free(pKeyCrtCli);
    SOPC_KeyManager_Certificate_Free(pCrtCli);
    SOPC_CryptoProvider_Free(provider);
}

void session_core_bs__client_activate_session_resp_check(
    const constants__t_session_i session_core_bs__p_session,
    const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
    const constants__t_msg_i session_core_bs__p_resp_msg,
    t_bool* const session_core_bs__valid)
{
    (void) session_core_bs__p_session;
    (void) session_core_bs__p_channel_config_idx;
    (void) session_core_bs__p_resp_msg;
    /* TODO: check parameter + add other operation to retrieve new server nonce ?*/
    *session_core_bs__valid = true;
}

void session_core_bs__client_close_session_req_msg(const constants__t_msg_i session_core_bs__req_msg)
{
    (void) session_core_bs__req_msg;
}

void session_core_bs__client_close_session_resp_msg(const constants__t_msg_i session_core_bs__resp_msg)
{
    (void) session_core_bs__resp_msg;
}

void session_core_bs__server_close_session_check_req(const constants__t_msg_i session_core_bs__req_msg,
                                                     const constants__t_msg_i session_core_bs__resp_msg)
{
    (void) session_core_bs__req_msg;
    (void) session_core_bs__resp_msg;
}

void session_core_bs__session_do_nothing(const constants__t_session_i session_core_bs__session)
{
    (void) session_core_bs__session;
}

void session_core_bs__set_session_app_context(const constants__t_session_i session_core_bs__p_session,
                                              const constants__t_application_context_i session_core_bs__p_app_context)
{
    session_client_app_context[session_core_bs__p_session] = session_core_bs__p_app_context;
}

void session_core_bs__client_gen_activate_orphaned_session_internal_event(
    const constants__t_session_i session_core_bs__session,
    const constants__t_channel_config_idx_i session_core_bs__channel_config_idx)
{
    SOPC_Services_EnqueueEvent(SE_TO_SE_ACTIVATE_ORPHANED_SESSION, session_core_bs__session, NULL,
                               session_core_bs__channel_config_idx);
}

void session_core_bs__client_gen_activate_user_session_internal_event(
    const constants__t_session_i session_core_bs__session,
    const constants__t_user_token_i session_core_bs__p_user_token)
{
    SOPC_Services_EnqueueEvent(SE_TO_SE_ACTIVATE_SESSION, session_core_bs__session,
                               (void*) session_core_bs__p_user_token, 0);
}

void session_core_bs__client_gen_create_session_internal_event(
    const constants__t_session_i session_core_bs__session,
    const constants__t_channel_config_idx_i session_core_bs__channel_config_idx)
{
    SOPC_Services_EnqueueEvent(SE_TO_SE_CREATE_SESSION, session_core_bs__session, NULL,
                               session_core_bs__channel_config_idx);
}

void session_core_bs__server_session_timeout_evaluation(const constants__t_session_i session_core_bs__session,
                                                        t_bool* const session_core_bs__expired)
{
    *session_core_bs__expired = true;
    SOPC_TimeReference current = 0;
    SOPC_TimeReference latestMsg = 0;
    SOPC_TimeReference elapsedSinceLatestMsg = 0;
    SOPC_EventDispatcherParams eventParams;
    uint32_t timerId = 0;

    if (constants__c_session_indet != session_core_bs__session)
    {
        session_expiration_timer[session_core_bs__session] = 0; // Do not keep reference on timer that expired
        current = SOPC_TimeReference_GetCurrent();
        latestMsg = server_session_latest_msg_receveived[session_core_bs__session];
        if (current >= latestMsg)
        {
            elapsedSinceLatestMsg = current - latestMsg;
            if (elapsedSinceLatestMsg < session_RevisedSessionTimeout[session_core_bs__session])
            {
                // Session is not expired
                *session_core_bs__expired = false;
                // Re-activate timer for next verification
                eventParams.eltId = session_core_bs__session;
                eventParams.event = TIMER_SE_EVAL_SESSION_TIMEOUT;
                eventParams.params = NULL;
                eventParams.auxParam = 0;
                eventParams.debugName = NULL;
                // Note: next timer is not revised session timeout but revised timeout - latest msg received time
                timerId = SOPC_EventTimer_Create(
                    SOPC_Services_GetEventDispatcher(), eventParams,
                    session_RevisedSessionTimeout[session_core_bs__session] - elapsedSinceLatestMsg);
                session_expiration_timer[session_core_bs__session] = timerId;
                if (0 == timerId)
                {
                    SOPC_Logger_TraceError("Services: session=%" PRId32 " expiration timer renew failed",
                                           session_core_bs__session);
                }
            }
        }
        if (*session_core_bs__expired != false)
        {
            SOPC_Logger_TraceDebug("Services: session=%" PRId32 " expired on timeout evaluation",
                                   session_core_bs__session);
        }
    }
}

void session_core_bs__server_session_timeout_msg_received(const constants__t_session_i session_core_bs__session)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        server_session_latest_msg_receveived[session_core_bs__session] = SOPC_TimeReference_GetCurrent();
    }
}

void session_core_bs__server_session_timeout_start_timer(const constants__t_session_i session_core_bs__session,
                                                         const constants__t_msg_i session_core_bs__resp_msg)
{
    const OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) session_core_bs__resp_msg;
    uint32_t timerId = 0;
    SOPC_EventDispatcherParams eventParams;
    if (constants__c_session_indet != session_core_bs__session)
    {
        if (NULL == pResp || pResp->RevisedSessionTimeout < SOPC_MIN_SESSION_TIMEOUT)
        {
            session_RevisedSessionTimeout[session_core_bs__session] = SOPC_MIN_SESSION_TIMEOUT;
        }
        else
        {
            session_RevisedSessionTimeout[session_core_bs__session] =
                (uint64_t) pResp->RevisedSessionTimeout; // nb milliseconds
        }
        eventParams.eltId = session_core_bs__session;
        eventParams.event = TIMER_SE_EVAL_SESSION_TIMEOUT;
        eventParams.params = NULL;
        eventParams.auxParam = 0;
        eventParams.debugName = NULL;
        timerId = SOPC_EventTimer_Create(SOPC_Services_GetEventDispatcher(), eventParams,
                                         session_RevisedSessionTimeout[session_core_bs__session]);
        session_expiration_timer[session_core_bs__session] = timerId;
        if (0 == timerId)
        {
            SOPC_Logger_TraceError("Services: session=%" PRId32 " expiration timer creation failed",
                                   session_core_bs__session);
        }
    }
}

void session_core_bs__server_session_timeout_stop_timer(const constants__t_session_i session_core_bs__session)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        SOPC_EventTimer_Cancel(session_expiration_timer[session_core_bs__session]);
        session_expiration_timer[session_core_bs__session] = 0;
        session_RevisedSessionTimeout[session_core_bs__session] = 0;
        server_session_latest_msg_receveived[session_core_bs__session] = 0;
    }
}
