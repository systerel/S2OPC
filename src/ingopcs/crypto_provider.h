/*
 * Defines the cryptographic providers: structure r/w data alongside a read-only cryptoprofile.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */


#ifndef INGOPCS_CRYPTO_PROVIDER_H_
#define INGOPCS_CRYPTO_PROVIDER_H_


#include "crypto_profiles.h"
#include "ua_builtintypes.h"


typedef struct CryptoProvider
{
    const struct CryptoProfile * const pProfile;
    void *pCryptolibContext;
} CryptoProvider;


CryptoProvider *CryptoProvider_Create(const UA_String *uri);
//CryptoProvider *CryptoProvider_Create(const char *uri);
void CryptoProvider_Delete(CryptoProvider *pCryptoProvider);
StatusCode CryptoProvider_LibInit(CryptoProvider *pCryptoProvider);
StatusCode CryptoProvider_LibDeinit(CryptoProvider *pCryptoProvider);


#endif  // INGOPCS_CRYPTO_PROVIDER_H_
