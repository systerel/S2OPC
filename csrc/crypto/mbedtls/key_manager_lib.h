/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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
 * Implementations for AsymmetricKey and Certificate are mainly lib-specific.
 */

#ifndef SOPC_KEY_MANAGER_LIB_H_
#define SOPC_KEY_MANAGER_LIB_H_

#include <stdbool.h>
#include "../sopc_crypto_decl.h"
#include "sopc_toolkit_constants.h"

#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"

/**
 * \brief   The asymmetric key representation.
 *
 *          It should be treated as an abstract handle.
 *          The asymmetric key structure is mainly lib-specific. Its content can be enriched for future uses.
 */
struct SOPC_AsymmetricKey
{
    mbedtls_pk_context pk;   /**< The context of the key, mbedtls_ specific */
    bool isBorrowedFromCert; /**< Says whether the context is borrowed from a context or not. In the latter case, the
                                context must be mbedtls_freed */
};

/**
 * \brief   The signed public key representation.
 *
 *          It should be treated as an abstract handle.
 *          The certificate structure is mainly lib-specific. Its content can be enriched for future uses.
 */
struct SOPC_Certificate
{
    mbedtls_x509_crt crt; /**< Certificate as a lib-dependent format */
    uint8_t*
        crt_der; /**< Certificate in the DER format, which should be canonical. Points to internal mbedtls buffer.*/
    uint32_t len_der; /**< Length of crt_der. */
};

/**
 * \brief   Certificate Revocation List.
 *
 *          Unspecified yet.
 *          This current  implementation might be too much tainted by mbedtls.
 */
typedef struct SOPC_CertificateRevList
{
    mbedtls_x509_crl crl;
} CertificateRevList;

/**
 * \brief           Returns the internal public key of the given signed public key.
 *
 * \warning         The returned AsymmetricKey must not be freed with KeyManager_AsymmetricKey_Free()
 *                  and the key must not be used after the Certificate is freed by KeyManager_Certificate_Free().
 *
 *                  A special flag \p isBorrowedFromCert is set to !FALSE in this case in the AsymmetricKey.
 *
 * \param pCert     A valid pointer to the signed public key.
 * \param pKey      A valid pointer to the AsymmetricKey which will be rewritten to contain the public key.
 *                  This is not a deep copy, and the key is not valid anymore when the certificate is not valid.
 *
 * \note            Content of the certificate is unspecified when return value is not SOPC_STATUS_OK.
 *
 * \return          SOPC_STATUS_OK when successful, SOPC_STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and SOPC_STATUS_NOK when there was an error.
 */
SOPC_ReturnStatus KeyManager_Certificate_GetPublicKey(const SOPC_Certificate* pCert, SOPC_AsymmetricKey* pKey);

#endif /* SOPC_KEY_MANAGER_LIB_H_ */
