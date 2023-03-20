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
 * \brief Defines the common declarations for the cryptographic objects.
 *
 * Avoids the circular dependencies.
 */

#ifndef SOPC_CRYPTO_DECL_H_
#define SOPC_CRYPTO_DECL_H_

typedef struct SOPC_CryptoProvider SOPC_CryptoProvider;
typedef struct SOPC_CryptoProfile SOPC_CryptoProfile;
typedef struct SOPC_CryptoProfile_PubSub SOPC_CryptoProfile_PubSub;
typedef struct SOPC_CryptolibContext SOPC_CryptolibContext;
typedef struct SOPC_AsymmetricKey SOPC_AsymmetricKey;
typedef struct SOPC_CertificateList SOPC_CertificateList;
typedef struct SOPC_CRLList SOPC_CRLList;
typedef struct SOPC_PKIProvider SOPC_PKIProvider;

#endif /* SOPC_CRYPTO_DECL_H_ */
