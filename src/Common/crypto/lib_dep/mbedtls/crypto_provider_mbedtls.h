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
 * \brief Defines the part of the SOPC_CryptoProvider which is lib-specific: SOPC_CryptolibContext.
 */

#ifndef SOPC_CRYPTO_PROVIDER_LIB_H_
#define SOPC_CRYPTO_PROVIDER_LIB_H_

// Note : this file MUST be included before other mbedtls headers
#include "mbedtls_common.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"

struct SOPC_CryptolibContext
{
    mbedtls_entropy_context ctxEnt;
    mbedtls_ctr_drbg_context ctxDrbg;
};

#endif /* SOPC_CRYPTO_PROVIDER_LIB_H_ */
