/** \file
 *
 * \brief Defines the cryptographic profiles: constants and struct.
 *
 * \note The constants defined in this file are mainly based on the test specification (Part 7).
 */
/*
 *  Copyright (C) 2016 Systerel and others.
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

#ifndef SOPC_CRYPTO_PROFILES_H_
#define SOPC_CRYPTO_PROFILES_H_


#include "sopc_crypto_decl.h"
#include "sopc_toolkit_constants.h"

// API
const SOPC_CryptoProfile * SOPC_CryptoProfile_Get(const char *uri);

// Crypto profiles uri and ID
#define SOPC_SecurityPolicy_Invalid_ID           0
#define SOPC_SecurityPolicy_Basic256Sha256_URI   "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
#define SOPC_SecurityPolicy_Basic256Sha256_ID    1
#define SOPC_SecurityPolicy_Basic256_URI         "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
#define SOPC_SecurityPolicy_Basic256_ID          2
#define SOPC_SecurityPolicy_None_URI             "http://opcfoundation.org/UA/SecurityPolicy#None"
#define SOPC_SecurityPolicy_None_ID              3


// Basic256Sha256, sizes in bytes
#define SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block         16
#define SOPC_SecurityPolicy_Basic256Sha256_SymmLen_CryptoKey     32
#define SOPC_SecurityPolicy_Basic256Sha256_SymmLen_SignKey       32
#define SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Signature     32
#define SOPC_SecurityPolicy_Basic256Sha256_CertLen_Thumbprint    20
#define SOPC_SecurityPolicy_Basic256Sha256_AsymLen_OAEP_Hash     20 /*< RSA OAEP uses SHA-1 */
#define SOPC_SecurityPolicy_Basic256Sha256_AsymLen_PSS_Hash      32 /*< RSASS PSS uses SHA-256 in this context (unused, Basic256Sha256 uses PKCS#1, not PSS) */
#define SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits    2048
#define SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits    4096
#define SOPC_SecurityPolicy_Basic256Sha256_URI_SignAlgo          "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256"

// Basic256, sizes in bytes
#define SOPC_SecurityPolicy_Basic256_SymmLen_Block               16
#define SOPC_SecurityPolicy_Basic256_SymmLen_CryptoKey           32
#define SOPC_SecurityPolicy_Basic256_SymmLen_SignKey             24
#define SOPC_SecurityPolicy_Basic256_SymmLen_Signature           20
#define SOPC_SecurityPolicy_Basic256_CertLen_Thumbprint          20
#define SOPC_SecurityPolicy_Basic256_AsymLen_OAEP_Hash           20 /*< RSA OAEP uses SHA-1 */
#define SOPC_SecurityPolicy_Basic256_AsymLen_PSS_Hash            20 /*< RSASS PSS uses SHA-1 in this context (unused, Basic256 uses PKCS#1, not PSS) */
#define SOPC_SecurityPolicy_Basic256_AsymLen_KeyMinBits          1024
#define SOPC_SecurityPolicy_Basic256_AsymLen_KeyMaxBits          2048
#define SOPC_SecurityPolicy_Basic256_URI_SignAlgo                "http://www.w3.org/2000/09/xmldsig#rsa-sha1"

// CryptoProfiles instances
extern const SOPC_CryptoProfile sopc_g_cpBasic256Sha256;
extern const SOPC_CryptoProfile sopc_g_cpBasic256;
extern const SOPC_CryptoProfile sopc_g_cpNone;


#include "sopc_secret_buffer.h"
#include "sopc_crypto_decl.h"


/* ------------------------------------------------------------------------------------------------
 * Internal CryptoProfile function pointers.
 * ------------------------------------------------------------------------------------------------
 */
typedef SOPC_StatusCode (*FnSymmetricEncrypt) (const SOPC_CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenPlainText,
                                          const SOPC_ExposedBuffer *pKey,
                                          const SOPC_ExposedBuffer *pIV,
                                          uint8_t *pOutput,
                                          uint32_t lenOutput);
