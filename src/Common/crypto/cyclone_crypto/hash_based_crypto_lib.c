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

/** \file hash_based_crypto_lib.c
 *
 * Gathers the definitions of the lib-specific and crypto-related functions to performing hash mechanisms.
 *
 * \warning     These functions should only be called through the stack API, as they don't verify
 *              nor sanitize their arguments.
 */

#include "hash_based_crypto_lib.h"
#include "sopc_assert.h"
#include "sopc_mem_alloc.h"

#include "hash/sha256.h"
#include "kdf/pbkdf.h"

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

    error_t errLib = pbkdf2(&sha256HashAlgo, pSecret, (size_t) lenSecret, pSalt, (size_t) lenSalt, iteration_count,
                            pOutput, (size_t) lenOutput);

    if (errLib)
    {
        memset(pOutput, 0, lenOutput);
        SOPC_Free(pOutput);
        return SOPC_STATUS_NOK;
    }

    *ppOutput = pOutput;
    return SOPC_STATUS_OK;
}
