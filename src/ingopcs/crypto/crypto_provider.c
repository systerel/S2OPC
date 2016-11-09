/** \file
 *
 *  Copyright (C) 2016 Systerel and others.
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

#include <secret_buffer.h>

#include "sopc_base_types.h"
#include "crypto_types.h"
#include "crypto_provider.h"
#include "crypto_profiles.h"
#include "key_manager.h"
#include "pki.h"


/* ------------------------------------------------------------------------------------------------
 * CryptoProvider creation
 * ------------------------------------------------------------------------------------------------
 */

CryptoProvider *CryptoProvider_Create(const char *uri)
{
    CryptoProvider *pCryptoProvider = NULL;
    const CryptoProfile *pProfile = NULL;

    pProfile = CryptoProfile_Get(uri);
    if(NULL != pProfile)
    {
        pCryptoProvider = (CryptoProvider *)malloc(sizeof(CryptoProvider));
        if(NULL != pCryptoProvider)
        {
            *(const CryptoProfile **)(&pCryptoProvider->pProfile) = pProfile; // TODO: this is a side-effect of putting too much const
            if(STATUS_OK != CryptoProvider_Init(pCryptoProvider))
            {
                free(pCryptoProvider);
                pCryptoProvider = NULL;
            }
        }
    }

    return pCryptoProvider;
}


void CryptoProvider_Free(CryptoProvider* pCryptoProvider)
{
    if(NULL != pCryptoProvider)
    {
        CryptoProvider_Deinit(pCryptoProvider);
        free(pCryptoProvider);
    }
}


/* ------------------------------------------------------------------------------------------------
 * CryptoProvider get-length operations
 * ------------------------------------------------------------------------------------------------
 */

SOPC_StatusCode CryptoProvider_SymmetricGetLength_Key(const CryptoProvider *pProvider,
                                                     uint32_t *pLength)
{
    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pLength)
        return STATUS_INVALID_PARAMETERS;

    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_NOK;
    case SecurityPolicy_Basic256Sha256_ID:
        *pLength = SecurityPolicy_Basic256Sha256_SymmLen_Key;
        break;
    }

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_SymmetricGetLength_Encryption(const CryptoProvider *pProvider,
                                                        uint32_t lengthIn,
                                                        uint32_t *pLengthOut)
{
    (void) pProvider; // Reserved for future use
    if(NULL == pLengthOut)
        return STATUS_INVALID_PARAMETERS;

    *pLengthOut = lengthIn;

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_SymmetricGetLength_Decryption(const CryptoProvider *pProvider,
                                                        uint32_t lengthIn,
                                                        uint32_t *pLengthOut)
{
    (void) pProvider; // Reserved for future use
    if(NULL == pLengthOut)
        return STATUS_INVALID_PARAMETERS;

    *pLengthOut = lengthIn;

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_SymmetricGetLength_Signature(const CryptoProvider *pProvider,
                                                       uint32_t *pLength)
{
    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pLength)
        return STATUS_INVALID_PARAMETERS;

    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_NOK;
    case SecurityPolicy_Basic256Sha256_ID:
        *pLength = SecurityPolicy_Basic256Sha256_SymmLen_Signature;
        break;
    }

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_SymmetricGetLength_Blocks(const CryptoProvider *pProvider,
                                                    uint32_t *pCipherTextBlockSize,
                                                    uint32_t *pPlainTextBlockSize)
{
    if(NULL == pProvider || NULL == pProvider->pProfile)
        return STATUS_INVALID_PARAMETERS;

    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_INVALID_PARAMETERS;
    case SecurityPolicy_Basic256Sha256_ID:
        if(NULL != pCipherTextBlockSize)
            *pCipherTextBlockSize = SecurityPolicy_Basic256Sha256_SymmLen_Block;
        if(NULL != pPlainTextBlockSize)
            *pPlainTextBlockSize = SecurityPolicy_Basic256Sha256_SymmLen_Block;
        break;
    }

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_DeriveGetLengths(const CryptoProvider *pProvider,
                                           uint32_t *pSymmCryptoKeyLength,
                                           uint32_t *pSymmSignKeyLength,
                                           uint32_t *pSymmInitVectorLength)
{
    SOPC_StatusCode status;

    if(NULL == pProvider || NULL == pSymmCryptoKeyLength || NULL == pSymmSignKeyLength || NULL == pSymmInitVectorLength)
        return STATUS_INVALID_PARAMETERS;

    *pSymmCryptoKeyLength = 0;
    *pSymmSignKeyLength = 0;
    *pSymmInitVectorLength = 0;

    status = CryptoProvider_SymmetricGetLength_Key(pProvider, pSymmCryptoKeyLength);
    if(status == STATUS_OK)
    {
        *pSymmSignKeyLength = *pSymmCryptoKeyLength;
        status = CryptoProvider_SymmetricGetLength_Blocks(pProvider, pSymmInitVectorLength, NULL);
    }

    return status;
}


SOPC_StatusCode CryptoProvider_AsymmetricGetLength_KeyBytes(const CryptoProvider *pProvider,
                                                       const AsymmetricKey *pKey,
                                                       uint32_t *pLenKeyBits)
{
    SOPC_StatusCode status = STATUS_OK;

    status = CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKey, pLenKeyBits);
    if(status == STATUS_OK)
        *pLenKeyBits /= 8;

    return status;
}


