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
#include <string.h>

#include "sopc_crypto_provider.h"

#include "sopc_crypto_decl.h"
#include "sopc_crypto_profiles.h"
#include "sopc_key_manager.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki.h"
#include "sopc_secret_buffer.h"

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider
 * ------------------------------------------------------------------------------------------------
 */

SOPC_CryptoProvider* SOPC_CryptoProvider_Create(const char* uri)
{
    const SOPC_CryptoProfile* pProfile = NULL;

    pProfile = SOPC_CryptoProfile_Get(uri);
    if (NULL == pProfile)
    {
        return NULL;
    }

    SOPC_CryptoProvider* pCryptoProvider = SOPC_Calloc(1, sizeof(SOPC_CryptoProvider));
    if (NULL != pCryptoProvider)
    {
        // The crypto provider profile shall be const after this init
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        *(const SOPC_CryptoProfile**) (&pCryptoProvider->pProfile) = pProfile;
        SOPC_GCC_DIAGNOSTIC_RESTORE
        if (SOPC_STATUS_OK != SOPC_CryptoProvider_Init(pCryptoProvider))
        {
            SOPC_Free(pCryptoProvider);
            pCryptoProvider = NULL;
        }
    }

    return pCryptoProvider;
}

SOPC_CryptoProvider* SOPC_CryptoProvider_CreatePubSub(const char* uri)
{
    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProfile_PubSub_Get(uri);
    if (NULL == pProfilePubSub)
    {
        return NULL;
    }

    SOPC_CryptoProvider* pCryptoProvider = SOPC_Calloc(1, sizeof(SOPC_CryptoProvider));
    if (NULL != pCryptoProvider)
    {
        // The crypto provider profile shall be const after this init
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        *(const SOPC_CryptoProfile_PubSub**) (&pCryptoProvider->pProfilePubSub) = pProfilePubSub;
        SOPC_GCC_DIAGNOSTIC_RESTORE
        if (SOPC_STATUS_OK != SOPC_CryptoProvider_Init(pCryptoProvider))
        {
            SOPC_Free(pCryptoProvider);
            pCryptoProvider = NULL;
        }
    }

    return pCryptoProvider;
}

void SOPC_CryptoProvider_Free(SOPC_CryptoProvider* pCryptoProvider)
{
    if (NULL != pCryptoProvider)
    {
        SOPC_CryptoProvider_Deinit(pCryptoProvider);
        SOPC_Free(pCryptoProvider);
    }
}

const SOPC_CryptoProfile* SOPC_CryptoProvider_GetProfileServices(const SOPC_CryptoProvider* pProvider)
{
    assert(NULL != pProvider);
    if (NULL != pProvider->pProfilePubSub)
    {
        return NULL;
    }
    return pProvider->pProfile;
}

