/*
 * Defines the cryptographic providers: structure r/w data alongside a read-only cryptoprofile.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */



#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>

#include "ua_builtintypes.h"
#include "crypto_provider.h"
#include "crypto_profiles.h"


CryptoProvider *CryptoProvider_Create_char(const char *uri)
{
    CryptoProvider *pCryptoProvider;

    pCryptoProvider = (CryptoProvider *)malloc(sizeof(CryptoProvider));
    // TODO: get correct profile

    return pCryptoProvider;
}


CryptoProvider *CryptoProvider_Create(const UA_String *uri)
{
    char *cUri = String_GetCString(uri);
    CryptoProvider *pCrypto = UA_NULL;

    if(UA_NULL != cUri)
    {
        pCrypto = CryptoProvider_Create_char(cUri);
        free(cUri);
    }

    return pCrypto;
}


void CryptoProvider_Delete(CryptoProvider* pCryptoProvider)
{
    if(UA_NULL != pCryptoProvider)
    {
        // TODO
        free(pCryptoProvider);
    }
}

#ifdef __cplusplus
}
#endif
