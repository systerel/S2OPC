/*
 * Defines constants for the cryptographic profiles. CryptoProfiles are defined in crypto_types and
 *  are defined as struct of pointers. These immutable struct are extern and const, because they are
 *  lib-specific (hence CryptoProfile_Get and these variables are in different translation units).
 * The CryptoProfiles should be accessed through CryptoProfile_Get ONLY.
 *
 *  Created on: Sep 9, 2016
 *      Author: PAB
 */

#ifndef INGOPCS_CRYPTO_PROFILES_H_
#define INGOPCS_CRYPTO_PROFILES_H_


#include "crypto_types.h"


// API
const CryptoProfile * CryptoProfile_Get(const char *uri);

// Crypto profiles uri and ID
#define SecurityPolicy_Invalid_ID           0
#define SecurityPolicy_Basic256Sha256_URI   "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
#define SecurityPolicy_Basic256Sha256_ID    1

// Basic256Sha256, sizes in bytes
#define SecurityPolicy_Basic256Sha256_SymmLen_Block         16
#define SecurityPolicy_Basic256Sha256_SymmLen_Key           32
#define SecurityPolicy_Basic256Sha256_SymmLen_Signature     32

// CryptoProfiles instances
extern const CryptoProfile g_cpBasic256Sha256;


#endif  // INGOPCS_CRYPTO_PROFILES_H_