typedef SOPC_StatusCode (*FnSymmetricDecrypt) (const SOPC_CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenCipherText,
                                          const SOPC_ExposedBuffer *pKey,
                                          const SOPC_ExposedBuffer *pIV,
                                          uint8_t *pOutput,
                                          uint32_t lenOutput);
typedef SOPC_StatusCode (*FnSymmetricSign) (const SOPC_CryptoProvider *pProvider,
                                       const uint8_t *pInput,
                                       uint32_t lenInput,
                                       const SOPC_ExposedBuffer *pKey,
                                       uint8_t *pOutput);
typedef SOPC_StatusCode (*FnSymmetricVerify) (const SOPC_CryptoProvider *pProvider,
                                         const uint8_t *pInput,
                                         uint32_t lenInput,
                                         const SOPC_ExposedBuffer *pKey,
                                         const uint8_t *pSignature);
typedef SOPC_StatusCode (*FnGenerateRandom) (const SOPC_CryptoProvider *pProvider,
                                             SOPC_ExposedBuffer *pData,
                                             uint32_t lenData);
typedef SOPC_StatusCode (*FnDerivePseudoRandomData) (const SOPC_CryptoProvider *pProvider,
                                                const SOPC_ExposedBuffer *pSecret,
                                                uint32_t lenSecret,
                                                const SOPC_ExposedBuffer *pSeed,
                                                uint32_t lenSeed,
                                                SOPC_ExposedBuffer *pOutput,
                                                uint32_t lenOutput);
typedef SOPC_StatusCode (*FnAsymmetricEncrypt) (const SOPC_CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenPlainText,
                                           const SOPC_AsymmetricKey *pKey,
                                           uint8_t *pOutput);
typedef SOPC_StatusCode (*FnAsymmetricDecrypt) (const SOPC_CryptoProvider *pProvider,
                                           const uint8_t *pInput,
                                           uint32_t lenCipherText,
                                           const SOPC_AsymmetricKey *pKey,
                                           uint8_t *pOutput,
                                           uint32_t *lenWritten);
typedef SOPC_StatusCode (*FnAsymmetricSign) (const SOPC_CryptoProvider *pProvider,
                                        const uint8_t *pInput,
                                        uint32_t lenInput,
                                        const SOPC_AsymmetricKey *pKey,
                                        uint8_t *pSignature);
typedef SOPC_StatusCode (*FnAsymmetricVerify) (const SOPC_CryptoProvider *pProvider,
                                          const uint8_t *pInput,
                                          uint32_t lenInput,
                                          const SOPC_AsymmetricKey *pKey,
                                          const uint8_t *pSignature);
typedef SOPC_StatusCode (*FnCertificateVerify) (const SOPC_CryptoProvider *pCrypto,
                                           const SOPC_Certificate *pCert);



/* ------------------------------------------------------------------------------------------------
 * The CryptoProfile definition
 * ------------------------------------------------------------------------------------------------
 */
/**
 * \brief   CryptoProfiles gather pointers to cryptographic functions associated to a security policy.
 *
 * CryptoProfiles are defined as struct of pointers. These immutable struct are extern and const, because they are
 * lib-specific (hence CryptoProfile_Get and these variables are in different translation units).
 * The CryptoProfiles should be accessed through CryptoProfile_Get ONLY, and should not be modified,
 * as multiple calls to CryptoProfile_Get returns the same instances.
 */
struct SOPC_CryptoProfile {
    const uint32_t                  SecurityPolicyID;
    const FnSymmetricEncrypt        pFnSymmEncrypt;
    const FnSymmetricDecrypt        pFnSymmDecrypt;
    const FnSymmetricSign           pFnSymmSign;
    const FnSymmetricVerify         pFnSymmVerif;
    const FnGenerateRandom          pFnGenRnd;
    const FnDerivePseudoRandomData  pFnDeriveData;
    const FnAsymmetricEncrypt       pFnAsymEncrypt;
    const FnAsymmetricDecrypt       pFnAsymDecrypt;
    const FnAsymmetricSign          pFnAsymSign;
    const FnAsymmetricVerify        pFnAsymVerify;
    const FnCertificateVerify       pFnCertVerify;
};


#endif  /* SOPC_CRYPTO_PROFILES_H_ */