SOPC_StatusCode CryptoProvider_AsymmetricGetLength_OAEPHashLength(const CryptoProvider *pProvider,
                                                             uint32_t *length)
{
    if(NULL == pProvider || NULL == pProvider->pProfile || SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID || NULL == length)
        return STATUS_INVALID_PARAMETERS;

    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_INVALID_PARAMETERS;
    case SecurityPolicy_Basic256Sha256_ID:
        *length = SecurityPolicy_Basic256Sha256_AsymLen_OAEP_Hash;
        break;
    }

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_AsymmetricGetLength_PSSHashLength(const CryptoProvider *pProvider,
                                                            uint32_t *length)
{
    if(NULL == pProvider || NULL == pProvider->pProfile || SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID || NULL == length)
        return STATUS_INVALID_PARAMETERS;

    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_INVALID_PARAMETERS;
    case SecurityPolicy_Basic256Sha256_ID:
        *length = SecurityPolicy_Basic256Sha256_AsymLen_PSS_Hash;
        break;
    }

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_AsymmetricGetLength_Msgs(const CryptoProvider *pProvider,
                                                   const AsymmetricKey *pKey,
                                                   uint32_t *pCipherTextBlockSize,
                                                   uint32_t *pPlainTextBlockSize)
{
    SOPC_StatusCode statusA = STATUS_OK, statusB = STATUS_OK;

    if(NULL == pProvider || NULL == pKey)
        return STATUS_INVALID_PARAMETERS;

    if(NULL != pCipherTextBlockSize)
    {
        *pCipherTextBlockSize = 0;
        statusA = CryptoProvider_AsymmetricGetLength_MsgCipherText(pProvider, pKey, pCipherTextBlockSize);
    }
    if(NULL != pPlainTextBlockSize)
    {
        *pPlainTextBlockSize = 0;
        statusB = CryptoProvider_AsymmetricGetLength_MsgPlainText(pProvider, pKey, pPlainTextBlockSize);
    }

    if(STATUS_OK != statusA || STATUS_OK != statusB)
        return STATUS_NOK;

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_AsymmetricGetLength_Encryption(const CryptoProvider *pProvider,
                                                         const AsymmetricKey *pKey,
                                                         uint32_t lengthIn,
                                                         uint32_t *pLengthOut)
{
    uint32_t lenCiph = 0, lenPlain = 0;
    uint32_t nMsgs = 0;

    if(NULL == pProvider || NULL == pKey || NULL == pLengthOut)
        return STATUS_INVALID_PARAMETERS;

    if(0 == lengthIn)
    {
        *pLengthOut = 0;
        return STATUS_OK;
    }

    if(STATUS_OK != CryptoProvider_AsymmetricGetLength_Msgs(pProvider, pKey, &lenCiph, &lenPlain))
        return STATUS_NOK;

    // Calculates the number of messages
    nMsgs = lengthIn/lenPlain;
    if((lengthIn%lenPlain) > 0)
        ++nMsgs;

    // Deduces the output length
    *pLengthOut = nMsgs*lenCiph;

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_AsymmetricGetLength_Decryption(const CryptoProvider *pProvider,
                                                         const AsymmetricKey *pKey,
                                                         uint32_t lengthIn,
                                                         uint32_t *pLengthOut)
{
    uint32_t lenCiph = 0, lenPlain = 0;
    uint32_t nMsgs = 0;

    if(NULL == pProvider || NULL == pKey || NULL == pLengthOut)
        return STATUS_INVALID_PARAMETERS;

    if(0 == lengthIn)
    {
        *pLengthOut = 0;
        return STATUS_OK;
    }

    if(STATUS_OK != CryptoProvider_AsymmetricGetLength_Msgs(pProvider, pKey, &lenCiph, &lenPlain))
        return STATUS_NOK;

    // Calculates the number of messages
    nMsgs = lengthIn/lenCiph;
    if((lengthIn%lenCiph) > 0)
        ++nMsgs;

    // Deduces the output length
    *pLengthOut = nMsgs*lenPlain;

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_AsymmetricGetLength_Signature(const CryptoProvider *pProvider,
                                                        const AsymmetricKey *pKey,
                                                        uint32_t *pLength)
{
    if(NULL == pProvider || NULL == pKey || NULL == pLength)
        return STATUS_INVALID_PARAMETERS;

    // The signature is a message long.
    return CryptoProvider_AsymmetricGetLength_Msgs(pProvider, pKey, pLength, NULL);
}


SOPC_StatusCode CryptoProvider_CertificateGetLength_Thumbprint(const CryptoProvider *pProvider,
                                                          uint32_t *pLength)
{
    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pLength)
        return STATUS_INVALID_PARAMETERS;

    *pLength = 0;
    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_NOK;
    case SecurityPolicy_Basic256Sha256_ID:
        *pLength = SecurityPolicy_Basic256Sha256_CertLen_Thumbprint;
        break;
    }

    return STATUS_OK;
}


/* ------------------------------------------------------------------------------------------------
 * Symmetric cryptography
 * ------------------------------------------------------------------------------------------------
 */
SOPC_StatusCode CryptoProvider_SymmetricEncrypt(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenPlainText,
                                           const SecretBuffer *pKey,
                                           const SecretBuffer *pIV,
                                           uint8_t *pOutput,
                                           uint32_t lenOutput)
{
    SOPC_StatusCode status = STATUS_OK;
    ExposedBuffer* pExpKey = NULL;
    ExposedBuffer* pExpIV = NULL;
    uint32_t lenCiphered = 0;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pIV || NULL == pOutput ||
       NULL == pProvider->pProfile->pFnSymmEncrypt)
        return STATUS_INVALID_PARAMETERS;

    if(CryptoProvider_SymmetricGetLength_Encryption(pProvider, lenPlainText, &lenCiphered) != STATUS_OK)
        return STATUS_NOK;
    if(lenCiphered != lenOutput)
        return STATUS_INVALID_PARAMETERS;

    // TODO: unit-test these watchdogs
    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_INVALID_PARAMETERS;
    case SecurityPolicy_Basic256Sha256_ID:
        if((lenPlainText%SecurityPolicy_Basic256Sha256_SymmLen_Block) != 0) // Not block-aligned
            return STATUS_INVALID_PARAMETERS;
        if(SecretBuffer_GetLength(pKey) != SecurityPolicy_Basic256Sha256_SymmLen_Key) // Wrong key size
            return STATUS_INVALID_PARAMETERS;
        if(SecretBuffer_GetLength(pIV) != SecurityPolicy_Basic256Sha256_SymmLen_Block) // Wrong IV size (should be block size)
            return STATUS_INVALID_PARAMETERS;
        break;
    }

    pExpKey = SecretBuffer_Expose(pKey);
    pExpIV = SecretBuffer_Expose(pIV);

    status = pProvider->pProfile->pFnSymmEncrypt(pProvider, pInput, lenPlainText, pExpKey, pExpIV, pOutput, lenOutput);

    SecretBuffer_Unexpose(pExpKey);
    SecretBuffer_Unexpose(pExpIV);

    return status;
}


