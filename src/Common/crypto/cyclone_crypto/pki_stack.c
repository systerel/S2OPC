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

#include "cyclone_crypto/pkix/x509_cert_validate.h"
#include "cyclone_crypto/pkix/x509_crl_validate.h"

#define MAX_CERTIFICATE_DEPTH 10

enum
{
    ERROR_CERT_IN_CHAIN_WITH_BAD_SIGNATURE_SIGNALGO = 0,
    ERROR_CERT_IN_CHAIN_WITH_BAD_SIGNATURE_HASHALGO,
    ERROR_CERT_NOT_TRUSTED,
    ERROR_CERT_IN_CHAIN_WITH_INVALID_KEY_LENGTH,
    ERROR_INVALID_KEY_TYPE,
};

/* Definition of the profile cert structure */
typedef struct crt_profile
{
    uint8_t allowed_mds;
    size_t min_rsa_keySize;
} crt_profile;

/**
 * The minimal profile supported by the PKIProviderStack. It requires cacert signed with
 * at least SHA-256, with an RSA key of at least 2048 bits.
 */

static const crt_profile crt_profile_minimal = {
    .allowed_mds = (uint8_t) 3, // sha256 = 1, sha384 = 2, sha512 = 3
    .min_rsa_keySize = 2048,
};

static uint32_t PKIProviderStack_GetCertificateValidationError(uint32_t failure_reasons)
{
    if (0 == failure_reasons)
    {
        return 0;
    }

    return SOPC_CertificateValidationError_Unkown;
}

/* Returns 0 if all key usages and extended key usages are ok.
 * Only checks the first cert of the CertList (like MbedTLS).
 */
static SOPC_ReturnStatus check_key_usages(const SOPC_CertificateList* cert, bool isUserPki)
{
    int err = 0;
    unsigned int keyUsage = cert->crt.tbsCert.extensions.keyUsage.bitmap;       // cert KU
    unsigned int extKeyUsage = cert->crt.tbsCert.extensions.extKeyUsage.bitmap; // cert extKU
    unsigned int usages = 0;
    if (!isUserPki)
    {
        usages = X509_KEY_USAGE_DIGITAL_SIGNATURE | X509_KEY_USAGE_NON_REPUDIATION | X509_KEY_USAGE_KEY_ENCIPHERMENT |
                 X509_KEY_USAGE_DATA_ENCIPHERMENT;
        err = ((keyUsage & usages) == keyUsage ? 0 : 1);

        /* If the ext usage is neither server auth nor client auth, it shall be rejected */
        /* TODO: check whether the crt is for a server or a client, and only check the corresponding ext usage */
        bool missSer = (extKeyUsage == X509_EXT_KEY_USAGE_SERVER_AUTH);
        bool missCli = (extKeyUsage == X509_EXT_KEY_USAGE_CLIENT_AUTH);

        if (missSer && missCli)
        {
            err = 1;
        }
    }
    else
    {
        /* Check the key usages for user certificate (it is not part of the OPC UA but it makes sense to keep it). */
        usages = X509_KEY_USAGE_DIGITAL_SIGNATURE;
        err = ((keyUsage & usages) == keyUsage ? 0 : 1);
        if (!err)
        {
            /* The CA flag shall be FALSE for user certificate */
            err = cert->crt.tbsCert.extensions.basicConstraints.cA;
        }
    }

    if (!err)
    {
        return SOPC_STATUS_OK;
    }
    else
    {
        return SOPC_STATUS_NOK;
    }
}

/* Check in all the CRLs present in \param crl if child is revoked. */
static int crt_check_revocation(const SOPC_CertificateList* child, SOPC_CRLList* crl)
{
    SOPC_CRLList* cur_crl = NULL;
    int errLib = -1;

    for (cur_crl = crl; NULL != cur_crl; cur_crl = cur_crl->next)
    {
        errLib = x509CheckRevokedCertificate(&child->crt, &cur_crl->crl);
        if (errLib)
        {
            return errLib;
        }
    }

    return 0;
}

/* Find a parent in \param candidates chain */
static int crt_find_parent_in(const SOPC_CertificateList* child,
                              SOPC_CertificateList* candidates,
                              SOPC_CertificateList** ppParent)
{
    SOPC_CertificateList* parent = NULL;
    int errLib = -1;

    for (parent = candidates; NULL != parent; parent = parent->next)
    {
        /* This function :
         * - checks the validity of child ;
         * - checks if parent is the parent of child ;
         * - verifies the signature ;
         * Returns 0 if all is ok.
         */
        errLib = x509ValidateCertificate(&child->crt, &parent->crt, 0);
        if (errLib)
        {
            continue; // test next parent candidate
        }

        *ppParent = parent;

        break;
    }

    return 0;
}

