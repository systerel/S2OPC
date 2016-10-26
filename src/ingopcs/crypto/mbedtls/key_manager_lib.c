/*
 * KeyManager provides functions for Asymmetric Key Management such as loading a signed public key,
 *  the corresponding private key, and provides the ability to verify signatures with x509 certificates.
 * KeyManager replaces the old concept of PKIProvider. PrivateKey should not be in the PublicKeyInfrastructure...
 *
 * Most of the functions are lib-dependent, and defined here.
 *
 *  Created on: Oct. 20 2016
 *      Author: PAB
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ua_base_types.h"
#include "crypto_types.h"
#include "crypto_profiles.h"
#include "crypto_provider.h"
#include "key_manager.h"

#include "mbedtls/pk.h"
#include "mbedtls/x509.h"


/* ------------------------------------------------------------------------------------------------
 * AsymmetricKey API
 * ------------------------------------------------------------------------------------------------
 */

/**
 * \brief   Computes the maximal size of a buffer to be encrypted in a single pass.
 *          RFC 3447 provides the formula used with OAEPadding.
 *          A message shorter than or as long as this size is treated as a single message. A longer message
 *          is cut into pieces of this size before treatment.
 */
StatusCode KeyManager_AsymmetricKeyGetLength_MsgPlainText(const CryptoProvider *pProvider,
                                                          const AsymmetricKey *pKey,
                                                          uint32_t *lenMsg)
{
    uint32_t lenHash = 0;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pKey || NULL == lenMsg)
        return STATUS_INVALID_PARAMETERS;
    if(SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID)
        return STATUS_INVALID_PARAMETERS;

    *lenMsg = mbedtls_pk_get_len(&pKey->pk);
    if(lenMsg == 0)
        return STATUS_NOK;

    switch(pProvider->pProfile->SecurityPolicyID) // TODO: should we build some API to fetch the SecurityPolicyID, or avoid to switch on it at all?
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_NOK;
    case SecurityPolicy_Basic256Sha256_ID: // TODO: this seems overkill to fetch the size of the chosen OAEP hash function...
        if(CryptoProvider_AsymmetricGetLength_OAEPHashLength(pProvider, &lenHash) != STATUS_OK)
            return STATUS_NOK;
        *lenMsg -= 2*lenHash + 2; // TODO: check for underflow?
        break;
    }

    return STATUS_OK;
}


/**
 * \brief   Computes the size of an encrypted buffer unit.
 *          This is the length of the public key modulus.
 */
StatusCode KeyManager_AsymmetricKeyGetLength_MsgCipherText(const CryptoProvider *pProvider,
                                                           const AsymmetricKey *pKey,
                                                           uint32_t *lenMsg)
{
    (void)(pProvider);

    if(NULL == pKey || NULL == lenMsg)
        return STATUS_INVALID_PARAMETERS;

    *lenMsg = mbedtls_pk_get_len(&pKey->pk);
    if(lenMsg == 0)
        return STATUS_NOK;

    return STATUS_OK;
}


/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */
StatusCode KeyManager_Certificate_CreateFromDER(const KeyManager *pManager,
                                                const uint8_t *bufferDER, uint32_t lenDER,
                                                Certificate **ppCert)
{
    mbedtls_x509_crt *crt = NULL;
    Certificate *certif = NULL;

    if(NULL == pManager || NULL == bufferDER || 0 == lenDER || NULL == ppCert)
        return STATUS_INVALID_PARAMETERS;

    // Mem alloc
    certif = malloc(sizeof(Certificate));
    if(NULL == certif)
        return STATUS_NOK;

    // Init
    crt = &certif->crt;
    mbedtls_x509_crt_init(crt);
    certif->crt_der = NULL;
    certif->len_der = 0;

    // Parsing
    if(mbedtls_x509_crt_parse(crt, bufferDER, lenDER) == 0)
    {
        certif->crt_der = certif->crt.raw.p;
        certif->len_der = certif->crt.raw.len;
        *ppCert = certif;
        return STATUS_OK;
    }
    else
    {
        KeyManager_Certificate_Free(certif);
        return STATUS_NOK;
    }
}


/** \Note   Tested but not part of the test suites.
 * \Note    Same as CreateFromDER, except for a single call, can we refactor?
 *
 */
StatusCode KeyManager_Certificate_CreateFromFile(const KeyManager *pManager,
                                                 const int8_t *szPath,
                                                 Certificate **ppCert)
{
    mbedtls_x509_crt *crt = NULL;
    Certificate *certif = NULL;

    if(NULL == pManager || NULL == szPath || NULL == ppCert)
        return STATUS_INVALID_PARAMETERS;

    // Mem alloc
    certif = malloc(sizeof(Certificate));
    if(NULL == certif)
        return STATUS_NOK;

    // Init
    crt = &certif->crt;
    mbedtls_x509_crt_init(crt);
    certif->crt_der = NULL;
    certif->len_der = 0;

    // Parsing
    if(mbedtls_x509_crt_parse_file(crt, (const char *)szPath) == 0)
    {
        certif->crt_der = certif->crt.raw.p;
        certif->len_der = certif->crt.raw.len;
        *ppCert = certif;
        return STATUS_OK;
    }
    else
    {
        KeyManager_Certificate_Free(certif);
        return STATUS_NOK;
    }
}


void KeyManager_Certificate_Free(Certificate *cert)
{
    if(NULL == cert)
        return;

    mbedtls_x509_crt_free(&cert->crt);
    cert->crt_der = NULL;
    cert->len_der = 0;
}


StatusCode KeyManager_Certificate_GetThumbprint(const KeyManager *pManager,
                                                const Certificate *pCert,
                                                uint8_t *pDest, uint32_t lenDest)
{
    StatusCode status = STATUS_OK;
    uint32_t lenSupposed = 0;
    uint8_t *pDER = NULL;
    uint32_t lenDER = 0;
    mbedtls_md_type_t type = MBEDTLS_MD_NONE;

    if(NULL == pManager || NULL == pManager->pProvider || NULL == pManager->pProvider->pProfile || NULL == pCert || NULL == pDest || 0 == lenDest)
        return STATUS_INVALID_PARAMETERS;

    if(KeyManager_CertificateGetLength_Thumbprint(pManager, &lenSupposed) != STATUS_OK)
        return STATUS_NOK;

    if(lenDest != lenSupposed)
        return STATUS_INVALID_PARAMETERS;

    // Get DER
    if(KeyManager_Certificate_CopyDER(pManager, pCert, &pDER, &lenDER) != STATUS_OK)
        return STATUS_NOK;

    // Hash DER with SHA-1
    switch(pManager->pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        status = STATUS_NOK;
        break;
    case SecurityPolicy_Basic256Sha256_ID:
        type = MBEDTLS_MD_SHA1;
        break;
    }

    if(STATUS_OK == status)
    {
        const mbedtls_md_info_t *pmd = mbedtls_md_info_from_type(type);
        if(mbedtls_md(pmd, pDER, lenDER, pDest) != 0)
            status = STATUS_NOK;
    }

    free(pDER);

    return status;
}


/*StatusCode KeyManager_Certificate_GetPublicKey(const KeyManager *pManager,
                                               const Certificate *pCert,
                                               AsymetricKey *pKey)
{
    return STATUS_OK;
}*/