SOPC_StatusCode CryptoProvider_SymmetricDecrypt(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenCipherText,
                                           const SecretBuffer *pKey,
                                           const SecretBuffer *pIV,
                                           uint8_t *pOutput,
                                           uint32_t lenOutput)
{
    SOPC_StatusCode status = STATUS_OK;
    ExposedBuffer* pExpKey = NULL;
    ExposedBuffer* pExpIV = NULL;
    uint32_t lenDeciphered = 0;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pIV || NULL == pOutput ||
       NULL == pProvider->pProfile->pFnSymmDecrypt)
        return STATUS_INVALID_PARAMETERS;

    if(CryptoProvider_SymmetricGetLength_Decryption(pProvider, lenCipherText, &lenDeciphered) != STATUS_OK)
        return STATUS_NOK;
    if(lenDeciphered != lenOutput)
        return STATUS_INVALID_PARAMETERS;

    // TODO: unit-test these watchdogs
    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_INVALID_PARAMETERS;
    case SecurityPolicy_Basic256Sha256_ID:
        if((lenCipherText%SecurityPolicy_Basic256Sha256_SymmLen_Block) != 0) // Not block-aligned
            return STATUS_INVALID_PARAMETERS;
        if(SecretBuffer_GetLength(pKey) != SecurityPolicy_Basic256Sha256_SymmLen_Key) // Wrong key size
            return STATUS_INVALID_PARAMETERS;
        if(SecretBuffer_GetLength(pIV) != SecurityPolicy_Basic256Sha256_SymmLen_Block) // Wrong IV size (should be block size)
            return STATUS_INVALID_PARAMETERS;
        break;
    }

    pExpKey = SecretBuffer_Expose(pKey);
    pExpIV = SecretBuffer_Expose(pIV);

    status = pProvider->pProfile->pFnSymmDecrypt(pProvider, pInput, lenCipherText, pExpKey, pExpIV, pOutput, lenOutput);

    SecretBuffer_Unexpose(pExpKey);
    SecretBuffer_Unexpose(pExpIV);

    return status;
}


