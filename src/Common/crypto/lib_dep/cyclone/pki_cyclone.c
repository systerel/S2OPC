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
#include "sopc_common_constants.h"
#include "sopc_crypto_decl.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_filesystem.h"
#include "sopc_helper_string.h"
#include "sopc_helper_uri.h"
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pki_stack.h"
#include "sopc_pki_struct_lib_internal.h"
#include "sopc_time.h"

#include "key_manager_cyclone.h"

#include "encoding/oid.h"
#include "pkix/x509_cert_validate.h"
#include "pkix/x509_crl_validate.h"
#include "pkix/x509_sign_verify.h"

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

#define HEX_THUMBPRINT_SIZE 40

typedef struct
{
    const SOPC_CertificateList* trustedCrts;
    const SOPC_CRLList* allCRLs;
    bool isTrustedInChain;
    bool disableRevocationCheck;
} SOPC_CheckTrustedAndCRLinChain;

// Bitmask errors for OPCUA error order compliance
#define X509_BADCERT_EXPIRED 0x01 /**< The certificate validity has expired. */
#define X509_BADCERT_REVOKED 0x02 /**< The certificate has been revoked (is on a CRL). */
#define X509_BADCERT_CN_MISMATCH 0x04 /**< The certificate Common Name (CN) does not match with the expected CN. */
#define X509_BADCERT_NOT_TRUSTED 0x08 /**< The certificate is not correctly signed by the trusted CA. */
#define X509_BADCRL_NOT_TRUSTED 0x10 /**< The CRL is not correctly signed by the trusted CA. */
#define X509_BADCRL_EXPIRED 0x20 /**< The CRL is expired. */
#define X509_BADCERT_MISSING 0x40 /**< Certificate was missing. */
#define X509_BADCERT_SKIP_VERIFY 0x80 /**< Certificate verification was skipped. */
#define X509_BADCERT_OTHER 0x0100 /**< Other reason (can be used by verify callback) */
#define X509_BADCERT_FUTURE 0x0200 /**< The certificate validity starts in the future. */
#define X509_BADCRL_FUTURE 0x0400 /**< The CRL is from the future */
#define X509_BADCERT_KEY_USAGE 0x0800 /**< Usage does not match the keyUsage extension. */
#define X509_BADCERT_EXT_KEY_USAGE 0x1000 /**< Usage does not match the extendedKeyUsage extension. */
#define X509_BADCERT_NS_CERT_TYPE 0x2000 /**< Usage does not match the nsCertType extension. */
#define X509_BADCERT_BAD_MD 0x4000 /**< The certificate is signed with an unacceptable hash. */
#define X509_BADCERT_BAD_PK 0x8000 /**< The certificate is signed with an unacceptable PK alg (eg RSA vs ECDSA). */
#define X509_BADCERT_BAD_KEY \
    0x010000 /**< The certificate is signed with an unacceptable key (eg bad curve, RSA too short). */
#define X509_BADCRL_BAD_MD 0x020000 /**< The CRL is signed with an unacceptable hash. */
#define X509_BADCRL_BAD_PK 0x040000 /**< The CRL is signed with an unacceptable PK alg (eg RSA vs ECDSA). */
#define X509_BADCRL_BAD_KEY 0x080000 /**< The CRL is signed with an unacceptable key (eg bad curve, RSA too short). */

// Bitmask for MD algos
#define X509_ID_FLAG_MD_SHA1 0x01
#define X509_ID_FLAG_MD_SHA224 0x02
#define X509_ID_FLAG_MD_SHA256 0x04
#define X509_ID_FLAG_MD_SHA384 0x08
#define X509_ID_FLAG_MD_SHA512 0x10

// Bitmask for public key algos
#define X509_ID_FLAG_PK_RSA 0x01

/* Definition of the CycloneCRYPTO profile cert structure */
typedef struct cyclone_crypto_crt_profile
{
    uint32_t allowed_mds;    /**< MDs for signatures         */
    uint32_t allowed_pks;    /**< PK algs for public keys;
                              *   this applies to all certificates
                              *   in the provided chain.     */
    uint32_t allowed_curves; /**< Elliptic curves for ECDSA  */
    uint32_t rsa_min_bitlen; /**< Minimum size for RSA keys  */
} cyclone_crypto_crt_profile;

static uint32_t PKIProviderStack_GetCertificateValidationError(uint32_t failure_reasons)
{
    if ((failure_reasons & X509_BADCERT_MISSING) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & X509_BADCERT_KEY_USAGE) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & X509_BADCERT_EXT_KEY_USAGE) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & X509_BADCERT_NS_CERT_TYPE) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & X509_BADCERT_SKIP_VERIFY) != 0)
    {
        return SOPC_CertificateValidationError_UseNotAllowed;
    }

    else if ((failure_reasons & X509_BADCERT_BAD_KEY) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & X509_BADCERT_BAD_MD) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & X509_BADCERT_BAD_PK) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }

    if ((failure_reasons & X509_BADCERT_NOT_TRUSTED) != 0)
    {
        return SOPC_CertificateValidationError_Untrusted;
    }

    else if ((failure_reasons & X509_BADCERT_EXPIRED) != 0)
    {
        return SOPC_CertificateValidationError_TimeInvalid;
    }
    else if ((failure_reasons & X509_BADCERT_FUTURE) != 0)
    {
        return SOPC_CertificateValidationError_TimeInvalid;
    }

    else if ((failure_reasons & X509_BADCERT_CN_MISMATCH) != 0)
    {
        return SOPC_CertificateValidationError_HostNameInvalid;
    }

    else if ((failure_reasons & X509_BADCRL_NOT_TRUSTED) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & X509_BADCRL_EXPIRED) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & X509_BADCRL_FUTURE) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & X509_BADCRL_BAD_MD) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & X509_BADCRL_BAD_PK) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & X509_BADCRL_BAD_KEY) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }

    else if ((failure_reasons & X509_BADCERT_REVOKED) != 0)
    {
        return SOPC_CertificateValidationError_Revoked;
    }
    else if ((failure_reasons & X509_BADCERT_OTHER) != 0)
    {
        return SOPC_CertificateValidationError_Untrusted;
    }

    return SOPC_CertificateValidationError_Unknown;
}

static int verify_cert(SOPC_CheckTrustedAndCRLinChain* checkTrustedAndCRL,
                       SOPC_CertificateList* crt,
                       int certificate_depth,
                       uint32_t* flags)
{
    SOPC_CheckTrustedAndCRLinChain* checkTrustedAndCRLinChain = checkTrustedAndCRL;
    int err = 0;

    /* Check if a revocation list is present when the certificate is not a leaf
     * (that might have CA bit set if self-signed and OPC UA backward compatibility active)
     * It is your responsibility to provide up-to-date CRLs for all trusted CAs. If no CRL is provided for the CA
     * that was used to sign the certificate, CRL verification is skipped silently, that is without setting any flag.
     */
    if (0 != certificate_depth && !checkTrustedAndCRLinChain->disableRevocationCheck)
    {
        bool matchCRL = false;
        // Unlink cert temporarily for CRL verification
        SOPC_CertificateList* backupNextCrt = crt->next;
        crt->next = NULL;
        SOPC_ReturnStatus status = SOPC_STATUS_NOK;
        if (NULL != checkTrustedAndCRLinChain->allCRLs)
        {
            status =
                SOPC_KeyManagerInternal_CertificateList_CheckCRL(crt, checkTrustedAndCRLinChain->allCRLs, &matchCRL);
        }
        // Restore link
        crt->next = backupNextCrt;
        if (SOPC_STATUS_OK != status)
        {
            matchCRL = false;
        }
        if (!matchCRL)
        {
            *flags |= X509_BADCRL_NOT_TRUSTED;
            err = -1;
        }
    }

    /* Check if the certificate is part of PKI trusted certificates */
    const SOPC_CertificateList* crtTrusted = NULL;
    if (NULL != checkTrustedAndCRLinChain->trustedCrts)
    {
        crtTrusted = checkTrustedAndCRLinChain->trustedCrts; /* Current cert */
    }
    while (!checkTrustedAndCRLinChain->isTrustedInChain && NULL != crtTrusted)
    {
        if (crt->crt.tbsCert.subject.rawDataLen == crtTrusted->crt.tbsCert.subject.rawDataLen &&
            crt->raw->length == crtTrusted->raw->length &&
            0 == memcmp(crt->crt.tbsCert.subject.rawData, crtTrusted->crt.tbsCert.subject.rawData,
                        crt->crt.tbsCert.subject.rawDataLen) &&
            0 == memcmp(crt->raw->data, crtTrusted->raw->data, crt->crt.tbsCert.subject.rawDataLen))
        {
            checkTrustedAndCRLinChain->isTrustedInChain = true;
        }
        crtTrusted = crtTrusted->next;
    }

    return err;
}

