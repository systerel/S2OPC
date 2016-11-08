/*
 * Defines the cryptographic structures: CryptoProfiles and CryptoProviders.
 *
 *  Created on: Oct 04, 2016
 *      Author: PAB
 */


#ifndef INGOPCS_CRYPTO_TYPES_H_
#define INGOPCS_CRYPTO_TYPES_H_


#include "sopc_base_types.h"
#include "secret_buffer.h"

struct CryptoProvider;
struct CryptoProfile;
struct CryptolibContext;
struct AsymmetricKey;
struct Certificate;

/* ------------------------------------------------------------------------------------------------
 * CryptoProfile, internal API
 * ------------------------------------------------------------------------------------------------
 */
typedef SOPC_StatusCode (*FnSymmetricEncrypt) (const struct CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenPlainText,
                                          const ExposedBuffer *pKey,
                                          const ExposedBuffer *pIV,
                                          uint8_t *pOutput,
                                          uint32_t lenOutput);
typedef SOPC_StatusCode (*FnSymmetricDecrypt) (const struct CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenCipherText,
                                          const ExposedBuffer *pKey,
                                          const ExposedBuffer *pIV,
                                          uint8_t *pOutput,
                                          uint32_t lenOutput);
typedef SOPC_StatusCode (*FnSymmetricSign) (const struct CryptoProvider *pProvider,
                                       const uint8_t *pInput,
                                       uint32_t lenInput,
                                       const ExposedBuffer *pKey,
                                       uint8_t *pOutput);
typedef SOPC_StatusCode (*FnSymmetricVerify) (const struct CryptoProvider *pProvider,
                                         const uint8_t *pInput,
                                         uint32_t lenInput,
                                         const ExposedBuffer *pKey,
                                         const uint8_t *pSignature);
typedef SOPC_StatusCode (*FnSymmetricGenKey) (const struct CryptoProvider *pProvider,
                                         ExposedBuffer *pKey);
typedef SOPC_StatusCode (*FnDerivePseudoRandomData) (const struct CryptoProvider *pProvider,
                                                const ExposedBuffer *pSecret,
                                                uint32_t lenSecret,
                                                const ExposedBuffer *pSeed,
                                                uint32_t lenSeed,
                                                ExposedBuffer *pOutput,
                                                uint32_t lenOutput);
typedef SOPC_StatusCode (*FnAsymmetricEncrypt) (const struct CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenPlainText,
                                           const struct AsymmetricKey *pKey,
                                           uint8_t *pOutput);
typedef SOPC_StatusCode (*FnAsymmetricDecrypt) (const struct CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenCipherText,
                                           const struct AsymmetricKey *pKey,
                                           uint8_t *pOutput,
                                           uint32_t *lenWritten);
typedef SOPC_StatusCode (*FnAsymmetricSign) (const struct CryptoProvider *pProvider,
                                        const uint8_t *pInput,
                                        uint32_t lenInput,
                                        const struct AsymmetricKey *pKey,
                                        uint8_t *pSignature);
typedef SOPC_StatusCode (*FnAsymmetricVerify) (const struct CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenInput,
                                          const struct AsymmetricKey *pKey,
                                          const uint8_t *pSignature);
typedef SOPC_StatusCode (*FnCertificateVerify) (const struct CryptoProvider *pCrypto,
                                           const struct Certificate *pCert);

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
    const FnCertificateVerify   pFnCertVerify;
} CryptoProfile;


/* ------------------------------------------------------------------------------------------------
 * CryptoProvider
 * ------------------------------------------------------------------------------------------------
 */
/**
 * \brief   The CryptoProvider context.
 *
 * A pointer to a const CryptoProfile which should not be modified and contains pointers to the
 * cryptographic functions associated to a SecurityPolicy,
 * and a CryptolibContext, which are library-specific structures defined in crypto_provider_lib.h/c
 */
typedef struct CryptoProvider
{
    const CryptoProfile * const pProfile; /**< CryptoProfile associated to the chosen Security policy. You should not attempt to modify the content of this pointer. */
    struct CryptolibContext *pCryptolibContext; /**< A lib-specific context. This should not be accessed directly as its content may change depending on the chosen crypto-lib implementation. */
} CryptoProvider;


#endif // INGOPCS_CRYPTO_TYPES_H_

