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

/** \file sopc_pki_stack.h
 *
 * \brief Defines the minimal PKI implementation provided by the stack
 *
 * The stack will not to provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 * The stack provides a thread-safe PKI, it is necessary
 * for OPC UA client use case (shared between services and secure channel layers)
 * and PKI trust list update feature (shared between S2OPC library layers and possibly application thread).
 *
 * See sopc_pki_stack_lib_itf.h for API.
 */

#ifndef SOPC_PKI_STACK_H_
#define SOPC_PKI_STACK_H_

#include "sopc_assert.h"
#include "sopc_crypto_profiles.h"
#include "sopc_helper_string.h"
#include "sopc_key_manager_lib_itf.h"
#include "sopc_pki_stack_lib_itf.h"
#include "sopc_pki_struct_lib_internal.h"

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

// Copy newPKI content into currentPKI by preserving currentPKI mutex and then clear previous PKI content.
// Then frees the new PKI structure.
void SOPC_Internal_ReplacePKIAndClear(SOPC_PKIProvider* currentPKI, SOPC_PKIProvider** newPKI);

/**
 * \brief Frees allocated PKIs.
 *
 * \param ppPKI A valid pointer to a PKI.
 *
 */
void SOPC_PKIProvider_Free(SOPC_PKIProvider** ppPKI);

/**
 * \brief Gets the leaf profile from the security policy.
 *
 * \param uri a valid pointer to the URI of a security policy configuration.
 *
 * \return The leaf profile extracted from the security policy.
 *
 */
const SOPC_PKI_LeafProfile* SOPC_PKIProvider_GetLeafProfileFromSecurityPolicy(const char* uri);

/**
 * \brief Gets the chain profile from the security policy.
 *
 * \param uri A valid pointer to the URI of a security policy configuration.
 *
 * \return The chain profile extracted from the security policy.
 *
 */
const SOPC_PKI_ChainProfile* SOPC_PKIProvider_GetChainProfileFromSecurityPolicy(const char* uri);

#endif /* SOPC_PKI_STACK_H_ */

