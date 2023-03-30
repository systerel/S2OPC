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

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_secret_buffer.h"

#include "crypto_functions_lib.h"
#include "crypto_provider_lib.h"
#include "key_manager_lib.h"

// TODO: the right cyclone_crypto includes here

/* ------------------------------------------------------------------------------------------------
 * Aes128-Sha256-RsaOaep
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus CryptoProvider_SymmEncrypt_AES128(const SOPC_CryptoProvider* pProvider,
                                                    const uint8_t* pInput,
                                                    uint32_t lenPlainText,
                                                    const SOPC_ExposedBuffer* pKey,
                                                    const SOPC_ExposedBuffer* pIV,
                                                    uint8_t* pOutput,
                                                    uint32_t lenOutput)
{
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenPlainText);
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(pIV);
    SOPC_UNUSED_ARG(pOutput);
    SOPC_UNUSED_ARG(lenOutput);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus CryptoProvider_SymmDecrypt_AES128(const SOPC_CryptoProvider* pProvider,
                                                    const uint8_t* pInput,
                                                    uint32_t lenCipherText,
                                                    const SOPC_ExposedBuffer* pKey,
                                                    const SOPC_ExposedBuffer* pIV,
                                                    uint8_t* pOutput,
                                                    uint32_t lenOutput)
{
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenCipherText);
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(pIV);
    SOPC_UNUSED_ARG(pOutput);
    SOPC_UNUSED_ARG(lenOutput);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

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
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenPlainText);
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(pIV);
    SOPC_UNUSED_ARG(pOutput);
    SOPC_UNUSED_ARG(lenOutput);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

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
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenCipherText);
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(pIV);
    SOPC_UNUSED_ARG(pOutput);
    SOPC_UNUSED_ARG(lenOutput);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

static inline SOPC_ReturnStatus HMAC_hashtype_sign(const SOPC_CryptoProvider* pProvider,
                                                   const uint8_t* pInput,
                                                   uint32_t lenInput,
                                                   const SOPC_ExposedBuffer* pKey,
                                                   uint8_t* pOutput);
static inline SOPC_ReturnStatus HMAC_hashtype_verify(const SOPC_CryptoProvider* pProvider,
                                                     const uint8_t* pInput,
                                                     uint32_t lenInput,
                                                     const SOPC_ExposedBuffer* pKey,
                                                     const uint8_t* pSignature);

static inline SOPC_ReturnStatus HMAC_hashtype_sign(const SOPC_CryptoProvider* pProvider,
                                                   const uint8_t* pInput,
                                                   uint32_t lenInput,
                                                   const SOPC_ExposedBuffer* pKey,
                                                   uint8_t* pOutput)
{
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenInput);
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(pOutput);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

static inline SOPC_ReturnStatus HMAC_hashtype_verify(const SOPC_CryptoProvider* pProvider,
                                                     const uint8_t* pInput,
                                                     uint32_t lenInput,
                                                     const SOPC_ExposedBuffer* pKey,
                                                     const uint8_t* pSignature)
{
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenInput);
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(pSignature);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus CryptoProvider_SymmSign_HMAC_SHA256(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenInput,
                                                      const SOPC_ExposedBuffer* pKey,
                                                      uint8_t* pOutput)
{
    return HMAC_hashtype_sign(pProvider, pInput, lenInput, pKey, pOutput);
}

SOPC_ReturnStatus CryptoProvider_SymmVerify_HMAC_SHA256(const SOPC_CryptoProvider* pProvider,
                                                        const uint8_t* pInput,
                                                        uint32_t lenInput,
                                                        const SOPC_ExposedBuffer* pKey,
                                                        const uint8_t* pSignature)
{
    return HMAC_hashtype_verify(pProvider, pInput, lenInput, pKey, pSignature);
}

// Fills a buffer with "truly" random data
SOPC_ReturnStatus CryptoProvider_GenTrueRnd(const SOPC_CryptoProvider* pProvider,
                                            SOPC_ExposedBuffer* pData,
                                            uint32_t lenData)
{
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pData);
    SOPC_UNUSED_ARG(lenData);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

// PRF with SHA256 as defined in RFC 5246 (TLS v1.2), §5, without label.
// Based on a HMAC with SHA-256.
static inline SOPC_ReturnStatus PSHA_outer(uint8_t* bufA,
                                           uint32_t lenBufA,
                                           const SOPC_ExposedBuffer* pSecret,
                                           uint32_t lenSecret,
                                           const SOPC_ExposedBuffer* pSeed,
                                           uint32_t lenSeed,
                                           SOPC_ExposedBuffer* pOutput,
                                           uint32_t lenOutput);

static inline SOPC_ReturnStatus PSHA(uint8_t* bufA,
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
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pSecret);
    SOPC_UNUSED_ARG(lenSecret);
    SOPC_UNUSED_ARG(pSeed);
    SOPC_UNUSED_ARG(lenSeed);
    SOPC_UNUSED_ARG(pOutput);
    SOPC_UNUSED_ARG(lenOutput);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

static inline SOPC_ReturnStatus PSHA_outer(uint8_t* bufA,
                                           uint32_t lenBufA,
                                           const SOPC_ExposedBuffer* pSecret,
                                           uint32_t lenSecret,
                                           const SOPC_ExposedBuffer* pSeed,
                                           uint32_t lenSeed,
                                           SOPC_ExposedBuffer* pOutput,
                                           uint32_t lenOutput)
{
    SOPC_UNUSED_ARG(bufA);
    SOPC_UNUSED_ARG(lenBufA);
    SOPC_UNUSED_ARG(pSecret);
    SOPC_UNUSED_ARG(lenSecret);
    SOPC_UNUSED_ARG(pSeed);
    SOPC_UNUSED_ARG(lenSeed);
    SOPC_UNUSED_ARG(pOutput);
    SOPC_UNUSED_ARG(lenOutput);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

static inline SOPC_ReturnStatus PSHA(uint8_t* bufA,
                                     uint32_t lenBufA,
                                     const SOPC_ExposedBuffer* pSecret,
                                     uint32_t lenSecret,
                                     const SOPC_ExposedBuffer* pSeed,
                                     uint32_t lenSeed,
                                     SOPC_ExposedBuffer* pOutput,
                                     uint32_t lenOutput)
{
    SOPC_UNUSED_ARG(bufA);
    SOPC_UNUSED_ARG(lenBufA);
    SOPC_UNUSED_ARG(pSecret);
    SOPC_UNUSED_ARG(lenSecret);
    SOPC_UNUSED_ARG(pSeed);
    SOPC_UNUSED_ARG(lenSeed);
    SOPC_UNUSED_ARG(pOutput);
    SOPC_UNUSED_ARG(lenOutput);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus AsymEncrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                       const uint8_t* pInput,
                                       uint32_t lenPlainText,
                                       const SOPC_AsymmetricKey* pKey,
                                       uint8_t* pOutput);

SOPC_ReturnStatus AsymDecrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                       const uint8_t* pInput,
                                       uint32_t lenCipherText,
                                       const SOPC_AsymmetricKey* pKey,
                                       uint8_t* pOutput,
                                       uint32_t* pLenWritten);

SOPC_ReturnStatus AsymEncrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                       const uint8_t* pInput,
                                       uint32_t lenPlainText,
                                       const SOPC_AsymmetricKey* pKey,
                                       uint8_t* pOutput)
{
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenPlainText);
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(pOutput);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus AsymDecrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                       const uint8_t* pInput,
                                       uint32_t lenCipherText,
                                       const SOPC_AsymmetricKey* pKey,
                                       uint8_t* pOutput,
                                       uint32_t* pLenWritten)
{
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenCipherText);
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(pOutput);
    SOPC_UNUSED_ARG(pLenWritten);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus CryptoProvider_AsymEncrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenPlainText,
                                                      const SOPC_AsymmetricKey* pKey,
                                                      uint8_t* pOutput)
{
    return AsymEncrypt_RSA_OAEP(pProvider, pInput, lenPlainText, pKey, pOutput);
}

SOPC_ReturnStatus CryptoProvider_AsymDecrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenCipherText,
                                                      const SOPC_AsymmetricKey* pKey,
                                                      uint8_t* pOutput,
                                                      uint32_t* pLenWritten)
{
    return AsymDecrypt_RSA_OAEP(pProvider, pInput, lenCipherText, pKey, pOutput, pLenWritten);
}

SOPC_ReturnStatus CryptoProvider_AsymEncrypt_RSA_OAEP_SHA256(const SOPC_CryptoProvider* pProvider,
                                                             const uint8_t* pInput,
                                                             uint32_t lenPlainText,
                                                             const SOPC_AsymmetricKey* pKey,
                                                             uint8_t* pOutput)
{
    return AsymEncrypt_RSA_OAEP(pProvider, pInput, lenPlainText, pKey, pOutput);
}

SOPC_ReturnStatus CryptoProvider_AsymDecrypt_RSA_OAEP_SHA256(const SOPC_CryptoProvider* pProvider,
                                                             const uint8_t* pInput,
                                                             uint32_t lenCipherText,
                                                             const SOPC_AsymmetricKey* pKey,
                                                             uint8_t* pOutput,
                                                             uint32_t* pLenWritten)
{
    return AsymDecrypt_RSA_OAEP(pProvider, pInput, lenCipherText, pKey, pOutput, pLenWritten);
}

/**
 * (Internal) Allocates and compute SHA-256 of \p pInput. You must free it.
 */
