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

#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_crypto_decl.h"
#include "sopc_filesystem.h"
#include "sopc_helper_string.h"
#include "sopc_helper_uri.h"
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pki_stack.h"
#include "sopc_pki_struct_lib_internal.h"

#include "key_manager_mbedtls.h"
#include "mbedtls_common.h"

#include "mbedtls/oid.h"
#include "mbedtls/version.h"

#ifndef STR_TRUSTLIST_NAME
#define STR_TRUSTLIST_NAME "/updatedTrustList"
#endif

#define STR_TRUSTED "/trusted"
#define STR_TRUSTED_CERTS "/trusted/certs"
#define STR_TRUSTED_CRL "/trusted/crl"
#define STR_ISSUERS "/issuers"
#define STR_ISSUERS_CERTS "/issuers/certs"
#define STR_ISSUERS_CRL "/issuers/crl"
#define STR_REJECTED "/rejected"

typedef struct
{
    const SOPC_CertificateList* trustedCrts;
    const SOPC_CRLList* allCRLs;
    bool isTrustedInChain;
    bool disableRevocationCheck;
} SOPC_CheckTrustedAndCRLinChain;

static uint32_t PKIProviderStack_GetCertificateValidationError(uint32_t failure_reasons)
{
    // Order compliant with part 4 (1.04) Table 106

    /* Certificate structure */
    if ((failure_reasons & MBEDTLS_X509_BADCERT_MISSING) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_KEY_USAGE) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_EXT_KEY_USAGE) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_NS_CERT_TYPE) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_SKIP_VERIFY) != 0)
    {
        return SOPC_CertificateValidationError_UseNotAllowed;
    }

    /* Signature */
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_BAD_KEY) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_BAD_MD) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_BAD_PK) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }

    /* Trust list check*/

    /* Generic signature error */
    if ((failure_reasons & MBEDTLS_X509_BADCERT_NOT_TRUSTED) != 0)
    {
        return SOPC_CertificateValidationError_Untrusted;
    }

    /* Validity period */
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_EXPIRED) != 0)
    {
        return SOPC_CertificateValidationError_TimeInvalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_FUTURE) != 0)
    {
        return SOPC_CertificateValidationError_TimeInvalid;
    }

    /* Host Name */
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_CN_MISMATCH) != 0)
    {
        return SOPC_CertificateValidationError_HostNameInvalid;
    }
    /* URI */

    /* Certificate Usage */
    // Checked in PKIProviderStack_ValidateCertificate

    /* (Find) Revocation List */
    else if ((failure_reasons & MBEDTLS_X509_BADCRL_NOT_TRUSTED) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCRL_EXPIRED) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCRL_FUTURE) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCRL_BAD_MD) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCRL_BAD_PK) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCRL_BAD_KEY) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }

    /* Revocation check */
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_REVOKED) != 0)
    {
        return SOPC_CertificateValidationError_Revoked;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_OTHER) != 0)
    {
        return SOPC_CertificateValidationError_Untrusted;
    }

    return SOPC_CertificateValidationError_Unknown;
}

