/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/** \file
 *
 * \brief Defines the cryptographic profiles: constants and struct.
 *
 * \note The constants defined in this file are mainly based on the test specification (Part 7).
 */

#ifndef SOPC_CRYPTO_PROFILES_H_
#define SOPC_CRYPTO_PROFILES_H_

#include "sopc_crypto_decl.h"
#include "sopc_toolkit_constants.h"

// API
const SOPC_CryptoProfile* SOPC_CryptoProfile_Get(const char* uri);

// Crypto profiles uri and ID
#define SOPC_SecurityPolicy_Invalid_ID 0
#define SOPC_SecurityPolicy_Basic256Sha256_URI "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"
#define SOPC_SecurityPolicy_Basic256Sha256_ID 1
#define SOPC_SecurityPolicy_Basic256_URI "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
#define SOPC_SecurityPolicy_Basic256_ID 2
#define SOPC_SecurityPolicy_None_URI "http://opcfoundation.org/UA/SecurityPolicy#None"
#define SOPC_SecurityPolicy_None_ID 3

// Basic256Sha256, sizes in bytes
#define SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block 16
#define SOPC_SecurityPolicy_Basic256Sha256_SymmLen_CryptoKey 32
#define SOPC_SecurityPolicy_Basic256Sha256_SymmLen_SignKey 32
#define SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Signature 32
#define SOPC_SecurityPolicy_Basic256Sha256_CertLen_Thumbprint 20
#define SOPC_SecurityPolicy_Basic256Sha256_AsymLen_OAEP_Hash 20 /*< RSA OAEP uses SHA-1 */
#define SOPC_SecurityPolicy_Basic256Sha256_AsymLen_PSS_Hash \
    32 /*< RSASS PSS uses SHA-256 in this context (unused, Basic256Sha256 uses PKCS#1, not PSS) */
#define SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMinBits 2048
#define SOPC_SecurityPolicy_Basic256Sha256_AsymLen_KeyMaxBits 4096
#define SOPC_SecurityPolicy_Basic256Sha256_URI_SignAlgo "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256"

// Basic256, sizes in bytes
#define SOPC_SecurityPolicy_Basic256_SymmLen_Block 16
#define SOPC_SecurityPolicy_Basic256_SymmLen_CryptoKey 32
#define SOPC_SecurityPolicy_Basic256_SymmLen_SignKey 24
#define SOPC_SecurityPolicy_Basic256_SymmLen_Signature 20
#define SOPC_SecurityPolicy_Basic256_CertLen_Thumbprint 20
#define SOPC_SecurityPolicy_Basic256_AsymLen_OAEP_Hash 20 /*< RSA OAEP uses SHA-1 */
#define SOPC_SecurityPolicy_Basic256_AsymLen_PSS_Hash \
    20 /*< RSASS PSS uses SHA-1 in this context (unused, Basic256 uses PKCS#1, not PSS) */
#define SOPC_SecurityPolicy_Basic256_AsymLen_KeyMinBits 1024
#define SOPC_SecurityPolicy_Basic256_AsymLen_KeyMaxBits 2048
#define SOPC_SecurityPolicy_Basic256_URI_SignAlgo "http://www.w3.org/2000/09/xmldsig#rsa-sha1"

// CryptoProfiles instances
extern const SOPC_CryptoProfile sopc_g_cpBasic256Sha256;
extern const SOPC_CryptoProfile sopc_g_cpBasic256;
extern const SOPC_CryptoProfile sopc_g_cpNone;

#include "sopc_crypto_decl.h"
#include "sopc_secret_buffer.h"

/* ------------------------------------------------------------------------------------------------
 * Internal CryptoProfile function pointers.
 * ------------------------------------------------------------------------------------------------
 */
typedef SOPC_ReturnStatus (*FnSymmetricEncrypt)(const SOPC_CryptoProvider* pProvider,
                                                const uint8_t* pInput,
                                                uint32_t lenPlainText,
                                                const SOPC_ExposedBuffer* pKey,
                                                const SOPC_ExposedBuffer* pIV,
                                                uint8_t* pOutput,
                                                uint32_t lenOutput);
