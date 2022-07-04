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
#include <string.h>

#include "msg_session_bs.h"

#include "sopc_encoder.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"

#include "util_b2c.h"
#include "util_discovery_services.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_session_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void msg_session_bs__write_activate_msg_user(const constants__t_msg_i msg_session_bs__msg,
                                             const constants__t_user_token_i msg_session_bs__p_user_token)
{
    OpcUa_ActivateSessionRequest* req = (OpcUa_ActivateSessionRequest*) msg_session_bs__msg;

    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_ReturnStatus status = SOPC_ExtensionObject_Move(&req->UserIdentityToken, msg_session_bs__p_user_token);
    SOPC_Free(msg_session_bs__p_user_token);
    SOPC_GCC_DIAGNOSTIC_RESTORE
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "msg_session_bs__write_activate_msg_user: userToken copy failed");
        assert(false);
    }
}

void msg_session_bs__write_create_session_req_msg_endpointUrl(
    const constants__t_msg_i msg_session_bs__msg,
    const constants__t_channel_config_idx_i msg_session_bs__channel_config_idx)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_CreateSessionRequest* createSessionReq = (OpcUa_CreateSessionRequest*) msg_session_bs__msg;
    SOPC_SecureChannel_Config* chConfig = SOPC_ToolkitClient_GetSecureChannelConfig(msg_session_bs__channel_config_idx);
    if (NULL != chConfig)
    {
        status = SOPC_String_CopyFromCString(&createSessionReq->EndpointUrl, chConfig->url);
    }
    assert(SOPC_STATUS_OK == status);
}

void msg_session_bs__write_create_session_req_msg_serverUri(
    const constants__t_msg_i msg_session_bs__msg,
    const constants__t_channel_config_idx_i msg_session_bs__channel_config_idx)
{
    OpcUa_CreateSessionRequest* createSessionReq = (OpcUa_CreateSessionRequest*) msg_session_bs__msg;
    SOPC_SecureChannel_Config* chConfig = SOPC_ToolkitClient_GetSecureChannelConfig(msg_session_bs__channel_config_idx);
    if (NULL != chConfig && NULL != chConfig->serverUri)
    {
        SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&createSessionReq->ServerUri, chConfig->serverUri);
        assert(SOPC_STATUS_OK == status);
    }
}

void msg_session_bs__write_create_session_req_msg_sessionName(
    const constants__t_msg_i msg_session_bs__p_req_msg,
    const constants__t_session_application_context_i msg_session_bs__p_app_context)
{
    SOPC_Internal_SessionAppContext* sessionAppCtx = (SOPC_Internal_SessionAppContext*) msg_session_bs__p_app_context;
    OpcUa_CreateSessionRequest* createSessionReq = (OpcUa_CreateSessionRequest*) msg_session_bs__p_req_msg;
    if (NULL == sessionAppCtx || NULL == sessionAppCtx->sessionName)
    {
        return;
    }

    SOPC_ReturnStatus status =
        SOPC_String_AttachFromCstring(&createSessionReq->SessionName, sessionAppCtx->sessionName);
    assert(SOPC_STATUS_OK == status);
}

void msg_session_bs__write_create_session_req_msg_sessionTimeout(
    const constants__t_msg_i msg_session_bs__create_req_msg)
{
    OpcUa_CreateSessionRequest* createSessionReq = (OpcUa_CreateSessionRequest*) msg_session_bs__create_req_msg;
    createSessionReq->RequestedSessionTimeout = SOPC_REQUESTED_SESSION_TIMEOUT;
}

