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
 * \brief Provides compatibility with different versions of MBEDTLS
 */

#ifndef SOPC_CRYPTO_MBEDTLS_COMMON_H_
#define SOPC_CRYPTO_MBEDTLS_COMMON_H_

#include "mbedtls/version.h"

#if MBEDTLS_VERSION_MAJOR == 2
/* MBEDTLS V2 */
#define MBEDTLS_RSA_RSAES_OAEP_ENCRYPT(ctx, f_rng, p_rng, label, label_len, ilen, input, output) \
    mbedtls_rsa_rsaes_oaep_encrypt(ctx, f_rng, p_rng, MBEDTLS_RSA_PUBLIC, label, label_len, ilen, input, output)
#define MBEDTLS_RSA_RSAES_OAEP_DECRYPT(ctx, f_rng, p_rng, label, label_len, olen, input, output, output_max_len)  \
    mbedtls_rsa_rsaes_oaep_decrypt(ctx, f_rng, p_rng, MBEDTLS_RSA_PRIVATE, label, label_len, olen, input, output, \
                                   output_max_len)
#define MBEDTLS_RSA_RSASSA_PKCS1_V15_SIGN(ctx, f_rng, p_rng, md_alg, hashlen, hash, sig) \
    mbedtls_rsa_rsassa_pkcs1_v15_sign(ctx, f_rng, p_rng, MBEDTLS_RSA_PRIVATE, md_alg, hashlen, hash, sig)
#define MBEDTLS_RSA_RSASSA_PKCS1_V15_VERIFY(ctx, md_alg, hashlen, hash, sig) \
    mbedtls_rsa_rsassa_pkcs1_v15_verify(ctx, NULL, NULL, MBEDTLS_RSA_PUBLIC, md_alg, hashlen, hash, sig)
#define MBEDTLS_RSA_RSASSA_PSS_SIGN(ctx, f_rng, p_rng, md_alg, hashlen, hash, sig) \
    mbedtls_rsa_rsassa_pss_sign(ctx, f_rng, p_rng, MBEDTLS_RSA_PRIVATE, md_alg, hashlen, hash, sig)
#define MBEDTLS_RSA_RSASSA_PSS_VERIFY(ctx, md_alg, hashlen, hash, sig) \
    mbedtls_rsa_rsassa_pss_verify(ctx, NULL, NULL, MBEDTLS_RSA_PUBLIC, md_alg, hashlen, hash, sig)
#define MBEDTLS_PK_PARSE_KEY mbedtls_pk_parse_key
#define MBEDTLS_RSA_SET_PADDING(prsa, padding, hash_id) mbedtls_rsa_set_padding(prsa, padding, (int) hash_id)

#elif MBEDTLS_VERSION_MAJOR == 3
/* MBEDTLS V3 */
#define MBEDTLS_RSA_RSAES_OAEP_ENCRYPT mbedtls_rsa_rsaes_oaep_encrypt
#define MBEDTLS_RSA_RSAES_OAEP_DECRYPT mbedtls_rsa_rsaes_oaep_decrypt
#define MBEDTLS_RSA_RSASSA_PKCS1_V15_SIGN mbedtls_rsa_rsassa_pkcs1_v15_sign
#define MBEDTLS_RSA_RSASSA_PKCS1_V15_VERIFY mbedtls_rsa_rsassa_pkcs1_v15_verify
#define MBEDTLS_RSA_RSASSA_PSS_SIGN mbedtls_rsa_rsassa_pss_sign
#define MBEDTLS_RSA_RSASSA_PSS_VERIFY mbedtls_rsa_rsassa_pss_verify
#define MBEDTLS_RSA_SET_PADDING(prsa, padding, hash_id) mbedtls_rsa_set_padding(prsa, padding, hash_id)

// Note: f_rng is set to NULL. mbedtls_pk_parse_key documentation states that it must not be NULL.
// However, this rng parameters are only used in scope of Elliptic curves.
#define MBEDTLS_PK_PARSE_KEY(ctx, key, keylen, pwd, pwdlen) \
    mbedtls_pk_parse_key(ctx, key, keylen, pwd, pwdlen, NULL, NULL)
#ifdef MBEDTLS_ECP_C
// TODO : f_rng cannot be NULL in case of use of Elliptic curves
#error "Cannot use elliptic curves with MBEDTLS V3. MBEDTLS_PK_PARSE_KEY must be modified to receive f_rng"
#endif

// These defines shall be set before including any other MBEDTLS headers
#ifndef MBEDTLS_ALLOW_PRIVATE_ACCESS
#define MBEDTLS_ALLOW_PRIVATE_ACCESS
#endif

#else /* MBEDTLS_VERSION_MAJOR neither 2 nor 3 */
#error "Unsupported MBEDTLS VERSION (see MBEDTLS_VERSION_MAJOR)"
#endif

#endif /* SOPC_CRYPTO_MBEDTLS_COMMON_H_ */
