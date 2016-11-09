/** \file
 * Defines the library specific functions of the CryptoProvider.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */

#include <string.h>
#include <stdlib.h>

#include "sopc_base_types.h"
#include "crypto_types.h"
#include "crypto_profiles.h"
#include "crypto_provider.h"
#include "crypto_provider_lib.h"
#include "key_manager.h"


/* ------------------------------------------------------------------------------------------------
 * CryptoProvider creation
 * ------------------------------------------------------------------------------------------------
 */

SOPC_StatusCode CryptoProvider_Init(CryptoProvider *pCryptoProvider)
{
    CryptolibContext *pctx = NULL;

    if(NULL == pCryptoProvider)
        return STATUS_INVALID_PARAMETERS;

    pctx = (CryptolibContext *)malloc(sizeof(CryptolibContext));
    if(NULL == pctx)
        return STATUS_NOK;
    memset(pctx, 0, sizeof(CryptolibContext));

    pCryptoProvider->pCryptolibContext = pctx;
    mbedtls_entropy_init(&pctx->ctxEnt); //
    mbedtls_ctr_drbg_init(&pctx->ctxDrbg);
    if(mbedtls_ctr_drbg_seed(&pctx->ctxDrbg, &mbedtls_entropy_func, &pctx->ctxEnt, NULL, 0) != 0)
        return STATUS_NOK;

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_Deinit(CryptoProvider *pCryptoProvider)
{
    CryptolibContext *pCtx = NULL;

    if(NULL == pCryptoProvider)
        return STATUS_INVALID_PARAMETERS;

    pCtx = pCryptoProvider->pCryptolibContext;
    if(NULL != pCtx)
    {
        mbedtls_ctr_drbg_free(&pCtx->ctxDrbg);
        mbedtls_entropy_free(&pCtx->ctxEnt);
        free(pCtx);
    }

    return STATUS_OK;
}


/* ------------------------------------------------------------------------------------------------
 * CryptoProvider get-length operations
 * ------------------------------------------------------------------------------------------------
 */

SOPC_StatusCode CryptoProvider_AsymmetricGetLength_KeyBits(const CryptoProvider *pProvider,
                                                      const AsymmetricKey *pKey,
                                                      uint32_t *lenKeyBits)
{
    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pKey || NULL == lenKeyBits)
        return STATUS_INVALID_PARAMETERS;
    if(SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID)
        return STATUS_INVALID_PARAMETERS;

    *lenKeyBits = mbedtls_pk_get_bitlen(&pKey->pk);

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_AsymmetricGetLength_MsgPlainText(const CryptoProvider *pProvider,
                                                           const AsymmetricKey *pKey,
                                                           uint32_t *pLenMsg)
{
    uint32_t lenHash = 0;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pKey || NULL == pLenMsg)
        return STATUS_INVALID_PARAMETERS;
    if(SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID)
        return STATUS_INVALID_PARAMETERS;

    *pLenMsg = mbedtls_pk_get_len(&pKey->pk);
    if(pLenMsg == 0)
        return STATUS_NOK;

    switch(pProvider->pProfile->SecurityPolicyID) // TODO: should we build some API to fetch the SecurityPolicyID, or avoid to switch on it at all?
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_NOK;
    case SecurityPolicy_Basic256Sha256_ID: // TODO: this seems overkill to fetch the size of the chosen OAEP hash function...
        if(CryptoProvider_AsymmetricGetLength_OAEPHashLength(pProvider, &lenHash) != STATUS_OK)
            return STATUS_NOK;
        *pLenMsg -= 2*lenHash + 2; // TODO: check for underflow?
        break;
    }

    return STATUS_OK;
}


/**
 * \brief   Computes the size of an encrypted buffer unit.
 *          This is the length of the public key modulus.
 */
SOPC_StatusCode CryptoProvider_AsymmetricGetLength_MsgCipherText(const CryptoProvider *pProvider,
                                                            const AsymmetricKey *pKey,
                                                            uint32_t *pLenMsg)
{
    (void)(pProvider);

    if(NULL == pKey || NULL == pLenMsg)
        return STATUS_INVALID_PARAMETERS;

    *pLenMsg = mbedtls_pk_get_len(&pKey->pk);
    if(pLenMsg == 0)
        return STATUS_NOK;

    return STATUS_OK;
}


