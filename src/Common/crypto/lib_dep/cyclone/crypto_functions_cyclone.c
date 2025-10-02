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

/** \file
 *
 * Gathers the sources of the lib-specific and crypto-related functions.
 *
 * \warning     These functions should only be called through the stack API, as they don't verify
 *              nor sanitize their arguments.
 */

#include "sopc_assert.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_random.h"

#include "crypto_functions_cyclone.h"
#include "crypto_provider_cyclone.h"
#include "key_manager_cyclone.h"

// CycloneCRYPTO internal includes
#include "cipher/aes.h"
#include "cipher_modes/cbc.h"
#include "cipher_modes/ctr.h"
#include "hash/sha1.h"
#include "hash/sha256.h"
#include "mac/hmac.h"

static error_t CyclonePrngRead(void* context, uint8_t* output, size_t length)
{
    if (NULL == context || NULL == output || 0 == length)
    {
        return ERROR_INVALID_PARAMETER;
    }

    // Initialize at state "error"
    error_t errLib = ERROR_FAILURE;

    SOPC_Buffer* buffer_output = SOPC_Buffer_Create((uint32_t) length);
    SOPC_ReturnStatus status = SOPC_GetRandom(buffer_output, (uint32_t) length);
    memcpy(output, buffer_output->data, length);
    SOPC_Buffer_Delete(buffer_output);

    if (SOPC_STATUS_OK == status)
    {
        // Successful processing
        errLib = NO_ERROR;
    }

    return errLib;
}

const PrngAlgo CyclonePrng = {NULL, 0, NULL, NULL, NULL, (PrngAlgoRead) CyclonePrngRead, NULL};

static SOPC_ReturnStatus generic_SymmCrypt(SOPC_SecurityPolicy_ID policyId,
                                           const uint8_t* pInput,
                                           uint32_t lenInput,
                                           const SOPC_ExposedBuffer* pKey,
                                           const SOPC_ExposedBuffer* pIV,
                                           uint8_t* pOutput,
                                           uint32_t lenOutput,
                                           bool isEncrypt)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    const SOPC_SecurityPolicy_Config* policy = SOPC_SecurityPolicy_Config_Get(policyId);
    const uint32_t symmLen_Block = policy->symmLen_Block;
    const uint32_t symmLen_CryptoKey = policy->symmLen_CryptoKey;

    if (lenOutput >= lenInput)
    {
        /* Perform a copy of pIV because pIV is modified during the operation */
        unsigned char* iv_cpy = (unsigned char*) SOPC_Malloc(symmLen_Block);
        SOPC_ASSERT(NULL != iv_cpy);
        memcpy(iv_cpy, pIV, symmLen_Block);

        /* Declare and initialize the AES context */
        AesContext aes = {0};
        error_t errLib = aesInit(&aes, pKey, symmLen_CryptoKey);

        if (errLib == 0)
        {
            if (isEncrypt)
            {
                errLib = cbcEncrypt(&aesCipherAlgo, &aes, (uint8_t*) iv_cpy, pInput, pOutput, lenInput);
            }
            else
            {
                errLib = cbcDecrypt(&aesCipherAlgo, &aes, (uint8_t*) iv_cpy, pInput, pOutput, lenInput);
            }

            if (errLib == 0)
            {
                memset(iv_cpy, 0, symmLen_Block);
                status = SOPC_STATUS_OK;
            }
        }
        aesDeinit(&aes);
        SOPC_Free(iv_cpy);
    }

    return status;
}

/* ------------------------------------------------------------------------------------------------
 * Aes128-Sha256-RsaOaep
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus CryptoProvider_SymmCrypt_AES128(const SOPC_CryptoProvider* pProvider,
                                                  const uint8_t* pInput,
                                                  uint32_t lenInput,
                                                  const SOPC_ExposedBuffer* pKey,
                                                  const SOPC_ExposedBuffer* pIV,
                                                  uint8_t* pOutput,
                                                  uint32_t lenOutput,
                                                  bool isEncrypt)
{
    SOPC_UNUSED_ARG(pProvider);
    if (isEncrypt)
    {
        return generic_SymmCrypt(SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID, pInput, lenInput, pKey, pIV, pOutput,
                                 lenOutput, true);
    }
    else
    {
        return generic_SymmCrypt(SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID, pInput, lenInput, pKey, pIV, pOutput,
                                 lenOutput, false);
    }
}

/* ------------------------------------------------------------------------------------------------
 * Basic256Sha256
 * ------------------------------------------------------------------------------------------------
 */

