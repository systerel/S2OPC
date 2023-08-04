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
 * \brief Defines the minimal PKI implementation provided by the stack.
 *
 * The stack will not to provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 */

#ifndef SOPC_PKI_STACK_H_
#define SOPC_PKI_STACK_H_

#include "sopc_crypto_provider.h"
#include "sopc_key_manager.h"
#include "sopc_pki.h"

/**
 * \brief           Creates the minimal validation implementation provided by the stack,
 *                  which fulfills the SOPC_PKIProvider interface.
 *
 *   This verifies the certificate in the safest manner (whole certificate chain, with date validation),
 *   with a single certificate authority, and its revocation list.
 *   Certificate authority requirements depend on the chosen OPC UA security policy.
 *   The CRL must be the CRL of the certificate authority.
 *
 * \warning         Provided certificates must be valid until the destruction of the created PKI (they are not copied).
 *
 * \param pCertAuth A valid pointer to the serialized certificate of the certification authority.
 *                  This object is internally copied, and only the internal copy is freed by SOPC_PKIProvider_Free.
 * \param pRevocationList  An certificate chain containing the revocation list of the certificate authority.
 *                  This object is borrowed and is freed by SOPC_PKIProvider_Free.
 * \param ppPKI     A valid pointer to the newly created SOPC_PKIProvider. You should free such provider with
 *                  SOPC_PKIProvider_Free().
 *
 * \note            Content of the pki is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            The pki is not safe to share across threads.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProviderStack_Create(SOPC_SerializedCertificate* pCertAuth,
                                               SOPC_CRLList* pRevocationList,
                                               SOPC_PKIProvider** ppPKI);

/**
 * \brief   Define whether the PKI is used for user or application certificates,
 *          The extensions verification for users:
 *              - The keyUsage is expected to be filled with digitalSignature.
 *              - The extendedKeyUsage is not checked.
 *              - The CA flag is expected to be FALSE
 *          The extensions verification for applications:
 *              - The keyUsage is expected to be filled with digitalSignature, nonRepudiation, keyEncipherment
 *                and dataEncipherment.
 *              - The extendedKeyUsage is filled with serverAuth and/or clientAuth.
 *
 * \note By default, the PKI is used for application instance certificates.
 *
 * \param pPKI   A valid pointer to the SOPC_PKIProvider.
 * \param bIsUserPki Define whether the PKI is used for user or application certificates.
 *
 * \return  SOPC_STATUS_OK when successful or SOPC_STATUS_INVALID_PARAMETERS when \p pPKI is NULL.
 *
 */
SOPC_ReturnStatus SOPC_PKIProviderStack_SetUserCert(SOPC_PKIProvider* pPKI, bool bIsUserPki);

