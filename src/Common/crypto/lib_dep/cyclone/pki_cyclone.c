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

/** \file pki_stack.c
 *
 * The minimal PKI implementation provided by the stack. It is lib-specific.
 *
 * This is not the role of the stack to provide a full-blown configurable PKI.
 * The stack provides only a minimal, always safe validating PKI.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_common_constants.h"
#include "sopc_crypto_decl.h"
#include "sopc_date_time.h"
#include "sopc_filesystem.h"
#include "sopc_helper_string.h"
#include "sopc_helper_uri.h"
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pki_stack.h"
#include "sopc_pki_stack_lib_internal_itf.h"
#include "sopc_pki_struct_lib_internal.h"

#include "key_manager_cyclone.h"

#include "encoding/oid.h"
#include "pkix/x509_cert_validate.h"
#include "pkix/x509_crl_validate.h"
#include "pkix/x509_sign_verify.h"

typedef struct
{
    const SOPC_CertificateList* trustedCrts;
    bool isTrustedInChain;
} SOPC_CheckTrusted;

// Bitmask errors for OPCUA error order compliance
#define PKI_CYCLONE_X509_BADCERT_EXPIRED 0x01 /**< The certificate validity has expired. */
#define PKI_CYCLONE_X509_BADCERT_REVOKED 0x02 /**< The certificate has been revoked (is on a CRL). */
#define PKI_CYCLONE_X509_BADCERT_NOT_TRUSTED 0x04 /**< The certificate is not correctly signed by the trusted CA. */
#define PKI_CYCLONE_X509_BADCERT_BAD_MD 0x10 /**< The certificate is signed with an unacceptable hash. */
#define PKI_CYCLONE_X509_BADCERT_BAD_PK \
    0x20 /**< The certificate is signed with an unacceptable PK alg (eg RSA vs ECDSA). */
#define PKI_CYCLONE_X509_BADCERT_BAD_KEY \
    0x40 /**< The certificate is signed with an unacceptable key (eg bad curve, RSA too short). */

#define PKI_CYCLONE_X509_BADCRL_EXPIRED 0x80
#define PKI_CYCLONE_X509_BADCRL_NOT_TRUSTED 0x100 /**< The CRL is not correctly signed by the trusted CA. */
#define PKI_CYCLONE_X509_BADCRL_BAD_MD 0x200
#define PKI_CYCLONE_X509_BADCRL_BAD_PK 0x400

static uint32_t PKIProviderStack_GetCertificateValidationError(uint32_t failure_reasons)
{
    if (0 != (failure_reasons &
              (PKI_CYCLONE_X509_BADCERT_BAD_KEY | PKI_CYCLONE_X509_BADCERT_BAD_MD | PKI_CYCLONE_X509_BADCERT_BAD_PK)))
    {
        return SOPC_CertificateValidationError_Invalid;
    }

    if (0 != (failure_reasons & PKI_CYCLONE_X509_BADCERT_NOT_TRUSTED))
    {
        return SOPC_CertificateValidationError_Untrusted;
    }

    if (0 != (failure_reasons & PKI_CYCLONE_X509_BADCERT_EXPIRED))
    {
        return SOPC_CertificateValidationError_TimeInvalid;
    }

    if (0 != (failure_reasons & (PKI_CYCLONE_X509_BADCRL_EXPIRED | PKI_CYCLONE_X509_BADCRL_NOT_TRUSTED |
                                 PKI_CYCLONE_X509_BADCRL_BAD_MD | PKI_CYCLONE_X509_BADCRL_BAD_PK)))
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }

    if (0 != (failure_reasons & PKI_CYCLONE_X509_BADCERT_REVOKED))
    {
        return SOPC_CertificateValidationError_Revoked;
    }

    return SOPC_CertificateValidationError_Unknown;
}

// removes cert from a linked list
static void sopc_remove_cert_from_list(SOPC_CertificateList* pPrev,
                                       SOPC_CertificateList** ppCur,
                                       SOPC_CertificateList** ppHeadCertList);

// PKI remove certificate from rejected list declaration
static void sopc_pki_remove_rejected_cert(SOPC_CertificateList** ppRejectedList, const SOPC_CertificateList* pCert);

