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
#include "sopc_crypto_provider.h"
#include "sopc_key_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki.h"
#include "sopc_pki_stack.h"

#include "key_manager_lib.h"
#include "mbedtls_common.h"

#include "mbedtls/oid.h"
#include "mbedtls/ssl.h"
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

    /* Only fatal errors whould be returned here, as this error code will be forwarded to the caller of
     * mbedtls_x509_crt_verify_with_profile, and the verification stopped.
     * Errors may be MBEDTLS_ERR_X509_FATAL_ERROR, or application specific */
    return 0;
}

/* Returns 0 if all key usages and extended key usages are ok */
static SOPC_ReturnStatus check_key_usages(const mbedtls_x509_crt* crt, bool isUserPki)
{
    int err = 0;
    unsigned int usages = 0;
    if (!isUserPki)
    {
        usages = MBEDTLS_X509_KU_DIGITAL_SIGNATURE | MBEDTLS_X509_KU_NON_REPUDIATION |
                 MBEDTLS_X509_KU_KEY_ENCIPHERMENT | MBEDTLS_X509_KU_DATA_ENCIPHERMENT;
        err = mbedtls_x509_crt_check_key_usage(crt, usages);

        /* If the ext usage is neither server auth nor client auth, it shall be rejected */
        /* TODO: check whether the crt is for a server or a client, and only check the corresponding ext usage */
        bool missSer = mbedtls_x509_crt_check_extended_key_usage(crt, MBEDTLS_OID_SERVER_AUTH,
                                                                 MBEDTLS_OID_SIZE(MBEDTLS_OID_SERVER_AUTH));
        bool missCli = mbedtls_x509_crt_check_extended_key_usage(crt, MBEDTLS_OID_CLIENT_AUTH,
                                                                 MBEDTLS_OID_SIZE(MBEDTLS_OID_CLIENT_AUTH));
        if (missSer && missCli)
        {
            err = 1;
        }
    }
    else
    {
        /* Check the key usages for user certificate (it is not part of the OPC UA but it makes sense to keep it). */
        usages = MBEDTLS_X509_KU_DIGITAL_SIGNATURE;
        err = mbedtls_x509_crt_check_key_usage(crt, usages);
        if (0 == err)
        {
            /* The CA flag shall be FALSE for user certificate */
            err = crt->ca_istrue;
        }
    }

    if (0 == err)
    {
        return SOPC_STATUS_OK;
    }
    else
    {
        return SOPC_STATUS_NOK;
    }
}