void msg_session_bs__write_create_session_req_msg_crypto(
    const constants__t_msg_i msg_session_bs__p_req_msg,
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
    const constants__t_Nonce_i msg_session_bs__p_nonce,
    t_bool* const msg_session_bs__bret)
{
    *msg_session_bs__bret = false;
    SOPC_SecureChannel_Config* pSCCfg = NULL;
    OpcUa_CreateSessionRequest* pReq = (OpcUa_CreateSessionRequest*) msg_session_bs__p_req_msg;
    const SOPC_Buffer* pSerialCertCli = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    /* Retrieve the certificate */
    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(msg_session_bs__p_channel_config_idx);

    if (NULL == pSCCfg)
    {
        return;
    }

    pSerialCertCli = pSCCfg->crt_cli;

    if (NULL == pSerialCertCli)
    {
        return;
    }

    /* Write the Certificate */
    SOPC_ByteString_Clear(&pReq->ClientCertificate);

    assert(pSerialCertCli->length <= INT32_MAX);
    status =
        SOPC_ByteString_CopyFromBytes(&pReq->ClientCertificate, pSerialCertCli->data, (int32_t) pSerialCertCli->length);
    if (SOPC_STATUS_OK != status)
        return;
    pReq->ClientCertificate.Length = (int32_t) pSerialCertCli->length;

    /* Write the nonce */
    SOPC_ByteString_Clear(&pReq->ClientNonce);

    status = SOPC_ByteString_Copy(&pReq->ClientNonce, msg_session_bs__p_nonce);
    if (SOPC_STATUS_OK != status)
        return;

    SOPC_CertificateList* pCertCli = NULL;

    if (SOPC_STATUS_OK != SOPC_KeyManager_SerializedCertificate_Deserialize(pSerialCertCli, &pCertCli))
        return;

    size_t len = 0;
    char* applicationURI = NULL;
    if (SOPC_STATUS_OK == SOPC_KeyManager_Certificate_GetMaybeApplicationUri(pCertCli, &applicationURI, &len) &&
        len <= INT32_MAX)
    {
        // Clear the previously configured ApplicationURI
        SOPC_String_Clear(&pReq->ClientDescription.ApplicationUri);
        pReq->ClientDescription.ApplicationUri.Data = (SOPC_Byte*) applicationURI;
        pReq->ClientDescription.ApplicationUri.Length = (int32_t) len;
        *msg_session_bs__bret = true;
    }
    else
    {
        if (pReq->ClientDescription.ApplicationUri.Length > 0)
        {
            // An application URI was already configured, keep it but signal the extraction failed
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "write_create_session_req_msg_crypto: Failed to extract ApplicationUri from client certificate on "
                "scConfigIdx=%" PRIu32 ", the configured one '%s' will be used instead",
                msg_session_bs__p_channel_config_idx, (char*) pReq->ClientDescription.ApplicationUri.Data);
            *msg_session_bs__bret = true;
        }
        else
        {
            // No applicationURI configured, terminate in error
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "write_create_session_req_msg_crypto: Failed to extract ApplicationUri from client certificate on "
                "scConfigIdx=%" PRIu32,
                msg_session_bs__p_channel_config_idx);
        }
    }

    SOPC_KeyManager_Certificate_Free(pCertCli);
}

void msg_session_bs__write_create_session_msg_session_token(
    const constants__t_msg_i msg_session_bs__msg,
    const constants__t_session_i msg_session_bs__session,
    const constants__t_session_token_i msg_session_bs__session_token)
{
    OpcUa_CreateSessionResponse* createSessionResp = (OpcUa_CreateSessionResponse*) msg_session_bs__msg;
    SOPC_ReturnStatus status;
    const SOPC_NodeId* nodeId = msg_session_bs__session_token;
    status = SOPC_NodeId_Copy(&createSessionResp->AuthenticationToken, nodeId);
    assert(SOPC_STATUS_OK == status);
    createSessionResp->SessionId.IdentifierType = SOPC_IdentifierType_Numeric;
    createSessionResp->SessionId.Data.Numeric = msg_session_bs__session;
    createSessionResp->SessionId.Data.Numeric += 100000;
}

void msg_session_bs__write_create_session_msg_session_revised_timeout(const constants__t_msg_i msg_session_bs__req_msg,
                                                                      const constants__t_msg_i msg_session_bs__resp_msg)
{
    OpcUa_CreateSessionRequest* createSessionReq = (OpcUa_CreateSessionRequest*) msg_session_bs__req_msg;
    OpcUa_CreateSessionResponse* createSessionResp = (OpcUa_CreateSessionResponse*) msg_session_bs__resp_msg;

    if (createSessionReq->RequestedSessionTimeout < SOPC_MIN_SESSION_TIMEOUT)
    {
        createSessionResp->RevisedSessionTimeout = SOPC_MIN_SESSION_TIMEOUT;
    }
    else if (createSessionReq->RequestedSessionTimeout > SOPC_MAX_SESSION_TIMEOUT)
    {
        createSessionResp->RevisedSessionTimeout = SOPC_MAX_SESSION_TIMEOUT;
    }
    else
    {
        createSessionResp->RevisedSessionTimeout = createSessionReq->RequestedSessionTimeout;
    }
}

