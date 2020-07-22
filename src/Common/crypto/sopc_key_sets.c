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

#include <stddef.h>

#include "sopc_key_sets.h"
#include "sopc_mem_alloc.h"

SOPC_SC_SecurityKeySet* SOPC_KeySet_Create(void)
{
    SOPC_SC_SecurityKeySet* keySet = SOPC_Malloc(sizeof(SOPC_SC_SecurityKeySet));
    return keySet;
}

void SOPC_KeySet_Delete(SOPC_SC_SecurityKeySet* keySet)
{
    if (keySet != NULL)
    {
        SOPC_SecretBuffer_DeleteClear(keySet->encryptKey);
        SOPC_SecretBuffer_DeleteClear(keySet->initVector);
        SOPC_SecretBuffer_DeleteClear(keySet->signKey);
        SOPC_Free(keySet);
    }
}