static inline SOPC_ReturnStatus NewMsgDigestBuffer(const uint8_t* pInput, uint32_t lenInput, uint8_t** ppHash)
{
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenInput);
    SOPC_UNUSED_ARG(ppHash);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus AsymSign_RSASSA(const SOPC_CryptoProvider* pProvider,
                                  const uint8_t* pInput,
                                  uint32_t lenInput,
                                  const SOPC_AsymmetricKey* pKey,
                                  uint8_t* pSignature,
                                  int padding,
                                  unsigned int hash_len,
                                  bool pss);

SOPC_ReturnStatus AsymVerify_RSASSA(const SOPC_CryptoProvider* pProvider,
                                    const uint8_t* pInput,
                                    uint32_t lenInput,
                                    const SOPC_AsymmetricKey* pKey,
                                    const uint8_t* pSignature,
                                    int padding,
                                    unsigned int hash_len,
                                    bool pss);

SOPC_ReturnStatus AsymSign_RSASSA(const SOPC_CryptoProvider* pProvider,
                                  const uint8_t* pInput,
                                  uint32_t lenInput,
                                  const SOPC_AsymmetricKey* pKey,
                                  uint8_t* pSignature,
                                  int padding,
                                  unsigned int hash_len,
                                  bool pss)
{
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenInput);
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(pSignature);
    SOPC_UNUSED_ARG(padding);
    SOPC_UNUSED_ARG(hash_len);
    SOPC_UNUSED_ARG(pss);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus AsymVerify_RSASSA(const SOPC_CryptoProvider* pProvider,
                                    const uint8_t* pInput,
                                    uint32_t lenInput,
                                    const SOPC_AsymmetricKey* pKey,
                                    const uint8_t* pSignature,
                                    int padding,
                                    unsigned int hash_len,
                                    bool pss)
{
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenInput);
    SOPC_UNUSED_ARG(pKey);
    SOPC_UNUSED_ARG(pSignature);
    SOPC_UNUSED_ARG(padding);
    SOPC_UNUSED_ARG(hash_len);
    SOPC_UNUSED_ARG(pss);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA256(const SOPC_CryptoProvider* pProvider,
                                                                    const uint8_t* pInput,
                                                                    uint32_t lenInput,
                                                                    const SOPC_AsymmetricKey* pKey,
                                                                    uint8_t* pSignature)
{
    return AsymSign_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, 0, 32, false);
}