void msg_session_bs__write_create_session_msg_server_endpoints(
    const constants__t_msg_i msg_session_bs__req_msg,
    const constants__t_msg_i msg_session_bs__resp_msg,
    const constants__t_endpoint_config_idx_i msg_session_bs__endpoint_config_idx,
    constants_statuscodes_bs__t_StatusCode_i* const msg_session_bs__ret)
{
    OpcUa_CreateSessionRequest* createSessionReq = (OpcUa_CreateSessionRequest*) msg_session_bs__req_msg;
    OpcUa_CreateSessionResponse* createSessionResp = (OpcUa_CreateSessionResponse*) msg_session_bs__resp_msg;

    *msg_session_bs__ret = SOPC_Discovery_GetEndPointsDescriptions(
        msg_session_bs__endpoint_config_idx, true, &createSessionReq->EndpointUrl, 0, NULL,
        (uint32_t*) &createSessionResp->NoOfServerEndpoints, &createSessionResp->ServerEndpoints);
}

void msg_session_bs__write_create_session_resp_cert(
    const constants__t_msg_i msg_session_bs__p_msg,
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
    t_bool* const msg_session_bs__bret)
{
    SOPC_SecureChannel_Config* pSCCfg = NULL;
    const SOPC_Buffer* pCrtSrv = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool result = true;
    OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) msg_session_bs__p_msg;

    /* Retrieve the certificate */
    pSCCfg = SOPC_ToolkitServer_GetSecureChannelConfig(msg_session_bs__p_channel_config_idx);
    if (NULL == pSCCfg)
    {
        result = false;
    }
    if (result)
    {
        pCrtSrv = pSCCfg->crt_srv;
        if (NULL == pCrtSrv)
        {
            result = false;
        }
    }

    /* Write the Certificate */
    if (result)
    {
        SOPC_ByteString_Clear(&pResp->ServerCertificate);
        assert(pCrtSrv->length <= INT32_MAX);
        status = SOPC_ByteString_CopyFromBytes(&pResp->ServerCertificate, pCrtSrv->data, (int32_t) pCrtSrv->length);
        result = SOPC_STATUS_OK == status;
    }

    *msg_session_bs__bret = result;
}

void msg_session_bs__write_create_session_resp_nonce(const constants__t_msg_i msg_session_bs__p_msg,
                                                     const constants__t_Nonce_i msg_session_bs__p_nonce)
{
    OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) msg_session_bs__p_msg;

    /* Borrow Nonce */
    pResp->ServerNonce.DoNotClear = true;
    pResp->ServerNonce.Data = msg_session_bs__p_nonce->Data;
    pResp->ServerNonce.Length = msg_session_bs__p_nonce->Length;
}

void msg_session_bs__write_create_session_resp_signature(const constants__t_msg_i msg_session_bs__p_msg,
                                                         const constants__t_SignatureData_i msg_session_bs__p_signature,
                                                         t_bool* const msg_session_bs__bret)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool result = true;
    OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) msg_session_bs__p_msg;
    OpcUa_SignatureData* pSig = msg_session_bs__p_signature;

    /* Write the Signature */
    if (result)
    {
        /* TODO: should borrow a reference instead of copy */
        /* Copy Signature, which is not a built-in, so copy its fields */
        if (SOPC_STATUS_OK == status && constants__c_SignatureData_indet != pSig)
        {
            status = SOPC_String_Copy(&pResp->ServerSignature.Algorithm, &pSig->Algorithm);
        }
        if (SOPC_STATUS_OK == status && constants__c_SignatureData_indet != pSig)
        {
            status = SOPC_ByteString_Copy(&pResp->ServerSignature.Signature, &pSig->Signature);
        }

        if (status != SOPC_STATUS_OK)
        {
            result = false;
        }
    }

    *msg_session_bs__bret = result;
}

