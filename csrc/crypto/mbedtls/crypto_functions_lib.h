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
 * Gathers the definitions of the lib-specific and crypto-related functions.
 * Sources for these functions should be split in the future to provide
 * finer grained linking options (https://www.ingopcs.net/trac/ingopcs.projects/ticket/187).
 */

#ifndef SOPC_CRYPTO_FUNCTIONS_LIB_
#define SOPC_CRYPTO_FUNCTIONS_LIB_

#include "../sopc_crypto_decl.h"
#include "../sopc_key_manager.h"
#include "../sopc_secret_buffer.h"

/* ------------------------------------------------------------------------------------------------
 * Basic256Sha256
 * ------------------------------------------------------------------------------------------------
 */
SOPC_ReturnStatus CryptoProvider_SymmEncrypt_AES256(const SOPC_CryptoProvider* pProvider,
                                                    const uint8_t* pInput,
                                                    uint32_t lenPlainText,
                                                    const SOPC_ExposedBuffer* pKey,
                                                    const SOPC_ExposedBuffer* pIV,
                                                    uint8_t* pOutput,
                                                    uint32_t lenOutput);
SOPC_ReturnStatus CryptoProvider_SymmDecrypt_AES256(const SOPC_CryptoProvider* pProvider,
                                                    const uint8_t* pInput,
                                                    uint32_t lenCipherText,
                                                    const SOPC_ExposedBuffer* pKey,
                                                    const SOPC_ExposedBuffer* pIV,
                                                    uint8_t* pOutput,
                                                    uint32_t lenOutput);
SOPC_ReturnStatus CryptoProvider_SymmSign_HMAC_SHA256(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenInput,
                                                      const SOPC_ExposedBuffer* pKey,
                                                      uint8_t* pOutput);
SOPC_ReturnStatus CryptoProvider_SymmVerify_HMAC_SHA256(const SOPC_CryptoProvider* pProvider,
                                                        const uint8_t* pInput,
                                                        uint32_t lenInput,
                                                        const SOPC_ExposedBuffer* pKey,
                                                        const uint8_t* pSignature);
SOPC_ReturnStatus CryptoProvider_GenTrueRnd(const SOPC_CryptoProvider* pProvider,
                                            SOPC_ExposedBuffer* pData,
                                            uint32_t lenData);
SOPC_ReturnStatus CryptoProvider_DeriveData_PRF_SHA256(const SOPC_CryptoProvider* pProvider,
                                                       const SOPC_ExposedBuffer* pSecret,
                                                       uint32_t lenSecret,
                                                       const SOPC_ExposedBuffer* pSeed,
                                                       uint32_t lenSeed,
                                                       SOPC_ExposedBuffer* pOutput,
                                                       uint32_t lenOutput);

SOPC_ReturnStatus CryptoProvider_AsymEncrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenPlainText,
                                                      const SOPC_AsymmetricKey* pKey,
                                                      uint8_t* pOutput);
SOPC_ReturnStatus CryptoProvider_AsymDecrypt_RSA_OAEP(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenPlainText,
                                                      const SOPC_AsymmetricKey* pKey,
                                                      uint8_t* pOutput,
                                                      uint32_t* pLenWritten);
/**
 * This one is too up-to-date, don't use (but was tested). As not said by the current security policy, classic stack
 * uses PKCS#1 v1.5 padding, not PSS...
 *
 * https://tools.ietf.org/html/rfc3447#section-8
 *   Two signature schemes with appendix are specified in this document:
 *   RSASSA-PSS and RSASSA-PKCS1-v1_5.  Although no attacks are known
 *   against RSASSA-PKCS1-v1_5, in the interest of increased robustness,
 *   RSASSA-PSS is recommended for eventual adoption in new applications.
 *   RSASSA-PKCS1-v1_5 is included for compatibility with existing
 *   applications, and while still appropriate for new applications, a
 *   gradual transition to RSASSA-PSS is encouraged.
 */
