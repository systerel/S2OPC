/** \file key_manager_lib.h
 *
 * Implementations for AsymmetricKey and Certificate are mainly lib-specific.
 *
 *  Created on: Oct. 19 2016
 *      Author: PAB
 */

#ifndef INGOPCS_KEY_MANAGER_LIB_H_
#define INGOPCS_KEY_MANAGER_LIB_H_


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


#endif // INGOPCS_KEY_MANAGER_LIB_H_
