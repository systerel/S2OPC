/*
 * Defines the cryptographic providers: structure r/w data alongside a read-only cryptoprofile.
 *
 *  Created on: Sep 28, 2016
 *      Author: PAB
 */


#include <stdlib.h>
#include <string.h>

#include "ua_builtintypes.h"
#include "crypto_provider.h"
#include "crypto_profiles.h"


CryptoProvider *CryptoProvider_Create_char(const char *uri)
{
    CryptoProvider *pCryptoProvider = UA_NULL;
    const CryptoProfile *pProfile = UA_NULL;

    pProfile = CryptoProfile_Get(uri);
    if(UA_NULL != pProfile)
    {
        pCryptoProvider = (CryptoProvider *)malloc(sizeof(CryptoProvider));
        if(UA_NULL != pCryptoProvider)
        {
            *(const CryptoProfile **)(pCryptoProvider->pProfile) = pProfile; // TODO: this is a side-effect of putting too much const
            if(STATUS_OK != CryptoProvider_LibInit(pCryptoProvider))
            {
                free(pCryptoProvider);
                pCryptoProvider = UA_NULL;
            }
        }
    }

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
        CryptoProvider_LibDeinit(pCryptoProvider);
        free(pCryptoProvider);
    }
}

