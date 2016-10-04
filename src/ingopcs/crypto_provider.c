/*
 * Defines the cryptographic providers: structure r/w data alongside a read-only cryptoprofile.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */


#include <stdlib.h>
#include <string.h>

//#include "ua_builtintypes.h"
#include "crypto_provider.h"
#include "crypto_profiles.h"


/* ------------------------------------------------------------------------------------------------
 * SecretBuffer (mangled key) and ExposedKeyBuffer (contiguous deciphered buffered) APIs.
 * TODO: move these decl secret_buffer.h ?
 * ------------------------------------------------------------------------------------------------
 */

/* Creates a new SecretBuffer from an exposed buffer and a length. Copies the buffer, so ExposedBuffer
 *  can be cleared and de-allocated.
 * You must call SecretBuffer_DeleteClear() for each call to _New
 */
SecretBuffer *SecretBuffer_NewFromExposedBuffer(ExposedBuffer *buf, uint32_t len)
{
    SecretBuffer *sec = UA_NULL;

    if(UA_NULL != buf && 0 != len)
    {
        sec = malloc(sizeof(SecretBuffer));
        if(UA_NULL != sec)
        {
            sec->len = len;
            sec->buf = malloc(len);
            if(UA_NULL != sec->buf)
                memcpy(sec->buf, buf, len);
            else
            {
                free(sec->buf);
                sec = UA_NULL;
            }
        }
    }

    return sec;
}
void SecretBuffer_DeleteClear(SecretBuffer *sec)
{
    if(UA_NULL != sec)
    {
        if(sec->buf)
        {
            memset(sec->buf, 0, sec->len);
            free(sec->buf);
        }
        free(sec);
    }
}

uint32_t SecretBuffer_GetLength(SecretBuffer *sec)
{
    if(UA_NULL == sec)
        return 0;
    return sec->len;
}

/* Exposes the secret buffer to a contiguous buffer.
 * Each call to Expose should be followed by a call to Unexpose.
 * SecretBuffer should not be destroyed before the call to Unexpose.
 */
ExposedBuffer *SecretBuffer_Expose(SecretBuffer *sec)
{
    if(UA_NULL != sec)
        return sec->buf;
    return UA_NULL;
}

void SecretBuffer_Unexpose(ExposedBuffer *buf)
{
    (void) buf;
}

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider
 * ------------------------------------------------------------------------------------------------
 */
CryptoProvider *CryptoProvider_Create(const uint8_t *uri)
{
    CryptoProvider *pCryptoProvider = UA_NULL;
    const CryptoProfile *pProfile = UA_NULL;

    pProfile = CryptoProfile_Get(uri);
    if(UA_NULL != pProfile)
    {
        pCryptoProvider = (CryptoProvider *)malloc(sizeof(CryptoProvider));
        if(UA_NULL != pCryptoProvider)
        {
            *(const CryptoProfile **)(&pCryptoProvider->pProfile) = pProfile; // TODO: this is a side-effect of putting too much const
            if(STATUS_OK != CryptoProvider_LibInit(pCryptoProvider))
            {
                free(pCryptoProvider);
                pCryptoProvider = UA_NULL;
            }
        }
    }

    return pCryptoProvider;
}


void CryptoProvider_Delete(CryptoProvider* pCryptoProvider)
{
    if(UA_NULL != pCryptoProvider)
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
    ExposedBuffer* pExpKey = UA_NULL;
    ExposedBuffer* pExpIV = UA_NULL;

    if(UA_NULL == pProvider || UA_NULL == pProvider->pProfile || UA_NULL == pInput || UA_NULL == pKey || UA_NULL == pIV || UA_NULL == pOutput)
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
    ExposedBuffer* pExpKey = UA_NULL;
    ExposedBuffer* pExpIV = UA_NULL;

    if(UA_NULL == pProvider || UA_NULL == pProvider->pProfile || UA_NULL == pInput || UA_NULL == pKey || UA_NULL == pIV || UA_NULL == pOutput)
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
    if(UA_NULL == pProvider || UA_NULL == pProvider->pProfile || UA_NULL == length)
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
    if(UA_NULL == pLengthOut)
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
    if(UA_NULL == pLengthOut)
        return STATUS_INVALID_PARAMETERS;

    *pLengthOut = lengthIn;

    return STATUS_OK;
}


StatusCode CryptoProvider_SymmetricGetLength_Signature(const CryptoProvider *pProvider,
                                                       uint32_t *pLength)
{
    if(UA_NULL == pProvider || UA_NULL == pProvider->pProfile || UA_NULL == pLength)
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


// pLenOutput can be UA_NULL
StatusCode CryptoProvider_SymmetricSign(const CryptoProvider *pProvider,
                                        const uint8_t *pInput,
                                        uint32_t lenInput,
                                        const SecretBuffer *pKey,
                                        uint8_t *pOutput,
                                        uint32_t lenOutput)
{
    StatusCode status = STATUS_OK;
    ExposedBuffer* pExpKey = UA_NULL;
    uint32_t len;

    if(UA_NULL == pProvider || UA_NULL == pProvider->pProfile || UA_NULL == pInput || UA_NULL == pKey || UA_NULL == pOutput)
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
    if(UA_NULL == pKey)
        return STATUS_NOK;

    status = pProvider->pProfile->pFnSymmSign(pProvider, pInput, lenInput, pExpKey, pOutput);

    SecretBuffer_Unexpose(pExpKey);
    return status;
}


StatusCode CryptoProvider_SymmetricVerify_Low(const CryptoProvider *pProvider,
                                              const uint8_t *pInput,
                                              uint32_t lenInput,
                                              const SecretBuffer *pKey,
                                              const uint8_t *pSignature,
                                              uint32_t lenOutput)
{
    StatusCode status = STATUS_OK;
    ExposedBuffer* pExpKey = UA_NULL;
    uint32_t len;

    if(UA_NULL == pProvider || UA_NULL == pProvider->pProfile || UA_NULL == pInput || UA_NULL == pKey || UA_NULL == pSignature)
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
    if(UA_NULL == pKey)
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

    if(UA_NULL == pProvider || UA_NULL == ppKeyGenerated)
        return STATUS_INVALID_PARAMETERS;

    // Empties pointer in case an error occurs after that point.
    *ppKeyGenerated = UA_NULL;

    if(CryptoProvider_SymmetricGetLength_Key(pProvider, &lenKeyAPI) != STATUS_OK)
        return STATUS_NOK;

    pExpKey = (ExposedBuffer *)malloc(lenKeyAPI);
    if(UA_NULL == pExpKey)
        return STATUS_NOK;

    status = pProvider->pProfile->pFnSymmGenKey(pProvider, pExpKey);
    if(STATUS_OK == status)
    {
        *ppKeyGenerated = SecretBuffer_NewFromExposedBuffer(pExpKey, lenKeyAPI);
        if(UA_NULL == *ppKeyGenerated)
            status = STATUS_NOK;
    }

    free(pExpKey);

    return status;
}