static int verify_cert(void* checkTrustedAndCRL, mbedtls_x509_crt* crt, int certificate_depth, uint32_t* flags)
{
    SOPC_CheckTrustedAndCRLinChain* checkTrustedAndCRLinChain = (SOPC_CheckTrustedAndCRLinChain*) checkTrustedAndCRL;

    /* Check if a revocation list is present when not the certificate leaf
       (that might have CA bit set if self-signed and OPC UA backward compatibility active) */
    /*
     * Note: it is necessary because mbedtls make it our responsibility:
     * It is your responsibility to provide up-to-date CRLs for all trusted CAs. If no CRL is provided for the CA
     * that was used to sign the certificate, CRL verification is skipped silently, that is without setting any flag.
     */
    if (0 != certificate_depth && !checkTrustedAndCRLinChain->disableRevocationCheck)
    {
        bool matchCRL = false;
        // Unlink cert temporarily for CRL verification
        mbedtls_x509_crt* backupNextCrt = crt->next;
        crt->next = NULL;
        SOPC_CertificateList certList = {.crt = *crt};
        SOPC_ReturnStatus status = SOPC_STATUS_NOK;
        if (NULL != checkTrustedAndCRLinChain->allCRLs)
        {
            status = SOPC_KeyManager_CertificateList_CheckCRL(&certList, checkTrustedAndCRLinChain->allCRLs, &matchCRL);
        }
        // Restore link
        crt->next = backupNextCrt;
        if (SOPC_STATUS_OK != status)
        {
            matchCRL = false;
        }
        if (!matchCRL)
        {
            *flags = *flags | MBEDTLS_X509_BADCRL_NOT_TRUSTED;
        }
    }

    /* Check if certificate chain element is part of PKI trusted certificates */
    const mbedtls_x509_crt* crtTrusted = NULL;
    if (NULL != checkTrustedAndCRLinChain->trustedCrts)
    {
        crtTrusted = &checkTrustedAndCRLinChain->trustedCrts->crt; /* Current cert */
    }
    while (!checkTrustedAndCRLinChain->isTrustedInChain && NULL != crtTrusted)
    {
        if (crt->subject_raw.len == crtTrusted->subject_raw.len && crt->raw.len == crtTrusted->raw.len &&
            0 == memcmp(crt->subject_raw.p, crtTrusted->subject_raw.p, crt->subject_raw.len) &&
            0 == memcmp(crt->raw.p, crtTrusted->raw.p, crt->subject_raw.len))
        {
            checkTrustedAndCRLinChain->isTrustedInChain = true;
        }
        crtTrusted = crtTrusted->next;
    }

    /* Only fatal errors could be returned here, as this error code will be forwarded to the caller of
     * mbedtls_x509_crt_verify_with_profile, and the verification stopped.
     * Errors may be MBEDTLS_ERR_X509_FATAL_ERROR, or application specific */
    return 0;
}

// removes cert from a linked list
static void sopc_remove_cert_from_list(mbedtls_x509_crt* pPrev,
                                       mbedtls_x509_crt** ppCur,
                                       SOPC_CertificateList** ppHeadCertList);

// PKI remove certificate from rejected list declaration
static void sopc_pki_remove_rejected_cert(SOPC_CertificateList** ppRejectedList, const SOPC_CertificateList* pCert);

