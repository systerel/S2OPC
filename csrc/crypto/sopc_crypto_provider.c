/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>

#include "sopc_crypto_provider.h"

#include "sopc_crypto_decl.h"
#include "sopc_crypto_profiles.h"
#include "sopc_key_manager.h"
#include "sopc_pki.h"
#include "sopc_secret_buffer.h"

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider creation
 * ------------------------------------------------------------------------------------------------
 */

SOPC_CryptoProvider* SOPC_CryptoProvider_Create(const char* uri)
{
    SOPC_CryptoProvider* pCryptoProvider = NULL;
    const SOPC_CryptoProfile* pProfile = NULL;

    pProfile = SOPC_CryptoProfile_Get(uri);
    if (NULL != pProfile)
    {
        pCryptoProvider = (SOPC_CryptoProvider*) malloc(sizeof(SOPC_CryptoProvider));
        if (NULL != pCryptoProvider)
        {
            // The crypto provider profile shall be const after this init
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            *(const SOPC_CryptoProfile**) (&pCryptoProvider->pProfile) = pProfile;
            SOPC_GCC_DIAGNOSTIC_RESTORE
            if (SOPC_STATUS_OK != SOPC_CryptoProvider_Init(pCryptoProvider))
            {
                free(pCryptoProvider);
                pCryptoProvider = NULL;
            }
        }
    }

    return pCryptoProvider;
}

void SOPC_CryptoProvider_Free(SOPC_CryptoProvider* pCryptoProvider)
{
    if (NULL != pCryptoProvider)
    {
        SOPC_CryptoProvider_Deinit(pCryptoProvider);
        free(pCryptoProvider);
    }
}

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider get-length operations
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(const SOPC_CryptoProvider* pProvider,
                                                                   uint32_t* pLength)
{
    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pLength)
        return SOPC_STATUS_INVALID_PARAMETERS;

    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_NOK;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256Sha256_SymmLen_CryptoKey;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256_SymmLen_CryptoKey;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Encryption(const SOPC_CryptoProvider* pProvider,
                                                                    uint32_t lengthIn,
                                                                    uint32_t* pLengthOut)
{
    if (NULL == pProvider || NULL == pProvider->pProfile)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == pLengthOut)
        return SOPC_STATUS_INVALID_PARAMETERS;

    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_NOK;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLengthOut = lengthIn;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Decryption(const SOPC_CryptoProvider* pProvider,
                                                                    uint32_t lengthIn,
                                                                    uint32_t* pLengthOut)
{
    if (NULL == pProvider || NULL == pProvider->pProfile)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == pLengthOut)
        return SOPC_STATUS_INVALID_PARAMETERS;

    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_NOK;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLengthOut = lengthIn;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_SignKey(const SOPC_CryptoProvider* pProvider,
                                                                 uint32_t* pLength)
{
    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pLength)
        return SOPC_STATUS_INVALID_PARAMETERS;

    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_NOK;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256Sha256_SymmLen_SignKey;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256_SymmLen_SignKey;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Signature(const SOPC_CryptoProvider* pProvider,
                                                                   uint32_t* pLength)
{
    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pLength)
        return SOPC_STATUS_INVALID_PARAMETERS;

    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_NOK;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Signature;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256_SymmLen_Signature;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_Blocks(const SOPC_CryptoProvider* pProvider,
                                                                uint32_t* pCipherTextBlockSize,
                                                                uint32_t* pPlainTextBlockSize)
{
    if (NULL == pProvider || NULL == pProvider->pProfile)
        return SOPC_STATUS_INVALID_PARAMETERS;

    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if (NULL != pCipherTextBlockSize)
            *pCipherTextBlockSize = SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block;
        if (NULL != pPlainTextBlockSize)
            *pPlainTextBlockSize = SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if (NULL != pCipherTextBlockSize)
            *pCipherTextBlockSize = SOPC_SecurityPolicy_Basic256_SymmLen_Block;
        if (NULL != pPlainTextBlockSize)
            *pPlainTextBlockSize = SOPC_SecurityPolicy_Basic256_SymmLen_Block;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_SymmetricGetLength_SecureChannelNonce(const SOPC_CryptoProvider* pProvider,
                                                                            uint32_t* pLenNonce)
{
    if (NULL == pProvider || NULL == pProvider->pProfile)
        return SOPC_STATUS_INVALID_PARAMETERS;

    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
    case SOPC_SecurityPolicy_Basic256_ID:
        return SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(pProvider, pLenNonce);
    }
}