SOPC_ReturnStatus SOPC_PKIProvider_CheckSecurityPolicy(const SOPC_CertificateList* pToValidate,
                                                       const SOPC_PKI_LeafProfile* pConfig)
{
    SOPC_ASSERT(NULL != pToValidate);
    SOPC_ASSERT(NULL != pConfig);

    SOPC_AsymmetricKey pub_key;
    uint_t keyLenBits = 0;
    bool bErr = false;
    // Retrieve key
    SOPC_ReturnStatus status = SOPC_KeyManagerInternal_Certificate_GetPublicKey(pToValidate, &pub_key);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    char* thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidate);
    if (NULL == thumbprint)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    // Retrieves and verifies key type
    /* Useless to do it it with CycloneCRYPTO since the only key type implemented is RSA. See issue #1356. */
    // Retrieves key length
    keyLenBits = mpiGetBitLength(&pub_key.pubKey.n);
    // Verifies key length: min-max
    if (keyLenBits < pConfig->RSAMinimumKeySize || keyLenBits > pConfig->RSAMaximumKeySize)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed : unexpected key size %" PRIu32
                               " for certificate thumbprint %s",
                               (uint32_t) keyLenBits, thumbprint);
        status = SOPC_STATUS_NOK;
    }

    // Verifies md of the signing algorithm
    const X509SignatureAlgoId* signAlgoId = &pToValidate->crt.signatureAlgo;
    // Point to the object identifier
    const uint8_t* oid = signAlgoId->oid.value;
    size_t oidLen = signAlgoId->oid.length;
    bErr = false;
    switch (pConfig->mdSign)
    {
    case SOPC_PKI_MD_SHA1_OR_ABOVE:
        // Function oidComp() returns 0 if the 2 object identifiers are equal
        if (0 != oidComp(oid, oidLen, SHA1_WITH_RSA_ENCRYPTION_OID, sizeof(SHA1_WITH_RSA_ENCRYPTION_OID)) &&
            0 != oidComp(oid, oidLen, SHA224_WITH_RSA_ENCRYPTION_OID, sizeof(SHA224_WITH_RSA_ENCRYPTION_OID)) &&
            0 != oidComp(oid, oidLen, SHA256_WITH_RSA_ENCRYPTION_OID, sizeof(SHA256_WITH_RSA_ENCRYPTION_OID)) &&
            0 != oidComp(oid, oidLen, SHA384_WITH_RSA_ENCRYPTION_OID, sizeof(SHA384_WITH_RSA_ENCRYPTION_OID)) &&
            0 != oidComp(oid, oidLen, SHA512_WITH_RSA_ENCRYPTION_OID, sizeof(SHA512_WITH_RSA_ENCRYPTION_OID)))
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA256_OR_ABOVE:
        if (0 != oidComp(oid, oidLen, SHA256_WITH_RSA_ENCRYPTION_OID, sizeof(SHA256_WITH_RSA_ENCRYPTION_OID)) &&
            0 != oidComp(oid, oidLen, SHA384_WITH_RSA_ENCRYPTION_OID, sizeof(SHA384_WITH_RSA_ENCRYPTION_OID)) &&
            0 != oidComp(oid, oidLen, SHA512_WITH_RSA_ENCRYPTION_OID, sizeof(SHA512_WITH_RSA_ENCRYPTION_OID)))
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA1:
        if (0 != oidComp(oid, oidLen, SHA1_WITH_RSA_ENCRYPTION_OID, sizeof(SHA1_WITH_RSA_ENCRYPTION_OID)))
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA256:
        if (0 != oidComp(oid, oidLen, SHA256_WITH_RSA_ENCRYPTION_OID, sizeof(SHA256_WITH_RSA_ENCRYPTION_OID)))
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA1_AND_SHA256:
        if (0 != oidComp(oid, oidLen, SHA1_WITH_RSA_ENCRYPTION_OID, sizeof(SHA1_WITH_RSA_ENCRYPTION_OID)) &&
            0 != oidComp(oid, oidLen, SHA256_WITH_RSA_ENCRYPTION_OID, sizeof(SHA256_WITH_RSA_ENCRYPTION_OID)))
        {
            bErr = true;
        }
        break;
    default:
        bErr = true;
    }
    if (bErr)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed : unexpected signing algorithm"
                               " for certificate thumbprint %s",
                               thumbprint);
        status = SOPC_STATUS_NOK;
    }

    SOPC_Free(thumbprint);
    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckHostName(const SOPC_CertificateList* pToValidate,
                                                 const char* url,
                                                 char** certUrl)
{
    // TODO : Add a domain name resolution (issue #1189)

    if (NULL == pToValidate || NULL == url)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    char* thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidate);
    if (NULL == thumbprint)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_UriType type = SOPC_URI_UNDETERMINED;
    char* pHostName = NULL;
    char* pPort = NULL;
    status = SOPC_Helper_URI_SplitUri(url, &type, &pHostName, &pPort);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed : Unable to split the given url %s to retrieve the hostName",
                               url);
        SOPC_Free(thumbprint);
        return SOPC_STATUS_NOK;
    }

    SOPC_ASSERT(NULL != pHostName);
    const size_t hostnameLen = strlen(pHostName);
    bool found = false;
    int match = -1;
    // Get the SANs of the cert
    X509SubjectAltName certSubjectAltName = pToValidate->crt.tbsCert.extensions.subjectAltName;

    for (size_t i = 0; i < certSubjectAltName.numGeneralNames && !found; i++)
    {
        if (X509_GENERAL_NAME_TYPE_DNS == certSubjectAltName.generalNames[i].type)
        {
            if (certSubjectAltName.generalNames[i].length == hostnameLen)
            {
                match = SOPC_strncmp_ignore_case(pHostName, certSubjectAltName.generalNames[i].value,
                                                 certSubjectAltName.generalNames[i].length);
                if (0 == match)
                {
                    /* stop research */
                    found = true;
                }
            }
            if (!found)
            {
                SOPC_Logger_TraceWarning(
                    SOPC_LOG_MODULE_COMMON,
                    "> PKI validation : dnsName %s of certificate thumbprint %s is not the expected one (%s)",
                    certSubjectAltName.generalNames[i].value, thumbprint, pHostName);
                if (NULL != certUrl && NULL == *certUrl)
                {
                    *certUrl = SOPC_strdup(certSubjectAltName.generalNames[i].value);
                }
            }
        }
    }

    if (!found)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed : hostName %s is not found in the dnsName SAN extension of "
                               "certificate thumbprint %s",
                               pHostName, thumbprint);
        status = SOPC_STATUS_NOK;
    }
    else
    {
        status = SOPC_STATUS_OK;
    }

    SOPC_Free(pHostName);
    SOPC_Free(pPort);
    SOPC_Free(thumbprint);

    return status;
}

static unsigned int get_lib_ku_from_sopc_ku(const SOPC_PKI_KeyUsage_Mask sopc_pki_ku)
{
    unsigned int usages = 0;
    if (SOPC_PKI_KU_DIGITAL_SIGNATURE & sopc_pki_ku)
    {
        usages |= X509_KEY_USAGE_DIGITAL_SIGNATURE;
    }
    if (SOPC_PKI_KU_NON_REPUDIATION & sopc_pki_ku)
    {
        usages |= X509_KEY_USAGE_NON_REPUDIATION;
    }
    if (SOPC_PKI_KU_KEY_ENCIPHERMENT & sopc_pki_ku)
    {
        usages |= X509_KEY_USAGE_KEY_ENCIPHERMENT;
    }
    if (SOPC_PKI_KU_KEY_DATA_ENCIPHERMENT & sopc_pki_ku)
    {
        usages |= X509_KEY_USAGE_DATA_ENCIPHERMENT;
    }
    if (SOPC_PKI_KU_KEY_CERT_SIGN & sopc_pki_ku)
    {
        usages |= X509_KEY_USAGE_KEY_CERT_SIGN;
    }
    if (SOPC_PKI_KU_KEY_CRL_SIGN & sopc_pki_ku)
    {
        usages |= X509_KEY_USAGE_CRL_SIGN;
    }

    return usages;
}

/* Returns SOPC_STATUS_OK if pToValidate has all the expected_extended_key_usages extended key usages.
 */