SOPC_StatusCode CryptoProvider_SymmetricSign(const CryptoProvider *pProvider,
                                        const uint8_t *pInput,
                                        uint32_t lenInput,
                                        const SecretBuffer *pKey,
                                        uint8_t *pOutput,
                                        uint32_t lenOutput)
{
    SOPC_StatusCode status = STATUS_OK;
    ExposedBuffer* pExpKey = NULL;
    uint32_t len;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pOutput ||
       NULL == pProvider->pProfile->pFnSymmSign)
        return STATUS_INVALID_PARAMETERS;

    // Assert output size
    if(CryptoProvider_SymmetricGetLength_Signature(pProvider, &len) != STATUS_OK)
        return STATUS_NOK;
    if(lenOutput != len)
        return STATUS_INVALID_PARAMETERS;

    // Assert key size
    if(CryptoProvider_SymmetricGetLength_Key(pProvider, &len) != STATUS_OK)
        return STATUS_NOK;
    if(SecretBuffer_GetLength(pKey) != len)
        return STATUS_INVALID_PARAMETERS;

    pExpKey = SecretBuffer_Expose(pKey);
    if(NULL == pKey)
        return STATUS_NOK;

    status = pProvider->pProfile->pFnSymmSign(pProvider, pInput, lenInput, pExpKey, pOutput);

    SecretBuffer_Unexpose(pExpKey);
    return status;
}


SOPC_StatusCode CryptoProvider_SymmetricVerify(const CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenInput,
                                          const SecretBuffer *pKey,
                                          const uint8_t *pSignature,
                                          uint32_t lenOutput)
{
    SOPC_StatusCode status = STATUS_OK;
    ExposedBuffer* pExpKey = NULL;
    uint32_t len;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pSignature ||
       NULL == pProvider->pProfile->pFnSymmVerif)
        return STATUS_INVALID_PARAMETERS;

    // Assert output size
    if(CryptoProvider_SymmetricGetLength_Signature(pProvider, &len) != STATUS_OK)
        return STATUS_NOK;
    if(lenOutput != len)
        return STATUS_INVALID_PARAMETERS;

    // Assert key size
    if(CryptoProvider_SymmetricGetLength_Key(pProvider, &len) != STATUS_OK)
        return STATUS_NOK;
    if(SecretBuffer_GetLength(pKey) != len)
        return STATUS_INVALID_PARAMETERS;

    pExpKey = SecretBuffer_Expose(pKey);
    if(NULL == pKey)
        return STATUS_NOK;

    status = pProvider->pProfile->pFnSymmVerif(pProvider, pInput, lenInput, pExpKey, pSignature);

    SecretBuffer_Unexpose(pExpKey);
    return status;
}


