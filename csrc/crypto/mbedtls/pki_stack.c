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
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "sopc_crypto_provider.h"
#include "sopc_key_manager.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki.h"
#include "sopc_pki_stack.h"

#include "key_manager_lib.h"

#include "mbedtls/x509.h"

/**
 * The minimal profile supported by the PKIProviderStack. It requires cacert signed with
 *  at least SHA-256, with an RSA key of at least 2048 bits.
 */
static const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_minimal = {
    /* Hashes from SHA-256 and above */
    .allowed_mds = MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256) | MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA384) |
                   MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA512),
    .allowed_pks = 0xFFFFFFF,     /* Any PK alg */
    .allowed_curves = 0xFFFFFFFF, /* Any curve  */
    .rsa_min_bitlen = 2048,
};

static uint32_t PKIProviderStack_GetCertificateValidationError(uint32_t failure_reasons)
{
    if ((failure_reasons & MBEDTLS_X509_BADCERT_EXPIRED) != 0)
    {
        return SOPC_CertificateValidationError_TimeInvalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_REVOKED) != 0)
    {
        return SOPC_CertificateValidationError_Revoked;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_CN_MISMATCH) != 0)
    {
        return SOPC_CertificateValidationError_HostNameInvalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_NOT_TRUSTED) != 0)
    {
        return SOPC_CertificateValidationError_Untrusted;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCRL_NOT_TRUSTED) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCRL_EXPIRED) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_MISSING) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_SKIP_VERIFY) != 0)
    {
        return SOPC_CertificateValidationError_UseNotAllowed;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_OTHER) != 0)
    {
        return SOPC_CertificateValidationError_Untrusted;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_FUTURE) != 0)
    {
        return SOPC_CertificateValidationError_TimeInvalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCRL_FUTURE) != 0)
    {
        return SOPC_CertificateValidationError_RevocationUnknown;
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
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_BAD_MD) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_BAD_PK) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_BAD_KEY) != 0)
    {
        return SOPC_CertificateValidationError_Invalid;
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

    return SOPC_CertificateValidationError_Unkown;
}

static int verify_cert(void* trust_li, mbedtls_x509_crt* crt, int certificate_depth, uint32_t* flags)
{
    /* When we have an issued certificate that is trusted, but we don't trust its parents,
     * we have verified the chain certificates, but we should still mark the certificate as trusted.
     * "NOT_TRUSTED" should be the sole problem.
     */
    if (NULL != trust_li && 0 == certificate_depth && MBEDTLS_X509_BADCERT_NOT_TRUSTED == *flags)
    {
        /* Verify if crt is in trust_li */
        /* x509_crt_check_ee_locally_trusted() does this but only for self-signed end-entity certificates (!) */
        for (mbedtls_x509_crt* cur = (mbedtls_x509_crt*) trust_li; NULL != cur; cur = cur->next)
        {
            if (crt->raw.len == cur->raw.len && 0 == memcmp(crt->raw.p, cur->raw.p, crt->raw.len))
            {
                *flags = 0;
                break;
            }
        }
    }

    /* Other errors are MBEDTLS_ERR_X509_FATAL_ERROR, or app specific */
    return 0;
}

