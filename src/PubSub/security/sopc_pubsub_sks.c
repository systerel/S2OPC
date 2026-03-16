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

#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pubsub_constants.h"
#include "sopc_pubsub_sks.h"
#include "sopc_sk_secu_group_managers.h"

void SOPC_PubSubSKS_Init(void)
{
    // Initialize the SK Managers for security groups
    SOPC_SK_SecurityGroup_Managers_Init();
}

void SOPC_PubSubSKS_Clear(void)
{
    // Clear the SK Managers for security groups
    SOPC_SK_SecurityGroup_Managers_Clear();
}

bool SOPC_PubSubSKS_AddSkManager(SOPC_SKManager* skm)
{
    if (NULL == skm || NULL == skm->securityGroupId)
    {
        return false;
    }

    SOPC_SKManager* prevSkManager = SOPC_SK_SecurityGroup_GetSkManager(skm->securityGroupId);
    bool result = SOPC_SK_SecurityGroup_SetSkManager(skm);
    if (result && NULL != prevSkManager)
    {
        // If a previous SK Manager exists, delete it
        SOPC_SKManager_Clear(prevSkManager);
        SOPC_Free(prevSkManager);
    }
    return result;
}

SOPC_PubSubSKS_Keys* SOPC_PubSubSKS_GetSecurityKeys(const char* securityGroupid, uint32_t tokenId)
{
    SOPC_PubSubSKS_Keys* returnedKeys = NULL;
    SOPC_PubSubSKS_GetUpdateSecurityKeys(securityGroupid, tokenId, &returnedKeys);
    return returnedKeys;
}

