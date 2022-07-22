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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_encodeable.h"
#include "sopc_event_handler.h"
#include "sopc_event_timer_manager.h"
#include "sopc_internal_app_dispatcher.h"
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_secret_buffer.h"
#include "sopc_services_api_internal.h"
#include "sopc_time.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"
#include "sopc_user_manager.h"

#include "session_core_bs.h"

#include "channel_mgr_bs.h"
#include "session_core_1.h"
#include "util_b2c.h"
#include "util_user.h"

#define LENGTH_NONCE 32

typedef struct ServerSessionData
{
    SOPC_NodeId sessionToken; /* IMPORTANT NOTE: on server side token (numeric value) <=> session index */
    SOPC_ByteString nonceServer;
    OpcUa_SignatureData signatureData; /* TODO: remove ? => no need to be stored */
    constants__t_user_i user_server;   /* TODO: remove user management */
} ServerSessionData;

typedef struct ClientSessionData
{
    SOPC_NodeId sessionToken; /* IMPORTANT NOTE: on server side token (numeric value) <=> session index */
    SOPC_ByteString nonceServer;
    SOPC_ByteString nonceClient;           /* TODO: remove ? => no need to be store if returned directly */
    OpcUa_SignatureData signatureData;     /* TODO: remove ? => no need to be stored */
    constants__t_user_token_i user_client; /* TODO: remove user management */
    constants__t_SecurityPolicy user_secu_client;
    SOPC_Buffer user_client_server_certificate; // Server certificate for client user encryption
} ClientSessionData;

static ServerSessionData serverSessionDataArray[constants__t_session_i_max + 1]; // index 0 is indet session
static ClientSessionData clientSessionDataArray[constants__t_session_i_max + 1]; // index 0 is indet session

static constants__t_session_application_context_i session_client_app_context[SOPC_MAX_SESSIONS + 1];

static uint32_t session_expiration_timer[SOPC_MAX_SESSIONS + 1];
static uint64_t session_RevisedSessionTimeout[SOPC_MAX_SESSIONS + 1];
static SOPC_TimeReference server_session_latest_msg_receveived[SOPC_MAX_SESSIONS + 1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_bs__INITIALISATION(void)
{
    for (int32_t idx = 0; idx <= constants__t_session_i_max; idx++)
    {
        ServerSessionData* serverSessionData = &(serverSessionDataArray[idx]);
        SOPC_NodeId_Initialize(&serverSessionData->sessionToken);
        SOPC_ByteString_Initialize(&serverSessionData->nonceServer);
        OpcUa_SignatureData_Initialize(&serverSessionData->signatureData);
        serverSessionData->user_server = constants__c_user_indet;

        ClientSessionData* clientSessionData = &(clientSessionDataArray[idx]);
        SOPC_NodeId_Initialize(&clientSessionData->sessionToken);
        SOPC_ByteString_Initialize(&clientSessionData->nonceClient);
        SOPC_ByteString_Initialize(&clientSessionData->nonceServer);
        OpcUa_SignatureData_Initialize(&clientSessionData->signatureData);
        clientSessionData->user_client = constants__c_user_token_indet;
        clientSessionData->user_secu_client = constants__e_secpol_B256S256;
        memset(&clientSessionData->user_client_server_certificate, 0,
               sizeof(clientSessionData->user_client_server_certificate));
    }

    assert(SOPC_MAX_SESSIONS + 1 <= SIZE_MAX / sizeof(constants__t_user_i));
    memset(session_client_app_context, (int) 0,
           sizeof(constants__t_session_application_context_i) * (SOPC_MAX_SESSIONS + 1));
    memset(session_expiration_timer, (int) 0, sizeof(uint32_t) * (SOPC_MAX_SESSIONS + 1));
    memset(session_RevisedSessionTimeout, (int) 0, sizeof(uint64_t) * (SOPC_MAX_SESSIONS + 1));
    memset(server_session_latest_msg_receveived, (int) 0, sizeof(SOPC_TimeReference) * (SOPC_MAX_SESSIONS + 1));
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void session_core_bs__may_validate_server_certificate(
    const constants__t_session_i session_core_bs__p_session,
    const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
    const constants__t_byte_buffer_i session_core_bs__p_user_server_cert,
    const constants__t_SecurityPolicy session_core_bs__p_user_secu_policy,
    t_bool* const session_core_bs__valid_cert)
{
    assert(constants__e_secpol_None != session_core_bs__p_user_secu_policy);

    *session_core_bs__valid_cert = false;
    SOPC_SecureChannel_Config* pSCCfg = NULL;

    /* Retrieve the certificate */
    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(session_core_bs__p_channel_config_idx);

    if (NULL == pSCCfg)
    {
        return;
    }

    if (NULL == pSCCfg->crt_srv)
    {
        if (NULL == pSCCfg->pki)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Services: session=%" PRIu32
                                   " user activation impossible because no PKI available to validate server "
                                   "certificate with channel config %" PRIu32,
                                   session_core_bs__p_session, session_core_bs__p_channel_config_idx);
            return;
        }
        // CreateSessionResponse certificate not validated with SC establishment
        SOPC_CryptoProvider* cp =
            SOPC_CryptoProvider_Create(util_channel__SecurityPolicy_B_to_C(session_core_bs__p_user_secu_policy));
        if (NULL == cp)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "Services: session=%" PRIu32
                " user activation impossible because user security policy invalid using channel config %" PRIu32,
                session_core_bs__p_session, session_core_bs__p_channel_config_idx);
            return;
        }
        SOPC_CertificateList* serverCert = NULL;
        uint32_t errorCode = 0;
        SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(
            session_core_bs__p_user_server_cert->data, session_core_bs__p_user_server_cert->length, &serverCert);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CryptoProvider_Certificate_Validate(cp, pSCCfg->pki, serverCert, &errorCode);
            *session_core_bs__valid_cert = (SOPC_STATUS_OK == status);

            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "Services: session=%" PRIu32
                                       " user activation impossible because server certificate validation failed "
                                       "using channel config %" PRIu32,
                                       session_core_bs__p_session, session_core_bs__p_channel_config_idx);
            }
        }
        SOPC_KeyManager_Certificate_Free(serverCert);
        SOPC_CryptoProvider_Free(cp);

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "Services: session=%" PRIu32
                " user activation impossible because server certificate validation failed using channel config %" PRIu32
                " with error code: %" PRIu32,
                session_core_bs__p_session, session_core_bs__p_channel_config_idx, errorCode);
        }
    }
    else
    {
        // If SC certificate is not NULL, it is already validated during SC establishment and
        // CreateSessionResponse certificate 'p_user_server_cert' is already verified to be the same
        // (see 'create_session_resp_check_server_certificate')
        *session_core_bs__valid_cert = true;
    }
}