SOPC_ReturnStatus CryptoProvider_AsymSign_RSASSA_PSS(const SOPC_CryptoProvider* pProvider,
                                                     const uint8_t* pInput,
                                                     uint32_t lenInput,
                                                     const SOPC_AsymmetricKey* pKey,
                                                     uint8_t* pSignature);
/**
 * This one is too up-to-date, don't use (but was tested). As not said by the current security policy, classic stack
 * uses PKCS#1 v1.5 padding, not PSS...
 *
 * https://tools.ietf.org/html/rfc3447#section-8
 *   Two signature schemes with appendix are specified in this document:
 *   RSASSA-PSS and RSASSA-PKCS1-v1_5.  Although no attacks are known
 *   against RSASSA-PKCS1-v1_5, in the interest of increased robustness,
 *   RSASSA-PSS is recommended for eventual adoption in new applications.
 *   RSASSA-PKCS1-v1_5 is included for compatibility with existing
 *   applications, and while still appropriate for new applications, a
 *   gradual transition to RSASSA-PSS is encouraged.
 */
SOPC_ReturnStatus CryptoProvider_AsymVerify_RSASSA_PSS(const SOPC_CryptoProvider* pProvider,
                                                       const uint8_t* pInput,
                                                       uint32_t lenInput,
                                                       const SOPC_AsymmetricKey* pKey,
                                                       const uint8_t* pSignature);

SOPC_ReturnStatus CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA256(const SOPC_CryptoProvider* pProvider,
                                                                    const uint8_t* pInput,
                                                                    uint32_t lenInput,
                                                                    const SOPC_AsymmetricKey* pKey,
                                                                    uint8_t* pSignature);
SOPC_ReturnStatus CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA256(const SOPC_CryptoProvider* pProvider,
                                                                      const uint8_t* pInput,
                                                                      uint32_t lenInput,
                                                                      const SOPC_AsymmetricKey* pKey,
                                                                      const uint8_t* pSignature);

SOPC_ReturnStatus CryptoProvider_CertVerify_RSA_SHA256_2048_4096(const SOPC_CryptoProvider* pCrypto,
                                                                 const SOPC_Certificate* pCert);

/* ------------------------------------------------------------------------------------------------
 * Basic256
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus CryptoProvider_SymmSign_HMAC_SHA1(const SOPC_CryptoProvider* pProvider,
                                                    const uint8_t* pInput,
                                                    uint32_t lenInput,
                                                    const SOPC_ExposedBuffer* pKey,
                                                    uint8_t* pOutput);
SOPC_ReturnStatus CryptoProvider_SymmVerify_HMAC_SHA1(const SOPC_CryptoProvider* pProvider,
                                                      const uint8_t* pInput,
                                                      uint32_t lenInput,
                                                      const SOPC_ExposedBuffer* pKey,
                                                      const uint8_t* pSignature);
SOPC_ReturnStatus CryptoProvider_DeriveData_PRF_SHA1(const SOPC_CryptoProvider* pProvider,
                                                     const SOPC_ExposedBuffer* pSecret,
                                                     uint32_t lenSecret,
                                                     const SOPC_ExposedBuffer* pSeed,
                                                     uint32_t lenSeed,
                                                     SOPC_ExposedBuffer* pOutput,
                                                     uint32_t lenOutput);
SOPC_ReturnStatus CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA1(const SOPC_CryptoProvider* pProvider,
                                                                  const uint8_t* pInput,
                                                                  uint32_t lenInput,
                                                                  const SOPC_AsymmetricKey* pKey,
                                                                  uint8_t* pSignature);
SOPC_ReturnStatus CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA1(const SOPC_CryptoProvider* pProvider,
                                                                    const uint8_t* pInput,
                                                                    uint32_t lenInput,
                                                                    const SOPC_AsymmetricKey* pKey,
                                                                    const uint8_t* pSignature);
SOPC_ReturnStatus CryptoProvider_CertVerify_RSA_SHA1_SHA256_1024_2048(const SOPC_CryptoProvider* pCrypto,
                                                                      const SOPC_Certificate* pCert);

#endif /* SOPC_CRYPTO_FUNCTIONS_LIB_ */