/**
 * \brief           Creates a SOPC_PKIProviderStack using lists of paths.
 *
 *   This verifies the certificate in the safest manner (whole certificate chain, with date validation,
 *   mandatory certificate revocation lists).
 *   Certificate Authority (CA) requirements (such as the hash algorithm used for the signature)
 *   depend on the chosen OPC UA security policy.
 *
 *   There are 3 types of certificates to provide to the PKI:
 *   - The "trusted issuers" are CAs which issued certificates are also trusted.
 *     All the certificates of the signing chain including the root CA must be provided.
 *   - The "issued certificates" are certificates issued by untrusted CA.
 *     These certificates are considered trustworthy.
 *     The difference between trusted issuers and trusted issued certificates is that issued
 *     certificates are trusted on a one by one basis, whereas the trusted issuer may emit a large
 *     number of trusted certificates.
 *   - The "untrusted issuers" are CAs that are used to verify the signing chain of the
 *     "issued certificates". Each issued certificate must have its whole signing CA chain in the
 *     untrusted issuers or the trusted issuers up to the root CA.
 *
 *   In addition, there are two more concepts:
 *   - A link CA is part of the certificate validation chain.
 *     All links between a certificate and a root certificate must be provided.
 *   - A root CA is all always trusted, even if there are other root CAs that signed it.
 *     Hence the parent of root CAs will never be checked, and the validation stops on root CAs.
 *
 *   The list of Certificate Revocation List (CRL) must contain exactly one list for each CA of the
 *   provided CAs, either link or root, trusted or untrusted.
 *
 *   Issued certificates should not have CRLs, as they cannot be used to trust any other certificate.
 *   When an issued certificate is used to protect a Secure Channel, it's signing chain will be verified.
 *   For instance, if the certificate is not self signed and appears on the CRL of its signing CA,
 *   the connection will fail as the certificate is in fact invalid.
 *
 * \param lPathTrustedIssuerRoots   A pointer to an array of paths to root trusted issuers of the validation chains.
 *                  The array must contain a NULL pointer to indicate its end.
 *                  Each path is a zero-terminated relative path to the certificate from the current working directory.
 * \param lPathTrustedIssuerLinks   A pointer to an array of paths to intermediate certificate authorities.
 *                  This list contain only the trusted intermediate issuers.
 *                  This list must be ordered so that certificate signed by a parent must be present in the list
 *                  before its signing parent.
 *                  Each issued certificate must have its signing certificate chain in this list.
 * \param lPathUntrustedIssuerRoots A pointer to an array of paths to root untrusted issuers of the validation chains.
 *                  The array must contain a NULL pointer to indicate its end.
 *                  Each path is a zero-terminated relative path to the certificate from the current working directory.
 * \param lPathUntrustedIssuerLinks A pointer to an array of paths to intermediate certificate authorities.
 *                  This list contain only the untrusted intermediate issuers.
 *                  This list must be ordered so that certificate signed by a parent must be present in the list
 *                  before its signing parent.
 *                  Each issued certificate must have its signing certificate chain in this list.
 * \param lPathIssuedCerts  A pointer to an array of paths to issued certificates.
 *                  The array must contain a NULL pointer to indicate its end.
 *                  Each path is a zero-terminated relative path to the certificate from the current working directory.
 * \param lPathCRL  A pointer to an array of paths to each certificate revocation list to use.
 *                  Each CA of the trusted issuers list and the untrusted issuers list must have a CRL in the list.
 *                  The array must contain a NULL pointer to indicate its end.
 *                  Each path is a zero-terminated relative path to the CRL from the current working directory.
 * \param ppPKI     A valid pointer to the newly created PKIProvider. You should free such provider with
 *                  SOPC_PKIProvider_Free().
 *
 * \warning         The \p lPathTrustedIssuerLinks and \p lPathUntrustedIssuerLinks must be sorted:
 *                  certificates must be provided in the child ->  parent order.
 *                  In other words, there may be several chains provided in the list, but a signed certificate
 *                  must always be provided before the certificate that signed it.
 *
 * \note            Content of the pki is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \note            The pki is not safe to share across threads.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProviderStack_CreateFromPaths(char** lPathTrustedIssuerRoots,
                                                        char** lPathTrustedIssuerLinks,
                                                        char** lPathUntrustedIssuerRoots,
                                                        char** lPathUntrustedIssuerLinks,
                                                        char** lPathIssuedCerts,
                                                        char** lPathCRL,
                                                        SOPC_PKIProvider** ppPKI);

/* ****************************************************************** */
/* ************************* NEW API ******************************** */

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
       |
       ---- updatedTrustList
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

/*
TODO :
    - Replace the error codes SOPC_CertificateValidationXXXXX directly by OpcUa_BadCertificateXXXXX
    - Refactor S2OPC with the new API.
    - Add mutex in the new API.
    - Handle that the security level of the update is not higher than the security level of the endpoint
*/

/**
 * \brief The PKIProvider object for the Public Key Infrastructure.
 */
typedef struct SOPC_PKIProviderNew SOPC_PKIProviderNew;

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
    SOPC_PKI_KU_DISABLE_CHECK = 0x0000,
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
    SOPC_PKI_EKU_DISABLE_CHECK,
    SOPC_PKI_EKU_CLIENT_AUTH,
    SOPC_PKI_EKU_SERVER_AUTH,
} SOPC_PKI_ExtendedKeyUsage_Mask;

/**
 * \brief Type of PKI
 */
typedef enum
{
    SOPC_PKI_TYPE_CLIENT_APP, /**< Application client to validate server certificates */
    SOPC_PKI_TYPE_SERVER_APP, /**< Application server to validate server certificates */
    SOPC_PKI_TYPE_USER        /**< Application server to validate user certificates*/
} SOPC_PKI_Type;

