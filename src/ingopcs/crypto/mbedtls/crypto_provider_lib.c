/*
 * Defines the library specific init and deinit.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */

#include <string.h>
#include <stdlib.h>

#include "crypto_provider.h"
#include "crypto_provider_lib.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

StatusCode CryptoProvider_LibInit(CryptoProvider *pCryptoProvider)
{
    CryptolibContext *pctx = NULL;

    if(NULL == pCryptoProvider)
        return STATUS_INVALID_PARAMETERS;

    pctx = (CryptolibContext *)malloc(sizeof(CryptolibContext));
    if(NULL == pctx)
        return STATUS_NOK;
    memset(pctx, 0, sizeof(CryptolibContext));

    pCryptoProvider->pCryptolibContext = (void *)pctx;
    mbedtls_entropy_init(&pctx->ctxEnt); //
    mbedtls_ctr_drbg_init(&pctx->ctxDrbg);
    if(mbedtls_ctr_drbg_seed(&pctx->ctxDrbg, &mbedtls_entropy_func, &pctx->ctxEnt, NULL, 0) != 0)
        return STATUS_NOK;

    return STATUS_OK;
}


StatusCode CryptoProvider_LibDeinit(CryptoProvider *pCryptoProvider)
{
    CryptolibContext *pCtx = NULL;

    if(NULL == pCryptoProvider)
        return STATUS_INVALID_PARAMETERS;

    pCtx = pCryptoProvider->pCryptolibContext;
    if(NULL != pCtx)
    {
        mbedtls_ctr_drbg_free(&pCtx->ctxDrbg);
        mbedtls_entropy_free(&pCtx->ctxEnt);
        free(pCtx);
    }

    return STATUS_OK;
}
