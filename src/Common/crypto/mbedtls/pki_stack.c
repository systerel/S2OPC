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
#include "sopc_crypto_profiles.h"
#include "sopc_filesystem.h"
#include "sopc_helper_string.h"
#include "sopc_helper_uri.h"
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"

#include "key_manager_lib.h"
#include "mbedtls_common.h"

#include "mbedtls/oid.h"

#ifndef STR_TRUSTLIST_NAME
#define STR_TRUSTLIST_NAME "/updatedTrustList"
#endif

#define STR_TRUSTED "/trusted"
#define STR_TRUSTED_CERTS "/trusted/certs"
#define STR_TRUSTED_CRL "/trusted/crl"
#define STR_ISSUERS "/issuers"
#define STR_ISSUERS_CERTS "/issuers/certs"
#define STR_ISSUERS_CRL "/issuers/crl"

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

    return SOPC_CertificateValidationError_Unkown;
}

static int verify_cert(void* is_issued, mbedtls_x509_crt* crt, int certificate_depth, uint32_t* flags)
{
    /* The purpose of this callback is solely to treat self-signed issued certificates.
     * When a certificate is issued and self-signed, mbedtls does not find its parent,
     * and marks it as NOT_TRUSTED.
     * So, for issued certificates that are NOT_TRUSTED, we verify:
     * - it is self-signed
     * - its signature is correct
     * - (dates are already checked by mbedtls)
     * - (signature algorithms are also already checked)
     */
    bool bIssued = *(bool*) is_issued;
    if (bIssued && 0 == certificate_depth &&
        MBEDTLS_X509_BADCERT_NOT_TRUSTED == (*flags & MBEDTLS_X509_BADCERT_NOT_TRUSTED))
    {
        /* Is it self-signed? Issuer and subject are the same.
         * Note: this verification is not sufficient by itself to conclude that the certificate is self-signed,
         * but the self-signature verification is.
         */
        if (crt->issuer_raw.len == crt->subject_raw.len &&
            0 == memcmp(crt->issuer_raw.p, crt->subject_raw.p, crt->issuer_raw.len))
        {
            /* Is it correctly signed? Inspired by x509_crt_check_signature */
            const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(crt->sig_md);
            unsigned char hash[MBEDTLS_MD_MAX_SIZE];

            /* First hash the certificate, then verify it is signed */
            if (mbedtls_md(md_info, crt->tbs.p, crt->tbs.len, hash) == 0)
            {
                if (mbedtls_pk_verify_ext(crt->sig_pk, crt->sig_opts, &crt->pk, crt->sig_md, hash,
                                          mbedtls_md_get_size(md_info), crt->sig.p, crt->sig.len) == 0)
                {
                    /* Finally set the certificate as trusted */
                    *flags = (*flags & ~(uint32_t) MBEDTLS_X509_BADCERT_NOT_TRUSTED);
                }
            }
        }
    }

    /* Only fatal errors could be returned here, as this error code will be forwarded to the caller of
     * mbedtls_x509_crt_verify_with_profile, and the verification stopped.
     * Errors may be MBEDTLS_ERR_X509_FATAL_ERROR, or application specific */
    return 0;
}

typedef SOPC_ReturnStatus SOPC_FnValidateCert(const SOPC_PKIProvider* pPKI,
                                              const SOPC_CertificateList* pToValidate,
                                              const SOPC_PKI_Profile* pProfile,
                                              uint32_t* error);

/**
 * \brief The PKIProvider object for the Public Key Infrastructure.
 */
struct SOPC_PKIProvider
{
    char* directoryStorePath;            /*!< The directory store path of the PKI*/
    SOPC_CertificateList* pTrustedCerts; /*!< Trusted intermediate CA + trusted certificates*/
    SOPC_CertificateList* pTrustedRoots; /*!< Trusted root CA*/
    SOPC_CRLList* pTrustedCrl;           /*!< CRLs of trusted intermediate CA and trusted root CA*/
    SOPC_CertificateList* pIssuerCerts;  /*!< Issuer intermediate CA*/
    SOPC_CertificateList* pIssuerRoots;  /*!< Issuer root CA*/
    SOPC_CRLList* pIssuerCrl;            /*!< CRLs of issuer intermediate CA and issuer root CA*/

    SOPC_CertificateList* pAllCerts;      /*!< Issuer certs + trusted certs (root not included)*/
    SOPC_CertificateList* pAllRoots;      /*!< Issuer roots + trusted roots*/
    SOPC_CRLList* pAllCrl;                /*!< Issuer CRLs + trusted CRLs */
    SOPC_FnValidateCert* pFnValidateCert; /*!< Pointer to validation function*/
    bool isPermissive;                    /*!< Define whatever the PKI is permissive (without security)*/
};

static const SOPC_PKI_KeyUsage_Mask g_appKU = SOPC_PKI_KU_KEY_ENCIPHERMENT | SOPC_PKI_KU_KEY_DATA_ENCIPHERMENT |
                                              SOPC_PKI_KU_DIGITAL_SIGNATURE | SOPC_PKI_KU_NON_REPUDIATION;
static const SOPC_PKI_KeyUsage_Mask g_usrKU =
    SOPC_PKI_KU_DIGITAL_SIGNATURE; // it is not part of the OPC UA but it makes sense to keep it
static const SOPC_PKI_ExtendedKeyUsage_Mask g_clientEKU = SOPC_PKI_EKU_SERVER_AUTH;
static const SOPC_PKI_ExtendedKeyUsage_Mask g_serverEKU = SOPC_PKI_EKU_CLIENT_AUTH;
static const SOPC_PKI_ExtendedKeyUsage_Mask g_userEKU = SOPC_PKI_EKU_DISABLE_CHECK;

static const SOPC_PKI_LeafProfile g_leaf_profile_rsa_sha256_2048_4096 = {.mdSign = SOPC_PKI_MD_SHA256,
                                                                         .pkAlgo = SOPC_PKI_PK_RSA,
                                                                         .RSAMinimumKeySize = 2048,
                                                                         .RSAMaximumKeySize = 4096,
                                                                         .bApplySecurityPolicy = true,
                                                                         .keyUsage = SOPC_PKI_KU_DISABLE_CHECK,
                                                                         .extendedKeyUsage = SOPC_PKI_EKU_DISABLE_CHECK,
                                                                         .sanApplicationUri = NULL,
                                                                         .sanURL = NULL};

static const SOPC_PKI_LeafProfile g_leaf_profile_rsa_sha1_1024_2048 = {.mdSign = SOPC_PKI_MD_SHA1_AND_SHA256,
                                                                       .pkAlgo = SOPC_PKI_PK_RSA,
                                                                       .RSAMinimumKeySize = 1024,
                                                                       .RSAMaximumKeySize = 2048,
                                                                       .bApplySecurityPolicy = true,
                                                                       .keyUsage = SOPC_PKI_KU_DISABLE_CHECK,
                                                                       .extendedKeyUsage = SOPC_PKI_EKU_DISABLE_CHECK,
                                                                       .sanApplicationUri = NULL,
                                                                       .sanURL = NULL};

static const SOPC_PKI_ChainProfile g_chain_profile_rsa_sha256_2048 = {.curves = SOPC_PKI_CURVES_ANY,
                                                                      .mdSign = SOPC_PKI_MD_SHA256_OR_ABOVE,
                                                                      .pkAlgo = SOPC_PKI_PK_RSA,
                                                                      .RSAMinimumKeySize = 2048};

static const SOPC_PKI_ChainProfile g_chain_profile_rsa_sha1_1024 = {.curves = SOPC_PKI_CURVES_ANY,
                                                                    .mdSign = SOPC_PKI_MD_SHA1_OR_ABOVE,
                                                                    .pkAlgo = SOPC_PKI_PK_RSA,
                                                                    .RSAMinimumKeySize = 1024};

static const SOPC_PKI_LeafProfile* get_leaf_profile_from_security_policy(const char* uri)
{
    if (NULL == uri)
    {
        return NULL;
    }

    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI,
                                 strlen(SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI) + 1) == 0)
    {
        return &g_leaf_profile_rsa_sha256_2048_4096;
    }
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI,
                                 strlen(SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI) + 1) == 0)
    {
        return &g_leaf_profile_rsa_sha256_2048_4096;
    }
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Basic256Sha256_URI,
                                 strlen(SOPC_SecurityPolicy_Basic256Sha256_URI) + 1) == 0)
    {
        return &g_leaf_profile_rsa_sha256_2048_4096;
    }
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Basic256_URI, strlen(SOPC_SecurityPolicy_Basic256_URI) + 1) ==
        0)
    {
        return &g_leaf_profile_rsa_sha1_1024_2048;
    }

    return NULL;
}

