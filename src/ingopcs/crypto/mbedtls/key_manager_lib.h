/*
 * KeyManager provides functions for Asymmetric Key Management such as loading a signed public key,
 *  the corresponding private key, and provides the ability to verify signatures with x509 certificates.
 * KeyManager replaces the old concept of PKIProvider. PrivateKey should not be in the PublicKeyInfrastructure...
 *
 *  Created on: Oct. 19 2016
 *      Author: PAB
 */

#ifndef INGOPCS_KEY_MANAGER_LIB_H_
#define INGOPCS_KEY_MANAGER_LIB_H_


#include "mbedtls/pk.h"
#include "mbedtls/x509_crt.h"

/**
 * The asymmetric key structure is mainly lib-dependent. Its content can be enriched for future uses.
 */
typedef struct AsymmetricKey {
    mbedtls_pk_context pk;
} AsymmetricKey;


/**
 * The certificate is mainly lib-dependent. Its content can be enriched for future uses.
 * In fact, a certificate is a linked chain of signed public key signed by the next public key, until it reaches
 *  the top-level-root-self-signed and trusted public-key.
 */
typedef struct Certificate {
    mbedtls_x509_crt crt;   /**> Certificate as a lib-dependent format */
    uint8_t *crt_der;       /**> Certificate in the DER format, which should be canonical. Points to internal mbedtls buffer.*/
    uint32_t len_der;       /**> Length of crt_der. */
} Certificate;


#endif // INGOPCS_KEY_MANAGER_LIB_H_