static SOPC_ReturnStatus crt_check_extended_key_usage(const SOPC_CertificateList* pToValidate,
                                                      unsigned int expected_extended_key_usages)
{
    SOPC_ASSERT(NULL != pToValidate);

    unsigned int cert_extended_key_usages = pToValidate->crt.tbsCert.extensions.extKeyUsage.bitmap;
    unsigned int real_extended_key_usages = cert_extended_key_usages & expected_extended_key_usages;

    if (real_extended_key_usages != expected_extended_key_usages)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

/* Returns SOPC_STATUS_OK if pToValidate has all the expected_key_usages key usages.
 */
static SOPC_ReturnStatus crt_check_key_usage(const SOPC_CertificateList* pToValidate, unsigned int expected_key_usages)
{
    SOPC_ASSERT(NULL != pToValidate);

    unsigned int cert_key_usages = pToValidate->crt.tbsCert.extensions.keyUsage.bitmap;
    unsigned int real_key_usages = cert_key_usages & expected_key_usages;

    if (real_key_usages != expected_key_usages)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckCommonName(const SOPC_CertificateList* pToValidate)
{
    SOPC_ASSERT(NULL != pToValidate);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (0 == pToValidate->crt.tbsCert.subject.commonName.length)
    {
        char* pThumb = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidate);
        const char* thumb = NULL != pThumb ? pThumb : "NULL";

        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_COMMON,
            "> PKI validation failed : The Common Name attribute is not specified for certificate thumbprint %s",
            thumb);

        status = SOPC_STATUS_NOK;
        SOPC_Free(pThumb);
    }

    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckCertificateUsage(const SOPC_CertificateList* pToValidate,
                                                         const SOPC_PKI_LeafProfile* pProfile)
{
    SOPC_ASSERT(NULL != pToValidate);
    SOPC_ASSERT(NULL != pProfile);

    char* thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidate);
    if (NULL == thumbprint)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    unsigned int usages = 0;
    bool bErrorFound = false;
    /* Check key usages */
    usages = get_lib_ku_from_sopc_ku(pProfile->keyUsage);
    SOPC_ReturnStatus status = crt_check_key_usage(pToValidate, usages);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation error : missing expected key usage for certificate thumbprint %s",
                               thumbprint);
        bErrorFound = true;
    }
    /* Check extended key usages for client or server cert */
    if (SOPC_PKI_EKU_SERVER_AUTH & pProfile->extendedKeyUsage)
    {
        status = crt_check_extended_key_usage(pToValidate, X509_EXT_KEY_USAGE_SERVER_AUTH);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_COMMON,
            "> PKI validation error : missing SERVER_AUTH extended key usage for certificate thumbprint %s",
            thumbprint);
        bErrorFound = true;
    }

    if (SOPC_PKI_EKU_CLIENT_AUTH & pProfile->extendedKeyUsage)
    {
        status = crt_check_extended_key_usage(pToValidate, X509_EXT_KEY_USAGE_CLIENT_AUTH);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_COMMON,
            "> PKI validation error : missing CLIENT_AUTH extended key usage for certificate thumbprint %s",
            thumbprint);
        bErrorFound = true;
    }

    SOPC_Free(thumbprint);
    if (bErrorFound)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

/* Define the _weak_func used by x509ValidateCertificate() here.
 * TODO : Put this function in another file ?
 */
time_t getCurrentUnixTime(void)
{
    time_t res = 0;

    // Retrieve current time
    SOPC_DateTime dateTime = SOPC_Time_GetCurrentTimeUTC();

    // Convert it to the type time_t
    SOPC_ReturnStatus status = SOPC_Time_ToUnixTime(dateTime, &res);

    if (SOPC_STATUS_OK == status)
    {
        return res;
    }

    return 0;
}

/* Checks if the hashAlgo is an allowed algo for the profile.
 */
static bool checkMdAllowed(const HashAlgo* hashAlgo, const SOPC_PKI_ChainProfile* pProfile)
{
    SOPC_ASSERT(NULL != hashAlgo);
    SOPC_ASSERT(NULL != pProfile);

    const char* hashAlgoName = hashAlgo->name;
    bool bMatch = false;
    int comparison = strcmp(hashAlgoName, "SHA-1");
    if (0 == comparison)
    {
        if (SOPC_PKI_MD_SHA1 == pProfile->mdSign || SOPC_PKI_MD_SHA1_AND_SHA256 == pProfile->mdSign ||
            SOPC_PKI_MD_SHA1_OR_ABOVE == pProfile->mdSign)
        {
            bMatch = true;
        }
    }
    comparison = strcmp(hashAlgoName, "SHA-224");
    if (0 == comparison)
    {
        if (SOPC_PKI_MD_SHA1_OR_ABOVE == pProfile->mdSign)
        {
            bMatch = true;
        }
    }
    comparison = strcmp(hashAlgoName, "SHA-256");
    if (0 == comparison)
    {
        if (SOPC_PKI_MD_SHA256_OR_ABOVE == pProfile->mdSign || SOPC_PKI_MD_SHA1_AND_SHA256 == pProfile->mdSign ||
            SOPC_PKI_MD_SHA1_OR_ABOVE == pProfile->mdSign || SOPC_PKI_MD_SHA256 == pProfile->mdSign)
        {
            bMatch = true;
        }
    }
    comparison = strcmp(hashAlgoName, "SHA-384");
    if (0 == comparison)
    {
        if (SOPC_PKI_MD_SHA1_OR_ABOVE == pProfile->mdSign || SOPC_PKI_MD_SHA256_OR_ABOVE == pProfile->mdSign)
        {
            bMatch = true;
        }
    }
    comparison = strcmp(hashAlgoName, "SHA-512");
    if (0 == comparison)
    {
        if (SOPC_PKI_MD_SHA1_OR_ABOVE == pProfile->mdSign || SOPC_PKI_MD_SHA256_OR_ABOVE == pProfile->mdSign)
        {
            bMatch = true;
        }
    }

    return bMatch;
}