void session_core_bs__notify_set_session_state(
    const constants__t_session_i session_core_bs__session,
    const constants__t_sessionState session_core_bs__prec_state,
    const constants__t_sessionState session_core_bs__state,
    const constants_statuscodes_bs__t_StatusCode_i session_core_bs__sc_reason,
    const t_bool session_core_bs__is_client)
{
    if (session_core_bs__is_client)
    {
        if (NULL == session_client_app_context[session_core_bs__session])
        {
            // Note: this case should be guaranteed by B model but it needs some B model refactoring
            return;
        }

        /* CLIENT SIDE ONLY */
        if (session_core_bs__state == constants__e_session_userActivated)
        {
            SOPC_App_EnqueueComEvent(SE_ACTIVATED_SESSION, (uint32_t) session_core_bs__session, (uintptr_t) NULL,
                                     session_client_app_context[session_core_bs__session]->userSessionContext);
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
            SOPC_App_EnqueueComEvent(SE_SESSION_REACTIVATING, session_core_bs__session, (uintptr_t) NULL,
                                     session_client_app_context[session_core_bs__session]->userSessionContext);
        }
        else if (session_core_bs__state == constants__e_session_closed)
        {
            SOPC_StatusCode scReason;
            util_status_code__B_to_C(session_core_bs__sc_reason, &scReason);
            if (session_core_bs__prec_state != constants__e_session_closing &&
                session_core_bs__prec_state != constants__e_session_userActivated)
            {
                // If session not in closing state or already activated, it is in activation state regarding user app
                // => notify activation failed
                // Note: the user application context might be invalid in some cases
                //       (session closed by application using "invalid" index)
                if (NULL != session_client_app_context[session_core_bs__session])
                {
                    SOPC_App_EnqueueComEvent(SE_SESSION_ACTIVATION_FAILURE, session_core_bs__session,
                                             (uintptr_t) scReason,
                                             session_client_app_context[session_core_bs__session]
                                                 ->userSessionContext); // user application session context
                }
            }
            else
            {
                // Activated session closing
                SOPC_App_EnqueueComEvent(SE_CLOSED_SESSION, session_core_bs__session, (uintptr_t) scReason,
                                         session_client_app_context[session_core_bs__session]
                                             ->userSessionContext); // user application session context
            }
        }
    }
    else
    {
        /* SERVER SIDE ONLY */
        if ((session_core_bs__prec_state == constants__e_session_userActivated &&
             session_core_bs__state != constants__e_session_userActivated) ||
            session_core_bs__state == constants__e_session_closed)
        {
            // Session was active and is not anymore or session is closed:
            // - Session becomes inactive: subscription will clear session context only (publish requests)
            // - Session is closed: subscription will be closed
            SOPC_EventHandler_PostAsNext(SOPC_Services_GetEventHandler(), SE_TO_SE_SERVER_INACTIVATED_SESSION_PRIO,
                                         (uint32_t) session_core_bs__session, (uintptr_t) NULL,
                                         (uintptr_t) session_core_bs__state);
        }

        if (session_core_bs__state == constants__e_session_closed &&
            constants_statuscodes_bs__e_sc_ok != session_core_bs__sc_reason)
        {
            SOPC_StatusCode scReason;
            util_status_code__B_to_C(session_core_bs__sc_reason, &scReason);
            SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                                  "Services: session=%" PRIu32 " closed with bad status code '%X'",
                                  session_core_bs__session, scReason);
        }
    }
}

void session_core_bs__server_get_session_from_token(const constants__t_session_token_i session_core_bs__session_token,
                                                    constants__t_session_i* const session_core_bs__session)
{
    constants__t_session_i result = constants__c_session_indet;
    SOPC_NodeId* requestedToken = session_core_bs__session_token;
    int32_t comparison = 1;

    for (uint32_t idx = (uint32_t) constants__t_session_i_max; constants__c_session_indet == result && idx > 0; idx--)
    {
        SOPC_ReturnStatus status =
            SOPC_NodeId_Compare(&serverSessionDataArray[idx].sessionToken, requestedToken, &comparison);
        if (SOPC_STATUS_OK == status && 0 == comparison)
        {
            result = idx;
        }
    }

    *session_core_bs__session = result;
}

void session_core_bs__client_get_token_from_session(const constants__t_session_i session_core_bs__session,
                                                    constants__t_session_token_i* const session_core_bs__session_token)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        *session_core_bs__session_token = &(clientSessionDataArray[session_core_bs__session].sessionToken);
    }
    else
    {
        *session_core_bs__session_token = constants__c_session_token_indet;
    }
}

