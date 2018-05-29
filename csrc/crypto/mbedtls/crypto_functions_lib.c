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

/** \file
 *
 * Gathers the sources of the lib-specific and crypto-related functions.
 * Should be split in the future to provide finer grained linking options
 *  (https://www.ingopcs.net/trac/ingopcs.projects/ticket/187).
 *
 * \warning     These functions should only be called through the stack API, as they don't verify
 *              nor sanitize their arguments.
 */

#include <stdlib.h>
#include <string.h>

#include "../sopc_crypto_profiles.h"
#include "../sopc_crypto_provider.h"
#include "../sopc_secret_buffer.h"
#include "crypto_provider_lib.h"
#include "key_manager_lib.h"

#include "mbedtls/aes.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/md.h"
#include "mbedtls/rsa.h"

/* ------------------------------------------------------------------------------------------------
 * Basic256Sha256
 * ------------------------------------------------------------------------------------------------
 */

// TODO: think about the necessity of lenOutput and pInput might be an ExposedBuffer? Clean Symm + Asym
SOPC_ReturnStatus CryptoProvider_SymmEncrypt_AES256(const SOPC_CryptoProvider* pProvider,
                                                    const uint8_t* pInput,
                                                    uint32_t lenPlainText,
                                                    const SOPC_ExposedBuffer* pKey,
                                                    const SOPC_ExposedBuffer* pIV,
                                                    uint8_t* pOutput,
                                                    uint32_t lenOutput)
{
    mbedtls_aes_context aes; // Performance note: a context is initialized each time, as the _setkey operation
                             // initialize a new context.
    unsigned char iv_cpy[SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block]; // IV is modified during the operation, so
                                                                            // it must be copied first

    (void) pProvider;

    if (lenOutput < lenPlainText) // TODO: we are in our own lib, arguments have already been verified.
        return SOPC_STATUS_INVALID_PARAMETERS;

    memcpy(iv_cpy, pIV, SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block);

    if (mbedtls_aes_setkey_enc(&aes, (const unsigned char*) pKey, 256) != 0)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, lenPlainText, iv_cpy, (const unsigned char*) pInput,
                              (unsigned char*) pOutput) != 0)
        return SOPC_STATUS_INVALID_PARAMETERS;

    memset(iv_cpy, 0, SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block);
    mbedtls_aes_free(&aes);

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus CryptoProvider_SymmDecrypt_AES256(const SOPC_CryptoProvider* pProvider,
                                                    const uint8_t* pInput,
                                                    uint32_t lenCipherText,
                                                    const SOPC_ExposedBuffer* pKey,
                                                    const SOPC_ExposedBuffer* pIV,
                                                    uint8_t* pOutput,
                                                    uint32_t lenOutput)
{
    mbedtls_aes_context aes; // Performance note: a context is initialized each time, as the _setkey operation
                             // initialize a new context.
    unsigned char iv_cpy[SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block]; // IV is modified during the operation, so
                                                                            // it must be copied first

    (void) pProvider;

    if (lenOutput < lenCipherText)
        return SOPC_STATUS_INVALID_PARAMETERS;

    memcpy(iv_cpy, pIV, SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block);
    mbedtls_aes_init(&aes);

    if (mbedtls_aes_setkey_dec(&aes, (const unsigned char*) pKey,
                               SOPC_SecurityPolicy_Basic256Sha256_SymmLen_CryptoKey * 8) != 0)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, lenCipherText, iv_cpy, (const unsigned char*) pInput,
                              (unsigned char*) pOutput) != 0)
        return SOPC_STATUS_INVALID_PARAMETERS;

    memset(iv_cpy, 0, SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block);
    mbedtls_aes_free(&aes);

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus CryptoProvider_SymmSign_HMAC_SHA256(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenInput,
                                                      const SOPC_ExposedBuffer* pKey,
                                                      uint8_t* pOutput)
{
    uint32_t lenKey;

    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pOutput)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (SOPC_CryptoProvider_SymmetricGetLength_SignKey(pProvider, &lenKey) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    const mbedtls_md_info_t* pinfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (mbedtls_md_hmac(pinfo, pKey, lenKey, pInput, lenInput, pOutput) != 0)
        return SOPC_STATUS_NOK;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus CryptoProvider_SymmVerify_HMAC_SHA256(const SOPC_CryptoProvider* pProvider,
                                                        const uint8_t* pInput,
                                                        uint32_t lenInput,
                                                        const SOPC_ExposedBuffer* pKey,
                                                        const uint8_t* pSignature)
{
    uint32_t lenKey, lenSig;
    uint8_t* pCalcSig;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pSignature)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (SOPC_CryptoProvider_SymmetricGetLength_SignKey(pProvider, &lenKey) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    if (SOPC_CryptoProvider_SymmetricGetLength_Signature(pProvider, &lenSig) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    pCalcSig = malloc(lenSig);
    if (NULL == pCalcSig)
        return SOPC_STATUS_NOK;

    const mbedtls_md_info_t* pinfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    status = mbedtls_md_hmac(pinfo, pKey, lenKey, pInput, lenInput, pCalcSig) != 0 ? SOPC_STATUS_NOK : SOPC_STATUS_OK;

    if (SOPC_STATUS_OK == status)
        status = memcmp(pSignature, pCalcSig, lenSig) != 0 ? SOPC_STATUS_NOK : SOPC_STATUS_OK;

    free(pCalcSig);

    return status;
}

// Fills a buffer with "truly" random data
SOPC_ReturnStatus CryptoProvider_GenTrueRnd(const SOPC_CryptoProvider* pProvider,
                                            SOPC_ExposedBuffer* pData,
                                            uint32_t lenData)
{
    SOPC_CryptolibContext* pCtx = NULL;

    pCtx = pProvider->pCryptolibContext;
    if (mbedtls_ctr_drbg_random(&(pCtx->ctxDrbg), pData, lenData) != 0)
        return SOPC_STATUS_NOK;

    return SOPC_STATUS_OK;
}

// PRF with SHA256 as defined in RFC 5246 (TLS v1.2), §5, without label.
// Based on a HMAC with SHA-256.
static inline SOPC_ReturnStatus PSHA_outer(const mbedtls_md_info_t* pmd_info,
                                           uint8_t* bufA,
                                           uint32_t lenBufA,
                                           const SOPC_ExposedBuffer* pSecret,
                                           uint32_t lenSecret,
                                           const SOPC_ExposedBuffer* pSeed,
                                           uint32_t lenSeed,
                                           SOPC_ExposedBuffer* pOutput,
                                           uint32_t lenOutput);
static inline SOPC_ReturnStatus PSHA(mbedtls_md_context_t* pmd,
                                     const mbedtls_md_info_t* pmd_info,
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
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint8_t* bufA = NULL;
    uint32_t lenBufA = 0; // Stores A(i) + seed except for i = 0
    uint32_t lenHash = 0;

    (void) (pProvider);

    if (NULL == pSecret || 0 == lenSecret || NULL == pSeed || 0 == lenSeed || NULL == pOutput || 0 == lenOutput)
        return SOPC_STATUS_INVALID_PARAMETERS;

    const mbedtls_md_info_t* pmd_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    if (NULL == pmd_info)
        return SOPC_STATUS_NOK;

    lenHash = mbedtls_md_get_size(pmd_info);
    lenBufA = lenHash + lenSeed;
    if (lenHash == 0 || lenBufA <= lenSeed) // Test uint overflow
        return SOPC_STATUS_NOK;

    bufA = malloc(lenBufA);
    if (NULL == bufA)
        return SOPC_STATUS_NOK;

    // bufA contains A(i) + seed where + is the concatenation.
    // length(A(i)) and the content of seed do not change, so seed is written only once. The beginning of bufA is
    // initialized later.
    memcpy(bufA + lenHash, pSeed, lenSeed);

    // Next stage generates a context for the PSHA
    status = PSHA_outer(pmd_info, bufA, lenBufA, pSecret, lenSecret, pSeed, lenSeed, pOutput, lenOutput);

    // Clear and release A
    memset(bufA, 0, lenBufA);
    free(bufA);

    return status;
}

static inline SOPC_ReturnStatus PSHA_outer(const mbedtls_md_info_t* pmd_info,
                                           uint8_t* bufA,
                                           uint32_t lenBufA,
                                           const SOPC_ExposedBuffer* pSecret,
                                           uint32_t lenSecret,
                                           const SOPC_ExposedBuffer* pSeed,
                                           uint32_t lenSeed,
                                           SOPC_ExposedBuffer* pOutput,
                                           uint32_t lenOutput)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    mbedtls_md_context_t md_ctx;

    // Prepares context for HMAC operations
    mbedtls_md_init(&md_ctx);
    if (mbedtls_md_setup(&md_ctx, pmd_info, 1) != 0)
        return SOPC_STATUS_NOK;

    // Effectively does the PSHA with the correctly prepared context
    status = PSHA(&md_ctx, pmd_info, bufA, lenBufA, pSecret, lenSecret, pSeed, lenSeed, pOutput, lenOutput);

    // Free the context
    mbedtls_md_free(&md_ctx);

    return status;
}

static inline SOPC_ReturnStatus PSHA(mbedtls_md_context_t* pmd,
                                     const mbedtls_md_info_t* pmd_info,
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

    lenHash = mbedtls_md_get_size(pmd_info); // This has already been verified, and works fine.

    // A(0) is seed, A(1) = HMAC_SHA256(secret, A(0))
    if (mbedtls_md_hmac_starts(pmd, pSecret, lenSecret) != 0)
        return SOPC_STATUS_NOK;
    if (mbedtls_md_hmac_update(pmd, pSeed, lenSeed) != 0)
        return SOPC_STATUS_NOK;
    if (mbedtls_md_hmac_finish(pmd, bufA) != 0)
        return SOPC_STATUS_NOK;

    // Iterates and produces output
    while (offsetOutput < lenOutput)
    {
        // P_SHA256(i) = HMAC_SHA256(secret, A(i+1)+seed)
        if (mbedtls_md_hmac_reset(pmd) != 0)
            return SOPC_STATUS_NOK;
        if (mbedtls_md_hmac_update(pmd, bufA, lenBufA) != 0)
            return SOPC_STATUS_NOK;

        // Did we generate enough data yet?
        if (offsetOutput + lenHash < lenOutput) // Not yet
        {
            if (mbedtls_md_hmac_finish(pmd, pOutput + offsetOutput) != 0)
                return SOPC_STATUS_NOK;
            offsetOutput += lenHash;

            // A(i+2) = HMAC_SHA256(secret, A(i+1))
            if (mbedtls_md_hmac_reset(pmd) != 0)
                return SOPC_STATUS_NOK;
            if (mbedtls_md_hmac_update(pmd, bufA, lenHash) != 0)
                return SOPC_STATUS_NOK;
            if (mbedtls_md_hmac_finish(pmd, bufA) != 0)
                return SOPC_STATUS_NOK;
        }
        // We did generate enough data
        else
        {
            // We can't use pOUtput in hmac_finish anymore, we would overflow pOutput.
            // Copies P_SHA256 to A because we are not using A again afterwards.
            if (mbedtls_md_hmac_finish(pmd, bufA) != 0)
                return SOPC_STATUS_NOK;
            memcpy(pOutput + offsetOutput, bufA, lenOutput - offsetOutput);
            offsetOutput = lenOutput;
        }
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus CryptoProvider_AsymEncrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenPlainText,
                                                      const SOPC_AsymmetricKey* pKey,
                                                      uint8_t* pOutput)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t lenMsgPlain = 0, lenMsgCiph = 0, lenToCiph = 0;
    mbedtls_rsa_context rsa;

    // Verify the type of the key (this is done here because it is more convenient (lib-specific))
    if (mbedtls_pk_get_type(&pKey->pk) != MBEDTLS_PK_RSA) // TODO: maybe we should accept RSASSA_PSS... Undocumented.
        return SOPC_STATUS_INVALID_PARAMETERS;

    rsa = *mbedtls_pk_rsa(pKey->pk);

    // Sets the appropriate padding mode (SHA-1 for encryption/decryption but SHA-256 for signing/verifying)
    mbedtls_rsa_set_padding(&rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA1);

    // Input must be split into pieces that can be eaten by a single pass of rsa_*_encrypt
    if (SOPC_CryptoProvider_AsymmetricGetLength_Msgs(pProvider, pKey, &lenMsgCiph, &lenMsgPlain) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    while (lenPlainText > 0 && SOPC_STATUS_OK == status)
    {
        if (lenPlainText > lenMsgPlain)
            lenToCiph = lenMsgPlain; // A single pass of encrypt takes at most a message
        else
            lenToCiph = lenPlainText;

        if (mbedtls_rsa_rsaes_oaep_encrypt(&rsa, mbedtls_ctr_drbg_random, &pProvider->pCryptolibContext->ctxDrbg,
                                           MBEDTLS_RSA_PUBLIC, NULL, 0, lenToCiph, (const unsigned char*) pInput,
                                           (unsigned char*) pOutput) != 0)
        {
            status = SOPC_STATUS_NOK;
            break;
        }

        // Advance pointers
        lenPlainText -= lenToCiph;
        if (0 == lenPlainText)
            break;
        pInput += lenMsgPlain;
        pOutput += lenMsgCiph;
    }

    return status;
}

SOPC_ReturnStatus CryptoProvider_AsymDecrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenCipherText,
                                                      const SOPC_AsymmetricKey* pKey,
                                                      uint8_t* pOutput,
                                                      uint32_t* pLenWritten)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t lenMsgPlain = 0, lenMsgCiph = 0;
    size_t lenDeciphed = 0;
    mbedtls_rsa_context rsa;

    if (NULL != pLenWritten)
        *pLenWritten = 0;

    // Verify the type of the key (this is done here because it is more convenient (lib-specific))
    if (mbedtls_pk_get_type(&pKey->pk) != MBEDTLS_PK_RSA) // TODO: maybe we should accept RSASSA_PSS... Undocumented.
        return SOPC_STATUS_INVALID_PARAMETERS;

    rsa = *mbedtls_pk_rsa(pKey->pk);

    // Sets the appropriate padding mode (SHA-1 for encryption/decryption but SHA-256 for signing/verifying)
    mbedtls_rsa_set_padding(&rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA1);

    // Input must be split into pieces that can be eaten by a single pass of rsa_*_decrypt
    if (SOPC_CryptoProvider_AsymmetricGetLength_Msgs(pProvider, pKey, &lenMsgCiph, &lenMsgPlain) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    while (lenCipherText > 0 && SOPC_STATUS_OK == status)
    {
        // TODO: this might fail because of lenMsgPlain (doc recommend that it is at least sizeof(modulus), but here it
        // is the length of the content)
        if (mbedtls_rsa_rsaes_oaep_decrypt(&rsa, mbedtls_ctr_drbg_random, &pProvider->pCryptolibContext->ctxDrbg,
                                           MBEDTLS_RSA_PRIVATE, NULL, 0, &lenDeciphed, (const unsigned char*) pInput,
                                           (unsigned char*) pOutput, lenMsgPlain) != 0)
        {
            status = SOPC_STATUS_NOK;
            break;
        }

        if (NULL != pLenWritten)
        {
            if (lenDeciphed > UINT32_MAX)
                return SOPC_STATUS_NOK;
            *pLenWritten += (uint32_t) lenDeciphed;
        }

        // Advance pointers
        lenCipherText -= lenMsgCiph;
        if (0 == lenCipherText)
            break;
        pInput += lenMsgCiph;
        pOutput += lenDeciphed;
    }

    return status;
}

/**
 * (Internal) Allocates and compute SHA-256 of \p pInput. You must free it.
 */
static inline SOPC_ReturnStatus NewMsgDigestBuffer(const uint8_t* pInput,
                                                   uint32_t lenInput,
                                                   const mbedtls_md_info_t* pmd_info_hash,
                                                   uint8_t** ppHash)
{
    uint8_t* hash = NULL;
    uint32_t lenHash = 0;

    if (NULL == ppHash)
        return SOPC_STATUS_INVALID_PARAMETERS;
    *ppHash = NULL;

    if (NULL == pmd_info_hash)
        return SOPC_STATUS_NOK;

    lenHash = mbedtls_md_get_size(pmd_info_hash);
    hash = malloc(lenHash);
    if (NULL == hash)
        return SOPC_STATUS_NOK;
    *ppHash = hash;

    // Basic256Sha256 : it should be specified that the content to sign is only hashed with a SHA-256,
    // and then sent to pss_sign, which should be done with SHA-256 too.
    if (mbedtls_md(pmd_info_hash, pInput, lenInput, hash) != 0)
        /* TODO: potential hash leak here is avoided in CryptoProvider_AsymSign_* */
        return SOPC_STATUS_NOK;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA256(const SOPC_CryptoProvider* pProvider,
                                                                    const uint8_t* pInput,
                                                                    uint32_t lenInput,
                                                                    const SOPC_AsymmetricKey* pKey,
                                                                    uint8_t* pSignature)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint8_t* hash = NULL;
    mbedtls_rsa_context rsa;
    const mbedtls_md_info_t* pmd_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256); // Hash the message with SHA-256

    if (NewMsgDigestBuffer(pInput, lenInput, pmd_info, &hash) == SOPC_STATUS_OK)
    {
        /* Copy the RSA context from the private key */
        rsa = *mbedtls_pk_rsa(pKey->pk);
        // Sets the appropriate padding mode (no hash-id for PKCS_V15)
        mbedtls_rsa_set_padding(&rsa, MBEDTLS_RSA_PKCS_V15, 0);

        if (mbedtls_rsa_rsassa_pkcs1_v15_sign(&rsa, mbedtls_ctr_drbg_random, &pProvider->pCryptolibContext->ctxDrbg,
                                              MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256,
                                              32,                     // hashlen is optional, as md_alg is not MD_NONE
                                              hash, pSignature) != 0) // signature is as long as the key
            status = SOPC_STATUS_NOK;
        else
            status = SOPC_STATUS_OK;
    }

    if (NULL != hash)
        free(hash);
    return status;
}