static SOPC_ReturnStatus cert_is_self_signed(mbedtls_x509_crt* crt, bool* pbIsSelfSign)
{
    SOPC_ASSERT(NULL != crt);

    SOPC_CertificateList cert = {.crt = *crt};
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_IsSelfSigned(&cert, pbIsSelfSign);
    if (SOPC_STATUS_OK != status)
    {
        char* pThumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(&cert);
        const char* thumbprint = NULL == pThumbprint ? "NULL" : pThumbprint;
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_COMMON,
            "> PKI unexpected error : failed to run a self-signature check for certificate thumbprint %s", thumbprint);
        SOPC_Free(pThumbprint);
    }
    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckCommonName(const SOPC_CertificateList* pToValidate)
{
    SOPC_ASSERT(NULL != pToValidate);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    char* pThumb = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidate);
    const char* thumb = NULL != pThumb ? pThumb : "NULL";

    int found = -1;
    const mbedtls_x509_name* attribute = &pToValidate->crt.subject;
    /* mbedtls_asn1_find_named_data is not used because of const condition */
    while (NULL != attribute && 0 != found)
    {
        found = MBEDTLS_OID_CMP(MBEDTLS_OID_AT_CN, &attribute->oid);
        if (0 == found && NULL == attribute->val.p)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                     "> PKI validation : The Common Name attribute is specified with an empty name for "
                                     "certificate thumbprint %s",
                                     thumb);
        }
        attribute = attribute->next;
    }
    if (0 != found)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_COMMON,
            "> PKI validation failed : The Common Name attribute is not specified for certificate thumbprint %s",
            thumb);
        status = SOPC_STATUS_NOK;
    }
    SOPC_Free(pThumb);
    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckSecurityPolicy(const SOPC_CertificateList* pToValidate,
                                                       const SOPC_PKI_LeafProfile* pConfig)
{
    SOPC_ASSERT(NULL != pToValidate);
    SOPC_ASSERT(NULL != pConfig);

    SOPC_AsymmetricKey pub_key;
    size_t keyLenBits = 0;
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
    // Retrieve key type
    mbedtls_pk_type_t key_type = mbedtls_pk_get_type(&pub_key.pk);
    // Verifies key type: RSA
    switch (pConfig->pkAlgo)
    {
    case SOPC_PKI_PK_ANY:
        break;
    case SOPC_PKI_PK_RSA:
        if (SOPC_PKI_PK_RSA == pConfig->pkAlgo && MBEDTLS_PK_RSA != key_type)
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
                               "> PKI validation failed : unexpected key type %d"
                               " for certificate thumbprint %s",
                               (const int) key_type, thumbprint);
        status = SOPC_STATUS_NOK;
    }
    // Retrieve key length
    keyLenBits = mbedtls_pk_get_bitlen(&pub_key.pk);
    // Verifies key length: min-max
    if (keyLenBits < pConfig->RSAMinimumKeySize || keyLenBits > pConfig->RSAMaximumKeySize)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed : unexpected key size %" PRIu32
                               " for certificate thumbprint %s",
                               (uint32_t) keyLenBits, thumbprint);
        status = SOPC_STATUS_NOK;
    }
    // Verifies signing algorithm:
    mbedtls_md_type_t md = pToValidate->crt.sig_md;
    bErr = false;
    switch (pConfig->mdSign)
    {
    case SOPC_PKI_MD_SHA1_OR_ABOVE:
        if (MBEDTLS_MD_SHA1 != md && MBEDTLS_MD_SHA224 != md && MBEDTLS_MD_SHA256 != md && MBEDTLS_MD_SHA384 != md &&
            MBEDTLS_MD_SHA512 != md)
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA256_OR_ABOVE:
        if (MBEDTLS_MD_SHA256 != md && MBEDTLS_MD_SHA384 != md && MBEDTLS_MD_SHA512 != md)
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA1:
        if (MBEDTLS_MD_SHA1 != md)
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA256:
        if (MBEDTLS_MD_SHA256 != md)
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA1_AND_SHA256:
        if (MBEDTLS_MD_SHA1 != md && MBEDTLS_MD_SHA256 != md)
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
                               "> PKI validation failed : unexpected signing algorithm %d"
                               " for certificate thumbprint %s",
                               (int) md, thumbprint);
        status = SOPC_STATUS_NOK;
    }

    SOPC_Free(thumbprint);
    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckHostName(const SOPC_CertificateList* pToValidate, const char* url)
{
    // TODO : Add a domain name resolution (issue #1189)

    if (NULL == pToValidate || NULL == url)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

#if MBEDTLS_CAN_RESOLVE_HOSTNAME
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
    const mbedtls_x509_sequence* asn1_seq = &pToValidate->crt.subject_alt_names;
    mbedtls_x509_subject_alternative_name san_out = {0};
    int err = 0;
    bool found = false;
    int match = -1;
    while (NULL != asn1_seq && !found && 0 == err)
    {
        const mbedtls_x509_buf* pBuf = &san_out.san.unstructured_name;
        err = mbedtls_x509_parse_subject_alt_name(&asn1_seq->buf, &san_out);
        if (MBEDTLS_ERR_X509_FEATURE_UNAVAILABLE == err)
        {
            /* Only "dnsName" and "otherName" is supported by mbedtls */
            err = 0;
        }
        if (0 != err)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                   "> PKI validation failed : Subject Alternative Name parse failed with error code:: "
                                   "-0x%X for certificate thumbprint %s",
                                   (unsigned int) -err, thumbprint);
        }
        if (MBEDTLS_X509_SAN_DNS_NAME == san_out.type)
        {
            if (pBuf->len == hostnameLen)
            {
                match = SOPC_strncmp_ignore_case(pHostName, (const char*) pBuf->p, pBuf->len);
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
                    "> PKI validation : dnsName %.*s of certificate thumbprint %s is not the expected one (%s)",
                    (int) pBuf->len, (const char*) pBuf->p, thumbprint, pHostName);
            }
        }
        /* next iteration */
        memset(&san_out, 0, sizeof(mbedtls_x509_subject_alternative_name));
        asn1_seq = asn1_seq->next;
    }
    if (!found || 0 != err)
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
#else
    /* Not implemented in version prior to 2.28.0 */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                             "> PKI validation skipped as mbedtls_x509_parse_subject_alt_name is not implemented in "
                             "version %d.%d.%d of MbedTLS",
                             MBEDTLS_VERSION_MAJOR, MBEDTLS_VERSION_MINOR, MBEDTLS_VERSION_PATCH);
