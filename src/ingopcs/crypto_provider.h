/*
 * Defines the cryptographic providers: structure r/w data alongside a read-only cryptoprofile.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */


#ifndef INGOPCS_CRYPTO_PROVIDER_H_
#define INGOPCS_CRYPTO_PROVIDER_H_


#include "ua_builtintypes.h"  // StatusCode
#include "crypto_types.h"


// TODO: move API
SecretBuffer *SecretBuffer_NewFromExposedBuffer(ExposedBuffer *buf, uint32_t len);
void SecretBuffer_DeleteClear(SecretBuffer *sec);
uint32_t SecretBuffer_GetLength(SecretBuffer *sec);

ExposedBuffer * SecretBuffer_Expose(SecretBuffer *sec);
void SecretBuffer_Unexpose(ExposedBuffer *buf);


/* ------------------------------------------------------------------------------------------------
 * CryptoProvider API
 * ------------------------------------------------------------------------------------------------
 */


// Lib specific
StatusCode CryptoProvider_LibInit(CryptoProvider *pCryptoProvider);
StatusCode CryptoProvider_LibDeinit(CryptoProvider *pCryptoProvider);

// Real API ("_Low" suffix because temporary wrappers already use the shorter names)
CryptoProvider *CryptoProvider_Create(const char *uri);
void CryptoProvider_Delete(CryptoProvider *pCryptoProvider);

StatusCode CryptoProvider_SymmetricGetLength_Key(const CryptoProvider *pProvider,
                                                     uint32_t *length);
StatusCode CryptoProvider_SymmetricGetLength_Encryption(const CryptoProvider *pProvider,
                                                        uint32_t lengthIn,
                                                        uint32_t *pLengthOut);
StatusCode CryptoProvider_SymmetricGetLength_Decryption(const CryptoProvider *pProvider,
                                                        uint32_t lengthIn,
                                                        uint32_t *pLengthOut);
StatusCode CryptoProvider_SymmetricGetLength_Signature(const CryptoProvider *pProvider,
                                                       uint32_t *pLength);
StatusCode CryptoProvider_SymmetricGetLength_BlockSizes(const CryptoProvider *pProvider,
                                                        uint32_t *cipherTextBlockSize,
                                                        uint32_t *plainTextBlockSize);

StatusCode CryptoProvider_SymmetricEncrypt(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenPlainText,
                                           const SecretBuffer *pKey,
                                           const SecretBuffer *pIV,
                                           uint8_t *pOutput,
                                           uint32_t lenOutput);
StatusCode CryptoProvider_SymmetricDecrypt(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenCipherText,
                                           const SecretBuffer *pKey,
                                           const SecretBuffer *pIV,
                                           uint8_t *pOutput,
                                           uint32_t lenOutput);
StatusCode CryptoProvider_SymmetricSign(const CryptoProvider *pProvider,
                                        const uint8_t *pInput,
                                        uint32_t lenInput,
                                        const SecretBuffer *pKey,
                                        uint8_t *pOutput,
                                        uint32_t lenOutput);
StatusCode CryptoProvider_SymmetricVerify(const CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenInput,
                                          const SecretBuffer *pKey,
                                          const uint8_t *pSignature,
                                          uint32_t lenOutput);
StatusCode CryptoProvider_SymmetricGenerateKey(const CryptoProvider *pProvider,
                                               SecretBuffer **ppKeyGenerated);

#endif  // INGOPCS_CRYPTO_PROVIDER_H_
