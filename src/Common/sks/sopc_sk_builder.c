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

#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_sk_builder.h"

/*** DEFAULT IMPLEMENTATION FUNCTIONS ***/

static SOPC_ReturnStatus SOPC_SKBuilder_Update_Default_Setter(SOPC_SKBuilder* skb,
                                                              SOPC_SKProvider* skp,
                                                              SOPC_SKManager* skm)
{
    SOPC_ASSERT(NULL != skb && NULL != skp && NULL != skm);

    SOPC_String* SecurityPolicyUri = NULL;
    uint32_t FirstTokenId = 0;
    SOPC_ByteString* Keys = NULL;
    uint32_t NbToken = 0;
    uint32_t TimeToNextKey = 0;
    uint32_t KeyLifetime = 0;

    SOPC_ReturnStatus status =
        SOPC_SKProvider_GetKeys(skp, skm->securityGroupId, SOPC_SK_MANAGER_CURRENT_TOKEN_ID, UINT32_MAX,
                                &SecurityPolicyUri, &FirstTokenId, &Keys, &NbToken, &TimeToNextKey, &KeyLifetime);

    if (SOPC_STATUS_OK == status && NULL != Keys && 0 < NbToken)
    {
        status =
            SOPC_SKManager_SetKeys(skm, SecurityPolicyUri, FirstTokenId, Keys, NbToken, TimeToNextKey, KeyLifetime);

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "Security Key Service: Error Builder cannot set Keys");
        }

        for (uint32_t i = 0; i < NbToken; i++)
        {
            SOPC_ByteString_Clear(&Keys[i]);
        }
        SOPC_Free(Keys);
        SOPC_String_Clear(SecurityPolicyUri);
        SOPC_Free(SecurityPolicyUri);
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "Security Key Service: Error in Builder By SKS - cannot load keys");
    }
    return status;
}

typedef struct SOPC_SKBuilder_TruncateData
{
    SOPC_SKBuilder* skb;
    uint32_t sizeMax;
} SOPC_SKBuilder_TruncateData;

static SOPC_ReturnStatus SOPC_SKBuilder_Update_Truncate(SOPC_SKBuilder* skb, SOPC_SKProvider* skp, SOPC_SKManager* skm)
{
    SOPC_ASSERT(NULL != skb && NULL != skb->data);

    SOPC_SKBuilder_TruncateData* data = (SOPC_SKBuilder_TruncateData*) skb->data;

    SOPC_ReturnStatus status = SOPC_SKBuilder_Update(data->skb, skp, skm);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    uint32_t size = SOPC_SKManager_Size(skm);

    if (size > data->sizeMax)
    {
        SOPC_String* SecurityPolicyUri = NULL;
        uint32_t FirstTokenId = 0;
        SOPC_ByteString* Keys = NULL;
        uint32_t NbToken = 0;
        uint32_t TimeToNextKey = 0;
        uint32_t KeyLifetime = 0;

        status = SOPC_SKManager_GetKeys(skm, SOPC_SK_MANAGER_CURRENT_TOKEN_ID, data->sizeMax, &SecurityPolicyUri,
                                        &FirstTokenId, &Keys, &NbToken, &TimeToNextKey, &KeyLifetime);
        if (SOPC_STATUS_OK == status)
        {
            status =
                SOPC_SKManager_SetKeys(skm, SecurityPolicyUri, FirstTokenId, Keys, NbToken, TimeToNextKey, KeyLifetime);
        }

        SOPC_String_Clear(SecurityPolicyUri);
        SOPC_Free(SecurityPolicyUri);
        for (uint32_t i = 0; i < NbToken; i++)
        {
            SOPC_ByteString_Clear(&Keys[i]);
        }
        SOPC_Free(Keys);
    }
    return status;
}

static void SOPC_SKBuilder_Clear_Truncate(void* data)
{
    SOPC_SKBuilder_TruncateData* truncateData = (SOPC_SKBuilder_TruncateData*) data;
    SOPC_SKBuilder_Clear(truncateData->skb);
    SOPC_Free(truncateData->skb);
}

static SOPC_ReturnStatus SOPC_SKBuilder_Update_Default_Append(SOPC_SKBuilder* skb,
                                                              SOPC_SKProvider* skp,
                                                              SOPC_SKManager* skm)
{
    SOPC_ASSERT(NULL != skb && NULL != skp && NULL != skm);

    SOPC_ByteString* Keys = NULL;
    uint32_t NbKeys = 0;

    // TODO add a security group list in data and current token
    SOPC_ReturnStatus status =
        SOPC_SKProvider_GetKeys(skp, skm->securityGroupId, 0, UINT32_MAX, NULL, NULL, &Keys, &NbKeys, NULL, NULL);

    if (SOPC_STATUS_OK == status && NULL != Keys && 0 < NbKeys)
    {
        uint32_t addedKeys = SOPC_SKManager_AddKeys(skm, Keys, NbKeys);

        if (addedKeys != NbKeys)
        {
            status = SOPC_STATUS_NOK;
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "Security Key Builder: Error with SK Manager");
        }
        for (uint32_t i = 0; i < NbKeys; i++)
        {
            SOPC_ByteString_Clear(&Keys[i]);
        }
        SOPC_Free(Keys);
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "Security Key Builder: Error with SK Provider - cannot load keys");
    }

    return status;
}