#endif
    return status;
}

static unsigned int get_lib_ku_from_sopc_ku(const SOPC_PKI_KeyUsage_Mask sopc_pki_ku)
{
    unsigned int usages = 0;
    if (SOPC_PKI_KU_DIGITAL_SIGNATURE & sopc_pki_ku)
    {
        usages |= MBEDTLS_X509_KU_DIGITAL_SIGNATURE;
    }
    if (SOPC_PKI_KU_NON_REPUDIATION & sopc_pki_ku)
    {
        usages |= MBEDTLS_X509_KU_NON_REPUDIATION;
    }
    if (SOPC_PKI_KU_KEY_ENCIPHERMENT & sopc_pki_ku)
    {
        usages |= MBEDTLS_X509_KU_KEY_ENCIPHERMENT;
    }
    if (SOPC_PKI_KU_KEY_DATA_ENCIPHERMENT & sopc_pki_ku)
    {
        usages |= MBEDTLS_X509_KU_DATA_ENCIPHERMENT;
    }
    if (SOPC_PKI_KU_KEY_CERT_SIGN & sopc_pki_ku)
    {
        usages |= MBEDTLS_X509_KU_KEY_CERT_SIGN;
    }
    if (SOPC_PKI_KU_KEY_CRL_SIGN & sopc_pki_ku)
    {
        usages |= MBEDTLS_X509_KU_CRL_SIGN;
    }

    return usages;
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
    int err = 0;
    bool bErrorFound = false;
    /* Check key usages */
    usages = get_lib_ku_from_sopc_ku(pProfile->keyUsage);
    err = mbedtls_x509_crt_check_key_usage(&pToValidate->crt, usages);
    if (0 != err)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_COMMON,
            "> PKI validation error '0x%04X': missing expected key usage for certificate thumbprint %s", err,
            thumbprint);
        bErrorFound = true;
    }
    /* Check extended key usages for client or server cert */
    err = 0;
    if (SOPC_PKI_EKU_SERVER_AUTH & pProfile->extendedKeyUsage)
    {
        err |= mbedtls_x509_crt_check_extended_key_usage(&pToValidate->crt, MBEDTLS_OID_SERVER_AUTH,
                                                         MBEDTLS_OID_SIZE(MBEDTLS_OID_SERVER_AUTH));
    }
    if (SOPC_PKI_EKU_CLIENT_AUTH & pProfile->extendedKeyUsage)
    {
        err |= mbedtls_x509_crt_check_extended_key_usage(&pToValidate->crt, MBEDTLS_OID_CLIENT_AUTH,
                                                         MBEDTLS_OID_SIZE(MBEDTLS_OID_CLIENT_AUTH));
    }
    if (0 != err)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_COMMON,
            "> PKI validation error '-0x%04X': missing expected extended key usage for certificate thumbprint %s", -err,
            thumbprint);
        bErrorFound = true;
    }

    SOPC_Free(thumbprint);
    if (bErrorFound)
    {
        return SOPC_STATUS_NOK;
    }
    else
    {
        return SOPC_STATUS_OK;
    }
}