/**
 * @struct SOPC_PKI_LeafProfile
 * @brief
 *   Structure containing the leaf certificate profile for validation
 *   with ::SOPC_PKIProviderNew_ValidateCertificate or with
 *   ::SOPC_PKIProviderNew_CheckLeafCertificate .
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
 *   If SOPC_PKI_KU_DISABLE_CHECK is set then the key usages are not checked.
 * @var SOPC_PKI_LeafProfile::extendedKeyUsage
 *   Defines the extended key usages mask of the certificates to validate.
 *   If SOPC_PKI_EKU_DISABLE_CHECK is set then the extended key usages are not checked.
 * @var SOPC_PKI_LeafProfile::sanApplicationUri
 *   The application URI to check in the subjectAltName field.
 *   If NULL is set then the URI is not checked.
 * @var SOPC_PKI_LeafProfile::sanURL
 *   The endpoint URL used for connection to check the HostName in the subjectAltName field.
 *   If NULL is set then the HostName is not checked.
 */
typedef struct SOPC_PKI_LeafProfile
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
} SOPC_PKI_LeafProfile;

/**
 * @struct SOPC_PKI_ChainProfile
 * @brief
 *   Structure containing the certificate chain profile for the validation
 *   with ::SOPC_PKIProviderNew_ValidateCertificate .
 * @var SOPC_PKI_ChainProfile::mdSign
 *   The message digest algorithm of the signature algorithm allowed.
 * @var SOPC_PKI_ChainProfile::pkAlgo
 *   The Public Key algorithm allowed.
 * @var SOPC_PKI_ChainProfile::curves
 *   The elliptic curves allowed.
 * @var SOPC_PKI_ChainProfile::RSAMinimumKeySize
 *   The minimum RSA key size allowed.
 */
typedef struct SOPC_PKI_ChainProfile
{
    SOPC_PKI_MdSign mdSign;
    SOPC_PKI_PkAlgo pkAlgo;
    SOPC_PKI_EllipticCurves curves;
    uint32_t RSAMinimumKeySize;
} SOPC_PKI_ChainProfile;

/**
 * @struct SOPC_PKI_Profile
 * @brief
 *   Structure containing the validation configuration
 * @var SOPC_PKI_Profile::leafProfile
 *   Validation configuration for the leaf certificate.
 *   The leaf certificate is verified with
 *   ::SOPC_PKIProviderNew_ValidateCertificate or ::SOPC_PKIProviderNew_CheckLeafCertificate .
 * @var SOPC_PKI_Profile::chainProfile
 *   Validation configuration for the chain. Each certificate properties in the chain are
 *   verified during ::SOPC_PKIProviderNew_ValidateCertificate .
 * @var SOPC_PKI_Profile::bBackwardInteroperability
 *   Defines if self-signed certificates whose basicConstraints CA flag
 *   set to True will be marked as root CA and as trusted certificates.
 * @var SOPC_PKI_Profile::bApplyLeafProfile
 *   Defines if the leaf properties is check during ::SOPC_PKIProviderNew_ValidateCertificate .
 */
typedef struct SOPC_PKI_Profile
{
    SOPC_PKI_LeafProfile* leafProfile;
    SOPC_PKI_ChainProfile* chainProfile;
    bool bBackwardInteroperability;
    bool bApplyLeafProfile;
} SOPC_PKI_Profile;