void msg_session_bs__write_activate_req_msg_locales(
    const constants__t_msg_i msg_session_bs__p_req_msg,
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_ActivateSessionRequest* pReq = (OpcUa_ActivateSessionRequest*) msg_session_bs__p_req_msg;
    SOPC_SecureChannel_Config* pSCCfg = NULL;
    int32_t i = 0;

    /* Retrieve the certificate */
    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(msg_session_bs__p_channel_config_idx);

    if (NULL == pSCCfg || NULL == pSCCfg->clientConfigPtr || NULL == pSCCfg->clientConfigPtr->clientLocaleIds ||
        NULL == pSCCfg->clientConfigPtr->clientLocaleIds[0])
    {
        return;
    }

    const SOPC_Client_Config* clientAppCfg = pSCCfg->clientConfigPtr;
    int32_t n_locale_ids = 0;

    for (i = 0; clientAppCfg->clientLocaleIds[i] != NULL; i++)
    {
        n_locale_ids++;
    }

    pReq->LocaleIds = SOPC_Calloc((size_t) n_locale_ids, sizeof(SOPC_String));

    if (pReq->LocaleIds != NULL)
    {
        for (i = 0; status == SOPC_STATUS_OK && i < n_locale_ids; i++)
        {
            status = SOPC_String_CopyFromCString(&pReq->LocaleIds[i], clientAppCfg->clientLocaleIds[i]);
        }

        if (SOPC_STATUS_OK == status)
        {
            pReq->NoOfLocaleIds = n_locale_ids;
        }
        else
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "msg_session_bs__write_activate_req_msg_locales: allocation of %" PRIi32
                                     " locale id / %" PRIi32 " locale ids failed.",
                                     i, n_locale_ids);

            for (i = 0; i < pReq->NoOfLocaleIds; i++)
            {
                SOPC_String_Clear(&pReq->LocaleIds[i]);
            }
            SOPC_Free(pReq->LocaleIds);
            pReq->LocaleIds = NULL;
        }
    }
    else
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "msg_session_bs__write_activate_req_msg_locales: allocation of %" PRIi32
                                 " locale ids failed.",
                                 n_locale_ids);
    }
}

void msg_session_bs__write_activate_session_req_msg_crypto(const constants__t_msg_i msg_session_bs__activate_req_msg,
                                                           const constants__t_SignatureData_i msg_session_bs__signature,
                                                           t_bool* const msg_session_bs__bret)

{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_ActivateSessionRequest* pReq = (OpcUa_ActivateSessionRequest*) msg_session_bs__activate_req_msg;
    OpcUa_SignatureData* pSig = msg_session_bs__signature;

    /* Copy Signature, which is not a built-in, so copy its fields */
    /* TODO: should borrow a reference instead of copy */
    status = SOPC_String_Copy(&pReq->ClientSignature.Algorithm, &pSig->Algorithm);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ByteString_Copy(&pReq->ClientSignature.Signature, &pSig->Signature);
    }

    if (SOPC_STATUS_OK == status)
    {
        *msg_session_bs__bret = true;
    }
    else
    {
        *msg_session_bs__bret = false;
    }
}

void msg_session_bs__write_activate_session_resp_nonce(const constants__t_msg_i msg_session_bs__activate_resp_msg,
                                                       const constants__t_Nonce_i msg_session_bs__nonce)
{
    OpcUa_ActivateSessionResponse* pResp = (OpcUa_ActivateSessionResponse*) msg_session_bs__activate_resp_msg;

    /* Borrow Nonce */
    pResp->ServerNonce.DoNotClear = true;
    pResp->ServerNonce.Data = msg_session_bs__nonce->Data;
    pResp->ServerNonce.Length = msg_session_bs__nonce->Length;
}

static void minimize_max_message_length_create_session_msg(
    bool isClient,
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
    uint32_t maxSendBodyMessageSize)
{
    SOPC_SecureChannel_Config* chConfig = NULL;
    if (isClient)
    {
        chConfig = SOPC_ToolkitClient_GetSecureChannelConfig(msg_session_bs__p_channel_config_idx);
    }
    else
    {
        chConfig = SOPC_ToolkitServer_GetSecureChannelConfig(msg_session_bs__p_channel_config_idx);
    }

    /* Update maximum OPC UA message body encodeable if new value is more restrictive */
    if (NULL != chConfig && maxSendBodyMessageSize != 0 &&
        maxSendBodyMessageSize < (uint32_t) chConfig->internalProtocolData)
    {
        // Note: This value is used by message_out_bs__encode_msg to create the buffer
        chConfig->internalProtocolData = (uintptr_t) maxSendBodyMessageSize;
    }
}