SOPC_ReturnStatus CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA256(const SOPC_CryptoProvider* pProvider,
                                                                      const uint8_t* pInput,
                                                                      uint32_t lenInput,
                                                                      const SOPC_AsymmetricKey* pKey,
                                                                      const uint8_t* pSignature)
{
    return AsymVerify_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, 0, 32, false);
}

SOPC_ReturnStatus CryptoProvider_AsymSign_RSASSA_PSS(const SOPC_CryptoProvider* pProvider,
                                                     const uint8_t* pInput,
                                                     uint32_t lenInput,
                                                     const SOPC_AsymmetricKey* pKey,
                                                     uint8_t* pSignature)
{
    return AsymSign_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, 0, 32, true);
}

SOPC_ReturnStatus CryptoProvider_AsymVerify_RSASSA_PSS(const SOPC_CryptoProvider* pProvider,
                                                       const uint8_t* pInput,
                                                       uint32_t lenInput,
                                                       const SOPC_AsymmetricKey* pKey,
                                                       const uint8_t* pSignature)
{
    return AsymVerify_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, 0, 32, true);
}

SOPC_ReturnStatus CryptoProvider_CertVerify_RSA_SHA256_2048_4096(const SOPC_CryptoProvider* pCrypto,
                                                                 const SOPC_CertificateList* pCert)
{
    SOPC_UNUSED_ARG(pCrypto);
    SOPC_UNUSED_ARG(pCert);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

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
    return HMAC_hashtype_sign(pProvider, pInput, lenInput, pKey, pOutput);
}
SOPC_ReturnStatus CryptoProvider_SymmVerify_HMAC_SHA1(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenInput,
                                                      const SOPC_ExposedBuffer* pKey,
                                                      const uint8_t* pSignature)
{
    return HMAC_hashtype_verify(pProvider, pInput, lenInput, pKey, pSignature);
}
SOPC_ReturnStatus CryptoProvider_DeriveData_PRF_SHA1(const SOPC_CryptoProvider* pProvider,
                                                     const SOPC_ExposedBuffer* pSecret,
                                                     uint32_t lenSecret,
                                                     const SOPC_ExposedBuffer* pSeed,
                                                     uint32_t lenSeed,
                                                     SOPC_ExposedBuffer* pOutput,
                                                     uint32_t lenOutput)
{
    SOPC_UNUSED_ARG(pProvider);
    SOPC_UNUSED_ARG(pSecret);
    SOPC_UNUSED_ARG(lenSecret);
    SOPC_UNUSED_ARG(pSeed);
    SOPC_UNUSED_ARG(lenSeed);
    SOPC_UNUSED_ARG(pOutput);
    SOPC_UNUSED_ARG(lenOutput);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA1(const SOPC_CryptoProvider* pProvider,
                                                                  const uint8_t* pInput,
                                                                  uint32_t lenInput,
                                                                  const SOPC_AsymmetricKey* pKey,
                                                                  uint8_t* pSignature)
{
    return AsymSign_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, 0, 20, false);
}

SOPC_ReturnStatus CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA1(const SOPC_CryptoProvider* pProvider,
                                                                    const uint8_t* pInput,
                                                                    uint32_t lenInput,
                                                                    const SOPC_AsymmetricKey* pKey,
                                                                    const uint8_t* pSignature)
{
    return AsymVerify_RSASSA(pProvider, pInput, lenInput, pKey, pSignature, 0, 20, false);
}

SOPC_ReturnStatus CryptoProvider_CertVerify_RSA_SHA1_SHA256_1024_2048(const SOPC_CryptoProvider* pCrypto,
                                                                      const SOPC_CertificateList* pCert)
{
    SOPC_UNUSED_ARG(pCrypto);
    SOPC_UNUSED_ARG(pCert);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
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
    SOPC_UNUSED_ARG(pInput);
    SOPC_UNUSED_ARG(lenInput);
    SOPC_UNUSED_ARG(pExpKey);
    SOPC_UNUSED_ARG(pExpNonce);
    SOPC_UNUSED_ARG(pRandom);
    SOPC_UNUSED_ARG(uSequenceNumber);
    SOPC_UNUSED_ARG(pOutput);

    SOPC_ASSERT(false && "NOT IMPLEMENTED YET");

    return SOPC_STATUS_OK;
}
