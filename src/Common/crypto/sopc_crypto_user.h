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

/** \file sopc_crypto_user.h
 *
 * \brief   Defines the cryptographic API to manage user authentication.
 */

#ifndef SOPC_CRYPTO_USER_H_
#define SOPC_CRYPTO_USER_H_

#include "sopc_enums.h"
#include "sopc_mem_alloc.h"
#include "sopc_secret_buffer.h"

/**
 * \brief   Defines the supported algorithms for user authentication.
 */
typedef enum SOPC_CryptoUser_Algo
{
    PBKDF2_HMAC_SHA256, /*!<  PBKDF2 with HMAC-SHA-256 (Password-Based Key Derivation Function) */
} SOPC_CryptoUser_Algo;

/**
 * \brief   internal cryptographic context to configure the algorithm used.
 */
typedef struct SOPC_CryptoUser_Ctx SOPC_CryptoUser_Ctx;

/**
 * \brief           Initialize a context with a cryptographic algorithm
 *
 * \param ctx               A valid pointer to the newly created context.
 *                          You should free it with SOPC_CryptoUser_Ctx_Free()
 * \param algo              The cryptographic algorithm desired.
 *
 * \note            Supported \p algo values : PBKDF2_HMAC_SHA256
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL or if \p
 * algo is not supported, and SOPC_STATUS_OUT_OF_MEMORY when memory allocation failed.
 */
SOPC_ReturnStatus SOPC_CryptoUser_Ctx_Create(SOPC_CryptoUser_Ctx** ctx, SOPC_CryptoUser_Algo algo);

/**
 * \brief   Free the internal context.
 *
 * \param ctx A valid pointer to the context to freed.
 */
void SOPC_CryptoUser_Ctx_Free(SOPC_CryptoUser_Ctx* ctx);

/**
 * \brief           Configure PBKDF2_HMAC_SHA256 algorithm.
 *
 * \param ctx               A valid pointer to the context previously configured with PBKDF2_HMAC_SHA256.
 * \param pSalt             A valid pointer which contains the salt.
 * \param lenSalt           Length of \p pSalt (bytes).
 * \param iteration_count   Desired iteration count (as large as possible).
 * \param lenOutput         Length of output desired.
 *
 * \note            Function should be called after call to SOPC_CryptoUser_Ctx_Create and after to
 * SOPC_CryptoUser_Hash.
 *
 * \note            Function specific to PBKDF2_HMAC_SHA256.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL
 *                  or if \p ctx was not initialize with PBKDF2_HMAC_SHA256.
 */
SOPC_ReturnStatus SOPC_CryptoUser_Config_PBKDF2(SOPC_CryptoUser_Ctx* ctx,
                                                const SOPC_ExposedBuffer* pSalt,
                                                uint32_t lenSalt,
                                                uint32_t iteration_count,
                                                uint32_t lenOutput);

/**
 * \brief           Function that allows to execute a password hashing mechanism
 *
 * \param ctx               A valid pointer to the context previously configured with a valid algorithm.
 * \param pSecret           A valid pointer which contains the data to hash.
 * \param lenSecret         Length of \p pSecret (bytes).
 * \param ppOutput          A valid pointer to the newly created buffer which will contain the generated output.
 *                          You should free it.
 *
 * \note            When \p ctx is configured with PBKDF2_HMAC_SHA256 then \p lenSecret must not exceed 64 bytes because
 *                  if the HMAC key is longer than the blocks in the hash function
 *                  then the \p pSecret is hashed beforehand, which can reduce the entropy of the derived \p ppOutput .
 *
 * \note            PBKDF2_HMAC_SHA256 requires to be previously configured with SOPC_CryptoUser_Config_PBKDF2.
 *
 * \note            Content of the \p ppOutput is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL
 *                  or if the algorithm used is not valid.
 *                  SOPC_STATUS_NOK when there was an error (e.g. PBKDF2_HMAC_SHA256 algorithm was not configured).
 */
SOPC_ReturnStatus SOPC_CryptoUser_Hash(const SOPC_CryptoUser_Ctx* ctx,
                                       const SOPC_ExposedBuffer* pSecret,
                                       uint32_t lenSecret,
                                       SOPC_ExposedBuffer** ppOutput);

#endif /* SOPC_CRYPTO_USER_H_ */
