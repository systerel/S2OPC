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

#include "sopc_assert.h"
#include "sopc_key_manager.h"

#include "cyclone_crypto/pkix/x509_common.h"

/**
 * \brief   The asymmetric key representation.
 */
struct SOPC_AsymmetricKey
{
    RsaPrivateKey privKey; /**< The context of the key, lib-specific */
    RsaPublicKey pubKey;
    bool isBorrowedFromCert; /**< Says whether the context is borrowed from a context or not.*/
};

/**
 * \brief   The signed public key representation.
 *
 *   It should be treated as an abstract handle.
 *   The certificate structure is mainly lib-specific.
 *   mbedtls certificates are chained.
 *
 *   Cyclone: - certificates are not chained.
 *            - add raw of the certificate because
 *              cannot export raw certificate with Cyclone
 */
struct SOPC_CertificateList
{
    X509CertificateInfo crt;
    SOPC_Buffer* raw;    // Raw data of the cert (format DER). Must be freed.
    RsaPublicKey pubKey; // Public key of the cert. Must be freed.
    SOPC_CertificateList* next;
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
    X509CrlInfo crl;
    SOPC_Buffer* raw;
    SOPC_CRLList* next;
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