static const SOPC_PKI_ChainProfile* get_chain_profile_from_security_policy(const char* uri)
{
    if (NULL == uri)
    {
        return NULL;
    }

    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI,
                                 strlen(SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI) + 1) == 0)
    {
        return &g_chain_profile_rsa_sha256_2048;
    }
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI,
                                 strlen(SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI) + 1) == 0)
    {
        return &g_chain_profile_rsa_sha256_2048;
    }
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Basic256Sha256_URI,
                                 strlen(SOPC_SecurityPolicy_Basic256Sha256_URI) + 1) == 0)
    {
        return &g_chain_profile_rsa_sha256_2048;
    }
    if (SOPC_strncmp_ignore_case(uri, SOPC_SecurityPolicy_Basic256_URI, strlen(SOPC_SecurityPolicy_Basic256_URI) + 1) ==
        0)
    {
        return &g_chain_profile_rsa_sha1_1024;
    }

    return NULL;
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateLeafProfile(const char* securityPolicyUri, SOPC_PKI_LeafProfile** ppProfile)
{
    if (NULL == ppProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const SOPC_PKI_LeafProfile* pProfileRef = NULL;
    if (NULL != securityPolicyUri)
    {
        pProfileRef = get_leaf_profile_from_security_policy(securityPolicyUri);
        if (NULL == pProfileRef)
        {
            return SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    SOPC_PKI_LeafProfile* pProfile = SOPC_Calloc(1, sizeof(SOPC_PKI_LeafProfile));
    if (NULL == pProfile)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (NULL != securityPolicyUri)
    {
        *pProfile = *pProfileRef;
    }

    *ppProfile = pProfile;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetUsageFromType(SOPC_PKI_LeafProfile* pProfile, SOPC_PKI_Type PKIType)
{
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    switch (PKIType)
    {
    case SOPC_PKI_TYPE_CLIENT_APP:
        pProfile->keyUsage = g_appKU;
        pProfile->extendedKeyUsage = g_clientEKU;
        break;
    case SOPC_PKI_TYPE_SERVER_APP:
        pProfile->keyUsage = g_appKU;
        pProfile->extendedKeyUsage = g_serverEKU;
        break;
    case SOPC_PKI_TYPE_USER:
        pProfile->keyUsage = g_usrKU;
        pProfile->extendedKeyUsage = g_userEKU;
        break;
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetURI(SOPC_PKI_LeafProfile* pProfile, const char* applicationUri)
{
    if (NULL == pProfile || NULL == applicationUri)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL != pProfile->sanApplicationUri)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    pProfile->sanApplicationUri = SOPC_strdup(applicationUri);
    if (NULL == pProfile->sanApplicationUri)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PKIProvider_LeafProfileSetURL(SOPC_PKI_LeafProfile* pProfile, const char* url)
{
    if (NULL == pProfile || NULL == url)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL != pProfile->sanURL)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    pProfile->sanURL = SOPC_strdup(url);
    if (NULL == pProfile->sanURL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    return SOPC_STATUS_OK;
}

void SOPC_PKIProvider_DeleteLeafProfile(SOPC_PKI_LeafProfile** ppProfile)
{
    if (NULL == ppProfile)
    {
        return;
    }
    SOPC_PKI_LeafProfile* pProfile = *ppProfile;
    if (NULL != pProfile)
    {
        SOPC_Free(pProfile->sanApplicationUri);
        SOPC_Free(pProfile->sanURL);
        SOPC_Free(pProfile);
        *ppProfile = NULL;
    }
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateProfile(const char* securityPolicyUri, SOPC_PKI_Profile** ppProfile)
{
    if (NULL == ppProfile || NULL == securityPolicyUri)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    const SOPC_PKI_ChainProfile* pChainProfileRef = get_chain_profile_from_security_policy(securityPolicyUri);
    if (NULL == pChainProfileRef)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_PKI_ChainProfile* pChainProfile = SOPC_Calloc(1, sizeof(SOPC_PKI_ChainProfile));
    if (NULL == pChainProfile)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    *pChainProfile = *pChainProfileRef;

    SOPC_PKI_LeafProfile* pLeafProfile = NULL;
    SOPC_PKI_Profile* pProfile = NULL;
    SOPC_ReturnStatus status = SOPC_PKIProvider_CreateLeafProfile(securityPolicyUri, &pLeafProfile);
    if (SOPC_STATUS_OK == status)
    {
        pProfile = SOPC_Calloc(1, sizeof(SOPC_PKI_Profile));
        if (NULL == pProfile)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        pProfile->leafProfile = pLeafProfile;
        pProfile->chainProfile = pChainProfile;
        pProfile->bBackwardInteroperability = true;
        pProfile->bApplyLeafProfile = true;
    }
    else
    {
        SOPC_Free(pChainProfile);
        SOPC_PKIProvider_DeleteLeafProfile(&pLeafProfile);
    }
    *ppProfile = pProfile;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetUsageFromType(SOPC_PKI_Profile* pProfile, SOPC_PKI_Type PKIType)
{
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_PKIProvider_LeafProfileSetUsageFromType(pProfile->leafProfile, PKIType);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    switch (PKIType)
    {
    case SOPC_PKI_TYPE_CLIENT_APP:
        pProfile->bApplyLeafProfile = true;
        pProfile->bBackwardInteroperability = true;
        break;
    case SOPC_PKI_TYPE_SERVER_APP:
        pProfile->bApplyLeafProfile = true;
        pProfile->bBackwardInteroperability = true;
        break;
    case SOPC_PKI_TYPE_USER:
        pProfile->bApplyLeafProfile = false;
        pProfile->bBackwardInteroperability = false;
        break;
    default:
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetURI(SOPC_PKI_Profile* pProfile, const char* applicationUri)
{
    return SOPC_PKIProvider_LeafProfileSetURI(pProfile->leafProfile, applicationUri);
}

SOPC_ReturnStatus SOPC_PKIProvider_ProfileSetURL(SOPC_PKI_Profile* pProfile, const char* url)
{
    return SOPC_PKIProvider_LeafProfileSetURL(pProfile->leafProfile, url);
}

void SOPC_PKIProvider_DeleteProfile(SOPC_PKI_Profile** ppProfile)
{
    if (NULL == ppProfile)
    {
        return;
    }
    SOPC_PKI_Profile* pProfile = *ppProfile;
    if (NULL != pProfile)
    {
        SOPC_Free(pProfile->chainProfile);
        SOPC_PKIProvider_DeleteLeafProfile(&pProfile->leafProfile);
        SOPC_Free(pProfile);
        *ppProfile = NULL;
    }
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateMinimalUserProfile(SOPC_PKI_Profile** ppProfile)
{
    /* Minimal profile for the chain.
       The leaf profile is not used for users during the validation process but the user certificate properties
       are checked according to the security policy during the activate session */
    if (NULL == ppProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_PKI_Profile* pProfile = NULL;
    SOPC_ReturnStatus status = SOPC_PKIProvider_CreateProfile(SOPC_SecurityPolicy_Basic256Sha256_URI, &pProfile);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_ProfileSetUsageFromType(pProfile, SOPC_PKI_TYPE_USER);
    }
    *ppProfile = pProfile;
    return status;
}

static SOPC_ReturnStatus cert_is_self_sign(mbedtls_x509_crt* crt, bool* pbIsSelfSign)
{
    SOPC_ASSERT(NULL != crt);

    SOPC_CertificateList cert = {.crt = *crt};
    SOPC_ReturnStatus status = SOPC_KeyManager_Certificate_IsSelfSigned(&cert, pbIsSelfSign);
    if (SOPC_STATUS_OK != status)
    {
        char* thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(&cert);
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_COMMON,
            "> PKI unexpected error : failed to run a self-signature check for certificate thumbprint %s", thumbprint);
        SOPC_Free(thumbprint);
    }
    return status;
}

static SOPC_ReturnStatus check_security_policy(const SOPC_CertificateList* pToValidate,
                                               const SOPC_PKI_LeafProfile* pConfig)
{
    SOPC_ASSERT(NULL != pToValidate);
    SOPC_ASSERT(NULL != pConfig);

    SOPC_AsymmetricKey pub_key;
    size_t keyLenBits = 0;
    bool bErr = false;
    // Retrieve key
    SOPC_ReturnStatus status = KeyManager_Certificate_GetPublicKey(pToValidate, &pub_key);
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
                               "> PKI validation failed : unexpected key type %" PRIu32
                               " for certificate thumbprint %s",
                               key_type, thumbprint);
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
                               "> PKI validation failed : unexpected signing algorithm %" PRIu32
                               " for certificate thumbprint %s",
                               md, thumbprint);
        status = SOPC_STATUS_NOK;
    }

    SOPC_Free(thumbprint);
    return status;
}

static SOPC_ReturnStatus check_host_name(const SOPC_CertificateList* pToValidate, const char* url)
{
    // TODO : Add a domain name resolution (issue #1189)

    SOPC_ASSERT(NULL != pToValidate);
    SOPC_ASSERT(NULL != url);

    char* thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidate);
    if (NULL == thumbprint)
    {
        return SOPC_STATUS_INVALID_STATE;
    }
    SOPC_UriType type = SOPC_URI_UNDETERMINED;
    char* pHostName = NULL;
    char* pPort = NULL;
    SOPC_ReturnStatus status = SOPC_Helper_URI_SplitUri(url, &type, &pHostName, &pPort);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed : Unable to split the given url %s to retrieve the hostName",
                               url);
        SOPC_Free(thumbprint);
        return SOPC_STATUS_NOK;
    }
    const mbedtls_x509_sequence* asn1_seq = &pToValidate->crt.subject_alt_names;
    mbedtls_x509_subject_alternative_name san_out = {0};
    int err = 0;
    bool found = false;
    char* pCertDns = NULL;
    int match = -1;
    while (NULL != asn1_seq && !found && 0 == err)
    {
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
            pCertDns = SOPC_Malloc((san_out.san.unstructured_name.len + 1) * sizeof(char));
            memcpy(pCertDns, san_out.san.unstructured_name.p, san_out.san.unstructured_name.len);
            pCertDns[san_out.san.unstructured_name.len] = '\0';
            match = SOPC_strcmp_ignore_case(pHostName, pCertDns);
            if (0 == match)
            {
                /* stop research */
                found = true;
            }
            else
            {
                SOPC_Logger_TraceWarning(
                    SOPC_LOG_MODULE_COMMON,
                    "> PKI validation : dnsName %s of certificate thumbprint %s is not the expected one (%s)", pCertDns,
                    thumbprint, pHostName);
            }
            SOPC_Free(pCertDns);
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

    return status;
}

static SOPC_ReturnStatus check_application_uri(const SOPC_CertificateList* pToValidate, const char* applicationUri)
{
    SOPC_ASSERT(NULL != pToValidate);
    SOPC_ASSERT(NULL != applicationUri);

    bool ok = SOPC_KeyManager_Certificate_CheckApplicationUri(pToValidate, applicationUri);
    if (!ok)
    {
        char* thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidate);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed : the application URI %s is not stored in the URI SAN "
                               "extension of certificate thumbprint %s",
                               applicationUri, thumbprint);
        SOPC_Free(thumbprint);
        return SOPC_STATUS_NOK;
    }
    return SOPC_STATUS_OK;
}

static unsigned int get_lib_ku_from_sopc_ku(const SOPC_PKI_KeyUsage_Mask sopc_pki_ku)
{
    unsigned int usages = 0;
    if (SOPC_PKI_KU_DISABLE_CHECK & sopc_pki_ku)
    {
        usages = UINT32_MAX; // All allowed
        return usages;
    }
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

static SOPC_ReturnStatus check_certificate_usage(const SOPC_CertificateList* pToValidate,
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
    if (SOPC_PKI_KU_DISABLE_CHECK != pProfile->keyUsage)
    {
        usages = get_lib_ku_from_sopc_ku(pProfile->keyUsage);
        err = mbedtls_x509_crt_check_key_usage(&pToValidate->crt, usages);
        if (err)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                   "> PKI validation failed : missing expected key usage for certificate thumbprint %s",
                                   thumbprint);
            bErrorFound = true;
        }
    }
    /* Check extended key usages for client or server cert */
    bool missing = false;
    if (SOPC_PKI_EKU_SERVER_AUTH & pProfile->extendedKeyUsage)
    {
        missing |= mbedtls_x509_crt_check_extended_key_usage(&pToValidate->crt, MBEDTLS_OID_SERVER_AUTH,
                                                             MBEDTLS_OID_SIZE(MBEDTLS_OID_SERVER_AUTH));
    }
    if (SOPC_PKI_EKU_CLIENT_AUTH & pProfile->extendedKeyUsage)
    {
        missing |= mbedtls_x509_crt_check_extended_key_usage(&pToValidate->crt, MBEDTLS_OID_CLIENT_AUTH,
                                                             MBEDTLS_OID_SIZE(MBEDTLS_OID_CLIENT_AUTH));
    }
    if (SOPC_PKI_EKU_DISABLE_CHECK == pProfile->extendedKeyUsage)
    {
        missing = false;
    }
    if (missing)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation : missing expected extended key usage for certificate thumbprint %s",
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

static SOPC_ReturnStatus sopc_validate_certificate_chain(const SOPC_PKIProvider* pPKI,
                                                         mbedtls_x509_crt* mbed_cert_list,
                                                         mbedtls_x509_crt_profile* mbed_profile,
                                                         bool bIsTrusted,
                                                         const char* thumbprint,
                                                         uint32_t* error)
{
    SOPC_ASSERT(NULL != pPKI);
    SOPC_ASSERT(NULL != mbed_cert_list);
    SOPC_ASSERT(NULL == mbed_cert_list->next);
    SOPC_ASSERT(NULL != mbed_profile);
    SOPC_ASSERT(NULL != error);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_CertificateList* trust_list = bIsTrusted ? pPKI->pAllRoots : pPKI->pTrustedRoots;
    SOPC_CRLList* cert_crl = pPKI->pAllCrl;
    /* Assumes that mbedtls does not modify the certificates */
    mbedtls_x509_crt* mbed_ca_root = (mbedtls_x509_crt*) (NULL != trust_list ? &trust_list->crt : NULL);
    mbedtls_x509_crl* mbed_crl = (mbedtls_x509_crl*) (NULL != cert_crl ? &cert_crl->crl : NULL);
    /* Link certificate to validate with intermediate certificates (trusted links or untrusted links) */
    mbedtls_x509_crt* pLinkCert = NULL;
    if (bIsTrusted)
    {
        if (NULL != pPKI->pAllCerts)
        {
            pLinkCert = &pPKI->pAllCerts->crt;
        }
    }
    else
    {
        if (NULL != pPKI->pTrustedCerts)
        {
            pLinkCert = &pPKI->pTrustedCerts->crt;
        }
    }
    mbed_cert_list->next = pLinkCert;
    /* Verify the certificate chain */
    uint32_t failure_reasons = 0;
    if (mbedtls_x509_crt_verify_with_profile(mbed_cert_list, mbed_ca_root, mbed_crl, mbed_profile, NULL,
                                             &failure_reasons, verify_cert, &bIsTrusted) != 0)
    {
        *error = PKIProviderStack_GetCertificateValidationError(failure_reasons);
        if (NULL != thumbprint)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                   "> PKI validation failed with error code %" PRIu32 " for certificate thumbprint %s",
                                   *error, thumbprint);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI validation failed with error code %" PRIu32 "",
                                   *error);
        }
        status = SOPC_STATUS_NOK;
    }
    /* Unlink mbed_cert_list, otherwise destroying the pToValidate will also destroy trusted or untrusted links */
    mbed_cert_list->next = NULL;
    return status;
}