typedef SOPC_ReturnStatus (*FnSymmetricDecrypt)(const SOPC_CryptoProvider* pProvider,
                                                const uint8_t* pInput,
                                                uint32_t lenCipherText,
                                                const SOPC_ExposedBuffer* pKey,
                                                const SOPC_ExposedBuffer* pIV,
                                                uint8_t* pOutput,
                                                uint32_t lenOutput);
typedef SOPC_ReturnStatus (*FnSymmetricSign)(const SOPC_CryptoProvider* pProvider,
                                             const uint8_t* pInput,
                                             uint32_t lenInput,
                                             const SOPC_ExposedBuffer* pKey,
                                             uint8_t* pOutput);
typedef SOPC_ReturnStatus (*FnSymmetricVerify)(const SOPC_CryptoProvider* pProvider,
                                               const uint8_t* pInput,
                                               uint32_t lenInput,
                                               const SOPC_ExposedBuffer* pKey,
                                               const uint8_t* pSignature);
typedef SOPC_ReturnStatus (*FnGenerateRandom)(const SOPC_CryptoProvider* pProvider,
                                              SOPC_ExposedBuffer* pData,
                                              uint32_t lenData);
typedef SOPC_ReturnStatus (*FnDerivePseudoRandomData)(const SOPC_CryptoProvider* pProvider,
                                                      const SOPC_ExposedBuffer* pSecret,
                                                      uint32_t lenSecret,
                                                      const SOPC_ExposedBuffer* pSeed,
                                                      uint32_t lenSeed,
                                                      SOPC_ExposedBuffer* pOutput,
                                                      uint32_t lenOutput);
typedef SOPC_ReturnStatus (*FnAsymmetricEncrypt)(const SOPC_CryptoProvider* pProvider,
                                                 const uint8_t* pInput,
                                                 uint32_t lenPlainText,
                                                 const SOPC_AsymmetricKey* pKey,
                                                 uint8_t* pOutput);
typedef SOPC_ReturnStatus (*FnAsymmetricDecrypt)(const SOPC_CryptoProvider* pProvider,
                                                 const uint8_t* pInput,
                                                 uint32_t lenCipherText,
                                                 const SOPC_AsymmetricKey* pKey,
                                                 uint8_t* pOutput,
                                                 uint32_t* lenWritten);
typedef SOPC_ReturnStatus (*FnAsymmetricSign)(const SOPC_CryptoProvider* pProvider,
                                              const uint8_t* pInput,
                                              uint32_t lenInput,
                                              const SOPC_AsymmetricKey* pKey,
                                              uint8_t* pSignature);
typedef SOPC_ReturnStatus (*FnAsymmetricVerify)(const SOPC_CryptoProvider* pProvider,
                                                const uint8_t* pInput,
                                                uint32_t lenInput,
                                                const SOPC_AsymmetricKey* pKey,
                                                const uint8_t* pSignature);
typedef SOPC_ReturnStatus (*FnCertificateVerify)(const SOPC_CryptoProvider* pCrypto, const SOPC_Certificate* pCert);

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
struct SOPC_CryptoProfile
{
    const uint32_t SecurityPolicyID;
    const FnSymmetricEncrypt pFnSymmEncrypt;
    const FnSymmetricDecrypt pFnSymmDecrypt;
    const FnSymmetricSign pFnSymmSign;
    const FnSymmetricVerify pFnSymmVerif;
    const FnGenerateRandom pFnGenRnd;
    const FnDerivePseudoRandomData pFnDeriveData;
    const FnAsymmetricEncrypt pFnAsymEncrypt;
    const FnAsymmetricDecrypt pFnAsymDecrypt;
    const FnAsymmetricSign pFnAsymSign;
    const FnAsymmetricVerify pFnAsymVerify;
    const FnCertificateVerify pFnCertVerify;
};

#endif /* SOPC_CRYPTO_PROFILES_H_ */