void session_core_bs__server_get_fresh_session_token(
    const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
    const constants__t_session_i session_core_bs__session,
    constants__t_session_token_i* const session_core_bs__token)
{
    SOPC_CryptoProvider* pProvider = NULL;
    SOPC_SecureChannel_Config* pSCCfg = NULL;

    // Important note: on server side, token is session index
    if (constants__c_session_indet != session_core_bs__session)
    {
        pSCCfg = SOPC_ToolkitServer_GetSecureChannelConfig(session_core_bs__p_channel_config_idx);
        if (NULL != pSCCfg)
        {
            pProvider = SOPC_CryptoProvider_Create(pSCCfg->reqSecuPolicyUri);
        }
        // Note: Namespace = 0 for session token ?
        serverSessionDataArray[session_core_bs__session].sessionToken.IdentifierType = SOPC_IdentifierType_Numeric;
        SOPC_ReturnStatus status = NULL != pProvider ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CryptoProvider_GenerateRandomID(
                pProvider, &serverSessionDataArray[session_core_bs__session].sessionToken.Data.Numeric);
        }
        if (SOPC_STATUS_OK != status)
        {
            serverSessionDataArray[session_core_bs__session].sessionToken.Data.Numeric = session_core_bs__session;
        }
        *session_core_bs__token = &(serverSessionDataArray[session_core_bs__session].sessionToken);

        SOPC_CryptoProvider_Free(pProvider);
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

void session_core_bs__client_set_NonceServer(const constants__t_session_i session_core_bs__p_session,
                                             const constants__t_msg_i session_core_bs__p_resp_msg)
{
    assert(constants__c_session_indet != session_core_bs__p_session);

    const OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) session_core_bs__p_resp_msg;
    // Shallow copy of server nonce
    clientSessionDataArray[session_core_bs__p_session].nonceServer = pResp->ServerNonce;
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    // Remove nonce content from create session resp message
    SOPC_ByteString_Initialize((SOPC_ByteString*) &pResp->ServerNonce);
    SOPC_GCC_DIAGNOSTIC_RESTORE
}

void session_core_bs__client_set_session_token(const constants__t_session_i session_core_bs__session,
                                               const constants__t_session_token_i session_core_bs__token)
{
    SOPC_NodeId* sessionToken = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (constants__c_session_indet != session_core_bs__session &&
        constants__c_session_token_indet != session_core_bs__token)
    {
        sessionToken = &(clientSessionDataArray[session_core_bs__session].sessionToken);
        status = SOPC_NodeId_Copy(sessionToken, session_core_bs__token);
        if (SOPC_STATUS_OK != status)
        {
            // In case of failure, ensure session token is cleared
            SOPC_NodeId_Clear(sessionToken);
        }
    }
}

void session_core_bs__delete_session_token(const constants__t_session_i session_core_bs__p_session,
                                           const t_bool session_core_bs__p_is_client)
{
    assert(constants__c_session_indet != session_core_bs__p_session);
    if (session_core_bs__p_is_client)
    {
        ClientSessionData* sData = &clientSessionDataArray[session_core_bs__p_session];
        SOPC_NodeId_Clear(&sData->sessionToken);
        SOPC_ExtensionObject_Clear(sData->user_client);
        SOPC_Free(sData->user_client);
        sData->user_client = NULL;
        sData->user_secu_client = constants__e_secpol_B256S256;
        SOPC_Buffer_Clear(&sData->user_client_server_certificate);
    }
    else
    {
        ServerSessionData* sData = &serverSessionDataArray[session_core_bs__p_session];
        SOPC_NodeId_Clear(&sData->sessionToken);
    }
}

void session_core_bs__delete_session_application_context(const constants__t_session_i session_core_bs__p_session)
{
    SOPC_Internal_SessionAppContext* sessionAppCtx = session_client_app_context[session_core_bs__p_session];
    if (NULL != sessionAppCtx)
    {
        SOPC_Free(sessionAppCtx->sessionName);
        SOPC_Free(sessionAppCtx);
    }
    session_client_app_context[session_core_bs__p_session] = NULL;
}

void session_core_bs__drop_user_server(const constants__t_session_i session_core_bs__p_session)
{
    SOPC_UserWithAuthorization* userauthz = serverSessionDataArray[session_core_bs__p_session].user_server;
    SOPC_UserWithAuthorization_Free(&userauthz);
    serverSessionDataArray[session_core_bs__p_session].user_server = NULL;
}

void session_core_bs__set_session_user_server(const constants__t_session_i session_core_bs__session,
                                              const constants__t_user_i session_core_bs__p_user)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        serverSessionDataArray[session_core_bs__session].user_server = session_core_bs__p_user;
    }
}

void session_core_bs__set_session_user_client(const constants__t_session_i session_core_bs__session,
                                              const constants__t_user_token_i session_core_bs__p_user_token)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        clientSessionDataArray[session_core_bs__session].user_client = session_core_bs__p_user_token;
    }
}

void session_core_bs__get_session_user_server(const constants__t_session_i session_core_bs__session,
                                              constants__t_user_i* const session_core_bs__p_user)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        *session_core_bs__p_user = serverSessionDataArray[session_core_bs__session].user_server;
    }
    else
    {
        *session_core_bs__p_user = constants__c_user_indet;
    }
}

void session_core_bs__get_session_user_server_certificate(
    const constants__t_session_i session_core_bs__session,
    constants__t_byte_buffer_i* const session_core_bs__p_user_server_cert)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        *session_core_bs__p_user_server_cert =
            &clientSessionDataArray[session_core_bs__session].user_client_server_certificate;
    }
    else
    {
        *session_core_bs__p_user_server_cert = constants__c_user_indet;
    }
}

void session_core_bs__is_same_user_server(const constants__t_user_i session_core_bs__p_user_left,
                                          const constants__t_user_i session_core_bs__p_user_right,
                                          t_bool* const session_core_bs__p_bres)
{
    *session_core_bs__p_bres = SOPC_User_Equal(SOPC_UserWithAuthorization_GetUser(session_core_bs__p_user_left),
                                               SOPC_UserWithAuthorization_GetUser(session_core_bs__p_user_right));
    return;
}

