/*
 * Gathers the definitions of the lib-specific and crypto-related functions.
 * Sources for these functions should be split in the future to provide
 *  finer grained linking options (https://www.ingopcs.net/trac/ingopcs.projects/ticket/187).
 *
 *  Created on: Oct 12, 2016
 *      Author: PAB
 */


#include "sopc_base_types.h"
#include "secret_buffer.h"
#include "crypto_types.h"
#include "key_manager.h"


/* ------------------------------------------------------------------------------------------------
 * Basic256Sha256
 * ------------------------------------------------------------------------------------------------
 */
SOPC_StatusCode CryptoProvider_SymmEncrypt_AES256(const CryptoProvider *pProvider,
                                                    const uint8_t *pInput,
                                                    uint32_t lenPlainText,
                                                    const ExposedBuffer *pKey,
                                                    const ExposedBuffer *pIV,
                                                    uint8_t *pOutput,
                                                    uint32_t lenOutput);
SOPC_StatusCode CryptoProvider_SymmDecrypt_AES256(const CryptoProvider *pProvider,
                                       const uint8_t *pInput,
                                       uint32_t lenCipherText,
                                       const ExposedBuffer *pKey,
                                       const ExposedBuffer *pIV,
                                       uint8_t *pOutput,
                                       uint32_t lenOutput);
SOPC_StatusCode CryptoProvider_SymmSign_HMAC_SHA256(const CryptoProvider *pProvider,
                                                      const uint8_t *pInput,
                                                      uint32_t lenInput,
                                                      const ExposedBuffer *pKey,
                                                      uint8_t *pOutput);
SOPC_StatusCode CryptoProvider_SymmVerify_HMAC_SHA256(const CryptoProvider *pProvider, // TODO: delete, useless
                                                        const uint8_t *pInput,
                                                        uint32_t lenInput,
                                                        const ExposedBuffer *pKey,
                                                        const uint8_t *pSignature);
SOPC_StatusCode CryptoProvider_SymmGenKey_AES256(const CryptoProvider *pProvider,
                                            ExposedBuffer *pKey);
SOPC_StatusCode CryptoProvider_DeriveData_PRF_SHA256(const CryptoProvider *pProvider,
                                                const ExposedBuffer *pSecret,
                                                uint32_t lenSecret,
                                                const ExposedBuffer *pSeed,
                                                uint32_t lenSeed,
                                                ExposedBuffer *pOutput,
                                                uint32_t lenOutput);

SOPC_StatusCode CryptoProvider_AsymEncrypt_RSA_OAEP(const CryptoProvider *pProvider,
                                               const uint8_t *pInput,
                                               uint32_t lenPlainText,
                                               const AsymmetricKey *pKey,
                                               uint8_t *pOutput);
SOPC_StatusCode CryptoProvider_AsymDecrypt_RSA_OAEP(const CryptoProvider *pProvider,
                                               const uint8_t *pInput,
                                               uint32_t lenPlainText,
                                               const AsymmetricKey *pKey,
                                               uint8_t *pOutput,
                                               uint32_t *pLenWritten);
/**
 * This one is too up-to-date, don't use (but was tested). As not said by the current security policy, classic stack uses PKCS#1 v1.5 padding, not PSS...
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
SOPC_StatusCode CryptoProvider_AsymSign_RSASSA_PSS(const CryptoProvider *pProvider,
                                              const uint8_t *pInput,
                                              uint32_t lenInput,
                                              const AsymmetricKey *pKey,
                                              uint8_t *pSignature);
/**
 * This one is too up-to-date, don't use (but was tested). As not said by the current security policy, classic stack uses PKCS#1 v1.5 padding, not PSS...
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
SOPC_StatusCode CryptoProvider_AsymVerify_RSASSA_PSS(const CryptoProvider *pProvider,
                                                const uint8_t *pInput,
                                                uint32_t lenInput,
                                                const AsymmetricKey *pKey,
                                                const uint8_t *pSignature);


SOPC_StatusCode CryptoProvider_AsymSign_RSASSA_PKCS1_v15(const CryptoProvider *pProvider,
                                                    const uint8_t *pInput,
                                                    uint32_t lenInput,
                                                    const AsymmetricKey *pKey,
                                                    uint8_t *pSignature);
SOPC_StatusCode CryptoProvider_AsymVerify_RSASSA_PKCS1_v15(const CryptoProvider *pProvider,
                                                      const uint8_t *pInput,
                                                      uint32_t lenInput,
                                                      const AsymmetricKey *pKey,
                                                      const uint8_t *pSignature);

SOPC_StatusCode CryptoProvider_CertVerify_RSA_SHA256_2048_4096(const CryptoProvider *pCrypto,
                                                          const Certificate *pCert);
