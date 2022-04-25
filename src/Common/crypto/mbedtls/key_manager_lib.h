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

/** \file key_manager_lib.h
 *
 * Implementations for SOPC_AsymmetricKey and SOPC_CertificateList are mainly lib-specific.
 */

#ifndef SOPC_KEY_MANAGER_LIB_H_
#define SOPC_KEY_MANAGER_LIB_H_

#include <stdbool.h>

#include "sopc_crypto_decl.h"
#include "sopc_enums.h"

// Note : this file MUST be included before other mbedtls headers
#include "mbedtls_common.h"

#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"

/**
 * \brief   The asymmetric key representation.
 *
 *   It should be treated as an abstract handle.
 *   The asymmetric key structure is mainly lib-specific. Its content can be enriched for future uses.
 */
struct SOPC_AsymmetricKey
{
    mbedtls_pk_context pk;   /**< The context of the key, mbedtls_ specific */
    bool isBorrowedFromCert; /**< Says whether the context is borrowed from a context or not. In the latter case, the
                                context must be mbedtls_freed */
};

/**
 * \brief   The signed public key representation, or a chained list of such keys.
 *
 *   It should be treated as an abstract handle.
 *   The certificate structure is mainly lib-specific.
 *   mbedtls certificates are chained.
 *   This structures represents a chained list of certificates.
 *
 *   Usually, the CertificateList has length 1 when working with asymmetric cryptographic primitives,
 *   and the CertificateList has length > 1 when working with certificate validation.
 *   In the latter case, the certificates are in fact certificate authorities.
 *   See SOPC_KeyManager_Certificate_GetListLength.
 */
struct SOPC_CertificateList
{
    mbedtls_x509_crt crt; /**< Certificate as a lib-dependent format */
};

/**
 * \brief   A list of Certificate Revocation Lists.
 *
 *   It should be treated as an abstract handle.
 *   The revocation list structure is mainly lib-specific.
 *   mbedtls revocation lists are chained.
 *
 *   Usually, this structure is a list of known revocation lists for the trusted certificates.
 *   Each revocation list in the list is associated to one certificate authority in the
 *   trusted certificate chain.
 */
struct SOPC_CRLList
{
    mbedtls_x509_crl crl;
};

/**
 * \brief           Returns the internal public key of the given signed public key.
 *
 * \warning         The returned SOPC_AsymmetricKey must not be freed with SOPC_KeyManager_AsymmetricKey_Free()
 *                  and the key must not be used after the SOPC_CertificateList is freed by
 *                  SOPC_KeyManager_Certificate_Free().
 *
 * \warning         A special flag \p isBorrowedFromCert is set to !FALSE in this case in the SOPC_AsymmetricKey.
 *
 * \param pCert     A valid pointer to the signed public key.
 * \param pKey      A valid pointer to the SOPC_AsymmetricKey which will be rewritten to contain the public key.
 *                  This is not a deep copy, and the key is not valid anymore when the certificate is not valid.
 *
 * \note            Content of the certificate is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus KeyManager_Certificate_GetPublicKey(const SOPC_CertificateList* pCert, SOPC_AsymmetricKey* pKey);

#endif /* SOPC_KEY_MANAGER_LIB_H_ */
