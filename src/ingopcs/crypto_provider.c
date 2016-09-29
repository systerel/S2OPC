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


CryptoProvider *CryptoProvider_Create_Low(const char *uri)
{
    CryptoProvider *pCryptoProvider = UA_NULL;
    const CryptoProfile *pProfile = UA_NULL;

    pProfile = CryptoProfile_Get(uri);
    if(UA_NULL != pProfile)
    {
        pCryptoProvider = (CryptoProvider *)malloc(sizeof(CryptoProvider));
        if(UA_NULL != pCryptoProvider)
        {
            *(const CryptoProfile **)(&pCryptoProvider->pProfile) = pProfile; // TODO: this is a side-effect of putting too much const
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
        pCrypto = CryptoProvider_Create_Low(cUri);
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


StatusCode CryptoProvider_SymmetricEncrypt_Low(const struct CryptoProvider *pProvider,
                                               const uint8_t *pInput,
                                               uint32_t lenPlainText,
                                               const KeyBuffer *pKey,
                                               const uint8_t *pIV,
                                               uint8_t *pOutput,
                                               uint32_t lenOutput)
{
    if(UA_NULL != pProvider && UA_NULL != pProvider->pProfile)
        return pProvider->pProfile->pFnSymmEncrypt(pProvider, pInput, lenPlainText, pKey, pIV, pOutput, lenOutput);

    return STATUS_INVALID_PARAMETERS;
}


StatusCode CryptoProvider_SymmetricDecrypt_Low(const struct CryptoProvider *pProvider,
                                               const uint8_t *pInput,
                                               uint32_t lenCipherText,
                                               const KeyBuffer *pKey,
                                               const uint8_t *pIV,
                                               uint8_t *pOutput,
                                               uint32_t lenOutput)
{
    if(UA_NULL != pProvider && UA_NULL != pProvider->pProfile)
        return pProvider->pProfile->pFnSymmDecrypt(pProvider, pInput, lenCipherText, pKey, pIV, pOutput, lenOutput);

    return STATUS_INVALID_PARAMETERS;
}

