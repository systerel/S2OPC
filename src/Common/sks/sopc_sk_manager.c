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

#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_sk_manager.h"
#include "sopc_time_reference.h"

/*** DEFAULT IMPLEMENTATION FUNCTIONS ***/

typedef struct SOPC_SKManager_DefaultData
{
    SOPC_Mutex mutex;
    /* Current token id. To be update in getter
       This 3 values are linked and should be changed together.
    */
    uint32_t CurrentTokenId;
    SOPC_TimeReference CurrentTokenTime; /* Time when current token was set */
    uint32_t CurrentTokenRemainingTime;  /* remaining time for current token */

    SOPC_Array* Keys; // array of SOPC_ByteString

    SOPC_String* SecurityPolicyUri;
    uint32_t FirstTokenId;

    uint32_t TimeToNextKey;
    uint32_t KeyLifetime;

} SOPC_SKManager_DefaultData;

static uint32_t SOPC_SKManager_Size_Default(SOPC_SKManager* skm)
{
    if (NULL == skm || NULL == skm->data)
    {
        return 0;
    }

    SOPC_SKManager_DefaultData* data = (SOPC_SKManager_DefaultData*) skm->data;
    SOPC_Mutex_Lock(&data->mutex);

    if (NULL == data->Keys)
    {
        SOPC_Mutex_Unlock(&data->mutex);
        return 0;
    }

    size_t size = SOPC_Array_Size(data->Keys);

    SOPC_Mutex_Unlock(&data->mutex);

    SOPC_ASSERT(size <= UINT32_MAX); /* TODO check when add keys */
    return (uint32_t) size;
}

static SOPC_ReturnStatus SOPC_SKManager_SetSecurityPolicyUri_Default(SOPC_SKManager* skm,
                                                                     SOPC_String* SecurityPolicyUri)
{
    /* Check Parameter */
    if (NULL == skm || NULL == skm->data || NULL == SecurityPolicyUri)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus status;
    SOPC_SKManager_DefaultData* data = (SOPC_SKManager_DefaultData*) skm->data;
    SOPC_Mutex_Lock(&data->mutex);
    data->SecurityPolicyUri = SOPC_Calloc(1, sizeof(SOPC_String));
    if (NULL == data->SecurityPolicyUri)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    else
    {
        SOPC_String_Initialize(data->SecurityPolicyUri);
        status = SOPC_String_Copy(data->SecurityPolicyUri, SecurityPolicyUri);
    }

    SOPC_Mutex_Unlock(&data->mutex);
    return status;
}

static void SOPC_SKManager_InitKeys_Default(SOPC_SKManager_DefaultData* data, uint32_t initialSize)
{
    if (NULL == data)
    {
        return;
    }

    SOPC_Array_Delete(data->Keys);
    data->Keys = SOPC_Array_Create(sizeof(SOPC_ByteString), initialSize, SOPC_ByteString_ClearAux);
    data->FirstTokenId = 0;
    data->TimeToNextKey = 0;
    data->KeyLifetime = SOPC_SK_MANAGER_DEFAULT_KEYLIFETIME;
    data->CurrentTokenId = 0;
    data->CurrentTokenTime = 0;
    data->CurrentTokenRemainingTime = 0;
}