SOPC_StatusCode CryptoProvider_SymmetricGenerateKey(const CryptoProvider *pProvider,
                                               SecretBuffer **ppKeyGenerated)
{
    SOPC_StatusCode status = STATUS_OK;
    ExposedBuffer *pExpKey;
    uint32_t lenKeyAPI;

    if(NULL == pProvider || NULL == ppKeyGenerated || NULL == pProvider->pProfile->pFnSymmGenKey)
        return STATUS_INVALID_PARAMETERS;

    // Empties pointer in case an error occurs after that point.
    *ppKeyGenerated = NULL;

    if(CryptoProvider_SymmetricGetLength_Key(pProvider, &lenKeyAPI) != STATUS_OK)
        return STATUS_NOK;

    pExpKey = (ExposedBuffer *)malloc(lenKeyAPI);
    if(NULL == pExpKey)
        return STATUS_NOK;

    status = pProvider->pProfile->pFnSymmGenKey(pProvider, pExpKey);
    if(STATUS_OK == status)
    {
        *ppKeyGenerated = SecretBuffer_NewFromExposedBuffer(pExpKey, lenKeyAPI);
        if(NULL == *ppKeyGenerated)
            status = STATUS_NOK;
    }

    memset(pExpKey, 0, lenKeyAPI);
    free(pExpKey);

    return status;
}


SOPC_StatusCode CryptoProvider_DerivePseudoRandomData(const CryptoProvider *pProvider,
                                                 const ExposedBuffer *pSecret,
                                                 uint32_t lenSecret,
                                                 const ExposedBuffer *pSeed,
                                                 uint32_t lenSeed,
                                                 ExposedBuffer *pOutput,
                                                 uint32_t lenOutput)
{
    if(NULL == pProvider || NULL == pProvider->pCryptolibContext || NULL == pProvider->pProfile ||
       NULL == pSecret || 0 == lenSecret || NULL == pSeed || 0 == lenSeed || NULL == pOutput || 0 == lenOutput ||
       NULL == pProvider->pProfile->pFnDeriveData)
        return STATUS_INVALID_PARAMETERS;

    return pProvider->pProfile->pFnDeriveData(pProvider,
                                              pSecret, lenSecret,
                                              pSeed, lenSeed,
                                              pOutput, lenOutput);
}


static inline SOPC_StatusCode DeriveKS(const CryptoProvider *pProvider,
                                  const ExposedBuffer *pSecret, uint32_t lenSecret,
                                  const ExposedBuffer *pSeed, uint32_t lenSeed,
                                  SC_SecurityKeySet *pks,
                                  uint8_t *genData, uint32_t lenData,
                                  uint32_t lenKeySign, uint32_t lenKeyEncr, uint32_t lenIV);