static void crt_verifycrl_and_check_revocation(const SOPC_CertificateList* child,
                                               const SOPC_CertificateList* parent,
                                               SOPC_CRLList* crl,
                                               const SOPC_PKI_ChainProfile* pProfile,
                                               uint32_t* failure_reasons)
{
    SOPC_ASSERT(NULL != child);
    SOPC_ASSERT(NULL != parent);
    SOPC_ASSERT(NULL != pProfile);
    SOPC_ASSERT(NULL != failure_reasons);

    // Initialize at state "error"
    error_t errLib = 1;

    // Find the CRL of parent and check if it is correctly signed
    SOPC_CRLList* parent_crl = crl;
    bool bFound = false;
    while (!bFound && NULL != parent_crl)
    {
        /* This Cyclone function:
         * - returns error if parent does not have the extension CRL_SIGN
         * - checks the validity of the CRL (thisUpdate, nextUpdate)
         * - returns 0 if parent is the issuer of the CRL and the signature is good
         */
        errLib = x509ValidateCrl(&parent_crl->crl, &parent->crt);
        if (0 == errLib)
        {
            bFound = true;
        }
        else if (ERROR_CERTIFICATE_EXPIRED == errLib)
        {
            bFound = true;
            *failure_reasons |= PKI_CYCLONE_X509_BADCRL_EXPIRED;
        }

        // If it's not parent's CRL, iterate on the next candidate.
        if (!bFound)
        {
            parent_crl = parent_crl->next;
        }
    }

    if (bFound)
    {
        // Check if md and pk algos of the signature of the CRL suits the profile. */
        X509SignatureAlgo signAlgo = {0};
        const HashAlgo* hashAlgo = NULL;
        errLib = x509GetSignHashAlgo(&parent_crl->crl.signatureAlgo, &signAlgo, &hashAlgo);
        if (0 == errLib)
        {
            if (SOPC_PKI_PK_RSA == pProfile->pkAlgo)
            {
                if (X509_SIGN_ALGO_RSA != signAlgo && X509_SIGN_ALGO_RSA_PSS != signAlgo)
                {
                    *failure_reasons |= PKI_CYCLONE_X509_BADCRL_BAD_PK;
                }
            }

            bool bMatch = checkMdAllowed(hashAlgo, pProfile);
            if (!bMatch) // If the hash algo is not an allowed md.
            {
                *failure_reasons |= PKI_CYCLONE_X509_BADCRL_BAD_MD;
            }
        }
        else
        {
            *failure_reasons |= PKI_CYCLONE_X509_BADCRL_BAD_PK;
        }

        // Check if the certificate is not revoked in the parent CRL
        errLib = x509CheckRevokedCertificate(&child->crt, &parent_crl->crl);
        if (ERROR_CERTIFICATE_REVOKED == errLib)
        {
            *failure_reasons |= PKI_CYCLONE_X509_BADCERT_REVOKED;
        }
    }
    else
    {
        *failure_reasons |= PKI_CYCLONE_X509_BADCRL_NOT_TRUSTED;
    }
}

/* Find a parent in the chain parentCandidates.
 * If a parent has been found, ppParent is filled with it.
 * Otherwise, ppParent is not modified.
 */
static void crt_find_parent_in(const SOPC_CertificateList* child,
                               SOPC_CertificateList* parentCandidates,
                               uint32_t* failure_reasons,
                               SOPC_CertificateList** ppParent)
{
    // No asserts here since they are in crt_find_parent and the two functions work together

    SOPC_CertificateList* parent = parentCandidates;
    bool bFound = false;
    error_t errLib = 1;

    while (!bFound && NULL != parent)
    {
        /* This Cyclone function :
         * - checks the validity of child ;
         * - checks if parent is the parent of child ;
         * - verifies the signature ;
         * Returns 0 if all is ok.
         */
        errLib = x509ValidateCertificate(&child->crt, &parent->crt, 0);
        if (0 == errLib)
        {
            bFound = true;
        }
        else if (ERROR_CERTIFICATE_EXPIRED == errLib)
        {
            bFound = true;
            *failure_reasons |= PKI_CYCLONE_X509_BADCERT_EXPIRED;
        }

        // If it's not the parent, iterate on the next candidate.
        if (!bFound)
        {
            parent = parent->next;
        }
    }

    if (bFound)
    {
        *ppParent = parent;
    }
}

/* Find the parent of child in the chain rootCA first, then up the leafAndIntCA chain if not found. */
static void crt_find_parent(const SOPC_CertificateList* child,
                            SOPC_CertificateList* leafAndIntCA,
                            SOPC_CertificateList* rootCA,
                            uint32_t* failure_reasons,
                            SOPC_CertificateList** ppParent,
                            bool* parent_is_trusted)
{
    // Make sure the container of the potential parent is not null, and empty.
    SOPC_ASSERT(NULL != ppParent);
    SOPC_ASSERT(NULL == *ppParent);
    SOPC_ASSERT(NULL != child);
    SOPC_ASSERT(NULL != leafAndIntCA);
    SOPC_ASSERT(NULL != failure_reasons);

    // Find a parent in rootCA
    *parent_is_trusted = true;
    crt_find_parent_in(child, rootCA, failure_reasons, ppParent);

    // If no parent has been found, search up the initial leaf chain
    if (NULL == *ppParent)
    {
        *parent_is_trusted = false;
        crt_find_parent_in(child, leafAndIntCA, failure_reasons, ppParent);
    }
}

/* Some verifications on the certificate with the profile:
 * - key type and size
 * - md and pk signature algos
 * The result of the verification process is stored in failure_reasons.
 */
static void crt_verify_profile_in_chain(const SOPC_CertificateList* pToValidate,
                                        const SOPC_PKI_ChainProfile* pProfile,
                                        uint32_t* failure_reasons)
{
    SOPC_ASSERT(NULL != pToValidate);
    SOPC_ASSERT(NULL != pProfile);
    SOPC_ASSERT(NULL != failure_reasons);

    /* 1) Check the type and size of the key */
    X509KeyType keyType = x509GetPublicKeyType(pToValidate->crt.tbsCert.subjectPublicKeyInfo.oid.value,
                                               pToValidate->crt.tbsCert.subjectPublicKeyInfo.oid.length);
    // If the profile is RSA
    if (SOPC_PKI_PK_RSA == pProfile->pkAlgo)
    {
        // Type.
        if (X509_KEY_TYPE_RSA != keyType && X509_KEY_TYPE_RSA_PSS != keyType)
        {
            *failure_reasons |= PKI_CYCLONE_X509_BADCERT_BAD_PK;
        }
        else
        {
            // If the key type suits to the profile, check its size.
            size_t keySize = pToValidate->crt.tbsCert.subjectPublicKeyInfo.rsaPublicKey.n.length * 8;
            if (keySize < pProfile->RSAMinimumKeySize)
            {
                *failure_reasons |= PKI_CYCLONE_X509_BADCERT_BAD_KEY;
            }
        }
    }

    /* 2) Check if md and pk algos of the signature of the cert suits to the profile. */
    X509SignatureAlgo signAlgo = {0};
    const HashAlgo* hashAlgo = NULL;
    error_t errLib = x509GetSignHashAlgo(&pToValidate->crt.signatureAlgo, &signAlgo, &hashAlgo);
    if (0 == errLib)
    {
        if (SOPC_PKI_PK_RSA == pProfile->pkAlgo)
        {
            if (X509_SIGN_ALGO_RSA != signAlgo && X509_SIGN_ALGO_RSA_PSS != signAlgo)
            {
                *failure_reasons |= PKI_CYCLONE_X509_BADCERT_BAD_PK;
            }
        }

        bool bMatch = checkMdAllowed(hashAlgo, pProfile);
        if (!bMatch) // If the hash algo is not an allowed md.
        {
            *failure_reasons |= PKI_CYCLONE_X509_BADCERT_BAD_MD;
        }
    }
    else
    {
        *failure_reasons |= PKI_CYCLONE_X509_BADCERT_BAD_PK;
    }
}