void msg_session_bs__write_create_session_req_msg_clientDescription(
    const constants__t_msg_i msg_session_bs__p_req_msg,
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx)
{
    OpcUa_CreateSessionRequest* pReq = (OpcUa_CreateSessionRequest*) msg_session_bs__p_req_msg;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_SecureChannel_Config* pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(msg_session_bs__p_channel_config_idx);
    if (NULL == pSCCfg)
    {
        return;
    }

    if (NULL == pSCCfg->clientConfigPtr)
    {
        // Client configuration not defined: at least set the application type in request
        pReq->ClientDescription.ApplicationType = OpcUa_ApplicationType_Client;
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Client configuration is not initialized for channel config=%" PRIu32,
                                 msg_session_bs__p_channel_config_idx);
        return;
    }

    const SOPC_Client_Config* clientAppCfg = pSCCfg->clientConfigPtr;

    if (clientAppCfg->clientDescription.ApplicationType < OpcUa_ApplicationType_Client ||
        clientAppCfg->clientDescription.ApplicationType > OpcUa_ApplicationType_ClientAndServer)
    {
        pReq->ClientDescription.ApplicationType = OpcUa_ApplicationType_Client;
        // Trace a warning since client description does not seem to be properly initialized
        SOPC_Logger_TraceWarning(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "Client description does not seem to be correctly initialized on channel config=%" PRIu32
            " (unexpected application type %d)",
            msg_session_bs__p_channel_config_idx, clientAppCfg->clientDescription.ApplicationType);
    }
    else
    {
        pReq->ClientDescription.ApplicationType = clientAppCfg->clientDescription.ApplicationType;
    }

    status = SOPC_String_Copy(&pReq->ClientDescription.ApplicationUri, &clientAppCfg->clientDescription.ApplicationUri);
    if (SOPC_STATUS_OK != status || pReq->ClientDescription.ApplicationUri.Length <= 0)
    {
        // Trace a warning since the applicationUri is usually checked by server
        SOPC_Logger_TraceWarning(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "No client application URI set in the create session request on channel config=%" PRIu32,
            msg_session_bs__p_channel_config_idx);
    }

    status = SOPC_String_Copy(&pReq->ClientDescription.ProductUri, &clientAppCfg->clientDescription.ProductUri);
    if (SOPC_STATUS_OK != status || pReq->ClientDescription.ProductUri.Length <= 0)
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                              "No client product URI set in the create session request on channel config=%" PRIu32,
                              msg_session_bs__p_channel_config_idx);
    }

    status = SOPC_LocalizedText_Copy(&pReq->ClientDescription.ApplicationName,
                                     &clientAppCfg->clientDescription.ApplicationName);
    if (SOPC_STATUS_OK != status || pReq->ClientDescription.ApplicationName.defaultText.Length <= 0)
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                              "No client application name set in the create session request on channel config=%" PRIu32,
                              msg_session_bs__p_channel_config_idx);
    }
}

void msg_session_bs__write_create_session_req_msg_maxResponseMessageSize(
    const constants__t_msg_i msg_session_bs__p_req_msg)
{
    // Set the size coherent with the maximum size on SC
    OpcUa_CreateSessionRequest* pReq = (OpcUa_CreateSessionRequest*) msg_session_bs__p_req_msg;
    const SOPC_Common_EncodingConstants* encCfg = SOPC_Internal_Common_GetEncodingConstants();
    pReq->MaxResponseMessageSize = encCfg->receive_max_msg_size;
}

void msg_session_bs__write_create_session_resp_msg_maxRequestMessageSize(
    const constants__t_msg_i msg_session_bs__p_resp_msg)
{
    // Set the size coherent with the maximum size on SC
    OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) msg_session_bs__p_resp_msg;
    const SOPC_Common_EncodingConstants* encCfg = SOPC_Internal_Common_GetEncodingConstants();
    pResp->MaxRequestMessageSize = encCfg->receive_max_msg_size;
}

