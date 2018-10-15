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

#include "user_authentication_bs.h"

#include "opcua_identifiers.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"
#include "sopc_user_manager_internal.h"

#include <assert.h>
#include <inttypes.h>
#include <string.h>

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
    SOPC_EncodeableType* tokenType = user_authentication_bs__p_user_token->Body.Object.ObjType;
    if (&OpcUa_AnonymousIdentityToken_EncodeableType == tokenType)
    {
        *user_authentication_bs__p_user_token_type = constants__e_userTokenType_anonymous;
    }
    else if (&OpcUa_UserNameIdentityToken_EncodeableType == tokenType)
    {
        *user_authentication_bs__p_user_token_type = constants__e_userTokenType_userName;
    }
    else if (&OpcUa_X509IdentityToken_EncodeableType == tokenType)
    {
        *user_authentication_bs__p_user_token_type = constants__e_userTokenType_x509;
    }
    else if (&OpcUa_IssuedIdentityToken_EncodeableType == tokenType)
    {
        *user_authentication_bs__p_user_token_type = constants__e_userTokenType_issued;
    }
    else
    {
        *user_authentication_bs__p_user_token_type = constants__c_userTokenType_indet;
    }
}

static bool isCompliantWithUserTokenPolicy(const OpcUa_UserTokenPolicy* userTokenPolicy,
                                           const constants__t_user_token_type_i user_token_type,
                                           const constants__t_user_token_i user_token)
{
    SOPC_String* tokenPolicyId = NULL;
    switch (userTokenPolicy->TokenType)
    {
    case OpcUa_UserTokenType_Anonymous:
        if (user_token_type != constants__e_userTokenType_anonymous)
        {
            return false;
        }
        else
        {
            /* Important Note: we do not check the token since it could be NULL in case of Anonymous.
             * PolicyId not checked in case of Anonymous token.*/
            return true;
        }
        break;
    case OpcUa_UserTokenType_UserName:
        if (user_token_type != constants__e_userTokenType_userName)
        {
            return false;
        }
        tokenPolicyId = &((OpcUa_UserNameIdentityToken*) user_token->Body.Object.Value)->PolicyId;
        break;
    case OpcUa_UserTokenType_Certificate:
        if (user_token_type != constants__e_userTokenType_x509)
        {
            return false;
        }
        tokenPolicyId = &((OpcUa_X509IdentityToken*) user_token->Body.Object.Value)->PolicyId;
        break;
    case OpcUa_UserTokenType_IssuedToken:
        if (user_token_type != constants__e_userTokenType_issued)
        {
            return false;
        }
        break;
    default:
        return false;
    }
    return SOPC_String_Equal(&userTokenPolicy->PolicyId, tokenPolicyId);
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

void user_authentication_bs__is_user_token_supported(
    const constants__t_user_token_type_i user_authentication_bs__p_user_token_type,
    const constants__t_user_token_i user_authentication_bs__p_user_token,
    const constants__t_channel_config_idx_i user_authentication_bs__p_channel_config_idx,
    const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
    t_bool* const user_authentication_bs__p_supported_user_token_type)
{
    *user_authentication_bs__p_supported_user_token_type = false;
    if (user_authentication_bs__p_user_token_type == constants__c_userTokenType_indet)
    {
        return;
    }

    SOPC_Endpoint_Config* epConfig =
        SOPC_ToolkitServer_GetEndpointConfig(user_authentication_bs__p_endpoint_config_idx);
    assert(NULL != epConfig);
    SOPC_SecureChannel_Config* scConfig =
        SOPC_ToolkitServer_GetSecureChannelConfig(user_authentication_bs__p_channel_config_idx);
    assert(NULL != scConfig);

    bool compliantPolicy = false;
    for (uint8_t epSecPolIdx = 0; epSecPolIdx < epConfig->nbSecuConfigs && compliantPolicy == false; epSecPolIdx++)
    {
        SOPC_SecurityPolicy* secPol = &epConfig->secuConfigurations[epSecPolIdx];

        if (0 == strcmp(scConfig->reqSecuPolicyUri, SOPC_String_GetRawCString(&secPol->securityPolicy)) &&
            secuModeEnumIncludedInSecuModeMasks(scConfig->msgSecurityMode, secPol->securityModes))
        {
            for (uint8_t userTokenIdx = 0; userTokenIdx < secPol->nbOfUserTokenPolicies && compliantPolicy == false;
                 userTokenIdx++)
            {
                compliantPolicy = isCompliantWithUserTokenPolicy(&secPol->userTokenPolicies[userTokenIdx],
                                                                 user_authentication_bs__p_user_token_type,
                                                                 user_authentication_bs__p_user_token);
            }
        }
    }
    *user_authentication_bs__p_supported_user_token_type = compliantPolicy;
}

void user_authentication_bs__is_valid_user_authentication(
    const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
    const constants__t_user_token_type_i user_authentication_bs__p_token_type,
    const constants__t_user_token_i user_authentication_bs__p_user_token,
    constants__t_StatusCode_i* const user_authentication_bs__p_sc_valid_user)
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

    if (SOPC_STATUS_OK != status)
    {
        /* Failure of the authentication manager: we do not know if the token was rejected or user denied */
        *user_authentication_bs__p_sc_valid_user = constants__e_sc_bad_user_access_denied;
        SOPC_Logger_TraceWarning(
            "User authentication manager failed to check user validity on endpoint config idx %" PRIu32,
            user_authentication_bs__p_endpoint_config_idx);
    }
    else
    {
        switch (authnStatus)
        {
        case SOPC_USER_AUTHENTICATION_OK:
            *user_authentication_bs__p_sc_valid_user = constants__e_sc_ok;
            break;
        case SOPC_USER_AUTHENTICATION_INVALID_TOKEN:
            *user_authentication_bs__p_sc_valid_user = constants__e_sc_bad_identity_token_invalid;
            break;
        case SOPC_USER_AUTHENTICATION_REJECTED_TOKEN:
            *user_authentication_bs__p_sc_valid_user = constants__e_sc_bad_identity_token_rejected;
            break;
        case SOPC_USER_AUTHENTICATION_ACCESS_DENIED:
            *user_authentication_bs__p_sc_valid_user = constants__e_sc_bad_user_access_denied;
            break;
        default:
            /* Invalid of the authentication manager: we do not know if the token was rejected or user denied */
            *user_authentication_bs__p_sc_valid_user = constants__e_sc_bad_user_access_denied;
            SOPC_Logger_TraceWarning(
                "User authentication manager returned an invalid authentication status on endpoint config idx %" PRIu32,
                user_authentication_bs__p_endpoint_config_idx);
            break;
        }
    }
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
