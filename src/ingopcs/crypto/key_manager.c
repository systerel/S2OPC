/*
 * KeyManager provides functions for Asymmetric Key Management such as loading a signed public key,
 *  the corresponding private key, and provides the ability to verify signatures with x509 certificates.
 * KeyManager replaces the old concept of PKIProvider. PrivateKey should not be in the PublicKeyInfrastructure...
 *
 * Most of the functions are lib-dependent. This file defines the others.
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


/* ------------------------------------------------------------------------------------------------
 * KeyManager API
 * ------------------------------------------------------------------------------------------------
 */
KeyManager *KeyManager_Create(CryptoProvider *pProvider,
                              const int8_t *pathTrusted, uint32_t lenPathTrusted, // Copied, will be \0 terminated
                              const int8_t *pathRevoked, uint32_t lenPathRevoked)
{
    KeyManager *out = NULL;

    if(NULL == pProvider || NULL == pProvider->pProfile || SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID)
        return NULL;
    //if(NULL == pathTrusted || NULL == pathRevoked || 0 == lenPathTrusted || 0 == lenPathRevoked) FIXME
    //    return NULL;

    out = (KeyManager *)malloc(sizeof(KeyManager));
    if(NULL != out)
    {
        out->pProvider = pProvider;
        out->pkPriv = NULL;
        out->pathTrusted = (int8_t *)malloc((lenPathTrusted+1)*sizeof(int8_t));
        out->pathRevoked = (int8_t *)malloc((lenPathRevoked+1)*sizeof(int8_t));
        if(out->pathTrusted)
        {
            strncpy((char *)out->pathTrusted, (const char *)pathTrusted, lenPathTrusted);
            out->pathTrusted[lenPathTrusted] = 0;
        }
        if(out->pathRevoked)
        {
            strncpy((char *)out->pathRevoked, (const char *)pathRevoked, lenPathRevoked);
            out->pathRevoked[lenPathRevoked] = 0;
        }
    }

    return out;
}


void KeyManager_Delete(KeyManager *pManager)
{
    if(NULL != pManager)
    {
        if(pManager->pathTrusted)
            free(pManager->pathTrusted);
        if(pManager->pathRevoked)
            free(pManager->pathRevoked);
        // TODO: pkPriv
    }
}


/* ------------------------------------------------------------------------------------------------
 * AsymetricKey API
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */
StatusCode KeyManager_CertificateGetLength_Thumbprint(const KeyManager *pManager,
                                                      uint32_t *length)
{
    if(NULL == pManager || NULL == pManager->pProvider || NULL == pManager->pProvider->pProfile || NULL == length)
        return STATUS_INVALID_PARAMETERS;

    *length = 0;
    switch(pManager->pProvider->pProfile->SecurityPolicyID)
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_NOK;
    case SecurityPolicy_Basic256Sha256_ID:
        *length = SecurityPolicy_Basic256Sha256_CertLen_Thumbprint;
        break;
    }

    return STATUS_OK;
}


StatusCode KeyManager_Certificate_CopyDER(const KeyManager *pManager,
                                          const Certificate *pCert,
                                          uint8_t **ppDest, uint32_t *lenAllocated)
{
    uint32_t lenToAllocate = 0;

    (void)(pManager);
    if(NULL == pCert || ppDest == NULL || 0 == lenAllocated)
        return STATUS_INVALID_PARAMETERS;

    // Allocation
    lenToAllocate = pCert->len_der;
    if(lenToAllocate == 0)
        return STATUS_NOK;

    (*ppDest) = (uint8_t *)malloc(lenToAllocate);
    if(NULL == *ppDest)
        return STATUS_NOK;

    // Copy
    memcpy((void *)(*ppDest), (void *)(pCert->crt_der), lenToAllocate);
    *lenAllocated = lenToAllocate;

    return STATUS_OK;
}




