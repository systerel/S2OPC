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


#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"

/**
 * \brief   The asymmetric key representation.
 *
 *          It should be treated as an abstract handle.
 *          The asymmetric key structure is mainly lib-specific. Its content can be enriched for future uses.
 */
typedef struct AsymmetricKey {
    mbedtls_pk_context pk;
} AsymmetricKey;


/**
 * \brief   The signed public key representation.
 *
 *          It should be treated as an abstract handle.
 *          The certificate structure is mainly lib-specific. Its content can be enriched for future uses.
 */
typedef struct Certificate {
    mbedtls_x509_crt crt;   /**< Certificate as a lib-dependent format */
    uint8_t *crt_der;       /**< Certificate in the DER format, which should be canonical. Points to internal mbedtls buffer.*/
    uint32_t len_der;       /**< Length of crt_der. */
} Certificate;

/**
 * \brief   Certificate Revocation List.
 *
 *          Unspecified yet.
 *          This current  implementation might be too much tainted by mbedtls.
 */
typedef struct CertificateRevList {
    mbedtls_x509_crl crl;
} CertificateRevList;


#endif /* SOPC_KEY_MANAGER_LIB_H_ */