// removes cert from a linked list
static void sopc_remove_cert_from_list(SOPC_CertificateList* pPrev,
                                       SOPC_CertificateList** ppCur,
                                       SOPC_CertificateList** ppHeadCertList);

// PKI remove certificate from rejected list declaration
static void sopc_pki_remove_rejected_cert(SOPC_CertificateList** ppRejectedList, const SOPC_CertificateList* pCert);

// PKI clear operation declaration
static void sopc_pki_clear(SOPC_PKIProvider* pPKI);

// Copy newPKI content into currentPKI by preserving currentPKI mutex and then clear previous PKI content.
// Then frees the new PKI structure.
static void SOPC_Internal_ReplacePKIAndClear(SOPC_PKIProvider* currentPKI, SOPC_PKIProvider** newPKI)
{
    // tmpPKI used for clear
    SOPC_PKIProvider tmpPKI = *currentPKI;
    tmpPKI.mutex = (*newPKI)->mutex;
    // Replace all except mutex which shall remain the same since PKI is already in use
    currentPKI = memcpy(((char*) currentPKI) + sizeof(SOPC_Mutex), ((char*) (*newPKI)) + sizeof(SOPC_Mutex),
                        sizeof(SOPC_PKIProvider) - sizeof(SOPC_Mutex));
    // clear previous PKI data and unused new PKI mutex
    sopc_pki_clear(&tmpPKI);
    // frees unused new PKI structure
    SOPC_Free(*newPKI);
    *newPKI = NULL;
}

static const SOPC_PKI_KeyUsage_Mask g_appKU = SOPC_PKI_KU_KEY_ENCIPHERMENT | SOPC_PKI_KU_KEY_DATA_ENCIPHERMENT |
                                              SOPC_PKI_KU_DIGITAL_SIGNATURE | SOPC_PKI_KU_NON_REPUDIATION;
static const SOPC_PKI_KeyUsage_Mask g_usrKU =
    SOPC_PKI_KU_DIGITAL_SIGNATURE; // it is not part of the OPC UA but it makes sense to keep it
static const SOPC_PKI_ExtendedKeyUsage_Mask g_clientEKU = SOPC_PKI_EKU_SERVER_AUTH;
static const SOPC_PKI_ExtendedKeyUsage_Mask g_serverEKU = SOPC_PKI_EKU_CLIENT_AUTH;
static const SOPC_PKI_ExtendedKeyUsage_Mask g_userEKU = SOPC_PKI_EKU_NONE;

static const SOPC_PKI_LeafProfile g_leaf_profile_rsa_sha256_2048_4096 = {.mdSign = SOPC_PKI_MD_SHA256,
                                                                         .pkAlgo = SOPC_PKI_PK_RSA,
                                                                         .RSAMinimumKeySize = 2048,
                                                                         .RSAMaximumKeySize = 4096,
                                                                         .bApplySecurityPolicy = true,
                                                                         .keyUsage = SOPC_PKI_KU_NONE,
                                                                         .extendedKeyUsage = SOPC_PKI_EKU_NONE,
                                                                         .sanApplicationUri = NULL,
                                                                         .sanURL = NULL};

static const SOPC_PKI_LeafProfile g_leaf_profile_rsa_sha1_1024_2048 = {.mdSign = SOPC_PKI_MD_SHA1_AND_SHA256,
                                                                       .pkAlgo = SOPC_PKI_PK_RSA,
                                                                       .RSAMinimumKeySize = 1024,
                                                                       .RSAMaximumKeySize = 2048,
                                                                       .bApplySecurityPolicy = true,
                                                                       .keyUsage = SOPC_PKI_KU_NONE,
                                                                       .extendedKeyUsage = SOPC_PKI_EKU_NONE,
                                                                       .sanApplicationUri = NULL,
                                                                       .sanURL = NULL};

static const SOPC_PKI_ChainProfile g_chain_profile_rsa_sha256_2048 = {.curves = SOPC_PKI_CURVES_ANY,
                                                                      .mdSign = SOPC_PKI_MD_SHA256_OR_ABOVE,
                                                                      .pkAlgo = SOPC_PKI_PK_RSA,
                                                                      .RSAMinimumKeySize = 2048,
                                                                      .bDisableRevocationCheck = false};

static const SOPC_PKI_ChainProfile g_chain_profile_rsa_sha1_1024 = {.curves = SOPC_PKI_CURVES_ANY,
                                                                    .mdSign = SOPC_PKI_MD_SHA1_OR_ABOVE,
                                                                    .pkAlgo = SOPC_PKI_PK_RSA,
                                                                    .RSAMinimumKeySize = 1024,
                                                                    .bDisableRevocationCheck = false};

typedef struct Profile_Cfg
{
    const SOPC_PKI_ChainProfile* chain;
    const SOPC_PKI_LeafProfile* leaf;
    const SOPC_SecurityPolicy_ID id;
} Profile_Cfg;

static const Profile_Cfg g_all_profiles[] = {
    {.id = SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID,
     .leaf = &g_leaf_profile_rsa_sha256_2048_4096,
     .chain = &g_chain_profile_rsa_sha256_2048},
    {.id = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID,
     .leaf = &g_leaf_profile_rsa_sha256_2048_4096,
     .chain = &g_chain_profile_rsa_sha256_2048},
    {.id = SOPC_SecurityPolicy_Basic256Sha256_ID,
     .leaf = &g_leaf_profile_rsa_sha256_2048_4096,
     .chain = &g_chain_profile_rsa_sha256_2048},
    {.id = SOPC_SecurityPolicy_Basic256_ID,
     .leaf = &g_leaf_profile_rsa_sha1_1024_2048,
     .chain = &g_chain_profile_rsa_sha1_1024},
};

#define NB_PROFILES (sizeof(g_all_profiles) / sizeof(*g_all_profiles))

static const SOPC_PKI_LeafProfile* get_leaf_profile_from_security_policy(const char* uri)
{
    if (NULL == uri)
    {
        return NULL;
    }

    const size_t len = strlen(uri) + 1;
    for (size_t i = 0; i < NB_PROFILES; ++i)
    {
        const struct Profile_Cfg* pProfile = &g_all_profiles[i];
        const SOPC_SecurityPolicy_Config* pPolicy = SOPC_SecurityPolicy_Config_Get(pProfile->id);
        const char* pUri = pPolicy->uri;
        if (pUri != NULL && SOPC_strncmp_ignore_case(uri, pUri, len) == 0)
        {
            return pProfile->leaf;
        }
    }
    return NULL;
}