SOPC_PubSub_SecurityStatus SOPC_PubSubSKS_GetUpdateSecurityKeys(const char* securityGroupId,
                                                                uint32_t tokenId,
                                                                SOPC_PubSubSKS_Keys** securityGroup)
{
    SOPC_PubSubSKS_Keys* returnedKeys = NULL;

    // Get keys from manager
    SOPC_String* SecurityPolicyUri = NULL;
    uint32_t FirstTokenId = 0;
    SOPC_ByteString* Keys = NULL;
    uint32_t NbKeys = 0;
    uint64_t TimeToNextKey = 0;
    uint64_t KeyLifetime = 0;
    SOPC_SKManager* skm = SOPC_SK_SecurityGroup_GetSkManager(securityGroupId);
    SOPC_PubSub_SecurityStatus secuStatus = SOPC_PUBSUB_STATUS_SECURITY_OK;

    // Invalid arguments
    if (NULL == skm || NULL == securityGroup)
    {
        return SOPC_PUBSUB_STATUS_SECURITY_INVALID_PARAMETERS;
    }

    // First check if the SG token id is not already the one requested (still current one or still the one requested)
    if (*securityGroup != NULL && (SOPC_PUBSUB_SKS_CURRENT_TOKENID == tokenId || (*securityGroup)->tokenId == tokenId))
    {
        // Ask for 0 keys only to know if the token id requested is available
        SOPC_ReturnStatus status =
            SOPC_SKManager_GetKeys(skm, tokenId, 0, NULL, &FirstTokenId, &Keys, &NbKeys, &TimeToNextKey, &KeyLifetime);

        if (SOPC_STATUS_OK != status)
        {
            // SKmanager fail to get keys
            return SOPC_PUBSUB_STATUS_SECURITY_NOK;
        }
        else if ((*securityGroup)->tokenId == FirstTokenId)
        {
            // the token id that will be returned is the same as SG current token id
            // => nothing to do, SG already has the requested and up to date token
            return SOPC_PUBSUB_STATUS_SECURITY_OK;
        }
    }

    // If there is an available key to update in our SG, ask for this key (one key needed)
    SOPC_ReturnStatus status = SOPC_SKManager_GetKeys(skm, tokenId, 1, &SecurityPolicyUri, &FirstTokenId, &Keys,
                                                      &NbKeys, &TimeToNextKey, &KeyLifetime);

    if (SOPC_STATUS_OK != status)
    {
        // SKmanager fail to get keys
        secuStatus = SOPC_PUBSUB_STATUS_SECURITY_NOK;
    }
    else if (NbKeys <= 0)
    {
        // No keys available from SK Manager
        secuStatus = SOPC_PUBSUB_STATUS_SECURITY_NO_KEYS;
    }
    else
    {
        /** Check if received keys are different from the one in use */
        if (*securityGroup == NULL || (*securityGroup)->tokenId != FirstTokenId)
        {
            /** Fill Outputs **/

            // Initialize returned keys if GetKeys returned valid Keys corresponding to requested token
            SOPC_ByteString* byteString = NULL;
            if ((SOPC_PUBSUB_SKS_CURRENT_TOKENID == tokenId || tokenId == FirstTokenId))
            {
                byteString = &Keys[0];
                if ((32 + 32 + 4) == byteString->Length)
                {
                    returnedKeys = SOPC_Calloc(1, sizeof(SOPC_PubSubSKS_Keys));
                }
                secuStatus = NULL != returnedKeys ? SOPC_PUBSUB_STATUS_SECURITY_OK : SOPC_PUBSUB_STATUS_SECURITY_NOK;
            }
            else if (FirstTokenId < tokenId)
            {
                secuStatus = SOPC_PUBSUB_STATUS_SECURITY_SKS_KEYS_OLDER;
            }
            else // FirstTokenId > tokenId
            {
                // Received Keys are newer than the one required
                secuStatus = SOPC_PUBSUB_STATUS_SECURITY_SKS_KEYS_NEWER;
            }

            if (NULL != returnedKeys)
            {
                returnedKeys->tokenId = FirstTokenId;
                returnedKeys->signingKey = SOPC_SecretBuffer_NewFromExposedBuffer(byteString->Data, 32);
                returnedKeys->encryptKey = SOPC_SecretBuffer_NewFromExposedBuffer(&(byteString->Data[32]), 32);
                returnedKeys->keyNonce = SOPC_SecretBuffer_NewFromExposedBuffer(&(byteString->Data[64]), 4);
                if (NULL == returnedKeys->signingKey || NULL == returnedKeys->encryptKey ||
                    NULL == returnedKeys->keyNonce)
                {
                    SOPC_PubSubSKS_Keys_Delete(returnedKeys);
                    SOPC_Free(returnedKeys);
                    returnedKeys = NULL;
                    secuStatus = SOPC_PUBSUB_STATUS_SECURITY_NOK;
                }
            }
        }
    }

    SOPC_String_Clear(SecurityPolicyUri);
    SOPC_Free(SecurityPolicyUri);
    for (uint32_t i = 0; i < NbKeys && NULL != Keys; i++)
    {
        SOPC_ByteString_Clear(&Keys[i]);
    }
    SOPC_Free(Keys);

    /** Update the key with new one */
    if (NULL != returnedKeys)
    {
        if (*securityGroup != NULL)
        {
            SOPC_PubSubSKS_Keys_Delete(*securityGroup);
            SOPC_Free(*securityGroup);
        }
        (*securityGroup) = returnedKeys;
    }
    return secuStatus;
}

void SOPC_PubSubSKS_Keys_Delete(SOPC_PubSubSKS_Keys* keys)
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

static bool mayAddSecurityGroupId(const char* securityGroupId)
{
    SOPC_ASSERT(NULL != securityGroupId);
    bool result = true;
    if (NULL == SOPC_SK_SecurityGroup_GetSkManager(securityGroupId))
    {
        SOPC_SKManager* skm = SOPC_SKManager_Create(securityGroupId, 0);

        if (NULL != skm)
        {
            result = SOPC_PubSubSKS_AddSkManager(skm);
        }
        else
        {
            SOPC_SKManager_Clear(skm);
            SOPC_Free(skm);
            result = false;
        }
    }
    return result;
}

