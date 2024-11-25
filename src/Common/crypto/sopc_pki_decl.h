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
 * \brief Defines the common declarations for the PKI objects.
 * The structures and macros defined in this file are required before including
 * library-specific files (from lib_itf).
 *
 * Avoids the circular dependencies.
 */

#ifndef SOPC_CRYPTO_PKI_DECL_H_
#define SOPC_CRYPTO_PKI_DECL_H_

#include "sopc_crypto_decl.h"

/* The maximum number of rejected certificate stored by the PKI */
#ifndef SOPC_PKI_MAX_NB_CERT_REJECTED
#define SOPC_PKI_MAX_NB_CERT_REJECTED 10
#endif

/* The maximum number of trusted/issuer certificate/CRL stored by the PKI */
#ifndef SOPC_PKI_MAX_NB_CERT_AND_CRL
#define SOPC_PKI_MAX_NB_CERT_AND_CRL 50
#endif

/*
 The directory store shall be organized as follows:
  .
  |
  ---- <Directory_store_name>
       |
       |---- trusted
       |     |
       |     ---- certs
       |     ---- crl
       |---- issuers
       |     |
       |     ---- certs
       |     ---- crl
       |---- rejected
       |
       ---- (updatedTrustList) [automatically created for runtime update persistence]
       |    |
       |    ---- trusted
       |    |    |
       |    |    ---- certs
       |    |    ---- crl
       |    ---- issuers
       |         |
       |         ---- certs
       |         ---- crl
*/

/**
 * \brief  Message digests for signatures
 */
typedef enum
{
    SOPC_PKI_MD_SHA1,
    SOPC_PKI_MD_SHA256,
    SOPC_PKI_MD_SHA1_AND_SHA256,
    SOPC_PKI_MD_SHA1_OR_ABOVE,
    SOPC_PKI_MD_SHA256_OR_ABOVE,
} SOPC_PKI_MdSign;

/**
 * \brief Public key algorithms
 */
typedef enum
{
    SOPC_PKI_PK_ANY,
    SOPC_PKI_PK_RSA
} SOPC_PKI_PkAlgo;

/**
 * \brief Elliptic curves for ECDSA
 */
typedef enum
{
    SOPC_PKI_CURVES_ANY,
} SOPC_PKI_EllipticCurves;

/**
 * \brief Key usage
 */
typedef enum
{
    SOPC_PKI_KU_NONE = 0x0000,
    SOPC_PKI_KU_NON_REPUDIATION = 0x0001,
    SOPC_PKI_KU_DIGITAL_SIGNATURE = 0x0002,
    SOPC_PKI_KU_KEY_ENCIPHERMENT = 0x0004,
    SOPC_PKI_KU_KEY_DATA_ENCIPHERMENT = 0x0008,
    SOPC_PKI_KU_KEY_CERT_SIGN = 0x0010,
    SOPC_PKI_KU_KEY_CRL_SIGN = 0x00100
} SOPC_PKI_KeyUsage_Mask;

/**
 * \brief Extended Key usage
 */
typedef enum
{
    SOPC_PKI_EKU_NONE = 0x0000,
    SOPC_PKI_EKU_CLIENT_AUTH = 0x0001,
    SOPC_PKI_EKU_SERVER_AUTH = 0x0002,
} SOPC_PKI_ExtendedKeyUsage_Mask;

/**
 * \brief Type of PKI
 */
typedef enum
{
    SOPC_PKI_TYPE_CLIENT_APP, /**< Application client to validate server certificates */
    SOPC_PKI_TYPE_SERVER_APP, /**< Application server to validate client certificates */
    SOPC_PKI_TYPE_USER        /**< Application server to validate user certificates*/
} SOPC_PKI_Type;

/**
 * \brief Type of the callback triggered on PKI certificates update
 *
 * \param updateParam a user defined parameter for the callback
 */
typedef void SOPC_PKIProviderUpdateCb(uintptr_t updateParam);

typedef struct SOPC_PKIProvider SOPC_PKIProvider;
typedef struct SOPC_PKI_Profile SOPC_PKI_Profile;
typedef struct SOPC_PKI_ChainProfile SOPC_PKI_ChainProfile;
typedef struct SOPC_PKI_LeafProfile SOPC_PKI_LeafProfile;

typedef struct SOPC_PKI_Cert_Failure_Context
{
    char* invalidURI;
    char* invalidHostname;
} SOPC_PKI_Cert_Failure_Context;

typedef SOPC_ReturnStatus SOPC_FnValidateCert(SOPC_PKIProvider* pPKI,
                                              const SOPC_CertificateList* pToValidate,
                                              const SOPC_PKI_Profile* pProfile,
                                              uint32_t* error,
                                              SOPC_PKI_Cert_Failure_Context* context);

#endif /* SOPC_CRYPTO_PKI_DECL_H_ */
