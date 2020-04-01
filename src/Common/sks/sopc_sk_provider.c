/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_mem_alloc.h"
#include "sopc_sk_provider.h"

/* Internal constantes */
#define SOPC_SK_PROVIDER_INTERNAL_PUBSUB_KEYS_SIZE (32 + 32 + 4)
#define SOPC_SK_PROVIDER_INTERNAL_PUBSUB_SECURITY_POLICY SOPC_SecurityPolicy_PubSub_Aes256_URI

/*
 * Data is SOPC_CryptoProvider
 *
 */
static void SOPC_SKProvider_Clear_RandomPubSub_Aes256(void* data);

/*** DEFAULT IMPLEMENTATION FUNCTIONS ***/

typedef struct SOPC_SKProvider_TryList
{
    SOPC_SKProvider** providers;
    uint32_t nbProviders;
} SOPC_SKProvider_TryList;

/* TryList internal functions */

/**
 * \brief GetKeys function for SK Provider default implementation
 * Data is SOPC_SKProvider_TryList
 */
static SOPC_ReturnStatus SOPC_SKProvider_GetKeys_TryList(SOPC_SKProvider* skp,
                                                         uint32_t StartingTokenId,
                                                         SOPC_String** SecurityPolicyUri,
                                                         uint32_t* FirstTokenId,
                                                         SOPC_ByteString** Keys,
                                                         uint32_t* NbToken,
                                                         uint32_t* TimeToNextKey,
                                                         uint32_t* KeyLifetime)
{
    if (NULL == skp || NULL == skp->data || NULL == Keys || NULL == NbToken)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_SKProvider_TryList* data = (SOPC_SKProvider_TryList*) skp->data;
    assert(NULL != data);
    assert(NULL != data->providers);
    for (uint32_t i = 0; i < data->nbProviders; i++)
    {
        printf("Try GetKeys with provider %u\n", i + 1);
        status = SOPC_SKProvider_GetKeys(data->providers[i], StartingTokenId, SecurityPolicyUri, FirstTokenId, Keys,
                                         NbToken, TimeToNextKey, KeyLifetime);
        if (SOPC_STATUS_OK == status && 0 < *NbToken)
        {
            break;
        }
    }

    return status;
}

static void SOPC_SKProvider_Clear_TryList(void* pdata)
{
    if (NULL == pdata)
    {
        return;
    }

    SOPC_SKProvider_TryList* data = (SOPC_SKProvider_TryList*) pdata;
    assert(NULL != data->providers);
    for (uint32_t i = 0; i < data->nbProviders; i++)
    {
        SOPC_SKProvider_Clear(data->providers[i]);
        SOPC_Free(data->providers[i]);
    }
    SOPC_Free(data->providers);
    data->providers = NULL;
}

/* RandomPubSub internal functions */

/**
 * \brief GetKeys function for SK Provider default implementation
 * Data is SOPC_CryptoProvider
 */
static SOPC_ReturnStatus SOPC_SKProvider_GetKeys_RandomPubSub_Aes256(SOPC_SKProvider* skp,
                                                                     uint32_t StartingTokenId,
                                                                     SOPC_String** SecurityPolicyUri,
                                                                     uint32_t* FirstTokenId,
                                                                     SOPC_ByteString** Keys,
                                                                     uint32_t* NbToken,
                                                                     uint32_t* TimeToNextKey,
                                                                     uint32_t* KeyLifetime)
{
    /* Not used*/

    (void) StartingTokenId;
    (void) SecurityPolicyUri;
    (void) FirstTokenId;
    (void) TimeToNextKey;
    (void) KeyLifetime;

    /* Only skp */
    if (NULL == skp || NULL == skp->data || NULL == Keys || NULL == NbToken)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    int keyIndex;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_ExposedBuffer* ppBuffer;

    SOPC_CryptoProvider* provider = (SOPC_CryptoProvider*) skp->data;

    SOPC_ByteString* generatedKeys = SOPC_Calloc(SOPC_SK_PROVIDER_NB_GENERATED_KEYS, sizeof(SOPC_ByteString));
    if (NULL == generatedKeys)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Generate Random Keys and copy in out parameter */
    *NbToken = SOPC_SK_PROVIDER_NB_GENERATED_KEYS;
    for (keyIndex = 0; keyIndex < SOPC_SK_PROVIDER_NB_GENERATED_KEYS && SOPC_STATUS_OK == status; keyIndex++)
    {
        SOPC_ByteString_Initialize(&generatedKeys[keyIndex]);
        status =
            SOPC_CryptoProvider_GenerateRandomBytes(provider, SOPC_SK_PROVIDER_INTERNAL_PUBSUB_KEYS_SIZE, &ppBuffer);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ByteString_CopyFromBytes((&generatedKeys[keyIndex]), ppBuffer,
                                                   SOPC_SK_PROVIDER_INTERNAL_PUBSUB_KEYS_SIZE);
        }
        SOPC_Free(ppBuffer);
    }

    if (SOPC_STATUS_OK != status)
    {
        *NbToken = 0;
        /* Clear only initialized Keys */
        for (int i = 0; i < keyIndex && SOPC_STATUS_OK == status; i++)
        {
            SOPC_ByteString_Clear(&generatedKeys[i]);
        }
    }
    else
    {
        *Keys = generatedKeys;
        *NbToken = SOPC_SK_PROVIDER_NB_GENERATED_KEYS;
    }
    return status;
}