void session_core_bs__get_session_user_client(const constants__t_session_i session_core_bs__session,
                                              constants__t_user_token_i* const session_core_bs__p_user_token)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        *session_core_bs__p_user_token = clientSessionDataArray[session_core_bs__session].user_client;
    }
    else
    {
        *session_core_bs__p_user_token = constants__c_user_token_indet;
    }
}

void session_core_bs__get_session_user_secu_client(const constants__t_session_i session_core_bs__session,
                                                   constants__t_SecurityPolicy* const session_core_bs__p_user_secu)
{
    if (constants__c_session_indet != session_core_bs__session)
    {
        *session_core_bs__p_user_secu = clientSessionDataArray[session_core_bs__session].user_secu_client;
    }
    else
    {
        *session_core_bs__p_user_secu = constants__e_secpol_B256S256;
    }
}

/* Sets *ok to true only when the appUri could be checked correctly and it is correct. Otherwise sets it to false.
 * Returns SOPC_STATUS_OK if the appUri is empty.
 * Otherwise, returns an error code when the certData is empty or the creation of the certificate fails.
 */
static SOPC_ReturnStatus check_application_uri(const SOPC_ByteString* certData, const SOPC_String* appUri, bool* ok)
{
    SOPC_CertificateList* certificate = NULL;

    *ok = false;

    if (appUri->Length <= 0)
    {
        return SOPC_STATUS_OK;
    }
    if (certData->Length <= 0)
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_ReturnStatus status =
        SOPC_KeyManager_Certificate_CreateOrAddFromDER(certData->Data, (uint32_t) certData->Length, &certificate);

    if (status != SOPC_STATUS_OK)
    {
        return status;
    }

    *ok = SOPC_KeyManager_Certificate_CheckApplicationUri(certificate, SOPC_String_GetRawCString(appUri));
    SOPC_KeyManager_Certificate_Free(certificate);

    return SOPC_STATUS_OK;
}

void session_core_bs__server_create_session_req_do_crypto(
    const constants__t_session_i session_core_bs__p_session,
    const constants__t_msg_i session_core_bs__p_req_msg,
    const constants__t_endpoint_config_idx_i session_core_bs__p_endpoint_config_idx,
    const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
    constants_statuscodes_bs__t_StatusCode_i* const session_core_bs__status,
    constants__t_SignatureData_i* const session_core_bs__signature)
{
    SOPC_CryptoProvider* pProvider = NULL;
    SOPC_Endpoint_Config* pECfg = NULL;
    SOPC_SecureChannel_Config* pSCCfg = NULL;
    ServerSessionData* pSession = NULL;
    OpcUa_SignatureData* pSign = NULL;
    uint8_t* pToSign = NULL;
    uint32_t lenToSign = 0;
    OpcUa_CreateSessionRequest* pReq = (OpcUa_CreateSessionRequest*) session_core_bs__p_req_msg;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t sigLength = 0;
    SOPC_AsymmetricKey* privateKey = NULL;
    const char* errorReason = "";

    *session_core_bs__signature = constants__c_SignatureData_indet;

    if (constants__c_session_indet == session_core_bs__p_session)
    {
        *session_core_bs__status = constants_statuscodes_bs__e_sc_bad_unexpected_error;
        return;
    }

    pSession = &serverSessionDataArray[session_core_bs__p_session];

    /* Retrieve the security policy and mode */
    pSCCfg = SOPC_ToolkitServer_GetSecureChannelConfig(session_core_bs__p_channel_config_idx);
    /* Retrieve the server certificate */
    pECfg = SOPC_ToolkitServer_GetEndpointConfig(session_core_bs__p_endpoint_config_idx);

    if (NULL == pSCCfg || NULL == pECfg)
    {
        *session_core_bs__status = constants_statuscodes_bs__e_sc_bad_unexpected_error;
        return;
    }

    /* If security policy is not None, generate the signature */
    if (SOPC_STATUS_OK == status && strcmp(pSCCfg->reqSecuPolicyUri, SOPC_SecurityPolicy_None_URI) != 0)
    {
        pSign = &pSession->signatureData;

        if (pReq->ClientNonce.Length < LENGTH_NONCE)
        {
            *session_core_bs__status = constants_statuscodes_bs__e_sc_bad_nonce_invalid;
            return;
        }

        /* Create the CryptoProvider */
        /* TODO: don't create it each time, maybe add it to the session */
        pProvider = SOPC_CryptoProvider_Create(pSCCfg->reqSecuPolicyUri);

        /* Use the server private key to sign the client certificate + client nonce */
        /* TODO: check client certificate is the one provided for the Secure Channel */
        /* a) Prepare the buffer to sign */

        if (pReq->ClientCertificate.Length >= 0 && pReq->ClientNonce.Length > 0 &&
            (uint64_t) pReq->ClientCertificate.Length + (uint64_t) pReq->ClientNonce.Length <=
                SIZE_MAX / sizeof(uint8_t))
        {
            lenToSign = (uint32_t) pReq->ClientCertificate.Length + (uint32_t) pReq->ClientNonce.Length;
            pToSign = SOPC_Malloc(sizeof(uint8_t) * (size_t) lenToSign);
        }
        else
        {
            pToSign = NULL;
        }
        if (NULL == pToSign)
        {
            status = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            assert(pECfg->serverConfigPtr->serverKey != NULL);
            status = SOPC_KeyManager_SerializedAsymmetricKey_Deserialize(pECfg->serverConfigPtr->serverKey, false,
                                                                         &privateKey);
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
                pSign->Signature.Data = SOPC_Malloc(sizeof(SOPC_Byte) * (size_t) pSign->Signature.Length);
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
            status =
                SOPC_CryptoProvider_AsymmetricSign(pProvider, pToSign, lenToSign, privateKey, pSign->Signature.Data,
                                                   (uint32_t) pSign->Signature.Length, &errorReason);
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

        bool application_uri_ok = false;

        if (SOPC_STATUS_OK == status)
        {
            status = check_application_uri(&pReq->ClientCertificate, &pReq->ClientDescription.ApplicationUri,
                                           &application_uri_ok);
        }

        if (SOPC_STATUS_OK == status)
        {
            *session_core_bs__status = application_uri_ok ? constants_statuscodes_bs__e_sc_ok
                                                          : constants_statuscodes_bs__e_sc_bad_certificate_uri_invalid;
        }
        else
        {
            *session_core_bs__status = constants_statuscodes_bs__e_sc_bad_unexpected_error;
        }
    }
    else
    {
        *session_core_bs__status = constants_statuscodes_bs__e_sc_ok;
    }

    /* Clean */
    if (constants_statuscodes_bs__e_sc_ok == *session_core_bs__status)
    {
        *session_core_bs__signature = pSign;
    }
    else
    {
        OpcUa_SignatureData_Clear(pSign);
    }

    if (NULL != pToSign)
    {
        SOPC_Free(pToSign);
        pToSign = NULL;
    }
    if (NULL != pProvider)
    {
        SOPC_CryptoProvider_Free(pProvider);
        pProvider = NULL;
    }
}

void session_core_bs__clear_Signature(const constants__t_session_i session_core_bs__p_session,
                                      const t_bool session_core_bs__p_is_client,
                                      const constants__t_SignatureData_i session_core_bs__p_signature)
{
    OpcUa_SignatureData* signature = NULL;
    if (session_core_bs__p_is_client)
    {
        signature = &clientSessionDataArray[session_core_bs__p_session].signatureData;
    }
    else
    {
        signature = &serverSessionDataArray[session_core_bs__p_session].signatureData;
    }
    // Check same signature since not proved by model
    assert(session_core_bs__p_signature == signature);
    OpcUa_SignatureData_Clear(signature);
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
    ClientSessionData* pSession = NULL;
    SOPC_AsymmetricKey* clientKey = NULL;
    SOPC_ByteString* serverNonce = NULL;
    const SOPC_Buffer* serverCert = NULL;
    OpcUa_SignatureData* pSign = NULL;
    uint8_t* pToSign = NULL;
    uint32_t lenToSign = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const char* errorReason = "";

    *session_core_bs__valid = false;
    *session_core_bs__signature = constants__c_SignatureData_indet;

    if (constants__c_session_indet == session_core_bs__session)
    {
        return;
    }

    pSession = &clientSessionDataArray[session_core_bs__session];
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
            pToSign = SOPC_Malloc(sizeof(uint8_t) * (size_t) lenToSign);
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
                pSign->Signature.Data = SOPC_Malloc(
                    sizeof(SOPC_Byte) *
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
                                                        (uint32_t) pSign->Signature.Length, &errorReason);
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
        SOPC_Free(pToSign);
        pToSign = NULL;
    }
    if (NULL != pProvider)
    {
        SOPC_CryptoProvider_Free(pProvider);
        pProvider = NULL;
    }
}

