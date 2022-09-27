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

/** \file sopc_crypto_user.c
 *
 * Provides cryptographic functions to manage user authentication such as password hash.
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "crypto_user_lib.h"
#include "sopc_crypto_user.h"

typedef struct SOPC_CryptoUser_Config
{
    const SOPC_ExposedBuffer* salt;
    uint32_t lenSalt;
    uint32_t iteration_count;
    uint32_t lenOutput;
} SOPC_CryptoUser_Config;

struct SOPC_CryptoUser_Ctx
{
    bool is_configure;
    SOPC_CryptoUser_Algo algo;
    SOPC_CryptoUser_Config config;
};

static bool SOPC_CryptoUser_IsValid_Algo(SOPC_CryptoUser_Algo algo)
{
    switch (algo)
    {
    case PBKDF2_HMAC_SHA256:
        break;
    default:
        return false;
    }

    return true;
}

SOPC_ReturnStatus SOPC_CryptoUser_Ctx_Create(SOPC_CryptoUser_Ctx** ctx, SOPC_CryptoUser_Algo algo)
{
    bool is_valid_algo = SOPC_CryptoUser_IsValid_Algo(algo);
    if (!is_valid_algo || ctx == NULL)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_CryptoUser_Ctx* _ctx = SOPC_Malloc(sizeof(SOPC_CryptoUser_Ctx));
    if (NULL == _ctx)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    _ctx->is_configure = false;
    _ctx->algo = algo;
    memset(&_ctx->config, 0, sizeof(SOPC_CryptoUser_Config));

    *ctx = _ctx;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoUser_Config_PBKDF2(SOPC_CryptoUser_Ctx* ctx,
                                                const SOPC_ExposedBuffer* pSalt,
                                                uint32_t lenSalt,
                                                uint32_t iteration_count,
                                                uint32_t lenOutput)
{
    if (NULL == ctx || NULL == pSalt || 0 == lenSalt || 0 == iteration_count)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (ctx->algo != PBKDF2_HMAC_SHA256)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    ctx->is_configure = true;
    ctx->config.salt = pSalt;
    ctx->config.lenSalt = lenSalt;
    ctx->config.iteration_count = iteration_count;
    ctx->config.lenOutput = lenOutput;

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_CryptoUser_Hash(const SOPC_CryptoUser_Ctx* ctx,
                                       const SOPC_ExposedBuffer* pSecret,
                                       uint32_t lenSecret,
                                       SOPC_ExposedBuffer** ppOutput)
{
    if (NULL == ctx || NULL == pSecret || NULL == ppOutput || 0 == lenSecret)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    bool is_valid_algo = SOPC_CryptoUser_IsValid_Algo(ctx->algo);
    if (!is_valid_algo)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (PBKDF2_HMAC_SHA256 == ctx->algo)
    {
        if (!ctx->is_configure)
        {
            return SOPC_STATUS_NOK;
        }
        status =
            CryptoUser_DeriveSecret_PBKDF2_HMAC_SHA256(pSecret, lenSecret, ctx->config.salt, ctx->config.lenSalt,
                                                       ctx->config.iteration_count, ppOutput, ctx->config.lenOutput);
    }

    return status;
}

void SOPC_CryptoUser_Ctx_Free(SOPC_CryptoUser_Ctx* ctx)
{
    if (NULL != ctx)
    {
        SOPC_Free(ctx);
    }
    ctx = NULL;
}
