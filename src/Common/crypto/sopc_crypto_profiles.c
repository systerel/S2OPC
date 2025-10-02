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

#include <string.h>

#include "sopc_assert.h"
#include "sopc_crypto_decl.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_profiles_lib_itf.h"
#include "sopc_helper_string.h"

// CryptoProfiles instances
extern const SOPC_CryptoProfile sopc_g_cpAes256Sha256RsaPss;
extern const SOPC_CryptoProfile sopc_g_cpAes128Sha256RsaOaep;
extern const SOPC_CryptoProfile sopc_g_cpBasic256Sha256;
extern const SOPC_CryptoProfile sopc_g_cpBasic256;
extern const SOPC_CryptoProfile sopc_g_cpNone;
extern const SOPC_CryptoProfile_PubSub sopc_g_cppsPubSubAes256;
extern const SOPC_CryptoProfile_PubSub sopc_g_cppsNone;

static const SOPC_CryptoProfile* SOPC_CryptoProfile_LibProfile_Get(const char* uri)
{
    if (uri == NULL)
    {
        return NULL;
    }

    if (strcmp(uri, SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI) == 0)
    {
        return &sopc_g_cpAes256Sha256RsaPss;
    }
    else if (strcmp(uri, SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI) == 0)
    {
        return &sopc_g_cpAes128Sha256RsaOaep;
    }
    else if (strcmp(uri, SOPC_SecurityPolicy_Basic256Sha256_URI) == 0)
    {
        return &sopc_g_cpBasic256Sha256;
    }
    else if (strcmp(uri, SOPC_SecurityPolicy_Basic256_URI) == 0)
    {
        return &sopc_g_cpBasic256;
    }
    else if (strcmp(uri, SOPC_SecurityPolicy_None_URI) == 0)
    {
        return &sopc_g_cpNone;
    }

    return NULL;
}

static const SOPC_CryptoProfile_PubSub* SOPC_CryptoProfile_LibPubSub_Get(const char* uri)
{
    if (uri == NULL)
    {
        return NULL;
    }

    if (strcmp(uri, SOPC_SecurityPolicy_PubSub_Aes256_URI) == 0)
    {
        return &sopc_g_cppsPubSubAes256;
    }
    else if (strcmp(uri, SOPC_SecurityPolicy_None_URI) == 0)
    {
        return &sopc_g_cppsNone;
    }

    return NULL;
}

