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
 * \brief Defines the cryptographic abstraction interface for the object.
 *        A cryptographic implementation must define all the objects declared in this file.
 */

#ifndef SOPC_CRYPTO_STRUCT_LIB_ITF_H_
#define SOPC_CRYPTO_STRUCT_LIB_ITF_H_

/**
 * \brief   Defines a lib-specific context for the SOPC_CryptoProvider object.
 *
 */
struct SOPC_CryptolibContext;

/**
 * \brief   The asymmetric key representation.
 *
 *   It should be treated as an abstract handle.
 *   The asymmetric key structure is mainly lib-specific. Its content can be enriched for future uses.
 */
struct SOPC_AsymmetricKey;

/**
 * \brief   The signed public key representation, or a chained list of such keys.
 *
 *   It should be treated as an abstract handle.
 *   The certificate structure is mainly lib-specific.
 *   This structures represents a chained list of certificates.
 *
 *   Usually, the CertificateList has length 1 when working with asymmetric cryptographic primitives,
 *   and the CertificateList has length > 1 when working with certificate validation.
 *   In the latter case, the certificates are in fact certificate authorities.
 *   See SOPC_KeyManager_Certificate_GetListLength.
 */
struct SOPC_CertificateList;

/**
 * \brief   A list of Certificate Revocation Lists.
 *
 *   It should be treated as an abstract handle.
 *   The revocation list structure is mainly lib-specific.
 *
 *   Usually, this structure is a list of known revocation lists for the trusted certificates.
 *   Each revocation list in the list is associated to one certificate authority in the
 *   trusted certificate chain.
 */
struct SOPC_CRLList;

/**
 * \brief The CSR representation.
 *
 *  It should be treated as an abstract handle.
 *  The CSR structure is mainly lib-specific. Its content can be enriched for future uses.
 */
struct SOPC_CSR;

#endif /* SOPC_CRYPTO_STRUCT_LIB_ITF_H_ */