static void SOPC_SKProvider_Clear_RandomPubSub_Aes256(void* pdata)
{
    if (NULL == pdata)
    {
        return;
    }

    SOPC_CryptoProvider* data = (SOPC_CryptoProvider*) pdata;
    SOPC_CryptoProvider_Deinit(data);
}

/*** API FUNCTIONS ***/

SOPC_SKProvider* SOPC_SKProvider_TryList_Create(SOPC_SKProvider** providers, uint32_t nbProviders)
{
    if (NULL == providers || 0 == nbProviders)
    {
        return NULL;
    }

    SOPC_SKProvider* skp = SOPC_Malloc(sizeof(SOPC_SKProvider));
    if (NULL == skp)
    {
        return NULL;
    }

    SOPC_SKProvider_TryList* data = SOPC_Calloc(1, sizeof(SOPC_SKProvider_TryList));
    if (NULL == data)
    {
        SOPC_Free(skp);
        return NULL;
    }

    skp->data = (void*) data;
    data->providers = providers;
    data->nbProviders = nbProviders;

    skp->ptrGetKeys = SOPC_SKProvider_GetKeys_TryList;
    skp->ptrClear = SOPC_SKProvider_Clear_TryList;

    return skp;
}

SOPC_SKProvider* SOPC_SKProvider_RandomPubSub_Create()
{
    SOPC_SKProvider* skp = SOPC_Malloc(sizeof(SOPC_SKProvider));
    if (NULL == skp)
    {
        return NULL;
    }

    skp->data = (void*) SOPC_CryptoProvider_CreatePubSub(SOPC_SK_PROVIDER_INTERNAL_PUBSUB_SECURITY_POLICY);
    if (NULL == skp->data)
    {
        SOPC_Free(skp);
        return NULL;
    }

    skp->ptrGetKeys = SOPC_SKProvider_GetKeys_RandomPubSub_Aes256;
    skp->ptrClear = SOPC_SKProvider_Clear_RandomPubSub_Aes256;

    return skp;
}

SOPC_ReturnStatus SOPC_SKProvider_GetKeys(SOPC_SKProvider* skp,
                                          uint32_t StartingTokenId,
                                          SOPC_String** SecurityPolicyUri,
                                          uint32_t* FirstTokenId,
                                          SOPC_ByteString** Keys,
                                          uint32_t* NbKeys,
                                          uint32_t* TimeToNextKey,
                                          uint32_t* KeyLifetime)
{
    if (NULL == skp)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return skp->ptrGetKeys(skp, StartingTokenId, SecurityPolicyUri, FirstTokenId, Keys, NbKeys, TimeToNextKey,
                           KeyLifetime);
}

void SOPC_SKProvider_Clear(SOPC_SKProvider* skp)
{
    if (NULL != skp && NULL != skp->ptrClear)
    {
        skp->ptrClear(skp->data);
    }
    SOPC_Free(skp->data);
    skp->data = NULL;
}
