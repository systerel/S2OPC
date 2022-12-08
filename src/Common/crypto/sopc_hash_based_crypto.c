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

/** \file sopc_hash_based_crypto.c
 *
 * Provides cryptographic functions to manage hash mechanisms.
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "hash_based_crypto_lib.h"
#include "sopc_hash_based_crypto.h"

/**
 * \brief   cryptographic structure to configure the algorithm used.
 */
struct SOPC_HashBasedCrypto_Config
{
    SOPC_HashBasedCrypto_Algo algo; /*!< The algorithm used */
    const SOPC_ByteString* pSalt;   /*!< The salt used */
    size_t iteration_count;         /*!< The number of iteration */
    size_t lenOutput;               /*!< The hash length in bytes */
};

SOPC_ReturnStatus SOPC_HashBasedCrypto_Config_Create(SOPC_HashBasedCrypto_Config** cfg)
{
    if (NULL == cfg)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_HashBasedCrypto_Config* _cfg = SOPC_Malloc(sizeof(SOPC_HashBasedCrypto_Config));
    if (NULL == _cfg)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    *cfg = _cfg;
    return SOPC_STATUS_OK;
}

void SOPC_HashBasedCrypto_Config_Free(SOPC_HashBasedCrypto_Config* cfg)
{
    SOPC_Free(cfg);
}

static bool hash_based_crypto_is_valid_config(const SOPC_HashBasedCrypto_Config* config)
{
    bool res = true;
    switch (config->algo)
    {
    case SOPC_HashBasedCrypto_PBKDF2_HMAC_SHA256:
        // INT32_MAX < config->lenOutput to check the cast beforehand SOPC_HashBasedCrypto_Run
        // same for UINT32_MAX < config->iteration_count
        if (NULL == config->pSalt || 0 == config->iteration_count || 0 == config->lenOutput ||
            INT32_MAX < config->lenOutput || UINT32_MAX < config->iteration_count)
        {
            res = false;
        }
        //  config->pSalt->Length > INT32_MAX to check the cast beforehand SOPC_HashBasedCrypto_Run
        if (NULL == config->pSalt->Data || 0 >= config->pSalt->Length || config->pSalt->Length > INT32_MAX)
        {
            res = false;
        }
        break;
    default:
        res = false;
        break;
    }

    return res;
}

SOPC_ReturnStatus SOPC_HashBasedCrypto_Config_PBKDF2(SOPC_HashBasedCrypto_Config* config,
                                                     const SOPC_ByteString* pSalt,
                                                     size_t iteration_count,
                                                     size_t lenOutput)
{
    if (NULL == config || NULL == pSalt || 0 == iteration_count || 0 == lenOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL == pSalt->Data || 0 >= pSalt->Length)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    config->algo = SOPC_HashBasedCrypto_PBKDF2_HMAC_SHA256;
    config->pSalt = pSalt;
    config->iteration_count = iteration_count;
    config->lenOutput = lenOutput;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_HashBasedCrypto_Run(const SOPC_HashBasedCrypto_Config* config,
                                           const SOPC_ByteString* pSecret,
                                           SOPC_ByteString** ppOutput)
{
    if (NULL == config || NULL == config->pSalt || NULL == pSecret || NULL == ppOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (NULL == pSecret->Data || 0 >= pSecret->Length)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    bool is_valid_config = hash_based_crypto_is_valid_config(config);
    if (!is_valid_config)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ByteString* pOutput = NULL;
    switch (config->algo)
    {
    case SOPC_HashBasedCrypto_PBKDF2_HMAC_SHA256:
        pOutput = SOPC_Malloc(sizeof(SOPC_ByteString));
        SOPC_ByteString_Initialize(pOutput);
        if (NULL == pOutput)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = HashBasedCrypto_DeriveSecret_PBKDF2_HMAC_SHA256(
                pSecret->Data, (uint32_t) pSecret->Length, config->pSalt->Data, (uint32_t) config->pSalt->Length,
                (uint32_t) config->iteration_count, &pOutput->Data, (uint32_t) config->lenOutput);
        }
        if (SOPC_STATUS_OK == status)
        {
            pOutput->Length = (int32_t) config->lenOutput;
            *ppOutput = pOutput;
        }
        else
        {
            SOPC_ByteString_Clear(pOutput);
        }
        break;
    default:
        status = SOPC_STATUS_INVALID_PARAMETERS;
        break;
    }

    return status;
}