static void crt_check_trusted(SOPC_CheckTrusted* checkTrusted, const SOPC_CertificateList* crt)
{
    SOPC_ASSERT(NULL != checkTrusted);
    SOPC_ASSERT(NULL != crt);

    /* Checks if the certificate is part of PKI trusted certificates */
    const SOPC_CertificateList* crtTrusted = NULL;
    if (NULL != checkTrusted->trustedCrts)
    {
        crtTrusted = checkTrusted->trustedCrts; /* Current cert */
    }
    while (!checkTrusted->isTrustedInChain && NULL != crtTrusted)
    {
        if (crt->crt.tbsCert.subject.raw.length == crtTrusted->crt.tbsCert.subject.raw.length &&
            crt->raw->length == crtTrusted->raw->length &&
            0 == memcmp(crt->crt.tbsCert.subject.raw.value, crtTrusted->crt.tbsCert.subject.raw.value,
                        crt->crt.tbsCert.subject.raw.length) &&
            0 == memcmp(crt->raw->data, crtTrusted->raw->data, crt->crt.tbsCert.subject.raw.length))
        {
            checkTrusted->isTrustedInChain = true;
        }
        crtTrusted = crtTrusted->next;
    }
}

/* Verifies the chain of the certificate.*/
static void crt_verify_chain(SOPC_CertificateList* pToValidate,
                             SOPC_CertificateList* trust_list,
                             SOPC_CRLList* cert_crl,
                             const SOPC_PKI_ChainProfile* pProfile,
                             uint32_t* failure_reasons,
                             SOPC_CheckTrusted* checkTrusted)
{
    // Check parameters
    SOPC_ASSERT(NULL != pToValidate);
    SOPC_ASSERT(NULL != pProfile);
    SOPC_ASSERT(NULL != failure_reasons);
    SOPC_ASSERT(NULL != checkTrusted);

    SOPC_CertificateList* leafAndIntCA = pToValidate;
    SOPC_CertificateList* parent = NULL;
    uint32_t failure_reason_on_certificate = 0;
    bool parent_is_trusted = false;
    bool leafAndIntCA_is_trusted = false;

    /**
     * While:
     * 1) The validation on the previous certificate of the chain went ok
     * 2) The new certificate of the chain is not trusted
     */
    while (0 == failure_reason_on_certificate && !leafAndIntCA_is_trusted)
    {
        // Verify with profile
        crt_verify_profile_in_chain(leafAndIntCA, pProfile, &failure_reason_on_certificate);

        // Check if trusted
        crt_check_trusted(checkTrusted, leafAndIntCA);

        // Find a parent in trusted CA first or in the pToValidate chain.
        // This function will also verify the signature if a parent is found,
        // and check time-validity of leafAndIntCA.
        crt_find_parent(leafAndIntCA, pToValidate, trust_list, &failure_reason_on_certificate, &parent,
                        &parent_is_trusted);
        if (NULL == parent)
        {
            failure_reason_on_certificate |= PKI_CYCLONE_X509_BADCERT_NOT_TRUSTED;
        }
        else
        {
            // If a parent has been found, proceed on some checks on its associated CRL
            // and check if the child is not revoked.
            if (!pProfile->bDisableRevocationCheck)
            {
                crt_verifycrl_and_check_revocation(leafAndIntCA, parent, cert_crl, pProfile,
                                                   &failure_reason_on_certificate);
            }
        }

        // Iterate.
        leafAndIntCA = parent;
        parent = NULL;
        leafAndIntCA_is_trusted = parent_is_trusted;
    }

    if (leafAndIntCA_is_trusted)
    {
        // In this case, the last certificate of the chain has not been verified. Verify it here.
        // Verify with profile
        crt_verify_profile_in_chain(leafAndIntCA, pProfile, &failure_reason_on_certificate);
        // Check if trusted
        crt_check_trusted(checkTrusted, leafAndIntCA);
    }

    // Merge the validation error with our global errors
    *failure_reasons |= failure_reason_on_certificate;
}

/* ------------------------------------------------------------------------------------------------
 * Self-signed certificates related functions
 * ------------------------------------------------------------------------------------------------
 */

/* Check the validity of the certificate. Inspired of the Cyclone function x509ValidateCertificate().
 */
static bool crt_check_validity(const X509CertificateInfo* crt)
{
    SOPC_ASSERT(NULL != crt);

    // Initialize at state "error"
    bool bIsValid = false;

    // Retrieve current time
    time_t currentTime = getCurrentUnixTime();

    if (0 != currentTime)
    {
        DateTime currentDate;
        const X509Validity* validity;

        // Convert Unix timestamp to date
        convertUnixTimeToDate(currentTime, &currentDate);

        // The certificate validity period is the time interval during which the
        // CA warrants that it will maintain information about the status of the
        // certificate
        validity = &crt->tbsCert.validity;

        // Check the validity period
        if (compareDateTime(&currentDate, &validity->notBefore) > 0 &&
            compareDateTime(&currentDate, &validity->notAfter) < 0)
        {
            bIsValid = true;
        }
    }

    return bIsValid;
}

static void crt_verify_self_signed(const SOPC_CertificateList* pToValidate,
                                   const SOPC_PKI_ChainProfile* pProfile,
                                   uint32_t* failure_reasons,
                                   SOPC_CheckTrusted* checkTrusted)
{
    // Check parameters
    SOPC_ASSERT(NULL != pToValidate);
    SOPC_ASSERT(NULL != pProfile);
    SOPC_ASSERT(NULL != failure_reasons);
    SOPC_ASSERT(NULL != checkTrusted);

    // Verify the certificate with the profile.
    crt_verify_profile_in_chain(pToValidate, pProfile, failure_reasons);

    // If the certificate is CA (and self-signed), verify it has the keyUsage keyCertSign
    if (1 == pToValidate->crt.tbsCert.extensions.basicConstraints.cA)
    {
        const X509Extensions* extensions = &pToValidate->crt.tbsCert.extensions;
        if (0 != extensions->keyUsage.bitmap)
        {
            // If the keyUsage extension is present, then the subject public key must
            // not be used to verify signatures on certificates unless the keyCertSign
            // bit is set (refer to RFC 5280, section 4.2.1.3)
            if (0 == (extensions->keyUsage.bitmap & X509_KEY_USAGE_KEY_CERT_SIGN))
            {
                *failure_reasons |= PKI_CYCLONE_X509_BADCERT_NOT_TRUSTED;
            }
        }
    }

    // Check validity
    bool bIsValid = crt_check_validity(&pToValidate->crt);
    if (!bIsValid)
    {
        *failure_reasons |= PKI_CYCLONE_X509_BADCERT_EXPIRED;
    }

    // Check if the certificate is trusted.
    crt_check_trusted(checkTrusted, pToValidate);
}