static SOPC_ReturnStatus SOPC_SKManager_SetKeys_Default(SOPC_SKManager* skm,
                                                        const SOPC_String* SecurityPolicyUri,
                                                        uint32_t FirstTokenId,
                                                        const SOPC_ByteString* Keys,
                                                        uint32_t NbToken,
                                                        uint32_t TimeToNextKey,
                                                        uint32_t KeyLifetime)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == skm || NULL == skm->data || NULL == SecurityPolicyUri || NULL == Keys || NbToken <= 0 ||
        SOPC_SK_MANAGER_CURRENT_TOKEN_ID == FirstTokenId || 0 == KeyLifetime)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SKManager_DefaultData* data = (SOPC_SKManager_DefaultData*) skm->data;
    SOPC_Mutex_Lock(&data->mutex);

    /* Do not keep old keys */
    SOPC_SKManager_InitKeys_Default(data, NbToken * 2);
    SOPC_String_Delete(data->SecurityPolicyUri);
    data->SecurityPolicyUri = NULL;

    if (NULL == data->Keys)
    {
        SOPC_Mutex_Unlock(&data->mutex);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    for (size_t i = 0; i < NbToken && SOPC_STATUS_OK == status; i++)
    {
        SOPC_ByteString byteString;
        SOPC_ByteString_Initialize(&byteString);
        status = SOPC_ByteString_Copy(&byteString, &Keys[i]);
        if (SOPC_STATUS_OK == status)
        {
            bool success = SOPC_Array_Append(data->Keys, byteString);
            status = success ? status : SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        data->SecurityPolicyUri = SOPC_Calloc(1, sizeof(SOPC_String));
        if (NULL == data->SecurityPolicyUri)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            SOPC_String_Initialize(data->SecurityPolicyUri);
            status = SOPC_String_Copy(data->SecurityPolicyUri, SecurityPolicyUri);
        }
    }

    /* Delete allocated data if bad status */
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Array_Delete(data->Keys);
        SOPC_String_Delete(data->SecurityPolicyUri);
        data->SecurityPolicyUri = NULL;
    }

    /* Otherwise fill data object */
    if (SOPC_STATUS_OK == status)
    {
        data->FirstTokenId = FirstTokenId;
        data->TimeToNextKey = TimeToNextKey;
        data->KeyLifetime = KeyLifetime;

        data->CurrentTokenId = FirstTokenId;
        data->CurrentTokenTime = SOPC_TimeReference_GetCurrent();
        data->CurrentTokenRemainingTime = TimeToNextKey;
    }

    SOPC_Mutex_Unlock(&data->mutex);
    return status;
}