const SOPC_SecurityPolicy_Config securityPolicy_cfg[SOPC_SecurityPolicy_Last_ID] = {
    // SOPC_SecurityPolicy_Invalid_ID
    {.uri = NULL,
     .name = "<Invalid>",
     .isInvalid = true,
     .profile = NULL,
     .psProfile = NULL,
     .secuPolicyWeight = 0,
     .symmLen_CryptoKey = 0,
     .symmLen_SignKey = 0,
     .symmLen_Signature = 0,
     .symmLen_Block = 0,
     .symmLen_KeyNonce = 0,
     .symmLen_MessageRandom = 0,
     .asymLen_OAEP_Hash = 0,
     .asymLen_KeyMinBits = 0,
     .asymLen_KeyMaxBits = 0,
     .secureChannelNonceLength = 0,
     .URI_SignAlgo = NULL,
     .certLen_Thumbprint = 0},
    // SOPC_SecurityPolicy_Basic256Sha256_ID
    {.uri = SOPC_SecurityPolicy_Basic256Sha256_URI,
     .name = "Basic256Sha256",
     .isInvalid = false,
     .profile = &SOPC_CryptoProfile_LibProfile_Get,
     .psProfile = NULL,
     .secuPolicyWeight = 3,
     .symmLen_CryptoKey = 32,
     .symmLen_SignKey = 32,
     .symmLen_Signature = 32,
     .symmLen_Block = 16,
     .symmLen_KeyNonce = 0,
     .symmLen_MessageRandom = 0,
     .asymLen_OAEP_Hash = 20,
     .asymLen_KeyMinBits = 2048,
     .asymLen_KeyMaxBits = 4096,
     .secureChannelNonceLength = 32,
     .URI_SignAlgo = SOPC_SecurityPolicy_Basic256Sha256_URI_SignAlgo,
     .certLen_Thumbprint = 20},
    // SOPC_SecurityPolicy_Basic256_ID
    {.uri = SOPC_SecurityPolicy_Basic256_URI,
     .name = "Basic256",
     .isInvalid = false,
     .profile = &SOPC_CryptoProfile_LibProfile_Get,
     .psProfile = NULL,
     .secuPolicyWeight = 2,
     .symmLen_CryptoKey = 32,
     .symmLen_SignKey = 24,
     .symmLen_Signature = 20,
     .symmLen_Block = 16,
     .symmLen_KeyNonce = 0,
     .symmLen_MessageRandom = 0,
     .asymLen_OAEP_Hash = 20,
     .asymLen_KeyMinBits = 1024,
     .asymLen_KeyMaxBits = 2048,
     .secureChannelNonceLength = 32,
     .URI_SignAlgo = SOPC_SecurityPolicy_Basic256_URI_SignAlgo,
     .certLen_Thumbprint = 20},
    // SOPC_SecurityPolicy_None_ID
    {.uri = SOPC_SecurityPolicy_None_URI,
     .name = "None",
     .isInvalid = false,
     .profile = &SOPC_CryptoProfile_LibProfile_Get,
     .psProfile = &SOPC_CryptoProfile_LibPubSub_Get,
     .secuPolicyWeight = 0,
     .symmLen_CryptoKey = 0,
     .symmLen_SignKey = 0,
     .symmLen_Signature = 0,
     .symmLen_Block = 0,
     .symmLen_KeyNonce = 0,
     .symmLen_MessageRandom = 0,
     .asymLen_OAEP_Hash = 0,
     .asymLen_KeyMinBits = 0,
     .asymLen_KeyMaxBits = 0,
     .secureChannelNonceLength = 0,
     .URI_SignAlgo = NULL,
     .certLen_Thumbprint = 0},
    // SOPC_SecurityPolicy_PubSub_Aes256_ID
    {.uri = SOPC_SecurityPolicy_PubSub_Aes256_URI,
     .name = "Aes256",
     .isInvalid = false,
     .profile = NULL,
     .psProfile = &SOPC_CryptoProfile_LibPubSub_Get,
     .secuPolicyWeight = 0,
     .symmLen_CryptoKey = 32,
     .symmLen_SignKey = 32,
     .symmLen_Signature = 32,
     .symmLen_Block = 0,
     .symmLen_KeyNonce = 4,
     .symmLen_MessageRandom = 4,
     .asymLen_OAEP_Hash = 0,
     .asymLen_KeyMinBits = 0,
     .asymLen_KeyMaxBits = 0,
     .secureChannelNonceLength = 0,
     .URI_SignAlgo = NULL,
     .certLen_Thumbprint = 0},
    // SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID
    {.uri = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI,
     .name = "Aes128-Sha256-RsaOaep",
     .isInvalid = false,
     .profile = &SOPC_CryptoProfile_LibProfile_Get,
     .psProfile = NULL,
     .secuPolicyWeight = 1,
     .symmLen_CryptoKey = 16,
     .symmLen_SignKey = 32,
     .symmLen_Signature = 32,
     .symmLen_Block = 16,
     .symmLen_KeyNonce = 0,
     .symmLen_MessageRandom = 0,
     .asymLen_OAEP_Hash = 20,
     .asymLen_KeyMinBits = 2048,
     .asymLen_KeyMaxBits = 4096,
     .secureChannelNonceLength = 32,
     .URI_SignAlgo = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI_SignAlgo,
     .certLen_Thumbprint = 20},
    // SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID
    {.uri = SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI,
     .name = "Aes256-Sha256-RsaPss",
     .isInvalid = false,
     .profile = &SOPC_CryptoProfile_LibProfile_Get,
     .psProfile = NULL,
     .secuPolicyWeight = 4,
     .symmLen_CryptoKey = 32,
     .symmLen_SignKey = 32,
     .symmLen_Signature = 32,
     .symmLen_Block = 16,
     .symmLen_KeyNonce = 0,
     .symmLen_MessageRandom = 0,
     .asymLen_OAEP_Hash = 32,
     .asymLen_KeyMinBits = 2048,
     .asymLen_KeyMaxBits = 4096,
     .secureChannelNonceLength = 32,
     .URI_SignAlgo = SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI_SignAlgo,
     .certLen_Thumbprint = 20},
};

const SOPC_SecurityPolicy_Config* SOPC_SecurityPolicy_Config_Get(SOPC_SecurityPolicy_ID policyId)
{
    if (((uint32_t) policyId) >= SOPC_SecurityPolicy_Last_ID)
    {
        policyId = SOPC_SecurityPolicy_Invalid_ID;
    }
    return &securityPolicy_cfg[policyId];
}

const SOPC_SecurityPolicy_Config* SOPC_CryptoProfile_Get(const char* uri)
{
    if (NULL == uri)
    {
        return NULL;
    }

    /* Compares len+1 to include the trailing \0 of the zero-terminated URI.
     * This avoids false positives with strings prefixed by a valid security policy. */
    const size_t len = strlen(uri) + 1;
    for (size_t i = 0; i < SOPC_SecurityPolicy_Last_ID; i++)
    {
        const SOPC_SecurityPolicy_Config* policy = &securityPolicy_cfg[i];

        if (SOPC_strncmp_ignore_case(uri, policy->uri, len) == 0)
        {
            return policy;
        }
    }

    return NULL;
}

const SOPC_CryptoProfile_PubSub* SOPC_CryptoProfile_PubSub_Get(const char* uri)
{
    if (NULL == uri)
    {
        return NULL;
    }

    /* Compares len+1 to include the trailing \0 of the zero-terminated URI.
     * This avoids false positives with strings prefixed by a valid security policy. */
    const size_t len = strlen(uri) + 1;
    for (size_t i = 0; i < SOPC_SecurityPolicy_Last_ID; i++)
    {
        const SOPC_SecurityPolicy_Config* policy = &securityPolicy_cfg[i];

        if (SOPC_strncmp_ignore_case(uri, policy->uri, len) == 0)
        {
            return policy->psProfile(uri);
        }
    }

    return NULL;
}