SOPC_ReturnStatus CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA256(const SOPC_CryptoProvider* pProvider,
                                                                      const uint8_t* pInput,
                                                                      uint32_t lenInput,
                                                                      const SOPC_AsymmetricKey* pKey,
                                                                      const uint8_t* pSignature)
{
    (void) (pProvider);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint8_t* hash = NULL;
    mbedtls_rsa_context rsa;
    const mbedtls_md_info_t* pmd_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    if (NewMsgDigestBuffer(pInput, lenInput, pmd_info, &hash) == SOPC_STATUS_OK)
    {
        /* Copy the RSA context from the private key */
        rsa = *mbedtls_pk_rsa(pKey->pk);
        // Sets the appropriate padding mode (no hash-id for PKCS_V15)
        mbedtls_rsa_set_padding(&rsa, MBEDTLS_RSA_PKCS_V15, 0);

        if (mbedtls_rsa_rsassa_pkcs1_v15_verify(&rsa, NULL, NULL,
                                                MBEDTLS_RSA_PUBLIC,    // Random functions are optional for verification
                                                MBEDTLS_MD_SHA256, 32, // hashlen is optional, as md_alg is not MD_NONE
                                                hash, pSignature) != 0) // signature is as long as the key
            status = SOPC_STATUS_NOK;
        else
            status = SOPC_STATUS_OK;
    }

    if (NULL != hash)
        free(hash);
    return status;
}