SOPC_StatusCode CryptoProvider_DeriveKeySets(const CryptoProvider *pProvider,
                                        const ExposedBuffer *pClientNonce,
                                        uint32_t lenClientNonce,
                                        const ExposedBuffer *pServerNonce,
                                        uint32_t lenServerNonce,
                                        SC_SecurityKeySet *pClientKeySet,
                                        SC_SecurityKeySet *pServerKeySet)
{
    SOPC_StatusCode status = STATUS_OK;
    uint8_t *genData = NULL;
    uint32_t lenData = 0;
    uint32_t lenKeyEncr = 0, lenKeySign = 0, lenIV = 0;

    // Verify pointers
    if(NULL == pProvider || NULL == pClientNonce || NULL == pServerNonce || NULL == pClientKeySet || NULL == pServerKeySet)
        return STATUS_INVALID_PARAMETERS;

    if(NULL == pClientKeySet->signKey || NULL == pClientKeySet->encryptKey || NULL == pClientKeySet->initVector)
        return STATUS_INVALID_PARAMETERS;

    if(NULL == pServerKeySet->signKey || NULL == pServerKeySet->encryptKey || NULL == pServerKeySet->initVector)
        return STATUS_INVALID_PARAMETERS;

    // Calculate expected lengths
    if(CryptoProvider_DeriveGetLengths(pProvider, &lenKeyEncr, &lenKeySign, &lenIV) != STATUS_OK)
        return STATUS_NOK;

    // Verify lengths
    if(SecretBuffer_GetLength(pClientKeySet->signKey) != lenKeySign ||
       SecretBuffer_GetLength(pClientKeySet->encryptKey) != lenKeyEncr ||
       SecretBuffer_GetLength(pClientKeySet->initVector) != lenIV)
        return STATUS_INVALID_PARAMETERS;

    if(SecretBuffer_GetLength(pServerKeySet->signKey) != lenKeySign ||
       SecretBuffer_GetLength(pServerKeySet->encryptKey) != lenKeyEncr ||
       SecretBuffer_GetLength(pServerKeySet->initVector) != lenIV)
        return STATUS_INVALID_PARAMETERS;

    // Allocate buffer for PRF generated data
    lenData = lenKeySign+lenKeyEncr+lenIV;
    genData = malloc(lenData);
    if(NULL == genData)
        return STATUS_NOK;

    // Derives keyset for the client
    status = DeriveKS(pProvider, pServerNonce, lenServerNonce, pClientNonce, lenClientNonce,
                      pClientKeySet, genData, lenData,
                      lenKeySign, lenKeyEncr, lenIV);
    // Derives keyset for the server
    if(STATUS_OK == status)
        status = DeriveKS(pProvider, pClientNonce, lenClientNonce, pServerNonce, lenServerNonce,
                          pServerKeySet, genData, lenData,
                          lenKeySign, lenKeyEncr, lenIV);

    // Clears and delete
    memset(genData, 0, lenData);
    free(genData);

    return status;
}

static inline SOPC_StatusCode DeriveKS(const CryptoProvider *pProvider,
                                  const ExposedBuffer *pSecret, uint32_t lenSecret,
                                  const ExposedBuffer *pSeed, uint32_t lenSeed,
                                  SC_SecurityKeySet *pks,
                                  uint8_t *genData, uint32_t lenData,
                                  uint32_t lenKeySign, uint32_t lenKeyEncr, uint32_t lenIV)
{
    SOPC_StatusCode status = STATUS_OK;
    ExposedBuffer *pExpEncr = NULL, *pExpSign = NULL, *pExpIV = NULL;

    // Exposes SecretBuffers
    pExpEncr = SecretBuffer_Expose(pks->encryptKey);
    pExpSign = SecretBuffer_Expose(pks->signKey);
    pExpIV = SecretBuffer_Expose(pks->initVector);

    // Verifies exposures
    if(NULL == pExpEncr || NULL == pExpSign || NULL == pExpIV)
        return STATUS_NOK;

    // Generates KeySet
    status = CryptoProvider_DerivePseudoRandomData(pProvider, pSecret, lenSecret, pSeed, lenSeed, genData, lenData);
    if(status == STATUS_OK)
    {
        memcpy(pExpSign, genData, lenKeySign);
        memcpy(pExpEncr, genData+lenKeySign, lenKeyEncr);
        memcpy(pExpIV, genData+lenKeySign+lenKeyEncr, lenIV);
    }

    // Release ExposedBuffers
    SecretBuffer_Unexpose(pExpEncr);
    SecretBuffer_Unexpose(pExpSign);
    SecretBuffer_Unexpose(pExpIV);

    return status;
}


SOPC_StatusCode CryptoProvider_DeriveKeySetsClient(const CryptoProvider *pProvider,
                                              const SecretBuffer *pClientNonce,
                                              const ExposedBuffer *pServerNonce,
                                              uint32_t lenServerNonce,
                                              SC_SecurityKeySet *pClientKeySet,
                                              SC_SecurityKeySet *pServerKeySet)
{
    SOPC_StatusCode status = STATUS_OK;
    ExposedBuffer *pExpCli = NULL;

    if(NULL == pProvider || NULL == pClientNonce || NULL == pServerNonce || NULL == pClientKeySet || NULL == pServerKeySet)
        return STATUS_INVALID_PARAMETERS;

    pExpCli = SecretBuffer_Expose(pClientNonce);
    if(NULL == pExpCli)
        status = STATUS_INVALID_PARAMETERS;

    if(STATUS_OK == status)
    {
        status = CryptoProvider_DeriveKeySets(pProvider,
                                                  pExpCli,
                                                  SecretBuffer_GetLength(pClientNonce),
                                                  pServerNonce,
                                                  lenServerNonce,
                                                  pClientKeySet,
                                                  pServerKeySet);
    }

    SecretBuffer_Unexpose(pExpCli);

    return status;
}


