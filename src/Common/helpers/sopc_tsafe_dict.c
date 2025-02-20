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

#include "sopc_tsafe_dict.h"
#include "sopc_assert.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"

struct _SOPC_TSafe_Dict
{
    SOPC_Mutex lock;
    SOPC_TSafe_Dict_KeyCopy_Fct* copy_func;
    SOPC_Dict* dict;
};

SOPC_TSafe_Dict* SOPC_TSafe_Dict_Create(uintptr_t empty_key,
                                        SOPC_Dict_KeyHash_Fct* key_hash,
                                        SOPC_Dict_KeyEqual_Fct* key_equal,
                                        SOPC_TSafe_Dict_KeyCopy_Fct* key_copy,
                                        SOPC_Dict_Free_Fct* key_free,
                                        SOPC_Dict_Free_Fct* value_free)
{
    SOPC_TSafe_Dict* safeDict = SOPC_Calloc(1, sizeof(SOPC_TSafe_Dict));
    if (NULL == safeDict)
    {
        return NULL;
    }
    SOPC_ReturnStatus status = SOPC_Mutex_Initialization(&safeDict->lock);
    if (SOPC_STATUS_OK == status)
    {
        safeDict->copy_func = key_copy;

        safeDict->dict = SOPC_Dict_Create(empty_key, key_hash, key_equal, key_free, value_free);
        if (NULL == safeDict->dict)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_TSafe_Dict_Delete(safeDict);
        safeDict = NULL;
    }
    return safeDict;
}

void SOPC_TSafe_Dict_Delete(SOPC_TSafe_Dict* d)
{
    if (NULL != d)
    {
        SOPC_Dict_Delete(d->dict);
        SOPC_Mutex_Clear(&d->lock);
        SOPC_Free(d);
    }
}

bool SOPC_TSafe_Dict_Reserve(SOPC_TSafe_Dict* d, size_t n_items)
{
    if (NULL == d)
    {
        return false;
    }
    SOPC_Mutex_Lock(&d->lock);
    bool res = SOPC_Dict_Reserve(d->dict, n_items);
    SOPC_Mutex_Unlock(&d->lock);
    return res;
}

void SOPC_TSafe_Dict_SetTombstoneKey(SOPC_TSafe_Dict* d, uintptr_t tombstone_key)
{
    SOPC_ASSERT(NULL != d);
    SOPC_Mutex_Lock(&d->lock);
    SOPC_Dict_SetTombstoneKey(d->dict, tombstone_key);
    SOPC_Mutex_Unlock(&d->lock);
}

bool SOPC_TSafe_Dict_Insert(SOPC_TSafe_Dict* d, uintptr_t key, uintptr_t value)
{
    if (NULL == d)
    {
        return false;
    }
    SOPC_Mutex_Lock(&d->lock);
    bool res = SOPC_Dict_Insert(d->dict, key, value);
    SOPC_Mutex_Unlock(&d->lock);
    return res;
}

uintptr_t SOPC_TSafe_Dict_GetAndLock(SOPC_TSafe_Dict* d, const uintptr_t key, bool* found)
{
    if (NULL == d)
    {
        return (uintptr_t) NULL;
    }
    SOPC_Mutex_Lock(&d->lock);
    uintptr_t result = SOPC_Dict_Get(d->dict, key, found);
    return result;
}

uintptr_t SOPC_TSafe_Dict_GetCopy(SOPC_TSafe_Dict* d, const uintptr_t key, bool* found)
{
    if (NULL == d || NULL == d->copy_func)
    {
        return (uintptr_t) NULL;
    }
    uintptr_t copy = (uintptr_t) NULL;
    SOPC_Mutex_Lock(&d->lock);
    uintptr_t result = SOPC_Dict_Get(d->dict, key, found);
    if ((uintptr_t) NULL != result)
    {
        copy = d->copy_func(result);
    }
    SOPC_Mutex_Unlock(&d->lock);
    return copy;
}

uintptr_t SOPC_TSafe_Dict_GetKeyAndLock(SOPC_TSafe_Dict* d, const uintptr_t key, bool* found)
{
    if (NULL == d)
    {
        return (uintptr_t) NULL;
    }
    SOPC_Mutex_Lock(&d->lock);
    uintptr_t result = SOPC_Dict_GetKey(d->dict, key, found);
    return result;
}

void SOPC_TSafe_Dict_Unlock(SOPC_TSafe_Dict* d)
{
    SOPC_ASSERT(NULL != d);
    SOPC_Mutex_Unlock(&d->lock);
}

void SOPC_TSafe_Dict_Remove(SOPC_TSafe_Dict* d, const uintptr_t key)
{
    SOPC_ASSERT(NULL != d);
    SOPC_Mutex_Lock(&d->lock);
    SOPC_Dict_Remove(d->dict, key);
    SOPC_Mutex_Unlock(&d->lock);
}

SOPC_Dict_Free_Fct* SOPC_TSafe_Dict_GetKeyFreeFunc(SOPC_TSafe_Dict* d)
{
    if (NULL == d)
    {
        return NULL;
    }
    SOPC_Mutex_Lock(&d->lock);
    SOPC_Dict_Free_Fct* freeFct = SOPC_Dict_GetKeyFreeFunc(d->dict);
    SOPC_Mutex_Unlock(&d->lock);
    return freeFct;
}

void SOPC_TSafe_Dict_SetKeyFreeFunc(SOPC_TSafe_Dict* d, SOPC_Dict_Free_Fct* func)
{
    SOPC_ASSERT(NULL != d);
    SOPC_Mutex_Lock(&d->lock);
    SOPC_Dict_SetKeyFreeFunc(d->dict, func);
    SOPC_Mutex_Unlock(&d->lock);
}

SOPC_Dict_Free_Fct* SOPC_TSafe_Dict_GetValueFreeFunc(SOPC_TSafe_Dict* d)
{
    if (NULL == d)
    {
        return NULL;
    }
    SOPC_Mutex_Lock(&d->lock);
    SOPC_Dict_Free_Fct* func = SOPC_Dict_GetValueFreeFunc(d->dict);
    SOPC_Mutex_Unlock(&d->lock);
    return func;
}

void SOPC_TSafe_Dict_SetValueFreeFunc(SOPC_TSafe_Dict* d, SOPC_Dict_Free_Fct* func)
{
    SOPC_ASSERT(NULL != d);
    SOPC_Mutex_Lock(&d->lock);
    SOPC_Dict_SetValueFreeFunc(d->dict, func);
    SOPC_Mutex_Unlock(&d->lock);
}

size_t SOPC_TSafe_Dict_Size(SOPC_TSafe_Dict* d)
{
    if (NULL == d)
    {
        return 0;
    }
    SOPC_Mutex_Lock(&d->lock);
    size_t s = SOPC_Dict_Size(d->dict);
    SOPC_Mutex_Unlock(&d->lock);
    return s;
}

size_t SOPC_TSafe_Dict_Capacity(SOPC_TSafe_Dict* d)
{
    if (NULL == d)
    {
        return 0;
    }
    SOPC_Mutex_Lock(&d->lock);
    size_t s = SOPC_Dict_Capacity(d->dict);
    SOPC_Mutex_Unlock(&d->lock);
    return s;
}

void SOPC_TSafe_Dict_ForEach(SOPC_TSafe_Dict* d, SOPC_Dict_ForEach_Fct* func, uintptr_t user_data)
{
    SOPC_ASSERT(NULL != d);
    SOPC_Mutex_Lock(&d->lock);
    SOPC_Dict_ForEach(d->dict, func, user_data);
    SOPC_Mutex_Unlock(&d->lock);
}