SOPC_ReturnStatus CryptoProvider_CertVerify_RSA_SHA256_2048_4096(const SOPC_CryptoProvider* pCrypto,
                                                                 const SOPC_Certificate* pCert)
{
    SOPC_AsymmetricKey pub_key;
    uint32_t key_length = 0;

    // Retrieve key
    if (KeyManager_Certificate_GetPublicKey(pCert, &pub_key) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    // Verifies key type: RSA
    switch (mbedtls_pk_get_type(&pub_key.pk))
    {
    case MBEDTLS_PK_RSA:
        // case MBEDTLS_PK_RSASSA_PSS: // Don't know the exact meaning of these two...
        // case MBEDTLS_PK_RSA_ALT:
        break;
    default:
        return SOPC_STATUS_NOK;
    }

    // Retrieve key length
    if (SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(pCrypto, &pub_key, &key_length) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;
    // Verifies key length: 2048-4096
    if (key_length < SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits ||
        key_length > SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits)
        return SOPC_STATUS_NOK;

    // Verifies signing algorithm: SHA-256
    if (pCert->crt.sig_md != MBEDTLS_MD_SHA256)
        return SOPC_STATUS_NOK;

    // Does not verify that key is capable of encryption and signing... (!!!)

    return SOPC_STATUS_OK;
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
    uint32_t lenKey;

    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pOutput)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (SOPC_CryptoProvider_SymmetricGetLength_SignKey(pProvider, &lenKey) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    const mbedtls_md_info_t* pinfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    if (mbedtls_md_hmac(pinfo, pKey, lenKey, pInput, lenInput, pOutput) != 0)
        return SOPC_STATUS_NOK;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus CryptoProvider_SymmVerify_HMAC_SHA1(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenInput,
                                                      const SOPC_ExposedBuffer* pKey,
                                                      const uint8_t* pSignature)
{
    uint32_t lenKey, lenSig;
    uint8_t* pCalcSig;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pInput || NULL == pKey || NULL == pSignature)
        return SOPC_STATUS_INVALID_PARAMETERS;

    if (SOPC_CryptoProvider_SymmetricGetLength_SignKey(pProvider, &lenKey) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    if (SOPC_CryptoProvider_SymmetricGetLength_Signature(pProvider, &lenSig) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    pCalcSig = malloc(lenSig);
    if (NULL == pCalcSig)
        return SOPC_STATUS_NOK;

    const mbedtls_md_info_t* pinfo = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    status = mbedtls_md_hmac(pinfo, pKey, lenKey, pInput, lenInput, pCalcSig) != 0 ? SOPC_STATUS_NOK : SOPC_STATUS_OK;

    if (SOPC_STATUS_OK == status)
        status = memcmp(pSignature, pCalcSig, lenSig) != 0 ? SOPC_STATUS_NOK : SOPC_STATUS_OK;

    free(pCalcSig);

    return status;
}

SOPC_ReturnStatus CryptoProvider_DeriveData_PRF_SHA1(const SOPC_CryptoProvider* pProvider,
                                                     const SOPC_ExposedBuffer* pSecret,
                                                     uint32_t lenSecret,
                                                     const SOPC_ExposedBuffer* pSeed,
                                                     uint32_t lenSeed,
                                                     SOPC_ExposedBuffer* pOutput,
                                                     uint32_t lenOutput)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint8_t* bufA = NULL;
    uint32_t lenBufA = 0; // Stores A(i) + seed except for i = 0
    uint32_t lenHash = 0;

    (void) (pProvider);

    if (NULL == pSecret || 0 == lenSecret || NULL == pSeed || 0 == lenSeed || NULL == pOutput || 0 == lenOutput)
        return SOPC_STATUS_INVALID_PARAMETERS;

    const mbedtls_md_info_t* pmd_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);

    if (NULL == pmd_info)
        return SOPC_STATUS_NOK;

    lenHash = mbedtls_md_get_size(pmd_info);
    lenBufA = lenHash + lenSeed;
    if (lenHash == 0 || lenBufA <= lenSeed) // Test uint overflow
        return SOPC_STATUS_NOK;

    bufA = malloc(lenBufA);
    if (NULL == bufA)
        return SOPC_STATUS_NOK;

    // bufA contains A(i) + seed where + is the concatenation.
    // length(A(i)) and the content of seed do not change, so seed is written only once. The beginning of bufA is
    // initialized later.
    memcpy(bufA + lenHash, pSeed, lenSeed);

    // Next stage generates a context for the PSHA
    status = PSHA_outer(pmd_info, bufA, lenBufA, pSecret, lenSecret, pSeed, lenSeed, pOutput, lenOutput);

    // Clear and release A
    memset(bufA, 0, lenBufA);
    free(bufA);

    return status;
}

SOPC_ReturnStatus CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA1(const SOPC_CryptoProvider* pProvider,
                                                                  const uint8_t* pInput,
                                                                  uint32_t lenInput,
                                                                  const SOPC_AsymmetricKey* pKey,
                                                                  uint8_t* pSignature)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint8_t* hash = NULL;
    mbedtls_rsa_context rsa;
    const mbedtls_md_info_t* pmd_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);

    if (NewMsgDigestBuffer(pInput, lenInput, pmd_info, &hash) == SOPC_STATUS_OK)
    {
        /* Copy the RSA context from the private key */
        rsa = *mbedtls_pk_rsa(pKey->pk);
        // Sets the appropriate padding mode (no hash-id for PKCS_V15)
        mbedtls_rsa_set_padding(&rsa, MBEDTLS_RSA_PKCS_V15, 0);

        if (mbedtls_rsa_rsassa_pkcs1_v15_sign(&rsa, mbedtls_ctr_drbg_random, &pProvider->pCryptolibContext->ctxDrbg,
                                              MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA1,
                                              20,                     // hashlen is optional, as md_alg is not MD_NONE
                                              hash, pSignature) != 0) // signature is as long as the key
            status = SOPC_STATUS_NOK;
        else
            status = SOPC_STATUS_OK;
    }

    if (NULL != hash)
        free(hash);
    return status;
}

