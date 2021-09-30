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

#include <assert.h>
#include <inttypes.h>
#include <string.h>

#include "user_authentication_bs.h"

#include "opcua_identifiers.h"
#include "sopc_crypto_provider.h"
#include "sopc_encoder.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"
#include "sopc_user_manager_internal.h"
#include "util_b2c.h"
#include "util_user.h"

/* The local user. This implementation avoids user creation,
 * but its authorization manager is changed according to the endpoint configuration */
static SOPC_UserWithAuthorization user_local = {.user = NULL, .authorizationManager = NULL};

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void user_authentication_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void user_authentication_bs__allocate_authenticated_user(
    const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
    const constants__t_user_token_i user_authentication_bs__p_user_token,
    t_bool* const user_authentication_bs__p_is_allocated_user,
    constants__t_user_i* const user_authentication_bs__p_user)
{
    SOPC_Endpoint_Config* epConfig =
        SOPC_ToolkitServer_GetEndpointConfig(user_authentication_bs__p_endpoint_config_idx);
    assert(NULL != epConfig);

    SOPC_UserAuthorization_Manager* authorizationManager = epConfig->authorizationManager;

    *user_authentication_bs__p_user =
        SOPC_UserWithAuthorization_CreateFromIdentityToken(user_authentication_bs__p_user_token, authorizationManager);
    *user_authentication_bs__p_is_allocated_user = (NULL != *user_authentication_bs__p_user);
}

void user_authentication_bs__get_user_token_type_from_token(
    const constants__t_user_token_i user_authentication_bs__p_user_token,
    constants__t_user_token_type_i* const user_authentication_bs__p_user_token_type)
{
    *user_authentication_bs__p_user_token_type =
        util_get_user_token_type_from_token(user_authentication_bs__p_user_token);
}

static bool secuModeEnumIncludedInSecuModeMasks(OpcUa_MessageSecurityMode msgSecurityMode, uint16_t securityModes)
{
    switch (msgSecurityMode)
    {
    case OpcUa_MessageSecurityMode_Invalid:
        return false;
    case OpcUa_MessageSecurityMode_None:
        return (securityModes & SOPC_SECURITY_MODE_NONE_MASK) != 0;
    case OpcUa_MessageSecurityMode_Sign:
        return (securityModes & SOPC_SECURITY_MODE_SIGN_MASK) != 0;
    case OpcUa_MessageSecurityMode_SignAndEncrypt:
        return (securityModes & SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK) != 0;
    default:
        return false;
    }
}

/*
 * If user token is provided we check it is compatible with 1 user token policy,
 * otherwise we only check some user policies are available */
static bool SOPC_UserTokenPolicyEval_Internal(
    const constants__t_channel_config_idx_i user_authentication_bs__p_channel_config_idx,
    const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
    const constants__t_user_token_type_i user_authentication_bs__p_user_token_type,
    const constants__t_user_token_i user_authentication_bs__p_user_token,
    constants__t_SecurityPolicy* usedSecurityPolicy)
{
    bool compliantPolicyOrAvailable = false;

    SOPC_Endpoint_Config* epConfig =
        SOPC_ToolkitServer_GetEndpointConfig(user_authentication_bs__p_endpoint_config_idx);
    assert(NULL != epConfig);
    SOPC_SecureChannel_Config* scConfig =
        SOPC_ToolkitServer_GetSecureChannelConfig(user_authentication_bs__p_channel_config_idx);
    assert(NULL != scConfig);

    constants__t_SecurityPolicy userSecurityPolicy = constants__e_secpol_B256S256;

    for (uint8_t epSecPolIdx = 0; epSecPolIdx < epConfig->nbSecuConfigs && compliantPolicyOrAvailable == false;
         epSecPolIdx++)
    {
        SOPC_SecurityPolicy* secPol = &epConfig->secuConfigurations[epSecPolIdx];

        if (0 == strcmp(scConfig->reqSecuPolicyUri, SOPC_String_GetRawCString(&secPol->securityPolicy)) &&
            secuModeEnumIncludedInSecuModeMasks(scConfig->msgSecurityMode, secPol->securityModes))
        {
            if (constants__c_userTokenType_indet != user_authentication_bs__p_user_token_type)
            {
                for (uint8_t i = 0; i < secPol->nbOfUserTokenPolicies && compliantPolicyOrAvailable == false; i++)
                {
                    compliantPolicyOrAvailable = util_check_user_token_policy_compliance(
                        scConfig, &secPol->userTokenPolicies[i], user_authentication_bs__p_user_token_type,
                        user_authentication_bs__p_user_token, &userSecurityPolicy);
                }
            }
            else
            {
                // No user token type provided for evaluation, we only check some policies are available
                compliantPolicyOrAvailable = (secPol->nbOfUserTokenPolicies > 0);
            }
        }
    }

    if (usedSecurityPolicy != NULL)
    {
        *usedSecurityPolicy = userSecurityPolicy;
    }
    return compliantPolicyOrAvailable;
}