static SOPC_ReturnStatus set_profile_from_configuration(const SOPC_PKI_ChainProfile* pProfile,
                                                        mbedtls_x509_crt_profile* pLibProfile)
{
    /* Set hashes allowed */
    if (SOPC_PKI_MD_SHA1_OR_ABOVE == pProfile->mdSign)
    {
        pLibProfile->allowed_mds = MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA1) | MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA224) |
                                   MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256) | MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA384) |
                                   MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA512);
    }
    else if (SOPC_PKI_MD_SHA256_OR_ABOVE == pProfile->mdSign)
    {
        pLibProfile->allowed_mds = MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256) | MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA384) |
                                   MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA512);
    }
    else if (SOPC_PKI_MD_SHA1 == pProfile->mdSign)
    {
        pLibProfile->allowed_mds = MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA1);
    }
    else if (SOPC_PKI_MD_SHA256 == pProfile->mdSign)
    {
        pLibProfile->allowed_mds = MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256);
    }
    else if (SOPC_PKI_MD_SHA1_AND_SHA256 == pProfile->mdSign)
    {
        pLibProfile->allowed_mds = MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA1) | MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256);
    }
    else
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* Set PK algo allowed */
    if (SOPC_PKI_PK_ANY == pProfile->pkAlgo)
    {
        pLibProfile->allowed_pks = UINT32_MAX;
    }
    else if (SOPC_PKI_PK_RSA == pProfile->pkAlgo)
    {
        pLibProfile->allowed_pks = MBEDTLS_X509_ID_FLAG(MBEDTLS_PK_RSA);
    }
    else
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* Set curve allowed */
    if (SOPC_PKI_CURVES_ANY == pProfile->curves)
    {
        pLibProfile->allowed_curves = UINT32_MAX;
    }
    else
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* Set minimum RSA key size allowed */
    pLibProfile->rsa_min_bitlen = pProfile->RSAMinimumKeySize;

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus sopc_validate_certificate(
    const SOPC_PKIProvider* pPKI,
    mbedtls_x509_crt* mbed_cert,
    mbedtls_x509_crt_profile* mbed_profile,
    bool bIsSelfSigned,           /* Certificate is self-signed: added with root CAs for validation */
    bool bForceTrustedCert,       /* Force the certificate to be trusted for the validation:
                                     for sopc_verify_every_certificate only */
    bool bDisableRevocationCheck, /* When flag is set, no error is reported if a CA certificate has no revocation list.
                                   */
    const char* thumbprint,
    uint32_t* error)
{
    SOPC_ASSERT(NULL != pPKI);
    SOPC_ASSERT(NULL != mbed_cert);
    SOPC_ASSERT(NULL == mbed_cert->next);
    SOPC_ASSERT(NULL != mbed_profile);
    SOPC_ASSERT(NULL != thumbprint);
    SOPC_ASSERT(NULL != error);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_CRLList* cert_crl = pPKI->pAllCrl;
    /* Assumes that mbedtls does not modify the certificates */
    /*
     * Note: we consider all root CAs as trusted since our criteria is that at least one certificate is trusted in the
     *       chain and it might not be the root CA.
     *       We do an additional check that it is the case during the verification.
     */
    mbedtls_x509_crt* mbed_ca_root = (mbedtls_x509_crt*) (NULL != pPKI->pAllRoots ? &pPKI->pAllRoots->crt : NULL);
    mbedtls_x509_crl* mbed_crl = (mbedtls_x509_crl*) (NULL != cert_crl ? &cert_crl->crl : NULL);
    /* Link certificate to validate with intermediate certificates (trusted links or untrusted links) */
    mbedtls_x509_crt* pLinkCert = (NULL != pPKI->pAllCerts ? &pPKI->pAllCerts->crt : NULL);

    /* Add self-signed to validate to root CAs */
    /*
     * Note from MbedTLS API:
     * The trust_ca list can contain two types of certificates: (1) those of trusted root CAs, so that certificates
     * chaining up to those CAs will be trusted, and (2) self-signed end-entity certificates to be trusted (for specific
     * peers you know) - in that case, the self-signed certificate doesn’t need to have the CA bit set.
     */
    mbedtls_x509_crt* lastRoot = NULL;
    if (bIsSelfSigned)
    {
        if (NULL == mbed_ca_root)
        {
            // Set self-signed validating cert as single element
            mbed_ca_root = mbed_cert;
        }
        else
        {
            lastRoot = mbed_ca_root;
            while (NULL != lastRoot->next)
            {
                lastRoot = lastRoot->next;
            }
            // Set self-signed validating cert as last element
            lastRoot->next = mbed_cert;
        }
    }
    else /* If certificate is not self signed, add the intermediate certificates to the trust chain to evaluate */
    {
        mbed_cert->next = pLinkCert;
    }

    SOPC_CheckTrustedAndCRLinChain checkTrustedAndCRL = {.trustedCrts = pPKI->pAllTrusted,
                                                         .allCRLs = pPKI->pAllCrl,
                                                         .isTrustedInChain = bForceTrustedCert,
                                                         .disableRevocationCheck = bDisableRevocationCheck};
    /* Verify the certificate chain */
    uint32_t failure_reasons = 0;
    int ret = mbedtls_x509_crt_verify_with_profile(mbed_cert, mbed_ca_root, mbed_crl, mbed_profile, NULL,
                                                   &failure_reasons, verify_cert, &checkTrustedAndCRL);
    // Check if at a least one trusted certificate is present in trust chain
    if (!checkTrustedAndCRL.isTrustedInChain)
    {
        ret = -1;
        failure_reasons = (failure_reasons | (uint32_t) MBEDTLS_X509_BADCERT_NOT_TRUSTED);
    }
    if (0 != ret)
    {
        *error = PKIProviderStack_GetCertificateValidationError(failure_reasons);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed with error code 0x%" PRIX32 " (mbedtls code 0x%" PRIX32
                               ") for certificate thumbprint %s",
                               *error, failure_reasons, thumbprint);
        status = SOPC_STATUS_NOK;
    }
    /* Unlink mbed_cert from root CAs if it was added */
    if (NULL != lastRoot)
    {
        lastRoot->next = NULL;
    }
    /* Unlink intermediate CAs from mbed_cert,
       otherwise destroying the pToValidate will also destroy trusted or untrusted links */
    mbed_cert->next = NULL;
    return status;
}