SOPC_StatusCode CryptoProvider_DeriveKeySetsServer(const CryptoProvider *pProvider,
                                              const ExposedBuffer *pClientNonce,
                                              uint32_t lenClientNonce,
                                              const SecretBuffer *pServerNonce,
                                              SC_SecurityKeySet *pClientKeySet,
                                              SC_SecurityKeySet *pServerKeySet)
{
    SOPC_StatusCode status = STATUS_OK;
    ExposedBuffer *pExpSer = NULL;

    if(NULL == pProvider || NULL == pClientNonce || NULL == pServerNonce || NULL == pClientKeySet || NULL == pServerKeySet)
        return STATUS_INVALID_PARAMETERS;

    pExpSer = SecretBuffer_Expose(pServerNonce);
    if(NULL == pExpSer)
        status = STATUS_INVALID_PARAMETERS;

    if(STATUS_OK == status)
    {
        status = CryptoProvider_DeriveKeySets(pProvider,
                                                  pClientNonce,
                                                  lenClientNonce,
                                                  pExpSer,
                                                  SecretBuffer_GetLength(pServerNonce),
                                                  pClientKeySet,
                                                  pServerKeySet);
    }

    SecretBuffer_Unexpose(pExpSer);

    return status;
}


/* ------------------------------------------------------------------------------------------------
 * Asymmetric API
 * ------------------------------------------------------------------------------------------------
 */

SOPC_StatusCode CryptoProvider_AsymmetricEncrypt(const CryptoProvider *pProvider,
                                            const uint8_t *pInput,
                                            uint32_t lenInput,
                                            const AsymmetricKey *pKey,
                                            uint8_t *pOutput,
                                            uint32_t lenOutput)
{
    uint32_t lenOutCalc = 0;
    uint32_t lenKey = 0;

    if(NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKey || NULL == pOutput || 0 == lenOutput)
        return STATUS_INVALID_PARAMETERS;
    if(NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnAsymEncrypt)
        return STATUS_INVALID_PARAMETERS;

    // Check buffer length
    if(CryptoProvider_AsymmetricGetLength_Encryption(pProvider, pKey, lenInput, &lenOutCalc) != STATUS_OK)
        return STATUS_INVALID_PARAMETERS;
    if(lenOutput < lenOutCalc)
        return STATUS_INVALID_PARAMETERS;

    // Check key length
    if(CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKey, &lenKey) != STATUS_OK)
        return STATUS_INVALID_PARAMETERS;
    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_INVALID_PARAMETERS;
    case SecurityPolicy_Basic256Sha256_ID:
        if(lenKey < SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits || lenKey > SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
            return STATUS_INVALID_PARAMETERS;
        break;
    }

    // We can now proceed
    return pProvider->pProfile->pFnAsymEncrypt(pProvider, pInput, lenInput, pKey, pOutput);
}


