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

/** \file
 *
 * \brief Defines the cryptographic abstraction interface to performing hash mechanisms.
 *        A cryptographic implementation must define all the functions declared in this file.
 *
 */

#ifndef SOPC_HASH_BASED_CRYPTO_LIB_ITF_H
#define SOPC_HASH_BASED_CRYPTO_LIB_ITF_H

#include "sopc_enums.h"
#include "sopc_secret_buffer.h"

/**
 * \brief           Password-Based Key Derivation Function.
 *
 * \param pSecret           A valid pointer which contains the data to use when generating output.
 * \param lenSecret         Length of \p pSecret (bytes).
 * \param pSalt             A valid pointer which contains the salt when generating output.
 * \param lenSalt           Length of \p pSalt (bytes).
 * \param iteration_count   Desired iteration count (as large as possible).
 * \param ppOutput          A valid pointer to the newly created buffer which will contain the generated output.
 *                          You should free it.
 * \param lenOutput         Length of \p ppOutput desired.
 *
 * \note            \p lenSecret should not exceed 32 bytes because a secret longer than digest size
 *                  does not protect more, because, HMAC-SHA256 is considered to have 256 bits maximum input entropy.
 *
 * \note            You doesn't have to free the content of \p ppOutput when there is an error.
 *
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error (e.g. no entropy source).
 */
SOPC_ReturnStatus HashBasedCrypto_DeriveSecret_PBKDF2_HMAC_SHA256(const SOPC_ExposedBuffer* pSecret,
                                                                  uint32_t lenSecret,
                                                                  const SOPC_ExposedBuffer* pSalt,
                                                                  uint32_t lenSalt,
                                                                  uint32_t iteration_count,
                                                                  SOPC_ExposedBuffer** ppOutput,
                                                                  uint32_t lenOutput);

#endif /* SOPC_HASH_BASED_CRYPTO_LIB_ITF_H */
