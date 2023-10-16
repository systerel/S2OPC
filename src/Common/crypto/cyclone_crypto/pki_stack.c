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
#include "sopc_platform_time.h"
#include "sopc_time.h"

#include "key_manager_lib.h"

#include "pkix/x509_cert_validate.h"
#include "pkix/x509_crl_validate.h"
#include "pkix/x509_sign_verify.h"

#define MAX_CERTIFICATE_DEPTH 10

// Bitmask errors for OPCUA error order compliance
#define X509_BADCERT_EXPIRED 0x01         /**< The certificate validity has expired. */
#define X509_BADCERT_REVOKED 0x02         /**< The certificate has been revoked (is on a CRL). */
#define X509_BADCERT_CN_MISMATCH 0x04     /**< The certificate Common Name (CN) does not match with the expected CN. */
#define X509_BADCERT_NOT_TRUSTED 0x08     /**< The certificate is not correctly signed by the trusted CA. */
#define X509_BADCRL_NOT_TRUSTED 0x10      /**< The CRL is not correctly signed by the trusted CA. */
#define X509_BADCRL_EXPIRED 0x20          /**< The CRL is expired. */
#define X509_BADCERT_MISSING 0x40         /**< Certificate was missing. */
#define X509_BADCERT_SKIP_VERIFY 0x80     /**< Certificate verification was skipped. */
#define X509_BADCERT_OTHER 0x0100         /**< Other reason (can be used by verify callback) */
#define X509_BADCERT_FUTURE 0x0200        /**< The certificate validity starts in the future. */
#define X509_BADCRL_FUTURE 0x0400         /**< The CRL is from the future */
#define X509_BADCERT_KEY_USAGE 0x0800     /**< Usage does not match the keyUsage extension. */
#define X509_BADCERT_EXT_KEY_USAGE 0x1000 /**< Usage does not match the extendedKeyUsage extension. */
#define X509_BADCERT_NS_CERT_TYPE 0x2000  /**< Usage does not match the nsCertType extension. */
#define X509_BADCERT_BAD_MD 0x4000        /**< The certificate is signed with an unacceptable hash. */
#define X509_BADCERT_BAD_PK 0x8000 /**< The certificate is signed with an unacceptable PK alg (eg RSA vs ECDSA). */
#define X509_BADCERT_BAD_KEY \
    0x010000 /**< The certificate is signed with an unacceptable key (eg bad curve, RSA too short). */
#define X509_BADCRL_BAD_MD 0x020000  /**< The CRL is signed with an unacceptable hash. */
#define X509_BADCRL_BAD_PK 0x040000  /**< The CRL is signed with an unacceptable PK alg (eg RSA vs ECDSA). */
#define X509_BADCRL_BAD_KEY 0x080000 /**< The CRL is signed with an unacceptable key (eg bad curve, RSA too short). */

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

static uint32_t PKIProviderStack_GetCertificateValidationError(uint32_t failure_reasons)
{
    // Order compliant with part 4 (1.04) Table 106

    /* Certificate structure */
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

    /* Signature */
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

    /* Trust list check*/

    /* Generic signature error */
    if ((failure_reasons & X509_BADCERT_NOT_TRUSTED) != 0)
    {
        return SOPC_CertificateValidationError_Untrusted;
    }

    /* Validity period */
    else if ((failure_reasons & X509_BADCERT_EXPIRED) != 0)
    {
        return SOPC_CertificateValidationError_TimeInvalid;
    }
    else if ((failure_reasons & X509_BADCERT_FUTURE) != 0)
    {
        return SOPC_CertificateValidationError_TimeInvalid;
    }

    /* Host Name */
    else if ((failure_reasons & X509_BADCERT_CN_MISMATCH) != 0)
    {
        return SOPC_CertificateValidationError_HostNameInvalid;
    }
    /* URI */

    /* Certificate Usage */
    // Checked in PKIProviderStack_ValidateCertificate

    /* (Find) Revocation List */
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

    /* Revocation check */
    else if ((failure_reasons & X509_BADCERT_REVOKED) != 0)
    {
        return SOPC_CertificateValidationError_Revoked;
    }
    else if ((failure_reasons & X509_BADCERT_OTHER) != 0)
    {
        return SOPC_CertificateValidationError_Untrusted;
    }

    return SOPC_CertificateValidationError_Unkown;
}

