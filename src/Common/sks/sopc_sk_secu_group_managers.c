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
#include <string.h>

#include "sopc_sk_secu_group_managers.h"

#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_dict.h"
#include "sopc_hash.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"

static const uintptr_t DICT_TOMBSTONE = UINTPTR_MAX;

static SOPC_Dict* g_dict_skManagers = NULL; // char* securityGroupId <-> SOPC_SKManager* skm
// Mutex to protect access to dictionary
SOPC_Mutex g_mutex;
// indicate this service is initialized
bool g_init = false;

static uint64_t str_hash(const uintptr_t data)
{
    return SOPC_DJBHash((const uint8_t*) data, strlen((const char*) data));
}

static bool str_equal(const uintptr_t a, const uintptr_t b)
{
    return strcmp((const char*) a, (const char*) b) == 0;
}

static void uintptr_t_free(uintptr_t elt)
{
    SOPC_Free((void*) elt);
}

static void sk_manager_free(uintptr_t elt)
{
    SOPC_SKManager* skm = (SOPC_SKManager*) elt;
    if (NULL != skm)
    {
        SOPC_SKManager_Clear(skm);
        SOPC_Free(skm);
    }
}

void SOPC_SK_SecurityGroup_Managers_Init(void)
{
    if (g_init || NULL != g_dict_skManagers)
    {
        return;
    }
    SOPC_Mutex_Initialization(&g_mutex);
    g_dict_skManagers = SOPC_Dict_Create((uintptr_t) NULL, str_hash, str_equal, uintptr_t_free, sk_manager_free);
    if (NULL == g_dict_skManagers)
    {
        SOPC_Mutex_Clear(&g_mutex);
        return; // OOM occurred
    }
    SOPC_Dict_SetTombstoneKey(g_dict_skManagers, DICT_TOMBSTONE);
    g_init = true;
}

void SOPC_SK_SecurityGroup_Managers_Clear(void)
{
    if (!g_init)
    {
        return;
    }
    SOPC_Dict_Delete(g_dict_skManagers);
    g_dict_skManagers = NULL;
    SOPC_Mutex_Clear(&g_mutex);
    g_init = false;
}

void SOPC_SK_SecurityGroup_SetSkManager(const char* securityGroupid, SOPC_SKManager* skm)
{
    if (!g_init || NULL == securityGroupid || NULL == skm || (uintptr_t) securityGroupid == DICT_TOMBSTONE)
    {
        // Do not set a SKManager if the service is not initialized or if parameters are invalid
        return;
    }
    char* secuGroupId = SOPC_strdup(securityGroupid);
    if (NULL == secuGroupId)
    {
        return;
    }
    SOPC_Mutex_Lock(&g_mutex);
    bool res = SOPC_Dict_Insert(g_dict_skManagers, (uintptr_t) secuGroupId, (uintptr_t) skm);
    SOPC_ASSERT(res); // shall not fail as we checked parameters previously
    SOPC_Mutex_Unlock(&g_mutex);
}

SOPC_SKManager* SOPC_SK_SecurityGroup_GetSkManager(const char* securityGroupid)
{
    if (!g_init || NULL == securityGroupid)
    {
        // Do not get a SKManager if the service is not initialized or if parameters are invalid
        return NULL;
    }
    return (SOPC_SKManager*) SOPC_Dict_Get(g_dict_skManagers, (const uintptr_t) securityGroupid, NULL);
}
