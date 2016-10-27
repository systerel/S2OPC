/*
 * Defines the cryptographic structures: CryptoProfiles and CryptoProviders.
 *
 *  Created on: Oct 04, 2016
 *      Author: PAB
 */


#ifndef INGOPCS_CRYPTO_TYPES_H_
#define INGOPCS_CRYPTO_TYPES_H_


#include "ua_base_types.h"
#include "secret_buffer.h"

struct CryptoProvider;
struct CryptoProfile;
struct CryptolibContext;
struct AsymmetricKey;

/* ------------------------------------------------------------------------------------------------
 * CryptoProfile, internal API
 * ------------------------------------------------------------------------------------------------
 */
typedef StatusCode (*FnSymmetricEncrypt) (const struct CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenPlainText,
                                          const ExposedBuffer *pKey,
                                          const ExposedBuffer *pIV,
                                          uint8_t *pOutput,
                                          uint32_t lenOutput);
typedef StatusCode (*FnSymmetricDecrypt) (const struct CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenCipherText,
                                          const ExposedBuffer *pKey,
                                          const ExposedBuffer *pIV,
                                          uint8_t *pOutput,
                                          uint32_t lenOutput);
typedef StatusCode (*FnSymmetricSign) (const struct CryptoProvider *pProvider,
                                       const uint8_t *pInput,
                                       uint32_t lenInput,
                                       const ExposedBuffer *pKey,
                                       uint8_t *pOutput);
typedef StatusCode (*FnSymmetricVerify) (const struct CryptoProvider *pProvider,
                                         const uint8_t *pInput,
                                         uint32_t lenInput,
                                         const ExposedBuffer *pKey,
                                         const uint8_t *pSignature);
typedef StatusCode (*FnSymmetricGenKey) (const struct CryptoProvider *pProvider,
                                         ExposedBuffer *pKey);
typedef StatusCode (*FnDerivePseudoRandomData) (const struct CryptoProvider *pProvider,
                                                const ExposedBuffer *pSecret,
                                                uint32_t lenSecret,
                                                const ExposedBuffer *pSeed,
                                                uint32_t lenSeed,
                                                ExposedBuffer *pOutput,
                                                uint32_t lenOutput);
typedef StatusCode (*FnAsymmetricEncrypt) (const struct CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenPlainText,
                                           const struct AsymmetricKey *pKey,
                                           uint8_t *pOutput);
typedef StatusCode (*FnAsymmetricDecrypt) (const struct CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenCipherText,
                                           const struct AsymmetricKey *pKey,
                                           uint8_t *pOutput,
                                           uint32_t *lenWritten);
typedef StatusCode (*FnAsymmetricSign) (const struct CryptoProvider *pProvider,
                                        const uint8_t *pInput,
                                        uint32_t lenInput,
                                        const struct AsymmetricKey *pKey,
                                        uint8_t *pSignature);
typedef StatusCode (*FnAsymmetricVerify) (const struct CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenInput,
                                          const struct AsymmetricKey *pKey,
                                          const uint8_t *pSignature);

typedef struct CryptoProfile
{
    const uint32_t      SecurityPolicyID;
    const FnSymmetricEncrypt    pFnSymmEncrypt;
    const FnSymmetricDecrypt    pFnSymmDecrypt;
    const FnSymmetricSign       pFnSymmSign;
    const FnSymmetricVerify     pFnSymmVerif;
    const FnSymmetricGenKey     pFnSymmGenKey;
    const FnDerivePseudoRandomData  pFnDeriveData;
    const FnAsymmetricEncrypt   pFnAsymEncrypt;
    const FnAsymmetricDecrypt   pFnAsymDecrypt;
    const FnAsymmetricSign      pFnAsymSign;
    const FnAsymmetricVerify    pFnAsymVerify;
} CryptoProfile;


/* ------------------------------------------------------------------------------------------------
 * CryptoProvider
 * CryptolibContext are library-specific and defined in crypto_provider_lib
 * ------------------------------------------------------------------------------------------------
 */
typedef struct CryptoProvider
{
    const CryptoProfile * const pProfile;
    struct CryptolibContext *pCryptolibContext;
} CryptoProvider;


#endif // INGOPCS_CRYPTO_TYPES_H_