static void sopc_remove_cert_from_list(mbedtls_x509_crt* pPrev,
                                       mbedtls_x509_crt** ppCur,
                                       SOPC_CertificateList** ppHeadCertList)
{
    SOPC_ASSERT(NULL != ppCur);
    SOPC_ASSERT(NULL != *ppCur); /* Current cert shall not be NULL */
    SOPC_ASSERT(NULL != ppHeadCertList);
    SOPC_ASSERT(NULL != *ppHeadCertList); /* Head shall not be NULL */

    SOPC_CertificateList* pHeadCertList = *ppHeadCertList;
    mbedtls_x509_crt* pCur = *ppCur;      /* Current cert */
    mbedtls_x509_crt* pNext = pCur->next; /* Next cert */
    pCur->next = NULL;
    mbedtls_x509_crt_free(pCur);
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
            pHeadCertList->crt = *pNext; /* Use an assignment operator to do the copy */
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
    SOPC_ASSERT(NULL == pCert->crt.next);

    /* Head of the rejected list */
    SOPC_CertificateList* pHeadRejectedCertList = *ppRejectedList;
    if (NULL == pHeadRejectedCertList)
    {
        /* the certificate list is empty, do nothing*/
        return;
    }
    const mbedtls_x509_crt* crt = &pCert->crt;
    mbedtls_x509_crt* cur = &pHeadRejectedCertList->crt; /* Current cert */
    mbedtls_x509_crt* prev = NULL;                       /* Parent of current cert */
    bool bFound = false;

    while (NULL != cur && !bFound)
    {
        if (crt->subject_raw.len == cur->subject_raw.len && crt->raw.len == cur->raw.len &&
            0 == memcmp(crt->subject_raw.p, cur->subject_raw.p, crt->subject_raw.len) &&
            0 == memcmp(crt->raw.p, cur->raw.p, crt->subject_raw.len))
        {
            bFound = true;
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

    bool bFound = false; // indicating whether the certificate is already in the rejected certificates list
    size_t listLength = 0;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_GetListLength(pCert, &listLength);
    if (SOPC_STATUS_OK != status || 1 != listLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (UINT32_MAX < pCert->crt.raw.len)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    listLength = 0;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (NULL != pPKI->pRejectedList)
    {
        status = SOPC_KeyManager_CertificateList_FindCertInList(pPKI->pRejectedList, pCert, &bFound);
        if (SOPC_STATUS_OK == status && !bFound)
        {
            status = SOPC_KeyManager_Certificate_GetListLength(pPKI->pRejectedList, &listLength);
            /* Remove the oldest certificate (HEAD of the chain) if the rejected list is too large */
            if (SOPC_STATUS_OK == status)
            {
                if (SOPC_PKI_MAX_NB_CERT_REJECTED == listLength)
                {
                    /* Change the HEAD */
                    mbedtls_x509_crt* cur = &pPKI->pRejectedList->crt;
                    mbedtls_x509_crt* next = cur->next;
                    cur->next = NULL;
                    if (NULL == next)
                    {
                        /* case where SOPC_PKI_MAX_NB_CERT_REJECTED == 1 */
                        SOPC_KeyManager_Certificate_Free(pPKI->pRejectedList);
                        pPKI->pRejectedList = NULL;
                    }
                    else
                    {
                        mbedtls_x509_crt_free(cur);
                        pPKI->pRejectedList->crt = *next; /* Copy with an assignment operator */
                        SOPC_Free(next);
                    }
                }
            }
        }
    }
    if (SOPC_STATUS_OK == status && !bFound)
    {
        /* Create the rejected list if empty or append at the end */
        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(pCert->crt.raw.p, (uint32_t) pCert->crt.raw.len,
                                                                &pPKI->pRejectedList);
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

SOPC_ReturnStatus SOPC_PKIProvider_ValidateProfileAndCertificate(SOPC_PKIProvider* pPKI,
                                                                 const SOPC_CertificateList* pToValidate,
                                                                 const SOPC_PKI_Profile* pProfile,
                                                                 uint32_t* error)
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
    mbedtls_x509_crt crt = pToValidateCpy->crt;
    bool bIsSelfSigned = false;
    char* pThumbprint = NULL;
    const char* thumbprint = NULL;
    status = cert_is_self_signed(&crt, &bIsSelfSigned);
    if (SOPC_STATUS_OK != status)
    {
        /* unexpected error : failed to run a self-signature */
        SOPC_KeyManager_Certificate_Free(pToValidateCpy);
        return status;
    }
    pThumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidateCpy);
    thumbprint = NULL == pThumbprint ? "NULL" : pThumbprint;
    /* Certificate shall not be a CA or only for self-signed backward compatibility
   (and pathLen shall be 0 which means crt.max_pathlen is 1 due to mbedtls choice: 1 higher than RFC 5280) */
    bool certToValidateConstraints =
        (!pToValidateCpy->crt.ca_istrue ||
         (bIsSelfSigned && pProfile->bBackwardInteroperability && 1 == pToValidateCpy->crt.max_pathlen));
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
        status = SOPC_PKIProvider_CheckLeafCertificate(pToValidateCpy, pProfile->leafProfile, &currentError);
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
    mbedtls_x509_crt_profile crt_profile = {0};
    /* Set the profile from configuration */
    status = set_profile_from_configuration(pProfile->chainProfile, &crt_profile);
    if (SOPC_STATUS_OK == status)
    {
        mbedtls_x509_crt* mbedCertToValidate = (mbedtls_x509_crt*) (&pToValidateCpy->crt);
        status = sopc_validate_certificate(pPKI, mbedCertToValidate, &crt_profile, bIsSelfSigned, false,
                                           pProfile->chainProfile->bDisableRevocationCheck, thumbprint, &currentError);
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
                                                       mbedtls_x509_crt_profile* mbed_profile,
                                                       const bool bDisableRevocationCheck,
                                                       bool* bErrorFound,
                                                       SOPC_Array* pErrors,
                                                       SOPC_Array* pThumbprints)
{
    SOPC_ASSERT(NULL != pPkiCerts);
    SOPC_ASSERT(NULL != mbed_profile);
    SOPC_ASSERT(NULL != pErrors);
    SOPC_ASSERT(NULL != pThumbprints);

    SOPC_CertificateList* pCertsCpy = NULL;
    SOPC_CertificateList crtThumbprint = {0};
    bool bResAppend = true;
    uint32_t error = 0;
    char* thumbprint = NULL;
    mbedtls_x509_crt* crt = NULL;
    mbedtls_x509_crt* save_next = NULL;

    SOPC_ReturnStatus statusChain = SOPC_STATUS_OK;
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_Copy(pPkiCerts, &pCertsCpy);
    if (SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    crt = (mbedtls_x509_crt*) (&pCertsCpy->crt);
    while (NULL != crt && SOPC_STATUS_OK == status)
    {
        bool bIsSelfSigned = false;
        /* unlink crt */
        save_next = crt->next;
        crt->next = NULL;
        /* Get thumbprint */
        crtThumbprint.crt = *crt;
        thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(&crtThumbprint);
        if (NULL == thumbprint)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = cert_is_self_signed(crt, &bIsSelfSigned);
        }
        if (SOPC_STATUS_OK == status)
        {
            // When verifying all certificates, we shall ignore trusted validation since we also validate untrusted ones
            const bool forceTrustedCert = true;
            statusChain = sopc_validate_certificate(pPKI, crt, mbed_profile, bIsSelfSigned, forceTrustedCert,
                                                    bDisableRevocationCheck, thumbprint, &error);
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
    mbedtls_x509_crt_profile crt_profile = {0};
    bool bErrorFound = false;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_Array* pThumbArray = SOPC_Array_Create(sizeof(char*), 0, sopc_free_c_string_from_ptr);
    SOPC_Array* pErrArray = NULL;
    if (NULL != pThumbArray)
    {
        pErrArray = SOPC_Array_Create(sizeof(uint32_t), 0, NULL);
    }
    if (NULL != pErrArray)
    {
        status = set_profile_from_configuration(pProfile, &crt_profile);
    }

    if (SOPC_STATUS_OK == status)
    {
        if (NULL != pPKI->pAllCerts)
        {
            status =
                sopc_verify_every_certificate(pPKI->pAllCerts, pPKI, &crt_profile, pProfile->bDisableRevocationCheck,
                                              &bErrorFound, pErrArray, pThumbArray);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != pPKI->pAllRoots)
        {
            status =
                sopc_verify_every_certificate(pPKI->pAllRoots, pPKI, &crt_profile, pProfile->bDisableRevocationCheck,
                                              &bErrorFound, pErrArray, pThumbArray);
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

/**
 * \brief Delete the roots of the list ppCerts. Create a new list ppRootCa with all roots from ppCerts.
 *        If there is no root, the content of ppRootCa is set to NULL.
 *        If ppCerts becomes empty, its content is set to NULL.
 */
SOPC_ReturnStatus SOPC_PKIProvider_SplitRootFromCertList(SOPC_CertificateList** ppCerts,
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
    mbedtls_x509_crt* cur = &pHeadCerts->crt; /* Start from the HEAD*/
    mbedtls_x509_crt* prev = NULL;            /* Parent of current cert */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    while (NULL != cur && SOPC_STATUS_OK == status)
    {
        bool is_root = true;
        bool self_sign = false;

        /* Skip certificates that are not authorities */
        if (!cur->ca_istrue)
        {
            is_root = false;
        }
        status = cert_is_self_signed(cur, &self_sign);
        if (!self_sign && is_root)
        {
            is_root = false;
        }
        if (is_root && SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(cur->raw.p, (uint32_t) cur->raw.len, &pHeadRoots);

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

void SOPC_PKIProvider_GetListStats(SOPC_CertificateList* pCert,
                                   uint32_t* caCount,
                                   uint32_t* listLength,
                                   uint32_t* rootCount)
{
    if (NULL == pCert)
    {
        return;
    }
    bool is_self_sign = false;
    mbedtls_x509_crt* crt = &pCert->crt;
    while (NULL != crt)
    {
        *listLength = *listLength + 1;
        if (crt->ca_istrue)
        {
            *caCount = *caCount + 1;
            cert_is_self_signed(crt, &is_self_sign);
            if (is_self_sign)
            {
                *rootCount = *rootCount + 1;
            }
        }
        crt = crt->next;
    }
}
