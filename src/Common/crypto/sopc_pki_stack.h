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

#include "sopc_key_manager.h"

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
    - Add mutex and do not direct access to the PKI until it is there (update, write, set operations)
    - Handle that the security level of the update is not higher than the security level of the endpoint
      (The following issue has been SUBMITTED : https://mantis.opcfoundation.org/view.php?id=8976)
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
 *   with ::SOPC_PKIProvider_ValidateCertificate .
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
 *   ::SOPC_PKIProvider_ValidateCertificate or ::SOPC_PKIProvider_CheckLeafCertificate .
 * @var SOPC_PKI_Profile::chainProfile
 *   Validation configuration for the chain. Each certificate properties in the chain are
 *   verified during ::SOPC_PKIProvider_ValidateCertificate .
 * @var SOPC_PKI_Profile::bBackwardInteroperability
 *   Defines if self-signed certificates whose basicConstraints CA flag
 *   set to True will be marked as root CA and as trusted certificates.
 * @var SOPC_PKI_Profile::bApplyLeafProfile
 *   Defines if the leaf properties is check during ::SOPC_PKIProvider_ValidateCertificate .
 * @var SOPC_PKI_Profile::bAppendRejectCert
 *   Defines if the certificates rejected during ::SOPC_PKIProvider_ValidateCertificate shall be stored by the PKI.
 */
typedef struct SOPC_PKI_Profile
{
    SOPC_PKI_LeafProfile* leafProfile;
    SOPC_PKI_ChainProfile* chainProfile;
    bool bBackwardInteroperability;
    bool bApplyLeafProfile;
    bool bAppendRejectCert;
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
 * - the number of certificates plus CRLs does not exceed \c SOPC_PKI_MAX_NB_CERT_AND_CRL .
 * - the certificate store is not empty.
 * - at least one trusted certificate is provided.
 * - each certificate from subfolder issuer/certs is CA.
 * - each CA has exactly one Certificate Revocation List (CRL).
 *
 * \note Content of the PKI is NULL when return value is not SOPC_STATUS_OK.
 *
 * \param directoryStorePath The directory path where certificates are stored.
 * \param[out] ppPKI A valid pointer to the newly created PKIProvider. You should free such provider with
 *                   ::SOPC_PKIProvider_Free().
 *
 * \return  SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *          and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProvider_CreateFromStore(const char* directoryStorePath, SOPC_PKIProvider** ppPKI);

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
 * - the number of certificates plus CRLs does not exceed \c SOPC_PKI_MAX_NB_CERT_AND_CRL .
 * - at least one cert from \p pTrustedCerts is provided.
 * - each certificate from \p pIssuerCerts is CA.
 * - each CA has exactly one Certificate Revocation List (CRL).
 *
 * \param pTrustedCerts A valid pointer to the trusted certificate list.
 * \param pTrustedCrl A valid pointer to the trusted CRL list.
 * \param pIssuerCerts A valid pointer to the issuer certificate list. NULL if not used.
 * \param pIssuerCrl A valid pointer to the issuer CRL list. NULL if not used.
 * \param[out] ppPKI A valid pointer to the newly created PKIProvider. You should free such provider with
 *                   ::SOPC_PKIProvider_Free().
 *
 * \return  SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *          and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus SOPC_PKIProvider_CreateFromList(SOPC_CertificateList* pTrustedCerts,
                                                  SOPC_CRLList* pTrustedCrl,
                                                  SOPC_CertificateList* pIssuerCerts,
                                                  SOPC_CRLList* pIssuerCrl,
                                                  SOPC_PKIProvider** ppPKI);

/**
 * \brief  Creates a PKI Provider without security.
 *
 * \param[out] ppPKI A valid pointer to the newly created PKIProvider. You should free such provider with
 *                   ::SOPC_PKIProvider_Free().
 * \warning  Using this PKI is considered harmful for the security of the connection.
 *           You can't export and update this PKI, it shall be used only for test.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIPermissive_Create(SOPC_PKIProvider** ppPKI);

/**
 * \brief Create a leaf certificate profile from security policy to check certificate properties.
 *
 *        KeyUsage, extendedKeyUsage, URI and HostName of subjectAltName are not configured here then
 *        these properties have to be defined manually or though specific functions eg
 *        ::SOPC_PKIProvider_LeafProfileSetUsageFromType , ::SOPC_PKIProvider_LeafProfileSetURI
 *        and ::SOPC_PKIProvider_LeafProfileSetURL
 *
 * \param securityPolicyUri The URI describing the security policy. If NULL then an empty profile is created.
 * \param[out] ppProfile The newly created leaf profile. You should delete it with
 * ::SOPC_PKIProvider_DeleteLeafProfile .
 *
 * \note If the profile is empty ( \p securityPolicyUri is NULL) then the functions that use this profile will not run
 * any checks.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProvider_CreateLeafProfile(const char* securityPolicyUri, SOPC_PKI_LeafProfile** ppProfile);

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
SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetUsageFromType(SOPC_PKI_LeafProfile* pProfile, SOPC_PKI_Type PKIType);

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
SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetURI(SOPC_PKI_LeafProfile* pProfile, const char* applicationUri);

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
SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetURL(SOPC_PKI_LeafProfile* pProfile, const char* url);

/**
 * \brief Delete a leaf profile.
 *
 * \param ppProfile The leaf profile.
 */
void SOPC_PKIProvider_DeleteLeafProfile(SOPC_PKI_LeafProfile** ppProfile);

/**
 * \brief Create a PKI profile for a validation process.
 *        Backward interoperability is enabled.
 *        Leaf profile and chain profile are created according the security policy.
 *        KeyUsage, extendedKeyUsage, URI and HostName of subjectAltName are not configured here then
 *        these properties have to be defined manually or though specific functions eg
 *        ::SOPC_PKIProvider_ProfileSetUsageFromType , ::SOPC_PKIProvider_ProfileSetURI
 *        and ::SOPC_PKIProvider_ProfileSetURL
 *
 * \param securityPolicyUri The URI describing the security policy. Shall not be NULL.
 * \param[out] ppProfile The newly created profile. You should delete it with ::SOPC_PKIProvider_DeleteProfile .
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProvider_CreateProfile(const char* securityPolicyUri, SOPC_PKI_Profile** ppProfile);

/**
 * \brief Set the properties to the PKI profile from the PKI type.
 *
 *        For users : the backward interoperability is disabled and the leaf profile will not be applied during
 *        ::SOPC_PKIProvider_ValidateCertificate.
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
SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetUsageFromType(SOPC_PKI_Profile* pProfile, SOPC_PKI_Type PKIType);

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
SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetURI(SOPC_PKI_Profile* pProfile, const char* applicationUri);

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
SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetURL(SOPC_PKI_Profile* pProfile, const char* url);

/**
 * \brief Create a minimal PKI profile for user validation process.
 *
 * \param ppProfile The newly created profile. You should delete it with ::SOPC_PKIProvider_DeleteProfile .
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProvider_CreateMinimalUserProfile(SOPC_PKI_Profile** ppProfile);

/**
 * \brief Delete a PKI profile.
 *
 * \param ppProfile The PKI profile.
 */
void SOPC_PKIProvider_DeleteProfile(SOPC_PKI_Profile** ppProfile);

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
 *       The certificate is internally stored if it is rejected.
 *
 * \warning In case of user PKI, the leaf profile part of \p pProfile is not applied to the certificate.
 *          The user leaf properties should be checked separately with ::SOPC_PKIProvider_CheckLeafCertificate .
 *
 * \return SOPC_STATUS_OK when the certificate is successfully validated, and
 *         SOPC_STATUS_INVALID_PARAMETERS or SOPC_STATUS_NOK.
 */
SOPC_ReturnStatus SOPC_PKIProvider_ValidateCertificate(const SOPC_PKIProvider* pPKI,
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
SOPC_ReturnStatus SOPC_PKIProvider_VerifyEveryCertificate(const SOPC_PKIProvider* pPKI,
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
SOPC_ReturnStatus SOPC_PKIProvider_CheckLeafCertificate(const SOPC_CertificateList* pToValidate,
                                                        const SOPC_PKI_LeafProfile* pProfile,
                                                        uint32_t* error);

/** \brief Redefines the directory store where the certificates will be stored with ::SOPC_PKIProvider_WriteToStore
 *
 * \param directoryStorePath The directory path where the certificates will be stored.
 * \param pPKI A valid pointer to the PKIProvider.
 *
 * \note The directory is created if \p directoryStorePath does not exist.
 * \warning In case of error, \p pPKI is unchanged.
 *
 * \return SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS or SOPC_STATUS_NOK in case of error.
 */
SOPC_ReturnStatus SOPC_PKIProvider_SetStorePath(const char* directoryStorePath, SOPC_PKIProvider* pPKI);

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
 * \param bEraseExistingFiles whether the existing files of the updatedTrustList folder shall be deleted.
 *
 * \warning If the \p pPKI is built from lists ( ::SOPC_PKIProvider_CreateFromList ) then
 *          you shall define the directory store path with ::SOPC_PKIProvider_SetStorePath .
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProvider_WriteToStore(const SOPC_PKIProvider* pPKI, const bool bEraseExistingFiles);

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
SOPC_ReturnStatus SOPC_PKIProvider_WriteOrAppendToList(const SOPC_PKIProvider* pPKI,
                                                       SOPC_CertificateList** ppTrustedCerts,
                                                       SOPC_CRLList** ppTrustedCrl,
                                                       SOPC_CertificateList** ppIssuerCerts,
                                                       SOPC_CRLList** ppIssuerCrl);

/** \brief Get the list of certificate that have been rejected.
 *
 * \param pPKI A valid pointer to the PKIProvider.
 * \param[out] ppCert A copy of the PKI rejected list.
 *
 * \note The maximum number of certificates returned is \c SOPC_PKI_MAX_NB_CERT_REJECTED.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProvider_WriteRejectedCertToList(const SOPC_PKIProvider* pPKI, SOPC_CertificateList** ppCert);

/** \brief Write the rejected certificates files in the rejected folder of the PKI storage.
 *         The format of the written files is DER.
 *
 * \param pPKI A valid pointer to the PKIProvider.
 * \param bEraseExistingFiles whether the existing files of the rejected folder shall be deleted.
 *
 * \note At the maximum, we could have 2 * \c SOPC_PKI_MAX_NB_CERT_REJECTED in the rejected folder.
 *       This function removes older files, if the maximum is reach.
 *
 * \warning If the \p pPKI is built from lists ( ::SOPC_PKIProvider_CreateFromList ) then
 *          you shall define the directory store path with ::SOPC_PKIProvider_SetStorePath .
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProvider_WriteRejectedCertToStore(const SOPC_PKIProvider* pPKI,
                                                            const bool bEraseExistingFiles);

/** \brief Add a certificate to the PKI rejected list.
 *
 * \param ppPKI A valid pointer to the PKIProvider.
 * \param pCert A valid pointer to the certificate to be added.
 *
 * \note The function removes the oldest certificate if the list size reaches \c SOPC_PKI_MAX_NB_CERT_REJECTED.
 *
 * \warning \p pCert shall contains a single certificate.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProvider_AddCertToRejectedList(SOPC_PKIProvider** ppPKI, const SOPC_CertificateList* pCert);

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
SOPC_ReturnStatus SOPC_PKIProvider_UpdateFromList(SOPC_PKIProvider** ppPKI,
                                                  const char* securityPolicyUri,
                                                  SOPC_CertificateList* pTrustedCerts,
                                                  SOPC_CRLList* pTrustedCrl,
                                                  SOPC_CertificateList* pIssuerCerts,
                                                  SOPC_CRLList* pIssuerCrl,
                                                  const bool bIncludeExistingList);

/** \brief  Remove All the certificate which match with the given thumbprint.
 *          If the Certificate is a CA Certificate then all the CRLs for that CA are removed.
 *
 * \warning This function will fail if \p pThumbprint does not match the SHA1 hex digest size.
 *
 * \param ppPKI A valid pointer to the PKIProvider.
 * \param pThumbprint The SHA1 of the certificate formatted as an hexadecimal C string (NULL terminated)
 *                    40 bytes shall be allocated in \p pThumbprint (+ 1 byte for the NULL character)
 * \param bIsTrusted whether the certificate to remove is a trusted certificate.
 * \param[out] pbIsRemove A valid pointer indicating whether the certificate has been found and deleted.
 * \param[out] pbIsIssuer A valid pointer indicating whether the deleted certificate is an issuer.
 *
 * \return SOPC_STATUS_OK when successful.
 */
SOPC_ReturnStatus SOPC_PKIProvider_RemoveCertificate(SOPC_PKIProvider** ppPKI,
                                                     const char* pThumbprint,
                                                     const bool bIsTrusted,
                                                     bool* pbIsRemove,
                                                     bool* pbIsIssuer);

/**
 * \brief Free a PKI provider.
 *
 * \param ppPKI The PKI.
 */
void SOPC_PKIProvider_Free(SOPC_PKIProvider** ppPKI);

#endif /* SOPC_PKI_STACK_H_ */