// TODO: think about the necessity of lenOutput and pInput might be an ExposedBuffer? Clean Symm + Asym
SOPC_ReturnStatus CryptoProvider_SymmCrypt_AES256(const SOPC_CryptoProvider* pProvider,
                                                  const uint8_t* pInput,
                                                  uint32_t lenInput,
                                                  const SOPC_ExposedBuffer* pKey,
                                                  const SOPC_ExposedBuffer* pIV,
                                                  uint8_t* pOutput,
                                                  uint32_t lenOutput,
                                                  bool isEncrypt)
{
    SOPC_UNUSED_ARG(pProvider);
    if (isEncrypt)
    {
        return generic_SymmCrypt(SOPC_SecurityPolicy_Basic256Sha256_ID, pInput, lenInput, pKey, pIV, pOutput, lenOutput,
                                 true);
    }
    else
    {
        return generic_SymmCrypt(SOPC_SecurityPolicy_Basic256Sha256_ID, pInput, lenInput, pKey, pIV, pOutput, lenOutput,
                                 false);
    }
}

static inline SOPC_ReturnStatus HMAC_hashtype_sign(const SOPC_CryptoProvider* pProvider,
                                                   const uint8_t* pInput,
                                                   uint32_t lenInput,
                                                   const SOPC_ExposedBuffer* pKey,
                                                   uint8_t* pOutput,
                                                   const HashAlgo* pHash);

static inline SOPC_ReturnStatus HMAC_hashtype_verify(const SOPC_CryptoProvider* pProvider,
                                                     const uint8_t* pInput,
                                                     uint32_t lenInput,
                                                     const SOPC_ExposedBuffer* pKey,
                                                     const uint8_t* pSignature,
                                                     const HashAlgo* pHash);

