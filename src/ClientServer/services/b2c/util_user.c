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

#include <stdbool.h>
#include <string.h>

#include "sopc_types.h"
#include "sopc_user_app_itf.h"

#include "util_b2c.h"
#include "util_user.h"

constants__t_user_token_type_i util_get_user_token_type_from_token(
    const constants__t_user_token_i user_authentication_bs__p_user_token)
{
    assert(NULL != user_authentication_bs__p_user_token);
    SOPC_EncodeableType* tokenType = user_authentication_bs__p_user_token->Body.Object.ObjType;
    if (&OpcUa_AnonymousIdentityToken_EncodeableType == tokenType)
    {
        return constants__e_userTokenType_anonymous;
    }
    else if (&OpcUa_UserNameIdentityToken_EncodeableType == tokenType)
    {
        return constants__e_userTokenType_userName;
    }
    else if (&OpcUa_X509IdentityToken_EncodeableType == tokenType)
    {
        return constants__e_userTokenType_x509;
    }
    else if (&OpcUa_IssuedIdentityToken_EncodeableType == tokenType)
    {
        return constants__e_userTokenType_issued;
    }
    else
    {
        return constants__c_userTokenType_indet;
    }
}

static bool checkEncryptionAlgorithm(constants__t_SecurityPolicy secpol, SOPC_String* encryptionAlgo)
{
    switch (secpol)
    {
    case constants__e_secpol_None:
        // Part 4: "EncryptionAlgo: This parameter is null if the password is not encrypted"
        return encryptionAlgo->Length <= 0;
    case constants__e_secpol_B256:
        return 0 == strcmp("http://www.w3.org/2001/04/xmlenc#rsa-oaep", SOPC_String_GetRawCString(encryptionAlgo));
    case constants__e_secpol_B256S256:
        return 0 == strcmp("http://www.w3.org/2001/04/xmlenc#rsa-oaep", SOPC_String_GetRawCString(encryptionAlgo));
    case constants__e_secpol_Aes128Sha256RsaOaep:
        return 0 == strcmp("http://www.w3.org/2001/04/xmlenc#rsa-oaep", SOPC_String_GetRawCString(encryptionAlgo));
    case constants__e_secpol_Aes256Sha256RsaPss:
        return 0 == strcmp("http://opcfoundation.org/UA/security/rsa-oaep-sha2-256",
                           SOPC_String_GetRawCString(encryptionAlgo));
    default:
        assert(false && "Invalid security policy");
        return false;
    }
}

const char* util_getEncryptionAlgorithm(constants__t_SecurityPolicy secpol)
{
    switch (secpol)
    {
    case constants__e_secpol_None:
        // Part 4: "EncryptionAlgo: This parameter is null if the password is not encrypted"
        return NULL;
    case constants__e_secpol_B256:
        return "http://www.w3.org/2001/04/xmlenc#rsa-oaep";
    case constants__e_secpol_B256S256:
        return "http://www.w3.org/2001/04/xmlenc#rsa-oaep";
    case constants__e_secpol_Aes128Sha256RsaOaep:
        return "http://www.w3.org/2001/04/xmlenc#rsa-oaep";
    case constants__e_secpol_Aes256Sha256RsaPss:
        return "http://opcfoundation.org/UA/security/rsa-oaep-sha2-256";
    default:
        assert(false && "Invalid security policy");
        return NULL;
    }
}

bool util_check_user_token_policy_compliance(const SOPC_SecureChannel_Config* scConfig,
                                             const OpcUa_UserTokenPolicy* userTokenPolicy,
                                             const constants__t_user_token_type_i user_token_type,
                                             const constants__t_user_token_i user_token,
                                             bool check_encryption_algo,
                                             constants__t_SecurityPolicy* secpol)
{
    SOPC_String* tokenPolicyId = NULL;
    assert(secpol != NULL);
    SOPC_String* encryptionAlgo = NULL;
    bool bres = false;

    // Retrieve security policy:
    if (userTokenPolicy->SecurityPolicyUri.Length <= 0)
    {
        // User token security policy empty, use the secure channel security policy
        bres = util_channel__SecurityPolicy_C_to_B(scConfig->reqSecuPolicyUri, secpol);
        if (!bres)
        {
            return false;
        }
    }
    else
    {
        bres =
            util_channel__SecurityPolicy_C_to_B(SOPC_String_GetRawCString(&userTokenPolicy->SecurityPolicyUri), secpol);
        if (!bres)
        {
            return false;
        }
    }

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
        assert(SOPC_ExtObjBodyEncoding_Object == user_token->Encoding);
        tokenPolicyId = &((OpcUa_UserNameIdentityToken*) user_token->Body.Object.Value)->PolicyId;
        // Check Encryption Algo / Security policy
        encryptionAlgo = &((OpcUa_UserNameIdentityToken*) user_token->Body.Object.Value)->EncryptionAlgorithm;
        if (check_encryption_algo)
        {
            bres = checkEncryptionAlgorithm(*secpol, encryptionAlgo);
            if (!bres)
            {
                return false;
            }
        }
        break;
    case OpcUa_UserTokenType_Certificate:
        if (user_token_type != constants__e_userTokenType_x509)
        {
            return false;
        }
        assert(SOPC_ExtObjBodyEncoding_Object == user_token->Encoding);
        tokenPolicyId = &((OpcUa_X509IdentityToken*) user_token->Body.Object.Value)->PolicyId;
        break;
    case OpcUa_UserTokenType_IssuedToken:
        if (user_token_type != constants__e_userTokenType_issued)
        {
            return false;
        }
        assert(SOPC_ExtObjBodyEncoding_Object == user_token->Encoding);
        tokenPolicyId = &((OpcUa_IssuedIdentityToken*) user_token->Body.Object.Value)->PolicyId;
        // Check Encryption Algo / Security policy
        encryptionAlgo = &((OpcUa_IssuedIdentityToken*) user_token->Body.Object.Value)->EncryptionAlgorithm;
        if (check_encryption_algo)
        {
            bres = checkEncryptionAlgorithm(*secpol, encryptionAlgo);
            if (!bres)
            {
                return false;
            }
        }
        break;
    default:
        return false;
    }
    if (SOPC_String_Equal(&userTokenPolicy->PolicyId, tokenPolicyId))
    {
        return true;
    }
    return false;
}
