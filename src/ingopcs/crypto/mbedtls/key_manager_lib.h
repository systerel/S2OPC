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
 * The asymetric key structure is mainly lib-dependent. Its content can be enriched for future uses.
 */
typedef struct AsymetricKey {
    mbedtls_pk_context pk;
} AsymetricKey;


/**
 * The certificate is mainly lib-dependent. Its content can be enriched for future uses.
 * In fact, a certificate is a linked chain of signed public key signed by the next public key, until it reaches
 *  the top-level-root-self-signed and trusted public-key.
 */
typedef struct Certificate {
    mbedtls_x509_crt crt;
} Certificate;


#endif // INGOPCS_KEY_MANAGER_LIB_H_