static const SOPC_PKI_ChainProfile* get_chain_profile_from_security_policy(const char* uri)
{
    if (NULL == uri)
    {
        return NULL;
    }

    const size_t len = strlen(uri) + 1;
    for (size_t i = 0; i < NB_PROFILES; ++i)
    {
        const struct Profile_Cfg* pProfile = &g_all_profiles[i];
        const SOPC_SecurityPolicy_Config* pPolicy = SOPC_SecurityPolicy_Config_Get(pProfile->id);
        const char* pUri = pPolicy->uri;
        if (pUri != NULL && SOPC_strncmp_ignore_case(uri, pUri, len) == 0)
        {
            return pProfile->chain;
        }
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

static SOPC_ReturnStatus check_security_policy(const SOPC_CertificateList* pToValidate,
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
    /* Useless to do it it with CycloneCRYPTO since the only key type implemented is RSA */
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
    const uint8_t* oid = signAlgoId->oid;
    size_t oidLen = signAlgoId->oidLen;
    bErr = false;
    switch (pConfig->mdSign)
    {
    case SOPC_PKI_MD_SHA1_OR_ABOVE:
        // Function oidComp() returns 0 if the 2 object identifiers are equal
        if (oidComp(oid, oidLen, SHA1_WITH_RSA_ENCRYPTION_OID, sizeof(SHA1_WITH_RSA_ENCRYPTION_OID)) &&
            oidComp(oid, oidLen, SHA224_WITH_RSA_ENCRYPTION_OID, sizeof(SHA224_WITH_RSA_ENCRYPTION_OID)) &&
            oidComp(oid, oidLen, SHA256_WITH_RSA_ENCRYPTION_OID, sizeof(SHA256_WITH_RSA_ENCRYPTION_OID)) &&
            oidComp(oid, oidLen, SHA384_WITH_RSA_ENCRYPTION_OID, sizeof(SHA384_WITH_RSA_ENCRYPTION_OID)) &&
            oidComp(oid, oidLen, SHA512_WITH_RSA_ENCRYPTION_OID, sizeof(SHA512_WITH_RSA_ENCRYPTION_OID)))
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA256_OR_ABOVE:
        if (oidComp(oid, oidLen, SHA256_WITH_RSA_ENCRYPTION_OID, sizeof(SHA256_WITH_RSA_ENCRYPTION_OID)) &&
            oidComp(oid, oidLen, SHA384_WITH_RSA_ENCRYPTION_OID, sizeof(SHA384_WITH_RSA_ENCRYPTION_OID)) &&
            oidComp(oid, oidLen, SHA512_WITH_RSA_ENCRYPTION_OID, sizeof(SHA512_WITH_RSA_ENCRYPTION_OID)))
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA1:
        if (oidComp(oid, oidLen, SHA1_WITH_RSA_ENCRYPTION_OID, sizeof(SHA1_WITH_RSA_ENCRYPTION_OID)))
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA256:
        if (oidComp(oid, oidLen, SHA256_WITH_RSA_ENCRYPTION_OID, sizeof(SHA256_WITH_RSA_ENCRYPTION_OID)))
        {
            bErr = true;
        }
        break;
    case SOPC_PKI_MD_SHA1_AND_SHA256:
        if (oidComp(oid, oidLen, SHA1_WITH_RSA_ENCRYPTION_OID, sizeof(SHA1_WITH_RSA_ENCRYPTION_OID)) &&
            oidComp(oid, oidLen, SHA256_WITH_RSA_ENCRYPTION_OID, sizeof(SHA256_WITH_RSA_ENCRYPTION_OID)))
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

static SOPC_ReturnStatus check_host_name(const SOPC_CertificateList* pToValidate, const char* url)
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

static SOPC_ReturnStatus check_application_uri(const SOPC_CertificateList* pToValidate, const char* applicationUri)
{
    SOPC_ASSERT(NULL != pToValidate);
    SOPC_ASSERT(NULL != applicationUri);

    bool ok = SOPC_KeyManager_Certificate_CheckApplicationUri(pToValidate, applicationUri);
    if (!ok)
    {
        char* pThumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(pToValidate);
        const char* thumbprint = NULL == pThumbprint ? "NULL" : pThumbprint;
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI validation failed : the application URI %s is not stored in the URI SAN "
                               "extension of certificate thumbprint %s",
                               applicationUri, thumbprint);
        SOPC_Free(pThumbprint);
        return SOPC_STATUS_NOK;
    }
    return SOPC_STATUS_OK;
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
    unsigned int cert_key_usages = pToValidate->crt.tbsCert.extensions.keyUsage.bitmap;
    unsigned int real_key_usages = cert_key_usages & expected_key_usages;

    if (real_key_usages != expected_key_usages)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
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

static SOPC_ReturnStatus set_profile_from_configuration(const SOPC_PKI_ChainProfile* pProfile,
                                                        cyclone_crypto_crt_profile* pLibProfile)
{
    /* Set hashes allowed */
    if (SOPC_PKI_MD_SHA1_OR_ABOVE == pProfile->mdSign)
    {
        pLibProfile->allowed_mds = X509_ID_FLAG_MD_SHA1 | X509_ID_FLAG_MD_SHA224 | X509_ID_FLAG_MD_SHA256 |
                                   X509_ID_FLAG_MD_SHA384 | X509_ID_FLAG_MD_SHA512;
    }
    else if (SOPC_PKI_MD_SHA256_OR_ABOVE == pProfile->mdSign)
    {
        pLibProfile->allowed_mds = X509_ID_FLAG_MD_SHA256 | X509_ID_FLAG_MD_SHA384 | X509_ID_FLAG_MD_SHA512;
    }
    else if (SOPC_PKI_MD_SHA1 == pProfile->mdSign)
    {
        pLibProfile->allowed_mds = X509_ID_FLAG_MD_SHA1;
    }
    else if (SOPC_PKI_MD_SHA256 == pProfile->mdSign)
    {
        pLibProfile->allowed_mds = X509_ID_FLAG_MD_SHA256;
    }
    else if (SOPC_PKI_MD_SHA1_AND_SHA256 == pProfile->mdSign)
    {
        pLibProfile->allowed_mds = X509_ID_FLAG_MD_SHA1 | X509_ID_FLAG_MD_SHA256;
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
        pLibProfile->allowed_pks = X509_ID_FLAG_PK_RSA;
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

/* Define the _weak_func used by x509ValidateCertificate() here.
 * TODO : Put this function in another file ?
 */
time_t getCurrentUnixTime(void)
{
    time_t res = 0;

    // Retrieve current time
    SOPC_DateTime dateTime = SOPC_Time_GetCurrentTimeUTC();

    // Convert it to the type time_t
    SOPC_ReturnStatus status = SOPC_Time_ToTimeT(dateTime, &res);

    if (SOPC_STATUS_OK == status)
    {
        return res;
    }

    return 0;
}

/* Check in all the CRLs present in crl if child is revoked.
 * Returns -1 if child is revoked, 0 otherwise.
 */
static int crt_check_revocation(const SOPC_CertificateList* child, SOPC_CRLList* crl)
{
    SOPC_CRLList* cur_crl = NULL;
    error_t errLib = 1;

    for (cur_crl = crl; NULL != cur_crl; cur_crl = cur_crl->next)
    {
        errLib = x509CheckRevokedCertificate(&child->crt, &cur_crl->crl);
        if (ERROR_CERTIFICATE_REVOKED == errLib)
        {
            return -1;
        }
    }

    return 0;
}

/* Find a parent in the chain candidates.
 * If a parent has been found, ppParent is filled with it.
 * Otherwise, ppParent is not modified.
 */
static void crt_find_parent_in(const SOPC_CertificateList* child,
                               SOPC_CertificateList* candidates,
                               uint32_t* failure_reasons,
                               SOPC_CertificateList** ppParent)
{
    SOPC_CertificateList* parent = NULL;
    error_t errLib = 1;

    for (parent = candidates; NULL != parent; parent = parent->next)
    {
        /* This Cyclone function :
         * - checks the validity of child ;
         * - checks if parent is the parent of child ;
         * - verifies the signature ;
         * Returns 0 if all is ok.
         */
        errLib = x509ValidateCertificate(&child->crt, &parent->crt, 0);
        if (ERROR_CERTIFICATE_EXPIRED == errLib)
        {
            *failure_reasons |= X509_BADCERT_EXPIRED;
            break;
        }

        if (errLib)
        {
            continue; // test next parent candidate
        }

        *ppParent = parent;
        break;
    }
}

/* Find a parent in the chain candidates OR in the chain of child.
 */
static void crt_find_parent(const SOPC_CertificateList* child,
                            SOPC_CertificateList* candidates,
                            uint32_t* failure_reasons,
                            SOPC_CertificateList** ppParent,
                            int* parent_is_trusted)
{
    // Make sure the container of the potential parent is not null, and empty.
    SOPC_ASSERT(NULL != ppParent);
    SOPC_ASSERT(NULL == *ppParent);

    SOPC_CertificateList* search_list = NULL;
    *parent_is_trusted = 1;

    while (1)
    {
        search_list = *parent_is_trusted ? candidates : child->next;
        crt_find_parent_in(child, search_list, failure_reasons, ppParent);

        /* stop here if found or already in second iteration */
        if (NULL != *ppParent || 0 == *parent_is_trusted)
        {
            break;
        }

        /* prepare second iteration */
        *parent_is_trusted = 0;

        // We assume the loop stops after the second iteration.
    }
}

/* Converts a HashAlgo object to a its bitmask.
 * Returns 0 if it corresponds to a not known hash algo.
 */
static uint32_t HashAlgoToBitmask(const HashAlgo* hashAlgo)
{
    const char* hashAlgoName = hashAlgo->name;
    int comparison = strcmp(hashAlgoName, "SHA-1");
    if (0 == comparison)
    {
        return 0x01;
    }
    comparison = strcmp(hashAlgoName, "SHA-224");
    if (0 == comparison)
    {
        return 0x02;
    }
    comparison = strcmp(hashAlgoName, "SHA-256");
    if (0 == comparison)
    {
        return 0x04;
    }
    comparison = strcmp(hashAlgoName, "SHA-384");
    if (0 == comparison)
    {
        return 0x08;
    }
    comparison = strcmp(hashAlgoName, "SHA-512");
    if (0 == comparison)
    {
        return 0x10;
    }

    return 0;
}

/* Verifies the chain of the certificate.
 * Returns -1 if the verification failed, and in this case
 * the failure reason is stored in the variable failure_reasons.
 * Returns 0 on success.
 */
static int crt_verify_chain(SOPC_CertificateList* pToValidate,
                            SOPC_CertificateList* trust_list,
                            SOPC_CRLList* cert_crl,
                            const cyclone_crypto_crt_profile* crt_profile,
                            uint32_t* failure_reasons,
                            SOPC_CheckTrustedAndCRLinChain* checkTrustedAndCRL)
{
    SOPC_CertificateList* child = pToValidate;
    SOPC_CertificateList* parent = NULL;
    SOPC_CertificateList* cur_trust_ca = NULL;
    int err = 0;
    int parent_is_trusted = 0;
    int child_is_trusted = 0;
    int certificate_depth = 0;

    while (1 && 0 == err)
    {
        err = verify_cert(checkTrustedAndCRL, child, certificate_depth, failure_reasons);
        if (err)
        {
            return err;
        }

        if (child_is_trusted)
        {
            return 0;
        }

        // a) Find a parent in trusted CA first or in the child chain.
        // This function will also verify the signature if a parent is found,
        // and check time-validity of child.
        cur_trust_ca = trust_list;
        crt_find_parent(child, cur_trust_ca, failure_reasons, &parent, &parent_is_trusted);
        if (NULL == parent)
        {
            *failure_reasons |= X509_BADCERT_NOT_TRUSTED;
            return -1;
        }

        // b) Check if the parent's key suits to the profile.
        size_t keySize = parent->crt.tbsCert.subjectPublicKeyInfo.rsaPublicKey.nLen * 8;
        if (keySize < crt_profile->rsa_min_bitlen)
        {
            *failure_reasons |= X509_BADCERT_BAD_KEY;
            return -1;
        }

        // c) Check if the cert is not revoked, in any valid CRL.
        // TODO : proceed on some checks on the CRL (if it the signature suits the profile and if it is signed by a
        // trusted CA).
        err = crt_check_revocation(child, cert_crl);
        if (err)
        {
            *failure_reasons |= X509_BADCERT_REVOKED;
        }

        // d) Iterate.
        child = parent;
        parent = NULL;
        child_is_trusted = parent_is_trusted;
        certificate_depth += 1;
    }

    return -1;
}

/* ------------------------------------------------------------------------------------------------
 * Self-signed certificates related functions
 * ------------------------------------------------------------------------------------------------
 */

/* Check the validity of the certificate. Inspired of the Cyclone function x509ValidateCertificate().
 * Returns 0 if the certififcate is valid, -1 otherwise.
 */
static int crt_check_validity(const X509CertificateInfo* crt)
{
    // Retrieve current time
    time_t currentTime = getCurrentUnixTime();

    // Any real-time clock implemented?
    if (currentTime != 0)
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
        if (compareDateTime(&currentDate, &validity->notBefore) < 0 ||
            compareDateTime(&currentDate, &validity->notAfter) > 0)
        {
            // The certificate has expired or is not yet valid
            return -1;
        }
    }

    return 0;
}

/* Self-signed validation function.
 * Basically do the same as x509ValidateCertificate() except
 * verifying that the issuer is CA.
 * Returns 0 if the certificate is validated, -1 otherwise.
 */
static int ValidateSelfSignedCertificate(const X509CertificateInfo* crt)
{
    // Initializes at states error
    int err = -1;
    error_t errLib = 1;

    const X509Extensions* extensions;

    // Point to the X.509 extensions of the issuer certificate
    extensions = &crt->tbsCert.extensions;

    // Check if the keyUsage extension is present
    if (extensions->keyUsage.bitmap != 0)
    {
        // If the keyUsage extension is present, then the subject public key must
        // not be used to verify signatures on certificates unless the keyCertSign
        // bit is set (refer to RFC 5280, section 4.2.1.3)
        if ((extensions->keyUsage.bitmap & X509_KEY_USAGE_KEY_CERT_SIGN) == 0)
        {
            return ERROR_BAD_CERTIFICATE;
        }
    }

    // The ASN.1 DER-encoded tbsCertificate is used as the input to the signature
    // function
    errLib = x509VerifySignature(crt->tbsCert.rawData, crt->tbsCert.rawDataLen, &crt->signatureAlgo,
                                 &crt->tbsCert.subjectPublicKeyInfo, &crt->signatureValue);

    // If no error
    if (!errLib)
    {
        err = 0;
    }

    return err;
}

static int crt_verify_self_signed(SOPC_CertificateList* pToValidate,
                                  SOPC_CRLList* cert_crl,
                                  uint32_t* failure_reasons,
                                  SOPC_CheckTrustedAndCRLinChain* checkTrustedAndCRL)
{
    int err = 0;

    int ret = verify_cert(checkTrustedAndCRL, pToValidate, 0, failure_reasons);
    if (ret)
    {
        err = -1;
    }

    // Check validity
    ret = crt_check_validity(&pToValidate->crt);
    if (ret)
    {
        err = -1;
        *failure_reasons |= X509_BADCERT_EXPIRED;
    }

    // Validate certificate
    ret = ValidateSelfSignedCertificate(&pToValidate->crt);
    if (ret)
    {
        err = -1;
        *failure_reasons |= X509_BADCERT_NOT_TRUSTED;
    }

    // Check revocation
    ret = crt_check_revocation(pToValidate, cert_crl);
    if (ret)
    {
        err = -1;
        *failure_reasons |= X509_BADCERT_REVOKED;
    }

    return err;
}

/* Some verifications on the certificate.
 * Returns -1 if the certificate failed the verifications, 0 on success.
 */
static int crt_verify_with_profile(SOPC_CertificateList* pToValidate,
                                   const cyclone_crypto_crt_profile* crt_profile,
                                   uint32_t* failure_reasons)
{
    /* 1) Check the type and size of the key */
    X509KeyType keyType = x509GetPublicKeyType(pToValidate->crt.tbsCert.subjectPublicKeyInfo.oid,
                                               pToValidate->crt.tbsCert.subjectPublicKeyInfo.oidLen);
    if (X509_KEY_TYPE_UNKNOWN == keyType)
    {
        *failure_reasons |= X509_BADCERT_BAD_PK;
    }
    // Size.
    size_t keySize = pToValidate->crt.tbsCert.subjectPublicKeyInfo.rsaPublicKey.nLen * 8;
    if (keySize < crt_profile->rsa_min_bitlen)
    {
        *failure_reasons |= X509_BADCERT_BAD_KEY;
    }

    /* 2) Check if md and pk algos of the signature of the cert suits to the profile. */
    X509SignatureAlgo signAlgo = {0};
    const HashAlgo* hashAlgo = NULL;
    // Returns ERROR_UNSUPPORTED_SIGNATURE_ALGO on errors
    error_t errLib = x509GetSignHashAlgo(&pToValidate->crt.signatureAlgo, &signAlgo, &hashAlgo);
    if (ERROR_UNSUPPORTED_SIGNATURE_ALGO == errLib)
    {
        *failure_reasons |= X509_BADCERT_BAD_PK;
    }

    // hashAlgo should be at least SHA-256.
    uint32_t hashAlgoId = HashAlgoToBitmask(hashAlgo);
    if (0 == (hashAlgoId & crt_profile->allowed_mds)) // If the hash algo is not an allowed md.
    {
        *failure_reasons |= X509_BADCERT_BAD_MD;
    }

    if (0 != *failure_reasons) // If there's at least one failure reason, return error
    {
        return -1;
    }

    return 0;
}

static SOPC_ReturnStatus sopc_validate_certificate(const SOPC_PKIProvider* pPKI,
                                                   SOPC_CertificateList* cert,
                                                   cyclone_crypto_crt_profile* crt_profile,
                                                   bool bIsSelfSigned,
                                                   bool bForceTrustedCert,
                                                   bool bDisableRevocationCheck,
                                                   const char* thumbprint,
                                                   uint32_t* error)
{
    SOPC_ASSERT(NULL != pPKI);
    SOPC_ASSERT(NULL != cert);
    SOPC_ASSERT(NULL == cert->next);
    SOPC_ASSERT(NULL != crt_profile);
    SOPC_ASSERT(NULL != thumbprint);
    SOPC_ASSERT(NULL != error);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_CRLList* cert_crl = pPKI->pAllCrl;
    /* Note: we consider all root CAs as trusted since our criteria is that at least one certificate is trusted in the
     *       chain and it might not be the root CA.
     *       We do an additional check that it is the case during the verification.
     */
    SOPC_CertificateList* ca_root = (NULL != pPKI->pAllRoots ? pPKI->pAllRoots : NULL);
    SOPC_CRLList* crl = (NULL != cert_crl ? cert_crl : NULL);
    /* Link certificate to validate with intermediate certificates (trusted links or untrusted links) */
    SOPC_CertificateList* pLinkCert = pPKI->pAllCerts;

    /* Common verifications on the certificate (if it is self-signed or not) */
    uint32_t failure_reasons = 0;
    int ret = crt_verify_with_profile(cert, crt_profile, &failure_reasons);

    SOPC_CheckTrustedAndCRLinChain checkTrustedAndCRL = {.trustedCrts = pPKI->pAllTrusted,
                                                         .allCRLs = pPKI->pAllCrl,
                                                         .isTrustedInChain = bForceTrustedCert,
                                                         .disableRevocationCheck = bDisableRevocationCheck};
    /* CycloneCRYPTO : x509ValidateCertificate() function returns error if
     * the issuer of the certificate is not CA. It is then a special case.
     */
    if (bIsSelfSigned)
    {
        ret = crt_verify_self_signed(cert, crl, &failure_reasons, &checkTrustedAndCRL);
    }
    else
    {
        /* If certificate is not self signed, add the intermediate certificates to the trust chain to evaluate */
        cert->next = pLinkCert;
        /* Verify the certificate chain */
        ret = crt_verify_chain(cert, ca_root, crl, crt_profile, &failure_reasons, &checkTrustedAndCRL);
    }

    // Check if at a least one trusted certificate is present in trust chain
    if (!checkTrustedAndCRL.isTrustedInChain)
    {
        ret = -1;
        failure_reasons = (failure_reasons | (uint32_t) X509_BADCERT_NOT_TRUSTED);
    }

    if (0 != ret)
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
        bMatchName = x509CompareName(pCert->crt.tbsCert.subject.rawData, pCert->crt.tbsCert.subject.rawDataLen,
                                     cur->crt.tbsCert.subject.rawData, cur->crt.tbsCert.subject.rawDataLen);
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
    if (UINT32_MAX < pCert->raw->length)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
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
                    SOPC_KeyManager_Certificate_Free(cur);
                    pPKI->pRejectedList = next; /* Copy with an assignment operator */
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

static SOPC_ReturnStatus sopc_PKI_validate_profile_and_certificate(SOPC_PKIProvider* pPKI,
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
    /* Certificate shall not be a CA or only for self-signed backward compatibility
       (and pathLen shall be 0 which means crt.max_pathlen is 1 due to mbedtls choice: 1 higher than RFC 5280) */
    if (!pToValidateCpy->crt.tbsCert.extensions.basicConstraints.cA ||
        (bIsSelfSigned && pProfile->bBackwardInteroperability &&
         1 == pToValidateCpy->crt.tbsCert.extensions.basicConstraints.pathLenConstraint))
    {
        // No error detected
    }
    else
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

    cyclone_crypto_crt_profile crt_profile = {0};
    /* Set the profile from configuration */
    status = set_profile_from_configuration(pProfile->chainProfile, &crt_profile);

    if (SOPC_STATUS_OK == status)
    {
        status = sopc_validate_certificate(pPKI, pToValidateCpy, &crt_profile, bIsSelfSigned, false,
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

static SOPC_ReturnStatus sopc_validate_anything(SOPC_PKIProvider* pPKI,
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

SOPC_ReturnStatus SOPC_PKIProvider_ValidateCertificate(SOPC_PKIProvider* pPKI,
                                                       const SOPC_CertificateList* pToValidate,
                                                       const SOPC_PKI_Profile* pProfile,
                                                       uint32_t* error)
{
    if (NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != pPKI->pFnValidateCert)
    {
        status = pPKI->pFnValidateCert(pPKI, pToValidate, pProfile, error);
    }
    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
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
                                                       cyclone_crypto_crt_profile* crt_profile,
                                                       const bool bDisableRevocationCheck,
                                                       bool* bErrorFound,
                                                       SOPC_Array* pErrors,
                                                       SOPC_Array* pThumbprints)
{
    SOPC_ASSERT(NULL != pPkiCerts);
    SOPC_ASSERT(NULL != crt_profile);
    SOPC_ASSERT(NULL != pErrors);
    SOPC_ASSERT(NULL != pThumbprints);

    SOPC_CertificateList* pCertsCpy = NULL;
    SOPC_CertificateList* crtThumbprint = NULL;
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
        crtThumbprint = crt;
        thumbprint = SOPC_KeyManager_Certificate_GetCstring_SHA1(crtThumbprint);
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
            statusChain = sopc_validate_certificate(pPKI, crt, crt_profile, bIsSelfSigned, forceTrustedCert,
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
    cyclone_crypto_crt_profile crt_profile = {0};
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

SOPC_ReturnStatus SOPC_PKIProvider_CheckLeafCertificate(const SOPC_CertificateList* pToValidate,
                                                        const SOPC_PKI_LeafProfile* pProfile,
                                                        uint32_t* error)
{
    if (NULL == pToValidate || NULL == pProfile || NULL == error)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *error = SOPC_CertificateValidationError_Unknown;
    uint32_t firstError = SOPC_CertificateValidationError_Unknown;
    uint32_t currentError = SOPC_CertificateValidationError_Unknown;
    bool bErrorFound = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (pProfile->bApplySecurityPolicy)
    {
        status = check_security_policy(pToValidate, pProfile);
        if (SOPC_STATUS_OK != status)
        {
            firstError = SOPC_CertificateValidationError_PolicyCheckFailed;
            bErrorFound = true;
        }
    }

    if (NULL != pProfile->sanURL)
    {
        status = check_host_name(pToValidate, pProfile->sanURL);
        if (SOPC_STATUS_OK != status)
        {
            currentError = SOPC_CertificateValidationError_HostNameInvalid;
            if (!bErrorFound)
            {
                firstError = currentError;
                bErrorFound = true;
            }
        }
    }

    if (NULL != pProfile->sanApplicationUri)
    {
        status = check_application_uri(pToValidate, pProfile->sanApplicationUri);
        if (SOPC_STATUS_OK != status)
        {
            currentError = SOPC_CertificateValidationError_UriInvalid;
            if (!bErrorFound)
            {
                firstError = currentError;
                bErrorFound = true;
            }
        }
    }
    status = check_certificate_usage(pToValidate, pProfile);
    if (SOPC_STATUS_OK != status)
    {
        currentError = SOPC_CertificateValidationError_UseNotAllowed;
        if (!bErrorFound)
        {
            firstError = currentError;
            bErrorFound = true;
        }
    }

    if (bErrorFound)
    {
        *error = firstError;
        status = SOPC_STATUS_NOK;
    }

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
    else
    {
        if (bIscrl)
        {
            *ppCrl = pCrl;
        }
        else
        {
            *ppCerts = pCerts;
        }
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
                SOPC_CertificateList* next = cur->next;
                // Dereferencing cur->next, otherwise freeing cur will free the whole chain
                cur->next = NULL;
                // We don't call SOPC_KeyManager_Certificate_Free() because it will delete the CertificateList.
                SOPC_Buffer_Delete(cur->raw);
                rsaFreePublicKey(&cur->pubKey);
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
                        *pHeadCerts = *next;
                        SOPC_Free(next);
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
    SOPC_CertificateList* pCertCopy = pCert;
    while (NULL != pCertCopy)
    {
        *listLength = *listLength + 1;
        if (pCertCopy->crt.tbsCert.extensions.basicConstraints.cA)
        {
            *caCount = *caCount + 1;
            SOPC_KeyManager_Certificate_IsSelfSigned(pCertCopy, &is_self_sign);
            if (is_self_sign)
            {
                *rootCount = *rootCount + 1;
            }
        }
        pCertCopy = pCertCopy->next;
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
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                 "> PKI creation warning: trusted CA certificates are provided but no CRL.");
    }
    get_list_stats(pIssuerCerts, &issuer_ca_count, &issuer_list_length, &issuer_root_count);
    /* issuer CA => issuer CRL*/
    if (0 != issuer_ca_count && NULL == pIssuerCrl)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                 "> PKI creation warning: issuer CA certificates are provided but no CRL.");
    }
    /* Check if issuerCerts list is only filled with CA. */
    if (issuer_list_length != issuer_ca_count)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "> PKI creation error: not all issuer certificates are CAs.");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* check and warn in case no roots is provided wheras at least one trusted leaf certificate is provided. */
    if ((0 == issuer_root_count) && (0 == trusted_root_count) && (0 != issued_cert_count))
    {
        /* In this case, only trusted self-signed leaf certificates will be accepted. */
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                 "> PKI creation warning: no root (CA) defined: only trusted self-signed leaf "
                                 "certificates will be accepted without possibility to revoke them (no CRL).");
    }
    *bTrustedCaFound = 0 != trusted_ca_count;
    *bIssuerCaFound = 0 != issuer_ca_count;
    return status;
}

static SOPC_ReturnStatus get_list_length(const SOPC_CertificateList* pTrustedCerts,
                                         const SOPC_CRLList* pTrustedCrl,
                                         const SOPC_CertificateList* pIssuerCerts,
                                         const SOPC_CRLList* pIssuerCrl,
                                         uint32_t* listLength)
{
    *listLength = 0;
    size_t lenTrustedCerts = 0;
    size_t lenTrustedCrl = 0;
    size_t lenIssuerCerts = 0;
    size_t lenIssuerCrl = 0;
    size_t lenTot = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL != pTrustedCerts)
    {
        status = SOPC_KeyManager_Certificate_GetListLength(pTrustedCerts, &lenTrustedCerts);
    }
    if (NULL != pTrustedCrl && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_GetListLength(pTrustedCrl, &lenTrustedCrl);
    }
    if (NULL != pIssuerCerts && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_GetListLength(pIssuerCerts, &lenIssuerCerts);
    }
    if (NULL != pIssuerCrl && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_GetListLength(pIssuerCrl, &lenIssuerCrl);
    }
    if (SOPC_STATUS_OK == status)
    {
        lenTot = lenTrustedCerts + lenTrustedCrl + lenIssuerCerts + lenIssuerCrl;
        if (UINT32_MAX < lenTot)
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
        else
        {
            *listLength = (uint32_t) lenTot;
        }
    }
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
    SOPC_CertificateList* tmp_pAllTrusted = NULL;   /* trusted CAs and certs */

    SOPC_CRLList* tmp_pAllCrl = NULL; /* issuer crl + trusted crl  */

    SOPC_CertificateList* tmp_pTrustedCerts = NULL; /* trusted intermediate CA + trusted certificates */
    SOPC_CRLList* tmp_pTrustedCrl = NULL;           /* CRLs of trusted intermediate CA and trusted root CA */
    SOPC_CertificateList* tmp_pIssuerCerts = NULL;  /* issuer intermediate CA + issuer root CA */
    SOPC_CRLList* tmp_pIssuerCrl = NULL;            /* CRLs of issuer intermediate CA and issuer root CA */
    bool bTrustedCaFound = false;
    bool bIssuerCaFound = false;
    uint32_t listLength = 0;

    if (NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Check the number of certificates plus CRLs of the PKI */
    status = get_list_length(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, &listLength);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    if (SOPC_PKI_MAX_NB_CERT_AND_CRL < listLength)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI creation error: too many (%" PRIu32
                               ") certificates and CRLs. The maximum configured is %" PRIu32
                               ", please change SOPC_PKI_MAX_NB_CERT_AND_CRL",
                               listLength, (uint32_t) SOPC_PKI_MAX_NB_CERT_AND_CRL);
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
        if (bTrustedCaFound && NULL != tmp_pTrustedCrl)
        {
            status = SOPC_KeyManagerInternal_CertificateList_CheckCRL(tmp_pTrustedCerts, tmp_pTrustedCrl, &bTrustedCRL);
        }
        else
        {
            bTrustedCRL = true;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (bIssuerCaFound && NULL != tmp_pIssuerCrl)
        {
            status = SOPC_KeyManagerInternal_CertificateList_CheckCRL(tmp_pIssuerCerts, tmp_pIssuerCrl, &bIssuerCRL);
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
                "> PKI creation warning: Not all certificate authorities in given trusted certificates have at least "
                "one certificate revocation list! Certificates issued by these CAs will be refused.");
        }
        if (!bIssuerCRL)
        {
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_COMMON,
                "> PKI creation warning: Not all certificate authorities in given issuer certificates have at least "
                "one certificate revocation list! Certificates issued by these CAs will be refused.");
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
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_Copy(pTrustedCerts, &tmp_pAllTrusted);
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
        SOPC_ReturnStatus mutStatus = SOPC_Mutex_Initialization(&pPKI->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        pPKI->pTrustedRoots = tmp_pTrustedRoots;
        pPKI->pTrustedCerts = tmp_pTrustedCerts;
        pPKI->pTrustedCrl = tmp_pTrustedCrl;
        pPKI->pIssuerRoots = tmp_pIssuerRoots;
        pPKI->pIssuerCerts = tmp_pIssuerCerts;
        pPKI->pIssuerCrl = tmp_pIssuerCrl;
        pPKI->pAllCerts = tmp_pAllCerts;
        pPKI->pAllRoots = tmp_pAllRoots;
        pPKI->pAllTrusted = tmp_pAllTrusted;
        pPKI->pAllCrl = tmp_pAllCrl;
        pPKI->pRejectedList = NULL;
        pPKI->directoryStorePath = NULL;
        pPKI->pFnValidateCert = &sopc_PKI_validate_profile_and_certificate;
        pPKI->isPermissive = false;
        *ppPKI = pPKI;
    }
    else
    {
        SOPC_KeyManager_Certificate_Free(tmp_pTrustedRoots);
        SOPC_KeyManager_Certificate_Free(tmp_pIssuerRoots);
        SOPC_KeyManager_Certificate_Free(tmp_pAllRoots);
        SOPC_KeyManager_Certificate_Free(tmp_pAllTrusted);
        SOPC_KeyManager_Certificate_Free(tmp_pTrustedCerts);
        SOPC_KeyManager_Certificate_Free(tmp_pIssuerCerts);
        SOPC_KeyManager_Certificate_Free(tmp_pAllCerts);
        SOPC_KeyManager_CRL_Free(tmp_pTrustedCrl);
        SOPC_KeyManager_CRL_Free(tmp_pIssuerCrl);
        SOPC_KeyManager_CRL_Free(tmp_pAllCrl);
    }

    return status;
}

static bool pki_updated_trust_list_dir_exists(const char* path)
{
    SOPC_Array* pFilePaths = NULL;
    SOPC_FileSystem_GetDirResult dirRes = SOPC_FileSystem_GetDirFilePaths(path, &pFilePaths);
    SOPC_Array_Delete(pFilePaths);
    return (SOPC_FileSystem_GetDir_OK == dirRes);
}

static SOPC_ReturnStatus pki_create_from_store(
    const char* directoryStorePath,
    bool bDefaultBuild, /* If true load the root PKI directory without trust list update,
                           if false try to load the updated trust list subdirectory.*/
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

    /* Select the right folder: add updateTrustList subdirectory path containing PKI update*/
    if (!bDefaultBuild)
    {
        status = SOPC_StrConcat(directoryStorePath, STR_TRUSTLIST_NAME, &path);
        if (SOPC_STATUS_OK != status)
        {
            return status;
        }
        if (pki_updated_trust_list_dir_exists(path))
        {
            basePath = path;
        }
        else
        {
            SOPC_Free(path);
            path = NULL;
            status = SOPC_STATUS_WOULD_BLOCK;
        }
    }
    else
    {
        basePath = directoryStorePath;
    }
    /* Load the files from the directory Store path */
    if (SOPC_STATUS_OK == status)
    {
        status = load_certificate_and_crl_list_from_store(basePath, &pTrustedCerts, &pTrustedCrl, &pIssuerCerts,
                                                          &pIssuerCrl, bDefaultBuild);
    }
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
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_COMMON,
                              "> PKI creation: loading PKI store root directory %s content (default behavior).",
                              directoryStorePath);
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_COMMON,
                              "> PKI creation: the updated PKI store subdirectory %s%s is absent"
                              " or its content cannot be loaded (see warnings in this case).",
                              directoryStorePath, STR_TRUSTLIST_NAME);
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
                *ppPKI = NULL;
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

SOPC_ReturnStatus SOPC_PKIPermissive_Create(SOPC_PKIProvider** ppPKI)
{
    SOPC_PKIProvider* pPKI = NULL;

    if (NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pPKI = SOPC_Calloc(1, sizeof(SOPC_PKIProvider));

    if (NULL == pPKI)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Initialization(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    pPKI->pTrustedRoots = NULL;
    pPKI->pTrustedCerts = NULL;
    pPKI->pTrustedCrl = NULL;
    pPKI->pIssuerRoots = NULL;
    pPKI->pIssuerCerts = NULL;
    pPKI->pIssuerCrl = NULL;
    pPKI->pAllCerts = NULL;
    pPKI->pAllRoots = NULL;
    pPKI->pAllCrl = NULL;
    pPKI->pRejectedList = NULL;
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
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_KeyManager_Certificate_Free(pPKI->pTrustedRoots);
    SOPC_KeyManager_Certificate_Free(pPKI->pIssuerRoots);
    SOPC_KeyManager_Certificate_Free(pPKI->pAllRoots);
    SOPC_KeyManager_Certificate_Free(pPKI->pAllTrusted);
    SOPC_KeyManager_Certificate_Free(pPKI->pTrustedCerts);
    SOPC_KeyManager_Certificate_Free(pPKI->pIssuerCerts);
    SOPC_KeyManager_Certificate_Free(pPKI->pAllCerts);
    SOPC_KeyManager_CRL_Free(pPKI->pTrustedCrl);
    SOPC_KeyManager_CRL_Free(pPKI->pIssuerCrl);
    SOPC_KeyManager_CRL_Free(pPKI->pAllCrl);
    SOPC_KeyManager_Certificate_Free(pPKI->pRejectedList);
    SOPC_Free(pPKI->directoryStorePath);
    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    mutStatus = SOPC_Mutex_Clear(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
}

void SOPC_PKIProvider_Free(SOPC_PKIProvider** ppPKI)
{
    if (NULL == ppPKI || NULL == *ppPKI)
    {
        return;
    }
    sopc_pki_clear(*ppPKI);
    SOPC_Free(*ppPKI);
    *ppPKI = NULL;
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

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!pPKI->isPermissive)
    {
        status = SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Create if necessary the store */
        SOPC_FileSystem_CreationResult mkdir_res = SOPC_FileSystem_mkdir(directoryStorePath);
        if (SOPC_FileSystem_Creation_Error_PathAlreadyExists != mkdir_res && SOPC_FileSystem_Creation_OK != mkdir_res)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Copy the directory store path before exchange the data */
        char* pCopyPath = SOPC_strdup(directoryStorePath);
        if (NULL == pCopyPath)
        {
            status = SOPC_STATUS_NOK;
        }
        SOPC_Free(pPKI->directoryStorePath);
        pPKI->directoryStorePath = pCopyPath;
    }

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_WriteOrAppendToList(SOPC_PKIProvider* pPKI,
                                                       SOPC_CertificateList** ppTrustedCerts,
                                                       SOPC_CRLList** ppTrustedCrl,
                                                       SOPC_CertificateList** ppIssuerCerts,
                                                       SOPC_CRLList** ppIssuerCrl)
{
    if (NULL == pPKI || NULL == ppTrustedCerts || NULL == ppTrustedCrl || NULL == ppIssuerCerts || NULL == ppIssuerCrl)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!pPKI->isPermissive)
    {
        status = SOPC_STATUS_OK;
    }

    SOPC_CertificateList* pTrustedCerts = *ppTrustedCerts;
    SOPC_CRLList* pTrustedCrl = *ppTrustedCrl;
    SOPC_CertificateList* pIssuerCerts = *ppIssuerCerts;
    SOPC_CRLList* pIssuerCrl = *ppIssuerCrl;

    if (SOPC_STATUS_OK == status)
    {
        status = merge_certificates(pPKI->pTrustedRoots, pPKI->pTrustedCerts, &pTrustedCerts);
    }
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

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_WriteToStore(SOPC_PKIProvider* pPKI, const bool bEraseExistingFiles)
{
    if (NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    char* basePath = NULL;
    char* path = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!pPKI->isPermissive)
    {
        status = SOPC_STATUS_OK;
    }
    /* The case of the PKI is built from buffer (there is no store) */
    if (SOPC_STATUS_OK == status && NULL == pPKI->directoryStorePath)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = may_create_pki_folder(pPKI->directoryStorePath, STR_TRUSTLIST_NAME, &basePath);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = may_create_pki_folder(basePath, STR_TRUSTED, &path);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_TRUSTED_CERTS, &path);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Note: might use pPKI->pAllTrusted instead which is equivalent to the merged list
        status = write_cert_to_der_files(pPKI->pTrustedRoots, pPKI->pTrustedCerts, path, bEraseExistingFiles);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_TRUSTED_CRL, &path);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = write_crl_to_der_files(pPKI->pTrustedCrl, path, bEraseExistingFiles);
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
    }
    if (SOPC_STATUS_OK == status)
    {
        status = write_cert_to_der_files(pPKI->pIssuerRoots, pPKI->pIssuerCerts, path, bEraseExistingFiles);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Free(path);
        status = may_create_pki_folder(basePath, STR_ISSUERS_CRL, &path);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = write_crl_to_der_files(pPKI->pIssuerCrl, path, bEraseExistingFiles);
    }

    SOPC_Free(basePath);
    SOPC_Free(path);

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_CopyRejectedList(SOPC_PKIProvider* pPKI, SOPC_CertificateList** ppCert)

{
    if (NULL == pPKI || NULL == ppCert)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CertificateList* pRejected = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!pPKI->isPermissive)
    {
        status = SOPC_STATUS_OK;
    }

    bool bFound = false;
    SOPC_CertificateList* next = NULL;
    SOPC_CertificateList* crt = NULL != pPKI->pRejectedList ? pPKI->pRejectedList : NULL;
    while (NULL != crt && SOPC_STATUS_OK == status)
    {
        /* Unlink */
        next = crt->next;
        crt->next = NULL;
        if (NULL != pRejected)
        {
            SOPC_CertificateList* crtToFind = crt;
            status = SOPC_KeyManager_CertificateList_FindCertInList(pRejected, crtToFind, &bFound);
        }
        if (!bFound && SOPC_STATUS_OK == status)
        {
            if (UINT32_MAX < crt->raw->length)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(crt->raw->data, crt->raw->length, &pRejected);
            }
        }
        /* Link */
        crt->next = next;
        /* iterate */
        crt = crt->next;
        bFound = false;
    }
    /* Clear */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_KeyManager_Certificate_Free(pRejected);
        pRejected = NULL;
    }
    *ppCert = pRejected;

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_WriteRejectedCertToStore(SOPC_PKIProvider* pPKI)

{
    if (NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    char* path = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!pPKI->isPermissive)
    {
        status = SOPC_STATUS_OK;
    }

    /* The case of the PKI is built from buffer (there is no store) */
    if (SOPC_STATUS_OK == status && NULL == pPKI->directoryStorePath)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = may_create_pki_folder(pPKI->directoryStorePath, STR_REJECTED, &path);
        if (SOPC_STATUS_OK == status)
        {
            status = remove_files(path);
        }
    }
    if (SOPC_STATUS_OK == status && NULL != pPKI->pRejectedList)
    {
        status = SOPC_KeyManager_Certificate_ToDER_Files(pPKI->pRejectedList, path);
    }
    SOPC_Free(path);

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

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

static SOPC_ReturnStatus check_list_length(SOPC_PKIProvider* pPKI,
                                           SOPC_CertificateList* pTrustedCerts,
                                           SOPC_CRLList* pTrustedCrl,
                                           SOPC_CertificateList* pIssuerCerts,
                                           SOPC_CRLList* pIssuerCrl,
                                           const bool bIncludeExistingList)
{
    SOPC_ASSERT(NULL != pPKI);
    uint32_t PKILen = 0;
    uint32_t updateLen = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (bIncludeExistingList)
    {
        status = get_list_length(pPKI->pTrustedCerts, pPKI->pTrustedCrl, pPKI->pIssuerCerts, pPKI->pIssuerCrl, &PKILen);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = get_list_length(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, &updateLen);
    }
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    if (SOPC_PKI_MAX_NB_CERT_AND_CRL < PKILen + updateLen)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "> PKI creation error: too many (%" PRIu32
                               ") certificates and CRLs. The maximum configured is %" PRIu32
                               ", please change SOPC_PKI_MAX_NB_CERT_AND_CRL",
                               PKILen + updateLen, (uint32_t) SOPC_PKI_MAX_NB_CERT_AND_CRL);
    }
    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_UpdateFromList(SOPC_PKIProvider* pPKI,
                                                  const char* securityPolicyUri,
                                                  SOPC_CertificateList* pTrustedCerts,
                                                  SOPC_CRLList* pTrustedCrl,
                                                  SOPC_CertificateList* pIssuerCerts,
                                                  SOPC_CRLList* pIssuerCrl,
                                                  const bool bIncludeExistingList)
{
    /* Check parameters */
    if (NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_PKIProvider* pTmpPKI = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!pPKI->isPermissive)
    {
        status = SOPC_STATUS_OK;
    }

    /* Check the number of certificates plus CRLs */
    if (SOPC_STATUS_OK == status)
    {
        status = check_list_length(pPKI, pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, bIncludeExistingList);
    }

    /* Handle that the security level of the update isn't higher than the
       security level of the secure channel. (§7.3.4 part 2 v1.05) */
    if (SOPC_STATUS_OK == status)
    {
        status =
            check_security_level_of_the_update(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, securityPolicyUri);
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Includes the existing TrustList plus any updates */
        if (bIncludeExistingList)
        {
            SOPC_CertificateList* tmp_pTrustedCerts = NULL; /* trusted intermediate CA + trusted certificates */
            SOPC_CertificateList* tmp_pTrustedCertsTmp = NULL;
            SOPC_CRLList* tmp_pTrustedCrl = NULL;          /* CRLs of trusted intermediate CA and trusted root CA */
            SOPC_CertificateList* tmp_pIssuerCerts = NULL; /* issuer intermediate CA + issuer root CA */
            SOPC_CertificateList* tmp_pIssuerCertsTmp = NULL;
            SOPC_CRLList* tmp_pIssuerCrl = NULL; /* CRLs of issuer intermediate CA and issuer root CA */

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

            SOPC_KeyManager_Certificate_Free(tmp_pTrustedCerts);
            SOPC_KeyManager_Certificate_Free(tmp_pTrustedCertsTmp);
            SOPC_KeyManager_Certificate_Free(tmp_pIssuerCerts);
            SOPC_KeyManager_Certificate_Free(tmp_pIssuerCertsTmp);
            SOPC_KeyManager_CRL_Free(tmp_pTrustedCrl);
            SOPC_KeyManager_CRL_Free(tmp_pIssuerCrl);
        }
        else
        {
            /* Create a new tmp PKI without the existing TrustList */
            status = SOPC_PKIProvider_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl, &pTmpPKI);
        }
    }
    /* Copy the rejected list before exchange the data */
    if (SOPC_STATUS_OK == status && NULL != pPKI->pRejectedList)
    {
        status = SOPC_KeyManager_Certificate_Copy(pPKI->pRejectedList, &pTmpPKI->pRejectedList);
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

    // Exchange the internal data between tmpPKI and PKI, clear previous data and free tmpPKI structure
    // Note: mutex is kept since PKI should already be in use
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Internal_ReplacePKIAndClear(pPKI, &pTmpPKI);
    }

    // In case of failure we need to clear and free the temporary PKI
    if (NULL != pTmpPKI)
    {
        sopc_pki_clear(pTmpPKI);
        SOPC_Free(pTmpPKI);
        pTmpPKI = NULL;
    }

    // Unlock PKI prior to possibly clearing it
    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

static SOPC_ReturnStatus sopc_pki_remove_cert_by_thumbprint(SOPC_CertificateList** ppList,
                                                            SOPC_CRLList** ppCRLList,
                                                            const char* pThumbprint,
                                                            const char* listName,
                                                            bool* pbIsRemoved,
                                                            bool* pbIsIssuer)
{
    SOPC_ASSERT(NULL != ppList);
    SOPC_ASSERT(NULL != ppCRLList);
    SOPC_ASSERT(NULL != pThumbprint);
    SOPC_ASSERT(NULL != pbIsRemoved);
    SOPC_ASSERT(NULL != pbIsIssuer);

    size_t lenThumb = strlen(pThumbprint);
    SOPC_ASSERT(HEX_THUMBPRINT_SIZE == lenThumb);

    /* Initialized the value to return */
    *pbIsRemoved = false;
    *pbIsIssuer = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == *ppList)
    {
        /* the certificate list is empty, do nothing*/
        return SOPC_STATUS_OK;
    }

    uint32_t count = 0;
    bool bIsIssuer = false;
    bool bAtLeastOneIssuer = false;
    bool bAtLeastOne = false;

    bool bCertIsRemoved = true;
    while (bCertIsRemoved && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CertificateList_RemoveCertFromSHA1(ppList, ppCRLList, pThumbprint, &bCertIsRemoved,
                                                                    &bIsIssuer);
        if (bCertIsRemoved)
        {
            if (bIsIssuer)
            {
                bAtLeastOneIssuer = true;
            }
            if (bAtLeastOneIssuer && !bIsIssuer)
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                         "> PKI remove: certificate thumbprint <%s> has been found both as CA and as "
                                         "leaf certificate from %s",
                                         pThumbprint, listName);
            }
            bAtLeastOne = true;
            count = count + 1;
        }
    }

    if (bAtLeastOne && NULL != listName)
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_COMMON,
                               "> PKI remove: certificate thumbprint <%s> has been removed (%" PRIu32 " times) from %s",
                               pThumbprint, count, listName);
    }

    *pbIsIssuer = bAtLeastOneIssuer;
    *pbIsRemoved = bAtLeastOne;
    return status;
}