typedef struct SOPC_SKBuilder_CallbackData
{
    SOPC_SKBuilder* skb;
    SOPC_SKBuilder_Callback_Func callback;
    uintptr_t userParam;
} SOPC_SKBuilder_CallbackData;

static SOPC_ReturnStatus SOPC_SKBuilder_Update_Callback(SOPC_SKBuilder* skb, SOPC_SKProvider* skp, SOPC_SKManager* skm)
{
    SOPC_ASSERT(NULL != skb && NULL != skb->data);

    SOPC_SKBuilder_CallbackData* data = (SOPC_SKBuilder_CallbackData*) skb->data;

    SOPC_ReturnStatus status = SOPC_SKBuilder_Update(data->skb, skp, skm);
    if (SOPC_STATUS_OK == status && NULL != data->callback)
    {
        data->callback(skb, skm, data->userParam);
    }

    return status;
}

static void SOPC_SKBuilder_Clear_Callback(void* data)
{
    SOPC_SKBuilder_CallbackData* callbackData = (SOPC_SKBuilder_CallbackData*) data;
    SOPC_SKBuilder_Clear(callbackData->skb);
    SOPC_Free(callbackData->skb);
}

/*** API FUNCTIONS ***/

SOPC_SKBuilder* SOPC_SKBuilder_Append_Create(void)
{
    SOPC_SKBuilder* skb = SOPC_Calloc(1, sizeof(SOPC_SKBuilder));
    if (NULL == skb)
    {
        return NULL;
    }

    skb->data = NULL;
    skb->referencesCounter = 0;

    skb->ptrUpdate = SOPC_SKBuilder_Update_Default_Append;
    skb->ptrClear = NULL;

    return skb;
}

SOPC_SKBuilder* SOPC_SKBuilder_Truncate_Create(SOPC_SKBuilder* skb, uint32_t sizeMax)
{
    SOPC_SKBuilder* result = SOPC_Calloc(1, sizeof(SOPC_SKBuilder));
    if (NULL == result)
    {
        return NULL;
    }

    result->data = SOPC_Calloc(1, sizeof(SOPC_SKBuilder_TruncateData));
    if (NULL == result->data)
    {
        SOPC_Free(result);
        return NULL;
    }

    SOPC_SKBuilder_TruncateData* data = (SOPC_SKBuilder_TruncateData*) result->data;

    data->skb = skb;
    data->sizeMax = sizeMax;

    result->ptrUpdate = SOPC_SKBuilder_Update_Truncate;
    result->ptrClear = SOPC_SKBuilder_Clear_Truncate;

    return result;
}

SOPC_SKBuilder* SOPC_SKBuilder_Setter_Create(void)
{
    SOPC_SKBuilder* skb = SOPC_Calloc(1, sizeof(SOPC_SKBuilder));
    if (NULL == skb)
    {
        return NULL;
    }

    skb->data = NULL;
    skb->referencesCounter = 0;

    skb->ptrUpdate = SOPC_SKBuilder_Update_Default_Setter;
    skb->ptrClear = NULL;

    return skb;
}

SOPC_SKBuilder* SOPC_SKBuilder_Callback_Create(SOPC_SKBuilder* skb,
                                               SOPC_SKBuilder_Callback_Func callback,
                                               uintptr_t userParam)
{
    SOPC_SKBuilder* result = SOPC_Calloc(1, sizeof(SOPC_SKBuilder));
    if (NULL == result)
    {
        return NULL;
    }

    result->data = SOPC_Calloc(1, sizeof(SOPC_SKBuilder_CallbackData));
    if (NULL == result->data)
    {
        SOPC_Free(result);
        return NULL;
    }

    SOPC_SKBuilder_CallbackData* data = (SOPC_SKBuilder_CallbackData*) result->data;

    data->skb = skb;
    data->callback = callback;
    data->userParam = userParam;

    result->ptrUpdate = SOPC_SKBuilder_Update_Callback;
    result->ptrClear = SOPC_SKBuilder_Clear_Callback;

    return result;
}

SOPC_ReturnStatus SOPC_SKBuilder_Update(SOPC_SKBuilder* skb, SOPC_SKProvider* skp, SOPC_SKManager* skm)
{
    if (NULL == skb || NULL == skb->ptrUpdate)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return skb->ptrUpdate(skb, skp, skm);
}

void SOPC_SKBuilder_Clear(SOPC_SKBuilder* skb)
{
    if (NULL != skb)
    {
        if (NULL != skb->ptrClear)
        {
            skb->ptrClear(skb->data);
        }
        SOPC_Free(skb->data);
        skb->data = NULL;
        skb->referencesCounter = 0;
    }
}

void SOPC_SKBuilder_MayDelete(SOPC_SKBuilder** skb)
{
    if (NULL == skb || NULL == *skb)
    {
        return;
    }
    SOPC_SKBuilder* builder = *skb;
    // Updates the counter if not already zero
    builder->referencesCounter = builder->referencesCounter > 0 ? builder->referencesCounter - 1 : 0;
    if (builder->referencesCounter == 0)
    {
        SOPC_SKBuilder_Clear(builder);
        SOPC_Free(builder);
        *skb = NULL;
    }
}