void session_core_bs__get_NonceServer(const constants__t_session_i session_core_bs__p_session,
                                      const t_bool session_core_bs__p_is_client,
                                      constants__t_Nonce_i* const session_core_bs__nonce)
{
    if (constants__c_session_indet != session_core_bs__p_session)
    {
        if (session_core_bs__p_is_client)
        {
            *session_core_bs__nonce = &clientSessionDataArray[session_core_bs__p_session].nonceServer;
        }
        else
        {
            *session_core_bs__nonce = &serverSessionDataArray[session_core_bs__p_session].nonceServer;
        }
    }
    else
    {
        *session_core_bs__nonce = constants__c_Nonce_indet;
    }
}

void session_core_bs__remove_NonceServer(const constants__t_session_i session_core_bs__p_session,
                                         const t_bool session_core_bs__p_is_client)
{
    SOPC_ByteString* nonce = NULL;
    if (session_core_bs__p_is_client)
    {
        nonce = &clientSessionDataArray[session_core_bs__p_session].nonceServer;
    }
    else
    {
        nonce = &serverSessionDataArray[session_core_bs__p_session].nonceServer;
    }

    SOPC_ByteString_Clear(nonce);
}

void session_core_bs__client_create_session_req_do_crypto(
    const constants__t_session_i session_core_bs__p_session,
    const constants__t_channel_i session_core_bs__p_channel,
    const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
    t_bool* const session_core_bs__valid,
    t_bool* const session_core_bs__nonce_needed)
{
    SOPC_UNUSED_ARG(session_core_bs__p_channel);
    /* Produce the Nonce when SC:Sec_pol is not "None" */
    SOPC_CryptoProvider* pProvider = NULL;
    SOPC_SecureChannel_Config* pSCCfg = NULL;
    ClientSessionData* pSession = NULL;
    SOPC_ByteString* pNonce = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    /* Default answer */
    *session_core_bs__valid = false;
    *session_core_bs__nonce_needed = false;

    if (constants__c_session_indet != session_core_bs__p_session)
    {
        pSession = &clientSessionDataArray[session_core_bs__p_session];

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

void session_core_bs__client_create_session_set_user_token_secu_properties(
    const constants__t_session_i session_core_bs__p_session,
    const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
    const constants__t_msg_i session_core_bs__p_resp_msg,
    t_bool* const session_core_bs__p_valid)
{
    *session_core_bs__p_valid = false;
    const OpcUa_CreateSessionResponse* createSessionRespMsg = session_core_bs__p_resp_msg;
    /* Retrieve the secure channel configuration and provided unencrypted user token */
    const SOPC_SecureChannel_Config* scConfig =
        SOPC_ToolkitClient_GetSecureChannelConfig(session_core_bs__p_channel_config_idx);
    constants__t_user_token_i user_token = constants__c_user_token_indet;
    session_core_bs__get_session_user_client(session_core_bs__p_session, &user_token);
    if (NULL == scConfig || constants__c_user_token_indet == user_token)
    {
        return;
    }
    /* Retrieve the requested policyId from user token */
    constants__t_user_token_type_i user_token_type = util_get_user_token_type_from_token(user_token);

    bool foundUserSecuPolicy = false;
    constants__t_SecurityPolicy usedSecPol = constants__e_secpol_B256S256;

    /* Find the user token security policy for the requested policyId */
    for (int32_t epIdx = 0; epIdx < createSessionRespMsg->NoOfServerEndpoints && !foundUserSecuPolicy; epIdx++)
    {
        const OpcUa_EndpointDescription* epDesc = &createSessionRespMsg->ServerEndpoints[epIdx];
        if (0 == strcmp(scConfig->reqSecuPolicyUri, SOPC_String_GetRawCString(&epDesc->SecurityPolicyUri)) &&
            scConfig->msgSecurityMode == epDesc->SecurityMode)
        {
            for (int32_t userPolicyIdx = 0; userPolicyIdx < epDesc->NoOfUserIdentityTokens && !foundUserSecuPolicy;
                 userPolicyIdx++)
            {
                foundUserSecuPolicy =
                    util_check_user_token_policy_compliance(scConfig, &epDesc->UserIdentityTokens[userPolicyIdx],
                                                            user_token_type, user_token, false, &usedSecPol);
            }
        }
    }

    if (foundUserSecuPolicy)
    {
        if ((NULL == createSessionRespMsg->ServerCertificate.Data ||
             0 >= createSessionRespMsg->ServerCertificate.Length) &&
            constants__e_secpol_None != usedSecPol)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "Services: session=%" PRIu32
                " session activation aborted due to missing server certificate in CreateSessionResponse",
                session_core_bs__p_session);
            return;
        }

        // Record the user security policy used for this session (user token security policy or secure channel if empty)
        clientSessionDataArray[session_core_bs__p_session].user_secu_client = usedSecPol;

        // Shallow copy of server certificate
        clientSessionDataArray[session_core_bs__p_session].user_client_server_certificate.data =
            createSessionRespMsg->ServerCertificate.Data;
        clientSessionDataArray[session_core_bs__p_session].user_client_server_certificate.length =
            (uint32_t) createSessionRespMsg->ServerCertificate.Length;

        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        // Remove certificate content from create session resp message
        SOPC_ByteString_Initialize((SOPC_ByteString*) &createSessionRespMsg->ServerCertificate);
        SOPC_GCC_DIAGNOSTIC_RESTORE
        *session_core_bs__p_valid = true;
    }
    else
    {
        const char* userPolicyId = NULL;
        switch (user_token_type)
        {
        case constants__e_userTokenType_anonymous:
            userPolicyId =
                SOPC_String_GetRawCString(&((OpcUa_AnonymousIdentityToken*) user_token->Body.Object.Value)->PolicyId);
            break;
        case constants__e_userTokenType_userName:
            userPolicyId =
                SOPC_String_GetRawCString(&((OpcUa_UserNameIdentityToken*) user_token->Body.Object.Value)->PolicyId);
            break;
        case constants__e_userTokenType_x509:
            userPolicyId =
                SOPC_String_GetRawCString(&((OpcUa_X509IdentityToken*) user_token->Body.Object.Value)->PolicyId);
            break;
        case constants__e_userTokenType_issued:
            userPolicyId =
                SOPC_String_GetRawCString(&((OpcUa_IssuedIdentityToken*) user_token->Body.Object.Value)->PolicyId);
            break;
        default:
            userPolicyId = "<invalid user token type>";
            break;
        }

        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Services: session=%" PRIu32
                               " session activation aborted due to incompatible PolicyId '%s' requested by user",
                               session_core_bs__p_session, userPolicyId);
    }
}

