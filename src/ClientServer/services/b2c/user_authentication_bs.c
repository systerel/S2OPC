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

#define ENCRYPTED_USER_IDENTITY_TOKEN_LENGTH_FIELD_SIZE 4

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
            util_SecuModeEnumIncludedInSecuModeMasks(scConfig->msgSecurityMode, secPol->securityModes))
        {
            if (constants__c_userTokenType_indet != user_authentication_bs__p_user_token_type)
            {
                for (uint8_t i = 0; i < secPol->nbOfUserTokenPolicies && compliantPolicyOrAvailable == false; i++)
                {
                    compliantPolicyOrAvailable = util_check_user_token_policy_compliance(
                        scConfig, &secPol->userTokenPolicies[i], user_authentication_bs__p_user_token_type,
                        user_authentication_bs__p_user_token, true, &userSecurityPolicy);
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
    SOPC_UNUSED_ARG(user_authentication_bs__p_token_type); // Only for B precondition corresponding to asserts:
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

void user_authentication_bs__shallow_copy_user_token(
    const constants__t_user_token_type_i user_authentication_bs__p_token_type,
    const constants__t_user_token_i user_authentication_bs__p_user_token,
    t_bool* const user_authentication_bs__p_sc_valid_user_token,
    constants__t_user_token_i* const user_authentication_bs__p_user_token_copy)
{
    *user_authentication_bs__p_sc_valid_user_token = false;
    *user_authentication_bs__p_user_token_copy = NULL;

    OpcUa_AnonymousIdentityToken *anonS = NULL, *anonD = NULL;
    OpcUa_UserNameIdentityToken *usernameS = NULL, *usernameD = NULL;
    OpcUa_X509IdentityToken *x509S = NULL, *x509D = NULL;
    OpcUa_IssuedIdentityToken *issuedS = NULL, *issuedD = NULL;

    SOPC_ExtensionObject* user = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));

    if (NULL == user)
    {
        return;
    }

    void* token = NULL;
    SOPC_ReturnStatus status =
        SOPC_Encodeable_CreateExtension(user, user_authentication_bs__p_user_token->Body.Object.ObjType, &token);

    if (SOPC_STATUS_OK == status)
    {
        switch (user_authentication_bs__p_token_type)
        {
        case constants__e_userTokenType_anonymous:
            anonS = user_authentication_bs__p_user_token->Body.Object.Value;
            anonD = token;
            status = SOPC_String_AttachFrom(&anonD->PolicyId, &anonS->PolicyId);
            break;
        case constants__e_userTokenType_userName:
            usernameS = user_authentication_bs__p_user_token->Body.Object.Value;
            usernameD = token;
            status = SOPC_String_AttachFrom(&usernameD->PolicyId, &usernameS->PolicyId);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_String_AttachFrom(&usernameD->UserName, &usernameS->UserName);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ByteString_Copy(&usernameD->Password, &usernameS->Password);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_String_AttachFrom(&usernameD->EncryptionAlgorithm, &usernameS->EncryptionAlgorithm);
            }
            break;
        case constants__e_userTokenType_x509:
            x509S = user_authentication_bs__p_user_token->Body.Object.Value;
            x509D = token;
            status = SOPC_String_AttachFrom(&x509D->PolicyId, &x509S->PolicyId);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ByteString_Copy(&x509D->CertificateData, &x509S->CertificateData);
            }
            break;
        case constants__e_userTokenType_issued:
            issuedS = user_authentication_bs__p_user_token->Body.Object.Value;
            issuedD = token;
            status = SOPC_String_AttachFrom(&issuedD->PolicyId, &issuedS->PolicyId);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ByteString_Copy(&issuedD->TokenData, &issuedS->TokenData);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_String_AttachFrom(&usernameD->EncryptionAlgorithm, &usernameS->EncryptionAlgorithm);
            }
            break;
        default:
            status = SOPC_STATUS_INVALID_PARAMETERS;
            break;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *user_authentication_bs__p_user_token_copy = user;
        *user_authentication_bs__p_sc_valid_user_token = true;
    }
    else
    {
        SOPC_ExtensionObject_Clear(user);
        SOPC_Free(user);
    }
}

