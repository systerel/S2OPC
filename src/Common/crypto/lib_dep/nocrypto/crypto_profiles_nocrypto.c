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
 * Defines the cryptographic profiles: the set of functions associated to each cryptographic profiles.
 * These profiles are defined as struct of pointers. These immutable struct are extern and const, so that a profile
 * could not be modified before execution time.
 */

// The services which are implemented in this file are declared here
#include "sopc_crypto_profiles_lib_itf.h"

#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_time_reference.h"

#include "crypto_struct_nocrypto.h"

static SOPC_ReturnStatus pseudoRandom(const SOPC_CryptoProvider* pProvider,
                                      SOPC_ExposedBuffer* pData,
                                      uint32_t lenData);

const SOPC_CryptoProfile sopc_g_cpAes256Sha256RsaPss = {
    .SecurityPolicyID = SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID,
    .pFnSymmEncrypt = NULL,
    .pFnSymmDecrypt = NULL,
    .pFnSymmSign = NULL,
    .pFnSymmVerif = NULL,
    .pFnGenRnd = NULL,
    .pFnDeriveData = NULL,
    .pFnAsymEncrypt = NULL,
    .pFnAsymDecrypt = NULL,
    .pFnAsymSign = NULL,
    .pFnAsymVerify = NULL,
};

const SOPC_CryptoProfile sopc_g_cpAes128Sha256RsaOaep = {
    .SecurityPolicyID = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID,
    .pFnSymmEncrypt = NULL,
    .pFnSymmDecrypt = NULL,
    .pFnSymmSign = NULL,
    .pFnSymmVerif = NULL,
    .pFnGenRnd = NULL,
    .pFnDeriveData = NULL,
    .pFnAsymEncrypt = NULL,
    .pFnAsymDecrypt = NULL,
    .pFnAsymSign = NULL,
    .pFnAsymVerify = NULL,
};

const SOPC_CryptoProfile sopc_g_cpBasic256Sha256 = {
    .SecurityPolicyID = SOPC_SecurityPolicy_Basic256Sha256_ID,
    .pFnSymmEncrypt = NULL,
    .pFnSymmDecrypt = NULL,
    .pFnSymmSign = NULL,
    .pFnSymmVerif = NULL,
    .pFnGenRnd = NULL,
    .pFnDeriveData = NULL,
    .pFnAsymEncrypt = NULL,
    .pFnAsymDecrypt = NULL,
    .pFnAsymSign = NULL,
    .pFnAsymVerify = NULL,
};

const SOPC_CryptoProfile sopc_g_cpBasic256 = {
    .SecurityPolicyID = SOPC_SecurityPolicy_Basic256_ID,
    .pFnSymmEncrypt = NULL,
    .pFnSymmDecrypt = NULL,
    .pFnSymmSign = NULL,
    .pFnSymmVerif = NULL,
    .pFnGenRnd = NULL,
    .pFnDeriveData = NULL,
    .pFnAsymEncrypt = NULL,
    .pFnAsymDecrypt = NULL,
    .pFnAsymSign = NULL,
    .pFnAsymVerify = NULL,
};

const SOPC_CryptoProfile sopc_g_cpNone = {
    .SecurityPolicyID = SOPC_SecurityPolicy_None_ID,
    .pFnSymmEncrypt = NULL,
    .pFnSymmDecrypt = NULL,
    .pFnSymmSign = NULL,
    .pFnSymmVerif = NULL,
    .pFnGenRnd = &pseudoRandom,
    .pFnDeriveData = NULL,
    .pFnAsymEncrypt = NULL,
    .pFnAsymDecrypt = NULL,
    .pFnAsymSign = NULL,
    .pFnAsymVerify = NULL,
};

/* PubSub security policies */

const SOPC_CryptoProfile_PubSub sopc_g_cppsPubSubAes256 = {
    .SecurityPolicyID = SOPC_SecurityPolicy_PubSub_Aes256_ID,
    .pFnCrypt = NULL,
    .pFnSymmSign = NULL,
    .pFnSymmVerif = NULL,
    .pFnGenRnd = NULL,
};

const SOPC_CryptoProfile_PubSub sopc_g_cppsNone = {
    .SecurityPolicyID = SOPC_SecurityPolicy_None_ID,
    .pFnCrypt = NULL,
    .pFnSymmSign = NULL,
    .pFnSymmVerif = NULL,
    .pFnGenRnd = &pseudoRandom,
};

bool SOPC_CryptoProfile_Is_Implemented(void)
{
    return false;
}

/** Fills a buffer with pseudo-random data. The random seed uses current time added to a memorized context of previous
 * call to avoid reproducing the same sequences even without external RNG */
static SOPC_ReturnStatus pseudoRandom(const SOPC_CryptoProvider* pProvider, SOPC_ExposedBuffer* pData, uint32_t lenData)
{
    if (NULL == pProvider || NULL == pData || NULL == pProvider->pCryptolibContext)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t a = 1664525;
    uint32_t c = 1013904223;

    uint64_t x = (uint64_t) SOPC_TimeReference_GetCurrent();

    x += pProvider->pCryptolibContext->randomCtx;

    for (uint32_t i = 0; i < lenData; i++)
    {
        x = (a * x + c);
        pData[i] = (uint8_t)(x & 0xFF);
    }

    // Memorize last value to avoid repetitions
    pProvider->pCryptolibContext->randomCtx = x;

    return SOPC_STATUS_OK;
}
