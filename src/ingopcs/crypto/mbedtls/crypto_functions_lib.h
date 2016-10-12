/*
 * Gathers the definitions of the lib-specific and crypto-related functions.
 * Sources for these functions should be split in the future to provide
 *  finer grained linking options (https://www.ingopcs.net/trac/ingopcs.projects/ticket/187).
 *
 *  Created on: Oct 12, 2016
 *      Author: PAB
 */


#include "ua_base_types.h"
#include "secret_buffer.h"
#include "crypto_types.h"


/* ------------------------------------------------------------------------------------------------
 * Basic256Sha256
 * ------------------------------------------------------------------------------------------------
 */
StatusCode CryptoProvider_SymmEncrypt_AES256(const CryptoProvider *pProvider,
                                                    const uint8_t *pInput,
                                                    uint32_t lenPlainText,
                                                    const ExposedBuffer *pKey,
                                                    const ExposedBuffer *pIV,
                                                    uint8_t *pOutput,
                                                    uint32_t lenOutput);
StatusCode CryptoProvider_SymmDecrypt_AES256(const CryptoProvider *pProvider,
                                       const uint8_t *pInput,
                                       uint32_t lenCipherText,
                                       const ExposedBuffer *pKey,
                                       const ExposedBuffer *pIV,
                                       uint8_t *pOutput,
                                       uint32_t lenOutput);
StatusCode CryptoProvider_SymmSign_HMAC_SHA256(const CryptoProvider *pProvider,
                                                      const uint8_t *pInput,
                                                      uint32_t lenInput,
                                                      const ExposedBuffer *pKey,
                                                      uint8_t *pOutput);
StatusCode CryptoProvider_SymmVerify_HMAC_SHA256(const CryptoProvider *pProvider,
                                                        const uint8_t *pInput,
                                                        uint32_t lenInput,
                                                        const ExposedBuffer *pKey,
                                                        const uint8_t *pSignature);
StatusCode CryptoProvider_SymmGenKey_AES256(const CryptoProvider *pProvider,
                                            ExposedBuffer *pKey);

