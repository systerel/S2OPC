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

#include <string.h>

#include "sopc_assert.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "crypto_provider_lib.h"
#include "key_manager_lib.h"

/* ------------------------------------------------------------------------------------------------
 * CryptoProvider creation
 * ------------------------------------------------------------------------------------------------
 */

SOPC_ReturnStatus SOPC_CryptoProvider_Init(SOPC_CryptoProvider* pCryptoProvider)
{
    SOPC_CryptolibContext* pctx = NULL;

    if (NULL == pCryptoProvider)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pctx = SOPC_Malloc(sizeof(SOPC_CryptolibContext));
    if (NULL == pctx)
    {
        return SOPC_STATUS_NOK;
    }

    memset(pctx, 0, sizeof(SOPC_CryptolibContext));

    // TODO: it may be nice to not create a full context with SecuPolicy None
    pCryptoProvider->pCryptolibContext = pctx;
    int errLib = yarrowInit(&pctx->YarrowCtx);
    if (errLib)
    {
        return SOPC_STATUS_NOK;
    }

    /* We first gather some random data, which will be our initial seed.
     * TODO1 : We add entropy with external sources, gathered in input_random.
     * For the moment, we will take raw data for making the initial seed.
     */
    // The length of the input should be > 32
    size_t length_input = 32;
    const uint8_t input_random[] = {10, 145, 76, 42, 98, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0,  0,   0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    /* We seed the context. */
    errLib = yarrowSeed(&pctx->YarrowCtx, input_random, length_input);
    if (errLib)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_Deinit(SOPC_CryptoProvider* pCryptoProvider)
{
    SOPC_CryptolibContext* pCtx = NULL;

    if (NULL == pCryptoProvider)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    pCtx = pCryptoProvider->pCryptolibContext;
    if (NULL != pCtx)
    {
        yarrowDeinit(&pCtx->YarrowCtx);
        memset(pCtx, 0, sizeof(SOPC_CryptolibContext));
        SOPC_Free(pCtx);
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
    if (NULL == pProvider || NULL == pKey || NULL == lenKeyBits)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (pProfile->SecurityPolicyID == SOPC_SecurityPolicy_Invalid_ID)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint_t lenBits = mpiGetBitLength(&pKey->pubKey.n);

    if (lenBits > UINT32_MAX)
    {
        return SOPC_STATUS_NOK;
    }

    *lenKeyBits = (uint32_t) lenBits;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoProvider_AsymmetricGetLength_MsgPlainText(const SOPC_CryptoProvider* pProvider,
                                                                       const SOPC_AsymmetricKey* pKey,
                                                                       uint32_t* pLenMsg)
{
    if (NULL == pProvider || NULL == pKey || NULL == pLenMsg)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    const SOPC_CryptoProfile* pProfile = SOPC_CryptoProvider_GetProfileServices(pProvider);
    if (NULL == pProfile)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t uSecPolID = pProfile->SecurityPolicyID;
    if (SOPC_SecurityPolicy_Invalid_ID == uSecPolID)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint_t lenMessage = mpiGetByteLength(&pKey->pubKey.n);
    ;

    if (0 == lenMessage || lenMessage > UINT32_MAX)
    {
        return SOPC_STATUS_NOK;
    }

    uint32_t lenHash = 0;
    SOPC_ReturnStatus status = SOPC_CryptoProvider_AsymmetricGetLength_OAEPHashLength(pProvider, &lenHash);
    if (SOPC_STATUS_OK != status)
    {
        return SOPC_STATUS_NOK;
    }

    if (2 * lenHash + 2 > lenMessage)
    {
        return SOPC_STATUS_NOK;
    }

    lenMessage -= 2 * lenHash + 2;
    *pLenMsg = (uint32_t) lenMessage;

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
    SOPC_UNUSED_ARG(pProvider);

    if (NULL == pKey || NULL == pLenMsg)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint_t lenMessage = mpiGetByteLength(&pKey->pubKey.n);

    if (lenMessage > UINT32_MAX)
    {
        return SOPC_STATUS_NOK;
    }

    *pLenMsg = (uint32_t) lenMessage;
    if (0 == *pLenMsg)
    {
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}