bool SOPC_PubSubSKS_CreateManagersFromConfig(const SOPC_PubSubConfiguration* pubSubConfig)
{
    if (NULL == pubSubConfig)
    {
        return false;
    }

    bool result = true;

    // Process publisher connections
    for (uint32_t i = 0; result && i < SOPC_PubSubConfiguration_Nb_PubConnection(pubSubConfig); i++)
    {
        const SOPC_PubSubConnection* pubConnection = SOPC_PubSubConfiguration_Get_PubConnection_At(pubSubConfig, i);

        // Check writer groups
        for (uint16_t j = 0; result && j < SOPC_PubSubConnection_Nb_WriterGroup(pubConnection); j++)
        {
            const SOPC_WriterGroup* group = SOPC_PubSubConnection_Get_WriterGroup_At(pubConnection, j);
            const char* securityGroupId = SOPC_WriterGroup_Get_SecurityGroupId(group);
            const SOPC_SecurityMode_Type secuMode = SOPC_WriterGroup_Get_SecurityMode(group);

            if (NULL == securityGroupId && SOPC_SecurityMode_None != secuMode)
            {
                result = false;
            }
            else if (SOPC_SecurityMode_None != secuMode)
            {
                // Ensure the security group id is added to the SKManager
                result = mayAddSecurityGroupId(securityGroupId);
            }
        }
    }

    // Process subscriber connections
    for (uint32_t i = 0; result && i < SOPC_PubSubConfiguration_Nb_SubConnection(pubSubConfig); i++)
    {
        const SOPC_PubSubConnection* subConnection = SOPC_PubSubConfiguration_Get_SubConnection_At(pubSubConfig, i);

        // Check reader groups
        for (uint16_t j = 0; result && j < SOPC_PubSubConnection_Nb_ReaderGroup(subConnection); j++)
        {
            const SOPC_ReaderGroup* group = SOPC_PubSubConnection_Get_ReaderGroup_At(subConnection, j);
            const char* securityGroupId = SOPC_ReaderGroup_Get_SecurityGroupId(group);
            const SOPC_SecurityMode_Type secuMode = SOPC_ReaderGroup_Get_SecurityMode(group);

            if (NULL == securityGroupId && SOPC_SecurityMode_None != secuMode)
            {
                result = false;
            }
            else if (SOPC_SecurityMode_None != secuMode)
            {
                // Ensure the security group id is added to the SKManager
                result = mayAddSecurityGroupId(securityGroupId);
            }
        }
    }

    return result;
}

typedef struct
{
    SOPC_SKscheduler* scheduler;
    SOPC_SKBuilder* builder;
    SOPC_SKProvider* provider;
    uint32_t msPeriod;
    bool result;
} AddTaskContext;

static void addTask(const char* securityGroupId, SOPC_SKManager* manager, void* userData)
{
    SOPC_UNUSED_ARG(securityGroupId);
    AddTaskContext* ctx = (AddTaskContext*) userData;
    if (!ctx->result)
    {
        return; // Stop on first error
    }

    SOPC_ReturnStatus status =
        SOPC_SKscheduler_AddTask(ctx->scheduler, ctx->builder, ctx->provider, manager, ctx->msPeriod);
    ctx->result = (SOPC_STATUS_OK == status);
}

bool SOPC_PubSubSKS_AddTasks(SOPC_SKscheduler* scheduler,
                             SOPC_SKBuilder* builder,
                             SOPC_SKProvider* provider,
                             uint32_t msFirstUpdate)
{
    if (NULL == scheduler || NULL == builder || NULL == provider)
    {
        return false;
    }

    AddTaskContext context = {
        .scheduler = scheduler, .builder = builder, .provider = provider, .msPeriod = msFirstUpdate, .result = true};

    SOPC_SecurityGroup_ForEach(addTask, &context);

    return context.result;
}

SOPC_SKManager* SOPC_PubSubSKS_GetSkManager(const char* securityGroupId)
{
    if (NULL == securityGroupId)
    {
        return NULL;
    }
    return SOPC_SK_SecurityGroup_GetSkManager(securityGroupId);
}
