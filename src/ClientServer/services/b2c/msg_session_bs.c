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

#include "msg_session_bs.h"

#include "sopc_encoder.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"
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
    if (SOPC_STATUS_OK == SOPC_KeyManager_Certificate_GetMaybeApplicationUri(
                              pCertCli, (char**) &pReq->ClientDescription.ApplicationUri.Data, &len))
    {
        if (len <= INT32_MAX)
        {
            pReq->ClientDescription.ApplicationUri.Length = (int32_t) len;
        }
        *msg_session_bs__bret = true;
    }
    else
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "write_create_session_req_msg_crypto: Failed to extract ApplicationUri from client certificate on "
            "scConfigIdx=%" PRIu32,
            msg_session_bs__p_channel_config_idx);
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

void msg_session_bs__write_create_session_resp_msg_crypto(
    const constants__t_msg_i msg_session_bs__p_msg,
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
    const constants__t_Nonce_i msg_session_bs__p_nonce,
    const constants__t_SignatureData_i msg_session_bs__p_signature,
    t_bool* const msg_session_bs__bret)
{
    SOPC_SecureChannel_Config* pSCCfg = NULL;
    const SOPC_Buffer* pCrtSrv = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool result = true;
    OpcUa_CreateSessionResponse* pResp = (OpcUa_CreateSessionResponse*) msg_session_bs__p_msg;
    OpcUa_SignatureData* pSig = msg_session_bs__p_signature;

    /* Retrieve the certificate */
    pSCCfg = SOPC_ToolkitServer_GetSecureChannelConfig(msg_session_bs__p_channel_config_idx);
    if (NULL == pSCCfg)
    {
        result = false;
    }
    if (result != false)
    {
        pCrtSrv = pSCCfg->crt_srv;
        if (NULL == pCrtSrv)
        {
            result = false;
        }
    }

    /* Write the Certificate */
    if (result != false)
    {
        SOPC_ByteString_Clear(&pResp->ServerCertificate);
        assert(pCrtSrv->length <= INT32_MAX);
        status = SOPC_ByteString_CopyFromBytes(&pResp->ServerCertificate, pCrtSrv->data, (int32_t) pCrtSrv->length);

        if (SOPC_STATUS_OK == status)
        {
            pResp->ServerCertificate.Length = (int32_t) pCrtSrv->length;
            /* TODO: should borrow a reference instead of copy */
            /* Copy Nonce */
            status = SOPC_ByteString_Copy(&pResp->ServerNonce, msg_session_bs__p_nonce);
        }

        /* TODO: should borrow a reference instead of copy */
        /* Copy Signature, which is not a built-in, so copy its fields */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_Copy(&pResp->ServerSignature.Algorithm, &pSig->Algorithm);
        }
        if (SOPC_STATUS_OK == status)
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

void msg_session_bs__write_activate_session_resp_msg_crypto(const constants__t_msg_i msg_session_bs__activate_resp_msg,
                                                            const constants__t_Nonce_i msg_session_bs__nonce)
{
    OpcUa_ActivateSessionResponse* pResp = (OpcUa_ActivateSessionResponse*) msg_session_bs__activate_resp_msg;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    /* Write the nonce */
    /* TODO: this can also fail because of the malloc */
    status = SOPC_ByteString_Copy(&pResp->ServerNonce, msg_session_bs__nonce);
    assert(SOPC_STATUS_OK == status);
}

void msg_session_bs__minimize_max_message_length_create_session_msg(
    const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
    const constants__t_msg_i msg_session_bs__p_create_session_req)
{
    SOPC_SecureChannel_Config* chConfig =
        SOPC_ToolkitServer_GetSecureChannelConfig(msg_session_bs__p_channel_config_idx);
    uint32_t maxResponseMessageSize =
        ((OpcUa_CreateSessionRequest*) msg_session_bs__p_create_session_req)->MaxResponseMessageSize;

    /* Update sendMaxMessageSize if new value is more restrictive */
    if (NULL != chConfig && maxResponseMessageSize != 0 &&
        maxResponseMessageSize < (uint32_t) chConfig->internalProtocolData)
    {
        chConfig->internalProtocolData = (uintptr_t) maxResponseMessageSize;
    }
}