/**
 * \brief A function to validate the PKI when creating a PKI Provider without security with SOPC_PKIPermissive_Create.
 *
 * \param pPKI A valid pointer to a Public Key Infrastructure.
 *
 * \param pToValidate A valid pointer to a certificate list of certificates to validate.
 *
 * \param pProfile A valid pointer to a leaf profile.
 *
 * \param error A valid pointer to a variable.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_ValidateAnything(SOPC_PKIProvider* pPKI,
                                                    const SOPC_CertificateList* pToValidate,
                                                    const SOPC_PKI_Profile* pProfile,
                                                    uint32_t* error);

/**
 * \brief A function to merge 2 certificates.
 *
 * \param pLeft A valid pointer to a certificate to merge.
 *
 * \param pRight A valid pointer to a second certificate to merge.
 *
 * \param ppRes A valid pointer to the two merged certificates.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_MergeCertificates(SOPC_CertificateList* pLeft,
                                                     SOPC_CertificateList* pRight,
                                                     SOPC_CertificateList** ppRes);

/**
 * \brief Checks the number of certificates and CRLs of a PKI.
 *
 * \param pPKI A valid pointer to a PKI.
 * 
 * \param pTrustedCerts A valid pointer to a trusted certificate.
 *
 * \param pTrustedCrl A valid pointer to a trusted Certificate Revocation List (CRL).
 *
 * \param pIssuerCerts A valid pointer to an issuer certificate.
 *
 * \param pIssuerCrl A valid pointer to an issuer Certificate Revocation List (CRL).
 *
 * \param bIncludeExistingList A boolean value to include
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_CheckListLength(SOPC_PKIProvider* pPKI,
                                                   SOPC_CertificateList* pTrustedCerts,
                                                   SOPC_CRLList* pTrustedCrl,
                                                   SOPC_CertificateList* pIssuerCerts,
                                                   SOPC_CRLList* pIssuerCrl,
                                                   const bool bIncludeExistingList);

/**
 * \brief Handle that the security level of the update isn't higher than the
          security level of the secure channel. (§7.3.4 part 2 v1.05).
 *
 * \param pTrustedCerts A valid pointer to the trusted certificate list.

 * \param pTrustedCrl A valid pointer to the trusted CRL list.

 * \param pIssuerCerts A valid pointer to the issuer certificate list.

 * \param pIssuerCrl A valid pointer to the issuer CRL list.
 *
 * \param securityPolicyUri The URI describing the security policy of the secure channel.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_CheckSecurityLevelOfTheUpdate(const SOPC_CertificateList* pTrustedCerts,
                                                                 const SOPC_CRLList* pTrustedCrl,
                                                                 const SOPC_CertificateList* pIssuerCerts,
                                                                 const SOPC_CRLList* pIssuerCrl,
                                                                 const char* securityPolicyUri);

/**
 * \brief A function to merge 2 certificates.
 *
 * \param pLeft A valid pointer to a certificate to merge.
 *
 * \param pRight A valid pointer to a second certificate to merge.
 *
 * \param ppRes A valid pointer to the two merged certificates.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_MergeCRLs(SOPC_CRLList* pLeft, SOPC_CRLList* pRight, SOPC_CRLList** ppRes);

/**
 * \brief A function to remove certificates from their thumbprint.
 *
 * \param ppList A valid pointer to a certificate list.
 *
 * \param ppCRLList A valid pointer to a CRL list.
 *
 * \param pThumbprint A valid pointer to the thumbprint of a function.
 *
 * \param listName A valid pointer to the name of the list.
 *
 * \param pbIsRemoved A valid boolean pointer to indicate that a certificate has been removed.
 *
 * \param pbIsIssuer A valid boolean pointer indicating whether the deleted certificate is an issuer.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_RemoveCertByThumbprint(SOPC_CertificateList** ppList,
                                                          SOPC_CRLList** ppCRLList,
                                                          const char* pThumbprint,
                                                          const char* listName,
                                                          bool* pbIsRemoved,
                                                          bool* pbIsIssuer);

/**
 * \brief A function to load certificates and CRL lists from the store.
 *
 * \param basePath A valid pointer to a path where certificates can be found.
 *
 * \param ppTrustedCerts A valid pointer to a pointer to a list of trusted certificates.
 *
 * \param ppTrustedCrl A valid pointer to a pointer to CRL list.
 *
 * \param ppIssuerCerts A valid pointer to a pointer to issuer certificate list.
 *
 * \param ppIssuerCrl A valid pointer to a pointer to issuer CRL list.
 *
 * \param bDefaultBuild A boolean value read by the function load_certificate_or_crl_list(), to
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_LoadCertificateAndCRLListFromStore(const char* basePath,
                                                                      SOPC_CertificateList** ppTrustedCerts,
                                                                      SOPC_CRLList** ppTrustedCrl,
                                                                      SOPC_CertificateList** ppIssuerCerts,
                                                                      SOPC_CRLList** ppIssuerCrl,
                                                                      bool bDefaultBuild);

/**
 * \brief A function to obtain the length of certificates lists and CRL lists.
 *
 * \param pTrustedCerts A valid pointer to a list of trusted certificates.
 *
 * \param pTrustedCrl A valid pointer to a list of trusted CRLs.
 *
 * \param pIssuerCerts A valid pointer to a list of issuer certificates.
 *
 * \param pIssuerCrl A valid pointer to a list of issuer CRLs.
 *
 * \param listLength A valid pointer to contain the list length.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_GetListLength(const SOPC_CertificateList* pTrustedCerts,
                                                 const SOPC_CRLList* pTrustedCrl,
                                                 const SOPC_CertificateList* pIssuerCerts,
                                                 const SOPC_CRLList* pIssuerCrl,
                                                 uint32_t* listLength);

/**
 * \brief Checks if the trusted and issuer certificates chains are valid.
 *
 * \param pTrustedCerts a valid pointer to trusted certificates list.
 *
 * \param pIssuerCerts a valid pointer to issuer certificates list
 *
 * \param pTrustedCrl a valid pointer to trusted certificates revocation list.
 *
 * \param pIssuerCrl a valid pointer to issuer certificates revocation list.
 *
 * \param bTrustedCaFound a valid pointer to indicate if a certificate authority has been found for trusted certificates
 * list.
 *
 * \param bIssuerCaFound a valid pointer to indicate if a certificate authority has been found for issuer certificates
 * list.
 *
 * \return SOPC_STATUS_OK when successful.
 *
 */
SOPC_ReturnStatus SOPC_PKIProvider_CheckLists(SOPC_CertificateList* pTrustedCerts,
                                              SOPC_CertificateList* pIssuerCerts,
                                              SOPC_CRLList* pTrustedCrl,
                                              SOPC_CRLList* pIssuerCrl,
                                              bool* bTrustedCaFound,
                                              bool* bIssuerCaFound);
