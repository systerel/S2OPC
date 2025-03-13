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

#ifndef CRYPTO_STRUCT_NOCRYPTO_H_
#define CRYPTO_STRUCT_NOCRYPTO_H_

#include <string.h>

// The services which are implemented in this file are declared here
#include "sopc_crypto_struct_lib_itf.h"

#include "sopc_crypto_decl.h"

#define EMPTY_STRUCT(x) \
    struct x            \
    {                   \
        int unused;     \
    }

struct SOPC_CryptolibContext
{
    uint32_t randomCtx;
};
EMPTY_STRUCT(SOPC_AsymmetricKey);
EMPTY_STRUCT(SOPC_CertificateList);
EMPTY_STRUCT(SOPC_CRLList);
EMPTY_STRUCT(SOPC_CSR);

#endif // CRYPTO_STRUCT_NOCRYPTO_H_