static SOPC_ReturnStatus PKIProviderStack_ValidateCertificate(const SOPC_PKIProvider* pPKI,
                                                              const SOPC_CertificateList* pToValidate,
                                                              uint32_t* error)
{
    (void) (pPKI);
    assert(NULL != error);
    *error = SOPC_CertificateValidationError_Unkown;
    SOPC_CertificateList* cert_ca = NULL;
    SOPC_CRLList* cert_rev_list = NULL;
    mbedtls_x509_crl* rev_list = NULL;
    uint32_t failure_reasons = 0;

    if (NULL == pPKI || NULL == pToValidate)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == pPKI->pFnValidateCertificate || NULL == pPKI->pUserCertAuthList) // TODO: useful pFn verification?
        return SOPC_STATUS_INVALID_PARAMETERS;

    // Gathers certificates from pki structure
    if (NULL != pPKI->pUserCertRevocList)
    {
        cert_rev_list = (SOPC_CRLList*) (pPKI->pUserCertRevocList);
        rev_list = (mbedtls_x509_crl*) (&cert_rev_list->crl);
    }
    cert_ca = (SOPC_CertificateList*) (pPKI->pUserCertAuthList);

    // Now, verifies the certificate
    // crt are not const in crt_verify, but this function does not look like to modify them

    // Assumption that certificate is not modified by mbedtls
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    if (mbedtls_x509_crt_verify_with_profile(
            (mbedtls_x509_crt*) (&pToValidate->crt), (mbedtls_x509_crt*) (&cert_ca->crt), rev_list,
            &mbedtls_x509_crt_profile_minimal, NULL, /* You can specify an expected Common Name here */
            &failure_reasons, verify_cert, &cert_ca->crt) != 0)
    {
        *error = PKIProviderStack_GetCertificateValidationError(failure_reasons);
        return SOPC_STATUS_NOK;
    }
    SOPC_GCC_DIAGNOSTIC_RESTORE

    return SOPC_STATUS_OK;
}

static void PKIProviderStack_Free(SOPC_PKIProvider* pPKI)
{
    if (pPKI == NULL)
    {
        return;
    }

    SOPC_KeyManager_Certificate_Free(pPKI->pUserCertAuthList);
    SOPC_Free(pPKI);
}

static SOPC_PKIProvider* create_pkistack(SOPC_CertificateList* ca, SOPC_CRLList* crl, void* pUserData)
{
    SOPC_PKIProvider* pki = SOPC_Malloc(sizeof(SOPC_PKIProvider));

    if (NULL != pki)
    {
        /* The pki function pointer shall be const after this init */
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        *(SOPC_PKIProvider_Free_Func*) (&pki->pFnFree) = &PKIProviderStack_Free;
        *(SOPC_FnValidateCertificate*) (&pki->pFnValidateCertificate) = &PKIProviderStack_ValidateCertificate;
        SOPC_GCC_DIAGNOSTIC_RESTORE

        pki->pUserCertAuthList = ca;
        pki->pUserCertRevocList = crl;
        pki->pUserData = pUserData;
    }

    return pki;
}

SOPC_ReturnStatus SOPC_PKIProviderStack_Create(SOPC_SerializedCertificate* pCertAuth,
                                               SOPC_CRLList* pRevocationList,
                                               SOPC_PKIProvider** ppPKI)
{
    if (NULL == pCertAuth || NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CertificateList* caCert = NULL;
    SOPC_PKIProvider* pki = NULL;
    SOPC_ReturnStatus status = SOPC_KeyManager_SerializedCertificate_Deserialize(pCertAuth, &caCert);

    if (SOPC_STATUS_OK == status)
    {
        pki = create_pkistack(caCert, pRevocationList, NULL);
        if (NULL == pki)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *ppPKI = pki;
    }
    /* Clear partial alloc */
    else
    {
        SOPC_KeyManager_Certificate_Free(caCert);
        SOPC_Free(pki);
    }

    return status;
}

SOPC_ReturnStatus SOPC_PKIProviderStack_CreateFromPaths(char** lPathCA, char** lPathCRL, SOPC_PKIProvider** ppPKI)
{
    if (NULL == lPathCA || NULL == lPathCRL || NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_CertificateList* ca = NULL;
    char *cur = *lPathCA;
    while (NULL != cur && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_CreateOrAddFromFile(cur, &ca);
        ++lPathCA;
        cur = *lPathCA;
    }

    SOPC_CRLList* crl = NULL;
    cur = *lPathCRL;
    while (NULL != cur && SOPC_STATUS_OK == status)
    {
        /* TODO: first create the CRL API */
        status = SOPC_STATUS_OK;
        ++lPathCRL;
        cur = *lPathCRL;
    }

    SOPC_PKIProvider* pki = NULL;
    if (SOPC_STATUS_OK == status)
    {
        pki = create_pkistack(ca, crl, NULL);
        if (NULL == pki)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *ppPKI = pki;
    }
    /* Clear partial alloc */
    else
    {
        SOPC_KeyManager_Certificate_Free(ca);
        /* TODO: CRL Free */
    }

    return status;
}
