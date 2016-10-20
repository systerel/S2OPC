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


#include <stdlib.h>
#include <string.h>

#include "ua_base_types.h"
#include "crypto_types.h"
#include "crypto_profiles.h"
#include "key_manager.h"

#include "mbedtls/x509.h"


StatusCode KeyManager_Certificate_Load(const KeyManager *pManager,
                                       const uint8_t *bufferDER, uint32_t lenDER,
                                       Certificate *pCert)
{
    mbedtls_x509_crt *crt = NULL;

    if(NULL == pManager || NULL == bufferDER || 0 == lenDER || NULL == pCert)
        return STATUS_INVALID_PARAMETERS;

    crt = &pCert->crt;
    mbedtls_x509_crt_init(crt);

    if(mbedtls_x509_crt_parse(crt, bufferDER, lenDER) != 0)
        return STATUS_NOK;

    return STATUS_OK;
}


StatusCode KeyManager_Certificate_LoadFromFile(const KeyManager *pManager,
                                               const int8_t *szPath,
                                               Certificate *pCert)
{
    mbedtls_x509_crt *crt = NULL;

    if(NULL == pManager || NULL == szPath || NULL == pCert)
        return STATUS_INVALID_PARAMETERS;

    crt = &pCert->crt;
    mbedtls_x509_crt_init(crt);

    if(mbedtls_x509_crt_parse_file(crt, (const char *)szPath) != 0)
        return STATUS_NOK;

    return STATUS_OK;
}


StatusCode KeyManager_Certificate_ToNewDER(const KeyManager *pManager,
                                           const Certificate *pCert,
                                           uint8_t **ppDest, uint32_t *lenAllocated)
{
    uint32_t lenToAllocate = 0;

    (void)(pManager);
    if(NULL == pCert || ppDest == NULL || NULL == lenAllocated)
        return STATUS_INVALID_PARAMETERS;

    // Allocation
    lenToAllocate = pCert->crt.raw.len;
    if(lenToAllocate == 0)
        return STATUS_NOK;

    (*ppDest) = (uint8_t *)malloc(lenToAllocate);
    if(NULL == *ppDest)
        return STATUS_NOK;

    // Copy
    memcpy((void *)(*ppDest), (void *)(pCert->crt.raw.p), lenToAllocate);
    *lenAllocated = lenToAllocate;

    return STATUS_OK;
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
    if(KeyManager_Certificate_ToNewDER(pManager, pCert, &pDER, &lenDER) != STATUS_OK)
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


