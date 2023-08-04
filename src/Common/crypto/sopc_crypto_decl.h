/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
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
 * \brief Defines the common declarations for the cryptographic objects.
 * The structures and macros defined in this file are required before including
 * library-specific files (from lib_itf)
 *
 * Avoids the circular dependencies.
 */

#ifndef SOPC_CRYPTO_DECL_H_
#define SOPC_CRYPTO_DECL_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_buffer.h"
#include "sopc_secret_buffer.h"

typedef struct SOPC_CryptoProvider SOPC_CryptoProvider;
typedef struct SOPC_CryptoProfile SOPC_CryptoProfile;
typedef struct SOPC_CryptoProfile_PubSub SOPC_CryptoProfile_PubSub;

// This section contains structs that required specific definition from implementation
typedef struct SOPC_CryptolibContext SOPC_CryptolibContext;
typedef struct SOPC_AsymmetricKey SOPC_AsymmetricKey;
typedef struct SOPC_CertificateList SOPC_CertificateList;
typedef struct SOPC_CRLList SOPC_CRLList;
typedef struct SOPC_CSR SOPC_CSR;

#define SOPC_CertificateValidationError_Invalid 0x80120000
#define SOPC_CertificateValidationError_PolicyCheckFailed 0x81140000
#define SOPC_CertificateValidationError_TimeInvalid 0x80140000
#define SOPC_CertificateValidationError_IssuerTimeInvalid 0x80150000
#define SOPC_CertificateValidationError_HostNameInvalid 0x80160000
#define SOPC_CertificateValidationError_UriInvalid 0x80170000
#define SOPC_CertificateValidationError_UseNotAllowed 0x80180000
#define SOPC_CertificateValidationError_IssuerUseNotAllowed 0x80190000
#define SOPC_CertificateValidationError_Untrusted 0x801A0000
#define SOPC_CertificateValidationError_RevocationUnknown 0x801B0000
#define SOPC_CertificateValidationError_IssuerRevocationUnknown 0x801C0000
#define SOPC_CertificateValidationError_Revoked 0x801D0000
#define SOPC_CertificateValidationError_IssuerRevoked 0x801E0000
#define SOPC_CertificateValidationError_ChainIncomplete 0x810D0000
#define SOPC_CertificateValidationError_Unknown 0x80000000

/**
 * \brief A serialized representation of an asymmetric key.
 *
 * This representation is safe to share across threads.
 */
typedef SOPC_SecretBuffer SOPC_SerializedAsymmetricKey;

/**
 * \brief A serialized representation of a certificate in DER format.
 *
 * This representation is safe to share across threads.
 */
typedef SOPC_Buffer SOPC_SerializedCertificate;

/**
 * \brief A serialized representation of a CRL.
 *
 * This representation is safe to share across threads.
 */
typedef SOPC_Buffer SOPC_SerializedCRL;

#endif /* SOPC_CRYPTO_DECL_H_ */
