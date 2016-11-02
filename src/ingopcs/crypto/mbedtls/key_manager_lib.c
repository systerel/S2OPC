/**
 * \file
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
 * Creates an asymmetric key from a \p buffer, in DER or PEM format.
 */
StatusCode KeyManager_AsymmetricKey_CreateFromBuffer(const KeyManager *pManager,
                                                     const uint8_t *buffer, uint32_t lenBuf,
                                                     AsymmetricKey **ppKey)
{
    AsymmetricKey *key = NULL;

    if(NULL == pManager || NULL == buffer || 0 == lenBuf || NULL == ppKey)
        return STATUS_INVALID_PARAMETERS;

    key = malloc(sizeof(AsymmetricKey));
    if(NULL == key)
        return STATUS_NOK;
    mbedtls_pk_init(&key->pk);

    if(mbedtls_pk_parse_key(&key->pk, buffer, lenBuf, NULL, 0) != 0) // This should also parse PEM keys.
    {
        free(key);
        return STATUS_NOK;
    }

    *ppKey = key;

    return STATUS_OK;
}


StatusCode KeyManager_AsymmetricKey_CreateFromFile(const KeyManager *pManager,
                                                   const char *szPath,
                                                   AsymmetricKey **ppKey)
{
    AsymmetricKey *key = NULL;

    if(NULL == pManager || NULL == szPath || NULL == ppKey)
        return STATUS_INVALID_PARAMETERS;

    key = malloc(sizeof(AsymmetricKey));
    if(NULL == key)
        return STATUS_NOK;
    mbedtls_pk_init(&key->pk);

    if(mbedtls_pk_parse_keyfile(&key->pk, szPath, NULL) != 0)
    {
        free(key);
        return STATUS_NOK;
    }

    *ppKey = key;

    return STATUS_OK;
}


/**
 * Frees an asymmetric key created with KeyManager_AsymmetricKey_Create*().
 *
 * \warning     Only keys created with KeyManager_AsymmetricKey_Create*() should be freed that way.
 */
void KeyManager_AsymmetricKey_Free(AsymmetricKey *pKey)
{
    if(NULL == pKey)
        return;

    mbedtls_pk_free(&pKey->pk);
    free(pKey);
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


/**
 * \note    Tested but not part of the test suites.
 * \note    Same as CreateFromDER, except for a single call, can we refactor?
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
    free(cert);
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


/**
 * \brief       Fills \p pKey with public key information retrieved from \p pCert.
 * \warning     \p pKey is not valid anymore when \p pCert is freed.
 */
StatusCode KeyManager_Certificate_GetPublicKey(const KeyManager *pManager,
                                               const Certificate *pCert,
                                               AsymmetricKey *pKey)
{
    (void)(pManager);
    if(NULL == pCert || NULL == pKey)
        return STATUS_INVALID_PARAMETERS;

    memcpy(pKey, &pCert->crt.pk, sizeof(mbedtls_pk_context));

    return STATUS_OK;
}