static SOPC_ReturnStatus sopc_validate_certificate(const SOPC_PKIProvider* pPKI,
                                                   const SOPC_CertificateList* pToValidate,
                                                   const SOPC_PKI_Profile* pProfile,
                                                   uint32_t* error)
{
    *error = SOPC_CertificateValidationError_Unkown;

    if (NULL == pPKI || NULL == pToValidate || NULL == pProfile || NULL == error)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

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

    uint32_t firstError = *error;
    bool bErrorFound = false;
    bool bIsTrusted = false;
    mbedtls_x509_crt crt = pToValidateCpy->crt;
    bool bIsSelfSign = false;
    char* thumbprint = NULL;
    status = cert_is_self_sign(&crt, &bIsSelfSign);
    if (SOPC_STATUS_OK != status)
    {
        /* unexpected error : failed to run a self-signature */
        SOPC_KeyManager_Certificate_Free(pToValidateCpy);
        return status;
    }
    /* It does not matter if thumbprint is NULL */
    thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidateCpy);
    /* If CA root and backward interoperability */
    if (pToValidateCpy->crt.ca_istrue && bIsSelfSign && pProfile->bBackwardInteroperability)
    {
        /* Root is trusted? */
        if (NULL != pPKI->pTrustedRoots)
        {
            status = SOPC_KeyManager_CertificateList_FindCertInList(pPKI->pTrustedRoots, pToValidateCpy, &bIsTrusted);
            SOPC_ASSERT(SOPC_STATUS_OK == status); /* parameters OK */
        }
    }
    else if (!pToValidateCpy->crt.ca_istrue)
    {
        /* Cert is trusted? */
        if (NULL != pPKI->pTrustedCerts)
        {
            status = SOPC_KeyManager_CertificateList_FindCertInList(pPKI->pTrustedCerts, pToValidateCpy, &bIsTrusted);
            SOPC_ASSERT(SOPC_STATUS_OK == status); /* parameters OK */
        }
    }
    else
    {
        /* CA certificates are always rejected (except for roots if backward interoperability is enabled) */
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI validation failed : certificate thumbprint %s is a CA",
                               thumbprint);
        *error = SOPC_CertificateValidationError_UseNotAllowed;
        firstError = *error;
        bErrorFound = true;
    }
    /* Apply verification on the certificate */
    if (pProfile->bApplyLeafProfile)
    {
        status = SOPC_PKIProvider_CheckLeafCertificate(pToValidateCpy, pProfile->leafProfile, error);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                   "> PKI validation failed : bad properties of certificate thumbprint %s", thumbprint);
            if (bErrorFound)
            {
                *error = firstError;
            }
            bErrorFound = true;
            firstError = *error;
        }
    }
    mbedtls_x509_crt_profile crt_profile = {0};
    /* Set the profile from configuration */
    status = set_profile_from_configuration(pProfile->chainProfile, &crt_profile);
    if (SOPC_STATUS_OK == status)
    {
        mbedtls_x509_crt* mbed_cert_list = (mbedtls_x509_crt*) (&pToValidateCpy->crt);
        status = sopc_validate_certificate_chain(pPKI, mbed_cert_list, &crt_profile, bIsTrusted, thumbprint, error);
        if (SOPC_STATUS_NOK == status)
        {
            if (bErrorFound)
            {
                *error = firstError;
            }
            bErrorFound = true;
        }
    }

    SOPC_Free(thumbprint);
    SOPC_KeyManager_Certificate_Free(pToValidateCpy);
    status = bErrorFound ? SOPC_STATUS_NOK : status;
    return status;
}

