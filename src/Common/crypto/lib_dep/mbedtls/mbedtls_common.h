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

#if MBEDTLS_VERSION_MAJOR == 3
/* MBEDTLS V3 */
#define MBEDTLS_RSA_RSAES_OAEP_ENCRYPT mbedtls_rsa_rsaes_oaep_encrypt
#define MBEDTLS_RSA_RSAES_OAEP_DECRYPT mbedtls_rsa_rsaes_oaep_decrypt
#define MBEDTLS_RSA_RSASSA_PKCS1_V15_SIGN mbedtls_rsa_rsassa_pkcs1_v15_sign
#define MBEDTLS_RSA_RSASSA_PKCS1_V15_VERIFY mbedtls_rsa_rsassa_pkcs1_v15_verify
#define MBEDTLS_RSA_RSASSA_PSS_SIGN mbedtls_rsa_rsassa_pss_sign
#define MBEDTLS_RSA_RSASSA_PSS_VERIFY mbedtls_rsa_rsassa_pss_verify
#define MBEDTLS_RSA_SET_PADDING(prsa, padding, hash_id) mbedtls_rsa_set_padding(prsa, padding, hash_id)
#define MBEDTLS_X509WRITE_CSR_SET_EXTENSION(ctx, oid, oid_len, val, val_len) \
    mbedtls_x509write_csr_set_extension(ctx, oid, oid_len, 0, val, val_len)

#define MBEDTLS_MD5_UPDATE(ctx, pwd, pwdLen) mbedtls_md5_update(ctx, pwd, pwdLen)
#define MBEDTLS_MD5_FINISH(ctx, pSum) mbedtls_md5_finish(ctx, pSum)
#define MBEDTLS_MD5_STARTS(ctx) mbedtls_md5_starts(ctx)

#define MBEDTLS_PK_PARSE_KEY(ctx, key, keylen, pwd, pwdlen, f_rng, p_rng) \
    mbedtls_pk_parse_key(ctx, key, keylen, pwd, pwdlen, f_rng, p_rng)
#define MBEDTLS_PK_PARSE_KEY_FILE(ctx, path, password, f_rng, p_rng) \
    mbedtls_pk_parse_keyfile(ctx, path, password, f_rng, p_rng)

// These defines shall be set before including any other MBEDTLS headers
#ifndef MBEDTLS_ALLOW_PRIVATE_ACCESS
#define MBEDTLS_ALLOW_PRIVATE_ACCESS
#endif

#else /* MBEDTLS_VERSION_MAJOR neither 2 nor 3 */
#error "Unsupported MBEDTLS VERSION (see MBEDTLS_VERSION_MAJOR)"
#endif

#endif /* SOPC_CRYPTO_MBEDTLS_COMMON_H_ */
