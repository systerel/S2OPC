/** \file
 */
/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdlib.h>

#include "sopc_base_types.h"
#include "crypto_decl.h"
#include "crypto_profiles.h"
#include "crypto_provider.h"
#include "crypto_provider_lib.h"
#include "key_manager.h"
#include "key_manager_lib.h"


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

    // TODO: it may be nice to not create a full context with SecuPolicy None
    pCryptoProvider->pCryptolibContext = pctx;
    mbedtls_entropy_init(&pctx->ctxEnt);
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
    size_t lenBits = 0;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pKey || NULL == lenKeyBits)
        return STATUS_INVALID_PARAMETERS;
    if(SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID)
        return STATUS_INVALID_PARAMETERS;

    lenBits = mbedtls_pk_get_bitlen(&pKey->pk);
    if(lenBits > UINT32_MAX)
        return STATUS_NOK;

    *lenKeyBits = (uint32_t) lenBits;

    return STATUS_OK;
}


SOPC_StatusCode CryptoProvider_AsymmetricGetLength_MsgPlainText(const CryptoProvider *pProvider,
                                                           const AsymmetricKey *pKey,
                                                           uint32_t *pLenMsg)
{
    uint32_t lenHash = 0;
    size_t lenMessage = 0;

    if(NULL == pProvider || NULL == pProvider->pProfile || NULL == pKey || NULL == pLenMsg)
        return STATUS_INVALID_PARAMETERS;
    if(SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID)
        return STATUS_INVALID_PARAMETERS;

    lenMessage = mbedtls_pk_get_len(&pKey->pk);
    if(lenMessage > UINT32_MAX)
        return STATUS_NOK;

    *pLenMsg = (uint32_t)lenMessage;
    if(pLenMsg == 0)
        return STATUS_NOK;

    switch(pProvider->pProfile->SecurityPolicyID) // TODO: should we build some API to fetch the SecurityPolicyID, or avoid to switch on it at all?
    {
    case SecurityPolicy_Invalid_ID:
    default:
        return STATUS_NOK;
    case SecurityPolicy_Basic256Sha256_ID: // TODO: this seems overkill to fetch the size of the chosen OAEP hash function...
    case SecurityPolicy_Basic256_ID:
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
    size_t lenMessage = 0;

    if(NULL == pKey || NULL == pLenMsg)
        return STATUS_INVALID_PARAMETERS;

    lenMessage = mbedtls_pk_get_len(&pKey->pk);
    if(lenMessage > UINT32_MAX)
        return STATUS_NOK;

    *pLenMsg = (uint32_t)lenMessage;
    if(pLenMsg == 0)
        return STATUS_NOK;

    return STATUS_OK;
}