/**
 * \brief Creates the PKIProvider from a directory where certificates are stored.
 *
 * The directory store shall be organized as follows:
 *
 * - \<Directory_store_name\>/trusted/certs (.DER or .PEM files)
 * - \<Directory_store_name\>/trusted/crl (.DER or .PEM files)
 * - \<Directory_store_name\>/issuers/certs (.DER or .PEM files)
 * - \<Directory_store_name\>/issuers/crl (.DER or .PEM files
 *
 * Optional updated trust list directory (for runtime update persistence) :
 *
 * - \<Directory_store_name\>/updatedTrustList/trusted/certs (.DER or .PEM files)
 * - \<Directory_store_name\>/updatedTrustList/trusted/crl (.DER or .PEM files)
 * - \<Directory_store_name\>/updatedTrustList/issuers/certs (.DER or .PEM files)
 * - \<Directory_store_name\>/updatedTrustList/issuers/crl (.DER or .PEM files)
 *
 * The function attempts to build the PKI from the updatedTrustList directory
 * and in case of error (missing, empty or malformed), it switches to the root trusted and issuers directories.
 *
 * Notions :
 * - CA is a root CA if it is self-signed.
 * - trusted/certs = trusted root CA + trusted link CA + trusted cert.
 * - trusted/crl = CRLs of the trusted root CA + trusted link CA.
 * - issuer/certs = untrusted root CA + untrusted link CA.
 * - issuer/crl = CRLs of the untrusted root CA + untrusted link CA.
 * - CAs from trusted/certs and issuers/certs allow to verify the signing chain of a cert which is included into
 *   trusted/certs.
 * - CAs from trusted/certs allow to verify the signing chain of a cert which is not included into trusted/certs.
 *
 * This function checks that :
 * - the certificate store is not empty.
 * - at least one trusted certificate is provided.
 * - each certificate from subfolder issuer/certs is CA.
 * - each CA has exactly one Certificate Revocation List (CRL).
 *
 * \note Content of the PKI is NULL when return value is not SOPC_STATUS_OK.
 *
 * \param directoryStorePath The directory path where certificates are stored.
 * \param[out] ppPKI A valid pointer to the newly created PKIProvider. You should free such provider with
 *                   ::SOPC_PKIProviderNew_Free().
 *
 * \return  SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *          and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_CreateFromStore(const char* directoryStorePath, SOPC_PKIProviderNew** ppPKI);

/**
 * \brief Create the PKIProvider from list representation.
 *
 * Notions :
 * - CA is a root CA if it is self-signed.
 * - \p pTrustedCerts = trusted root CA + trusted link CA + trusted cert.
 * - \p pTrustedCrl = CRLs of the trusted root CA + trusted link CA.
 * - \p pIssuerCerts = untrusted root CA + untrusted link CA.
 * - \p pIssuerCrl = CRLs of the untrusted root CA + untrusted link CA.
 * - CAs from trusted/certs and issuers/certs allow to verify the signing chain of a cert which is included into
 *   trusted/certs.
 * - CAs from trusted/certs allow to verify the signing chain of a cert which is not included into trusted/certs.
 *
 * This function checks that :
 * - at least one cert from \p pTrustedCerts is provided.
 * - each certificate from \p pIssuerCerts is CA.
 * - each CA has exactly one Certificate Revocation List (CRL).
 *
 * \param pTrustedCerts A valid pointer to the trusted certificate list.
 * \param pTrustedCrl A valid pointer to the trusted CRL list.
 * \param pIssuerCerts A valid pointer to the issuer certificate list. NULL if not used.
 * \param pIssuerCrl A valid pointer to the issuer CRL list. NULL if not used.
 * \param[out] ppPKI A valid pointer to the newly created PKIProvider. You should free such provider with
 *                   ::SOPC_PKIProviderNew_Free().
 *
 * \return  SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *          and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_CreateFromList(SOPC_CertificateList* pTrustedCerts,
                                                     SOPC_CRLList* pTrustedCrl,
                                                     SOPC_CertificateList* pIssuerCerts,
                                                     SOPC_CRLList* pIssuerCrl,
                                                     SOPC_PKIProviderNew** ppPKI);

/**
 * \brief  Creates a PKI Provider without security.
 *
 * \param[out] ppPKI A valid pointer to the newly created PKIProvider. You should free such provider with
 *                   ::SOPC_PKIProviderNew_Free().
 * \warning  Using this PKI is considered harmful for the security of the connection.
 *           You can't export and update this PKI.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIPermissiveNew_Create(SOPC_PKIProviderNew** ppPKI);

/**
 * \brief Create a leaf certificate profile from security policy to check certificate properties.
 *
 *        KeyUsage, extendedKeyUsage, URI and HostName of subjectAltName are not configured here then
 *        these properties have to be defined manually or though specific functions eg
 *        ::SOPC_PKIProviderNew_LeafProfileSetUsageFromType , ::SOPC_PKIProviderNew_LeafProfileSetURI
 *        and ::SOPC_PKIProviderNew_LeafProfileSetURL
 *
 * \param securityPolicyUri The URI describing the security policy. If NULL then an empty profile is created.
 * \param[out] ppProfile The newly created leaf profile. You should delete it with
 * ::SOPC_PKIProviderNew_DeleteLeafProfile .
 *
 * \note If the profile is empty ( \p securityPolicyUri is NULL) then the functions that use this profile will not run
 * any checks.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_CreateLeafProfile(const char* securityPolicyUri,
                                                        SOPC_PKI_LeafProfile** ppProfile);

/**
 * \brief Set the keyUsage and extendedKeyUsage to the leaf profile from the PKI type.
 *
 *        For users : the keyUsage is expected to be filled with digitalSignature and the extendedKeyUsage is not
 *        checked.
 *        For clients : the keyUsage is expected to be filled with digitalSignature, nonRepudiation, keyEncipherment
 *        and dataEncipherment. The extendedKeyUsage is filled with serverAuth.
 *        For server : the keyUsage is expected to be filled with digitalSignature, nonRepudiation, keyEncipherment
 *        and dataEncipherment. The extendedKeyUsage is filled with clientAuth.
 *
 * \param pProfile A valid pointer to the leaf profile.
 * \param PKIType Defines the type of PKI (user, client or server)
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_LeafProfileSetUsageFromType(SOPC_PKI_LeafProfile* pProfile,
                                                                  SOPC_PKI_Type PKIType);

/**
 * \brief Set the application URI to the leaf profile.
 *
 * \param pProfile A valid pointer to the leaf profile.
 * \param applicationUri The application URI to set in \p pProfile .
 *
 * \warning If the application URI is already defined in \p pProfile , you can not define it again.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_LeafProfileSetURI(SOPC_PKI_LeafProfile* pProfile, const char* applicationUri);

/**
 * \brief Set the endpoint URL used for connection to the leaf profile.
 *
 * \param pProfile A valid pointer to the leaf profile.
 * \param url The endpoint URL used for connection to set in \p pProfile .
 *
 * \warning If the URL is already defined in \p pProfile , you can not define it again.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_LeafProfileSetURL(SOPC_PKI_LeafProfile* pProfile, const char* url);

/**
 * \brief Delete a leaf profile.
 *
 * \param ppProfile The leaf profile.
 */
