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
 * \brief Defines the PKI structures used internally.
 *
 * \note Those structures are declared as common to all libraries
 *       but it might be changed to be specific in the future (only used by tests).
 */

#ifndef SOPC_PKI_STRUCT_LIB_INTERNAL_H_
#define SOPC_PKI_STRUCT_LIB_INTERNAL_H_

#include "sopc_crypto_decl.h"
#include "sopc_mutexes.h"
#include "sopc_pki_decl.h"

/**
 * @struct SOPC_PKI_LeafProfile
 * @brief
 *   Structure containing the leaf certificate profile for validation
 *   with ::SOPC_PKIProvider_ValidateCertificate or with
 *   ::SOPC_PKIProvider_CheckLeafCertificate .
 * @var SOPC_PKI_LeafProfile::mdSign
 *   The message digest algorithm of the signature algorithm allowed.
 * @var SOPC_PKI_LeafProfile::pkAlgo
 *   The Public Key algorithm allowed.
 * @var SOPC_PKI_LeafProfile::RSAMinimumKeySize
 *   The minimum RSA key size allowed.
 * @var SOPC_PKI_LeafProfile::RSAMaximumKeySize
 *   The maximum RSA key size allowed.
 * @var SOPC_PKI_LeafProfile::bApplySecurityPolicy
 *   Defines if mdSign, pkAlgo, RSAMinimumKeySize and RSAMaximumKeySize should be checked.
 * @var SOPC_PKI_LeafProfile::keyUsage
 *   Defines the key usages mask of the certificates to validate.
 * @var SOPC_PKI_LeafProfile::extendedKeyUsage
 *   Defines the extended key usages mask of the certificates to validate.
 * @var SOPC_PKI_LeafProfile::sanApplicationUri
 *   The application URI to check in the subjectAltName field.
 *   If NULL is set then the URI is not checked.
 * @var SOPC_PKI_LeafProfile::sanURL
 *   The endpoint URL used for connection to check the HostName in the subjectAltName field.
 *   If NULL is set then the HostName is not checked.
 */
struct SOPC_PKI_LeafProfile
{
    SOPC_PKI_MdSign mdSign;
    SOPC_PKI_PkAlgo pkAlgo;
    uint32_t RSAMinimumKeySize;
    uint32_t RSAMaximumKeySize;
    bool bApplySecurityPolicy;
    SOPC_PKI_KeyUsage_Mask keyUsage;
    SOPC_PKI_ExtendedKeyUsage_Mask extendedKeyUsage;
    char* sanApplicationUri; /* extension subjectAltName */
    char* sanURL;            /* extension subjectAltName */
};

/**
 * @struct SOPC_PKI_ChainProfile
 * @brief
 *   Structure containing the certificate chain profile for the validation
 *   with ::SOPC_PKIProvider_ValidateCertificate .
 * @var SOPC_PKI_ChainProfile::mdSign
 *   The message digest algorithm of the signature algorithm allowed.
 * @var SOPC_PKI_ChainProfile::pkAlgo
 *   The Public Key algorithm allowed.
 * @var SOPC_PKI_ChainProfile::curves
 *   The elliptic curves allowed.
 * @var SOPC_PKI_ChainProfile::RSAMinimumKeySize
 *   The minimum RSA key size allowed.
 * @var SOPC_PKI_ChainProfile::bDisableRevocationCheck
 *   When flag is set, no error is reported if a CA certificate has no revocation list.
 */
struct SOPC_PKI_ChainProfile
{
    SOPC_PKI_MdSign mdSign;
    SOPC_PKI_PkAlgo pkAlgo;
    SOPC_PKI_EllipticCurves curves;
    uint32_t RSAMinimumKeySize;
    bool bDisableRevocationCheck;
};

/**
 * @struct SOPC_PKI_Profile
 * @brief
 *   Structure containing the validation configuration
 * @var SOPC_PKI_Profile::leafProfile
 *   Validation configuration for the leaf certificate.
 *   The leaf certificate is verified with
 *   ::SOPC_PKIProvider_ValidateCertificate or ::SOPC_PKIProvider_CheckLeafCertificate .
 * @var SOPC_PKI_Profile::chainProfile
 *   Validation configuration for the chain. Each certificate properties in the chain are
 *   verified during ::SOPC_PKIProvider_ValidateCertificate .
 * @var SOPC_PKI_Profile::bBackwardInteroperability
 *   Defines if self-signed certificates whose basicConstraints CA flag
 *   set to True will be marked as root CA and as trusted certificates.
 * @var SOPC_PKI_Profile::bApplyLeafProfile
 *   Defines if the leaf properties is check during ::SOPC_PKIProvider_ValidateCertificate .
 */
struct SOPC_PKI_Profile
{
    SOPC_PKI_LeafProfile* leafProfile;
    SOPC_PKI_ChainProfile* chainProfile;
    bool bBackwardInteroperability;
    bool bApplyLeafProfile;
};

/**
 * \brief The PKIProvider object for the Public Key Infrastructure.
 */
struct SOPC_PKIProvider
{
    SOPC_Mutex mutex;                    /*!< The mutex used to have thread-safe accesses to PKI.
                                              IMPORTANT: it shall remain the first field for SOPC_Internal_ReplacePKIAndClear treatment */
    char* directoryStorePath;            /*!< The directory store path of the PKI*/
    SOPC_CertificateList* pTrustedCerts; /*!< Trusted intermediate CA + trusted certificates*/
    SOPC_CertificateList* pTrustedRoots; /*!< Trusted root CA*/
    SOPC_CRLList* pTrustedCrl;           /*!< CRLs of trusted intermediate CA and trusted root CA*/
    SOPC_CertificateList* pIssuerCerts;  /*!< Issuer intermediate CA*/
    SOPC_CertificateList* pIssuerRoots;  /*!< Issuer root CA*/
    SOPC_CRLList* pIssuerCrl;            /*!< CRLs of issuer intermediate CA and issuer root CA*/
    SOPC_CertificateList* pRejectedList; /*!< The list of Certificates that have been rejected */

    SOPC_CertificateList* pAllCerts;   /*!< Issuer certs + trusted certs (root not included)*/
    SOPC_CertificateList* pAllRoots;   /*!< Issuer roots + trusted roots*/
    SOPC_CertificateList* pAllTrusted; /*!< trusted root + trusted intermediate CAs + trusted certs */

    SOPC_CRLList* pAllCrl;                /*!< Issuer CRLs + trusted CRLs */
    SOPC_FnValidateCert* pFnValidateCert; /*!< Pointer to validation function*/
    bool isPermissive;                    /*!< Define whatever the PKI is permissive (without security)*/
};

#endif /* SOPC_PKI_STRUCT_LIB_INTERNAL_H_ */