static bool check_certificate_same_as_SC(const constants__t_channel_config_idx_i p_channel_config_idx,
                                         const char* scSecurityPolicy,
                                         const SOPC_SerializedCertificate* scCertificate,
                                         const SOPC_ByteString* pCreateSessionCert)
{
    bool result = false;
    bool sameCertificate = false;

    constants__t_SecurityPolicy SCsecPol = constants__e_secpol_B256S256;

    bool scHasCertificate = scCertificate != NULL;

    /* If SC certificate provided, check if the certificate is the same. */
    if (scHasCertificate && pCreateSessionCert->Length > 0)
    {
        const SOPC_Buffer* scSrvCert = SOPC_KeyManager_SerializedCertificate_Data(scCertificate);

        if (scSrvCert->length == (uint32_t) pCreateSessionCert->Length)
        {
            int comparison = memcmp(scSrvCert->data, pCreateSessionCert->Data, (size_t) scSrvCert->length);
            sameCertificate = (comparison == 0);
        }
    }

    if (sameCertificate)
    {
        result = true;
    }
    else if (scHasCertificate)
    {
        /* From OPC UA part 4, CreateSesssion parameters:
         * The Client shall verify that this Certificate is the same as the one it used to create the SecureChannel.
         */
        // The certificate shall be present and the same
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "msg_session_bs__create_session_req/resp_check_client/server_certificate: certificate not the same "
            "as the one provided for SecureChanel establishement in channel config %" PRIu32,
            p_channel_config_idx);
    }
    else
    {
        /* Certificate is absent, check if it can be ignored (only when SC security policy == NONE) */
        bool validSecPolicy = util_channel__SecurityPolicy_C_to_B(scSecurityPolicy, &SCsecPol);
        if (!validSecPolicy)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "msg_session_bs__create_session_req/resp_check_client/server_certificate: invalid "
                                   "security policy %s in channel "
                                   "config %" PRIu32,
                                   scSecurityPolicy, p_channel_config_idx);
        }
        else if (constants__e_secpol_None == SCsecPol) // Check current SC security policy is None
        {
            // The certificate will be validated during activate session in case it is necessary for user encryption
            // otherwise it can be ignored (see From OPC UA part 4, CreateSession Service Parameters table)
            result = true;
        }
        else
        {
            // Unexpected error: no certificate with a non-None policy
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "msg_session_bs__create_session_req/resp_check_client/server_certificate: "
                                   "Certificate missing in SC config %" PRIu32 " whereas policy is not None",
                                   p_channel_config_idx);
        }
    }
    return result;
}

void msg_session_bs__create_session_req_check_client_certificate(
    const constants__t_msg_i msg_session_bs__p_req_msg,
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
    t_bool* const msg_session_bs__valid)
{
    *msg_session_bs__valid = false;
    SOPC_SecureChannel_Config* pSCCfg = NULL;

    OpcUa_CreateSessionRequest* pReq = (OpcUa_CreateSessionRequest*) msg_session_bs__p_req_msg;

    /* Retrieve the certificate */
    pSCCfg = SOPC_ToolkitServer_GetSecureChannelConfig(msg_session_bs__p_channel_config_idx);

    if (NULL == pSCCfg)
    {
        return;
    }

    *msg_session_bs__valid = check_certificate_same_as_SC(
        msg_session_bs__p_channel_config_idx, pSCCfg->reqSecuPolicyUri, pSCCfg->crt_cli, &pReq->ClientCertificate);
}

void msg_session_bs__create_session_resp_check_server_certificate(
    const constants__t_msg_i msg_session_bs__p_resp_msg,
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
    t_bool* const msg_session_bs__valid)
{
    *msg_session_bs__valid = false;
    SOPC_SecureChannel_Config* pSCCfg = NULL;

    OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) msg_session_bs__p_resp_msg;

    /* Retrieve the certificate */
    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(msg_session_bs__p_channel_config_idx);

    if (NULL == pSCCfg)
    {
        return;
    }

    *msg_session_bs__valid = check_certificate_same_as_SC(
        msg_session_bs__p_channel_config_idx, pSCCfg->reqSecuPolicyUri, pSCCfg->crt_srv, &pResp->ServerCertificate);
}