static SOPC_ReturnStatus sopc_validate_certificate(const SOPC_PKIProvider* pPKI,
                                                   SOPC_CertificateList* cert,
                                                   const SOPC_PKI_ChainProfile* pProfile,
                                                   bool bIsSelfSigned,
                                                   bool bForceTrustedCert,
                                                   const char* thumbprint,
                                                   uint32_t* error)
{
    SOPC_ASSERT(NULL != pPKI);
    SOPC_ASSERT(NULL != cert);
    SOPC_ASSERT(NULL == cert->next);
    SOPC_ASSERT(NULL != pProfile);
    SOPC_ASSERT(NULL != thumbprint);
    SOPC_ASSERT(NULL != error);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    /* Note: we consider all root CAs as trusted since our criteria is that at least one certificate is trusted in the
     *       chain and it might not be the root CA.
     *       We do an additional check that it is the case during the verification.
     */
    SOPC_CertificateList* ca_root = pPKI->pAllRoots;
    SOPC_CRLList* crl = pPKI->pAllCrl;
    /* Link certificate to validate with intermediate certificates (trusted links or untrusted links) */
    SOPC_CertificateList* pLinkCert = pPKI->pAllCerts;

    uint32_t failure_reasons = 0;
    SOPC_CheckTrusted checkTrusted = {.trustedCrts = pPKI->pAllTrusted, .isTrustedInChain = bForceTrustedCert};

    /* CycloneCRYPTO : special case if the certificate is not CA. */
    if (bIsSelfSigned)
    {
        /* Verify the self-signed certificate. The certificate has already been validated by
         * SOPC_KeyManager_Certificate_IsSelfSigned. */
        crt_verify_self_signed(cert, pProfile, &failure_reasons, &checkTrusted);
    }
    else
    {
        /* If certificate is not self signed, add the intermediate certificates to the trust chain to evaluate */
        cert->next = pLinkCert;
        /* Verify and validate the chain */
        crt_verify_chain(cert, ca_root, crl, pProfile, &failure_reasons, &checkTrusted);
    }

    // Check if at a least one trusted certificate is present in trust chain
    if (!checkTrusted.isTrustedInChain)
    {
        failure_reasons = (failure_reasons | (uint32_t) PKI_CYCLONE_X509_BADCERT_NOT_TRUSTED);
    }

    if (0 != failure_reasons)
    {
        *error = PKIProviderStack_GetCertificateValidationError(failure_reasons);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed with error code %" PRIX32 " for certificate thumbprint %s",
                               *error, thumbprint);
        status = SOPC_STATUS_NOK;
    }
    /* Unlink intermediate CAs from cert,
       otherwise destroying the pToValidate will also destroy trusted or untrusted links */
    cert->next = NULL;

    return status;
}

static void sopc_remove_cert_from_list(SOPC_CertificateList* pPrev,
                                       SOPC_CertificateList** ppCur,
                                       SOPC_CertificateList** ppHeadCertList)
{
    SOPC_ASSERT(NULL != ppCur);
    SOPC_ASSERT(NULL != *ppCur); /* Current cert shall not be NULL */
    SOPC_ASSERT(NULL != ppHeadCertList);
    SOPC_ASSERT(NULL != *ppHeadCertList); /* Head shall not be NULL */

    SOPC_CertificateList* pHeadCertList = *ppHeadCertList;
    SOPC_CertificateList* pCur = *ppCur;      /* Current cert */
    SOPC_CertificateList* pNext = pCur->next; /* Next cert */
    pCur->next = NULL;
    SOPC_Buffer_Delete(pCur->raw);
    rsaFreePublicKey(&pCur->pubKey);
    if (NULL == pPrev)
    {
        if (NULL == pNext)
        {
            /* The list is empty, Free it and stop the iteration  */
            SOPC_Free(pHeadCertList);
            pHeadCertList = NULL;
            pCur = NULL;
        }
        else
        {
            /* Head of the list is a special case */
            *pHeadCertList = *pNext; /* Use an assignment operator to do the copy */
            /* We have to free the new next certificate */
            SOPC_Free(pNext);

            /* Do not iterate: current certificate has changed with the new head (pCur = &pHeadCertList->crt) */
        }
    }
    else
    {
        /* We have to free the certificate if it is not the first in the list */
        SOPC_Free(pCur);
        pPrev->next = pNext;
        /* Iterate */
        pCur = pNext;
    }
    *ppCur = pCur;
    *ppHeadCertList = pHeadCertList;
}

static void sopc_pki_remove_rejected_cert(SOPC_CertificateList** ppRejectedList, const SOPC_CertificateList* pCert)
{
    SOPC_ASSERT(NULL != ppRejectedList);
    SOPC_ASSERT(NULL != pCert);
    SOPC_ASSERT(NULL == pCert->next);

    /* Head of the rejected list */
    SOPC_CertificateList* pHeadRejectedCertList = *ppRejectedList;
    if (NULL == pHeadRejectedCertList)
    {
        /* the certificate list is empty, do nothing*/
        return;
    }
    SOPC_CertificateList* cur = pHeadRejectedCertList; /* Current cert */
    SOPC_CertificateList* prev = NULL;                 /* Parent of current cert */
    bool bFound = false;
    bool_t bMatchName = false;

    while (NULL != cur && !bFound)
    {
        /* Compare the subject name */
        bMatchName = x509CompareName(pCert->crt.tbsCert.subject.raw.value, pCert->crt.tbsCert.subject.raw.length,
                                     cur->crt.tbsCert.subject.raw.value, cur->crt.tbsCert.subject.raw.length);
        if (bMatchName)
        {
            if (0 == memcmp(pCert->raw->data, cur->raw->data, pCert->raw->length))
            {
                bFound = true;
            }
        }
        if (bFound)
        {
            sopc_remove_cert_from_list(prev, &cur, &pHeadRejectedCertList);
        }
        else
        {
            prev = cur;
            cur = cur->next;
        }
    }
    *ppRejectedList = pHeadRejectedCertList;
    return;
}