SOPC_ReturnStatus SOPC_PKIProvider_RemoveCertificate(SOPC_PKIProvider* pPKI,
                                                     const char* pThumbprint,
                                                     const bool bIsTrusted,
                                                     bool* pIsRemoved,
                                                     bool* pIsIssuer)
{
    /* Initialized the value to return */
    *pIsRemoved = false;
    *pIsIssuer = false;
    if (NULL == pPKI || NULL == pThumbprint)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    size_t lenThumbprint = strlen(pThumbprint);
    if (HEX_THUMBPRINT_SIZE != lenThumbprint)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    bool bRootIsRemoved = false;
    bool bCertIsRemoved = false;
    bool bCertIsCA = false;
    bool bRootIsCA = false;

    bool bIsIssuer = false;
    bool bIsRemoved = false;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (!pPKI->isPermissive)
    {
        status = SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Remove from trusted certificates */
        if (bIsTrusted)
        {
            if (NULL != pPKI->pTrustedCerts)
            {
                status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pTrustedCerts, &pPKI->pTrustedCrl, pThumbprint,
                                                            "trusted list", &bCertIsRemoved, &bCertIsCA);
            }
            if (NULL != pPKI->pTrustedRoots && SOPC_STATUS_OK == status)
            {
                status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pTrustedRoots, &pPKI->pTrustedCrl, pThumbprint,
                                                            "trusted root list", &bRootIsRemoved, &bRootIsCA);
                SOPC_ASSERT(SOPC_STATUS_OK != status || bRootIsCA == bRootIsRemoved);
            }
            if (NULL != pPKI->pAllTrusted && SOPC_STATUS_OK == status)
            {
                bool bAllTrustedRemoved = false;
                bool bAllTrustedIsCA = false;
                status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pAllTrusted, &pPKI->pTrustedCrl, pThumbprint,
                                                            "trusted list", &bAllTrustedRemoved, &bAllTrustedIsCA);
                SOPC_ASSERT(SOPC_STATUS_OK != status || (bAllTrustedRemoved == (bRootIsRemoved || bCertIsRemoved) &&
                                                         (bAllTrustedIsCA == (bCertIsCA || bRootIsCA))));
            }
        }
        else
        {
            /* Remove from issuer certificates */
            if (NULL != pPKI->pIssuerCerts)
            {
                status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pIssuerCerts, &pPKI->pIssuerCrl, pThumbprint,
                                                            "issuer list", &bCertIsRemoved, &bCertIsCA);
                SOPC_ASSERT(SOPC_STATUS_OK != status || bCertIsCA == bCertIsRemoved);
            }
            if (NULL != pPKI->pIssuerRoots && SOPC_STATUS_OK == status)
            {
                status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pIssuerRoots, &pPKI->pIssuerCrl, pThumbprint,
                                                            "issuer root list", &bRootIsRemoved, &bRootIsCA);
                SOPC_ASSERT(SOPC_STATUS_OK != status || bRootIsCA == bRootIsRemoved);
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (bCertIsRemoved || bRootIsRemoved)
        {
            bIsIssuer = bCertIsCA || bRootIsCA;
            bIsRemoved = true;
        }
        else
        {
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                     "> PKI remove: certificate thumbprint <%s> has not been found", pThumbprint);
        }
    }
    if (SOPC_STATUS_OK == status && bIsRemoved)
    {
        bool bAllCertIsRemoved = false;
        bool bAllRootIsRemoved = false;
        bool bAllCertIsCA = false;
        bool bAllRootIsCA = false;
        /* Remove from all list */
        if (NULL != pPKI->pAllCerts)
        {
            status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pAllCerts, &pPKI->pAllCrl, pThumbprint, NULL,
                                                        &bAllCertIsRemoved, &bAllCertIsCA);
        }
        if (NULL != pPKI->pAllRoots && SOPC_STATUS_OK == status)
        {
            status = sopc_pki_remove_cert_by_thumbprint(&pPKI->pAllRoots, &pPKI->pAllCrl, pThumbprint, NULL,
                                                        &bAllRootIsRemoved, &bAllRootIsCA);
            SOPC_ASSERT(SOPC_STATUS_OK != status || bAllRootIsCA == bAllRootIsRemoved);
        }
        SOPC_ASSERT(SOPC_STATUS_OK != status || (bCertIsRemoved == bAllCertIsRemoved && bCertIsCA == bAllCertIsCA));
        SOPC_ASSERT(SOPC_STATUS_OK != status || (bRootIsRemoved == bAllRootIsRemoved && bRootIsCA == bAllRootIsCA));
    }

    *pIsIssuer = bIsIssuer;
    *pIsRemoved = bIsRemoved;

    mutStatus = SOPC_Mutex_Unlock(&pPKI->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}