const SOPC_CryptoProfile_PubSub* SOPC_CryptoProvider_GetProfilePubSub(const SOPC_CryptoProvider* pProvider)
{
    assert(NULL != pProvider);
    if (NULL != pProvider->pProfile)
    {
        return NULL;
    }
    return pProvider->pProfilePubSub;
}

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider get-length operations
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(const SOPC_CryptoProvider* pProvider,
                                                                   uint32_t* pLength)
{
    if (NULL == pProvider || NULL == pLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProvider_GetProfilePubSub(pProvider);
    uint32_t uSecPolID = SOPC_SecurityPolicy_Invalid_ID;
    if (NULL != pProfile)
    {
        uSecPolID = pProfile->SecurityPolicyID;
    }
    else if (NULL != pProfilePubSub)
    {
        uSecPolID = pProfilePubSub->SecurityPolicyID;
    }
    else
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (uSecPolID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        *pLength = SOPC_SecurityPolicy_Aes256Sha256RsaPss_SymmLen_CryptoKey;
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        *pLength = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SymmLen_CryptoKey;
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256Sha256_SymmLen_CryptoKey;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256_SymmLen_CryptoKey;
        break;
    case SOPC_SecurityPolicy_PubSub_Aes256_ID:
        *pLength = SOPC_SecurityPolicy_PubSub_Aes256_SymmLen_CryptoKey;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Encryption(const SOPC_CryptoProvider* pProvider,
                                                                    uint32_t lengthIn,
                                                                    uint32_t* pLengthOut)
{
    if (NULL == pProvider || NULL == pLengthOut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProvider_GetProfilePubSub(pProvider);
    uint32_t uSecPolID = SOPC_SecurityPolicy_Invalid_ID;
    if (NULL != pProfile)
    {
        uSecPolID = pProfile->SecurityPolicyID;
    }
    else if (NULL != pProfilePubSub)
    {
        uSecPolID = pProfilePubSub->SecurityPolicyID;
    }
    else
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (uSecPolID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
    case SOPC_SecurityPolicy_Basic256_ID:
    case SOPC_SecurityPolicy_PubSub_Aes256_ID:
        *pLengthOut = lengthIn;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Decryption(const SOPC_CryptoProvider* pProvider,
                                                                    uint32_t lengthIn,
                                                                    uint32_t* pLengthOut)
{
    if (NULL == pProvider || NULL == pLengthOut)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProvider_GetProfilePubSub(pProvider);
    uint32_t uSecPolID = SOPC_SecurityPolicy_Invalid_ID;
    if (NULL != pProfile)
    {
        uSecPolID = pProfile->SecurityPolicyID;
    }
    else if (NULL != pProfilePubSub)
    {
        uSecPolID = pProfilePubSub->SecurityPolicyID;
    }
    else
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (uSecPolID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
    case SOPC_SecurityPolicy_Basic256_ID:
    case SOPC_SecurityPolicy_PubSub_Aes256_ID:
        *pLengthOut = lengthIn;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_SignKey(const SOPC_CryptoProvider* pProvider,
                                                                 uint32_t* pLength)
{
    if (NULL == pProvider || NULL == pLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProvider_GetProfilePubSub(pProvider);
    uint32_t uSecPolID = SOPC_SecurityPolicy_Invalid_ID;
    if (NULL != pProfile)
    {
        uSecPolID = pProfile->SecurityPolicyID;
    }
    else if (NULL != pProfilePubSub)
    {
        uSecPolID = pProfilePubSub->SecurityPolicyID;
    }
    else
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (uSecPolID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        *pLength = SOPC_SecurityPolicy_Aes256Sha256RsaPss_SymmLen_SignKey;
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        *pLength = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SymmLen_SignKey;
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256Sha256_SymmLen_SignKey;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256_SymmLen_SignKey;
        break;
    case SOPC_SecurityPolicy_PubSub_Aes256_ID:
        *pLength = SOPC_SecurityPolicy_PubSub_Aes256_SymmLen_SignKey;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Signature(const SOPC_CryptoProvider* pProvider,
                                                                   uint32_t* pLength)
{
    if (NULL == pProvider || NULL == pLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProvider_GetProfilePubSub(pProvider);
    uint32_t uSecPolID = SOPC_SecurityPolicy_Invalid_ID;
    if (NULL != pProfile)
    {
        uSecPolID = pProfile->SecurityPolicyID;
    }
    else if (NULL != pProfilePubSub)
    {
        uSecPolID = pProfilePubSub->SecurityPolicyID;
    }
    else
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (uSecPolID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        *pLength = SOPC_SecurityPolicy_Aes256Sha256RsaPss_SymmLen_Signature;
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        *pLength = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SymmLen_Signature;
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Signature;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256_SymmLen_Signature;
        break;
    case SOPC_SecurityPolicy_PubSub_Aes256_ID:
        *pLength = SOPC_SecurityPolicy_PubSub_Aes256_SymmLen_Signature;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Blocks(const SOPC_CryptoProvider* pProvider,
                                                                uint32_t* pCipherTextBlockSize,
                                                                uint32_t* pPlainTextBlockSize)
{
    if (NULL == pProvider)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        if (NULL != pCipherTextBlockSize)
        {
            *pCipherTextBlockSize = SOPC_SecurityPolicy_Aes256Sha256RsaPss_SymmLen_Block;
        }
        if (NULL != pPlainTextBlockSize)
        {
            *pPlainTextBlockSize = SOPC_SecurityPolicy_Aes256Sha256RsaPss_SymmLen_Block;
        }
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        if (NULL != pCipherTextBlockSize)
        {
            *pCipherTextBlockSize = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SymmLen_Block;
        }
        if (NULL != pPlainTextBlockSize)
        {
            *pPlainTextBlockSize = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SymmLen_Block;
        }
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if (NULL != pCipherTextBlockSize)
        {
            *pCipherTextBlockSize = SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block;
        }
        if (NULL != pPlainTextBlockSize)
        {
            *pPlainTextBlockSize = SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block;
        }
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if (NULL != pCipherTextBlockSize)
        {
            *pCipherTextBlockSize = SOPC_SecurityPolicy_Basic256_SymmLen_Block;
        }
        if (NULL != pPlainTextBlockSize)
        {
            *pPlainTextBlockSize = SOPC_SecurityPolicy_Basic256_SymmLen_Block;
        }
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_SecureChannelNonce(const SOPC_CryptoProvider* pProvider,
                                                                            uint32_t* pLenNonce)
{
    if (NULL == pProvider)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        *pLenNonce = SOPC_SecurityPolicy_Aes256Sha256RsaPss_SecureChannelNonceLength;
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        *pLenNonce = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SecureChannelNonceLength;
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *pLenNonce = SOPC_SecurityPolicy_Basic256Sha256_SecureChannelNonceLength;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLenNonce = SOPC_SecurityPolicy_Basic256_SecureChannelNonceLength;
        break;
    }
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_DeriveGetLengths(const SOPC_CryptoProvider* pProvider,
                                                       uint32_t* pSymmCryptoKeyLength,
                                                       uint32_t* pSymmSignKeyLength,
                                                       uint32_t* pSymmInitVectorLength)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (NULL == pProvider || NULL == pSymmCryptoKeyLength || NULL == pSymmSignKeyLength ||
        NULL == pSymmInitVectorLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *pSymmCryptoKeyLength = 0;
    *pSymmSignKeyLength = 0;
    *pSymmInitVectorLength = 0;

    status = SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(pProvider, pSymmCryptoKeyLength);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CryptoProvider_SymmetricGetLength_SignKey(pProvider, pSymmSignKeyLength);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CryptoProvider_SymmetricGetLength_Blocks(pProvider, pSymmInitVectorLength, NULL);
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes(const SOPC_CryptoProvider* pProvider,
                                                                   const SOPC_AsymmetricKey* pKey,
                                                                   uint32_t* pLenKeyBits)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    status = SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKey, pLenKeyBits);
    if (SOPC_STATUS_OK == status)
        *pLenKeyBits /= 8;

    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_OAEPHashLength(const SOPC_CryptoProvider* pProvider,
                                                                         uint32_t* pLength)
{
    if (NULL == pProvider || NULL == pLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        *pLength = SOPC_SecurityPolicy_Aes256Sha256RsaPss_AsymLen_OAEP_Hash;
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        *pLength = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_AsymLen_OAEP_Hash;
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256Sha256_AsymLen_OAEP_Hash;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256_AsymLen_OAEP_Hash;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_Msgs(const SOPC_CryptoProvider* pProvider,
                                                               const SOPC_AsymmetricKey* pKey,
                                                               uint32_t* pCipherTextBlockSize,
                                                               uint32_t* pPlainTextBlockSize)
{
    SOPC_ReturnStatus statusA = SOPC_STATUS_OK, statusB = SOPC_STATUS_OK;

    if (NULL == pProvider || NULL == pKey)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (NULL != pCipherTextBlockSize)
    {
        *pCipherTextBlockSize = 0;
        statusA = SOPC_CryptoProvider_AsymmetricGetLength_MsgCipherText(pProvider, pKey, pCipherTextBlockSize);
    }
    if (NULL != pPlainTextBlockSize)
    {
        *pPlainTextBlockSize = 0;
        statusB = SOPC_CryptoProvider_AsymmetricGetLength_MsgPlainText(pProvider, pKey, pPlainTextBlockSize);
    }

    if (SOPC_STATUS_OK != statusA || SOPC_STATUS_OK != statusB)
        return SOPC_STATUS_NOK;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_Encryption(const SOPC_CryptoProvider* pProvider,
                                                                     const SOPC_AsymmetricKey* pKey,
                                                                     uint32_t lengthIn,
                                                                     uint32_t* pLengthOut)
{
    uint32_t lenCiph = 0, lenPlain = 0;
    uint32_t nMsgs = 0;

    if (NULL == pProvider || NULL == pKey || NULL == pLengthOut)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (0 == lengthIn)
    {
        *pLengthOut = 0;
        return SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK != SOPC_CryptoProvider_AsymmetricGetLength_Msgs(pProvider, pKey, &lenCiph, &lenPlain))
        return SOPC_STATUS_NOK;

    // Calculates the number of messages
    nMsgs = lengthIn / lenPlain;
    if ((lengthIn % lenPlain) > 0)
        ++nMsgs;

    // Deduces the output length
    *pLengthOut = nMsgs * lenCiph;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_Decryption(const SOPC_CryptoProvider* pProvider,
                                                                     const SOPC_AsymmetricKey* pKey,
                                                                     uint32_t lengthIn,
                                                                     uint32_t* pLengthOut)
{
    uint32_t lenCiph = 0, lenPlain = 0;
    uint32_t nMsgs = 0;

    if (NULL == pProvider || NULL == pKey || NULL == pLengthOut)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (0 == lengthIn)
    {
        *pLengthOut = 0;
        return SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK != SOPC_CryptoProvider_AsymmetricGetLength_Msgs(pProvider, pKey, &lenCiph, &lenPlain))
        return SOPC_STATUS_NOK;

    // Calculates the number of messages
    nMsgs = lengthIn / lenCiph;
    if ((lengthIn % lenCiph) > 0)
        ++nMsgs;

    // Deduces the output length
    *pLengthOut = nMsgs * lenPlain;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_Signature(const SOPC_CryptoProvider* pProvider,
                                                                    const SOPC_AsymmetricKey* pKey,
                                                                    uint32_t* pLength)
{
    if (NULL == pProvider || NULL == pKey || NULL == pLength)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // The signature is a message long.
    return SOPC_CryptoProvider_AsymmetricGetLength_Msgs(pProvider, pKey, pLength, NULL);
}

const char* SOPC_CryptoProvider_AsymmetricGetUri_SignAlgorithm(const SOPC_CryptoProvider* pProvider)
{
    if (NULL == pProvider)
    {
        return NULL;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile)
    {
        return NULL;
    }

    switch (pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return NULL;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        return SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI_SignAlgo;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        return SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI_SignAlgo;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        return SOPC_SecurityPolicy_Basic256Sha256_URI_SignAlgo;
    case SOPC_SecurityPolicy_Basic256_ID:
        return SOPC_SecurityPolicy_Basic256_URI_SignAlgo;
    }
}

SOPC_ReturnStatus SOPC_CryptoProvider_CertificateGetLength_Thumbprint(const SOPC_CryptoProvider* pProvider,
                                                                      uint32_t* pLength)
{
    if (NULL == pProvider)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *pLength = 0;
    switch (pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_NOK;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        *pLength = SOPC_SecurityPolicy_Aes256Sha256RsaPss_CertLen_Thumbprint;
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        *pLength = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_CertLen_Thumbprint;
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256Sha256_CertLen_Thumbprint;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256_CertLen_Thumbprint;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_PubSubGetLength_KeyNonce(const SOPC_CryptoProvider* pProvider, uint32_t* pLength)
{
    if (NULL == pProvider || NULL == pLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProvider_GetProfilePubSub(pProvider);
    if (NULL == pProfilePubSub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (pProfilePubSub->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_PubSub_Aes256_ID:
        *pLength = SOPC_SecurityPolicy_PubSub_Aes256_SymmLen_KeyNonce;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_PubSubGetLength_MessageRandom(const SOPC_CryptoProvider* pProvider,
                                                                    uint32_t* pLength)
{
    if (NULL == pProvider || NULL == pLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProvider_GetProfilePubSub(pProvider);
    if (NULL == pProfilePubSub)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    switch (pProfilePubSub->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_PubSub_Aes256_ID:
        *pLength = SOPC_SecurityPolicy_PubSub_Aes256_SymmLen_MessageRandom;
        break;
    }

    return SOPC_STATUS_OK;
}

/* ------------------------------------------------------------------------------------------------
 * Symmetric cryptography
 * ------------------------------------------------------------------------------------------------
 */
SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricEncrypt(const SOPC_CryptoProvider* pProvider,
                                                       const uint8_t* pInput,
                                                       uint32_t lenPlainText,
                                                       SOPC_SecretBuffer* pKey,
                                                       SOPC_SecretBuffer* pIV,
                                                       uint8_t* pOutput,
                                                       uint32_t lenOutput)
{
    const SOPC_ExposedBuffer* pExpKey = NULL;
    const SOPC_ExposedBuffer* pExpIV = NULL;
    uint32_t lenCiphered = 0;

    if (NULL == pProvider || NULL == pInput || NULL == pKey || NULL == pIV || NULL == pOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile || NULL == pProfile->pFnSymmEncrypt)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_CryptoProvider_SymmetricGetLength_Encryption(pProvider, lenPlainText, &lenCiphered);
    if (SOPC_STATUS_OK != status || lenCiphered != lenOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // TODO: unit-test these watchdogs
    switch (pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        if ((lenPlainText % SOPC_SecurityPolicy_Aes256Sha256RsaPss_SymmLen_Block) != 0) // Not block-aligned
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pKey) !=
            SOPC_SecurityPolicy_Aes256Sha256RsaPss_SymmLen_CryptoKey) // Wrong key size
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pIV) != SOPC_SecurityPolicy_Aes256Sha256RsaPss_SymmLen_Block)
        { // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        if ((lenPlainText % SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SymmLen_Block) != 0) // Not block-aligned
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pKey) !=
            SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SymmLen_CryptoKey) // Wrong key size
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pIV) != SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SymmLen_Block)
        { // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if ((lenPlainText % SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block) != 0) // Not block-aligned
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pKey) != SOPC_SecurityPolicy_Basic256Sha256_SymmLen_CryptoKey) // Wrong key size
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pIV) != SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block)
        { // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if ((lenPlainText % SOPC_SecurityPolicy_Basic256_SymmLen_Block) != 0) // Not block-aligned
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pKey) != SOPC_SecurityPolicy_Basic256_SymmLen_CryptoKey) // Wrong key size
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pIV) != SOPC_SecurityPolicy_Basic256_SymmLen_Block)
        { // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    }

    pExpKey = SOPC_SecretBuffer_Expose(pKey);
    pExpIV = SOPC_SecretBuffer_Expose(pIV);

    status = pProfile->pFnSymmEncrypt(pProvider, pInput, lenPlainText, pExpKey, pExpIV, pOutput, lenOutput);

    SOPC_SecretBuffer_Unexpose(pExpKey, pKey);
    SOPC_SecretBuffer_Unexpose(pExpIV, pIV);

    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricDecrypt(const SOPC_CryptoProvider* pProvider,
                                                       const uint8_t* pInput,
                                                       uint32_t lenCipherText,
                                                       SOPC_SecretBuffer* pKey,
                                                       SOPC_SecretBuffer* pIV,
                                                       uint8_t* pOutput,
                                                       uint32_t lenOutput)
{
    const SOPC_ExposedBuffer* pExpKey = NULL;
    const SOPC_ExposedBuffer* pExpIV = NULL;
    uint32_t lenDeciphered = 0;

    if (NULL == pProvider || NULL == pInput || NULL == pKey || NULL == pIV || NULL == pOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile || NULL == pProfile->pFnSymmDecrypt)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status =
        SOPC_CryptoProvider_SymmetricGetLength_Decryption(pProvider, lenCipherText, &lenDeciphered);
    if (SOPC_STATUS_OK != status || lenDeciphered != lenOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // TODO: unit-test these watchdogs
    switch (pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        if ((lenCipherText % SOPC_SecurityPolicy_Aes256Sha256RsaPss_SymmLen_Block) != 0) // Not block-aligned
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pKey) !=
            SOPC_SecurityPolicy_Aes256Sha256RsaPss_SymmLen_CryptoKey) // Wrong key size
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pIV) != SOPC_SecurityPolicy_Aes256Sha256RsaPss_SymmLen_Block)
        { // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        if ((lenCipherText % SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SymmLen_Block) != 0) // Not block-aligned
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pKey) !=
            SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SymmLen_CryptoKey) // Wrong key size
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pIV) != SOPC_SecurityPolicy_Aes128Sha256RsaOaep_SymmLen_Block)
        { // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if ((lenCipherText % SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block) != 0) // Not block-aligned
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pKey) != SOPC_SecurityPolicy_Basic256Sha256_SymmLen_CryptoKey) // Wrong key size
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pIV) != SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block)
        { // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if ((lenCipherText % SOPC_SecurityPolicy_Basic256_SymmLen_Block) != 0) // Not block-aligned
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pKey) != SOPC_SecurityPolicy_Basic256_SymmLen_CryptoKey) // Wrong key size
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        if (SOPC_SecretBuffer_GetLength(pIV) != SOPC_SecurityPolicy_Basic256_SymmLen_Block)
        { // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    }

    pExpKey = SOPC_SecretBuffer_Expose(pKey);
    pExpIV = SOPC_SecretBuffer_Expose(pIV);

    status = pProfile->pFnSymmDecrypt(pProvider, pInput, lenCipherText, pExpKey, pExpIV, pOutput, lenOutput);

    SOPC_SecretBuffer_Unexpose(pExpKey, pKey);
    SOPC_SecretBuffer_Unexpose(pExpIV, pIV);

    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_PubSubCrypt(const SOPC_CryptoProvider* pProvider,
                                                  const uint8_t* pInput,
                                                  uint32_t lenInput,
                                                  SOPC_SecretBuffer* pKey,
                                                  SOPC_SecretBuffer* pKeyNonce,
                                                  const SOPC_ExposedBuffer* pRandom,
                                                  uint32_t lenRandom,
                                                  uint32_t uSequenceNumber,
                                                  uint8_t* pOutput,
                                                  uint32_t lenOutput)
{
    if (NULL == pProvider || NULL == pInput || NULL == pKey || NULL == pKeyNonce || NULL == pRandom || NULL == pOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (lenInput != lenOutput || 0 == lenInput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProvider_GetProfilePubSub(pProvider);
    if (NULL == pProfilePubSub || NULL == pProfilePubSub->pFnCrypt)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* TODO: unit-test these watchdogs */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool bInvalid = false;
    switch (pProfilePubSub->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_PubSub_Aes256_ID:
        /* Key size check, KeyNonce size check, MessageRandom size check */
        bInvalid = (SOPC_SecretBuffer_GetLength(pKey) != SOPC_SecurityPolicy_PubSub_Aes256_SymmLen_CryptoKey ||
                    SOPC_SecretBuffer_GetLength(pKeyNonce) != SOPC_SecurityPolicy_PubSub_Aes256_SymmLen_KeyNonce ||
                    SOPC_SecurityPolicy_PubSub_Aes256_SymmLen_MessageRandom != lenRandom);
        if (bInvalid)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    }

    if (SOPC_STATUS_OK == status)
    {
        const SOPC_ExposedBuffer* pExpKey = SOPC_SecretBuffer_Expose(pKey);
        const SOPC_ExposedBuffer* pExpNonce = SOPC_SecretBuffer_Expose(pKeyNonce);

        status = pProfilePubSub->pFnCrypt(pProvider, pInput, lenInput, pExpKey, pExpNonce, pRandom, uSequenceNumber,
                                          pOutput);

        SOPC_SecretBuffer_Unexpose(pExpKey, pKey);
        SOPC_SecretBuffer_Unexpose(pExpNonce, pKeyNonce);
    }

    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricSign(const SOPC_CryptoProvider* pProvider,
                                                    const uint8_t* pInput,
                                                    uint32_t lenInput,
                                                    SOPC_SecretBuffer* pKey,
                                                    uint8_t* pOutput,
                                                    uint32_t lenOutput)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const SOPC_ExposedBuffer* pExpKey = NULL;
    uint32_t len;

    if (NULL == pProvider || NULL == pInput || NULL == pKey || NULL == pOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProvider_GetProfilePubSub(pProvider);
    FnSymmetricSign* pFnSign = NULL;
    if (NULL != pProfile)
    {
        pFnSign = pProfile->pFnSymmSign;
    }
    else if (NULL != pProfilePubSub)
    {
        pFnSign = pProfilePubSub->pFnSymmSign;
    }

    if (NULL == pFnSign)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Assert output size
    if (SOPC_CryptoProvider_SymmetricGetLength_Signature(pProvider, &len) != SOPC_STATUS_OK)
    {
        return SOPC_STATUS_NOK;
    }
    if (lenOutput != len)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Assert key size
    if (SOPC_CryptoProvider_SymmetricGetLength_SignKey(pProvider, &len) != SOPC_STATUS_OK)
    {
        return SOPC_STATUS_NOK;
    }
    if (SOPC_SecretBuffer_GetLength(pKey) != len)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pExpKey = SOPC_SecretBuffer_Expose(pKey);
    if (NULL == pExpKey)
    {
        return SOPC_STATUS_NOK;
    }

    status = pFnSign(pProvider, pInput, lenInput, pExpKey, pOutput);

    SOPC_SecretBuffer_Unexpose(pExpKey, pKey);
    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricVerify(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenInput,
                                                      SOPC_SecretBuffer* pKey,
                                                      const uint8_t* pSignature,
                                                      uint32_t lenOutput)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const SOPC_ExposedBuffer* pExpKey = NULL;
    uint32_t len;

    if (NULL == pProvider || NULL == pInput || NULL == pKey || NULL == pSignature)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProvider_GetProfilePubSub(pProvider);
    FnSymmetricVerify* pFnVerif = NULL;
    if (NULL != pProfile)
    {
        pFnVerif = pProfile->pFnSymmVerif;
    }
    else if (NULL != pProfilePubSub)
    {
        pFnVerif = pProfilePubSub->pFnSymmVerif;
    }

    if (NULL == pFnVerif)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Assert output size
    if (SOPC_CryptoProvider_SymmetricGetLength_Signature(pProvider, &len) != SOPC_STATUS_OK)
    {
        return SOPC_STATUS_NOK;
    }
    if (lenOutput != len)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Assert key size
    if (SOPC_CryptoProvider_SymmetricGetLength_SignKey(pProvider, &len) != SOPC_STATUS_OK)
    {
        return SOPC_STATUS_NOK;
    }
    if (SOPC_SecretBuffer_GetLength(pKey) != len)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pExpKey = SOPC_SecretBuffer_Expose(pKey);
    if (NULL == pExpKey)
    {
        return SOPC_STATUS_NOK;
    }

    status = pFnVerif(pProvider, pInput, lenInput, pExpKey, pSignature);

    SOPC_SecretBuffer_Unexpose(pExpKey, pKey);
    return status;
}

/* ------------------------------------------------------------------------------------------------
 * Random and pseudo-random functionalities
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus SOPC_CryptoProvider_GenerateRandomBytes(const SOPC_CryptoProvider* pProvider,
                                                          uint32_t nBytes,
                                                          SOPC_ExposedBuffer** ppBuffer)
{
    if (NULL == pProvider || nBytes == 0 || NULL == ppBuffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    const SOPC_CryptoProfile_PubSub* pProfilePubSub = SOPC_CryptoProvider_GetProfilePubSub(pProvider);
    FnGenerateRandom* pFnRnd = NULL;
    if (NULL != pProfile)
    {
        pFnRnd = pProfile->pFnGenRnd;
    }
    else if (NULL != pProfilePubSub)
    {
        pFnRnd = pProfilePubSub->pFnGenRnd;
    }

    if (NULL == pFnRnd)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ExposedBuffer* pExp = NULL;

    /* Empties pointer in case an error occurs after that point. */
    *ppBuffer = NULL;

    pExp = SOPC_Malloc(nBytes);
    if (NULL == pExp)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = pFnRnd(pProvider, pExp, nBytes);
        if (SOPC_STATUS_OK == status)
        {
            *ppBuffer = pExp;
        }
        else
        {
            SOPC_Free(pExp);
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_GenerateSecureChannelNonce(const SOPC_CryptoProvider* pProvider,
                                                                 SOPC_SecretBuffer** ppNonce)
{
    uint32_t lenNonce = 0;
    SOPC_ExposedBuffer* pExp = NULL;

    if (NULL == pProvider || NULL == ppNonce)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_CryptoProvider_SymmetricGetLength_SecureChannelNonce(pProvider, &lenNonce);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CryptoProvider_GenerateRandomBytes(pProvider, lenNonce, &pExp);
    }

    if (SOPC_STATUS_OK == status)
    {
        *ppNonce = SOPC_SecretBuffer_NewFromExposedBuffer(pExp, lenNonce);
        if (NULL == *ppNonce)
        {
            status = SOPC_STATUS_NOK;
        }

        memset(pExp, 0, lenNonce);
        SOPC_Free(pExp);
    }

    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_GenerateRandomID(const SOPC_CryptoProvider* pProvider, uint32_t* pID)
{
    if (NULL == pProvider || NULL == pID)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile || NULL == pProfile->pFnGenRnd)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return pProfile->pFnGenRnd(pProvider, (SOPC_ExposedBuffer*) pID, sizeof(uint32_t));
}

SOPC_ReturnStatus SOPC_CryptoProvider_DerivePseudoRandomData(const SOPC_CryptoProvider* pProvider,
                                                             const SOPC_ExposedBuffer* pSecret,
                                                             uint32_t lenSecret,
                                                             const SOPC_ExposedBuffer* pSeed,
                                                             uint32_t lenSeed,
                                                             SOPC_ExposedBuffer* pOutput,
                                                             uint32_t lenOutput)
{
    if (NULL == pProvider || NULL == pProvider->pCryptolibContext || NULL == pSecret || 0 == lenSecret ||
        NULL == pSeed || 0 == lenSeed || NULL == pOutput || 0 == lenOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile || NULL == pProfile->pFnDeriveData)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return pProfile->pFnDeriveData(pProvider, pSecret, lenSecret, pSeed, lenSeed, pOutput, lenOutput);
}

static inline SOPC_ReturnStatus DeriveKS(const SOPC_CryptoProvider* pProvider,
                                         const SOPC_ExposedBuffer* pSecret,
                                         uint32_t lenSecret,
                                         const SOPC_ExposedBuffer* pSeed,
                                         uint32_t lenSeed,
                                         SOPC_SC_SecurityKeySet* pks,
                                         uint8_t* genData,
                                         uint32_t lenData,
                                         uint32_t lenKeySign,
                                         uint32_t lenKeyEncr,
                                         uint32_t lenIV);
SOPC_ReturnStatus SOPC_CryptoProvider_DeriveKeySets(const SOPC_CryptoProvider* pProvider,
                                                    const SOPC_ExposedBuffer* pClientNonce,
                                                    uint32_t lenClientNonce,
                                                    const SOPC_ExposedBuffer* pServerNonce,
                                                    uint32_t lenServerNonce,
                                                    SOPC_SC_SecurityKeySet* pClientKeySet,
                                                    SOPC_SC_SecurityKeySet* pServerKeySet)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint8_t* genData = NULL;
    uint32_t lenData = 0;
    uint32_t lenKeyEncr = 0, lenKeySign = 0, lenIV = 0;

    // Verify pointers
    if (NULL == pProvider || NULL == pClientNonce || NULL == pServerNonce || NULL == pClientKeySet ||
        NULL == pServerKeySet)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (NULL == pClientKeySet->signKey || NULL == pClientKeySet->encryptKey || NULL == pClientKeySet->initVector)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (NULL == pServerKeySet->signKey || NULL == pServerKeySet->encryptKey || NULL == pServerKeySet->initVector)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Calculate expected lengths
    if (SOPC_CryptoProvider_DeriveGetLengths(pProvider, &lenKeyEncr, &lenKeySign, &lenIV) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    // Verify lengths
    if (SOPC_SecretBuffer_GetLength(pClientKeySet->signKey) != lenKeySign ||
        SOPC_SecretBuffer_GetLength(pClientKeySet->encryptKey) != lenKeyEncr ||
        SOPC_SecretBuffer_GetLength(pClientKeySet->initVector) != lenIV)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (SOPC_SecretBuffer_GetLength(pServerKeySet->signKey) != lenKeySign ||
        SOPC_SecretBuffer_GetLength(pServerKeySet->encryptKey) != lenKeyEncr ||
        SOPC_SecretBuffer_GetLength(pServerKeySet->initVector) != lenIV)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Allocate buffer for PRF generated data
    lenData = lenKeySign + lenKeyEncr + lenIV;
    genData = SOPC_Malloc(lenData);
    if (NULL == genData)
        return SOPC_STATUS_NOK;

    // Derives keyset for the client
    status = DeriveKS(pProvider, pServerNonce, lenServerNonce, pClientNonce, lenClientNonce, pClientKeySet, genData,
                      lenData, lenKeySign, lenKeyEncr, lenIV);
    // Derives keyset for the server
    if (SOPC_STATUS_OK == status)
        status = DeriveKS(pProvider, pClientNonce, lenClientNonce, pServerNonce, lenServerNonce, pServerKeySet, genData,
                          lenData, lenKeySign, lenKeyEncr, lenIV);

    // Clears and delete
    memset(genData, 0, lenData);
    SOPC_Free(genData);

    return status;
}

static inline SOPC_ReturnStatus DeriveKS(const SOPC_CryptoProvider* pProvider,
                                         const SOPC_ExposedBuffer* pSecret,
                                         uint32_t lenSecret,
                                         const SOPC_ExposedBuffer* pSeed,
                                         uint32_t lenSeed,
                                         SOPC_SC_SecurityKeySet* pks,
                                         uint8_t* genData,
                                         uint32_t lenData,
                                         uint32_t lenKeySign,
                                         uint32_t lenKeyEncr,
                                         uint32_t lenIV)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ExposedBuffer *pExpEncr = NULL, *pExpSign = NULL, *pExpIV = NULL;

    // Exposes SecretBuffers
    pExpEncr = SOPC_SecretBuffer_ExposeModify(pks->encryptKey);
    pExpSign = SOPC_SecretBuffer_ExposeModify(pks->signKey);
    pExpIV = SOPC_SecretBuffer_ExposeModify(pks->initVector);

    // Verifies exposures
    if (NULL == pExpEncr || NULL == pExpSign || NULL == pExpIV)
        return SOPC_STATUS_NOK;

    // Generates KeySet
    status =
        SOPC_CryptoProvider_DerivePseudoRandomData(pProvider, pSecret, lenSecret, pSeed, lenSeed, genData, lenData);
    if (SOPC_STATUS_OK == status)
    {
        memcpy(pExpSign, genData, lenKeySign);
        memcpy(pExpEncr, genData + lenKeySign, lenKeyEncr);
        memcpy(pExpIV, genData + lenKeySign + lenKeyEncr, lenIV);
    }

    // Release ExposedBuffers
    SOPC_SecretBuffer_UnexposeModify(pExpEncr, pks->encryptKey);
    SOPC_SecretBuffer_UnexposeModify(pExpSign, pks->signKey);
    SOPC_SecretBuffer_UnexposeModify(pExpIV, pks->initVector);

    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_DeriveKeySetsClient(const SOPC_CryptoProvider* pProvider,
                                                          SOPC_SecretBuffer* pClientNonce,
                                                          const SOPC_ExposedBuffer* pServerNonce,
                                                          uint32_t lenServerNonce,
                                                          SOPC_SC_SecurityKeySet* pClientKeySet,
                                                          SOPC_SC_SecurityKeySet* pServerKeySet)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const SOPC_ExposedBuffer* pExpCli = NULL;

    if (NULL == pProvider || NULL == pClientNonce || NULL == pServerNonce || NULL == pClientKeySet ||
        NULL == pServerKeySet)
        return SOPC_STATUS_INVALID_PARAMETERS;

    pExpCli = SOPC_SecretBuffer_Expose(pClientNonce);
    if (NULL == pExpCli)
        status = SOPC_STATUS_INVALID_PARAMETERS;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CryptoProvider_DeriveKeySets(pProvider, pExpCli, SOPC_SecretBuffer_GetLength(pClientNonce),
                                                   pServerNonce, lenServerNonce, pClientKeySet, pServerKeySet);
    }

    SOPC_SecretBuffer_Unexpose(pExpCli, pClientNonce);

    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_DeriveKeySetsServer(const SOPC_CryptoProvider* pProvider,
                                                          const SOPC_ExposedBuffer* pClientNonce,
                                                          uint32_t lenClientNonce,
                                                          SOPC_SecretBuffer* pServerNonce,
                                                          SOPC_SC_SecurityKeySet* pClientKeySet,
                                                          SOPC_SC_SecurityKeySet* pServerKeySet)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const SOPC_ExposedBuffer* pExpSer = NULL;

    if (NULL == pProvider || NULL == pClientNonce || NULL == pServerNonce || NULL == pClientKeySet ||
        NULL == pServerKeySet)
        return SOPC_STATUS_INVALID_PARAMETERS;

    pExpSer = SOPC_SecretBuffer_Expose(pServerNonce);
    if (NULL == pExpSer)
        status = SOPC_STATUS_INVALID_PARAMETERS;

    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_CryptoProvider_DeriveKeySets(pProvider, pClientNonce, lenClientNonce, pExpSer,
                                              SOPC_SecretBuffer_GetLength(pServerNonce), pClientKeySet, pServerKeySet);
    }

    SOPC_SecretBuffer_Unexpose(pExpSer, pServerNonce);

    return status;
}

/* ------------------------------------------------------------------------------------------------
 * Asymmetric API
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricEncrypt(const SOPC_CryptoProvider* pProvider,
                                                        const uint8_t* pInput,
                                                        uint32_t lenInput,
                                                        const SOPC_AsymmetricKey* pKey,
                                                        uint8_t* pOutput,
                                                        uint32_t lenOutput,
                                                        const char** errorReason)
{
    assert(NULL != errorReason);
    *errorReason = "";

    uint32_t lenOutCalc = 0;
    uint32_t lenKey = 0;

    if (NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKey || NULL == pOutput || 0 == lenOutput)
    {
        *errorReason = "NULL parameter or 0 length provided";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile || NULL == pProfile->pFnAsymEncrypt)
    {
        *errorReason = "invalid cryptographic provider (invalid profile)";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Check buffer length
    if (SOPC_CryptoProvider_AsymmetricGetLength_Encryption(pProvider, pKey, lenInput, &lenOutCalc) != SOPC_STATUS_OK)
    {
        *errorReason = "error during computation of encrypted message size from public key";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (lenOutput != lenOutCalc)
    {
        *errorReason = "computed encrypted length from public key is not equal to output buffer provided";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Check key length
    if (SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKey, &lenKey) != SOPC_STATUS_OK)
    {
        *errorReason = "error extracting key length from public key";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    switch (pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        *errorReason = "invalid security policy in cryptographic provider";
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        if (lenKey < SOPC_SecurityPolicy_Aes256Sha256RsaPss_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Aes256Sha256RsaPss_AsymLen_KeyMaxBits)
        {
            *errorReason =
                "invalid public key size for Aes256-Sha256-RsaPss profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        if (lenKey < SOPC_SecurityPolicy_Aes128Sha256RsaOaep_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Aes128Sha256RsaOaep_AsymLen_KeyMaxBits)
        {
            *errorReason =
                "invalid public key size for Aes128-Sha256-RsaOaep profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
        {
            *errorReason = "invalid public key size for Basic256Sha256 profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256_AsymLen_KeyMaxBits)
        {
            *errorReason = "invalid public key size for Basic256 profile, expected 1024 <= keyLength <= 2048";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    }

    // We can now proceed
    SOPC_ReturnStatus status = pProfile->pFnAsymEncrypt(pProvider, pInput, lenInput, pKey, pOutput);
    if (SOPC_STATUS_OK != status)
    {
        *errorReason = "encryption processing failed (invalid key type or message length)";
    }
    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricDecrypt(const SOPC_CryptoProvider* pProvider,
                                                        const uint8_t* pInput,
                                                        uint32_t lenInput,
                                                        const SOPC_AsymmetricKey* pKey,
                                                        uint8_t* pOutput,
                                                        uint32_t lenOutput,
                                                        uint32_t* pLenWritten,
                                                        const char** errorReason)
{
    assert(NULL != errorReason);
    *errorReason = "";

    uint32_t lenOutCalc = 0;
    uint32_t lenKey = 0;

    if (NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKey || NULL == pOutput || 0 == lenOutput)
    {
        *errorReason = "NULL parameter or 0 length provided";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile || NULL == pProfile->pFnAsymDecrypt)
    {
        *errorReason = "invalid cryptographic provider (invalid profile)";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Check buffer length
    if (SOPC_CryptoProvider_AsymmetricGetLength_Decryption(pProvider, pKey, lenInput, &lenOutCalc) != SOPC_STATUS_OK)
    {
        *errorReason = "error during computation of encrypted message size from private key";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (lenOutput != lenOutCalc)
    {
        *errorReason = "computed encrypted length from private key is not equal to output buffer provided";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Check key length
    if (SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKey, &lenKey) != SOPC_STATUS_OK)
    {
        *errorReason = "error extracting key length from private key";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    switch (pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        *errorReason = "invalid security policy in cryptographic provider";
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        if (lenKey < SOPC_SecurityPolicy_Aes256Sha256RsaPss_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Aes256Sha256RsaPss_AsymLen_KeyMaxBits)
        {
            *errorReason =
                "invalid private key size for Aes256-Sha256-RsaPss profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        if (lenKey < SOPC_SecurityPolicy_Aes128Sha256RsaOaep_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Aes128Sha256RsaOaep_AsymLen_KeyMaxBits)
        {
            *errorReason =
                "invalid private key size for Aes128-Sha256-RsaOaep profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
        {
            *errorReason = "invalid private key size for Basic256Sha256 profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256_AsymLen_KeyMaxBits)
        {
            *errorReason = "invalid private key size for Basic256 profile, expected 1024 <= keyLength <= 2048";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    }

    // We can now proceed
    SOPC_ReturnStatus status = pProfile->pFnAsymDecrypt(pProvider, pInput, lenInput, pKey, pOutput, pLenWritten);
    if (SOPC_STATUS_OK != status)
    {
        *errorReason = "decryption processing failed (invalid key type or message length)";
    }
    return status;
}

/**
 * Asymmetric signature works with asymmetric keys. \p pKey is the local private key.
 * A hash of \p pInput is computed then encrypted with the \p pKey. To verify the signature,
 * one decrypts the \p pSignature with the corresponding public key, computes the hash of \p pInput,
 * and verify that both hashes are the same. Everyone can decrypt the signature,
 * but only the private key could have forged it.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricSign(const SOPC_CryptoProvider* pProvider,
                                                     const uint8_t* pInput,
                                                     uint32_t lenInput,
                                                     const SOPC_AsymmetricKey* pKeyPrivateLocal,
                                                     uint8_t* pSignature,
                                                     uint32_t lenSignature,
                                                     const char** errorReason)
{
    assert(NULL != errorReason);
    *errorReason = "";

    uint32_t lenSigCalc = 0, lenKey = 0;

    if (NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKeyPrivateLocal || NULL == pSignature ||
        0 == lenSignature)
    {
        *errorReason = "NULL parameter or 0 length provided";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile || NULL == pProfile->pFnAsymSign)
    {
        *errorReason = "invalid cryptographic provider (invalid profile)";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Check lengths
    if (SOPC_CryptoProvider_AsymmetricGetLength_Signature(pProvider, pKeyPrivateLocal, &lenSigCalc) != SOPC_STATUS_OK)
    {
        *errorReason = "error during computation of signature size from private key";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (lenSignature != lenSigCalc)
    {
        *errorReason = "computed signature length from private key is not equal to the one computed from public key";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKeyPrivateLocal, &lenKey) != SOPC_STATUS_OK)
    {
        *errorReason = "error extracting key length from private key";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    switch (pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        *errorReason = "invalid security policy in cryptographic provider";
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        if (lenKey < SOPC_SecurityPolicy_Aes256Sha256RsaPss_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Aes256Sha256RsaPss_AsymLen_KeyMaxBits)
        {
            *errorReason =
                "invalid private key size for Aes256-Sha256-RsaPss profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        if (lenKey < SOPC_SecurityPolicy_Aes128Sha256RsaOaep_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Aes128Sha256RsaOaep_AsymLen_KeyMaxBits)
        {
            *errorReason =
                "invalid private key size for Aes128-Sha256-RsaOaep profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
        {
            *errorReason = "invalid private key size for Basic256Sha256 profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256_AsymLen_KeyMaxBits)
        {
            *errorReason = "invalid private key size for Basic256 profile, expected 1024 <= keyLength <= 2048";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    }

    SOPC_ReturnStatus status = pProfile->pFnAsymSign(pProvider, pInput, lenInput, pKeyPrivateLocal, pSignature);
    if (SOPC_STATUS_OK != status)
    {
        *errorReason = "signature processing failed";
    }
    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricVerify(const SOPC_CryptoProvider* pProvider,
                                                       const uint8_t* pInput,
                                                       uint32_t lenInput,
                                                       const SOPC_AsymmetricKey* pKeyRemotePublic,
                                                       const uint8_t* pSignature,
                                                       uint32_t lenSignature,
                                                       const char** errorReason)
{
    assert(NULL != errorReason);
    *errorReason = "";

    uint32_t lenSigCalc = 0, lenKey = 0;

    if (NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKeyRemotePublic || NULL == pSignature ||
        0 == lenSignature)
    {
        *errorReason = "NULL parameter or 0 length provided";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile || NULL == pProfile->pFnAsymVerify)
    {
        *errorReason = "invalid cryptographic provider (invalid profile)";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Check lengths
    if (SOPC_CryptoProvider_AsymmetricGetLength_Signature(pProvider, pKeyRemotePublic, &lenSigCalc) != SOPC_STATUS_OK)
    {
        *errorReason = "error during computation of signature size from public key";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (lenSignature != lenSigCalc)
    {
        *errorReason = "computed signature length is not equal to output buffer provided";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKeyRemotePublic, &lenKey) != SOPC_STATUS_OK)
    {
        *errorReason = "error extracting key length from public key";
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    switch (pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        *errorReason = "invalid security policy in cryptographic provider";
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID:
        if (lenKey < SOPC_SecurityPolicy_Aes256Sha256RsaPss_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Aes256Sha256RsaPss_AsymLen_KeyMaxBits)
        {
            *errorReason =
                "invalid public key size for Aes256-Sha256-RsaPss profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID:
        if (lenKey < SOPC_SecurityPolicy_Aes128Sha256RsaOaep_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Aes128Sha256RsaOaep_AsymLen_KeyMaxBits)
        {
            *errorReason =
                "invalid public key size for Aes128-Sha256-RsaOaep profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
        {
            *errorReason = "invalid public key size for Basic256Sha256 profile, expected 2048 <= keyLength <= 4096";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256_AsymLen_KeyMaxBits)
        {
            *errorReason = "invalid public key size for Basic256 profile, expected 1024 <= keyLength <= 2048";
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
        break;
    }

    SOPC_ReturnStatus status = pProfile->pFnAsymVerify(pProvider, pInput, lenInput, pKeyRemotePublic, pSignature);
    if (SOPC_STATUS_OK != status)
    {
        *errorReason = "signature processing failed";
    }
    return status;
}

/* ------------------------------------------------------------------------------------------------
 * Certificate validation
 * ------------------------------------------------------------------------------------------------
 */
SOPC_ReturnStatus SOPC_CryptoProvider_Certificate_Validate(const SOPC_CryptoProvider* pProvider,
                                                           const SOPC_PKIProvider* pPKI,
                                                           const SOPC_CertificateList* pCert,
                                                           uint32_t* error)
{
    assert(NULL != error);

    // TODO: where is the key key_pub <-> key_priv association checked?
    if (NULL == pProvider || NULL == pPKI || NULL == pPKI->pFnValidateCertificate || NULL == pCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile || NULL == pProfile->pFnCertVerify)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Let the lib-specific code handle the verification for the current security policy
    if (pProfile->pFnCertVerify(pProvider, pCert) != SOPC_STATUS_OK)
    {
        *error = SOPC_CertificateValidationError_Invalid;
        return SOPC_STATUS_NOK;
    }

    // Verify certificate through PKIProvider callback
    return pPKI->pFnValidateCertificate(pPKI, pCert, error);
}