SOPC_ReturnStatus SOPC_CryptoProvider_DeriveGetLengths(const SOPC_CryptoProvider* pProvider,
                                                       uint32_t* pSymmCryptoKeyLength,
                                                       uint32_t* pSymmSignKeyLength,
                                                       uint32_t* pSymmInitVectorLength)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    if (NULL == pProvider || NULL == pSymmCryptoKeyLength || NULL == pSymmSignKeyLength ||
        NULL == pSymmInitVectorLength)
        return SOPC_STATUS_INVALID_PARAMETERS;

    *pSymmCryptoKeyLength = 0;
    *pSymmSignKeyLength = 0;
    *pSymmInitVectorLength = 0;

    status = SOPC_CryptoProvider_SymmetricGetLength_CryptoKey(pProvider, pSymmCryptoKeyLength);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CryptoProvider_SymmetricGetLength_SignKey(pProvider, pSymmSignKeyLength);
        if (SOPC_STATUS_OK == status)
            status = SOPC_CryptoProvider_SymmetricGetLength_Blocks(pProvider, pSymmInitVectorLength, NULL);
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
                                                                         uint32_t* length)
{
    if (NULL == pProvider || NULL == pProvider->pProfile ||
        SOPC_SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID || NULL == length)
        return SOPC_STATUS_INVALID_PARAMETERS;

    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *length = SOPC_SecurityPolicy_Basic256Sha256_AsymLen_OAEP_Hash;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *length = SOPC_SecurityPolicy_Basic256_AsymLen_OAEP_Hash;
        break;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_PSSHashLength(const SOPC_CryptoProvider* pProvider,
                                                                        uint32_t* length)
{
    if (NULL == pProvider || NULL == pProvider->pProfile ||
        SOPC_SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID || NULL == length)
        return SOPC_STATUS_INVALID_PARAMETERS;

    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *length = SOPC_SecurityPolicy_Basic256Sha256_AsymLen_PSS_Hash;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *length = SOPC_SecurityPolicy_Basic256_AsymLen_PSS_Hash;
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
    if (NULL == pProvider || NULL == pProvider->pProfile ||
        SOPC_SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID)
        return NULL;

    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return NULL;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        return SOPC_SecurityPolicy_Basic256Sha256_URI_SignAlgo;
    case SOPC_SecurityPolicy_Basic256_ID:
        return SOPC_SecurityPolicy_Basic256_URI_SignAlgo;
    }

    return NULL;
}

