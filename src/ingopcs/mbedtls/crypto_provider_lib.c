/*
 * Defines the library specific init and deinit.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */

#include "crypto_provider.h"

StatusCode CryptoProvider_LibInit(CryptoProvider *pCryptoProvider)
{
    pCryptoProvider->pCryptolibContext = UA_NULL;
    return STATUS_OK;
}


StatusCode CryptoProvider_LibDeinit(CryptoProvider *pCryptoProvider)
{
    return STATUS_OK;
}
