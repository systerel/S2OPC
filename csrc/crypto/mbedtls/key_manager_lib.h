/** \file key_manager_lib.h
 *
 * Implementations for AsymmetricKey and Certificate are mainly lib-specific.
 */
/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
struct SOPC_AsymmetricKey {
    mbedtls_pk_context pk;       /**< The context of the key, mbedtls_ specific */
    bool isBorrowedFromCert;     /**< Says whether the context is borrowed from a context or not. In the latter case, the context must be mbedtls_freed */
};

/**
 * \brief   The signed public key representation.
 *
 *          It should be treated as an abstract handle.
 *          The certificate structure is mainly lib-specific. Its content can be enriched for future uses.
 */
struct SOPC_Certificate {
    mbedtls_x509_crt crt;   /**< Certificate as a lib-dependent format */
    uint8_t *crt_der;       /**< Certificate in the DER format, which should be canonical. Points to internal mbedtls buffer.*/
    uint32_t len_der;       /**< Length of crt_der. */
};

/**
 * \brief   Certificate Revocation List.
 *
 *          Unspecified yet.
 *          This current  implementation might be too much tainted by mbedtls.
 */
typedef struct SOPC_CertificateRevList {
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
 * \note            Content of the certificate is unspecified when return value is not STATUS_OK.
 *
 * \return          STATUS_OK when successful, STATUS_INVALID_PARAMETERS when parameters are NULL,
 *                  and STATUS_NOK when there was an error.
 */
SOPC_StatusCode KeyManager_Certificate_GetPublicKey(const SOPC_Certificate *pCert,
                                                    SOPC_AsymmetricKey *pKey);


#endif /* SOPC_KEY_MANAGER_LIB_H_ */