void SOPC_PKIProviderNew_DeleteLeafProfile(SOPC_PKI_LeafProfile** ppProfile);

/**
 * \brief Create a PKI profile for a validation process.
 *        Backward interoperability is enabled.
 *        Leaf profile and chain profile are created according the security policy.
 *        KeyUsage, extendedKeyUsage, URI and HostName of subjectAltName are not configured here then
 *        these properties have to be defined manually or though specific functions eg
 *        ::SOPC_PKIProviderNew_ProfileSetUsageFromType , ::SOPC_PKIProviderNew_ProfileSetURI
 *        and ::SOPC_PKIProviderNew_ProfileSetURL
 *
 * \param securityPolicyUri The URI describing the security policy. Shall not be NULL.
 * \param[out] ppProfile The newly created profile. You should delete it with ::SOPC_PKIProviderNew_DeleteProfile .
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_CreateProfile(const char* securityPolicyUri, SOPC_PKI_Profile** ppProfile);

/**
 * \brief Set the properties to the PKI profile from the PKI type.
 *
 *        For users : the backward interoperability is disabled and the leaf profile will not be applied during
 *        ::SOPC_PKIProviderNew_ValidateCertificate.
 *        For clients : the keyUsage is expected to be filled with digitalSignature,
 *        nonRepudiation, keyEncipherment and dataEncipherment. The extendedKeyUsage is filled with serverAuth. Finally
 *        the backward interoperability is enabled.
 *        For Server : the keyUsage is expected to be filled with digitalSignature, nonRepudiation, keyEncipherment
 *        and dataEncipherment. The extendedKeyUsage is filled with clientAuth. Finally the backward interoperability
 *        is enabled.
 *
 * \param pProfile A valid pointer to the PKI profile.
 * \param PKIType Defines the type of PKI (user, client or server)
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_ProfileSetUsageFromType(SOPC_PKI_Profile* pProfile, SOPC_PKI_Type PKIType);

/**
 * \brief Set the application URI to the PKI profile.
 *
 * \param pProfile A valid pointer to the PKI profile.
 * \param applicationUri The application URI to set in \p pProfile .
 *
 * \warning If the application URI is already defined in \p pProfile , you can not define it again.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_ProfileSetURI(SOPC_PKI_Profile* pProfile, const char* applicationUri);

/**
 * \brief Set the endpoint URL used for connection to the PKI profile.
 *
 * \param pProfile A valid pointer to the PKI profile.
 * \param url The endpoint URL used for connection to set in \p pProfile .
 *
 * \warning If the URL is already defined in \p pProfile , you can not define it again.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_ProfileSetURL(SOPC_PKI_Profile* pProfile, const char* url);

/**
 * \brief Create a minimal PKI profile for user validation process.
 *
 * \param ppProfile The newly created profile. You should delete it with ::SOPC_PKIProviderNew_DeleteProfile .
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_CreateMinimalUserProfile(SOPC_PKI_Profile** ppProfile);

/**
 * \brief Delete a PKI profile.
 *
 * \param ppProfile The PKI profile.
 */
