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

/**
 * \file util_write_value_pool.c
 * \brief Implementation of the shared WriteValue static pool for data-changed events.
 */

#include "util_write_value_pool.h"

#include "sopc_static_pool.h"
#include "sopc_toolkit_config_constants.h"
#include "util_fast_init_clear.h"

static OpcUa_WriteValue sopc_writeValuePoolBuffer[SOPC_WRITE_VALUES_POOL_SIZE];
static SOPC_StaticPool sopc_writeValuePool;

void SOPC_WriteValuePool_Initialize(void)
{
    SOPC_StaticPool_Init(&sopc_writeValuePool, sopc_writeValuePoolBuffer, sizeof(OpcUa_WriteValue),
                         SOPC_WRITE_VALUES_POOL_SIZE);
}

OpcUa_WriteValue* SOPC_WriteValuePool_Alloc(void)
{
    OpcUa_WriteValue* wv = (OpcUa_WriteValue*) SOPC_StaticPool_Alloc(&sopc_writeValuePool);
    if (NULL != wv)
    {
        SOPC_WriteValue_FastInitialize(wv);
    }
    return wv;
}

void SOPC_WriteValuePool_ClearAndFree(OpcUa_WriteValue* wv)
{
    if (NULL == wv)
    {
        return;
    }
    SOPC_WriteValue_FastFreeContent(wv);
    SOPC_StaticPool_Free(&sopc_writeValuePool, wv);
}

size_t SOPC_WriteValuePool_GetNbFallbackAllocsStat(void)
{
    return SOPC_StaticPool_GetNbFallbackAllocsStat(&sopc_writeValuePool);
}
