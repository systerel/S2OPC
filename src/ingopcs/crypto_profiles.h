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

struct CryptoProfile;

struct CryptoProvider;  // TODO: move to crypto_provider.h
typedef uint8_t KeyBuffer;  // TODO: move towards private_key.h

struct CryptoProfile
{
    const uint32_t      DerivedSignatureKeyBitLength;
    const uint32_t      MinAsymmetricKeyBitLength;
    const uint32_t      MaxAsymmetricKeyBitLength;
    StatusCode (*const SymmetricEncrypt) (const struct CryptoProvider *,
                                          const uint8_t *,
                                          uint32_t,
                                          const KeyBuffer *,
                                          const uint8_t *,
                                          uint8_t *,
                                          uint32_t);
};

#endif  // INGOPCS_CRYPTO_PROFILES_H_