void SOPC_PKIProviderNew_DeleteProfile(SOPC_PKI_Profile** ppProfile);

/** \brief Validation function for a certificate with the PKI chain
 *
 *   It implements the validation with the certificate chain of the PKI.
 *
 * \param pPKI A valid pointer to the PKIProvider.
 * \param pToValidate A valid pointer to the Certificate to validate.
 * \param pProfile A valid pointer to the PKI profile.
 * \param[out] error Pointer to store the OpcUa error code when certificate validation failed.
 *
 * \note \p error is only set if returned status is different from SOPC_STATUS_OK.
 *
 * \warning In case of user PKI, the leaf profile part of \p pProfile is not applied to the certificate.
 *          The user leaf properties should be checked separately with ::SOPC_PKIProviderNew_CheckLeafCertificate .
 *
 * \return SOPC_STATUS_OK when the certificate is successfully validated, and
 *         SOPC_STATUS_INVALID_PARAMETERS or SOPC_STATUS_NOK.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_ValidateCertificate(const SOPC_PKIProviderNew* pPKI,
                                                          const SOPC_CertificateList* pToValidate,
                                                          const SOPC_PKI_Profile* pProfile,
                                                          uint32_t* error);

/** \brief Verify every certificate of the PKI
 *
 *   Each certificate of the chain is checked for signature, validity and profile.
 *
 * \param pPKI A valid pointer to the PKIProvider.
 * \param pProfile A valid pointer to the PKI chain profile.
 * \param[out] pErrors Array to store the OpcUa error code when a certificate is invalid.
 * \param[out] ppThumbprints Array to store the certificate thumbprint when a certificate is invalid.
 * \param[out] pLength The length of \p pErrors and \p ppThumbprints .
 *
 * \note \p pErrors and \p ppThumbprints are only created and set if the returned status is SOPC_STATUS_NOK.
 *       In case of invalid certificate (SOPC_STATUS_NOK) the thumbprint is associated to the error
 *       at the same index.
 *
 * \return SOPC_STATUS_OK when every certificate is successfully validated, and
 *         SOPC_STATUS_INVALID_PARAMETERS, SOPC_STATUS_INVALID_STATE, SOPC_STATUS_OUT_OF_MEMORY or SOPC_STATUS_NOK.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_VerifyEveryCertificate(const SOPC_PKIProviderNew* pPKI,
                                                             const SOPC_PKI_ChainProfile* pProfile,
                                                             uint32_t** pErrors,
                                                             char*** ppThumbprints,
                                                             uint32_t* pLength);

/** \brief Check leaf certificate properties
 *
 * \param pToValidate A valid pointer to the Certificate to validate.
 * \param pProfile A valid pointer to the leaf profile.
 * \param[out] error Pointer to store the OpcUa error code when certificate validation failed.
 *
 * \note \p error is only set if returned status is different from SOPC_STATUS_OK.
 *
 * \return SOPC_STATUS_OK when the certificate properties are successfully validated, and
 *         SOPC_STATUS_INVALID_PARAMETERS, SOPC_STATUS_INVALID_STATE or SOPC_STATUS_NOK.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_CheckLeafCertificate(const SOPC_CertificateList* pToValidate,
                                                           const SOPC_PKI_LeafProfile* pProfile,
                                                           uint32_t* error);

/** \brief Redefines the directory store where the certificates will be stored with ::SOPC_PKIProviderNew_WriteToStore
 *
 * \param directoryStorePath The directory path where the certificates will be stored.
 * \param pPKI A valid pointer to the PKIProvider.
 *
 * \note The directory is created if \p directoryStorePath does not exist.
 * \warning In case of error, \p pPKI is unchanged.
 *
 * \return SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS or SOPC_STATUS_NOK in case of error.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_SetStorePath(const char* directoryStorePath, SOPC_PKIProviderNew* pPKI);

/** \brief Write the certificate files in the updatedTrustList folder of the PKI storage.
 *         The updatedTrustList folder is created if it is missing.
 *         The format of the written files is DER.
 *         The updatedTrustList folder is organized as follows:
 *
 *         - updatedTrustList/trusted/certs
 *         - updatedTrustList/trusted/crl
 *         - updatedTrustList/issuers/certs
 *         - updatedTrustList/issuers/crl
 *
 * \param pPKI A valid pointer to the PKIProvider.
 * \param bEraseExistingFiles whether the existing files of the the trustList folder shall be deleted.
 *
 * \warning If the \p pPKI is built from lists ( ::SOPC_PKIProviderNew_CreateFromList ) then
 *          you shall define the directory store path with ::SOPC_PKIProviderNew_SetStorePath .
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_WriteToStore(const SOPC_PKIProviderNew* pPKI, const bool bEraseExistingFiles);

/** \brief Extracts certificates from the PKI object.
 *
 * \param pPKI A valid pointer to the PKIProvider.
 * \param ppTrustedCerts Write: a valid pointer to a copy of the trusted certificate list.
 *                       Append: a pointer to a pointer to a certificate list to which append the trusted certificate
 *                       list. In either cases, you should free this object.
 * \param ppTrustedCrl Write: a valid pointer to a copy of the trusted CRL list.
 *                     Append: a pointer to a pointer to a certificate list to which append the trusted CRL list.
 *                     In either cases, you should free this object.
 * \param ppIssuerCerts Write: a valid pointer to a copy of the issuer certificate list.
 *                      Append: a pointer to a pointer to a certificate list to which append the issuer certificate
 *                      list. In either cases, you should free this object.
 * \param ppIssuerCrl Write: a valid pointer to a copy of the issuer CRL list.
 *                    Append: a pointer to a pointer to a certificate list to which append the issuer CRL list.
 *                    In either cases, you should free this object.
 *
 * \note In case of error, the whole lists
 *       ( \p ppTrustedCerts , \p ppTrustedCrl , \p ppIssuerCerts and \p ppIssuerCrl ) are free and set to NULL.
 *
 * \note If the \p pPKI contains an empty list then nothing is write or append for this list.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_WriteOrAppendToList(const SOPC_PKIProviderNew* pPKI,
                                                          SOPC_CertificateList** ppTrustedCerts,
                                                          SOPC_CRLList** ppTrustedCrl,
                                                          SOPC_CertificateList** ppIssuerCerts,
                                                          SOPC_CRLList** ppIssuerCrl);

/** \brief Update the PKI with new lists of certificates and CRL.
 *
 * \param ppPKI A valid pointer to the PKIProvider.
 * \param securityPolicyUri The URI describing the security policy of the secure channel.
 * \param pTrustedCerts A valid pointer to the trusted certificate list. NULL if this part shall not updated.
 * \param pTrustedCrl A valid pointer to the trusted CRL list. NULL if this part shall not updated.
 * \param pIssuerCerts A valid pointer to the issuer certificate list. NULL if this part shall not updated.
 * \param pIssuerCrl A valid pointer to the issuer CRL list. NULL if this part shall not updated.
 * \param bIncludeExistingList whether the update shall includes the existing certificates of \p ppPKI plus
 *                             \p pTrustedCerts , \p pTrustedCrl , \p pIssuerCerts  and \p pIssuerCrl .
 *
 * \warning \p securityPolicyUri is not used yet and could be NULL.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProviderNew_UpdateFromList(SOPC_PKIProviderNew** ppPKI,
                                                     const char* securityPolicyUri,
                                                     SOPC_CertificateList* pTrustedCerts,
                                                     SOPC_CRLList* pTrustedCrl,
                                                     SOPC_CertificateList* pIssuerCerts,
                                                     SOPC_CRLList* pIssuerCrl,
                                                     const bool bIncludeExistingList);
/**
 * \brief Free a PKI provider.
 *
 * \param pPKI The PKI.
 */
void SOPC_PKIProviderNew_Free(SOPC_PKIProviderNew* pPKI);

#endif /* SOPC_PKI_STACK_H_ */