static SOPC_ReturnStatus sopc_validate_anything(const SOPC_PKIProvider* pPKI,
                                                const SOPC_CertificateList* pToValidate,
                                                const SOPC_PKI_Profile* pProfile,
                                                uint32_t* error)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(pToValidate);
    SOPC_UNUSED_ARG(pProfile);
    SOPC_UNUSED_ARG(error);
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PKIProvider_ValidateCertificate(const SOPC_PKIProvider* pPKI,
                                                       const SOPC_CertificateList* pToValidate,
                                                       const SOPC_PKI_Profile* pProfile,
                                                       uint32_t* error)
{
    if (NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (NULL == pPKI->pFnValidateCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = pPKI->pFnValidateCert(pPKI, pToValidate, pProfile, error);
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
        /* unlink crt */
        save_next = crt->next;
        crt->next = NULL;
        statusChain = sopc_validate_certificate_chain(pPKI, crt, mbed_profile, true, NULL, &error);
        if (SOPC_STATUS_OK != statusChain)
        {
            *bErrorFound = true;
            crtThumbprint.crt = *crt;
            thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(&crtThumbprint);
            if (NULL == thumbprint)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            if (SOPC_STATUS_OK == status)
            {
                /* Append the error */
                bResAppend = SOPC_Array_Append(pErrors, error);
                if (bResAppend)
                {
                    bResAppend = SOPC_Array_Append(pThumbprints, thumbprint);
                }
                status = bResAppend ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
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
        SOPC_Free(thumbprint);
    }

    SOPC_KeyManager_Certificate_Free(pCertsCpy);

    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_VerifyEveryCertificate(const SOPC_PKIProvider* pPKI,
                                                          const SOPC_PKI_ChainProfile* pProfile,
                                                          uint32_t** pErrors,
                                                          char*** ppThumbprints,
                                                          uint32_t* pLength)
{
    if (NULL == pPKI || NULL == pProfile || NULL == pErrors || NULL == ppThumbprints || NULL == pLength)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    mbedtls_x509_crt_profile crt_profile = {0};
    bool bErrorFound = false;

    SOPC_Array* pThumbArray = SOPC_Array_Create(sizeof(char*), 0, sopc_free_c_string_from_ptr);
    if (NULL == pThumbArray)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_Array* pErrArray = SOPC_Array_Create(sizeof(uint32_t), 0, NULL);
    if (NULL == pErrArray)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = set_profile_from_configuration(pProfile, &crt_profile);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != pPKI->pAllCerts)
        {
            status = sopc_verify_every_certificate(pPKI->pAllCerts, pPKI, &crt_profile, &bErrorFound, pErrArray,
                                                   pThumbArray);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != pPKI->pAllRoots)
        {
            status = sopc_verify_every_certificate(pPKI->pAllRoots, pPKI, &crt_profile, &bErrorFound, pErrArray,
                                                   pThumbArray);
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
        return status;
    }

    status = bErrorFound ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_CheckLeafCertificate(const SOPC_CertificateList* pToValidate,
                                                        const SOPC_PKI_LeafProfile* pProfile,
                                                        uint32_t* error)
{
    if (NULL == pToValidate || NULL == pProfile || NULL == error)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *error = SOPC_CertificateValidationError_Unkown;
    uint32_t firstError = *error;
    bool bErrorFound = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (pProfile->bApplySecurityPolicy)
    {
        status = check_security_policy(pToValidate, pProfile);
        if (SOPC_STATUS_OK != status)
        {
            *error = SOPC_CertificateValidationError_PolicyCheckFailed;
            firstError = *error;
            bErrorFound = true;
        }
    }
    if (NULL != pProfile->sanURL)
    {
        status = check_host_name(pToValidate, pProfile->sanURL);
        if (SOPC_STATUS_OK != status)
        {
            *error = SOPC_CertificateValidationError_HostNameInvalid;
            if (bErrorFound)
            {
                *error = firstError;
            }
            bErrorFound = true;
            firstError = *error;
        }
    }
    if (NULL != pProfile->sanApplicationUri)
    {
        status = check_application_uri(pToValidate, pProfile->sanApplicationUri);
        if (SOPC_STATUS_OK != status)
        {
            *error = SOPC_CertificateValidationError_UriInvalid;
            if (bErrorFound)
            {
                *error = firstError;
            }
            bErrorFound = true;
            firstError = *error;
        }
    }
    status = check_certificate_usage(pToValidate, pProfile);
    if (SOPC_STATUS_OK != status)
    {
        *error = SOPC_CertificateValidationError_UseNotAllowed;
        if (bErrorFound)
        {
            *error = firstError;
        }
        bErrorFound = true;
        firstError = *error;
    }

    status = bErrorFound ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
    return status;
}

static SOPC_ReturnStatus load_certificate_or_crl_list(const char* basePath,
                                                      SOPC_CertificateList** ppCerts,
                                                      SOPC_CRLList** ppCrl,
                                                      bool bIscrl,
                                                      bool bDefaultBuild)
{
    SOPC_ASSERT(NULL != basePath);
    if (bIscrl)
    {
        SOPC_ASSERT(NULL != ppCrl && NULL == ppCerts);
    }
    else
    {
        SOPC_ASSERT(NULL == ppCrl && NULL != ppCerts);
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Array* pFilePaths = NULL;
    SOPC_CertificateList* pCerts = NULL;
    SOPC_CRLList* pCrl = NULL;
    char* pFilePath = NULL;
    /* Get the array of path from basePath */
    SOPC_FileSystem_GetDirResult dirRes = SOPC_FileSystem_GetDirFilePaths(basePath, &pFilePaths);
    if (SOPC_FileSystem_GetDir_OK != dirRes)
    {
        if (!bDefaultBuild)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON, "> PKI creation warning: failed to open directory <%s>.",
                                     basePath);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI creation error: failed to open directory <%s>.",
                                   basePath);
        }
        return SOPC_STATUS_NOK;
    }
    /* Get the size and iterate for each item */
    size_t nbFiles = SOPC_Array_Size(pFilePaths);
    for (size_t idx = 0; idx < nbFiles && SOPC_STATUS_OK == status; idx++)
    {
        pFilePath = SOPC_Array_Get(pFilePaths, char*, idx);
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_COMMON, "> PKI loading file <%s>", pFilePath);
        if (bIscrl)
        {
            /* Load CRL */
            status = SOPC_KeyManager_CRL_CreateOrAddFromFile(pFilePath, &pCrl);
        }
        else
        {
            /* Load CERT */
            status = SOPC_KeyManager_Certificate_CreateOrAddFromFile(pFilePath, &pCerts);
        }
    }
    /* Clear in case of error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pCerts);
        SOPC_KeyManager_CRL_Free(pCrl);
    }
    if (bIscrl)
    {
        *ppCrl = pCrl;
    }
    else
    {
        *ppCerts = pCerts;
    }
    /* Always clear */
    SOPC_Array_Delete(pFilePaths);
    return status;
}

static SOPC_ReturnStatus load_certificate_and_crl_list_from_store(const char* basePath,
                                                                  SOPC_CertificateList** ppTrustedCerts,
                                                                  SOPC_CRLList** ppTrustedCrl,
                                                                  SOPC_CertificateList** ppIssuerCerts,
                                                                  SOPC_CRLList** ppIssuerCrl,
                                                                  bool bDefaultBuild)
{
    SOPC_ASSERT(NULL != basePath);
    SOPC_ASSERT(NULL != ppTrustedCerts);
    SOPC_ASSERT(NULL != ppTrustedCrl);
    SOPC_ASSERT(NULL != ppIssuerCerts);
    SOPC_ASSERT(NULL != ppIssuerCrl);
    /* Trusted Certs */
    char* trustedCertsPath = NULL;
    SOPC_ReturnStatus status = SOPC_StrConcat(basePath, STR_TRUSTED_CERTS, &trustedCertsPath);
    if (SOPC_STATUS_OK == status)
    {
        status = load_certificate_or_crl_list(trustedCertsPath, ppTrustedCerts, NULL, false, bDefaultBuild);
    }
    SOPC_Free(trustedCertsPath);
    /* Trusted Crl */
    if (SOPC_STATUS_OK == status)
    {
        char* trustedCrlPath = NULL;
        status = SOPC_StrConcat(basePath, STR_TRUSTED_CRL, &trustedCrlPath);
        if (SOPC_STATUS_OK == status)
        {
            status = load_certificate_or_crl_list(trustedCrlPath, NULL, ppTrustedCrl, true, bDefaultBuild);
        }
        SOPC_Free(trustedCrlPath);
    }
    /* Issuer Certs */
    if (SOPC_STATUS_OK == status)
    {
        char* issuerCertsPath = NULL;
        status = SOPC_StrConcat(basePath, STR_ISSUERS_CERTS, &issuerCertsPath);
        if (SOPC_STATUS_OK == status)
        {
            status = load_certificate_or_crl_list(issuerCertsPath, ppIssuerCerts, NULL, false, bDefaultBuild);
        }
        SOPC_Free(issuerCertsPath);
    }
    /* Issuer Crl */
    if (SOPC_STATUS_OK == status)
    {
        char* issuerCrlPath = NULL;
        status = SOPC_StrConcat(basePath, STR_ISSUERS_CRL, &issuerCrlPath);
        if (SOPC_STATUS_OK == status)
        {
            status = load_certificate_or_crl_list(issuerCrlPath, NULL, ppIssuerCrl, true, bDefaultBuild);
        }
        SOPC_Free(issuerCrlPath);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(*ppTrustedCerts);
        SOPC_KeyManager_Certificate_Free(*ppIssuerCerts);
        SOPC_KeyManager_CRL_Free(*ppTrustedCrl);
        SOPC_KeyManager_CRL_Free(*ppIssuerCrl);
        *ppTrustedCerts = NULL;
        *ppIssuerCerts = NULL;
        *ppTrustedCrl = NULL;
        *ppIssuerCrl = NULL;
    }
    return status;
}

/**
 * \brief Delete the roots of the list ppCerts. Create a new list ppRootCa with all roots from ppCerts.
 *        If there is no root, the content of ppRootCa is set to NULL.
 *        If ppCerts becomes empty, its content is set to NULL.
 */
static SOPC_ReturnStatus split_root_from_cert_list(SOPC_CertificateList** ppCerts, SOPC_CertificateList** ppRootCa)
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
        status = cert_is_self_sign(cur, &self_sign);
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
                mbedtls_x509_crt* next = cur->next;
                cur->next = NULL;
                mbedtls_x509_crt_free(cur);
                /* Set new next certificate (if possible) */
                if (NULL == prev)
                {
                    if (NULL == next)

                    {
                        /* The list is empty, Free it */
                        SOPC_Free(pHeadCerts);
                        pHeadCerts = NULL;
                        cur = NULL; // make iteration stop
                    }
                    else
                    {
                        /* Head of the chain is a special case */
                        pHeadCerts->crt = *next; /* Use an assignment operator to do the copy */
                        /* We have to free the new next certificate */
                        SOPC_Free(next);

                        /* Do not iterate: current certificate has changed with the new head (cur = &pHeadCerts->crt) */
                    }
                }
                else
                {
                    /* We have to free the certificate if it is not the first in the list */
                    SOPC_Free(cur);
                    prev->next = next;
                    /* Iterate */
                    cur = next;
                }
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

static SOPC_ReturnStatus merge_certificates(SOPC_CertificateList* pLeft,
                                            SOPC_CertificateList* pRight,
                                            SOPC_CertificateList** ppRes)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == ppRes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CertificateList* pRes = *ppRes;
    /* Left part */
    if (NULL != pLeft)
    {
        status = SOPC_KeyManager_Certificate_Copy(pLeft, &pRes);
    }
    /* Right part */
    if (NULL != pRight && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_Copy(pRight, &pRes);
    }
    /* clear if error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pRes);
        pRes = NULL;
    }
    *ppRes = pRes;
    return status;
}

static SOPC_ReturnStatus merge_crls(SOPC_CRLList* pLeft, SOPC_CRLList* pRight, SOPC_CRLList** ppRes)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == ppRes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CRLList* pRes = *ppRes;
    /* Left part */
    if (NULL != pLeft)
    {
        status = SOPC_KeyManager_CRL_Copy(pLeft, &pRes);
    }
    /* Right part */
    if (NULL != pRight && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_Copy(pRight, &pRes);
    }
    /* clear if error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_CRL_Free(pRes);
        pRes = NULL;
    }
    *ppRes = pRes;
    return status;
}

static void get_list_stats(SOPC_CertificateList* pCert, uint32_t* caCount, uint32_t* listLength, uint32_t* rootCount)
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
            cert_is_self_sign(crt, &is_self_sign);
            if (is_self_sign)
            {
                *rootCount = *rootCount + 1;
            }
        }
        crt = crt->next;
    }
}

static SOPC_ReturnStatus check_lists(SOPC_CertificateList* pTrustedCerts,
                                     SOPC_CertificateList* pIssuerCerts,
                                     SOPC_CRLList* pTrustedCrl,
                                     SOPC_CRLList* pIssuerCrl,
                                     bool* bTrustedCaFound,
                                     bool* bIssuerCaFound)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    /* Trusted stats */
    uint32_t trusted_ca_count = 0;
    uint32_t trusted_list_length = 0;
    uint32_t issued_cert_count = 0;
    uint32_t trusted_root_count = 0;
    /* Issuer stats */
    uint32_t issuer_ca_count = 0;
    uint32_t issuer_list_length = 0;
    uint32_t issuer_root_count = 0;
    *bTrustedCaFound = false;
    *bIssuerCaFound = false;

    if (NULL == pTrustedCerts)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI creation error: no trusted certificate is provided.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    get_list_stats(pTrustedCerts, &trusted_ca_count, &trusted_list_length, &trusted_root_count);
    issued_cert_count = trusted_list_length - trusted_ca_count;
    /* trusted CA => trusted CRL*/
    if (0 != trusted_ca_count && NULL == pTrustedCrl)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI creation error: trusted CA certificates are provided but no CRL.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* check and warn in case there is no trusted certificates and no trusted root (only trusted intermediate CA). */
    if ((0 == issued_cert_count) && (0 == trusted_root_count))
    {
        /* In this case, no certificates will be accepted. */
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI creation error: no trusted certificate and no trusted root is given: no "
                               "certificates will be accepted.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    get_list_stats(pIssuerCerts, &issuer_ca_count, &issuer_list_length, &issuer_root_count);
    /* issuer CA => issuer CRL*/
    if (0 != issuer_ca_count && NULL == pIssuerCrl)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI creation error: issuer CA certificates are provided but no CRL.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* Check if issuerCerts list is only filled with CA. */
    if (issuer_list_length != issuer_ca_count)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI creation error: not all issuer certificates are CAs.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* check and warn in case there is no trusted certificates but issuer certificates. */
    if ((0 != issuer_ca_count) && (0 == issued_cert_count))
    {
        /* In this case, only trusted root CA will be accepted (if Backward interoperability is enabled). */
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                 "> PKI creation warning: issuer certificates are given but no trusted certificates: "
                                 "only trusted root CA will be accepted (if backward interoperability is enabled)");
    }
    /* check and warn in case no root defined and trusted certificates defined. */
    if ((0 == issuer_root_count) && (0 == trusted_root_count) && (0 != issued_cert_count))
    {
        /* In this case, only trusted self-signed issued certificates will be accepted. */
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                 "> PKI creation warning: no root (CA) defined: only trusted self-signed issued "
                                 "certificates will be accepted without possibility to revoke them (no CRL).");
    }
    *bTrustedCaFound = 0 != trusted_ca_count;
    *bIssuerCaFound = 0 != issuer_ca_count;
    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateFromList(SOPC_CertificateList* pTrustedCerts,
                                                  SOPC_CRLList* pTrustedCrl,
                                                  SOPC_CertificateList* pIssuerCerts,
                                                  SOPC_CRLList* pIssuerCrl,
                                                  SOPC_PKIProvider** ppPKI)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_PKIProvider* pPKI = NULL;
    SOPC_CertificateList* tmp_pTrustedRoots = NULL; /* trusted root CA */
    SOPC_CertificateList* tmp_pIssuerRoots = NULL;  /* issuer root CA */
    SOPC_CertificateList* tmp_pAllRoots = NULL;     /* issuer + trusted roots */
    SOPC_CertificateList* tmp_pAllCerts = NULL;     /* issuer + trusted certs */
    SOPC_CRLList* tmp_pAllCrl = NULL;               /* issuer crl + trusted crl  */

    SOPC_CertificateList* tmp_pTrustedCerts = NULL; /* trusted intermediate CA + trusted certificates */
    SOPC_CRLList* tmp_pTrustedCrl = NULL;           /* CRLs of trusted intermediate CA and trusted root CA */
    SOPC_CertificateList* tmp_pIssuerCerts = NULL;  /* issuer intermediate CA + issuer root CA */
    SOPC_CRLList* tmp_pIssuerCrl = NULL;            /* CRLs of issuer intermediate CA and issuer root CA */
    bool bTrustedCaFound = false;
    bool bIssuerCaFound = false;

    if (NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /*
       - Check that pTrustedCerts is not empty.
       - Check if there are CAs but no CRLs.
       - Check if issuerCerts list is only filled with CA.
       - Check and warn if issuerCerts is not empty but pTrustedCerts is only filed with CA.
         In this case, if there is no root into pTrustedCerts then
         no certificates will be accepted during validation process.
       - Check and warn in case no root defined but trusted certificates defined.
         In this case, only trusted self-signed issued certificates will be accepted.
    */
    if (SOPC_STATUS_OK == status)
    {
        status = check_lists(pTrustedCerts, pIssuerCerts, pTrustedCrl, pIssuerCrl, &bTrustedCaFound, &bIssuerCaFound);
        if (SOPC_STATUS_OK != status)
        {
            return status;
        }
    }
    /* Copy the lists */
    status = SOPC_KeyManager_Certificate_Copy(pTrustedCerts, &tmp_pTrustedCerts);
    if (SOPC_STATUS_OK == status && NULL != pTrustedCrl)
    {
        status = SOPC_KeyManager_CRL_Copy(pTrustedCrl, &tmp_pTrustedCrl);
    }
    if (SOPC_STATUS_OK == status && NULL != pIssuerCerts)
    {
        status = SOPC_KeyManager_Certificate_Copy(pIssuerCerts, &tmp_pIssuerCerts);
    }
    if (SOPC_STATUS_OK == status && NULL != pIssuerCrl)
    {
        status = SOPC_KeyManager_CRL_Copy(pIssuerCrl, &tmp_pIssuerCrl);
    }

    /* Check the CRL-CA association before creating the PKI. */
    bool bTrustedCRL = false;
    bool bIssuerCRL = false;
    if (SOPC_STATUS_OK == status)
    {
        if (bTrustedCaFound)
        {
            status =
                SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(tmp_pTrustedCerts, tmp_pTrustedCrl, &bTrustedCRL);
        }
        else
        {
            bTrustedCRL = true;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (bIssuerCaFound)
        {
            status = SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(tmp_pIssuerCerts, tmp_pIssuerCrl, &bIssuerCRL);
        }
        else
        {
            bIssuerCRL = true;
        }
    }
    if (SOPC_STATUS_OK == status && (!bTrustedCRL || !bIssuerCRL))
    {
        if (!bTrustedCRL)
        {
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_COMMON,
                "> PKI creation warning: Not all certificate authorities in given trusted certificates have a single "
                "certificate revocation list! Certificates issued by these CAs will be refused.");
        }
        if (!bIssuerCRL)
        {
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_COMMON,
                "> PKI creation warning: Not all certificate authorities in given issuer certificates have a single "
                "certificate revocation list! Certificates issued by these CAs will be refused.");
        }
    }
    /* Retrieve the root from list */
    if (SOPC_STATUS_OK == status)
    {
        status = split_root_from_cert_list(&tmp_pTrustedCerts, &tmp_pTrustedRoots);
    }
    if (SOPC_STATUS_OK == status && NULL != tmp_pIssuerCerts)
    {
        status = split_root_from_cert_list(&tmp_pIssuerCerts, &tmp_pIssuerRoots);
    }
    /* Merge trusted and issuer list */
    if (SOPC_STATUS_OK == status)
    {
        status = merge_certificates(tmp_pIssuerCerts, tmp_pTrustedCerts, &tmp_pAllCerts);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = merge_certificates(tmp_pIssuerRoots, tmp_pTrustedRoots, &tmp_pAllRoots);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = merge_crls(tmp_pIssuerCrl, tmp_pTrustedCrl, &tmp_pAllCrl);
    }

    /* Create the PKI */
    if (SOPC_STATUS_OK == status)
    {
        pPKI = SOPC_Calloc(1, sizeof(SOPC_PKIProvider));
        if (NULL == pPKI)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        pPKI->pTrustedRoots = tmp_pTrustedRoots;
        pPKI->pTrustedCerts = tmp_pTrustedCerts;
        pPKI->pTrustedCrl = tmp_pTrustedCrl;
        pPKI->pIssuerRoots = tmp_pIssuerRoots;
        pPKI->pIssuerCerts = tmp_pIssuerCerts;
        pPKI->pIssuerCrl = tmp_pIssuerCrl;
        pPKI->pAllCerts = tmp_pAllCerts;
        pPKI->pAllRoots = tmp_pAllRoots;
        pPKI->pAllCrl = tmp_pAllCrl;
        pPKI->directoryStorePath = NULL;
        pPKI->pFnValidateCert = &sopc_validate_certificate;
        pPKI->isPermissive = false;
    }
    else
    {
        SOPC_KeyManager_Certificate_Free(tmp_pTrustedRoots);
        SOPC_KeyManager_Certificate_Free(tmp_pIssuerRoots);
        SOPC_KeyManager_Certificate_Free(tmp_pAllRoots);
        SOPC_KeyManager_Certificate_Free(tmp_pTrustedCerts);
        SOPC_KeyManager_Certificate_Free(tmp_pIssuerCerts);
        SOPC_KeyManager_Certificate_Free(tmp_pAllCerts);
        SOPC_KeyManager_CRL_Free(tmp_pTrustedCrl);
        SOPC_KeyManager_CRL_Free(tmp_pIssuerCrl);
        SOPC_KeyManager_CRL_Free(tmp_pAllCrl);
        SOPC_Free(pPKI);
    }

    *ppPKI = pPKI;
    return status;
}

