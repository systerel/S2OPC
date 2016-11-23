/** \file
 *
 * \brief Defines the cryptographic profiles: constants and struct.
 *
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


#include "crypto_decl.h"

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
#define SecurityPolicy_Basic256Sha256_CertLen_Thumbprint    20
#define SecurityPolicy_Basic256Sha256_AsymLen_OAEP_Hash     20 /**< RSA OAEP uses SHA-1 */
#define SecurityPolicy_Basic256Sha256_AsymLen_PSS_Hash      32 /**< RSASS PSS uses SHA-256 */
#define SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits    2048
#define SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits    4096

// CryptoProfiles instances
extern const CryptoProfile g_cpBasic256Sha256;


#include "sopc_base_types.h"
#include "secret_buffer.h"
#include "crypto_decl.h"


/* ------------------------------------------------------------------------------------------------
 * Internal CryptoProfile function pointers.
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
typedef SOPC_StatusCode (*FnGenerateRandom) (const struct CryptoProvider *pProvider,
                                             ExposedBuffer *pData,
                                             uint32_t lenData);
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
struct CryptoProfile {
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
