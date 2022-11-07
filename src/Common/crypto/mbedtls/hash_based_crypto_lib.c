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

/** \file crypto_user_lib.c
 *
 * Gathers the definitions of the lib-specific and crypto-related functions to performing hash mechanisms.
 *
 * \warning     These functions should only be called through the stack API, as they don't verify
 *              nor sanitize their arguments.
 */

#include "hash_based_crypto_lib.h"
#include "sopc_assert.h"
#include "sopc_mem_alloc.h"

// Note : this file MUST be included before other mbedtls headers
#include "mbedtls/md.h"
#include "mbedtls/pkcs5.h"
#include "mbedtls_common.h"

SOPC_ReturnStatus HashBasedCrypto_DeriveSecret_PBKDF2_HMAC_SHA256(const SOPC_ExposedBuffer* pSecret,
                                                                  uint32_t lenSecret,
                                                                  const SOPC_ExposedBuffer* pSalt,
                                                                  uint32_t lenSalt,
                                                                  uint32_t iteration_count,
                                                                  SOPC_ExposedBuffer** ppOutput,
                                                                  uint32_t lenOutput)
{
    if (NULL == pSecret || NULL == pSalt || NULL == ppOutput || 0 == lenSalt || 0 == lenOutput)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ExposedBuffer* pOutput = SOPC_Malloc(sizeof(SOPC_ExposedBuffer) * lenOutput);
    if (NULL == pOutput)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    mbedtls_md_context_t ctx = {0};
    const mbedtls_md_info_t* info = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    int err = -1;

    mbedtls_md_init(&ctx);
    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (NULL == info)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        err = mbedtls_md_setup(&ctx, info, 1);
        if (err)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        err = mbedtls_pkcs5_pbkdf2_hmac(&ctx, pSecret, lenSecret, pSalt, lenSalt, iteration_count, lenOutput, pOutput);
        if (err)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    // Free the context
    mbedtls_md_free(&ctx);
    // Free the output in case of error
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(pOutput);
        pOutput = NULL;
    }
    // Attach the output with the allocated buffer
    *ppOutput = pOutput;

    return status;
}