static uint32_t SOPC_SKManager_AddKeys_Default(SOPC_SKManager* skm, SOPC_ByteString* Keys, uint32_t NbToken)
{
    /* Check Parameter */
    if (NULL == skm || NULL == skm->data || NbToken <= 0)
    {
        return 0;
    }

    SOPC_SKManager_DefaultData* data = (SOPC_SKManager_DefaultData*) skm->data;
    SOPC_Mutex_Lock(&data->mutex);
    uint32_t nbAdded = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    for (size_t i = 0; i < NbToken && SOPC_STATUS_OK == status; i++)
    {
        SOPC_ByteString byteString;
        SOPC_ByteString_Initialize(&byteString);
        status = SOPC_ByteString_Copy(&byteString, &Keys[i]);
        if (SOPC_STATUS_OK == status)
        {
            bool bres = SOPC_Array_Append(data->Keys, byteString);
            if (bres)
            {
                nbAdded++;
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
        }
    }

    // If there was no token before this call, set First Token
    if (0 < nbAdded && 0 == data->FirstTokenId)
    {
        data->FirstTokenId = 1;
        data->CurrentTokenId = 1;
        data->CurrentTokenTime = SOPC_TimeReference_GetCurrent();
        data->CurrentTokenRemainingTime = data->KeyLifetime;
        data->TimeToNextKey = data->CurrentTokenRemainingTime;
    }

    SOPC_Mutex_Unlock(&data->mutex);
    return nbAdded;
}

static void SOPC_SKManager_UpdateCurrentToken_Default(SOPC_SKManager_DefaultData* data)
{
    SOPC_ASSERT(NULL != data);
    if (0 == data->CurrentTokenId)
    {
        return;
    }

    SOPC_TimeReference currentTime = SOPC_TimeReference_GetCurrent();
    SOPC_TimeReference timeElapsed = currentTime - data->CurrentTokenTime;
    SOPC_TimeReference overrunTime = 0;

    if (timeElapsed < data->CurrentTokenRemainingTime)
    {
        // current token is still the one to use
        data->CurrentTokenRemainingTime = data->CurrentTokenRemainingTime - (uint32_t) timeElapsed;
        data->CurrentTokenTime = currentTime;
    }
    else
    {
        // current token is obsolete. Find the next to use

        // compute overrun time for the current token after its remaining time
        overrunTime = timeElapsed - data->CurrentTokenRemainingTime;

        // Number of Next Token Id obsolete
        SOPC_ASSERT(0 < data->KeyLifetime);
        uint64_t nbRenewedTokenMissed = overrunTime / data->KeyLifetime;
        // Token Id are incremented by 1.

        SOPC_TimeReference newCurrentTokenId = (data->CurrentTokenId + nbRenewedTokenMissed + 1) % UINT32_MAX;
        // If the CurrentTokenId increments past the maximum value of UInt32 it restarts a 1.
        newCurrentTokenId = (0 == newCurrentTokenId ? 1 : newCurrentTokenId);
        data->CurrentTokenId = (uint32_t) newCurrentTokenId;
        data->CurrentTokenTime = currentTime;
        /* Substract overrun time from next remaining time */
        data->CurrentTokenRemainingTime = (uint32_t)(data->KeyLifetime - (overrunTime % data->KeyLifetime));
        SOPC_ASSERT(data->CurrentTokenRemainingTime <= data->KeyLifetime);
    }
}

static uint32_t SOPC_SKManager_GetAllKeysLifeTime_Default(SOPC_SKManager* skm)
{
    if (NULL == skm || NULL == skm->data)
    {
        return 0;
    }

    SOPC_SKManager_DefaultData* data = (SOPC_SKManager_DefaultData*) skm->data;
    SOPC_Mutex_Lock(&data->mutex);

    if (NULL == data->Keys)
    {
        SOPC_Mutex_Unlock(&data->mutex);
        return 0;
    }

    SOPC_SKManager_UpdateCurrentToken_Default(data);
    if (SOPC_SK_MANAGER_CURRENT_TOKEN_ID == data->CurrentTokenId)
    {
        SOPC_Mutex_Unlock(&data->mutex);
        return 0;
    }

    // Nb available = Number of stored keys - already used keys
    uint32_t nbAvailable = SOPC_SKManager_Size(skm) - (data->CurrentTokenId - data->FirstTokenId);

    uint32_t result = UINT32_MAX;
    if (data->CurrentTokenRemainingTime <= UINT32_MAX - (data->KeyLifetime * (nbAvailable - 1)))
    {
        // Current key remaining time + next available keys lifetime
        result = data->CurrentTokenRemainingTime + (data->KeyLifetime * (nbAvailable - 1));
    } // else: available lifetime > UINT32_MAX => return UINT32_MAX

    SOPC_Mutex_Unlock(&data->mutex);

    return result;
}

static SOPC_ReturnStatus SOPC_SKManager_SetKeyLifetime_Default(SOPC_SKManager* skm, uint32_t KeyLifetime)
{
    /* Check Parameter */
    if (NULL == skm || NULL == skm->data || 0 == KeyLifetime)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SKManager_DefaultData* data = (SOPC_SKManager_DefaultData*) skm->data;
    SOPC_Mutex_Lock(&data->mutex);
    data->KeyLifetime = KeyLifetime;
    SOPC_Mutex_Unlock(&data->mutex);
    return SOPC_STATUS_OK;
}

/**
 * \brief GetKeys function for SK Manager default implementation
 *
 */
static SOPC_ReturnStatus SOPC_SKManager_GetKeys_Default(SOPC_SKManager* skm,
                                                        uint32_t StartingTokenId,
                                                        uint32_t NbRequestedToken,
                                                        SOPC_String** SecurityPolicyUri,
                                                        uint32_t* FirstTokenId,
                                                        SOPC_ByteString** Keys,
                                                        uint32_t* NbToken,
                                                        uint32_t* TimeToNextKey,
                                                        uint32_t* KeyLifetime)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Check Parameter */
    if (NULL == skm || NULL == skm->data || NULL == SecurityPolicyUri || NULL == FirstTokenId || NULL == Keys ||
        NULL == NbToken || NULL == TimeToNextKey || NULL == KeyLifetime || 0 == NbRequestedToken)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SKManager_DefaultData* data = (SOPC_SKManager_DefaultData*) skm->data;
    SOPC_Mutex_Lock(&data->mutex);
    if (NULL == data->Keys)
    {
        /* Keys array should be initialized */
        SOPC_Mutex_Unlock(&data->mutex);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_SK_MANAGER_CURRENT_TOKEN_ID == data->FirstTokenId)
    {
        /* Return empty if no keys are managed */
        SOPC_Mutex_Unlock(&data->mutex);
        *SecurityPolicyUri = NULL;
        *FirstTokenId = 0;
        *Keys = NULL;
        *NbToken = 0;
        *TimeToNextKey = 0;
        *KeyLifetime = 0;
        return SOPC_STATUS_OK;
    }

    size_t size = SOPC_Array_Size(data->Keys);
    SOPC_ASSERT(size <= UINT32_MAX); /* TODO check when add keys */

    uint32_t nbManagedKeys = (uint32_t) size;

    SOPC_SKManager_UpdateCurrentToken_Default(data);

    /* Requested Token Id is the current Token Id */
    if (SOPC_SK_MANAGER_CURRENT_TOKEN_ID == StartingTokenId || data->CurrentTokenId == StartingTokenId)
    {
        // Return the current Token ID
        *FirstTokenId = data->CurrentTokenId;
        *TimeToNextKey = data->CurrentTokenRemainingTime;
        *KeyLifetime = data->KeyLifetime;
    }
    else if (data->FirstTokenId > StartingTokenId || StartingTokenId - data->FirstTokenId > nbManagedKeys)
    {
        /* Requested Token Id is before the first managed Token Id or after the last managed Token Id */
        // Requested Token id is unkown. Get Current Token ID
        *FirstTokenId = data->CurrentTokenId;
        *TimeToNextKey = data->CurrentTokenRemainingTime;
        *KeyLifetime = data->KeyLifetime;
    }
    else
    {
        // Requested Token id is after managed Starting Token ID. Take requested Token ID
        *FirstTokenId = StartingTokenId;
        *TimeToNextKey = 0;
        *KeyLifetime = data->KeyLifetime;
    }

    if (*FirstTokenId < data->FirstTokenId /* Token ID goes to the limit. This case is not managed */
        || *FirstTokenId - data->FirstTokenId > nbManagedKeys /* There is not enough keys */)
    {
        status = SOPC_STATUS_NOK;
    }

    // Copy SecurityPolicyUri
    if (SOPC_STATUS_OK == status)
    {
        *SecurityPolicyUri = SOPC_String_Create();
        if (NULL != *SecurityPolicyUri)
        {
            SOPC_String_Initialize(*SecurityPolicyUri);
            status = SOPC_String_Copy(*SecurityPolicyUri, data->SecurityPolicyUri);
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        // *FirstTokenId is already set;

        // index of the first token to return in keys array
        // Token Id are increased by 1
        uint32_t indexOfFirstToken = *FirstTokenId - data->FirstTokenId;

        *NbToken = nbManagedKeys - indexOfFirstToken;
        if (*NbToken > NbRequestedToken)
        {
            *NbToken = NbRequestedToken;
        }

        if (0 < *NbToken)
        {
            *Keys = SOPC_Calloc(*NbToken, sizeof(SOPC_ByteString));
            if (NULL == *Keys)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }

            // Copy Keys
            for (size_t i = 0; i < *NbToken && SOPC_STATUS_OK == status; i++)
            {
                size_t indexInManaged = i + indexOfFirstToken;
                void* byteString = SOPC_Array_Get_Ptr(data->Keys, indexInManaged);
                SOPC_ASSERT(NULL != byteString);
                SOPC_ByteString_Initialize(&(*Keys)[i]);
                status = SOPC_ByteString_CopyAux(&(*Keys)[i], byteString);
            }
        }
        else
        {
            *Keys = NULL;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_String_Delete(*SecurityPolicyUri);
        *SecurityPolicyUri = NULL;

        for (size_t i = 0; i < *NbToken; i++)
        {
            SOPC_ByteString_Clear(&(*Keys)[i]);
        }
        SOPC_Free(*Keys);
        *Keys = NULL;

        *FirstTokenId = 0;
        *NbToken = 0;
        *TimeToNextKey = 0;
        *KeyLifetime = 0;
    }

    SOPC_Mutex_Unlock(&data->mutex);
    return status;
}

static void SOPC_SKManager_Clear_Default(SOPC_SKManager* skm)
{
    if (NULL == skm || NULL == skm->data)
    {
        return;
    }

    SOPC_SKManager_DefaultData* data = (SOPC_SKManager_DefaultData*) skm->data;

    SOPC_Array_Delete(data->Keys);
    data->Keys = NULL;
    SOPC_String_Delete(data->SecurityPolicyUri);
    data->SecurityPolicyUri = NULL;
    data->FirstTokenId = 0;
    data->TimeToNextKey = 0;
    data->KeyLifetime = SOPC_SK_MANAGER_DEFAULT_KEYLIFETIME;
    data->CurrentTokenId = 0;
    data->CurrentTokenTime = 0;

    SOPC_Mutex_Clear(&data->mutex);

    SOPC_Free(skm->data);
    skm->data = NULL;
}

/*** API FUNCTIONS ***/

SOPC_SKManager* SOPC_SKManager_Create(const char* securityGroupId, uintptr_t userData)
{
    SOPC_SKManager* skm = SOPC_Malloc(sizeof(SOPC_SKManager));
    if (NULL == skm)
    {
        return NULL;
    }

    skm->securityGroupId = SOPC_strdup(securityGroupId);
    skm->userData = userData;
    skm->data = SOPC_Calloc(1, sizeof(SOPC_SKManager_DefaultData));
    if (NULL == skm->securityGroupId || NULL == skm->data)
    {
        SOPC_Free(skm->securityGroupId);
        SOPC_Free(skm->data);
        SOPC_Free(skm);
        return NULL;
    }

    SOPC_SKManager_DefaultData* data = (SOPC_SKManager_DefaultData*) skm->data;

    data->Keys =
        SOPC_Array_Create(sizeof(SOPC_ByteString), SOPC_SK_MANAGER_DEFAULT_INITIAL_SIZE, SOPC_ByteString_ClearAux);

    if (NULL == data->Keys)
    {
        SOPC_Free(skm->data);
        SOPC_Free(skm);
        return NULL;
    }

    SOPC_Mutex_Initialization(&data->mutex);
    data->CurrentTokenId = 0;
    data->CurrentTokenTime = 0;
    data->CurrentTokenRemainingTime = 0;
    data->SecurityPolicyUri = NULL;
    data->FirstTokenId = 0;
    data->TimeToNextKey = 0;
    data->KeyLifetime = SOPC_SK_MANAGER_DEFAULT_KEYLIFETIME;

    skm->ptrSize = SOPC_SKManager_Size_Default;
    skm->ptrSetKeyLifetime = SOPC_SKManager_SetKeyLifetime_Default;
    skm->ptrSetPolicy = SOPC_SKManager_SetSecurityPolicyUri_Default;
    skm->ptrSetKeys = SOPC_SKManager_SetKeys_Default;
    skm->ptrAddKeys = SOPC_SKManager_AddKeys_Default;
    skm->ptrGetKeys = SOPC_SKManager_GetKeys_Default;
    skm->ptrGetAllKeysLifeTime = SOPC_SKManager_GetAllKeysLifeTime_Default;
    skm->ptrClear = SOPC_SKManager_Clear_Default;

    return skm;
}

uint32_t SOPC_SKManager_Size(SOPC_SKManager* skm)
{
    if (NULL == skm)
    {
        return 0;
    }
    return skm->ptrSize(skm);
}

SOPC_ReturnStatus SOPC_SKManager_SetKeyLifetime(SOPC_SKManager* skm, uint32_t KeyLifetime)
{
    if (NULL == skm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return skm->ptrSetKeyLifetime(skm, KeyLifetime);
}

SOPC_ReturnStatus SOPC_SKManager_SetSecurityPolicyUri(SOPC_SKManager* skm, SOPC_String* SecurityPolicyUri)
{
    if (NULL == skm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return skm->ptrSetPolicy(skm, SecurityPolicyUri);
}

SOPC_ReturnStatus SOPC_SKManager_SetKeys(SOPC_SKManager* skm,
                                         const SOPC_String* SecurityPolicyUri,
                                         uint32_t FirstTokenId,
                                         const SOPC_ByteString* Keys,
                                         uint32_t NbKeys,
                                         uint32_t TimeToNextKey,
                                         uint32_t KeyLifetime)
{
    if (NULL == skm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return skm->ptrSetKeys(skm, SecurityPolicyUri, FirstTokenId, Keys, NbKeys, TimeToNextKey, KeyLifetime);
}

uint32_t SOPC_SKManager_AddKeys(SOPC_SKManager* skm, SOPC_ByteString* Keys, uint32_t NbToken)
{
    if (NULL == skm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return skm->ptrAddKeys(skm, Keys, NbToken);
}

SOPC_ReturnStatus SOPC_SKManager_GetKeys(SOPC_SKManager* skm,
                                         uint32_t StartingTokenId,
                                         uint32_t NbRequestedToken,
                                         SOPC_String** SecurityPolicyUri,
                                         uint32_t* FirstTokenId,
                                         SOPC_ByteString** Keys,
                                         uint32_t* NbKeys,
                                         uint32_t* TimeToNextKey,
                                         uint32_t* KeyLifetime)
{
    if (NULL == skm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return skm->ptrGetKeys(skm, StartingTokenId, NbRequestedToken, SecurityPolicyUri, FirstTokenId, Keys, NbKeys,
                           TimeToNextKey, KeyLifetime);
}

uint32_t SOPC_SKManager_GetAllKeysLifeTime(SOPC_SKManager* skm)
{
    if (NULL == skm)
    {
        return 0;
    }
    return skm->ptrGetAllKeysLifeTime(skm);
}

void SOPC_SKManager_Clear(SOPC_SKManager* skm)
{
    if (NULL != skm && NULL != skm->ptrClear)
    {
        skm->ptrClear(skm);
        SOPC_Free(skm->securityGroupId);
        skm->securityGroupId = NULL;
    }
}
