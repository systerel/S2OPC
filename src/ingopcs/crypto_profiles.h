/*
 * Defines the cryptographic profiles: the set of functions associated to each cryptographic profiles.
 * These profiles are defined as struct of pointers. These immutable struct are extern and const, so that a profile
 * could not be modified before execution time.
 *
 *  Created on: Sep 9, 2016
 *      Author: PAB
 */

#ifndef INGOPCS_CRYPTO_PROFILES_H_
#define INGOPCS_CRYPTO_PROFILES_H_


#include <inttypes.h>
#include "ua_builtintypes.h"  // StatusCode
#include "private_key.h"
#include "crypto_provider.h"


// Crypto profiles uri and ID
#define SecurityPolicy_Invalid_ID           0
#define SecurityPolicy_Basic256Sha256_URI   "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
#define SecurityPolicy_Basic256Sha256_ID    1

// Basic256Sha256, sizes in bytes
#define SecurityPolicy_Basic256Sha256_SymmLen_Block         16
#define SecurityPolicy_Basic256Sha256_SymmLen_Key           32
#define SecurityPolicy_Basic256Sha256_SymmLen_Signature     32


typedef uint8_t KeyBuffer;  // TODO: move towards private_key.h + This definition is unclear

struct CryptoProvider; // TODO: circular dependency CryptoProvider <-> CryptoProfile

// TODO: move these somewhere?
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

typedef struct CryptoProfile
{
    const uint32_t      SecurityPolicyID;
    const uint32_t      DerivedSignatureKeyBitLength;
    const uint32_t      MinAsymmetricKeyBitLength;
    const uint32_t      MaxAsymmetricKeyBitLength;
    const FnSymmetricEncrypt    pFnSymmEncrypt;
    const FnSymmetricDecrypt    pFnSymmDecrypt;
    const FnSymmetricSign       pFnSymmSign;
    const FnSymmetricVerify     pFnSymmVerif;
    const FnSymmetricGenKey     pFnSymmGenKey;
} CryptoProfile;


const CryptoProfile * CryptoProfile_Get(const uint8_t *uri);

extern const CryptoProfile g_cpBasic256Sha256;

#endif  // INGOPCS_CRYPTO_PROFILES_H_
