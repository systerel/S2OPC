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

#ifndef __CONFIG_CUSTOM_MBEDTLS_H__
#define __CONFIG_CUSTOM_MBEDTLS_H__

#define MBEDTLS_THREADING_C
#define MBEDTLS_THREADING_ALT

#undef MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#define MBEDTLS_ENTROPY_HARDWARE_ALT

#define MBEDTLS_CIPHER_MODE_CTR

#define MBEDTLS_PKCS1_V21
#define MBEDTLS_PK_WRITE_C

#define MBEDTLS_X509_CRL_PARSE_C
#define MBEDTLS_X509_CHECK_KEY_USAGE
#define MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE

#endif /* __CONFIG_CUSTOM_MBEDTLS_H__ */
