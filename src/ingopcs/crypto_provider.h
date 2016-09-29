/*
 * Defines the cryptographic providers: structure r/w data alongside a read-only cryptoprofile.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */


#ifndef INGOPCS_CRYPTO_PROVIDER_H_
#define INGOPCS_CRYPTO_PROVIDER_H_


#include "crypto_profiles.h"
#include "ua_builtintypes.h"


typedef uint8_t KeyBuffer;  // TODO: move towards private_key.h + This definition is unclear

typedef struct CryptoProvider
{
    const struct CryptoProfile * const pProfile;
    void *pCryptolibContext;
} CryptoProvider;


CryptoProvider *CryptoProvider_Create(const UA_String *uri);
CryptoProvider *CryptoProvider_Create_Low(const char *uri);
void CryptoProvider_Delete(CryptoProvider *pCryptoProvider);

// Lib specific
StatusCode CryptoProvider_LibInit(CryptoProvider *pCryptoProvider);
StatusCode CryptoProvider_LibDeinit(CryptoProvider *pCryptoProvider);

// Real API ("_Low" suffix because temporary wrappers already use the shorter names)
StatusCode CryptoProvider_SymmetricEncrypt_Low(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenPlainText,
                                           const KeyBuffer *pKey,
                                           const uint8_t *pIV,
                                           uint8_t *pOutput,
                                           uint32_t lenOutput);
StatusCode CryptoProvider_SymmetricDecrypt_Low(const CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenCipherText,
                                           const KeyBuffer *pKey,
                                           const uint8_t *pIV,
                                           uint8_t *pOutput,
                                           uint32_t lenOutput);
StatusCode CryptoProvider_Symmetric_GetKeyLength_Low(const CryptoProvider *pProvider,
                                                     uint32_t *length);
StatusCode CryptoProvider_Symmetric_GetOutputLength_Low(const CryptoProvider *pProvider,
                                                        uint32_t lengthIn,
                                                        uint32_t *pLengthOut);

#endif  // INGOPCS_CRYPTO_PROVIDER_H_