/* Find a parent in \param candidate chain OR in \param child chain.
 * \param parent = NULL if no parent found.
 * Returns 0 on success.
 */
static int crt_find_parent(const SOPC_CertificateList* child,
                           SOPC_CertificateList* candidates,
                           SOPC_CertificateList** parent,
                           int* parent_is_trusted)
{
    SOPC_CertificateList* search_list = NULL;
    *parent_is_trusted = 1;
    int errLib = -1;

    while (1)
    {
        search_list = *parent_is_trusted ? candidates : child->next;
        errLib = crt_find_parent_in(child, search_list, parent);

        /* stop here if found or already in second iteration */
        if (!errLib && (NULL != *parent || 0 == *parent_is_trusted))
        {
            break;
        }

        /* prepare second iteration */
        *parent_is_trusted = 0;
    }

    return 0;
}

/* Verifies the chain of \param pToValidate.
 * The result is stored in \param flag. Flag = 0 if the cert is accepted.
 * Returns 0 on success.
 */
static int crt_verify_chain(const SOPC_CertificateList* pToValidate,
                            SOPC_CertificateList* trust_list,
                            SOPC_CRLList* cert_crl,
                            const crt_profile* pCrt_profile,
                            uint32_t* flag,
                            int* certificate_depth)
{
    const SOPC_CertificateList* child = pToValidate;
    SOPC_CertificateList* parent = NULL;
    int parent_is_trusted = 0;
    int child_is_trusted = 0;
    int errLib = -1;

    while (1)
    {
        if (child_is_trusted)
        {
            *flag = 0;
            return 0;
        }

        // a) Check if md and pk algos of the signature of the cert suits to the profile.
        X509SignatureAlgo signAlgo = {0};
        const HashAlgo* hashAlgo = NULL;
        errLib = x509GetSignHashAlgo(&pToValidate->crt.signatureAlgo, &signAlgo, &hashAlgo);
        if (errLib || X509_SIGN_ALGO_NONE == signAlgo)
        {
            *flag = ERROR_CERT_IN_CHAIN_WITH_BAD_SIGNATURE_SIGNALGO;
            return 0;
        }

        // hashAlgo should be at least SHA-256.
        uint8_t hashAlgoOid = hashAlgo->oid[8];
        if (0 == (hashAlgoOid & pCrt_profile->allowed_mds)) // The hash algo is not an allowed md.
        {
            *flag = ERROR_CERT_IN_CHAIN_WITH_BAD_SIGNATURE_HASHALGO;
            return 0;
        }

        // b) Find a parent in trusted CA or in the child chain.
        // This function will also verify the signature if a parent is found,
        // and check time-validity of child.
        errLib = crt_find_parent(child, trust_list, &parent, &parent_is_trusted);
        if (parent == NULL)
        {
            *flag = ERROR_CERT_NOT_TRUSTED;
            return 0;
        }

        // c) Check if the parent key suits to the profile.
        size_t keySize = parent->crt.tbsCert.subjectPublicKeyInfo.rsaPublicKey.nLen * 8;
        if (keySize < pCrt_profile->min_rsa_keySize)
        {
            *flag = ERROR_CERT_IN_CHAIN_WITH_INVALID_KEY_LENGTH;
            return 0;
        }

        // d) Check if the cert is not revoked, in any valid CRL.
        // TODO : proceed on some checks on the CRL (if it the signature suits the profile and if it is signed by a
        // trusted CA). Maybe call SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL() before the call of
        // PKI_ValidateCertificate() ?
        errLib = crt_check_revocation(child, cert_crl);
        if (errLib)
        {
            *flag = (uint32_t) errLib;
            return 0;
        }

        // e) Iterate.
        child = parent;
        parent = NULL;
        child_is_trusted = parent_is_trusted;
        *certificate_depth += 1;
    }

    return -1; // error
}

