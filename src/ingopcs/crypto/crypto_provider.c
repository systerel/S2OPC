/*
 * Defines the cryptographic providers: structure r/w data alongside a read-only cryptoprofile.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */


#include <stdlib.h>
#include <string.h>

#include <ua_base_types.h>
#include <secret_buffer.h>

#include "crypto_types.h"
#include "crypto_provider.h"
#include "crypto_profiles.h"


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
            if(STATUS_OK != CryptoProvider_LibInit(pCryptoProvider))
            {
                free(pCryptoProvider);
                pCryptoProvider = NULL;
            }
        }
    }

    return pCryptoProvider;
}


void CryptoProvider_Delete(CryptoProvider* pCryptoProvider)
{
    if(NULL != pCryptoProvider)
    {
        CryptoProvider_LibDeinit(pCryptoProvider);
        free(pCryptoProvider);
    }
}


StatusCode CryptoProvider_SymmetricEncrypt(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenPlainText,
                                           const SecretBuffer *pKey,
                                           const SecretBuffer *pIV,
                                           uint8_t *pOutput,
                                           uint32_t lenOutput)
{
    StatusCode status = STATUS_OK;
    ExposedBuffer* pExpKey = NULL;
    ExposedBuffer* pExpIV = NULL;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pIV || NULL == pOutput)
        return STATUS_INVALID_PARAMETERS;
    if(lenPlainText != lenOutput)
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


StatusCode CryptoProvider_SymmetricDecrypt(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenCipherText,
                                           const SecretBuffer *pKey,
                                           const SecretBuffer *pIV,
                                           uint8_t *pOutput,
                                           uint32_t lenOutput)
{
    StatusCode status = STATUS_OK;
    ExposedBuffer* pExpKey = NULL;
    ExposedBuffer* pExpIV = NULL;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pIV || NULL == pOutput)
        return STATUS_INVALID_PARAMETERS;
    if(lenCipherText != lenOutput)
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


StatusCode CryptoProvider_SymmetricGetLength_Key(const CryptoProvider *pProvider,
                                                     uint32_t *length)
{
    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == length)
        return STATUS_INVALID_PARAMETERS;

    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_NOK;
    case SecurityPolicy_Basic256Sha256_ID:
        *length = SecurityPolicy_Basic256Sha256_SymmLen_Key;
        break;
    }

    return STATUS_OK;
}


/*
 * Does not perform padding-alignment
 */
StatusCode CryptoProvider_SymmetricGetLength_Encryption(const CryptoProvider *pProvider,
                                                        uint32_t lengthIn,
                                                        uint32_t *pLengthOut)
{
    (void) pProvider; // Reserved for future use
    if(NULL == pLengthOut)
        return STATUS_INVALID_PARAMETERS;

    *pLengthOut = lengthIn;

    return STATUS_OK;
}


/*
 * Does not perform padding-alignment
 */
StatusCode CryptoProvider_SymmetricGetLength_Decryption(const CryptoProvider *pProvider,
                                                        uint32_t lengthIn,
                                                        uint32_t *pLengthOut)
{
    (void) pProvider; // Reserved for future use
    if(NULL == pLengthOut)
        return STATUS_INVALID_PARAMETERS;

    *pLengthOut = lengthIn;

    return STATUS_OK;
}


StatusCode CryptoProvider_SymmetricGetLength_Signature(const CryptoProvider *pProvider,
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


StatusCode CryptoProvider_SymmetricGetLength_BlockSizes(const CryptoProvider *pProvider,
                                                        uint32_t *cipherTextBlockSize,
                                                        uint32_t *plainTextBlockSize)
{
    if(NULL == pProvider || NULL == pProvider->pProfile)
        return STATUS_INVALID_PARAMETERS;

    switch(pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_INVALID_PARAMETERS;
    case SecurityPolicy_Basic256Sha256_ID:
        if(NULL != cipherTextBlockSize)
            *cipherTextBlockSize = SecurityPolicy_Basic256Sha256_SymmLen_Block;
        if(NULL != plainTextBlockSize)
            *plainTextBlockSize = SecurityPolicy_Basic256Sha256_SymmLen_Block;
        break;
    }

    return STATUS_OK;
}


// pLenOutput can be NULL
StatusCode CryptoProvider_SymmetricSign(const CryptoProvider *pProvider,
                                        const uint8_t *pInput,
                                        uint32_t lenInput,
                                        const SecretBuffer *pKey,
                                        uint8_t *pOutput,
                                        uint32_t lenOutput)
{
    StatusCode status = STATUS_OK;
    ExposedBuffer* pExpKey = NULL;
    uint32_t len;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pOutput)
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


StatusCode CryptoProvider_SymmetricVerify(const CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenInput,
                                          const SecretBuffer *pKey,
                                          const uint8_t *pSignature,
                                          uint32_t lenOutput)
{
    StatusCode status = STATUS_OK;
    ExposedBuffer* pExpKey = NULL;
    uint32_t len;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pSignature)
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


StatusCode CryptoProvider_SymmetricGenerateKey(const CryptoProvider *pProvider,
                                               SecretBuffer **ppKeyGenerated)
{
    StatusCode status = STATUS_OK;
    ExposedBuffer *pExpKey;
    uint32_t lenKeyAPI;

    if(NULL == pProvider || NULL == ppKeyGenerated)
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