SOPC_ReturnStatus SOPC_PKIProvider_AddCertToRejectedList(SOPC_PKIProvider* pPKI, const SOPC_CertificateList* pCert)
{
    if (NULL == pPKI || NULL == pCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    size_t listLength = 0;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_GetListLength(pCert, &listLength);
    if (SOPC_STATUS_OK != status || 1 != listLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    listLength = 0;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (NULL != pPKI->pRejectedList)
    {
        status = SOPC_KeyManager_Certificate_GetListLength(pPKI->pRejectedList, &listLength);
        /* Remove the oldest certificate (HEAD of the chain) if the rejected list is too large */
        if (SOPC_STATUS_OK == status)
        {
            if (SOPC_PKI_MAX_NB_CERT_REJECTED == listLength)
            {
                /* Change the HEAD */
                SOPC_CertificateList* cur = pPKI->pRejectedList;
                SOPC_CertificateList* next = cur->next;
                cur->next = NULL;
                if (NULL == next)
                {
                    /* case where SOPC_PKI_MAX_NB_CERT_REJECTED == 1 */
                    SOPC_KeyManager_Certificate_Free(pPKI->pRejectedList);
                    pPKI->pRejectedList = NULL;
                }
                else
                {
                    rsaFreePublicKey(&cur->pubKey);
                    SOPC_Buffer_Delete(cur->raw);
                    *pPKI->pRejectedList = *next; /* Copy with an assignment operator */
                    SOPC_Free(next);
                }
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        /* Create the rejected list if empty or append at the end */
        status =
            SOPC_KeyManager_Certificate_CreateOrAddFromDER(pCert->raw->data, pCert->raw->length, &pPKI->pRejectedList);
    }

    if (SOPC_STATUS_OK != status)
    {
        char* pThumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pCert);
        const char* thumbprint = NULL == pThumbprint ? "NULL" : pThumbprint;
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                 "> PKI : cannot append the certificate thumbprint %s to the rejected list",
                                 thumbprint);

        SOPC_Free(pThumbprint);
    }

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_PKIProviderInternal_ValidateProfileAndCertificate(SOPC_PKIProvider* pPKI,
                                                                         const SOPC_CertificateList* pToValidate,
                                                                         const SOPC_PKI_Profile* pProfile,
                                                                         uint32_t* error,
                                                                         SOPC_PKI_Cert_Failure_Context* context)
{
    if (NULL == pPKI || NULL == pToValidate || NULL == pProfile || NULL == error)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    *error = SOPC_CertificateValidationError_Unknown;

    size_t listLength = 0;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_GetListLength(pToValidate, &listLength);
    if (1 != listLength || SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CertificateList* pToValidateCpy = NULL;
    status = SOPC_KeyManager_Certificate_Copy(pToValidate, &pToValidateCpy);
    if (SOPC_STATUS_OK != status || NULL == pToValidateCpy)
    {
        return status;
    }

    uint32_t firstError = SOPC_CertificateValidationError_Unknown;
    uint32_t currentError = SOPC_CertificateValidationError_Unknown;
    bool bErrorFound = false;
    bool bIsSelfSigned = false;
    char* pThumbprint = NULL;
    const char* thumbprint = NULL;
    status = SOPC_KeyManager_Certificate_IsSelfSigned(pToValidateCpy, &bIsSelfSigned);
    if (SOPC_STATUS_OK != status)
    {
        /* unexpected error : failed to run a self-signature */
        SOPC_KeyManager_Certificate_Free(pToValidateCpy);
        return status;
    }
    pThumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidateCpy);
    thumbprint = NULL == pThumbprint ? "NULL" : pThumbprint;
    /* Certificate shall not be a CA or only for self-signed backward compatibility */
    bool certToValidateConstraints = (!pToValidateCpy->crt.tbsCert.extensions.basicConstraints.cA ||
                                      (bIsSelfSigned && pProfile->bBackwardInteroperability &&
                                       0 == pToValidateCpy->crt.tbsCert.extensions.basicConstraints.pathLenConstraint));
    if (!certToValidateConstraints)
    {
        /* CA certificates are always rejected (except for roots if backward interoperability is enabled) */
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed : certificate thumbprint %s is a CA which is not expected",
                               thumbprint);
        firstError = SOPC_CertificateValidationError_UseNotAllowed;
        bErrorFound = true;
    }

    /* Apply verification on the certificate */
    if (pProfile->bApplyLeafProfile)
    {
        status = SOPC_PKIProvider_CheckLeafCertificate(pToValidateCpy, pProfile->leafProfile, &currentError, context);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                   "> PKI validation failed : bad properties of certificate thumbprint %s", thumbprint);
            if (!bErrorFound)
            {
                firstError = currentError;
                bErrorFound = true;
            }
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = sopc_validate_certificate(pPKI, pToValidateCpy, pProfile->chainProfile, bIsSelfSigned, false,
                                           thumbprint, &currentError);
        if (SOPC_STATUS_OK != status)
        {
            if (!bErrorFound)
            {
                firstError = currentError;
                bErrorFound = true;
            }
        }
    }

    if (bErrorFound)
    {
        status = SOPC_PKIProvider_AddCertToRejectedList(pPKI, pToValidateCpy);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                     "> PKI : AddCertToRejectedList failed for certificate thumbprint %s", thumbprint);
        }
        *error = firstError;
        status = SOPC_STATUS_NOK;
    }
    else
    {
        sopc_pki_remove_rejected_cert(&pPKI->pRejectedList, pToValidateCpy);
    }

    if (SOPC_STATUS_OK == status)
    {
        *error = 0;
    }
    SOPC_KeyManager_Certificate_Free(pToValidateCpy);
    SOPC_Free(pThumbprint);
    return status;
}

static void sopc_free_c_string_from_ptr(void* data)
{
    if (NULL != data)
    {
        SOPC_Free(*(char**) data);
    }
}

