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

/** \file sopc_hash_based_crypto.h
 *
 * \brief   Defines a cryptographic API to performing hash mechanisms
 */

#ifndef SOPC_HASH_BASED_CRYPTO_H_
#define SOPC_HASH_BASED_CRYPTO_H_

#include "sopc_builtintypes.h"
#include "sopc_enums.h"
#include "sopc_mem_alloc.h"
#include "sopc_secret_buffer.h"

/**
 * \brief   Defines the supported algorithms for user authentication.
 */
typedef enum SOPC_HashBasedCrypto_Algo
{
    SOPC_HashBasedCrypto_PBKDF2_HMAC_SHA256, /*!<  PBKDF2 with HMAC-SHA-256 (Password-Based Key Derivation Function) */
} SOPC_HashBasedCrypto_Algo;

/**
 * \brief   cryptographic structure to configure the algorithm used.
 */
typedef struct SOPC_HashBasedCrypto_Config
{
    SOPC_HashBasedCrypto_Algo algo; /*!< The algorithm used */
    const SOPC_ByteString* pSalt;   /*!< The salt used */
    size_t iteration_count;         /*!< The number of iteration */
    size_t lenOutput;               /*!< The hash length in bytes */
} SOPC_HashBasedCrypto_Config;

/**
 * \brief           Fills the configuration structure for SOPC_HashBasedCrypto_PBKDF2_HMAC_SHA256 algorithm.
 *
 * \param config            A valid pointer to the configuration structure.
 * \param pSalt             A valid pointer to a ByteString which contains the salt.
 * \param iteration_count   Desired iteration count (as large as possible).
 * \param lenOutput         Length of output desired.
 *
 * \note            Function specific to SOPC_HashBasedCrypto_PBKDF2_HMAC_SHA256.
 *
 * \return          SOPC_STATUS_OK when successful or SOPC_STATUS_INVALID_PARAMETERS are invalid
 */
SOPC_ReturnStatus SOPC_HashBasedCrypto_Config_PBKDF2(SOPC_HashBasedCrypto_Config* config,
                                                     const SOPC_ByteString* pSalt,
                                                     size_t iteration_count,
                                                     size_t lenOutput);

/**
 * \brief           Function that allows to execute a hashing mechanism
 *
 * \param config            A valid pointer to the configuration structure.
 * \param pSecret           A valid pointer to a ByteString which contains the data to hash.
 * \param ppOutput          A valid pointer to the newly created ByteString which will contain the generated hash.
 *                          You should free it.
 *
 * \note            When \p config is configured with SOPC_HashBasedCrypto_PBKDF2_HMAC_SHA256 then length of \p pSecret
 * should not exceed 64 bytes because if the HMAC key is longer than the blocks in the hash function then the \p pSecret
 * is hashed beforehand, which can reduce the entropy of the derived \p ppOutput .
 *
 * \note            Content of the \p ppOutput is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful otherwise SOPC_STATUS_INVALID_PARAMETERS, SOPC_STATUS_NOK or
 * SOPC_STATUS_OUT_OF_MEMORY.
 */
SOPC_ReturnStatus SOPC_HashBasedCrypto_Run(const SOPC_HashBasedCrypto_Config* config,
                                           const SOPC_ByteString* pSecret,
                                           SOPC_ByteString** ppOutput);

#endif /* SOPC_HASH_BASED_CRYPTO_H_ */