SOPC_StatusCode CryptoProvider_AsymmetricDecrypt(const CryptoProvider *pProvider,
                                            const uint8_t *pInput,
                                            uint32_t lenInput,
                                            const AsymmetricKey *pKey,
                                            uint8_t *pOutput,
                                            uint32_t lenOutput,
                                            uint32_t *pLenWritten)
{
    uint32_t lenOutCalc = 0;
    uint32_t lenKey = 0;

    if(NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKey || NULL == pOutput || 0 == lenOutput)
        return STATUS_INVALID_PARAMETERS;
    if(NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnAsymDecrypt)
        return STATUS_INVALID_PARAMETERS;

    // Check buffer length
    if(CryptoProvider_AsymmetricGetLength_Decryption(pProvider, pKey, lenInput, &lenOutCalc) != STATUS_OK)
        return STATUS_INVALID_PARAMETERS;
    if(lenOutput < lenOutCalc)
        return STATUS_INVALID_PARAMETERS;

    // Check key length
    if(CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKey, &lenKey) != STATUS_OK)
        return STATUS_INVALID_PARAMETERS;
    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_INVALID_PARAMETERS;
    case SecurityPolicy_Basic256Sha256_ID:
        if(lenKey < SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits || lenKey > SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
            return STATUS_INVALID_PARAMETERS;
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
SOPC_StatusCode CryptoProvider_AsymmetricSign(const CryptoProvider *pProvider,
                                         const uint8_t *pInput,
                                         uint32_t lenInput,
                                         const AsymmetricKey *pKeyPrivateLocal,
                                         uint8_t *pSignature,
                                         uint32_t lenSignature)
{
    uint32_t lenSigCalc = 0, lenKey = 0;

    if(NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKeyPrivateLocal || NULL == pSignature || 0 == lenSignature)
        return STATUS_INVALID_PARAMETERS;
    if(NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnAsymSign)
        return STATUS_INVALID_PARAMETERS;

    // Check lengths
    if(CryptoProvider_AsymmetricGetLength_Signature(pProvider, pKeyPrivateLocal, &lenSigCalc) != STATUS_OK)
        return STATUS_INVALID_PARAMETERS;
    if(lenSignature < lenSigCalc)
        return STATUS_INVALID_PARAMETERS;
    if(CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKeyPrivateLocal, &lenKey) != STATUS_OK)
        return STATUS_INVALID_PARAMETERS;
    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_INVALID_PARAMETERS;
    case SecurityPolicy_Basic256Sha256_ID:
        if(lenKey < SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits || lenKey > SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
            return STATUS_INVALID_PARAMETERS;
        break;
    }

    return pProvider->pProfile->pFnAsymSign(pProvider, pInput, lenInput, pKeyPrivateLocal, pSignature);
}


SOPC_StatusCode CryptoProvider_AsymmetricVerify(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenInput,
                                           const AsymmetricKey *pKeyRemotePublic,
                                           const uint8_t *pSignature,
                                           uint32_t lenSignature)
{
    uint32_t lenSigCalc = 0, lenKey = 0;

    if(NULL == pProvider || NULL == pInput || 0 == lenInput || NULL == pKeyRemotePublic || NULL == pSignature || 0 == lenSignature)
        return STATUS_INVALID_PARAMETERS;
    if(NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnAsymVerify)
        return STATUS_INVALID_PARAMETERS;

    // Check lengths
    if(CryptoProvider_AsymmetricGetLength_Signature(pProvider, pKeyRemotePublic, &lenSigCalc) != STATUS_OK)
        return STATUS_INVALID_PARAMETERS;
    if(lenSignature < lenSigCalc)
        return STATUS_INVALID_PARAMETERS;
    if(CryptoProvider_AsymmetricGetLength_KeyBits(pProvider, pKeyRemotePublic, &lenKey) != STATUS_OK)
        return STATUS_INVALID_PARAMETERS;
    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_INVALID_PARAMETERS;
    case SecurityPolicy_Basic256Sha256_ID:
        if(lenKey < SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits || lenKey > SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
            return STATUS_INVALID_PARAMETERS;
        break;
    }

    return pProvider->pProfile->pFnAsymVerify(pProvider, pInput, lenInput, pKeyRemotePublic, pSignature);
}


/* ------------------------------------------------------------------------------------------------
 * Certificate validation
 * ------------------------------------------------------------------------------------------------
 */
SOPC_StatusCode CryptoProvider_Certificate_Validate(const CryptoProvider *pProvider,
                                              const PKIProvider *pPKI,
                                              const Certificate *pCert)
{
    // TODO: where is the key key_pub <-> key_priv association checked?
    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pProvider->pProfile->pFnCertVerify || NULL == pPKI || NULL == pPKI->pFnValidateCertificate || NULL == pCert)
        return STATUS_INVALID_PARAMETERS;

    // Let the lib-specific code handle the verification for the current security policy
    if(pProvider->pProfile->pFnCertVerify(pProvider, pCert) != STATUS_OK)
        return STATUS_NOK;

    // Verify certificate through PKIProvider callback
    return pPKI->pFnValidateCertificate(pPKI, pCert);
}