static SOPC_ReturnStatus sopc_verify_every_certificate(SOPC_CertificateList* pPkiCerts,
                                                       const SOPC_PKIProvider* pPKI,
                                                       const SOPC_PKI_ChainProfile* pProfile,
                                                       bool* bErrorFound,
                                                       SOPC_Array* pErrors,
                                                       SOPC_Array* pThumbprints)
{
    SOPC_ASSERT(NULL != pPkiCerts);
    SOPC_ASSERT(NULL != pProfile);
    SOPC_ASSERT(NULL != pErrors);
    SOPC_ASSERT(NULL != pThumbprints);

    SOPC_CertificateList* pCertsCpy = NULL;
    bool bResAppend = true;
    uint32_t error = 0;
    char* thumbprint = NULL;
    SOPC_CertificateList* crt = NULL;
    SOPC_CertificateList* save_next = NULL;

    SOPC_ReturnStatus statusChain = SOPC_STATUS_OK;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_Copy(pPkiCerts, &pCertsCpy);
    if (SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    crt = pCertsCpy;
    while (NULL != crt && SOPC_STATUS_OK == status)
    {
        bool bIsSelfSigned = false;
        /* unlink crt */
        save_next = crt->next;
        crt->next = NULL;
        /* Get thumbprint */
        thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(crt);
        if (NULL == thumbprint)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_KeyManager_Certificate_IsSelfSigned(crt, &bIsSelfSigned);
        }
        if (SOPC_STATUS_OK == status)
        {
            const bool forceTrustedCert = true;
            statusChain =
                sopc_validate_certificate(pPKI, crt, pProfile, bIsSelfSigned, forceTrustedCert, thumbprint, &error);
            if (SOPC_STATUS_OK != statusChain)
            {
                *bErrorFound = true;
                /* Append the error */
                bResAppend = SOPC_Array_Append(pErrors, error);
                if (bResAppend)
                {
                    bResAppend = SOPC_Array_Append(pThumbprints, thumbprint);
                }
                status = bResAppend ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
                /* We do not release the thumbprint as the array is the ownership */
            }
            else
            {
                SOPC_Free(thumbprint);
            }
        }
        /* link crt */
        crt->next = save_next;
        /* iterate */
        crt = crt->next;
        error = 0;
    }

    if (SOPC_STATUS_OK != status)
    {
        /* SOPC_Array_Append failure case */
        SOPC_Free(thumbprint);
    }
    SOPC_KeyManager_Certificate_Free(pCertsCpy);

    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_VerifyEveryCertificate(SOPC_PKIProvider* pPKI,
                                                          const SOPC_PKI_ChainProfile* pProfile,
                                                          uint32_t** pErrors,
                                                          char*** ppThumbprints,
                                                          uint32_t* pLength)
{
    if (NULL == pPKI || NULL == pProfile || NULL == pErrors || NULL == ppThumbprints || NULL == pLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OUT_OF_MEMORY;
    bool bErrorFound = false;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_Array* pThumbArray = SOPC_Array_Create(sizeof(char*), 0, sopc_free_c_string_from_ptr);
    SOPC_Array* pErrArray = NULL;
    if (NULL != pThumbArray)
    {
        pErrArray = SOPC_Array_Create(sizeof(uint32_t), 0, NULL);
    }

    if (NULL != pPKI->pAllCerts)
    {
        status = sopc_verify_every_certificate(pPKI->pAllCerts, pPKI, pProfile, &bErrorFound, pErrArray, pThumbArray);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != pPKI->pAllRoots)
        {
            status =
                sopc_verify_every_certificate(pPKI->pAllRoots, pPKI, pProfile, &bErrorFound, pErrArray, pThumbArray);
        }
    }

    /* Verify lengths */
    if (SOPC_STATUS_OK == status && bErrorFound)
    {
        size_t lenError = SOPC_Array_Size(pErrArray);
        size_t lenThumb = SOPC_Array_Size(pThumbArray);
        if (lenError != lenThumb)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        else if (0 == lenError)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        else
        {
            *pLength = (uint32_t) lenError;
        }
    }
    /* retrieve C arrays */
    if (SOPC_STATUS_OK == status && bErrorFound)
    {
        *pErrors = SOPC_Array_Into_Raw(pErrArray);
        *ppThumbprints = SOPC_Array_Into_Raw(pThumbArray);
        if (NULL == *pErrors || NULL == *ppThumbprints)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        /* We shall free all te C array if error */
        if (SOPC_STATUS_OK != status)
        {
            if (NULL != *ppThumbprints)
            {
                for (uint32_t i = 0; i < *pLength; i++)
                {
                    SOPC_Free(*ppThumbprints[i]);
                }
                SOPC_Free(*ppThumbprints);
            }
            if (NULL != *pErrors)
            {
                SOPC_Free(*pErrors);
            }
        }
        pErrArray = NULL;
        pThumbArray = NULL;
    }

    /* Clear */
    SOPC_Array_Delete(pErrArray);
    SOPC_Array_Delete(pThumbArray);

    if (SOPC_STATUS_OK != status || !bErrorFound)
    {
        *pErrors = NULL;
        *ppThumbprints = NULL;
        *pLength = 0;
    }
    else if (bErrorFound)
    {
        status = SOPC_STATUS_NOK;
    }

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_PKIProviderInternal_SplitRootFromCertList(SOPC_CertificateList** ppCerts,
                                                                 SOPC_CertificateList** ppRootCa)
{
    SOPC_ASSERT(NULL != ppCerts);
    SOPC_ASSERT(NULL != ppRootCa);

    SOPC_CertificateList* pHeadCerts = *ppCerts;
    if (NULL == pHeadCerts)
    {
        /* The certificate list is empty */
        return SOPC_STATUS_OK;
    }
    SOPC_CertificateList* pHeadRoots = NULL;
    SOPC_CertificateList* cur = pHeadCerts; /* Start from the HEAD*/
    SOPC_CertificateList* prev = NULL;      /* Parent of current cert */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    while (NULL != cur && SOPC_STATUS_OK == status)
    {
        bool is_root = true;
        bool self_sign = false;

        /* Skip certificates that are not authorities */
        if (!cur->crt.tbsCert.extensions.basicConstraints.cA)
        {
            is_root = false;
        }
        status = SOPC_KeyManager_Certificate_IsSelfSigned(cur, &self_sign);
        if (!self_sign && is_root)
        {
            is_root = false;
        }
        if (is_root && SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(cur->raw->data, cur->raw->length, &pHeadRoots);

            if (SOPC_STATUS_OK == status)
            {
                /* Remove the certificate from the chain and safely delete it */
                sopc_remove_cert_from_list(prev, &cur, &pHeadCerts);
            }
        }
        else
        {
            /* Prepare next iteration */
            prev = cur;
            cur = cur->next;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pHeadRoots);
        pHeadRoots = NULL;
    }

    *ppRootCa = pHeadRoots;
    *ppCerts = pHeadCerts;

    return status;
}

void SOPC_PKIProviderInternal_GetListStats(SOPC_CertificateList* pCert,
                                           uint32_t* caCount,
                                           uint32_t* listLength,
                                           uint32_t* rootCount)
{
    if (NULL == pCert)
    {
        return;
    }
    bool is_self_sign = false;
    while (NULL != pCert)
    {
        *listLength = *listLength + 1;
        if (pCert->crt.tbsCert.extensions.basicConstraints.cA)
        {
            *caCount = *caCount + 1;
            SOPC_KeyManager_Certificate_IsSelfSigned(pCert, &is_self_sign);
            if (is_self_sign)
            {
                *rootCount = *rootCount + 1;
            }
        }
        pCert = pCert->next;
    }
}
