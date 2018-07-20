/*
 *  Copyright (C) 2018 Systerel and others.
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

#include <stdlib.h>
#include <string.h>

#include "crypto_provider_lib.h"

#include "../sopc_crypto_decl.h"
#include "../sopc_crypto_profiles.h"
#include "../sopc_crypto_provider.h"
#include "../sopc_key_manager.h"
#include "key_manager_lib.h"

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider creation
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus SOPC_CryptoProvider_Init(SOPC_CryptoProvider* pCryptoProvider)
{
    SOPC_CryptolibContext* pctx = NULL;

    if (NULL == pCryptoProvider)
        return SOPC_STATUS_INVALID_PARAMETERS;

    pctx = (SOPC_CryptolibContext*) malloc(sizeof(SOPC_CryptolibContext));
    if (NULL == pctx)
        return SOPC_STATUS_NOK;
    memset(pctx, 0, sizeof(SOPC_CryptolibContext));

    // TODO: it may be nice to not create a full context with SecuPolicy None
    pCryptoProvider->pCryptolibContext = pctx;
    mbedtls_entropy_init(&pctx->ctxEnt);
    mbedtls_ctr_drbg_init(&pctx->ctxDrbg);
    if (mbedtls_ctr_drbg_seed(&pctx->ctxDrbg, &mbedtls_entropy_func, &pctx->ctxEnt, NULL, 0) != 0)
        return SOPC_STATUS_NOK;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_Deinit(SOPC_CryptoProvider* pCryptoProvider)
{
    SOPC_CryptolibContext* pCtx = NULL;

    if (NULL == pCryptoProvider)
        return SOPC_STATUS_INVALID_PARAMETERS;

    pCtx = pCryptoProvider->pCryptolibContext;
    if (NULL != pCtx)
    {
        mbedtls_ctr_drbg_free(&pCtx->ctxDrbg);
        mbedtls_entropy_free(&pCtx->ctxEnt);
        free(pCtx);
    }

    return SOPC_STATUS_OK;
}

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider get-length operations
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_KeyBits(const SOPC_CryptoProvider* pProvider,
                                                                  const SOPC_AsymmetricKey* pKey,
                                                                  uint32_t* lenKeyBits)
{
    size_t lenBits = 0;

    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pKey || NULL == lenKeyBits)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (SOPC_SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID)
        return SOPC_STATUS_INVALID_PARAMETERS;

    lenBits = mbedtls_pk_get_bitlen(&pKey->pk);
    if (lenBits > UINT32_MAX)
        return SOPC_STATUS_NOK;

    *lenKeyBits = (uint32_t) lenBits;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_MsgPlainText(const SOPC_CryptoProvider* pProvider,
                                                                       const SOPC_AsymmetricKey* pKey,
                                                                       uint32_t* pLenMsg)
{
    uint32_t lenHash = 0;
    size_t lenMessage = 0;

    if (NULL == pProvider || NULL == pProvider->pProfile || NULL == pKey || NULL == pLenMsg)
        return SOPC_STATUS_INVALID_PARAMETERS;
    if (SOPC_SecurityPolicy_Invalid_ID == pProvider->pProfile->SecurityPolicyID)
        return SOPC_STATUS_INVALID_PARAMETERS;

    lenMessage = mbedtls_pk_get_len(&pKey->pk);
    if (lenMessage > UINT32_MAX)
        return SOPC_STATUS_NOK;

    *pLenMsg = (uint32_t) lenMessage;
    if (*pLenMsg == 0)
        return SOPC_STATUS_NOK;

    switch (pProvider->pProfile->SecurityPolicyID) // TODO: should we build some API to fetch the SecurityPolicyID, or
                                                   // avoid to switch on it at all?
    {
    case SOPC_SecurityPolicy_Invalid_ID:
    default:
        return SOPC_STATUS_NOK;
    case SOPC_SecurityPolicy_Basic256Sha256_ID: // TODO: this seems overkill to fetch the size of the chosen OAEP
                                                // hash function...
    case SOPC_SecurityPolicy_Basic256_ID:
        if (SOPC_CryptoProvider_AsymmetricGetLength_OAEPHashLength(pProvider, &lenHash) != SOPC_STATUS_OK)
            return SOPC_STATUS_NOK;
#ifdef __TRUSTINSOFT_BUGFIX__
        // add a test to ensure *pLenMsg > 0 (in report v1)
        if (2*lenHash + 2 >= *pLenMsg)
          return SOPC_STATUS_NOK;
#endif
        *pLenMsg -= 2 * lenHash + 2; // TODO: check for underflow?
        break;
    }

    return SOPC_STATUS_OK;
}

/**
 * \brief   Computes the size of an encrypted buffer unit.
 *          This is the length of the public key modulus.
 */
SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_MsgCipherText(const SOPC_CryptoProvider* pProvider,
                                                                        const SOPC_AsymmetricKey* pKey,
                                                                        uint32_t* pLenMsg)
{
    (void) (pProvider);
    size_t lenMessage = 0;

    if (NULL == pKey || NULL == pLenMsg)
        return SOPC_STATUS_INVALID_PARAMETERS;

    lenMessage = mbedtls_pk_get_len(&pKey->pk);
    if (lenMessage > UINT32_MAX)
        return SOPC_STATUS_NOK;

    *pLenMsg = (uint32_t) lenMessage;
    if (*pLenMsg == 0)
        return SOPC_STATUS_NOK;

    return SOPC_STATUS_OK;
}
