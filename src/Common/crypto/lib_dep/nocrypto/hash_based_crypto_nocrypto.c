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

#include "sopc_hash_based_crypto_lib_itf.h"
#include "sopc_macros.h"

SOPC_ReturnStatus HashBasedCrypto_DeriveSecret_PBKDF2_HMAC_SHA256(const SOPC_ExposedBuffer* pSecret,
                                                                  uint32_t lenSecret,
                                                                  const SOPC_ExposedBuffer* pSalt,
                                                                  uint32_t lenSalt,
                                                                  uint32_t iteration_count,
                                                                  SOPC_ExposedBuffer** ppOutput,
                                                                  uint32_t lenOutput)
{
    SOPC_UNUSED_ARG(pSecret);
    SOPC_UNUSED_ARG(lenSecret);
    SOPC_UNUSED_ARG(pSalt);
    SOPC_UNUSED_ARG(lenSalt);
    SOPC_UNUSED_ARG(iteration_count);
    SOPC_UNUSED_ARG(ppOutput);
    SOPC_UNUSED_ARG(lenOutput);
    return SOPC_STATUS_NOT_SUPPORTED;
}
