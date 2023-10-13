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

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_sk_provider.h"

/* Internal constantes */
#define SOPC_SK_PROVIDER_INTERNAL_PUBSUB_KEYS_SIZE (32 + 32 + 4)
#define SOPC_SK_PROVIDER_INTERNAL_PUBSUB_SECURITY_POLICY SOPC_SecurityPolicy_PubSub_Aes256_URI

typedef struct SOPC_SKProvider_RandomPubSub
{
    SOPC_CryptoProvider* cryptoProvider;
    uint32_t nbMaxKeys;
} SOPC_SKProvider_RandomPubSub;

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
                                                         uint32_t NbRequestedToken,
                                                         SOPC_String** SecurityPolicyUri,
                                                         uint32_t* FirstTokenId,
                                                         SOPC_ByteString** Keys,
                                                         uint32_t* NbToken,
                                                         uint32_t* TimeToNextKey,
                                                         uint32_t* KeyLifetime)
{
    if (NULL == skp || 0 == skp->data || NULL == Keys || NULL == NbToken || 0 == NbRequestedToken)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_SKProvider_TryList* data = (SOPC_SKProvider_TryList*) skp->data;
    SOPC_ASSERT(NULL != data);
    SOPC_ASSERT(NULL != data->providers);
    for (uint32_t i = 0; i < data->nbProviders; i++)
    {
        SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_COMMON, "Try GetKeys with provider %" PRIu32, i + 1);
        status = SOPC_SKProvider_GetKeys(data->providers[i], StartingTokenId, NbRequestedToken, SecurityPolicyUri,
                                         FirstTokenId, Keys, NbToken, TimeToNextKey, KeyLifetime);
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
    SOPC_ASSERT(NULL != data->providers);
    for (uint32_t i = 0; i < data->nbProviders; i++)
    {
        SOPC_SKProvider_Clear(data->providers[i]);
        SOPC_Free(data->providers[i]);
    }
    SOPC_Free(data->providers);
    data->providers = NULL;
    SOPC_Free(data);
}

/* RandomPubSub internal functions */

/**
 * \brief GetKeys function for SK Provider default implementation
 * Data is SOPC_CryptoProvider
 */
static SOPC_ReturnStatus SOPC_SKProvider_GetKeys_RandomPubSub_Aes256(SOPC_SKProvider* skp,
                                                                     uint32_t StartingTokenId,
                                                                     uint32_t NbRequestedToken,
                                                                     SOPC_String** SecurityPolicyUri,
                                                                     uint32_t* FirstTokenId,
                                                                     SOPC_ByteString** Keys,
                                                                     uint32_t* NbToken,
                                                                     uint32_t* TimeToNextKey,
                                                                     uint32_t* KeyLifetime)
{
    /* Not used*/

    SOPC_UNUSED_ARG(StartingTokenId);
    SOPC_UNUSED_ARG(SecurityPolicyUri);
    SOPC_UNUSED_ARG(FirstTokenId);
    SOPC_UNUSED_ARG(TimeToNextKey);
    SOPC_UNUSED_ARG(KeyLifetime);

    /* Only skp */
    if (NULL == skp || 0 == skp->data || NULL == Keys || NULL == NbToken || 0 == NbRequestedToken)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_SKProvider_RandomPubSub* data = (SOPC_SKProvider_RandomPubSub*) skp->data;

    if (0 == data->nbMaxKeys)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    uint32_t nbKeys = (NbRequestedToken < data->nbMaxKeys) ? NbRequestedToken : data->nbMaxKeys;

    SOPC_ByteString* generatedKeys = SOPC_Calloc(nbKeys, sizeof(SOPC_ByteString));
    if (NULL == generatedKeys)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Generate Random Keys and copy in out parameter */
    uint32_t keyIndex = 0;
    for (keyIndex = 0; keyIndex < nbKeys && SOPC_STATUS_OK == status; keyIndex++)
    {
        SOPC_ExposedBuffer* ppBuffer = NULL;
        SOPC_ByteString_Initialize(&generatedKeys[keyIndex]);
        status = SOPC_CryptoProvider_GenerateRandomBytes(data->cryptoProvider,
                                                         SOPC_SK_PROVIDER_INTERNAL_PUBSUB_KEYS_SIZE, &ppBuffer);
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
        for (uint32_t i = 0; i < keyIndex; i++)
        {
            SOPC_ByteString_Clear(&generatedKeys[i]);
        }
        SOPC_Free(generatedKeys);
    }
    else
    {
        *Keys = generatedKeys;
        *NbToken = nbKeys;
    }
    return status;
}

static void SOPC_SKProvider_Clear_RandomPubSub_Aes256(void* pdata)
{
    if (NULL == pdata)
    {
        return;
    }

    SOPC_SKProvider_RandomPubSub* data = (SOPC_SKProvider_RandomPubSub*) pdata;
    SOPC_CryptoProvider_Free(data->cryptoProvider);
    data->cryptoProvider = NULL;
    data->nbMaxKeys = 0;
    SOPC_Free(data);
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

    skp->data = (uintptr_t) data;
    data->providers = providers;
    data->nbProviders = nbProviders;

    skp->ptrGetKeys = SOPC_SKProvider_GetKeys_TryList;
    skp->ptrClear = SOPC_SKProvider_Clear_TryList;

    return skp;
}

SOPC_SKProvider* SOPC_SKProvider_RandomPubSub_Create(uint32_t maxKeys)
{
    SOPC_SKProvider* skp = SOPC_Malloc(sizeof(SOPC_SKProvider));
    if (NULL == skp)
    {
        return NULL;
    }

    skp->data = (uintptr_t) SOPC_Calloc(1, sizeof(SOPC_SKProvider_RandomPubSub));
    if ((uintptr_t) NULL == skp->data)
    {
        SOPC_Free(skp);
        return NULL;
    }

    SOPC_SKProvider_RandomPubSub* data = (SOPC_SKProvider_RandomPubSub*) skp->data;
    data->cryptoProvider = SOPC_CryptoProvider_CreatePubSub(SOPC_SK_PROVIDER_INTERNAL_PUBSUB_SECURITY_POLICY);
    if (NULL == data->cryptoProvider)
    {
        SOPC_Free((void*) skp->data);
        SOPC_Free(skp);
        return NULL;
    }
    data->nbMaxKeys = maxKeys;

    skp->ptrGetKeys = SOPC_SKProvider_GetKeys_RandomPubSub_Aes256;
    skp->ptrClear = SOPC_SKProvider_Clear_RandomPubSub_Aes256;

    return skp;
}

SOPC_ReturnStatus SOPC_SKProvider_GetKeys(SOPC_SKProvider* skp,
                                          uint32_t StartingTokenId,
                                          uint32_t NbRequestedToken,
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
    return skp->ptrGetKeys(skp, StartingTokenId, NbRequestedToken, SecurityPolicyUri, FirstTokenId, Keys, NbKeys,
                           TimeToNextKey, KeyLifetime);
}

void SOPC_SKProvider_Clear(SOPC_SKProvider* skp)
{
    if (NULL != skp)
    {
        if (NULL != skp->ptrClear)
        {
            skp->ptrClear((void*) skp->data);
        }
        skp->data = (uintptr_t) NULL;
    }
}
