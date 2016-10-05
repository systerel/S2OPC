/*
 * Defines the library specific structures.
 *
 *  Created on: Sep 30, 2016
 *      Author: PAB
 */

#ifndef INGOPCS_CRYPTO_PROVIDER_LIB_H_
#define INGOPCS_CRYPTO_PROVIDER_LIB_H_

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"


typedef struct CryptolibContext {
    mbedtls_entropy_context ctxEnt;
    mbedtls_ctr_drbg_context ctxDrbg;
} CryptolibContext;


#endif // INGOPCS_CRYPTO_PROVIDER_LIB_H_
