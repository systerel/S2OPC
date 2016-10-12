/*
 * Gathers the sources of the lib-specific and crypto-related functions.
 * Should be split in the future to provide finer grained linking options
 *  (https://www.ingopcs.net/trac/ingopcs.projects/ticket/187).
 *
 *  Created on: Oct 12, 2016
 *      Author: PAB
 */

#include <stdlib.h>
#include <string.h>

#include "ua_base_types.h"
#include "secret_buffer.h"
#include "crypto_profiles.h"
#include "crypto_provider.h"
#include "crypto_provider_lib.h"
#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"


StatusCode CryptoProvider_SymmEncrypt_AES256(const CryptoProvider *pProvider,
                                                    const uint8_t *pInput,
                                                    uint32_t lenPlainText,
                                                    const ExposedBuffer *pKey,
                                                    const ExposedBuffer *pIV,
                                                    uint8_t *pOutput,
                                                    uint32_t lenOutput)
{
    (void) pProvider;
    mbedtls_aes_context aes; // Performance note: a context is initialized each time, as the _setkey operation initialize a new context.
    unsigned char iv_cpy[16]; // Performance note: IV is modified during the operation, so it should be copied first

    if(lenOutput < lenPlainText)
        return STATUS_INVALID_PARAMETERS;

    memcpy(iv_cpy, pIV, 16);

    if(mbedtls_aes_setkey_enc(&aes, (unsigned char *)pKey, 256) != 0)
        return STATUS_INVALID_PARAMETERS;
    if(mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, lenPlainText, iv_cpy, (unsigned char *)pInput, (unsigned char *)pOutput) != 0)
        return STATUS_INVALID_PARAMETERS;

    return STATUS_OK;
}


StatusCode CryptoProvider_SymmDecrypt_AES256(const CryptoProvider *pProvider,
                                       const uint8_t *pInput,
                                       uint32_t lenCipherText,
                                       const ExposedBuffer *pKey,
                                       const ExposedBuffer *pIV,
                                       uint8_t *pOutput,
                                       uint32_t lenOutput)
{
    (void) pProvider;
    mbedtls_aes_context aes; // Performance note: a context is initialized each time, as the _setkey operation initialize a new context.
    unsigned char iv_cpy[16]; // Performance note: IV is modified during the operation, so it should be copied first

    if(lenOutput < lenCipherText)
        return STATUS_INVALID_PARAMETERS;

    memcpy(iv_cpy, pIV, 16);

    if(mbedtls_aes_setkey_dec(&aes, (unsigned char *)pKey, SecurityPolicy_Basic256Sha256_SymmLen_Key*8) != 0)
        return STATUS_INVALID_PARAMETERS;
    if(mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, lenCipherText, iv_cpy, (unsigned char *)pInput, (unsigned char *)pOutput) != 0)
        return STATUS_INVALID_PARAMETERS;

    return STATUS_OK;
}


StatusCode CryptoProvider_SymmSign_HMAC_SHA256(const CryptoProvider *pProvider,
                                                      const uint8_t *pInput,
                                                      uint32_t lenInput,
                                                      const ExposedBuffer *pKey,
                                                      uint8_t *pOutput)
{
    const mbedtls_md_info_t *pinfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    uint32_t lenKey;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pOutput)
        return STATUS_INVALID_PARAMETERS;

    if(CryptoProvider_SymmetricGetLength_Key(pProvider, &lenKey) != STATUS_OK)
        return STATUS_NOK;

    if(mbedtls_md_hmac(pinfo, pKey, lenKey, pInput, lenInput, pOutput) != 0)
        return STATUS_NOK;

    return STATUS_OK;
}


StatusCode CryptoProvider_SymmVerify_HMAC_SHA256(const CryptoProvider *pProvider,
                                                        const uint8_t *pInput,
                                                        uint32_t lenInput,
                                                        const ExposedBuffer *pKey,
                                                        const uint8_t *pSignature)
{
    const mbedtls_md_info_t *pinfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    uint32_t lenKey, lenSig;
    uint8_t *pCalcSig;
    StatusCode status = STATUS_OK;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pSignature)
        return STATUS_INVALID_PARAMETERS;

    if(CryptoProvider_SymmetricGetLength_Key(pProvider, &lenKey) != STATUS_OK)
        return STATUS_NOK;

    if(CryptoProvider_SymmetricGetLength_Signature(pProvider, &lenSig) != STATUS_OK)
        return STATUS_NOK;

    pCalcSig = malloc(lenSig);
    if(NULL == pCalcSig)
        return STATUS_NOK;

    status = mbedtls_md_hmac(pinfo, pKey, lenKey, pInput, lenInput, pCalcSig) != 0 ? STATUS_NOK : STATUS_OK;
    if(STATUS_OK == status)
        status = memcmp(pSignature, pCalcSig, lenSig) != 0 ? STATUS_NOK : STATUS_OK;

    free(pCalcSig);

    return status;
}