void session_core_bs__get_NonceClient(const constants__t_session_i session_core_bs__p_session,
                                      constants__t_Nonce_i* const session_core_bs__nonce)
{
    if (constants__c_session_indet != session_core_bs__p_session)
    {
        *session_core_bs__nonce = &clientSessionDataArray[session_core_bs__p_session].nonceClient;
    }
    else
    {
        *session_core_bs__nonce = constants__c_Nonce_indet;
    }
}

void session_core_bs__drop_NonceClient(const constants__t_session_i session_core_bs__p_session)
{
    SOPC_ByteString_Clear(&clientSessionDataArray[session_core_bs__p_session].nonceClient);
}

static SOPC_ReturnStatus check_signature_with_provider(SOPC_CryptoProvider* provider,
                                                       const SOPC_String* requestedSecurityPolicy,
                                                       const SOPC_AsymmetricKey* publicKey,
                                                       const SOPC_Buffer* payload,
                                                       const SOPC_ByteString* nonce,
                                                       const SOPC_String* signature)
{
    assert(NULL != provider);
    assert(NULL != requestedSecurityPolicy);
    assert(NULL != requestedSecurityPolicy->Data);
    assert(requestedSecurityPolicy->Length > 0);
    assert(NULL != payload);
    assert(NULL != nonce);
    assert(NULL != nonce->Data);
    assert(LENGTH_NONCE == nonce->Length);
    assert(NULL != signature);
    assert(NULL != signature->Data);
    assert(signature->Length > 0);
    const char* errorReason = "";

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
    uint8_t* verify_payload = SOPC_Calloc(verify_len, sizeof(uint8_t));
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (verify_payload != NULL)
    {
        memcpy(verify_payload, payload->data, payload->length);
        memcpy(verify_payload + payload->length, nonce->Data, LENGTH_NONCE);

        status = SOPC_CryptoProvider_AsymmetricVerify(provider, verify_payload, verify_len, publicKey, signature->Data,
                                                      (uint32_t) signature->Length, &errorReason);
        SOPC_Free(verify_payload);
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
    ClientSessionData* pSession = NULL;
    const OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) session_core_bs__p_resp_msg;
    const OpcUa_SignatureData* pSignCandid = &pResp->ServerSignature;
    SOPC_CertificateList* pCrtSrv = NULL;
    SOPC_AsymmetricKey* pKeyCrtSrv = NULL;

    /* Default answer */
    *session_core_bs__valid = false;

    if (constants__c_session_indet == session_core_bs__p_session)
    {
        return;
    }

    pSession = &clientSessionDataArray[session_core_bs__p_session];

    /* Check server signature algorithm is not empty */
    if (NULL == pSignCandid->Algorithm.Data || pSignCandid->Algorithm.Length <= 0)
    {
        return;
    }

    /* Check server signature is not empty */
    if (NULL == pSignCandid->Signature.Data || pSignCandid->Signature.Length <= 0)
    {
        return;
    }

    /* Retrieve the security policy and mode */
    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(session_core_bs__p_channel_config_idx);

    if (NULL == pSCCfg || NULL == pSCCfg->crt_cli || NULL == pSCCfg->crt_srv)
    {
        return;
    }

    /* TODO: Verify that the server certificate in the Response is the same as the one stored with the SecureChannel */

    if (pResp->ServerNonce.Length < LENGTH_NONCE ||
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
    SOPC_UNUSED_ARG(session_core_bs__channel);
    const SOPC_SecureChannel_Config* pSCCfg = NULL;
    SOPC_CryptoProvider* provider = NULL;
    ServerSessionData* pSession = NULL;
    SOPC_ByteString* pNonce = NULL;
    const OpcUa_ActivateSessionRequest* pReq = (OpcUa_ActivateSessionRequest*) session_core_bs__activate_req_msg;
    const OpcUa_SignatureData* pSignCandid = &pReq->ClientSignature;
    SOPC_CertificateList* pCrtCli = NULL;
    SOPC_AsymmetricKey* pKeyCrtCli = NULL;

    /* Default answer */
    *session_core_bs__valid = false;

    if (constants__c_session_indet == session_core_bs__session)
    {
        return;
    }

    pSession = &serverSessionDataArray[session_core_bs__session];

    /* Check client signature algorithm is not empty */
    if (NULL == pSignCandid->Algorithm.Data || pSignCandid->Algorithm.Length <= 0)
    {
        return;
    }
    /* Check client signature is not empty */
    if (NULL == pSignCandid->Signature.Data || pSignCandid->Signature.Length <= 0)
    {
        return;
    }

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
    SOPC_UNUSED_ARG(session_core_bs__p_session);
    SOPC_UNUSED_ARG(session_core_bs__p_channel_config_idx);
    SOPC_UNUSED_ARG(session_core_bs__p_resp_msg);
    /* TODO: check parameter + add other operation to retrieve new server nonce ?*/
    *session_core_bs__valid = true;
}

void session_core_bs__client_close_session_req_msg(const constants__t_msg_i session_core_bs__req_msg)
{
    SOPC_UNUSED_ARG(session_core_bs__req_msg);
}

void session_core_bs__client_close_session_resp_msg(const constants__t_msg_i session_core_bs__resp_msg)
{
    SOPC_UNUSED_ARG(session_core_bs__resp_msg);
}

void session_core_bs__server_close_session_check_req(const constants__t_msg_i session_core_bs__req_msg,
                                                     const constants__t_msg_i session_core_bs__resp_msg)
{
    SOPC_UNUSED_ARG(session_core_bs__req_msg);
    SOPC_UNUSED_ARG(session_core_bs__resp_msg);
}

void session_core_bs__session_do_nothing(const constants__t_session_i session_core_bs__session)
{
    SOPC_UNUSED_ARG(session_core_bs__session);
}

void session_core_bs__set_session_app_context(
    const constants__t_session_i session_core_bs__p_session,
    const constants__t_session_application_context_i session_core_bs__p_app_context)
{
    session_client_app_context[session_core_bs__p_session] = session_core_bs__p_app_context;
}

void session_core_bs__get_session_app_context(
    const constants__t_session_i session_core_bs__p_session,
    constants__t_session_application_context_i* const session_core_bs__p_app_context)
{
    assert(constants__c_session_indet != session_core_bs__p_session);
    *session_core_bs__p_app_context = session_client_app_context[session_core_bs__p_session];
}

void session_core_bs__client_gen_activate_orphaned_session_internal_event(
    const constants__t_session_i session_core_bs__session,
    const constants__t_channel_config_idx_i session_core_bs__channel_config_idx)
{
    SOPC_EventHandler_Post(SOPC_Services_GetEventHandler(), SE_TO_SE_ACTIVATE_ORPHANED_SESSION,
                           (uint32_t) session_core_bs__session, (uintptr_t) NULL,
                           (uintptr_t) session_core_bs__channel_config_idx);
}

void session_core_bs__client_gen_activate_user_session_internal_event(
    const constants__t_session_i session_core_bs__session,
    const constants__t_user_token_i session_core_bs__p_user_token)
{
    SOPC_EventHandler_Post(SOPC_Services_GetEventHandler(), SE_TO_SE_ACTIVATE_SESSION, session_core_bs__session,
                           (uintptr_t) session_core_bs__p_user_token, 0);
}

void session_core_bs__client_gen_create_session_internal_event(
    const constants__t_session_i session_core_bs__session,
    const constants__t_channel_config_idx_i session_core_bs__channel_config_idx)
{
    SOPC_EventHandler_Post(SOPC_Services_GetEventHandler(), SE_TO_SE_CREATE_SESSION,
                           (uint32_t) session_core_bs__session, (uintptr_t) NULL,
                           (uintptr_t) session_core_bs__channel_config_idx);
}

void session_core_bs__server_session_timeout_evaluation(const constants__t_session_i session_core_bs__session,
                                                        t_bool* const session_core_bs__expired)
{
    *session_core_bs__expired = true;
    SOPC_TimeReference current = 0;
    SOPC_TimeReference latestMsg = 0;
    SOPC_TimeReference elapsedSinceLatestMsg = 0;
    SOPC_Event event;
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
                // Re-activate timer for next verification
                event.eltId = session_core_bs__session;
                event.event = TIMER_SE_EVAL_SESSION_TIMEOUT;
                event.params = (uintptr_t) NULL;
                event.auxParam = 0;
                // Note: next timer is not revised session timeout but revised timeout - latest msg received
                // time
                timerId = SOPC_EventTimer_Create(
                    SOPC_Services_GetEventHandler(), event,
                    session_RevisedSessionTimeout[session_core_bs__session] - elapsedSinceLatestMsg);
                session_expiration_timer[session_core_bs__session] = timerId;
                if (0 == timerId)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "Services: session=%" PRIu32 " expiration timer renew failed",
                                           session_core_bs__session);
                }
                else
                {
                    /* Mark session as not expired */
                    *session_core_bs__expired = false;
                }
            }
        }
        if (*session_core_bs__expired != false)
        {
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Services: session=%" PRIu32 " expired on timeout evaluation",
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
                                                         const constants__t_msg_i session_core_bs__resp_msg,
                                                         t_bool* const session_core_bs__timer_created)
{
    const OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) session_core_bs__resp_msg;
    uint32_t timerId = 0;
    SOPC_Event event;
    *session_core_bs__timer_created = false;
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
        event.eltId = session_core_bs__session;
        event.event = TIMER_SE_EVAL_SESSION_TIMEOUT;
        event.params = (uintptr_t) NULL;
        event.auxParam = 0;
        timerId = SOPC_EventTimer_Create(SOPC_Services_GetEventHandler(), event,
                                         session_RevisedSessionTimeout[session_core_bs__session]);
        session_expiration_timer[session_core_bs__session] = timerId;
        if (0 == timerId)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Services: session=%" PRIu32 " expiration timer creation failed",
                                   session_core_bs__session);
        }
        else
        {
            *session_core_bs__timer_created = true;
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

void session_core_bs__server_set_fresh_nonce(
    const constants__t_session_i session_core_bs__p_session,
    const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
    t_bool* const session_core_bs__p_bres,
    constants__t_Nonce_i* const session_core_bs__p_nonce)
{
    SOPC_SecureChannel_Config* pSCCfg = NULL;
    SOPC_CryptoProvider* provider = NULL;
    ServerSessionData* pSession = NULL;
    SOPC_ByteString* pNonce = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    /* Default answer */
    *session_core_bs__p_bres = false;
    *session_core_bs__p_nonce = constants__c_Nonce_indet;

    assert(constants__c_session_indet != session_core_bs__p_session);

    pSession = &serverSessionDataArray[session_core_bs__p_session];

    /* Retrieve the security policy and mode */
    pSCCfg = SOPC_ToolkitServer_GetSecureChannelConfig(session_core_bs__p_channel_config_idx);

    if (NULL == pSCCfg)
    {
        return;
    }

    /* Retrieve ptrs to ServerNonce */
    pNonce = &pSession->nonceServer;

    /* Ask the CryptoProvider for LENGTH_NONCE random bytes */
    SOPC_ByteString_Clear(pNonce);
    pNonce->Length = LENGTH_NONCE;

    provider = SOPC_CryptoProvider_Create(pSCCfg->reqSecuPolicyUri);

    status = SOPC_CryptoProvider_GenerateRandomBytes(provider, (uint32_t) pNonce->Length, &pNonce->Data);
    if (SOPC_STATUS_OK == status)
    {
        *session_core_bs__p_bres = true;
        *session_core_bs__p_nonce = pNonce;
    }

    /* Clear */
    SOPC_CryptoProvider_Free(provider);
}

void session_core_bs__server_may_need_user_token_encryption(
    const constants__t_endpoint_config_idx_i session_core_bs__p_endpoint_config_idx,
    const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
    t_bool* const session_core_bs__p_bres)
{
    *session_core_bs__p_bres = false;

    /* Retrieve the secure channel configuration and available user token policies for the security policy and mode */
    SOPC_Endpoint_Config* epConfig = SOPC_ToolkitServer_GetEndpointConfig(session_core_bs__p_endpoint_config_idx);
    assert(NULL != epConfig);
    SOPC_SecureChannel_Config* scConfig =
        SOPC_ToolkitServer_GetSecureChannelConfig(session_core_bs__p_channel_config_idx);
    assert(NULL != scConfig);

    bool foundUserSecuPolicyWithEncryption = false;

    for (uint8_t epSecPolIdx = 0; epSecPolIdx < epConfig->nbSecuConfigs && !foundUserSecuPolicyWithEncryption;
         epSecPolIdx++)
    {
        SOPC_SecurityPolicy* secPol = &epConfig->secuConfigurations[epSecPolIdx];

        if (0 == strcmp(scConfig->reqSecuPolicyUri, SOPC_String_GetRawCString(&secPol->securityPolicy)) &&
            util_SecuModeEnumIncludedInSecuModeMasks(scConfig->msgSecurityMode, secPol->securityModes))
        {
            for (uint8_t i = 0; i < secPol->nbOfUserTokenPolicies && !foundUserSecuPolicyWithEncryption; i++)
            {
                if (secPol->userTokenPolicies[i].SecurityPolicyUri.Length > 0 &&
                    0 != strcmp(SOPC_SecurityPolicy_None_URI,
                                SOPC_String_GetRawCString(&secPol->userTokenPolicies[i].SecurityPolicyUri)))
                {
                    /* Note: since we check only None, we might return true with an invalid policy configured in server
                             which is ok since we only use it to generate additional parameters for session responses */
                    foundUserSecuPolicyWithEncryption = true;
                }
            }
        }
    }
    *session_core_bs__p_bres = foundUserSecuPolicyWithEncryption;
}