void user_authentication_bs__has_user_token_policy_available(
    const constants__t_channel_config_idx_i user_authentication_bs__p_channel_config_idx,
    const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
    t_bool* const user_authentication_bs__p_user_token_policy_available)
{
    *user_authentication_bs__p_user_token_policy_available = SOPC_UserTokenPolicyEval_Internal(
        user_authentication_bs__p_channel_config_idx, user_authentication_bs__p_endpoint_config_idx,
        constants__c_userTokenType_indet, NULL, NULL);
}

void user_authentication_bs__is_user_token_supported(
    const constants__t_user_token_type_i user_authentication_bs__p_user_token_type,
    const constants__t_user_token_i user_authentication_bs__p_user_token,
    const constants__t_channel_config_idx_i user_authentication_bs__p_channel_config_idx,
    const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
    t_bool* const user_authentication_bs__p_supported_user_token_type,
    constants__t_SecurityPolicy* const user_authentication_bs__p_user_security_policy)
{
    *user_authentication_bs__p_supported_user_token_type = false;
    if (user_authentication_bs__p_user_token_type == constants__c_userTokenType_indet)
    {
        return;
    }
    constants__t_SecurityPolicy usedSecuPolicy = constants__e_secpol_None;
    bool compliantPolicy = SOPC_UserTokenPolicyEval_Internal(
        user_authentication_bs__p_channel_config_idx, user_authentication_bs__p_endpoint_config_idx,
        user_authentication_bs__p_user_token_type, user_authentication_bs__p_user_token, &usedSecuPolicy);

    if (!compliantPolicy)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "User token not compliant with userTokenPolicies.");
    }

    *user_authentication_bs__p_supported_user_token_type = compliantPolicy;
    *user_authentication_bs__p_user_security_policy = usedSecuPolicy;
}

void user_authentication_bs__is_valid_user_authentication(
    const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
    const constants__t_user_token_type_i user_authentication_bs__p_token_type,
    const constants__t_user_token_i user_authentication_bs__p_user_token,
    constants_statuscodes_bs__t_StatusCode_i* const user_authentication_bs__p_sc_valid_user)
{
    (void) user_authentication_bs__p_token_type; // Only for B precondition corresponding to asserts:
    assert(user_authentication_bs__p_token_type != constants__c_userTokenType_indet);
    assert(user_authentication_bs__p_token_type != constants__e_userTokenType_anonymous);

    SOPC_Endpoint_Config* epConfig =
        SOPC_ToolkitServer_GetEndpointConfig(user_authentication_bs__p_endpoint_config_idx);
    assert(NULL != epConfig);

    SOPC_UserAuthentication_Manager* authenticationManager = epConfig->authenticationManager;

    SOPC_UserAuthentication_Status authnStatus = SOPC_USER_AUTHENTICATION_OK;

    SOPC_ReturnStatus status = SOPC_UserAuthentication_IsValidUserIdentity(
        authenticationManager, user_authentication_bs__p_user_token, &authnStatus);

    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_ExtensionObject_Clear(user_authentication_bs__p_user_token);
    SOPC_Free(user_authentication_bs__p_user_token);
    SOPC_GCC_DIAGNOSTIC_RESTORE

    if (SOPC_STATUS_OK != status)
    {
        /* Failure of the authentication manager: we do not know if the token was rejected or user denied */
        *user_authentication_bs__p_sc_valid_user = constants_statuscodes_bs__e_sc_bad_user_access_denied;
        SOPC_Logger_TraceWarning(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "User authentication manager failed to check user validity on endpoint config idx %" PRIu32,
            user_authentication_bs__p_endpoint_config_idx);
    }
    else
    {
        switch (authnStatus)
        {
        case SOPC_USER_AUTHENTICATION_OK:
            *user_authentication_bs__p_sc_valid_user = constants_statuscodes_bs__e_sc_ok;
            break;
        case SOPC_USER_AUTHENTICATION_INVALID_TOKEN:
            *user_authentication_bs__p_sc_valid_user = constants_statuscodes_bs__e_sc_bad_identity_token_invalid;
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "User identification failed: identity_token_invalid");
            break;
        case SOPC_USER_AUTHENTICATION_REJECTED_TOKEN:
            *user_authentication_bs__p_sc_valid_user = constants_statuscodes_bs__e_sc_bad_identity_token_rejected;
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "User identification failed: identity_token_rejected");
            break;
        case SOPC_USER_AUTHENTICATION_ACCESS_DENIED:
            *user_authentication_bs__p_sc_valid_user = constants_statuscodes_bs__e_sc_bad_user_access_denied;
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, "User identification failed: identity_token_denied");
            break;
        default:
            /* Invalid of the authentication manager: we do not know if the token was rejected or user denied */
            *user_authentication_bs__p_sc_valid_user = constants_statuscodes_bs__e_sc_bad_user_access_denied;
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "User authentication manager returned an invalid authentication status on "
                                     "endpoint config idx %" PRIu32,
                                     user_authentication_bs__p_endpoint_config_idx);
            break;
        }
    }
}