static inline SOPC_ReturnStatus HMAC_hashtype_sign(const SOPC_CryptoProvider* pProvider,
                                                   const uint8_t* pInput,
                                                   uint32_t lenInput,
                                                   const SOPC_ExposedBuffer* pKey,
                                                   uint8_t* pOutput,
                                                   const HashAlgo* pHash)
{
    if (NULL == pInput || NULL == pKey || NULL == pOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Get the key length from the provider */
    uint32_t lenKey = 0;
    SOPC_ReturnStatus status = SOPC_CryptoProvider_SymmetricGetLength_SignKey(pProvider, &lenKey);

    if (SOPC_STATUS_OK == status)
    {
        /* Do the HMAC */
        error_t errLib = hmacCompute(pHash, pKey, lenKey, pInput, lenInput, pOutput);
        if (0 != errLib)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

static inline SOPC_ReturnStatus HMAC_hashtype_verify(const SOPC_CryptoProvider* pProvider,
                                                     const uint8_t* pInput,
                                                     uint32_t lenInput,
                                                     const SOPC_ExposedBuffer* pKey,
                                                     const uint8_t* pSignature,
                                                     const HashAlgo* pHash)
{
    if (NULL == pSignature)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t lenSig = 0;
    uint8_t* pCalcSig = NULL;

    /* Get the signature length from the provider */
    SOPC_ReturnStatus status = SOPC_CryptoProvider_SymmetricGetLength_Signature(pProvider, &lenSig);

    if (SOPC_STATUS_OK == status)
    {
        pCalcSig = SOPC_Malloc(lenSig);
        if (NULL == pCalcSig)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Get the HMAC in pCalcSig */
        status = HMAC_hashtype_sign(pProvider, pInput, lenInput, pKey, pCalcSig, pHash);
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Compare pSignature (the original signature) to pCalcSig (the signature we just did) */
        int res = memcmp(pSignature, pCalcSig, lenSig);
        status = 0 != res ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    }

    SOPC_Free(pCalcSig);

    return status;
}

SOPC_ReturnStatus CryptoProvider_SymmSign_HMAC_SHA256(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenInput,
                                                      const SOPC_ExposedBuffer* pKey,
                                                      uint8_t* pOutput)
{
    return HMAC_hashtype_sign(pProvider, pInput, lenInput, pKey, pOutput, &sha256HashAlgo);
}

SOPC_ReturnStatus CryptoProvider_SymmVerify_HMAC_SHA256(const SOPC_CryptoProvider* pProvider,
                                                        const uint8_t* pInput,
                                                        uint32_t lenInput,
                                                        const SOPC_ExposedBuffer* pKey,
                                                        const uint8_t* pSignature)
{
    return HMAC_hashtype_verify(pProvider, pInput, lenInput, pKey, pSignature, &sha256HashAlgo);
}

SOPC_ReturnStatus CryptoProvider_GenTrueRnd(const SOPC_CryptoProvider* pProvider,
                                            SOPC_ExposedBuffer* pData,
                                            uint32_t lenData)
{
    error_t errLib = CyclonePrngRead(pProvider->pCryptolibContext, pData, lenData);
    if (0 == errLib)
    {
        return SOPC_STATUS_OK;
    }

    return SOPC_STATUS_NOK;
}

// PRF with SHA256 as defined in RFC 5246 (TLS v1.2), §5, without label.
// Based on a HMAC with SHA-256.
static inline SOPC_ReturnStatus PSHA_outer(const HashAlgo* pHash,
                                           uint8_t* bufA,
                                           uint32_t lenBufA,
                                           const SOPC_ExposedBuffer* pSecret,
                                           uint32_t lenSecret,
                                           const SOPC_ExposedBuffer* pSeed,
                                           uint32_t lenSeed,
                                           SOPC_ExposedBuffer* pOutput,
                                           uint32_t lenOutput);

static inline SOPC_ReturnStatus PSHA(HmacContext* context,
                                     const HashAlgo* pHash,
                                     uint8_t* bufA,
                                     uint32_t lenBufA,
                                     const SOPC_ExposedBuffer* pSecret,
                                     uint32_t lenSecret,
                                     const SOPC_ExposedBuffer* pSeed,
                                     uint32_t lenSeed,
                                     SOPC_ExposedBuffer* pOutput,
                                     uint32_t lenOutput);

SOPC_ReturnStatus CryptoProvider_DeriveData_PRF_SHA256(const SOPC_CryptoProvider* pProvider,
                                                       const SOPC_ExposedBuffer* pSecret,
                                                       uint32_t lenSecret,
                                                       const SOPC_ExposedBuffer* pSeed,
                                                       uint32_t lenSeed,
                                                       SOPC_ExposedBuffer* pOutput,
                                                       uint32_t lenOutput)
{
    uint8_t* bufA = NULL;
    uint32_t lenBufA = 0; // Stores A(i) + seed except for i = 0
    uint32_t lenHash = 0;

    SOPC_UNUSED_ARG(pProvider);

    if (NULL == pSecret || 0 == lenSecret || NULL == pSeed || 0 == lenSeed || NULL == pOutput || 0 == lenOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const HashAlgo* pHash = &sha256HashAlgo;

    lenHash = (uint32_t) pHash->digestSize;
    lenBufA = lenHash + lenSeed;
    if (lenHash == 0 || lenBufA <= lenSeed) // Test uint overflow
    {
        return SOPC_STATUS_NOK;
    }

    bufA = SOPC_Malloc(lenBufA);
    if (NULL == bufA)
    {
        return SOPC_STATUS_NOK;
    }

    // bufA contains A(i) + seed where + is the concatenation.
    // length(A(i)) and the content of seed do not change, so seed is written only once. The beginning of bufA is
    // initialized later.
    memcpy(bufA + lenHash, pSeed, lenSeed);

    // Next stage generates a context for the PSHA
    SOPC_ReturnStatus status = PSHA_outer(pHash, bufA, lenBufA, pSecret, lenSecret, pSeed, lenSeed, pOutput, lenOutput);

    // Clear and release A
    memset(bufA, 0, lenBufA);
    SOPC_Free(bufA);

    return status;
}

/**
 * We do our own hmacCompute() with the 2 fonctions PSHA_outer() and PSHA().
 * Indeed we can't use hmacCompute() because the algorithm PSHA is too specific.
 */
static inline SOPC_ReturnStatus PSHA_outer(const HashAlgo* pHash,
                                           uint8_t* bufA,
                                           uint32_t lenBufA,
                                           const SOPC_ExposedBuffer* pSecret,
                                           uint32_t lenSecret,
                                           const SOPC_ExposedBuffer* pSeed,
                                           uint32_t lenSeed,
                                           SOPC_ExposedBuffer* pOutput,
                                           uint32_t lenOutput)
{
    HmacContext* context = NULL;

    // Allocate a memory buffer to hold the HMAC context
    context = cryptoAllocMem(sizeof(HmacContext));
    if (NULL == context)
    {
        return SOPC_STATUS_NOK;
    }

    // Effectively does the PSHA with the correctly prepared context
    SOPC_ReturnStatus status =
        PSHA(context, pHash, bufA, lenBufA, pSecret, lenSecret, pSeed, lenSeed, pOutput, lenOutput);

    // Free previously allocated memory
    cryptoFreeMem(context);

    return status;
}

static inline SOPC_ReturnStatus PSHA(HmacContext* context,
                                     const HashAlgo* pHash,
                                     uint8_t* bufA,
                                     uint32_t lenBufA,
                                     const SOPC_ExposedBuffer* pSecret,
                                     uint32_t lenSecret,
                                     const SOPC_ExposedBuffer* pSeed,
                                     uint32_t lenSeed,
                                     SOPC_ExposedBuffer* pOutput,
                                     uint32_t lenOutput)
{
    uint32_t lenHash = 0;
    uint32_t offsetOutput = 0;

    lenHash = (uint32_t) pHash->digestSize;

    /* A(0) is seed, A(1) = HMAC_SHA256(secret, A(0)) */
    error_t errLib = hmacInit(context, pHash, pSecret, lenSecret);
    if (0 != errLib)
    {
        return SOPC_STATUS_NOK;
    }
    hmacUpdate(context, pSeed, lenSeed);
    hmacFinal(context, bufA);

    // Iterates and produces output
    while (offsetOutput < lenOutput)
    {
        // P_SHA256(i) = HMAC_SHA256(secret, A(i+1)+seed)
        errLib = hmacInit(context, pHash, pSecret, lenSecret);
        if (0 != errLib)
        {
            return SOPC_STATUS_NOK;
        }
        hmacUpdate(context, bufA, lenBufA);

        // Did we generate enough data yet?
        if (offsetOutput + lenHash < lenOutput) // Not yet
        {
            hmacFinal(context, pOutput + offsetOutput);
            offsetOutput += lenHash;

            // A(i+2) = HMAC_SHA256(secret, A(i+1))
            errLib = hmacInit(context, pHash, pSecret, lenSecret);
            if (0 != errLib)
            {
                return SOPC_STATUS_NOK;
            }
            hmacUpdate(context, bufA, lenHash);
            hmacFinal(context, bufA);
        }
        // We did generate enough data
        else
        {
            // We can't use pOUtput in hmac_finish anymore, we would overflow pOutput.
            // Copies P_SHA256 to A because we are not using A again afterwards.
            hmacFinal(context, bufA);
            memcpy(pOutput + offsetOutput, bufA, lenOutput - offsetOutput);
            offsetOutput = lenOutput;
        }
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus AsymEncrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                       const uint8_t* pInput,
                                       uint32_t lenPlainText,
                                       const SOPC_AsymmetricKey* pKey,
                                       uint8_t* pOutput,
                                       const HashAlgo* pHash);

SOPC_ReturnStatus AsymDecrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                       const uint8_t* pInput,
                                       uint32_t lenCipherText,
                                       const SOPC_AsymmetricKey* pKey,
                                       uint8_t* pOutput,
                                       uint32_t* pLenWritten,
                                       const HashAlgo* pHash);

SOPC_ReturnStatus AsymEncrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                       const uint8_t* pInput,
                                       uint32_t lenPlainText,
                                       const SOPC_AsymmetricKey* pKey,
                                       uint8_t* pOutput,
                                       const HashAlgo* pHash)
{
    /* TODO: lenWritten should be an array where are written all the lengths */
    size_t lenWritten = 0;

    uint32_t lenMsgPlain = 0;
    uint32_t lenMsgCiph = 0;
    uint32_t lenToCiph = 0;

    error_t errLib = 1;

    /**
     * lenPlainText should be < (n - 2*hash.digestSize - 2), where n is the size of the modulus of the key.
     * Units are bytes.
     **/
    SOPC_ReturnStatus status = SOPC_CryptoProvider_AsymmetricGetLength_Msgs(pProvider, pKey, &lenMsgCiph, &lenMsgPlain);
    if (SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_NOK;
    }

    while (lenPlainText > 0 && SOPC_STATUS_OK == status)
    {
        if (lenPlainText > lenMsgPlain)
        {
            lenToCiph = lenMsgPlain; // A single pass of encrypt takes at most a message
        }
        else
        {
            lenToCiph = lenPlainText;
        }

        errLib = rsaesOaepEncrypt(&CyclonePrng, pProvider->pCryptolibContext, &pKey->pubKey, pHash, NULL, pInput,
                                  (size_t) lenToCiph, pOutput, &lenWritten);

        if (0 != errLib)
        {
            status = SOPC_STATUS_NOK;
            break;
        }

        // Advance pointers
        lenPlainText -= lenToCiph;
        if (0 == lenPlainText)
        {
            break;
        }
        pInput += lenMsgPlain;
        pOutput += lenMsgCiph;
    }

    return status;
}

SOPC_ReturnStatus AsymDecrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                       const uint8_t* pInput,
                                       uint32_t lenCipherText,
                                       const SOPC_AsymmetricKey* pKey,
                                       uint8_t* pOutput,
                                       uint32_t* pLenWritten,
                                       const HashAlgo* pHash)
{
    uint32_t lenMsgPlain = 0, lenMsgCiph = 0;
    size_t lenDeciphed = 0;

    error_t errLib = 1;

    SOPC_ReturnStatus status = SOPC_CryptoProvider_AsymmetricGetLength_Msgs(pProvider, pKey, &lenMsgCiph, &lenMsgPlain);
    if (SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_NOK;
    }

    if (NULL != pLenWritten)
    {
        *pLenWritten = 0;
    }

    while (lenCipherText > 0 && SOPC_STATUS_OK == status)
    {
        errLib = rsaesOaepDecrypt(&pKey->privKey, pHash, NULL, pInput, (size_t) lenMsgCiph, pOutput,
                                  (size_t) lenMsgPlain, &lenDeciphed);

        if (0 != errLib)
        {
            status = SOPC_STATUS_NOK;
            break;
        }

        if (NULL != pLenWritten)
        {
            if (lenDeciphed > UINT32_MAX)
            {
                return SOPC_STATUS_NOK;
            }
            *pLenWritten += (uint32_t) lenDeciphed;
        }

        // Advance pointers
        lenCipherText -= lenMsgCiph;
        if (0 == lenCipherText)
        {
            break;
        }
        pInput += lenMsgCiph;
        pOutput += lenDeciphed;
    }

    return status;
}

SOPC_ReturnStatus CryptoProvider_AsymCrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                                    const uint8_t* pInput,
                                                    uint32_t lenInput,
                                                    const SOPC_AsymmetricKey* pKey,
                                                    uint8_t* pOutput,
                                                    uint32_t* pLenWritten,
                                                    bool isEncrypt)
{
    if (isEncrypt)
    {
        return AsymEncrypt_RSA_OAEP(pProvider, pInput, lenInput, pKey, pOutput, &sha1HashAlgo);
    }
    else
    {
        return AsymDecrypt_RSA_OAEP(pProvider, pInput, lenInput, pKey, pOutput, pLenWritten, &sha1HashAlgo);
    }
}

SOPC_ReturnStatus CryptoProvider_AsymCrypt_RSA_OAEP_SHA256(const SOPC_CryptoProvider* pProvider,
                                                           const uint8_t* pInput,
                                                           uint32_t lenInput,
                                                           const SOPC_AsymmetricKey* pKey,
                                                           uint8_t* pOutput,
                                                           uint32_t* pLenWritten,
                                                           bool isEncrypt)
{
    if (isEncrypt)
    {
        return AsymEncrypt_RSA_OAEP(pProvider, pInput, lenInput, pKey, pOutput, &sha256HashAlgo);
    }
    else
    {
        return AsymDecrypt_RSA_OAEP(pProvider, pInput, lenInput, pKey, pOutput, pLenWritten, &sha256HashAlgo);
    }
}

/**
 * (Internal) Allocates and compute the \p hashAlgo message digest of \p pInput.
 * You must free it after calling this function.
 */
static inline SOPC_ReturnStatus NewMsgDigestBuffer(const uint8_t* pInput,
                                                   uint32_t lenInput,
                                                   const HashAlgo* pHash,
                                                   uint8_t** ppHash)
{
    uint8_t* pHashRes = NULL;
    size_t hLen = pHash->digestSize;

    /* Check params and empty ppHash */
    if (NULL == ppHash)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *ppHash = NULL;

    /* Allocate pHash and put it in ppHash */
    pHashRes = SOPC_Malloc(hLen);
    if (NULL == pHashRes)
    {
        return SOPC_STATUS_NOK;
    }

    *ppHash = pHashRes;

    /* Compute the MD */
    error_t errLib = pHash->compute(pInput, (size_t) lenInput, pHashRes);
    if (0 != errLib)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus AsymSign_RSASSA(const SOPC_CryptoProvider* pProvider,
                                  const uint8_t* pInput,
                                  uint32_t lenInput,
                                  const SOPC_AsymmetricKey* pKey,
                                  uint8_t* pSignature,
                                  const HashAlgo* pHash,
                                  bool pss);

SOPC_ReturnStatus AsymVerify_RSASSA(const SOPC_CryptoProvider* pProvider,
                                    const uint8_t* pInput,
                                    uint32_t lenInput,
                                    const SOPC_AsymmetricKey* pKey,
                                    const uint8_t* pSignature,
                                    const HashAlgo* pHash,
                                    bool pss);

SOPC_ReturnStatus AsymSign_RSASSA(const SOPC_CryptoProvider* pProvider,
                                  const uint8_t* pInput,
                                  uint32_t lenInput,
                                  const SOPC_AsymmetricKey* pKey,
                                  uint8_t* pSignature,
                                  const HashAlgo* pHash,
                                  bool pss)
{
    uint8_t* hash = NULL;

    error_t errLib = 1;

    // signatureLen will contain the length of the resulting signature
    size_t signatureLen = 0;

    SOPC_ReturnStatus status = NewMsgDigestBuffer(pInput, lenInput, pHash, &hash);

    if (SOPC_STATUS_OK == status)
    {
        if (true == pss)
        {
            /**
             * We will take the max size for the saltLen, as we did with Mbedtls.
             * Normally this is the hash length, which is the maximum salt length
             * according to FIPS 185-4 §5.5 (e) and common practice. But the constraint
             * is that the hash length plus the salt length plus 2 bytes must be at most
             * the key length.
             */
            size_t maxSaltLen = 0;
            uint32_t keyLength = 0;
            size_t hLen = pHash->digestSize;

            status = SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes(pProvider, pKey, &keyLength);

            if (SOPC_STATUS_OK != status)
            {
                return SOPC_STATUS_NOK;
            }

            if (keyLength >= hLen + hLen + 2)
            {
                maxSaltLen = hLen;
            }
            else
            {
                maxSaltLen = keyLength - hLen - 2;
            }

            errLib = rsassaPssSign(&CyclonePrng, pProvider->pCryptolibContext, &pKey->privKey, pHash, maxSaltLen, hash,
                                   pSignature, &signatureLen);
        }
        else
        {
            errLib = rsassaPkcs1v15Sign(&pKey->privKey, pHash, hash, pSignature, &signatureLen);
        }

        if (0 != errLib) // signature is as long as the key
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (NULL != hash)
    {
        SOPC_Free(hash);
    }

    return status;
}

SOPC_ReturnStatus AsymVerify_RSASSA(const SOPC_CryptoProvider* pProvider,
                                    const uint8_t* pInput,
                                    uint32_t lenInput,
                                    const SOPC_AsymmetricKey* pKey,
                                    const uint8_t* pSignature,
                                    const HashAlgo* pHash,
                                    bool pss)
{
    uint8_t* hash = NULL;

    error_t errLib = 1;

    SOPC_ReturnStatus status = NewMsgDigestBuffer(pInput, lenInput, pHash, &hash);

    if (SOPC_STATUS_OK == status)
    {
        uint32_t keyLength = 0;

        status = SOPC_CryptoProvider_AsymmetricGetLength_KeyBytes(pProvider, pKey, &keyLength);

        if (SOPC_STATUS_OK != status)
        {
            return SOPC_STATUS_NOK;
        }

        if (true == pss)
        {
            size_t maxSaltLen = 0;
            size_t hLen = pHash->digestSize;
            if (keyLength >= hLen + hLen + 2)
            {
                maxSaltLen = hLen;
            }
            else
            {
                maxSaltLen = keyLength - hLen - 2;
            }

            // Signature length = key length
            errLib = rsassaPssVerify(&pKey->pubKey, pHash, maxSaltLen, hash, pSignature, (size_t) keyLength);
        }
        else
        {
            errLib = rsassaPkcs1v15Verify(&pKey->pubKey, pHash, hash, pSignature, (size_t) keyLength);
        }

        if (0 != errLib)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (NULL != hash)
    {
        SOPC_Free(hash);
    }

    return status;
}

SOPC_ReturnStatus CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA256(const SOPC_CryptoProvider* pProvider,
                                                                    const uint8_t* pInput,
                                                                    uint32_t lenInput,
                                                                    const SOPC_AsymmetricKey* pKey,
                                                                    uint8_t* pSignature)
{
    return AsymSign_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, &sha256HashAlgo, false);
}

SOPC_ReturnStatus CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA256(const SOPC_CryptoProvider* pProvider,
                                                                      const uint8_t* pInput,
                                                                      uint32_t lenInput,
                                                                      const SOPC_AsymmetricKey* pKey,
                                                                      const uint8_t* pSignature)
{
    return AsymVerify_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, &sha256HashAlgo, false);
}

SOPC_ReturnStatus CryptoProvider_AsymSign_RSASSA_PSS(const SOPC_CryptoProvider* pProvider,
                                                     const uint8_t* pInput,
                                                     uint32_t lenInput,
                                                     const SOPC_AsymmetricKey* pKey,
                                                     uint8_t* pSignature)
{
    return AsymSign_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, &sha256HashAlgo, true);
}

SOPC_ReturnStatus CryptoProvider_AsymVerify_RSASSA_PSS(const SOPC_CryptoProvider* pProvider,
                                                       const uint8_t* pInput,
                                                       uint32_t lenInput,
                                                       const SOPC_AsymmetricKey* pKey,
                                                       const uint8_t* pSignature)
{
    return AsymVerify_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, &sha256HashAlgo, true);
}

