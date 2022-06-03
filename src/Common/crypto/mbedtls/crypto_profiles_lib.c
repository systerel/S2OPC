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

#include <stddef.h>

#include "crypto_functions_lib.h"
#include "sopc_crypto_profiles.h"

const SOPC_CryptoProfile sopc_g_cpAes256Sha256RsaPss = {
    .SecurityPolicyID = SOPC_SecurityPolicy_Aes256Sha256RsaPss_ID,
    .pFnSymmEncrypt = &CryptoProvider_SymmEncrypt_AES256,
    .pFnSymmDecrypt = &CryptoProvider_SymmDecrypt_AES256,
    .pFnSymmSign = &CryptoProvider_SymmSign_HMAC_SHA256,
    .pFnSymmVerif = &CryptoProvider_SymmVerify_HMAC_SHA256,
    .pFnGenRnd = &CryptoProvider_GenTrueRnd,
    .pFnDeriveData = &CryptoProvider_DeriveData_PRF_SHA256,
    .pFnAsymEncrypt = &CryptoProvider_AsymEncrypt_RSA_OAEP_SHA256,
    .pFnAsymDecrypt = &CryptoProvider_AsymDecrypt_RSA_OAEP_SHA256,
    .pFnAsymSign = &CryptoProvider_AsymSign_RSASSA_PSS,
    .pFnAsymVerify = &CryptoProvider_AsymVerify_RSASSA_PSS,
    .pFnCertVerify = &CryptoProvider_CertVerify_RSA_SHA256_2048_4096,
};

const SOPC_CryptoProfile sopc_g_cpAes128Sha256RsaOaep = {
    .SecurityPolicyID = SOPC_SecurityPolicy_Aes128Sha256RsaOaep_ID,
    .pFnSymmEncrypt = &CryptoProvider_SymmEncrypt_AES128,
    .pFnSymmDecrypt = &CryptoProvider_SymmDecrypt_AES128,
    .pFnSymmSign = &CryptoProvider_SymmSign_HMAC_SHA256,
    .pFnSymmVerif = &CryptoProvider_SymmVerify_HMAC_SHA256,
    .pFnGenRnd = &CryptoProvider_GenTrueRnd,
    .pFnDeriveData = &CryptoProvider_DeriveData_PRF_SHA256,
    .pFnAsymEncrypt = &CryptoProvider_AsymEncrypt_RSA_OAEP,
    .pFnAsymDecrypt = &CryptoProvider_AsymDecrypt_RSA_OAEP,
    .pFnAsymSign = &CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA256,
    .pFnAsymVerify = &CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA256,
    .pFnCertVerify = &CryptoProvider_CertVerify_RSA_SHA256_2048_4096,
};

const SOPC_CryptoProfile sopc_g_cpBasic256Sha256 = {
    .SecurityPolicyID = SOPC_SecurityPolicy_Basic256Sha256_ID,
    .pFnSymmEncrypt = &CryptoProvider_SymmEncrypt_AES256,
    .pFnSymmDecrypt = &CryptoProvider_SymmDecrypt_AES256,
    .pFnSymmSign = &CryptoProvider_SymmSign_HMAC_SHA256,
    .pFnSymmVerif = &CryptoProvider_SymmVerify_HMAC_SHA256,
    .pFnGenRnd = &CryptoProvider_GenTrueRnd,
    .pFnDeriveData = &CryptoProvider_DeriveData_PRF_SHA256,
    .pFnAsymEncrypt = &CryptoProvider_AsymEncrypt_RSA_OAEP,
    .pFnAsymDecrypt = &CryptoProvider_AsymDecrypt_RSA_OAEP,
    .pFnAsymSign = &CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA256,
    .pFnAsymVerify = &CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA256,
    .pFnCertVerify = &CryptoProvider_CertVerify_RSA_SHA256_2048_4096,
};

const SOPC_CryptoProfile sopc_g_cpBasic256 = {
    .SecurityPolicyID = SOPC_SecurityPolicy_Basic256_ID,
    .pFnSymmEncrypt = &CryptoProvider_SymmEncrypt_AES256,
    .pFnSymmDecrypt = &CryptoProvider_SymmDecrypt_AES256,
    .pFnSymmSign = &CryptoProvider_SymmSign_HMAC_SHA1,
    .pFnSymmVerif = &CryptoProvider_SymmVerify_HMAC_SHA1,
    .pFnGenRnd = &CryptoProvider_GenTrueRnd,
    .pFnDeriveData = &CryptoProvider_DeriveData_PRF_SHA1,
    .pFnAsymEncrypt = &CryptoProvider_AsymEncrypt_RSA_OAEP,
    .pFnAsymDecrypt = &CryptoProvider_AsymDecrypt_RSA_OAEP,
    .pFnAsymSign = &CryptoProvider_AsymSign_RSASSA_PKCS1_v15_w_SHA1,
    .pFnAsymVerify = &CryptoProvider_AsymVerify_RSASSA_PKCS1_v15_w_SHA1,
    .pFnCertVerify = &CryptoProvider_CertVerify_RSA_SHA1_SHA256_1024_2048,
};

const SOPC_CryptoProfile sopc_g_cpNone = {
    .SecurityPolicyID = SOPC_SecurityPolicy_None_ID,
    .pFnSymmEncrypt = NULL,
    .pFnSymmDecrypt = NULL,
    .pFnSymmSign = NULL,
    .pFnSymmVerif = NULL,
    .pFnGenRnd = &CryptoProvider_GenTrueRnd,
    .pFnDeriveData = NULL,
    .pFnAsymEncrypt = NULL,
    .pFnAsymDecrypt = NULL,
    .pFnAsymSign = NULL,
    .pFnAsymVerify = NULL,
    .pFnCertVerify = NULL,
};

/* PubSub security policies */

const SOPC_CryptoProfile_PubSub sopc_g_cppsPubSubAes256 = {
    .SecurityPolicyID = SOPC_SecurityPolicy_PubSub_Aes256_ID,
    .pFnCrypt = &CryptoProvider_CTR_Crypt_AES256,
    .pFnSymmSign = &CryptoProvider_SymmSign_HMAC_SHA256,
    .pFnSymmVerif = &CryptoProvider_SymmVerify_HMAC_SHA256,
    .pFnGenRnd = &CryptoProvider_GenTrueRnd,
};

const SOPC_CryptoProfile_PubSub sopc_g_cppsNone = {
    .SecurityPolicyID = SOPC_SecurityPolicy_None_ID,
    .pFnCrypt = NULL,
    .pFnSymmSign = NULL,
    .pFnSymmVerif = NULL,
    .pFnGenRnd = &CryptoProvider_GenTrueRnd,
};