SOPC_ReturnStatus CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA1(const SOPC_CryptoProvider* pProvider,
                                                                    const uint8_t* pInput,
                                                                    uint32_t lenInput,
                                                                    const SOPC_AsymmetricKey* pKey,
                                                                    const uint8_t* pSignature)
{
    (void) (pProvider);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint8_t* hash = NULL;
    mbedtls_rsa_context rsa;
    const mbedtls_md_info_t* pmd_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);

    if (NewMsgDigestBuffer(pInput, lenInput, pmd_info, &hash) == SOPC_STATUS_OK)
    {
        /* Copy the RSA context from the private key */
        rsa = *mbedtls_pk_rsa(pKey->pk);
        // Sets the appropriate padding mode (no hash-id for PKCS_V15)
        mbedtls_rsa_set_padding(&rsa, MBEDTLS_RSA_PKCS_V15, 0);

        if (mbedtls_rsa_rsassa_pkcs1_v15_verify(&rsa, NULL, NULL,
                                                MBEDTLS_RSA_PUBLIC,  // Random functions are optional for verification
                                                MBEDTLS_MD_SHA1, 20, // hashlen is optional, as md_alg is not MD_NONE
                                                hash, pSignature) != 0) // signature is as long as the key
            status = SOPC_STATUS_NOK;
        else
            status = SOPC_STATUS_OK;
    }

    if (NULL != hash)
        free(hash);
    return status;
}

