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

#include <stdbool.h>
#include <stdio.h>

#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pubsub_constants.h"
#include "sopc_pubsub_local_sks.h"

static SOPC_SKManager* g_skManager = NULL;
// Mutex to protect access to g_skManager;
Mutex g_mutex;

// indicate this service is initialized
bool g_init = false;

void SOPC_LocalSKS_init(void)
{
    if (g_init)
    {
        return;
    }
    Mutex_Initialization(&g_mutex);
    g_init = true;
}

void SOPC_LocalSKS_setSkManager(SOPC_SKManager* skm)
{
    Mutex_Lock(&g_mutex);
    g_skManager = skm;
    Mutex_Unlock(&g_mutex);
}

SOPC_LocalSKS_Keys* SOPC_LocalSKS_GetSecurityKeys(uint32_t groupid, uint32_t tokenId)
{
    if (SOPC_PUBSUB_SKS_DEFAULT_GROUPID != groupid)
    {
        return NULL;
    }

    /** Get Keys from SK Manager **/

    Mutex_Lock(&g_mutex);
    if (NULL == g_skManager)
    {
        Mutex_Unlock(&g_mutex);
        return NULL;
    }
    // result
    SOPC_LocalSKS_Keys* returnedKeys = NULL;

    // Get keys from manager
    SOPC_String* SecurityPolicyUri = NULL;
    uint32_t FirstTokenId = 0;
    SOPC_ByteString* Keys = NULL;
    uint32_t NbKeys = 0;
    uint32_t TimeToNextKey = 0;
    uint32_t KeyLifetime = 0;
    SOPC_ReturnStatus status =
        SOPC_SKManager_GetKeys(g_skManager, tokenId, SOPC_PUBSUB_SKS_MAX_TOKEN_PER_CALL, &SecurityPolicyUri,
                               &FirstTokenId, &Keys, &NbKeys, &TimeToNextKey, &KeyLifetime);

    Mutex_Unlock(&g_mutex);

    /** Fill Outputs **/

    // Initialize returnedkeys if GetKeys returned valid Keys corresponding to requested token
    SOPC_ByteString* byteString = NULL;
    if (SOPC_STATUS_OK == status && 0 < NbKeys &&
        (SOPC_PUBSUB_SKS_CURRENT_TOKENID == tokenId || tokenId == FirstTokenId))
    {
        byteString = &Keys[0];
        if ((32 + 32 + 4) == byteString->Length)
        {
            returnedKeys = SOPC_Calloc(1, sizeof(SOPC_LocalSKS_Keys));
        }
    }

    if (NULL != returnedKeys)
    {
        returnedKeys->tokenId = FirstTokenId;
        returnedKeys->signingKey = SOPC_SecretBuffer_NewFromExposedBuffer(byteString->Data, 32);
        returnedKeys->encryptKey = SOPC_SecretBuffer_NewFromExposedBuffer(&(byteString->Data[32]), 32);
        returnedKeys->keyNonce = SOPC_SecretBuffer_NewFromExposedBuffer(&(byteString->Data[64]), 4);
        if (NULL == returnedKeys->signingKey || NULL == returnedKeys->encryptKey || NULL == returnedKeys->keyNonce)
        {
            SOPC_LocalSKS_Keys_Delete(returnedKeys);
            SOPC_Free(returnedKeys);
            returnedKeys = NULL;
        }
    }

    SOPC_String_Clear(SecurityPolicyUri);
    SOPC_Free(SecurityPolicyUri);
    for (uint32_t i = 0; i < NbKeys && NULL != Keys; i++)
    {
        SOPC_ByteString_Clear(&Keys[i]);
    }
    SOPC_Free(Keys);

    return returnedKeys;
}

void SOPC_LocalSKS_Keys_Delete(SOPC_LocalSKS_Keys* keys)
{
    if (NULL != keys)
    {
        SOPC_SecretBuffer_DeleteClear(keys->signingKey);
        keys->signingKey = NULL;
        SOPC_SecretBuffer_DeleteClear(keys->encryptKey);
        keys->encryptKey = NULL;
        SOPC_SecretBuffer_DeleteClear(keys->keyNonce);
        keys->keyNonce = NULL;
    }
}