/* ------------------------------------------------------------------------------------------------
 * Basic256
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus CryptoProvider_SymmSign_HMAC_SHA1(const SOPC_CryptoProvider* pProvider,
                                                    const uint8_t* pInput,
                                                    uint32_t lenInput,
                                                    const SOPC_ExposedBuffer* pKey,
                                                    uint8_t* pOutput)
{
    return HMAC_hashtype_sign(pProvider, pInput, lenInput, pKey, pOutput, &sha1HashAlgo);
}
SOPC_ReturnStatus CryptoProvider_SymmVerify_HMAC_SHA1(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenInput,
                                                      const SOPC_ExposedBuffer* pKey,
                                                      const uint8_t* pSignature)
{
    return HMAC_hashtype_verify(pProvider, pInput, lenInput, pKey, pSignature, &sha1HashAlgo);
}
SOPC_ReturnStatus CryptoProvider_DeriveData_PRF_SHA1(const SOPC_CryptoProvider* pProvider,
                                                     const SOPC_ExposedBuffer* pSecret,
                                                     uint32_t lenSecret,
                                                     const SOPC_ExposedBuffer* pSeed,
                                                     uint32_t lenSeed,
                                                     SOPC_ExposedBuffer* pOutput,
                                                     uint32_t lenOutput)
{
    uint8_t* bufA = NULL;
    uint32_t lenBufA = 0; // Stores A(i) + seed except for i = 0
    uint32_t lenHash = 0;

    SOPC_UNUSED_ARG(pProvider);

    if (NULL == pSecret || 0 == lenSecret || NULL == pSeed || 0 == lenSeed || NULL == pOutput || 0 == lenOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const HashAlgo* pHash = &sha1HashAlgo;

    lenHash = (uint32_t) pHash->digestSize;
    lenBufA = lenHash + lenSeed;
    if (lenHash == 0 || lenBufA <= lenSeed) // Test uint overflow
    {
        return SOPC_STATUS_NOK;
    }

    bufA = SOPC_Malloc(lenBufA);
    if (NULL == bufA)
    {
        return SOPC_STATUS_NOK;
    }

    // bufA contains A(i) + seed where + is the concatenation.
    // length(A(i)) and the content of seed do not change, so seed is written only once. The beginning of bufA is
    // initialized later.
    memcpy(bufA + lenHash, pSeed, lenSeed);

    // Next stage generates a context for the PSHA
    SOPC_ReturnStatus status = PSHA_outer(pHash, bufA, lenBufA, pSecret, lenSecret, pSeed, lenSeed, pOutput, lenOutput);

    // Clear and release A
    memset(bufA, 0, lenBufA);
    SOPC_Free(bufA);

    return status;
}

SOPC_ReturnStatus CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA1(const SOPC_CryptoProvider* pProvider,
                                                                  const uint8_t* pInput,
                                                                  uint32_t lenInput,
                                                                  const SOPC_AsymmetricKey* pKey,
                                                                  uint8_t* pSignature)
{
    return AsymSign_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, &sha1HashAlgo, false);
}

SOPC_ReturnStatus CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA1(const SOPC_CryptoProvider* pProvider,
                                                                    const uint8_t* pInput,
                                                                    uint32_t lenInput,
                                                                    const SOPC_AsymmetricKey* pKey,
                                                                    const uint8_t* pSignature)
{
    return AsymVerify_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, &sha1HashAlgo, false);
}

/* ------------------------------------------------------------------------------------------------
 * PubSub AES-256
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus CryptoProvider_CTR_Crypt_AES256(const SOPC_CryptoProvider* pProvider,
                                                  const uint8_t* pInput,
                                                  uint32_t lenInput,
                                                  const SOPC_ExposedBuffer* pExpKey,
                                                  const SOPC_ExposedBuffer* pExpNonce,
                                                  const SOPC_ExposedBuffer* pRandom,
                                                  uint32_t uSequenceNumber,
                                                  uint8_t* pOutput)
{
    SOPC_UNUSED_ARG(pProvider);
    const SOPC_SecurityPolicy_Config* policy = SOPC_SecurityPolicy_Config_Get(SOPC_SecurityPolicy_PubSub_Aes256_ID);
    const uint32_t keyNonce = policy->symmLen_KeyNonce;
    const uint32_t msgRandom = policy->symmLen_MessageRandom;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    AesContext aes = {0};
    error_t errLib = aesInit(&aes, pExpKey, 32);
    if (0 != errLib)
    {
        status = SOPC_STATUS_NOK;
    }

    /* Build the Nonce Counter */

    /* 4 bytes KeyNonce, 4 bytes MessageRandom, 4 bytes SequenceNumber (Little-endian), 4 for block counter
     * (Big-endian) */
    SOPC_ASSERT(16 == (keyNonce + msgRandom + sizeof(uint32_t) + 4 /* BlockCounter length */) &&
                "Invalid AES-CTR parameters, lengths must add up to 16 bytes block, as per AES specification...");

    uint8_t counter[16] = {0};
    uint8_t* p = counter;
    memcpy(p, pExpNonce, keyNonce);
    p += keyNonce;
    memcpy(p, pRandom, msgRandom);
    p += msgRandom;
    memcpy(p, &uSequenceNumber, sizeof(uint32_t));
    p += sizeof(uint32_t);

    /* BlockCounter, which is big endian is initialized to 1.
     * This is an errata from 1.04 revision of part 14 (see https://mantis.opcfoundation.org/view.php?id=6852) */
    p[0] = 0x00;
    p[1] = 0x00;
    p[2] = 0x00;
    p[3] = 0x01;

    p += 4;
    SOPC_ASSERT(p - counter == 16 && "Invalid pointer arithmetics");

    errLib = ctrEncrypt(&aesCipherAlgo, &aes, 128, counter, pInput, pOutput, (size_t) lenInput);
    if (errLib)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    aesDeinit(&aes);

    return status;
}