static SOPC_ReturnStatus pki_create_from_store(const char* directoryStorePath,
                                               bool bDefaultBuild,
                                               SOPC_PKIProvider** ppPKI)
{
    SOPC_ASSERT(NULL != directoryStorePath);
    SOPC_ASSERT(NULL != ppPKI);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_CertificateList* pTrustedCerts = NULL; /* trusted intermediate CA + trusted certificates */
    SOPC_CertificateList* pIssuerCerts = NULL;  /* issuer intermediate CA */
    SOPC_CRLList* pTrustedCrl = NULL;           /* CRLs of trusted intermediate CA and trusted root CA */
    SOPC_CRLList* pIssuerCrl = NULL;            /* CRLs of issuer intermediate CA and issuer root CA */
    const char* basePath = NULL;
    char* path = NULL;
    char* trust_list_name = NULL;

    /* Select the right folder*/
    if (!bDefaultBuild)
    {
        status = SOPC_StrConcat(directoryStorePath, STR_TRUSTLIST_NAME, &path);
        if (SOPC_STATUS_OK != status)
        {
            return status;
        }
        basePath = path;
    }
    else
    {
        basePath = directoryStorePath;
    }
    /* Load the files from the directory Store path */
    status = load_certificate_and_crl_list_from_store(basePath, &pTrustedCerts, &pTrustedCrl, &pIssuerCerts,
                                                      &pIssuerCrl, bDefaultBuild);
    /* Check if the trustList is empty */
    if (SOPC_STATUS_OK == status && NULL == pTrustedCerts && NULL == pTrustedCrl && NULL == pIssuerCerts &&
        NULL == pIssuerCrl)
    {
        status = SOPC_STATUS_NOK;
        if (!bDefaultBuild)
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON, "> PKI creation warning: certificate store is empty (%s).",
                                     path);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI creation error: certificate store is empty (%s).",
                                   path);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, ppPKI);
    }
    /* if error then try with trusted and issuers folder. */
    if (!bDefaultBuild && SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                 "> PKI creation warning: updated trustList is missing or bad built, switch to trusted "
                                 "and issuers folders.");
        status = pki_create_from_store(directoryStorePath, true, ppPKI);
    }
    /* Copy the directoryStorePath */
    if (SOPC_STATUS_OK == status)
    {
        /* Copy only if not done during the recursive call. */
        if (NULL == (*ppPKI)->directoryStorePath)
        {
            (*ppPKI)->directoryStorePath = SOPC_strdup(directoryStorePath);
            if (NULL == (*ppPKI)->directoryStorePath)
            {
                SOPC_PKIProvider_Free(ppPKI);
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
        }
    }

    /* Clear */
    SOPC_Free(path);
    SOPC_Free(trust_list_name);

    SOPC_KeyManager_Certificate_Free(pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pIssuerCerts);
    SOPC_KeyManager_CRL_Free(pTrustedCrl);
    SOPC_KeyManager_CRL_Free(pIssuerCrl);

    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_CreateFromStore(const char* directoryStorePath, SOPC_PKIProvider** ppPKI)
{
    if (NULL == directoryStorePath || NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = pki_create_from_store(directoryStorePath, false, ppPKI);
    return status;
}

SOPC_ReturnStatus SOPC_PKIPermissiveNew_Create(SOPC_PKIProvider** ppPKI)
{
    SOPC_PKIProvider* pPKI = NULL;

    if (NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pPKI = SOPC_Malloc(sizeof(SOPC_PKIProvider));

    if (NULL == pPKI)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    pPKI->pTrustedRoots = NULL;
    pPKI->pTrustedCerts = NULL;
    pPKI->pTrustedCrl = NULL;
    pPKI->pIssuerRoots = NULL;
    pPKI->pIssuerCerts = NULL;
    pPKI->pIssuerCrl = NULL;
    pPKI->pAllCerts = NULL;
    pPKI->pAllRoots = NULL;
    pPKI->pAllCrl = NULL;
    pPKI->directoryStorePath = NULL;
    pPKI->pFnValidateCert = &sopc_validate_anything;
    pPKI->isPermissive = true;
    *ppPKI = pPKI;
    return SOPC_STATUS_OK;
}

static void sopc_pki_clear(SOPC_PKIProvider* pPKI)
{
    if (NULL == pPKI)
    {
        return;
    }
    SOPC_KeyManager_Certificate_Free(pPKI->pTrustedRoots);
    SOPC_KeyManager_Certificate_Free(pPKI->pIssuerRoots);
    SOPC_KeyManager_Certificate_Free(pPKI->pAllRoots);
    SOPC_KeyManager_Certificate_Free(pPKI->pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pPKI->pIssuerCerts);
    SOPC_KeyManager_Certificate_Free(pPKI->pAllCerts);
    SOPC_KeyManager_CRL_Free(pPKI->pTrustedCrl);
    SOPC_KeyManager_CRL_Free(pPKI->pIssuerCrl);
    SOPC_KeyManager_CRL_Free(pPKI->pAllCrl);
    SOPC_Free(pPKI->directoryStorePath);
}

void SOPC_PKIProvider_Free(SOPC_PKIProvider** ppPKI)
{
    if (NULL == ppPKI)
    {
        return;
    }
    SOPC_PKIProvider* pPKI = *ppPKI;
    if (NULL == pPKI)
    {
        return;
    }
    sopc_pki_clear(pPKI);
    SOPC_Free(pPKI);
    pPKI = NULL;
}

#if SOPC_HAS_FILESYSTEM
static SOPC_ReturnStatus remove_files(const char* directoryPath)
{
    SOPC_ASSERT(NULL != directoryPath);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Array* pFilePaths = NULL;
    char* pFilePath = NULL;
    int res = -1;
    /* Get the array of file paths from the given directory */
    SOPC_FileSystem_GetDirResult dirRes = SOPC_FileSystem_GetDirFilePaths(directoryPath, &pFilePaths);
    if (SOPC_FileSystem_GetDir_OK != dirRes)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI write to store: failed to open directory <%s>.",
                               directoryPath);
        return SOPC_STATUS_NOK;
    }
    size_t nbFiles = SOPC_Array_Size(pFilePaths);
    for (size_t idx = 0; idx < nbFiles && SOPC_STATUS_OK == status; idx++)
    {
        pFilePath = SOPC_Array_Get(pFilePaths, char*, idx);
        res = remove(pFilePath);
        if (0 != res)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    SOPC_Array_Delete(pFilePaths);
    return status;
}
#else
static SOPC_ReturnStatus remove_files(const char* directoryPath)
{
    SOPC_UNUSED_ARG(directoryPath);
    return SOPC_STATUS_NOT_SUPPORTED;
}
#endif /* SOPC_HAS_FILESYSTEM */

static SOPC_ReturnStatus write_cert_to_der_files(SOPC_CertificateList* pRoots,
                                                 SOPC_CertificateList* pCerts,
                                                 const char* directoryPath,
                                                 const bool bEraseExistingFiles)
{
    SOPC_ASSERT(NULL != directoryPath);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (bEraseExistingFiles)
    {
        status = remove_files(directoryPath);
    }
    if (SOPC_STATUS_OK == status && NULL != pRoots)
    {
        status = SOPC_KeyManager_Certificate_ToDER_Files(pRoots, directoryPath);
    }
    if (SOPC_STATUS_OK == status && NULL != pCerts)
    {
        status = SOPC_KeyManager_Certificate_ToDER_Files(pCerts, directoryPath);
    }
    return status;
}

static SOPC_ReturnStatus write_crl_to_der_files(SOPC_CRLList* pCrl,
                                                const char* directoryPath,
                                                const bool bEraseExistingFiles)
{
    SOPC_ASSERT(NULL != directoryPath);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (bEraseExistingFiles)
    {
        status = remove_files(directoryPath);
    }
    if (SOPC_STATUS_OK == status && NULL != pCrl)
    {
        status = SOPC_KeyManager_CRL_ToDER_Files(pCrl, directoryPath);
    }
    return status;
}

static SOPC_ReturnStatus may_create_pki_folder(const char* pBasePath, const char* pSubPath, char** ppPath)
{
    SOPC_FileSystem_CreationResult mkdir_res = SOPC_FileSystem_Creation_Error_UnknownIssue;
    char* pPath = NULL;
    SOPC_ReturnStatus status = SOPC_StrConcat(pBasePath, pSubPath, &pPath);
    if (SOPC_STATUS_OK == status)
    {
        mkdir_res = SOPC_FileSystem_mkdir(pPath);
        if (SOPC_FileSystem_Creation_Error_PathAlreadyExists != mkdir_res && SOPC_FileSystem_Creation_OK != mkdir_res)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pPath);
        pPath = NULL;
    }
    *ppPath = pPath;
    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_SetStorePath(const char* directoryStorePath, SOPC_PKIProvider* pPKI)
{
    if (NULL == pPKI || NULL == directoryStorePath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (pPKI->isPermissive)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Create if necessary the store */
    SOPC_FileSystem_CreationResult mkdir_res = SOPC_FileSystem_mkdir(directoryStorePath);
    if (SOPC_FileSystem_Creation_Error_PathAlreadyExists != mkdir_res && SOPC_FileSystem_Creation_OK != mkdir_res)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Copy the directory store path before exchange the data */
    char* pCopyPath = SOPC_strdup(directoryStorePath);
    if (NULL == pCopyPath)
    {
        return SOPC_STATUS_NOK;
    }
    SOPC_Free(pPKI->directoryStorePath);
    pPKI->directoryStorePath = pCopyPath;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PKIProvider_WriteOrAppendToList(const SOPC_PKIProvider* pPKI,
                                                       SOPC_CertificateList** ppTrustedCerts,
                                                       SOPC_CRLList** ppTrustedCrl,
                                                       SOPC_CertificateList** ppIssuerCerts,
                                                       SOPC_CRLList** ppIssuerCrl)
{
    if (NULL == pPKI || NULL == ppTrustedCerts || NULL == ppTrustedCrl || NULL == ppIssuerCerts || NULL == ppIssuerCrl)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (pPKI->isPermissive)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_CertificateList* pTrustedCerts = *ppTrustedCerts;
    SOPC_CRLList* pTrustedCrl = *ppTrustedCrl;
    SOPC_CertificateList* pIssuerCerts = *ppIssuerCerts;
    SOPC_CRLList* pIssuerCrl = *ppIssuerCrl;
    SOPC_ReturnStatus status = merge_certificates(pPKI->pTrustedRoots, pPKI->pTrustedCerts, &pTrustedCerts);
    if (SOPC_STATUS_OK == status && NULL != pPKI->pTrustedCrl)
    {
        status = SOPC_KeyManager_CRL_Copy(pPKI->pTrustedCrl, &pTrustedCrl);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = merge_certificates(pPKI->pIssuerRoots, pPKI->pIssuerCerts, &pIssuerCerts);
    }
    if (SOPC_STATUS_OK == status && NULL != pPKI->pIssuerCrl)
    {
        status = SOPC_KeyManager_CRL_Copy(pPKI->pIssuerCrl, &pIssuerCrl);
    }
    /* Clear if error */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pTrustedCerts);
        SOPC_KeyManager_Certificate_Free(pIssuerCerts);
        SOPC_KeyManager_CRL_Free(pTrustedCrl);
        SOPC_KeyManager_CRL_Free(pIssuerCrl);
        pTrustedCerts = NULL;
        pIssuerCerts = NULL;
        pTrustedCrl = NULL;
        pIssuerCrl = NULL;
    }
    *ppTrustedCerts = pTrustedCerts;
    *ppIssuerCerts = pIssuerCerts;
    *ppTrustedCrl = pTrustedCrl;
    *ppIssuerCrl = pIssuerCrl;
    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_WriteToStore(const SOPC_PKIProvider* pPKI, const bool bEraseExistingFiles)
{
    if (NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    if (pPKI->isPermissive)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* The case of the PKI is built from buffer (there is no store) */
    if (NULL == pPKI->directoryStorePath)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    char* basePath = NULL;
    char* path = NULL;
    SOPC_ReturnStatus status = may_create_pki_folder(pPKI->directoryStorePath, STR_TRUSTLIST_NAME, &basePath);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    status = may_create_pki_folder(basePath, STR_TRUSTED, &path);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_TRUSTED_CERTS, &path);
        if (SOPC_STATUS_OK == status)
        {
            status = write_cert_to_der_files(pPKI->pTrustedRoots, pPKI->pTrustedCerts, path, bEraseExistingFiles);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_TRUSTED_CRL, &path);
        if (SOPC_STATUS_OK == status)
        {
            status = write_crl_to_der_files(pPKI->pTrustedCrl, path, bEraseExistingFiles);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_ISSUERS, &path);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_ISSUERS_CERTS, &path);
        if (SOPC_STATUS_OK == status)
        {
            status = write_cert_to_der_files(pPKI->pIssuerRoots, pPKI->pIssuerCerts, path, bEraseExistingFiles);
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_ISSUERS_CRL, &path);
        if (SOPC_STATUS_OK == status)
        {
            status = write_crl_to_der_files(pPKI->pIssuerCrl, path, bEraseExistingFiles);
        }
    }

    SOPC_Free(basePath);
    SOPC_Free(path);

    return status;
}

static SOPC_ReturnStatus check_security_level_of_the_update(const SOPC_CertificateList* pTrustedCerts,
                                                            const SOPC_CRLList* pTrustedCrl,
                                                            const SOPC_CertificateList* pIssuerCerts,
                                                            const SOPC_CRLList* pIssuerCrl,
                                                            const char* securityPolicyUri)
{
    /*
    TODO :

    -1 Add a way to configure the security level for each security policy uri (give them a weight)
    -2 For each certificate, retrieve their security policies from their properties.
       How to do it? The following issue has been SUBMITTED : https://mantis.opcfoundation.org/view.php?id=8976
    */

    SOPC_UNUSED_ARG(pTrustedCerts);
    SOPC_UNUSED_ARG(pTrustedCrl);
    SOPC_UNUSED_ARG(pIssuerCerts);
    SOPC_UNUSED_ARG(pIssuerCrl);
    SOPC_UNUSED_ARG(securityPolicyUri);

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_PKIProvider_UpdateFromList(SOPC_PKIProvider** ppPKI,
                                                  const char* securityPolicyUri,
                                                  SOPC_CertificateList* pTrustedCerts,
                                                  SOPC_CRLList* pTrustedCrl,
                                                  SOPC_CertificateList* pIssuerCerts,
                                                  SOPC_CRLList* pIssuerCrl,
                                                  const bool bIncludeExistingList)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_PKIProvider* pPKI = *ppPKI;
    /* Check parameters */
    if (NULL == pPKI)
    {
        return status;
    }
    if (pPKI->isPermissive)
    {
        return status;
    }
    /* Handle that the security level of the update isn't higher than the
       security level of the secure channel. (§7.3.4 part 2 v1.05) */
    status =
        check_security_level_of_the_update(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, securityPolicyUri);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    SOPC_PKIProvider* pTmpPKI = NULL;
    SOPC_CertificateList* tmp_pTrustedCerts = NULL; /* trusted intermediate CA + trusted certificates */
    SOPC_CertificateList* tmp_pTrustedCertsTmp = NULL;
    SOPC_CRLList* tmp_pTrustedCrl = NULL;          /* CRLs of trusted intermediate CA and trusted root CA */
    SOPC_CertificateList* tmp_pIssuerCerts = NULL; /* issuer intermediate CA + issuer root CA */
    SOPC_CertificateList* tmp_pIssuerCertsTmp = NULL;
    SOPC_CRLList* tmp_pIssuerCrl = NULL; /* CRLs of issuer intermediate CA and issuer root CA */

    /* Includes the existing TrustList plus any updates */
    if (bIncludeExistingList)
    {
        /* tmp_pTrustedCerts = pTrustedCerts + pPKI->pTrustedCerts + pPKI->pTrustedRoot */
        status = merge_certificates(pPKI->pTrustedCerts, pTrustedCerts, &tmp_pTrustedCertsTmp);
        if (SOPC_STATUS_OK == status)
        {
            status = merge_certificates(pPKI->pTrustedRoots, tmp_pTrustedCertsTmp, &tmp_pTrustedCerts);
        }
        /* tmp_pTrustedCrl = pTrustedCrl + pPKI->pTrustedCrl */
        if (SOPC_STATUS_OK == status)
        {
            status = merge_crls(pPKI->pTrustedCrl, pTrustedCrl, &tmp_pTrustedCrl);
        }
        /* tmp_pIssuerCerts = pIssuerCerts + pPKI->pIssuerCerts + pPKI->pIssuerRoot */
        if (SOPC_STATUS_OK == status)
        {
            status = merge_certificates(pPKI->pIssuerCerts, pIssuerCerts, &tmp_pIssuerCertsTmp);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = merge_certificates(pPKI->pIssuerRoots, tmp_pIssuerCertsTmp, &tmp_pIssuerCerts);
        }
        /* tmp_pIssuerCrl = pIssuerCrl + pPKI->pIssuerCrl */
        if (SOPC_STATUS_OK == status)
        {
            status = merge_crls(pPKI->pIssuerCrl, pIssuerCrl, &tmp_pIssuerCrl);
        }
        /* Create a new tmp PKI */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProvider_CreateFromList(tmp_pTrustedCerts, tmp_pTrustedCrl, tmp_pIssuerCerts,
                                                     tmp_pIssuerCrl, &pTmpPKI);
        }
    }
    else
    {
        /* Create a new tmp PKI without the existing TrustList */
        status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, &pTmpPKI);
    }
    /* Copy the directory store path before exchange the data */
    if (SOPC_STATUS_OK == status && NULL != pPKI->directoryStorePath)
    {
        pTmpPKI->directoryStorePath = SOPC_strdup(pPKI->directoryStorePath);
        if (NULL == pTmpPKI->directoryStorePath)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    /* Exchange the internal data */
    if (SOPC_STATUS_OK == status)
    {
        sopc_pki_clear(pPKI);
        *pPKI = *pTmpPKI;
    }

    SOPC_Free(pTmpPKI);
    SOPC_KeyManager_Certificate_Free(tmp_pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(tmp_pTrustedCertsTmp);
    SOPC_KeyManager_Certificate_Free(tmp_pIssuerCerts);
    SOPC_KeyManager_Certificate_Free(tmp_pIssuerCertsTmp);
    SOPC_KeyManager_CRL_Free(tmp_pTrustedCrl);
    SOPC_KeyManager_CRL_Free(tmp_pIssuerCrl);

    return status;
}