static bool internal_user_token_copy(OpcUa_UserNameIdentityToken* source, SOPC_ExtensionObject** dest)
{
    assert(NULL != source);
    assert(NULL != dest);
    SOPC_ExtensionObject* user = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
    OpcUa_UserNameIdentityToken* token = NULL;

    if (NULL == user)
    {
        return false;
    }

    SOPC_ReturnStatus status =
        SOPC_Encodeable_CreateExtension(user, &OpcUa_UserNameIdentityToken_EncodeableType, (void**) &token);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&token->UserName, &source->UserName);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&token->PolicyId, &source->PolicyId);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ByteString_Copy(&token->Password, &source->Password);
    }
    if (SOPC_STATUS_OK == status)
    {
        *dest = user;
        return true;
    }
    else
    {
        SOPC_ExtensionObject_Clear(user);
        SOPC_Free(user);
        return false;
    }
}

static SOPC_ReturnStatus internal_decrypt_user_password(OpcUa_UserNameIdentityToken* userToken,
                                                        SOPC_CryptoProvider* cp,
                                                        SOPC_SerializedAsymmetricKey* serverKey,
                                                        SOPC_Buffer** decryptedBuffer)
{
    assert(NULL != decryptedBuffer);
    SOPC_AsymmetricKey* key = NULL;
    uint32_t decryptedLength = 0;
    SOPC_Buffer* buffer = NULL;

    SOPC_ReturnStatus status = SOPC_KeyManager_SerializedAsymmetricKey_Deserialize(serverKey, false, &key);
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

    if (status == SOPC_STATUS_OK)
    {
        *decryptedBuffer = buffer;
    }
    else
    {
        SOPC_Buffer_Clear(buffer);
        SOPC_Free(buffer);
        *decryptedBuffer = NULL;
    }
    SOPC_KeyManager_AsymmetricKey_Free(key);
    return status;
}

