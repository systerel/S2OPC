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
    // Order compliant with part 4 (1.03) Table 104

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
    // Cf. generic error below: validity period hidden if generic error used here

    /* Validity period */
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_EXPIRED) != 0)
    {
        return SOPC_CertificateValidationError_TimeInvalid;
    }
    else if ((failure_reasons & MBEDTLS_X509_BADCERT_FUTURE) != 0)
    {
        return SOPC_CertificateValidationError_TimeInvalid;
    }

    /* Generic signature error (may include validity period) */
    if ((failure_reasons & MBEDTLS_X509_BADCERT_NOT_TRUSTED) != 0)
    {
        return SOPC_CertificateValidationError_Untrusted;
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
    if (bIssued && 0 == certificate_depth && MBEDTLS_X509_BADCERT_NOT_TRUSTED == *flags)
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
                    /* Finally accept the certificate */
                    *flags = 0;
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
static SOPC_ReturnStatus check_key_usages(const mbedtls_x509_crt* crt)
{
    unsigned int usages = MBEDTLS_X509_KU_DIGITAL_SIGNATURE | MBEDTLS_X509_KU_NON_REPUDIATION |
                          MBEDTLS_X509_KU_KEY_ENCIPHERMENT | MBEDTLS_X509_KU_DATA_ENCIPHERMENT;
    int err = mbedtls_x509_crt_check_key_usage(crt, usages);

    if (0 == err)
    {
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
    status = check_key_usages(mbed_chall);
    if (SOPC_STATUS_OK != status)
    {
        *error = SOPC_CertificateValidationError_UseNotAllowed;
    }

    /* Link certificate to validate with intermediate certificates (trusted links or untrusted links) */
    mbedtls_x509_crt* end_of_chall = mbed_chall;
    assert(NULL != end_of_chall);
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
                                         void* pUserData)
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
        pki = create_pkistack(caCert, NULL, NULL, NULL, NULL, pRevocationList, NULL);
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
    assert(NULL != paths && NULL != status);

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
    assert(NULL != ppPrev && NULL != ppNext);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_CertificateList* prev = *ppPrev;
    SOPC_CertificateList* next = *ppNext;
    /* Link two existing lists */
    if (NULL != prev && NULL != next)
    {
        mbedtls_x509_crt* crt = &prev->crt;
        /* crt should not be NULL, as either untrusted is NULL or at least one cert was created */
        assert(NULL != crt);
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
        pki = create_pkistack(lRootsTrusted, lLinksTrusted, lRootsUntrusted, lLinksUntrusted, lIssued, lCrls, NULL);
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