void msg_session_bs__create_session_resp_check_server_endpoints(
    const constants__t_msg_i msg_session_bs__p_resp_msg,
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
    t_bool* const msg_session_bs__valid)
{
    *msg_session_bs__valid = false;
    SOPC_SecureChannel_Config* pSCCfg = NULL;

    OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) msg_session_bs__p_resp_msg;

    pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(msg_session_bs__p_channel_config_idx);

    if (NULL == pSCCfg)
    {
        return;
    }

    if (NULL == pSCCfg->expectedEndpoints || NULL == pSCCfg->expectedEndpoints->Endpoints ||
        pSCCfg->expectedEndpoints->NoOfEndpoints <= 0)
    {
        SOPC_Logger_TraceInfo(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "msg_session_bs__create_session_resp_check_server_endpoints: no endpoint description in channel config "
            "%" PRIu32 " with the security policy %s",
            msg_session_bs__p_channel_config_idx, pSCCfg->reqSecuPolicyUri);

        // We have to consider in this case that connection configuration is not created from discovery endpoint
        *msg_session_bs__valid = true;
        return;
    }

    /* Check endpoint descriptions are the same */
    bool sameEndpoints = true;

    if (pSCCfg->expectedEndpoints->NoOfEndpoints != pResp->NoOfServerEndpoints)
    {
        sameEndpoints = false;
    }

    /* From OPC UA spec part 4:
     * It is recommended that Servers only include the server.applicationUri, endpointUrl,
     * securityMode, securityPolicyUri, userIdentityTokens, transportProfileUri and
     * securityLevel with all other parameters set to null. Only the recommended
     * parameters shall be verified by the client. */
    for (int32_t i = 0; sameEndpoints && i < pSCCfg->expectedEndpoints->NoOfEndpoints; i++)
    {
        const OpcUa_EndpointDescription* left = &pSCCfg->expectedEndpoints->Endpoints[i];
        const OpcUa_EndpointDescription* right = &pResp->ServerEndpoints[i];

        // Server application URI
        sameEndpoints = SOPC_String_Equal(&left->Server.ApplicationUri, &right->Server.ApplicationUri);

        // EndpointURL
        sameEndpoints = sameEndpoints && SOPC_String_Equal(&left->EndpointUrl, &right->EndpointUrl);

        // SecurityMode
        sameEndpoints = sameEndpoints && (left->SecurityMode == right->SecurityMode);

        // SecurityPolicyUri
        sameEndpoints = sameEndpoints && SOPC_String_Equal(&left->SecurityPolicyUri, &right->SecurityPolicyUri);

        // UserIdentityTokens
        sameEndpoints = sameEndpoints && (left->NoOfUserIdentityTokens == right->NoOfUserIdentityTokens);
        for (int32_t j = 0; sameEndpoints && j < left->NoOfUserIdentityTokens; j++)
        {
            OpcUa_UserTokenPolicy* leftUserTokens = &left->UserIdentityTokens[j];
            OpcUa_UserTokenPolicy* rightUserTokens = &right->UserIdentityTokens[j];
            sameEndpoints =
                sameEndpoints && SOPC_String_Equal(&leftUserTokens->IssuedTokenType, &rightUserTokens->IssuedTokenType);
            sameEndpoints = sameEndpoints &&
                            SOPC_String_Equal(&leftUserTokens->IssuerEndpointUrl, &rightUserTokens->IssuerEndpointUrl);
            sameEndpoints = sameEndpoints && SOPC_String_Equal(&leftUserTokens->PolicyId, &rightUserTokens->PolicyId);
            sameEndpoints = sameEndpoints &&
                            SOPC_String_Equal(&leftUserTokens->SecurityPolicyUri, &rightUserTokens->SecurityPolicyUri);
            sameEndpoints = sameEndpoints && (leftUserTokens->TokenType == rightUserTokens->TokenType);
        }

        // TransportProfileUri
        sameEndpoints = sameEndpoints && SOPC_String_Equal(&left->TransportProfileUri, &right->TransportProfileUri);

        // SecurityLevel
        sameEndpoints = sameEndpoints && (left->SecurityLevel == right->SecurityLevel);
    }

    if (!sameEndpoints)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "msg_session_bs__create_session_resp_check_server_endpoints: server endpoints verification failed");
    }

    *msg_session_bs__valid = sameEndpoints;
}

void msg_session_bs__create_session_resp_export_maxRequestMessageSize(
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
    const constants__t_msg_i msg_session_bs__p_resp_msg)
{
    minimize_max_message_length_create_session_msg(
        true, msg_session_bs__p_channel_config_idx,
        ((OpcUa_CreateSessionResponse*) msg_session_bs__p_resp_msg)->MaxRequestMessageSize);
}

void msg_session_bs__create_session_req_export_maxResponseMessageSize(
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
    const constants__t_msg_i msg_session_bs__p_req_msg)
{
    minimize_max_message_length_create_session_msg(
        false, msg_session_bs__p_channel_config_idx,
        ((OpcUa_CreateSessionRequest*) msg_session_bs__p_req_msg)->MaxResponseMessageSize);
}
