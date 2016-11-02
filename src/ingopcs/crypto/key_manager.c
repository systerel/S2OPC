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
 * AsymetricKey API
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 * Cert API
 * ------------------------------------------------------------------------------------------------
 */
StatusCode KeyManager_Certificate_CopyDER(const Certificate *pCert,
                                          uint8_t **ppDest, uint32_t *lenAllocated)
{
    uint32_t lenToAllocate = 0;

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