static int crt_validate_with_profile(const SOPC_CertificateList* pToValidate,
                                     SOPC_CertificateList* trust_list,
                                     SOPC_CRLList* cert_crl,
                                     const crt_profile* pCrt_profile,
                                     uint32_t* flag,
                                     bool* bIssued)
{
    /* 1) Check if the public key type and size of the cert suits to the profile */
    X509KeyType keyType = x509GetPublicKeyType(pToValidate->crt.tbsCert.subjectPublicKeyInfo.oid,
                                               pToValidate->crt.tbsCert.subjectPublicKeyInfo.oidLen);
    if (X509_KEY_TYPE_UNKNOWN == keyType)
    {
        *flag = ERROR_INVALID_KEY_TYPE;
        return 0;
    }

    size_t keySize = pToValidate->crt.tbsCert.subjectPublicKeyInfo.rsaPublicKey.nLen * 8;
    if (keySize <
        pCrt_profile->min_rsa_keySize) // Minimal profile requires cert signed with an RSA key of at least 2048 bits.
    {
        *flag = ERROR_INVALID_KEY_LENGTH;
        return 0;
    }

    /* 2) Verify the chain */
    int certificate_depth = 0;
    int errLib = crt_verify_chain(pToValidate, trust_list, cert_crl, pCrt_profile, flag, &certificate_depth);

    /* 3) Treat the cert if it is self-signed */
    if (*bIssued && 0 == certificate_depth && ERROR_CERT_NOT_TRUSTED == *flag)
    {
        errLib = x509ValidateCertificate(&pToValidate->crt, &pToValidate->crt, 0);
        if (!errLib)
        {
            *flag = 0;
        }
    }

    return errLib;
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

    /* Check certificate usages */
    bool isUserPki = (bool) pPKI->pUserData;
    status = check_key_usages(pToValidate, isUserPki);
    if (SOPC_STATUS_OK != status)
    {
        *error = SOPC_CertificateValidationError_UseNotAllowed;
    }

    /* Link certificate to validate with intermediate certificates (trusted links or untrusted links) */
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    SOPC_CertificateList* pToValidateCopy = (SOPC_CertificateList*) pToValidate;
    pToValidateCopy->next = bIssued ? pPKI->pUntrustedIssuerLinksList : pPKI->pTrustedIssuerLinksList;
    SOPC_GCC_DIAGNOSTIC_RESTORE

    /* Verify the certificate chain */
    int errLib = -1;
    if (SOPC_STATUS_OK == status)
    {
        uint32_t failure_reasons = 0;
        errLib = crt_validate_with_profile(pToValidateCopy, trust_list, cert_crl, &crt_profile_minimal,
                                           &failure_reasons, &bIssued);

        if (!errLib)
        {
            *error = PKIProviderStack_GetCertificateValidationError(failure_reasons);
        }
    }

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

static SOPC_ReturnStatus link_certificates(SOPC_CertificateList** ppPrev, SOPC_CertificateList** ppNext)
{
    SOPC_ASSERT(NULL != ppPrev && NULL != ppNext);

    SOPC_CertificateList* prev = *ppPrev;
    SOPC_CertificateList* next = *ppNext;

    /* Link two existing lists */
    if (NULL != prev && NULL != next)
    {
        SOPC_CertificateList* crt = prev;
        /* crt should not be NULL, as either untrusted is NULL or at least one cert was created */
        SOPC_ASSERT(NULL != crt);

        while (NULL != crt->next)
        {
            crt = crt->next;
        }
        /* crt is now the last certificate of the chain, link it with trusted */
        crt->next = next;
    }

    return SOPC_STATUS_OK;
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

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Load the CertificateLists
    SOPC_CertificateList* lRootsTrusted = load_certificate_list(lPathTrustedIssuerRoots, &status);
    SOPC_CertificateList* lLinksTrusted = load_certificate_list(lPathTrustedIssuerLinks, &status);
    SOPC_CertificateList* lRootsUntrusted = load_certificate_list(lPathUntrustedIssuerRoots, &status);
    SOPC_CertificateList* lLinksUntrusted = load_certificate_list(lPathUntrustedIssuerLinks, &status);
    SOPC_CertificateList* lIssued = load_certificate_list(lPathIssuedCerts, &status);

    // Load the CRLs
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
            /* CycloneCrypto does not verify that each CA has a CRL, so we must do it ourselves.
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

    /* Cyclone : maybe link the untrusted lists with the trusted lists ?
     * (untrusted roots -> trusted roots, untrusted links -> trusted links).
     * See if needed for later.
     */

    if (SOPC_STATUS_OK == status)
    {
        status = link_certificates(&lRootsUntrusted, &lRootsTrusted);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = link_certificates(&lLinksUntrusted, &lLinksTrusted);
    }

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
        // Reminder Cyclone : certificates lists are linked before this call.
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
         * -> Cyclone ?
         */
        SOPC_KeyManager_Certificate_Free(lRootsUntrusted);
        SOPC_KeyManager_Certificate_Free(lLinksUntrusted);
        SOPC_KeyManager_Certificate_Free(lIssued);
        SOPC_KeyManager_CRL_Free(lCrls);
        SOPC_Free(pki);
    }

    return status;
}