static SOPC_ReturnStatus PKIProviderStack_ValidateCertificate(const SOPC_PKIProvider* pPKI,
                                                              const SOPC_CertificateList* pToValidate,
                                                              uint32_t* error)
{
    if (NULL == pPKI || NULL == pToValidate || NULL == error)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    *error = SOPC_CertificateValidationError_Unkown;

    /* There are two cases for the validation process:
     * - if pToValidate is an issued certificate, we validate it with untrusted CAs + trusted CAs
     * - otherwise, we validate pToValidate only with the trusted CAs
     * The CRL list always contains CRLs from both untrusted and trusted CA lists
     */
    bool bIssued = false;
    SOPC_ReturnStatus status =
        SOPC_KeyManager_CertificateList_FindCertInList(pPKI->pIssuedCertsList, pToValidate, &bIssued);
    if (SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CertificateList* trust_list = bIssued ? pPKI->pUntrustedIssuerRootsList : pPKI->pTrustedIssuerRootsList;
    SOPC_CRLList* cert_crl = pPKI->pCertRevocList;
    if ((NULL == trust_list || NULL == cert_crl) && !bIssued)
    {
        // Empty trust list is not valid if the certificate is not issued (only possibility to be valid if self-signed)
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Assumes that mbedtls does not modify the certificates */
    mbedtls_x509_crt* mbed_ca = (mbedtls_x509_crt*) (NULL != trust_list ? &trust_list->crt : NULL);
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    mbedtls_x509_crt* mbed_chall = (mbedtls_x509_crt*) (&pToValidate->crt);
    mbedtls_x509_crl* mbed_crl = (mbedtls_x509_crl*) (NULL != cert_crl ? &cert_crl->crl : NULL);
    SOPC_GCC_DIAGNOSTIC_RESTORE

    /* Check certificate usages */
    bool isUserPki = (bool) pPKI->pUserData;
    status = check_key_usages(mbed_chall, isUserPki);
    if (SOPC_STATUS_OK != status)
    {
        *error = SOPC_CertificateValidationError_UseNotAllowed;
    }

    /* Link certificate to validate with intermediate certificates (trusted links or untrusted links) */
    mbedtls_x509_crt* end_of_chall = mbed_chall;
    SOPC_ASSERT(NULL != end_of_chall);
    while (NULL != end_of_chall->next)
    {
        end_of_chall = end_of_chall->next;
    }
    /* end_of_chall is now the last certificate of the chain, link it with links */
    end_of_chall->next = bIssued ? pPKI->pUntrustedIssuerLinksList : pPKI->pTrustedIssuerLinksList;

    /* Verify the certificate chain */
    if (SOPC_STATUS_OK == status)
    {
        uint32_t failure_reasons = 0;
        if (mbedtls_x509_crt_verify_with_profile(mbed_chall, mbed_ca, mbed_crl, &mbedtls_x509_crt_profile_minimal,
                                                 NULL /* You can specify an expected Common Name here */,
                                                 &failure_reasons, verify_cert, &bIssued) != 0)
        {
            *error = PKIProviderStack_GetCertificateValidationError(failure_reasons);
            status = SOPC_STATUS_NOK;
        }
    }

    /* Unlink end_of_chall, otherwise destroying the pToValidate will also destroy trusted or untrusted links */
    end_of_chall->next = NULL;

    return status;
}

static void PKIProviderStack_Free(SOPC_PKIProvider* pPKI)
{
    if (pPKI == NULL)
    {
        return;
    }

    /* Deleting the untrusted list will also clear the trusted list, as they are linked.
     * Hence mbedtls will call free on (&pPKI->pUserTrustedIssersList.crt), which is pPKI->pUserTrustedIssersList.
     */
    /* TODO: As the lists are not always generated the same way, there may be cases were they are not linked.
     *       (see Create, as opposed to CreateFromPaths).
     */
    if (NULL != pPKI->pUntrustedIssuerRootsList)
    {
        SOPC_KeyManager_Certificate_Free(pPKI->pUntrustedIssuerRootsList);
    }
    else
    {
        SOPC_KeyManager_Certificate_Free(pPKI->pTrustedIssuerRootsList);
    }
    if (NULL != pPKI->pUntrustedIssuerLinksList)
    {
        SOPC_KeyManager_Certificate_Free(pPKI->pUntrustedIssuerLinksList);
    }
    else
    {
        SOPC_KeyManager_Certificate_Free(pPKI->pTrustedIssuerLinksList);
    }
    SOPC_KeyManager_Certificate_Free(pPKI->pIssuedCertsList);
    SOPC_KeyManager_CRL_Free(pPKI->pCertRevocList);
    SOPC_Free(pPKI);
}

static SOPC_PKIProvider* create_pkistack(SOPC_CertificateList* lRootsTrusted,
                                         SOPC_CertificateList* lLinksTrusted,
                                         SOPC_CertificateList* lRootsUntrusted,
                                         SOPC_CertificateList* lLinksUntrusted,
                                         SOPC_CertificateList* lIssued,
                                         SOPC_CRLList* lCrl,
                                         bool isUserPki)
{
    SOPC_PKIProvider* pki = SOPC_Malloc(sizeof(SOPC_PKIProvider));

    if (NULL != pki)
    {
        /* The pki function pointer shall be const after this init */
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        *((SOPC_PKIProvider_Free_Func**) (&pki->pFnFree)) = &PKIProviderStack_Free;
        *((SOPC_FnValidateCertificate**) (&pki->pFnValidateCertificate)) = &PKIProviderStack_ValidateCertificate;
        SOPC_GCC_DIAGNOSTIC_RESTORE

        pki->pTrustedIssuerRootsList = lRootsTrusted;
        pki->pTrustedIssuerLinksList = lLinksTrusted;
        pki->pUntrustedIssuerRootsList = lRootsUntrusted;
        pki->pUntrustedIssuerLinksList = lLinksUntrusted;
        pki->pIssuedCertsList = lIssued;
        pki->pCertRevocList = lCrl;
        pki->pUserData = (uintptr_t) isUserPki;
    }

    return pki;
}

SOPC_ReturnStatus SOPC_PKIProviderStack_SetUserCert(SOPC_PKIProvider* pPKI, bool bIsUserPki)
{
    if (NULL == pPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pPKI->pUserData = (uintptr_t) bIsUserPki;
    return SOPC_STATUS_OK;
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

    /* Check the CRL-CA association before creating the PKI */
    if (SOPC_STATUS_OK == status)
    {
        /* mbedtls does not verify that each CA has a CRL, so we must do it ourselves */
        bool match = false;
        status = SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(caCert, pRevocationList, &match);
        if (SOPC_STATUS_OK == status && !match)
        {
            fprintf(
                stderr,
                "> PKI creation warning: Not all certificate authorities have a single certificate revocation list! "
                "Certificates issued by these CAs will be refused.\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        pki = create_pkistack(caCert, NULL, NULL, NULL, NULL, pRevocationList, false);
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

static SOPC_CertificateList* load_certificate_list(char** paths, SOPC_ReturnStatus* status)
{
    SOPC_ASSERT(NULL != paths && NULL != status);

    SOPC_CertificateList* certs = NULL;
    char* cur = *paths;
    while (NULL != cur && SOPC_STATUS_OK == *status)
    {
        *status = SOPC_KeyManager_Certificate_CreateOrAddFromFile(cur, &certs);
        ++paths;
        cur = *paths;
    }

    return certs;
}

/** \brief Create the prev list if required */
static SOPC_ReturnStatus link_certificates(SOPC_CertificateList** ppPrev, SOPC_CertificateList** ppNext)
{
    SOPC_ASSERT(NULL != ppPrev && NULL != ppNext);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_CertificateList* prev = *ppPrev;
    SOPC_CertificateList* next = *ppNext;
    /* Link two existing lists */
    if (NULL != prev && NULL != next)
    {
        mbedtls_x509_crt* crt = &prev->crt;
        /* crt should not be NULL, as either untrusted is NULL or at least one cert was created */
        SOPC_ASSERT(NULL != crt);
        while (NULL != crt->next)
        {
            crt = crt->next;
        }
        /* crt is now the last certificate of the chain, link it with trusted */
        crt->next = &next->crt;
    }
    /* The second list exists, but not the first */
    else if (NULL != next)
    {
        /* When there are no untrusted, we must create the structure */
        /* TODO: avoid the duplication of the first element of trusted */
        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(next->crt.raw.p, (uint32_t) next->crt.raw.len, ppPrev);
        if (SOPC_STATUS_OK == status)
        {
            prev = *ppPrev;
            prev->crt.next = &next->crt;
        }
    }
    /* else: The first list exists, but not the second, or both don't exist: nothing to do */

    return status;
}

SOPC_ReturnStatus SOPC_PKIProviderStack_CreateFromPaths(char** lPathTrustedIssuerRoots,
                                                        char** lPathTrustedIssuerLinks,
                                                        char** lPathUntrustedIssuerRoots,
                                                        char** lPathUntrustedIssuerLinks,
                                                        char** lPathIssuedCerts,
                                                        char** lPathCRL,
                                                        SOPC_PKIProvider** ppPKI)
{
    if (NULL == lPathTrustedIssuerRoots || NULL == lPathTrustedIssuerLinks || NULL == lPathUntrustedIssuerRoots ||
        NULL == lPathUntrustedIssuerLinks || NULL == lPathIssuedCerts || NULL == lPathCRL || NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Creates chains of issuers which will be used to verify the issued certificates.
     * Create the root lists which are separated between untrusted and trusted.
     * Untrusted roots are used to verify issued certificates.
     * Trusted roots are used to verify unknown certificates.
     * Links issuers are used to complete the chain between end-entry certificates
     * (certificate received in the OPN process) and the roots.
     *
     * In practise, as certificates are linked lists in mbedtls,
     * to verify issued certificates, we chain untrusted and trusted certificates,
     * we create a single list, and chain them: untrusted -> trusted.
     */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_CertificateList* lRootsTrusted = load_certificate_list(lPathTrustedIssuerRoots, &status);
    SOPC_CertificateList* lLinksTrusted = load_certificate_list(lPathTrustedIssuerLinks, &status);
    SOPC_CertificateList* lRootsUntrusted = load_certificate_list(lPathUntrustedIssuerRoots, &status);
    SOPC_CertificateList* lLinksUntrusted = load_certificate_list(lPathUntrustedIssuerLinks, &status);
    SOPC_CertificateList* lIssued = load_certificate_list(lPathIssuedCerts, &status);

    SOPC_CRLList* lCrls = NULL;
    char* cur = *lPathCRL;
    while (NULL != cur && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromFile(cur, &lCrls);
        ++lPathCRL;
        cur = *lPathCRL;
    }

    /* Check the CRL-CA association before creating the PKI.
     * Untrusted root list contains all known root CAs,
     * and untrusted link list contains all intermediate CAs. */
    bool bTrustedRootsCRL = false;
    bool bUntrustedRootsCRL = false;
    bool bTrustedLinksCRL = false;
    bool bUntrustedLinksCRL = false;
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != lRootsTrusted)
        {
            /* mbedtls does not verify that each CA has a CRL, so we must do it ourselves.
             * We must fail here, otherwise we can't report misconfigurations to the users */
            status = SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(lRootsTrusted, lCrls, &bTrustedRootsCRL);
        }
        else
        {
            bTrustedRootsCRL = true;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != lRootsUntrusted)
        {
            status = SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(lRootsUntrusted, lCrls, &bUntrustedRootsCRL);
        }
        else
        {
            bUntrustedRootsCRL = true;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != lLinksTrusted)
        {
            status = SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(lLinksTrusted, lCrls, &bTrustedLinksCRL);
        }
        else
        {
            bTrustedLinksCRL = true;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != lLinksUntrusted)
        {
            status = SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(lLinksUntrusted, lCrls, &bUntrustedLinksCRL);
        }
        else
        {
            bUntrustedLinksCRL = true;
        }
    }
    if (SOPC_STATUS_OK == status &&
        (!bTrustedRootsCRL || !bUntrustedRootsCRL || !bTrustedLinksCRL || !bUntrustedLinksCRL))
    {
        if (!bTrustedRootsCRL)
        {
            fprintf(stderr,
                    "> PKI creation warning: Not all certificate authorities in given trusted roots have a single "
                    "certificate revocation list! Certificates issued by these CAs will be refused.\n");
        }
        if (!bUntrustedRootsCRL)
        {
            fprintf(stderr,
                    "> PKI creation warning: Not all certificate authorities in given untrusted roots have a single "
                    "certificate revocation list! Certificates issued by these CAs will be refused.\n");
        }
        if (!bTrustedLinksCRL)
        {
            fprintf(
                stderr,
                "> PKI creation warning: Not all certificate authorities in given trusted issuer links have a single "
                "certificate revocation list! Certificates issued by these CAs will be refused.\n");
        }
        if (!bUntrustedLinksCRL)
        {
            fprintf(
                stderr,
                "> PKI creation warning: Not all certificate authorities in given untrusted issuer links have a single "
                "certificate revocation list! Certificates issued by these CAs will be refused.\n");
        }
    }

    // Display warning in case no root issuer defined and trusted issued defined
    if (NULL == lRootsTrusted && NULL == lRootsUntrusted && NULL != lIssued)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_COMMON,
                                 "No root issuer (CA) defined in PKI: only trusted self-signed issued "
                                 "certificates will be accepted without possibility to revoke them (no issuer CRL).");
    }

    /* Link the untrusted lists with the trusted lists
     * (untrusted roots -> trusted roots, untrusted links -> trusted links) */
    if (SOPC_STATUS_OK == status)
    {
        status = link_certificates(&lRootsUntrusted, &lRootsTrusted);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = link_certificates(&lLinksUntrusted, &lLinksTrusted);
    }

    /* TODO: Check that all issued certificates are either self-signed or have a CA in the untrusted+trusted list.
     * This is checked when receiving an issued certificate,
     * but it would be useful to report this misconfiguration.
     */

    /* Simpler case: check and warn that there is untrusted issuers but no issued certificates */
    if (SOPC_STATUS_OK == status)
    {
        // Use paths for untrusted issuers since the certificate list will never be empty (chained to trusted)
        bool bUntrustedAndNoIssued = (NULL != *lPathUntrustedIssuerRoots) && (NULL == lIssued);
        if (bUntrustedAndNoIssued)
        {
            fprintf(stderr,
                    "> PKI creation warning: untrusted certificates are given but no issued certificates are given.\n");
        }
    }

    SOPC_PKIProvider* pki = NULL;
    if (SOPC_STATUS_OK == status)
    {
        pki = create_pkistack(lRootsTrusted, lLinksTrusted, lRootsUntrusted, lLinksUntrusted, lIssued, lCrls, false);
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
        /* Deleting the untrusted list will also clear the trusted list, as they are linked.
         * Hence mbedtls will call free on (&pPKI->pTrustedIssuerRootsList.crt), which is pPKI->pTrustedIssuerRootsList.
         */
        SOPC_KeyManager_Certificate_Free(lRootsUntrusted);
        SOPC_KeyManager_Certificate_Free(lLinksUntrusted);
        SOPC_KeyManager_Certificate_Free(lIssued);
        SOPC_KeyManager_CRL_Free(lCrls);
        SOPC_Free(pki);
    }

    return status;
}

/* ****************************************************************** */
/* ************************* NEW API ******************************** */
#include "sopc_assert.h"
#include "sopc_filesystem.h"
#include "sopc_helper_string.h"

/**
 * \brief The PKIProvider object for the Public Key Infrastructure.
 */
struct SOPC_PKIProviderNew
{
    const char* directoryStorePath;
    bool bBackwardInteroperability;
    SOPC_CertificateList* pTrustedCerts;
    SOPC_CertificateList* pTrustedRoots;
    SOPC_CRLList* pTrustedCrl;
    SOPC_CertificateList* pIssuerCerts;
    SOPC_CertificateList* pIssuerRoots;
    SOPC_CRLList* pIssuerCrl;

    SOPC_CertificateList* pAllCerts;
    SOPC_CertificateList* pAllRoots;
    SOPC_CRLList* pAllCrl;
};

static SOPC_ReturnStatus load_certificate_or_crl_list(const char* basePath,
                                                      SOPC_CertificateList** ppCerts,
                                                      SOPC_CRLList** ppCrl,
                                                      bool bIscrl)
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
        fprintf(stderr, "> PKI creation error: failed to open directory <%s>.\n", basePath);
        return SOPC_STATUS_NOK;
    }
    /* Get the size and iterate for each item */
    size_t nbFiles = SOPC_Array_Size(pFilePaths);
    for (size_t idx = 0; idx < nbFiles && SOPC_STATUS_OK == status; idx++)
    {
        pFilePath = SOPC_Array_Get(pFilePaths, char*, idx);
        fprintf(stderr, "> PKI loading file <%s>\n", pFilePath);
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
                                                                  SOPC_CRLList** ppIssuerCrl)
{
    SOPC_ASSERT(NULL != basePath && NULL != ppTrustedCerts && NULL != ppTrustedCrl && NULL != ppIssuerCerts &&
                NULL != ppIssuerCrl);
    /* Trusted Certs */
    char* trustedCertsPath = NULL;
    SOPC_ReturnStatus status = SOPC_StrConcat(basePath, "/trusted/certs", &trustedCertsPath);
    if (SOPC_STATUS_OK == status)
    {
        status = load_certificate_or_crl_list(trustedCertsPath, ppTrustedCerts, NULL, false);
    }
    SOPC_Free(trustedCertsPath);
    /* Trusted Crl */
    if (SOPC_STATUS_OK == status)
    {
        char* trustedCrlPath = NULL;
        status = SOPC_StrConcat(basePath, "/trusted/crl", &trustedCrlPath);
        if (SOPC_STATUS_OK == status)
        {
            status = load_certificate_or_crl_list(trustedCrlPath, NULL, ppTrustedCrl, true);
        }
        SOPC_Free(trustedCrlPath);
    }
    /* Issuer Certs */
    if (SOPC_STATUS_OK == status)
    {
        char* issuerCertsPath = NULL;
        status = SOPC_StrConcat(basePath, "/issuers/certs", &issuerCertsPath);
        if (SOPC_STATUS_OK == status)
        {
            status = load_certificate_or_crl_list(issuerCertsPath, ppIssuerCerts, NULL, false);
        }
        SOPC_Free(issuerCertsPath);
    }
    /* Issuer Crl */
    if (SOPC_STATUS_OK == status)
    {
        char* issuerCrlPath = NULL;
        status = SOPC_StrConcat(basePath, "/issuers/crl", &issuerCrlPath);
        if (SOPC_STATUS_OK == status)
        {
            status = load_certificate_or_crl_list(issuerCrlPath, NULL, ppIssuerCrl, true);
        }
        SOPC_Free(issuerCrlPath);
    }

    return status;
}

static bool cert_is_self_sign(mbedtls_x509_crt* crt)
{
    SOPC_ASSERT(NULL != crt);

    bool is_self_sign = false;
    /* Verify that the CA is self sign */
    int res = memcmp(crt->issuer_raw.p, crt->subject_raw.p, crt->issuer_raw.len);
    if (crt->issuer_raw.len == crt->subject_raw.len && 0 == res)
    {
        /* Is it correctly signed? Inspired by x509_crt_check_signature */
        const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(crt->sig_md);
        unsigned char hash[MBEDTLS_MD_MAX_SIZE];

        /* First hash the certificate, then verify it is signed */
        res = mbedtls_md(md_info, crt->tbs.p, crt->tbs.len, hash);
        if (0 == res)
        {
            res = mbedtls_pk_verify_ext(crt->sig_pk, crt->sig_opts, &crt->pk, crt->sig_md, hash,
                                        mbedtls_md_get_size(md_info), crt->sig.p, crt->sig.len);
            if (0 == res)
            {
                /* Finally accept the certificate */
                is_self_sign = true;
            }
        }
    }
    return is_self_sign;
}

/**
 * \brief Delete the roots of the list ppCerts. Create a new list ppRootCa with all roots from ppCerts.
 *        If there is no root, the contain of ppRootCa is set to NULL.
 *        If ppCerts become empty, its contains is set to NULL.
 */
static SOPC_ReturnStatus split_root_from_cert_list(SOPC_CertificateList** ppCerts, SOPC_CertificateList** ppRootCa)
{
    SOPC_ASSERT(NULL != ppCerts && NULL != ppRootCa);

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
        self_sign = cert_is_self_sign(cur);
        if (!self_sign && is_root)
        {
            is_root = false;
        }
        if (is_root)
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

                        /* Do not iterate: current certificate has changed */
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

static SOPC_ReturnStatus merge_certficates(SOPC_CertificateList* pLeft,
                                           SOPC_CertificateList* pRight,
                                           SOPC_CertificateList** ppRes)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL == pLeft && NULL == pRight)
    {
        /* Nothing to merge */
        return status;
    }
    SOPC_CertificateList* pRes = NULL;
    mbedtls_x509_crt* crt = NULL;
    if (NULL != pLeft)
    {
        crt = &pLeft->crt;
    }
    do
    {
        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(crt->raw.p, (uint32_t) crt->raw.len, &pRes);
        crt = crt->next;
        if (NULL == crt && NULL != pRight)
        {
            crt = &pRight->crt;
        }
    } while (NULL != crt && SOPC_STATUS_OK != status);

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
    if (NULL == pLeft && NULL == pRight)
    {
        /* Nothing to merge */
        return status;
    }
    SOPC_CRLList* pRes = NULL;
    mbedtls_x509_crl* crl = NULL;
    if (NULL != pLeft)
    {
        crl = &pLeft->crl;
    }
    do
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(crl->raw.p, (uint32_t) crl->raw.len, &pRes);
        crl = crl->next;
        if (NULL == crl && NULL != pRight)
        {
            crl = &pRight->crl;
        }
    } while (NULL != crl && SOPC_STATUS_OK != status);

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
            is_self_sign = cert_is_self_sign(crt);
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
                                     SOPC_CRLList* pIssuerCrl)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    uint32_t ca_count = 0;
    uint32_t list_length = 0;
    uint32_t issued_cert_count = 0;
    uint32_t root_count = 0;
    uint32_t trusted_root_count = 0;

    if (NULL == pTrustedCerts)
    {
        fprintf(stderr, "> PKI creation error: no trusted certificate is provided.\n");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    get_list_stats(pTrustedCerts, &ca_count, &list_length, &root_count);
    issued_cert_count = list_length - ca_count;
    trusted_root_count = root_count;

    if (0 != ca_count && NULL == pTrustedCrl)
    {
        fprintf(stderr, "> PKI creation error: trusted CA certificates are provided but no CRL.\n");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* check and warn in case there is no trusted certificates and no trusted root (only trusted intermediate CA). */
    if ((0 == issued_cert_count) && (0 == trusted_root_count))
    {
        /* In this case, no certificates will be accepted. */
        fprintf(stderr,
                "> PKI creation error: no trusted certificate and no trusted root is given: no certificates will be "
                "accepted.\n");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    root_count = 0;
    ca_count = 0;
    list_length = 0;
    get_list_stats(pIssuerCerts, &ca_count, &list_length, &root_count);

    if (0 != ca_count && NULL == pIssuerCrl)
    {
        fprintf(stderr, "> PKI creation error: issuer CA certificates are provided but no CRL.\n");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* Check if issuerCerts list is only filled with CA. */
    if (list_length != ca_count)
    {
        fprintf(stderr, "> PKI creation error: not all issuer certificates are CAs.\n");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    /* check and warn in case there is no trusted certificates. */
    if ((0 != ca_count) && (0 == issued_cert_count))
    {
        /* In this case, only trusted root CA will be accepted (if Backward interoperability is enabled). */
        fprintf(stderr,
                "> PKI creation warning: issuer certificates are given but no trusted certificates: only trusted root "
                "CA will be accepted (if backward interoperability is enabled)\n");
    }
    /* check and warn in case no root defined and trusted certificates defined. */
    if ((0 == root_count) && (0 == trusted_root_count) && (0 != issued_cert_count))
    {
        /* In this case, only trusted self-signed issued certificates will be accepted. */
        fprintf(stderr,
                "> PKI creation warning: no root (CA) defined: only trusted self-signed issued certificates will be "
                "accepted without possibility to revoke them (no CRL).\n");
    }

    return status;
}

/*
RBA TODO:
    - check that each CA keyUsage is filed with keyCertSign and keyCrlSign.
    - Add a configuration to raise a warning or to return an error if the chain of signatures is not rigth for each
certificate.
        --> The objectif is to fail during the PKI update (certificate manager) but not during a "nominal" operation.
    - Maybe use a copy of arguments instead of borrowing them.
*/
SOPC_ReturnStatus SOPC_PKIProviderNew_CreateFromList(SOPC_CertificateList* pTrustedCerts,
                                                     SOPC_CRLList* pTrustedCrl,
                                                     SOPC_CertificateList* pIssuerCerts,
                                                     SOPC_CRLList* pIssuerCrl,
                                                     bool bBackwardInteroperability,
                                                     SOPC_PKIProviderNew** ppPKI)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_PKIProviderNew* pPKI = NULL;
    SOPC_CertificateList* pTrustedRoots = NULL; /* trusted root CA */
    SOPC_CertificateList* pIssuerRoots = NULL;  /* issuer root CA */
    SOPC_CertificateList* pAllRoots = NULL;     /* issuer + trusted roots */
    SOPC_CertificateList* pAllCerts = NULL;     /* issuer + trusted certs */
    SOPC_CRLList* pAllCrl = NULL;               /* */

    if (NULL == ppPKI)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /*
       - Check that pTrustedCerts is not empty.
       - Check if there are CAs but no CRLs.
       - Check if issuerCerts list is only filled with CA.
       - Check and warn if issuerCerts is not empty but pTrustedCerts is only filed with CA.
         In this case, if there is no root into pTrustedCerts then
         no certififcates will be accepted during validation process.
       - Check and warn in case no root defined but trusted certificates defined.
         In this case, only trusted self-signed issued certificates will be accepted.
    */
    if (SOPC_STATUS_OK == status)
    {
        status = check_lists(pTrustedCerts, pIssuerCerts, pTrustedCrl, pIssuerCrl);
    }

    /* Check the CRL-CA association before creating the PKI. */
    bool bTrustedCRL = false;
    bool bIssuerCRL = false;
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != pTrustedCerts)
        {
            status = SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(pTrustedCerts, pTrustedCrl, &bTrustedCRL);
        }
        else
        {
            bTrustedCRL = true;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (NULL != pIssuerCerts)
        {
            status = SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL(pIssuerCerts, pIssuerCrl, &bIssuerCRL);
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
            fprintf(
                stderr,
                "> PKI creation warning: Not all certificate authorities in given trusted certificates have a single "
                "certificate revocation list! Certificates issued by these CAs will be refused.\n");
        }
        if (!bIssuerCRL)
        {
            fprintf(
                stderr,
                "> PKI creation warning: Not all certificate authorities in given issuer certififcates have a single "
                "certificate revocation list! Certificates issued by these CAs will be refused.\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = split_root_from_cert_list(&pTrustedCerts, &pTrustedRoots);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = split_root_from_cert_list(&pIssuerCerts, &pIssuerRoots);
    }

    if (SOPC_STATUS_OK == status)
    {
        pPKI = SOPC_Calloc(1, sizeof(SOPC_PKIProviderNew));
        if (NULL == pPKI)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = merge_certficates(pIssuerCerts, pTrustedCerts, &pAllCerts);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = merge_certficates(pIssuerRoots, pTrustedRoots, &pAllRoots);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = merge_crls(pIssuerCrl, pTrustedCrl, &pAllCrl);
    }

    if (SOPC_STATUS_OK == status)
    {
        pPKI->pTrustedRoots = pTrustedRoots;
        pPKI->pTrustedCerts = pTrustedCerts;
        pPKI->pTrustedCrl = pTrustedCrl;
        pPKI->pIssuerRoots = pIssuerRoots;
        pPKI->pIssuerCerts = pIssuerCerts;
        pPKI->pIssuerCrl = pIssuerCrl;
        pPKI->bBackwardInteroperability = bBackwardInteroperability;
        pPKI->pAllCerts = pAllCerts;
        pPKI->pAllRoots = pAllRoots;
        pPKI->pAllCrl = pAllCrl;
    }
    else
    {
        SOPC_KeyManager_Certificate_Free(pTrustedRoots);
        SOPC_KeyManager_Certificate_Free(pIssuerRoots);
        SOPC_KeyManager_Certificate_Free(pAllRoots);
        SOPC_KeyManager_Certificate_Free(pTrustedCerts);
        SOPC_KeyManager_Certificate_Free(pIssuerCerts);
        SOPC_KeyManager_Certificate_Free(pAllCerts);
        SOPC_KeyManager_CRL_Free(pTrustedCrl);
        SOPC_KeyManager_CRL_Free(pIssuerCrl);
        SOPC_KeyManager_CRL_Free(pAllCrl);

        SOPC_Free(pPKI);
        pPKI = NULL;
    }

    *ppPKI = pPKI;
    return status;
}

SOPC_ReturnStatus SOPC_PKIProviderNew_CreateFromStore(const char* directoryStorePath,
                                                      bool bDefaultBuild,
                                                      bool bBackwardInteroperability,
                                                      SOPC_PKIProviderNew** ppPKI)
{
    if (NULL == directoryStorePath || NULL == ppPKI)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_CertificateList* pTrustedCerts = NULL; /* trusted intermediate CA + trusted certificates */
    SOPC_CertificateList* pIssuerCerts = NULL;  /* issuer intermediate CA */
    SOPC_CRLList* pTrustedCrl = NULL;           /* CRLs of trusted intermediate CA and trusted root CA */
    SOPC_CRLList* pIssuerCrl = NULL;            /* CRLs of issuer intermediate CA and issuer root CA */
    const char* basePath = NULL;
    char* path = NULL;

    /* Select the rigth folder*/
    if (bDefaultBuild)
    {
        status = SOPC_StrConcat(directoryStorePath, "/default", &path);
        if (SOPC_STATUS_OK != status)
        {
            return status;
        }
        basePath = path;
    }
    else
    {
        status = SOPC_StrConcat(directoryStorePath, "/trustList", &path);
        if (SOPC_STATUS_OK != status)
        {
            return status;
        }
        basePath = path;
    }
    /* Load the files from the directory Store path */
    status =
        load_certificate_and_crl_list_from_store(basePath, &pTrustedCerts, &pTrustedCrl, &pIssuerCerts, &pIssuerCrl);
    /* Check if the trustList is empty */
    if (SOPC_STATUS_OK == status && NULL == pTrustedCerts && NULL == pTrustedCrl && NULL == pIssuerCerts &&
        NULL == pIssuerCrl)
    {
        status = SOPC_STATUS_NOK;
        fprintf(stderr, "> PKI creation error: certificate store is empty (%s).\n", path);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProviderNew_CreateFromList(pTrustedCerts, pTrustedCrl, pIssuerCerts, pIssuerCrl,
                                                    bBackwardInteroperability, ppPKI);
    }
    /* if error then try with default build */
    if (!bDefaultBuild && SOPC_STATUS_OK != status)
    {
        fprintf(stderr, "> PKI creation warning: trustList missing or bad build switch to default store.\n");
        status = SOPC_PKIProviderNew_CreateFromStore(directoryStorePath, true, bBackwardInteroperability, ppPKI);
    }

    /* Clear */
    SOPC_Free(path);

    return status;
}

void SOPC_PKIProviderNew_Free(SOPC_PKIProviderNew* pPKI)
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
    SOPC_Free(pPKI);
}