void user_authentication_bs__decrypt_user_token(
    const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
    const constants__t_Nonce_i user_authentication_bs__p_server_nonce,
    const constants__t_SecurityPolicy user_authentication_bs__p_user_secu_policy,
    const constants__t_user_token_type_i user_authentication_bs__p_token_type,
    const constants__t_user_token_i user_authentication_bs__p_user_token,
    t_bool* const user_authentication_bs__p_sc_valid_user_token,
    constants__t_user_token_i* const user_authentication_bs__p_user_token_decrypted)
{
    assert(constants__e_userTokenType_userName == user_authentication_bs__p_token_type &&
           "Only encrypted username identity token supported");
    *user_authentication_bs__p_user_token_decrypted = NULL;
    *user_authentication_bs__p_sc_valid_user_token = false;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    OpcUa_UserNameIdentityToken* userToken = user_authentication_bs__p_user_token->Body.Object.Value;

    if (constants__e_secpol_None == user_authentication_bs__p_user_secu_policy)
    {
        // Create a copy of user token
        SOPC_ExtensionObject* user = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
        OpcUa_UserNameIdentityToken* token = NULL;

        if (NULL == user)
        {
            return;
        }

        status = SOPC_Encodeable_CreateExtension(user, &OpcUa_UserNameIdentityToken_EncodeableType, (void**) &token);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_Copy(&token->UserName, &userToken->UserName);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_Copy(&token->PolicyId, &userToken->PolicyId);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ByteString_Copy(&token->Password, &userToken->Password);
        }
        if (SOPC_STATUS_OK == status)
        {
            *user_authentication_bs__p_sc_valid_user_token = true;
            *user_authentication_bs__p_user_token_decrypted = user;
        }
        else
        {
            SOPC_ExtensionObject_Clear(user);
            SOPC_Free(user);
        }
        return;
    }

    if (userToken->Password.Length <= 0)
    {
        // TODO: define minimal size regarding encoding blocks
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "Client user decryption: encrypted password length invalid");
        return;
    }

    SOPC_Endpoint_Config* epConfig =
        SOPC_ToolkitServer_GetEndpointConfig(user_authentication_bs__p_endpoint_config_idx);
    assert(NULL != epConfig);

    SOPC_CryptoProvider* cp =
        SOPC_CryptoProvider_Create(util_channel__SecurityPolicy_B_to_C(user_authentication_bs__p_user_secu_policy));
    if (NULL == cp)
    {
        return;
    }

    SOPC_AsymmetricKey* key = NULL;
    uint32_t decryptedLength = 0;
    SOPC_Buffer* buffer = NULL;
    uint32_t lenNonce = 0;
    uint32_t totalLength = 0;
    uint32_t pwdLength = 0;
    SOPC_ExtensionObject* user = NULL;
    OpcUa_UserNameIdentityToken* token = NULL;

    status = SOPC_KeyManager_SerializedAsymmetricKey_Deserialize(epConfig->serverConfigPtr->serverKey, false, &key);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CryptoProvider_AsymmetricGetLength_Decryption(cp, key, (uint32_t) userToken->Password.Length,
                                                                    &decryptedLength);
        // Note: decrypted length is a multiple of plain block size (includes padding)
    }

    if (SOPC_STATUS_OK == status)
    {
        buffer = SOPC_Buffer_Create(decryptedLength);
        if (NULL != buffer)
        {
            const char* errorReason = "";
            status = SOPC_CryptoProvider_AsymmetricDecrypt(cp, userToken->Password.Data,
                                                           (uint32_t) userToken->Password.Length, key, buffer->data,
                                                           decryptedLength, &buffer->length, &errorReason);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "Client user decryption failed with reason: %s",
                                       errorReason);
            }
            else
            {
                decryptedLength = buffer->length;
            }
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CryptoProvider_SymmetricGetLength_SecureChannelNonce(cp, &lenNonce);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (lenNonce != (uint32_t) user_authentication_bs__p_server_nonce->Length)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Client user decryption: incompatible server Nonce length: expected %" PRIu32
                                   " actual length %" PRIi32,
                                   lenNonce, user_authentication_bs__p_server_nonce->Length);
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Read(&totalLength, buffer, 0);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (decryptedLength - 4 != totalLength)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Client user decryption: encoded length (%" PRIu32
                                   ") is not compatible with decryped length (%" PRIu32 " - 4)",
                                   totalLength, decryptedLength);
        }
    }

    // Compare server nonce
    if (SOPC_STATUS_OK == status)
    {
        pwdLength = totalLength - lenNonce;
        int cmp = memcmp(user_authentication_bs__p_server_nonce->Data, &buffer->data[4 + pwdLength], (size_t) lenNonce);
        if (cmp != 0)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Client used unexpected server nonce into encrypted user token");
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    // Create decrypted user token
    if (SOPC_STATUS_OK == status)
    {
        user = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
        status = SOPC_Encodeable_CreateExtension(user, &OpcUa_UserNameIdentityToken_EncodeableType, (void**) &token);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&token->UserName, &userToken->UserName);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&token->PolicyId, &userToken->PolicyId);
    }

    if (SOPC_STATUS_OK == status)
    {
        // UserIdentityToken encrypted format length (4 bytes) + token + server nonce
        token->Password.Data = SOPC_Calloc((size_t) pwdLength, sizeof(*token->Password.Data));
        if (NULL != token->Password.Data)
        {
            memcpy(token->Password.Data, buffer->data + 4, (size_t) pwdLength);

            token->Password.Length = (int32_t) pwdLength;
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *user_authentication_bs__p_sc_valid_user_token = true;
        *user_authentication_bs__p_user_token_decrypted = user;
    }
    else
    {
        SOPC_ExtensionObject_Clear(user);
        SOPC_Free(user);
    }
    SOPC_Buffer_Clear(buffer);
    SOPC_Free(buffer);
    SOPC_KeyManager_AsymmetricKey_Free(key);
    SOPC_CryptoProvider_Free(cp);
}

void user_authentication_bs__deallocate_user(const constants__t_user_i session_core_bs__p_user)
{
    SOPC_UserWithAuthorization* userauthz = session_core_bs__p_user;
    SOPC_UserWithAuthorization_Free(&userauthz);
}

void user_authentication_bs__get_local_user(
    const constants__t_endpoint_config_idx_i session_core_bs__p_endpoint_config_idx,
    constants__t_user_i* const session_core_bs__p_user)
{
    SOPC_Endpoint_Config* epConfig = SOPC_ToolkitServer_GetEndpointConfig(session_core_bs__p_endpoint_config_idx);
    assert(NULL != epConfig);

    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    user_local.user = (SOPC_User*) SOPC_User_GetLocal();
    SOPC_GCC_DIAGNOSTIC_RESTORE
    user_local.authorizationManager = epConfig->authorizationManager;
    *session_core_bs__p_user = &user_local;
}