// Fills pKey with SecurityPolicy_Basic256Sha256_Symm_KeyLength bytes of random data
StatusCode CryptoProvider_SymmGenKey_AES256(const CryptoProvider *pProvider,
                                            ExposedBuffer *pKey)
{
    CryptolibContext *pCtx = NULL;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pProvider->pCryptolibContext || NULL == pKey)
        return STATUS_INVALID_PARAMETERS;

    pCtx = pProvider->pCryptolibContext;
    if(mbedtls_ctr_drbg_random(&(pCtx->ctxDrbg), pKey, SecurityPolicy_Basic256Sha256_SymmLen_Key) != 0)
        return STATUS_NOK;

    return STATUS_OK;
}


// PRF with SHA256 as defined in RFC 5246 (TLS v1.2), §5, without label.
// Based on a HMAC with SHA-256.
StatusCode CryptoProvider_DeriveData_PRF_SHA256(const CryptoProvider *pProvider,
                                                const ExposedBuffer *pSecret,
                                                uint32_t lenSecret,
                                                const ExposedBuffer *pSeed,
                                                uint32_t lenSeed,
                                                ExposedBuffer *pOutput,
                                                uint32_t lenOutput)
{
    StatusCode status = STATUS_OK;
    mbedtls_md_context_t md_ctx;
    uint8_t *bufA = NULL;
    uint32_t lenBufA = 0; // Stores A(i) + seed except for i = 0
    uint32_t lenHash = 0;
    uint32_t offsetOutput = 0;
    //uint32_t i = 0;

    if(NULL == pProvider || NULL == pProvider->pCryptolibContext || NULL == pProvider->pProfile ||
       NULL == pSecret || 0 == lenSecret || NULL == pSeed || 0 == lenSeed || NULL == pOutput || 0 == lenOutput)
        return STATUS_INVALID_PARAMETERS;

    const mbedtls_md_info_t *pmd_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    if(NULL == pmd_info)
        return STATUS_NOK;

    lenHash = mbedtls_md_get_size(pmd_info);
    lenBufA = lenHash + lenSeed;
    if(lenHash == 0 || lenBufA <= lenSeed) // Test uint overflow
        return STATUS_NOK;

    bufA = malloc(lenBufA);
    if(NULL == bufA)
        return STATUS_NOK;
    // Hash has a constant length, so the seed is concatenated only once. The beginning of bufA is initialized later.
    memcpy(bufA + lenHash, pSeed, lenSeed);

    // Prepares context for HMAC operations
    mbedtls_md_init(&md_ctx);
    status = mbedtls_md_setup(&md_ctx, pmd_info, 1) == 0 ? STATUS_OK : STATUS_NOK;

    if(STATUS_OK == status)
    {
        // A(0) is seed, A(1) = HMAC_SHA256(secret, A(0))
        status = mbedtls_md_hmac_starts(&md_ctx, pSecret, lenSecret) == 0 ? STATUS_OK : STATUS_NOK;
        if(STATUS_OK == status)
            status = mbedtls_md_hmac_update(&md_ctx, pSeed, lenSeed) == 0 ? STATUS_OK : STATUS_NOK;
        if(STATUS_OK == status)
            status = mbedtls_md_hmac_finish(&md_ctx, bufA) == 0 ? STATUS_OK : STATUS_NOK;

        // Iterates and produces output
        while(offsetOutput < lenOutput && STATUS_OK == status)
        {
            // P_SHA256(i) = HMAC_SHA256(secret, A(i+1)+seed)
            if(STATUS_OK == status)
                status = mbedtls_md_hmac_reset(&md_ctx) == 0 ? STATUS_OK : STATUS_NOK;
            if(STATUS_OK == status)
                status = mbedtls_md_hmac_update(&md_ctx, bufA, lenBufA) == 0 ? STATUS_OK : STATUS_NOK;
            // We did not generate enough data yet
            if(offsetOutput + lenHash < lenOutput)
            {
                if(STATUS_OK == status)
                    status = mbedtls_md_hmac_finish(&md_ctx, &pOutput[offsetOutput]) == 0 ? STATUS_OK : STATUS_NOK;
                offsetOutput += lenHash;

                // A(i+2) = HMAC_SHA256(secret, A(i+1))
                if(STATUS_OK == status)
                    status = mbedtls_md_hmac_reset(&md_ctx) == 0 ? STATUS_OK : STATUS_NOK;
                if(STATUS_OK == status)
                    status = mbedtls_md_hmac_update(&md_ctx, bufA, lenHash) == 0 ? STATUS_OK : STATUS_NOK;
                if(STATUS_OK == status)
                    status = mbedtls_md_hmac_finish(&md_ctx, bufA) == 0 ? STATUS_OK : STATUS_NOK;
            }
            // We did generate enough data
            else
            {
                // Copies P_SHA256 to A because we are not using A again afterwards
                if(STATUS_OK == status)
                    status = mbedtls_md_hmac_finish(&md_ctx, bufA) == 0 ? STATUS_OK : STATUS_NOK;
                memcpy(&pOutput[offsetOutput], bufA, lenOutput-offsetOutput);
                offsetOutput = lenOutput;
            }
        }
        // Free the context
        mbedtls_md_free(&md_ctx);
    }

    // Clear and release A
    memset(bufA, 0, lenBufA);
    free(bufA);

    return STATUS_OK;
}