static SOPC_ReturnStatus internal_compare_user_decrypted_password_nonce(SOPC_ByteString* serverNonce,
                                                                        SOPC_CryptoProvider* cp,
                                                                        SOPC_Buffer* decryptedBuffer,
                                                                        uint32_t* passwordLength)
{
    assert(NULL != serverNonce);
    assert(NULL != decryptedBuffer);
    assert(NULL != passwordLength);

    uint32_t lenNonce = 0;
    uint32_t totalLength = 0;
    uint32_t pwdLength = 0;
    uint32_t decryptedLength = decryptedBuffer->length;

    SOPC_ReturnStatus status = SOPC_CryptoProvider_SymmetricGetLength_SecureChannelNonce(cp, &lenNonce);
    if (SOPC_STATUS_OK == status)
    {
        if (lenNonce != (uint32_t) serverNonce->Length)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Client user decryption: incompatible server Nonce length: expected %" PRIu32
                                   " actual length %" PRIi32,
                                   lenNonce, serverNonce->Length);
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_UInt32_Read(&totalLength, decryptedBuffer, 0);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (decryptedLength - ENCRYPTED_USER_IDENTITY_TOKEN_LENGTH_FIELD_SIZE != totalLength)
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
        int cmp = memcmp(serverNonce->Data,
                         &decryptedBuffer->data[ENCRYPTED_USER_IDENTITY_TOKEN_LENGTH_FIELD_SIZE + pwdLength],
                         (size_t) lenNonce);
        if (cmp != 0)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "Client used unexpected server nonce into encrypted user token");
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            *passwordLength = pwdLength;
        }
    }

    return status;
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

    OpcUa_UserNameIdentityToken* userToken = user_authentication_bs__p_user_token->Body.Object.Value;

    if (constants__e_secpol_None == user_authentication_bs__p_user_secu_policy)
    {
        // No encryption: create a copy of user token
        *user_authentication_bs__p_sc_valid_user_token =
            internal_user_token_copy(userToken, user_authentication_bs__p_user_token_decrypted);
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

    // Decrypt password and nonce
    SOPC_Buffer* decryptedBuffer = NULL;
    SOPC_ReturnStatus status =
        internal_decrypt_user_password(userToken, cp, epConfig->serverConfigPtr->serverKey, &decryptedBuffer);

    // Compare nonces and retrieve password length (depends on nonce length)
    uint32_t passwordLength = 0;
    if (SOPC_STATUS_OK == status)
    {
        status = internal_compare_user_decrypted_password_nonce(user_authentication_bs__p_server_nonce, cp,
                                                                decryptedBuffer, &passwordLength);
    }

    // Create decrypted user token
    SOPC_ExtensionObject* user = NULL;
    OpcUa_UserNameIdentityToken* token = NULL;
    if (SOPC_STATUS_OK == status)
    {
        user = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
        status = NULL != user ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Encodeable_CreateExtension(user, &OpcUa_UserNameIdentityToken_EncodeableType, (void**) &token);
    }
    // Copy user name and policy
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&token->UserName, &userToken->UserName);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&token->PolicyId, &userToken->PolicyId);
    }
    // Copy decrypted password
    if (SOPC_STATUS_OK == status)
    {
        token->Password.Data = SOPC_Calloc((size_t) passwordLength, sizeof(*token->Password.Data));
        if (NULL != token->Password.Data)
        {
            // UserIdentityToken encrypted format length (4 bytes) + token + server nonce
            memcpy(token->Password.Data, decryptedBuffer->data + ENCRYPTED_USER_IDENTITY_TOKEN_LENGTH_FIELD_SIZE,
                   (size_t) passwordLength);

            token->Password.Length = (int32_t) passwordLength;
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
    SOPC_Buffer_Clear(decryptedBuffer);
    SOPC_Free(decryptedBuffer);
    SOPC_CryptoProvider_Free(cp);
}

void user_authentication_bs__encrypt_user_token(
    const constants__t_channel_config_idx_i user_authentication_bs__p_channel_config_idx,
    const constants__t_byte_buffer_i user_authentication_bs__p_server_cert,
    const constants__t_Nonce_i user_authentication_bs__p_server_nonce,
    const constants__t_SecurityPolicy user_authentication_bs__p_user_secu_policy,
    const constants__t_user_token_type_i user_authentication_bs__p_token_type,
    const constants__t_user_token_i user_authentication_bs__p_user_token,
    t_bool* const user_authentication_bs__p_valid,
    constants__t_user_token_i* const user_authentication_bs__p_user_token_encrypted)
{
    assert(constants__e_userTokenType_userName == user_authentication_bs__p_token_type &&
           "Only encryption of username identity token supported");
    assert(NULL != user_authentication_bs__p_server_cert);
    assert(constants__c_user_token_indet != user_authentication_bs__p_user_token);
    *user_authentication_bs__p_user_token_encrypted = NULL;
    *user_authentication_bs__p_valid = false;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_SecureChannel_Config* scConfig =
        SOPC_ToolkitClient_GetSecureChannelConfig(user_authentication_bs__p_channel_config_idx);
    assert(NULL != scConfig);

    OpcUa_UserNameIdentityToken* userToken = user_authentication_bs__p_user_token->Body.Object.Value;

    // Create a copy of user token
    SOPC_ExtensionObject* encryptedTokenExtObj = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
    OpcUa_UserNameIdentityToken* encryptedToken = NULL;

    if (NULL == encryptedTokenExtObj)
    {
        return;
    }
    status = SOPC_Encodeable_CreateExtension(encryptedTokenExtObj, &OpcUa_UserNameIdentityToken_EncodeableType,
                                             (void**) &encryptedToken);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&encryptedToken->UserName, &userToken->UserName);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Copy(&encryptedToken->PolicyId, &userToken->PolicyId);
    }

    // No encryption if security policy is None
    if (SOPC_STATUS_OK == status && constants__e_secpol_None == user_authentication_bs__p_user_secu_policy)
    {
        // Forbids to transmit password as clear text on wire (only SignAndEncrypt mode accepted)
        if (OpcUa_MessageSecurityMode_SignAndEncrypt == scConfig->msgSecurityMode)
        {
            status = SOPC_ByteString_Copy(&encryptedToken->Password, &userToken->Password);
        }
        else
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "Services: user activation using channel config %" PRIu32
                " impossible because transmitting password as clear text is forbiden for security reasons",
                user_authentication_bs__p_channel_config_idx);
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_STATUS_OK == status)
        {
            *user_authentication_bs__p_valid = true;
            *user_authentication_bs__p_user_token_encrypted = encryptedTokenExtObj;
        }
        else
        {
            SOPC_ExtensionObject_Clear(encryptedTokenExtObj);
            SOPC_Free(encryptedTokenExtObj);
        }
        return;
    }

    SOPC_CryptoProvider* cp =
        SOPC_CryptoProvider_Create(util_channel__SecurityPolicy_B_to_C(user_authentication_bs__p_user_secu_policy));
    if (NULL == cp)
    {
        SOPC_ExtensionObject_Clear(encryptedTokenExtObj);
        SOPC_Free(encryptedTokenExtObj);
        return;
    }

    SOPC_CertificateList* serverCert = NULL;
    SOPC_AsymmetricKey* publicKey = NULL;
    uint32_t lenNonce = 0;
    uint32_t encryptedLength = 0;
    uint32_t unencryptedLength = 0;
    uint32_t pwdLength = 0;
    SOPC_Buffer* unencryptedBuffer = NULL;
    SOPC_Buffer* encryptedBuffer = NULL;

    // Get server public key
    status = SOPC_KeyManager_SerializedCertificate_Deserialize(user_authentication_bs__p_server_cert, &serverCert);
    if (SOPC_STATUS_OK == status)
    {
        // Retrieve public key from certificate
        status = SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(serverCert, &publicKey);
    }

    // Compute encrypted length
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CryptoProvider_SymmetricGetLength_SecureChannelNonce(cp, &lenNonce);
        if (SOPC_STATUS_OK == status && user_authentication_bs__p_server_nonce->Length != (int32_t) lenNonce)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "User password encryption: expected server Nonce length %" PRIu32
                                   " found length %" PRIi32,
                                   lenNonce, user_authentication_bs__p_server_nonce->Length);
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (userToken->Password.Length > 0)
        {
            pwdLength = (uint32_t) userToken->Password.Length;
        }
        // length field + password length + server nonce length
        unencryptedLength = 4 + pwdLength + lenNonce;

        status = SOPC_CryptoProvider_AsymmetricGetLength_Encryption(cp, publicKey, unencryptedLength, &encryptedLength);
    }

    // Copy unencrypted data into a buffer
    if (SOPC_STATUS_OK == status)
    {
        unencryptedBuffer = SOPC_Buffer_Create(unencryptedLength);
        if (NULL != unencryptedBuffer)
        {
            const uint32_t length = pwdLength + lenNonce;
            status = SOPC_UInt32_Write(&length, unencryptedBuffer, 0);
            if (SOPC_STATUS_OK == status && pwdLength > 0)
            {
                status = SOPC_Buffer_Write(unencryptedBuffer, userToken->Password.Data,
                                           (uint32_t) userToken->Password.Length);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_Buffer_Write(unencryptedBuffer, user_authentication_bs__p_server_nonce->Data,
                                           (uint32_t) user_authentication_bs__p_server_nonce->Length);
            }
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    // Create buffer for encryption
    if (SOPC_STATUS_OK == status)
    {
        encryptedBuffer = SOPC_Buffer_Create(encryptedLength);
        status = NULL == encryptedBuffer ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }

    // Encrypt password + server Nonce
    if (SOPC_STATUS_OK == status)
    {
        const char* errorReason;
        status = SOPC_CryptoProvider_AsymmetricEncrypt(cp, unencryptedBuffer->data, unencryptedLength, publicKey,
                                                       encryptedBuffer->data, encryptedLength, &errorReason);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "User password encryption: encryption failed with reason: %s", errorReason);
        }
    }

    // Copy encrypted buffer into user token
    if (SOPC_STATUS_OK == status)
    {
        encryptedToken->Password.Data = encryptedBuffer->data;
        encryptedToken->Password.Length = (int32_t) encryptedLength;
        encryptedBuffer->data = NULL;
    }

    // Set the encryption algorithm
    if (SOPC_STATUS_OK == status)
    {
        const char* encAlgo = util_getEncryptionAlgorithm(user_authentication_bs__p_user_secu_policy);
        if (NULL != encAlgo)
        {
            status = SOPC_String_CopyFromCString(&encryptedToken->EncryptionAlgorithm, encAlgo);
        }
        else
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *user_authentication_bs__p_valid = true;
        *user_authentication_bs__p_user_token_encrypted = encryptedTokenExtObj;
    }
    else
    {
        SOPC_ExtensionObject_Clear(encryptedTokenExtObj);
        SOPC_Free(encryptedTokenExtObj);
    }

    SOPC_KeyManager_AsymmetricKey_Free(publicKey);
    SOPC_KeyManager_Certificate_Free(serverCert);
    SOPC_CryptoProvider_Free(cp);
    SOPC_Buffer_Delete(unencryptedBuffer);
    SOPC_Buffer_Delete(encryptedBuffer);
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