/* Self-signed validation function.
 * Basically do the same as x509ValidateCertificate() except
 * verifying that the issuer is CA.
 * Returns 0 if \param is valid.
 */
static int ValidateSelfSignedCertificate(const X509CertificateInfo* crt)
{
    int err = 1;
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

    // Return status code
    if (!errLib)
    {
        err = 0;
    }

    return err;
}

/* Returns SOPC_STATUS_OK if all key usages and extended key usages are ok.
 * Only checks the first certificate of the CertList (like MbedTLS).
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
        err = ((keyUsage & usages) == usages ? 0 : 1);

        /* If the ext usage is neither server auth nor client auth, it shall be rejected */
        /* TODO: check whether the crt is for a server or a client, and only check the corresponding ext usage */
        bool missSer = ((extKeyUsage & X509_EXT_KEY_USAGE_SERVER_AUTH) == 0);
        bool missCli = ((extKeyUsage & X509_EXT_KEY_USAGE_CLIENT_AUTH) == 0);

        if (missSer && missCli)
        {
            err = 1;
        }
    }
    else
    {
        /* Check the key usages for user certificate (it is not part of the OPC UA but it makes sense to keep it). */
        usages = X509_KEY_USAGE_DIGITAL_SIGNATURE;
        err = !(keyUsage & usages);
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

/* Check in all the CRLs present in \param crl if \param child is revoked.
 * Returns X509_BADCERT_REVOKED if \param child is revoked, 0 otherwise.
 * TODO : Include the other returning error codes.
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
            return X509_BADCERT_REVOKED;
        }
    }

    return 0;
}

/* Find a parent in the chain \param candidates.
 * Always returns 0. TODO : Fix it.
 */
static int crt_find_parent_in(const SOPC_CertificateList* child,
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

    return 0;
}

/* Find a parent in the chain \param candidates OR in the chain \param child.
 * \param parent will contain a NULL pointer if no parent is found.
 * Always returns 0. TODO : Fix it.
 */
static error_t crt_find_parent(const SOPC_CertificateList* child,
                               SOPC_CertificateList* candidates,
                               uint32_t* failure_reasons,
                               SOPC_CertificateList** parent,
                               int* parent_is_trusted)
{
    SOPC_CertificateList* search_list = NULL;
    *parent_is_trusted = 1;
    int err = 1;

    while (1)
    {
        search_list = *parent_is_trusted ? candidates : child->next;
        err = crt_find_parent_in(child, search_list, failure_reasons, parent);

        /* stop here if found or already in second iteration */
        if (!err && (NULL != *parent || 0 == *parent_is_trusted))
        {
            break;
        }

        /* prepare second iteration */
        *parent_is_trusted = 0;
    }

    return 0;
}

/* Verifies the chain of \param pToValidate.
 * Always returns 0.
 * The result is stored in \param failure_reasons.
 * TODO : proceed in a different way because always returning 0
 * is non-sense.
 */
static int crt_verify_chain(const SOPC_CertificateList* pToValidate,
                            SOPC_CertificateList* trust_list,
                            SOPC_CRLList* cert_crl,
                            const crt_profile* pCrt_profile,
                            uint32_t* failure_reasons,
                            int* certificate_depth)
{
    const SOPC_CertificateList* child = pToValidate;
    SOPC_CertificateList* parent = NULL;
    SOPC_CertificateList* cur_trust_ca = NULL;
    int parent_is_trusted = 0;
    int child_is_trusted = 0;
    error_t errLib = 1;

    while (1)
    {
        if (child_is_trusted)
        {
            return 0;
        }

        // a) Check if md and pk algos of the signature of the cert suits to the profile.
        X509SignatureAlgo signAlgo = {0};
        const HashAlgo* hashAlgo = NULL;
        // Returns ERROR_UNSUPPORTED_SIGNATURE_ALGO on errors
        errLib = x509GetSignHashAlgo(&pToValidate->crt.signatureAlgo, &signAlgo, &hashAlgo);
        if (ERROR_UNSUPPORTED_SIGNATURE_ALGO == errLib)
        {
            *failure_reasons |= X509_BADCERT_BAD_PK;
        }

        // hashAlgo should be at least SHA-256.
        uint8_t hashAlgoOid = hashAlgo->oid[8];
        if (0 == (hashAlgoOid & pCrt_profile->allowed_mds)) // If the hash algo is not an allowed md.
        {
            *failure_reasons |= X509_BADCERT_BAD_MD;
        }

        // b) Find a parent in trusted CA first or in the child chain.
        // This function will also verify the signature if a parent is found,
        // and check time-validity of child.
        cur_trust_ca = trust_list;
        errLib = crt_find_parent(child, cur_trust_ca, failure_reasons, &parent, &parent_is_trusted);
        if (NULL == parent)
        {
            *failure_reasons |= X509_BADCERT_NOT_TRUSTED;
            return 0;
        }

        // c) Check if the parent's key suits to the profile.
        size_t keySize = parent->crt.tbsCert.subjectPublicKeyInfo.rsaPublicKey.nLen * 8;
        if (keySize < pCrt_profile->min_rsa_keySize)
        {
            *failure_reasons |= X509_BADCERT_BAD_KEY;
            return 0;
        }

        // d) Check if the cert is not revoked, in any valid CRL.
        // TODO : proceed on some checks on the CRL (if it the signature suits the profile and if it is signed by a
        // trusted CA). Maybe call SOPC_KeyManager_CertificateList_RemoveUnmatchedCRL() before the call of
        // PKI_ValidateCertificate() ?
        *failure_reasons |= (uint32_t) crt_check_revocation(child, cert_crl);

        // e) Iterate.
        child = parent;
        parent = NULL;
        child_is_trusted = parent_is_trusted;
        *certificate_depth += 1;
    }

    return -1;
}

/* Returns 0 if the certificate is validated.
 * Otherwise, returns -1.
 */
static int crt_validate_with_profile(const SOPC_CertificateList* pToValidate,
                                     SOPC_CertificateList* trust_list,
                                     SOPC_CRLList* cert_crl,
                                     const crt_profile* pCrt_profile,
                                     uint32_t* failure_reasons,
                                     bool* bIssued)
{
    /* 1) Check if the public key type and size of the cert suits to the profile */
    // Type. Returns X509KeyType X509_KEY_TYPE_UNKNOWN on errors
    X509KeyType keyType = x509GetPublicKeyType(pToValidate->crt.tbsCert.subjectPublicKeyInfo.oid,
                                               pToValidate->crt.tbsCert.subjectPublicKeyInfo.oidLen);
    if (X509_KEY_TYPE_UNKNOWN == keyType)
    {
        *failure_reasons |= X509_BADCERT_BAD_PK;
    }

    // Size.
    size_t keySize = pToValidate->crt.tbsCert.subjectPublicKeyInfo.rsaPublicKey.nLen * 8;
    if (keySize < pCrt_profile->min_rsa_keySize)
    {
        *failure_reasons |= X509_BADCERT_BAD_KEY;
    }

    /* 2) Verify the chain */
    int certificate_depth = 0;
    int err = crt_verify_chain(pToValidate, trust_list, cert_crl, pCrt_profile, failure_reasons, &certificate_depth);

    /* 3) Treat the cert if it is self-signed */
    if (*bIssued && 0 == certificate_depth && 0 != (*failure_reasons & X509_BADCERT_NOT_TRUSTED))
    {
        err = ValidateSelfSignedCertificate(&pToValidate->crt);
        if (!err)
        {
            *failure_reasons ^= X509_BADCERT_NOT_TRUSTED;
        }
    }

    if (0 != *failure_reasons) // If there's at least one failure reason, return error
    {
        return -1;
    }

    return err;
}

/* Returns :
 * - SOPC_STATUS_NOK if the certificate has not been validated.
 *   In this case, stores the failure reasons in \param error.
 * - SOPC_STATUS_OK if the certificate has been validated.
 *   In this last case, we can ignore what's in \param error.
 */
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

    // Check if pToValidate is issued (for our PKI) or not.
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

    /* Check pToValidate key usages */
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
    int err = -1;
    if (SOPC_STATUS_OK == status)
    {
        uint32_t failure_reasons = 0;
        err = crt_validate_with_profile(pToValidateCopy, trust_list, cert_crl, &crt_profile_minimal, &failure_reasons,
                                        &bIssued);

        if (0 != err) // The certificate is not validated, get the error reasons
        {
            *error = PKIProviderStack_GetCertificateValidationError(failure_reasons);
            status = SOPC_STATUS_NOK;
        }
    }

    /* Unlink pToValidateCopy */
    pToValidateCopy->next = NULL;

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

    /* The second list exists, but not the first */
    else if (NULL != next)
    {
        *ppPrev = next;
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