SOPC_ReturnStatus CryptoProvider_CertVerify_RSA_SHA1_SHA256_1024_2048(const SOPC_CryptoProvider* pCrypto,
                                                                      const SOPC_Certificate* pCert)
{
    SOPC_AsymmetricKey pub_key;
    uint32_t key_length = 0;

    // Retrieve key
    if (KeyManager_Certificate_GetPublicKey(pCert, &pub_key) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;

    // Verifies key type: RSA
    switch (mbedtls_pk_get_type(&pub_key.pk))
    {
    case MBEDTLS_PK_RSA:
        // case MBEDTLS_PK_RSASSA_PSS: // Don't know the exact meaning of these two...
        // case MBEDTLS_PK_RSA_ALT:
        break;
    default:
        return SOPC_STATUS_NOK;
    }

    // Retrieve key length
    if (SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(pCrypto, &pub_key, &key_length) != SOPC_STATUS_OK)
        return SOPC_STATUS_NOK;
    // Verifies key length: 1024-2048
    if (key_length < SOPC_SecurityPolicy_Basic256_AsymLen_KeyMinBits ||
        key_length > SOPC_SecurityPolicy_Basic256_AsymLen_KeyMaxBits)
        return SOPC_STATUS_NOK;

    // Verifies signing algorithm: SHA-1 or SHA-256
    switch (pCert->crt.sig_md)
    {
    case MBEDTLS_MD_SHA1:
    case MBEDTLS_MD_SHA256:
        break;
    default:
        return SOPC_STATUS_NOK;
    }

    // Does not verify that key is capable of encryption and signing... (!!!)

    return SOPC_STATUS_OK;
}