SOPC_ReturnStatus SOPC_CryptoProvider_CertificateGetLength_Thumbprint(const SOPC_CryptoProvider* pProvider,
                                                                      uint32_t* pLength)
{
    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pLength)
        return SOPC_STATUS_INVALID_PARAMETERS;

    *pLength = 0;
    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_NOK;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256Sha256_CertLen_Thumbprint;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        *pLength = SOPC_SecurityPolicy_Basic256_CertLen_Thumbprint;
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
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ExposedBuffer* pExpKey = NULL;
    SOPC_ExposedBuffer* pExpIV = NULL;
    uint32_t lenCiphered = 0;

    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pIV ||
        NULL == pOutput || NULL == pProvider->pProfile->pFnSymmEncrypt)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (SOPC_CryptoProvider_SymmetricGetLength_Encryption(pProvider, lenPlainText, &lenCiphered) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;
    if (lenCiphered != lenOutput)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // TODO: unit-test these watchdogs
    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if ((lenPlainText % SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block) != 0) // Not block-aligned
            return SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_SecretBuffer_GetLength(pKey) != SOPC_SecurityPolicy_Basic256Sha256_SymmLen_CryptoKey) // Wrong key size
            return SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_SecretBuffer_GetLength(pIV) !=
            SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block) // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if ((lenPlainText % SOPC_SecurityPolicy_Basic256_SymmLen_Block) != 0) // Not block-aligned
            return SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_SecretBuffer_GetLength(pKey) != SOPC_SecurityPolicy_Basic256_SymmLen_CryptoKey) // Wrong key size
            return SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_SecretBuffer_GetLength(pIV) !=
            SOPC_SecurityPolicy_Basic256_SymmLen_Block) // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    pExpKey = SOPC_SecretBuffer_Expose(pKey);
    pExpIV = SOPC_SecretBuffer_Expose(pIV);

    status = pProvider->pProfile->pFnSymmEncrypt(pProvider, pInput, lenPlainText, pExpKey, pExpIV, pOutput, lenOutput);

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
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ExposedBuffer* pExpKey = NULL;
    SOPC_ExposedBuffer* pExpIV = NULL;
    uint32_t lenDeciphered = 0;

    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pIV ||
        NULL == pOutput || NULL == pProvider->pProfile->pFnSymmDecrypt)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (SOPC_CryptoProvider_SymmetricGetLength_Decryption(pProvider, lenCipherText, &lenDeciphered) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;
    if (lenDeciphered != lenOutput)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // TODO: unit-test these watchdogs
    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if ((lenCipherText % SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block) != 0) // Not block-aligned
            return SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_SecretBuffer_GetLength(pKey) != SOPC_SecurityPolicy_Basic256Sha256_SymmLen_CryptoKey) // Wrong key size
            return SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_SecretBuffer_GetLength(pIV) !=
            SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block) // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if ((lenCipherText % SOPC_SecurityPolicy_Basic256_SymmLen_Block) != 0) // Not block-aligned
            return SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_SecretBuffer_GetLength(pKey) != SOPC_SecurityPolicy_Basic256_SymmLen_CryptoKey) // Wrong key size
            return SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_SecretBuffer_GetLength(pIV) !=
            SOPC_SecurityPolicy_Basic256_SymmLen_Block) // Wrong IV size (should be block size)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    pExpKey = SOPC_SecretBuffer_Expose(pKey);
    pExpIV = SOPC_SecretBuffer_Expose(pIV);

    status = pProvider->pProfile->pFnSymmDecrypt(pProvider, pInput, lenCipherText, pExpKey, pExpIV, pOutput, lenOutput);

    SOPC_SecretBuffer_Unexpose(pExpKey, pKey);
    SOPC_SecretBuffer_Unexpose(pExpIV, pIV);

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
    SOPC_ExposedBuffer* pExpKey = NULL;
    uint32_t len;

    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pOutput ||
        NULL == pProvider->pProfile->pFnSymmSign)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Assert output size
    if (SOPC_CryptoProvider_SymmetricGetLength_Signature(pProvider, &len) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;
    if (lenOutput != len)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Assert key size
    if (SOPC_CryptoProvider_SymmetricGetLength_SignKey(pProvider, &len) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;
    if (SOPC_SecretBuffer_GetLength(pKey) != len)
        return SOPC_STATUS_INVALID_PARAMETERS;

    pExpKey = SOPC_SecretBuffer_Expose(pKey);
    if (NULL == pExpKey)
        return SOPC_STATUS_NOK;

    status = pProvider->pProfile->pFnSymmSign(pProvider, pInput, lenInput, pExpKey, pOutput);

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
    SOPC_ExposedBuffer* pExpKey = NULL;
    uint32_t len;

    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pSignature ||
        NULL == pProvider->pProfile->pFnSymmVerif)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Assert output size
    if (SOPC_CryptoProvider_SymmetricGetLength_Signature(pProvider, &len) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;
    if (lenOutput != len)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Assert key size
    if (SOPC_CryptoProvider_SymmetricGetLength_SignKey(pProvider, &len) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;
    if (SOPC_SecretBuffer_GetLength(pKey) != len)
        return SOPC_STATUS_INVALID_PARAMETERS;

    pExpKey = SOPC_SecretBuffer_Expose(pKey);
    if (NULL == pExpKey)
        return SOPC_STATUS_NOK;

    status = pProvider->pProfile->pFnSymmVerif(pProvider, pInput, lenInput, pExpKey, pSignature);

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
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ExposedBuffer* pExp;

    if (NULL == pProvider || nBytes == 0 || NULL == ppBuffer || NULL == pProvider->pProfile ||
        NULL == pProvider->pProfile->pFnGenRnd)
        return SOPC_STATUS_INVALID_PARAMETERS;

    /* Empties pointer in case an error occurs after that point. */
    *ppBuffer = NULL;

    pExp = (SOPC_ExposedBuffer*) malloc(nBytes);
    if (NULL == pExp)
        return SOPC_STATUS_NOK;

    status = pProvider->pProfile->pFnGenRnd(pProvider, pExp, nBytes);
    if (SOPC_STATUS_OK == status)
        *ppBuffer = pExp;

    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_GenerateSecureChannelNonce(const SOPC_CryptoProvider* pProvider,
                                                                 SOPC_SecretBuffer** ppNonce)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t lenNonce;
    SOPC_ExposedBuffer* pExp;

    if (NULL == pProvider || NULL == ppNonce || NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnGenRnd)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (SOPC_CryptoProvider_SymmetricGetLength_SecureChannelNonce(pProvider, &lenNonce) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    status = SOPC_CryptoProvider_GenerateRandomBytes(pProvider, lenNonce, &pExp);
    if (SOPC_STATUS_OK == status)
    {
        *ppNonce = SOPC_SecretBuffer_NewFromExposedBuffer(pExp, lenNonce);
        if (NULL == *ppNonce)
        {
            status = SOPC_STATUS_NOK;
        }

        memset(pExp, 0, lenNonce);
        free(pExp);
    }

    return status;
}

SOPC_ReturnStatus SOPC_CryptoProvider_GenerateRandomID(const SOPC_CryptoProvider* pProvider, uint32_t* pID)
{
    if (NULL == pProvider || NULL == pID || NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnGenRnd)
        return SOPC_STATUS_INVALID_PARAMETERS;

    return pProvider->pProfile->pFnGenRnd(pProvider, (SOPC_ExposedBuffer*) pID, sizeof(uint32_t));
}

SOPC_ReturnStatus SOPC_CryptoProvider_DerivePseudoRandomData(const SOPC_CryptoProvider* pProvider,
                                                             const SOPC_ExposedBuffer* pSecret,
                                                             uint32_t lenSecret,
                                                             const SOPC_ExposedBuffer* pSeed,
                                                             uint32_t lenSeed,
                                                             SOPC_ExposedBuffer* pOutput,
                                                             uint32_t lenOutput)
{
    if (NULL == pProvider || NULL == pProvider->pCryptolibContext || NULL == pProvider->pProfile || NULL == pSecret ||
        0 == lenSecret || NULL == pSeed || 0 == lenSeed || NULL == pOutput || 0 == lenOutput ||
        NULL == pProvider->pProfile->pFnDeriveData)
        return SOPC_STATUS_INVALID_PARAMETERS;

    return pProvider->pProfile->pFnDeriveData(pProvider, pSecret, lenSecret, pSeed, lenSeed, pOutput, lenOutput);
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
    genData = malloc(lenData);
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
    free(genData);

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
    pExpEncr = SOPC_SecretBuffer_Expose(pks->encryptKey);
    pExpSign = SOPC_SecretBuffer_Expose(pks->signKey);
    pExpIV = SOPC_SecretBuffer_Expose(pks->initVector);

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
    SOPC_SecretBuffer_Unexpose(pExpEncr, pks->encryptKey);
    SOPC_SecretBuffer_Unexpose(pExpSign, pks->signKey);
    SOPC_SecretBuffer_Unexpose(pExpIV, pks->initVector);

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
    SOPC_ExposedBuffer* pExpCli = NULL;

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
    SOPC_ExposedBuffer* pExpSer = NULL;

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
                                                        uint32_t lenOutput)
{
    uint32_t lenOutCalc = 0;
    uint32_t lenKey = 0;

    if (NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKey || NULL == pOutput || 0 == lenOutput)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnAsymEncrypt)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Check buffer length
    if (SOPC_CryptoProvider_AsymmetricGetLength_Encryption(pProvider, pKey, lenInput, &lenOutCalc) != SOPC_STATUS_OK)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (lenOutput != lenOutCalc)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Check key length
    if (SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKey, &lenKey) != SOPC_STATUS_OK)
        return SOPC_STATUS_INVALID_PARAMETERS;
    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256_AsymLen_KeyMaxBits)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    // We can now proceed
    return pProvider->pProfile->pFnAsymEncrypt(pProvider, pInput, lenInput, pKey, pOutput);
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricDecrypt(const SOPC_CryptoProvider* pProvider,
                                                        const uint8_t* pInput,
                                                        uint32_t lenInput,
                                                        const SOPC_AsymmetricKey* pKey,
                                                        uint8_t* pOutput,
                                                        uint32_t lenOutput,
                                                        uint32_t* pLenWritten)
{
    uint32_t lenOutCalc = 0;
    uint32_t lenKey = 0;

    if (NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKey || NULL == pOutput || 0 == lenOutput)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnAsymDecrypt)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Check buffer length
    if (SOPC_CryptoProvider_AsymmetricGetLength_Decryption(pProvider, pKey, lenInput, &lenOutCalc) != SOPC_STATUS_OK)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (lenOutput != lenOutCalc)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Check key length
    if (SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKey, &lenKey) != SOPC_STATUS_OK)
        return SOPC_STATUS_INVALID_PARAMETERS;
    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256_AsymLen_KeyMaxBits)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    // We can now proceed
    return pProvider->pProfile->pFnAsymDecrypt(pProvider, pInput, lenInput, pKey, pOutput, pLenWritten);
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
                                                     uint32_t lenSignature)
{
    uint32_t lenSigCalc = 0, lenKey = 0;

    if (NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKeyPrivateLocal || NULL == pSignature ||
        0 == lenSignature)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnAsymSign)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Check lengths
    if (SOPC_CryptoProvider_AsymmetricGetLength_Signature(pProvider, pKeyPrivateLocal, &lenSigCalc) != SOPC_STATUS_OK)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (lenSignature != lenSigCalc)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKeyPrivateLocal, &lenKey) != SOPC_STATUS_OK)
        return SOPC_STATUS_INVALID_PARAMETERS;
    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256_AsymLen_KeyMaxBits)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    return pProvider->pProfile->pFnAsymSign(pProvider, pInput, lenInput, pKeyPrivateLocal, pSignature);
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricVerify(const SOPC_CryptoProvider* pProvider,
                                                       const uint8_t* pInput,
                                                       uint32_t lenInput,
                                                       const SOPC_AsymmetricKey* pKeyRemotePublic,
                                                       const uint8_t* pSignature,
                                                       uint32_t lenSignature)
{
    uint32_t lenSigCalc = 0, lenKey = 0;

    if (NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKeyRemotePublic || NULL == pSignature ||
        0 == lenSignature)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnAsymVerify)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Check lengths
    if (SOPC_CryptoProvider_AsymmetricGetLength_Signature(pProvider, pKeyRemotePublic, &lenSigCalc) != SOPC_STATUS_OK)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (lenSignature != lenSigCalc)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKeyRemotePublic, &lenKey) != SOPC_STATUS_OK)
        return SOPC_STATUS_INVALID_PARAMETERS;
    switch (pProvider->pProfile->SecurityPolicyID)
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    case SOPC_SecurityPolicy_None_ID:
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    case SOPC_SecurityPolicy_Basic256Sha256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    case SOPC_SecurityPolicy_Basic256_ID:
        if (lenKey < SOPC_SecurityPolicy_Basic256_AsymLen_KeyMinBits ||
            lenKey > SOPC_SecurityPolicy_Basic256_AsymLen_KeyMaxBits)
            return SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    return pProvider->pProfile->pFnAsymVerify(pProvider, pInput, lenInput, pKeyRemotePublic, pSignature);
}

/* ------------------------------------------------------------------------------------------------
 * Certificate validation
 * ------------------------------------------------------------------------------------------------
 */
SOPC_ReturnStatus SOPC_CryptoProvider_Certificate_Validate(const SOPC_CryptoProvider* pProvider,
                                                           const SOPC_PKIProvider* pPKI,
                                                           const SOPC_Certificate* pCert)
{
    // TODO: where is the key key_pub <-> key_priv association checked?
    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnCertVerify ||
        NULL == pPKI || NULL == pPKI->pFnValidateCertificate || NULL == pCert)
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Let the lib-specific code handle the verification for the current security policy
    if (pProvider->pProfile->pFnCertVerify(pProvider, pCert) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    // Verify certificate through PKIProvider callback
    return pPKI->pFnValidateCertificate(pPKI, pCert);
}
