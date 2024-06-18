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

#include "unit_test_include.h"

#include "sopc_assert.h"
#include "sopc_enums.h"
#include "sopc_mem_alloc.h"

void suite_test_alloc_memory(int* index)
{
    PRINT("\nTEST %d: sopc_mem_alloc \n", *index);
    int* allocatedMemory = NULL;
    allocatedMemory = SOPC_Malloc(sizeof(int));
    SOPC_ASSERT(NULL != allocatedMemory);
    SOPC_Free(allocatedMemory);
    PRINT("Test1 : ok\n");

    allocatedMemory = NULL;
    allocatedMemory = SOPC_Malloc(0);
    SOPC_ASSERT(NULL != allocatedMemory);
    SOPC_Free(allocatedMemory);
    PRINT("Test2 : ok\n");

    allocatedMemory = NULL;
    const int nbMembers = 10;
    allocatedMemory = SOPC_Calloc(nbMembers, sizeof(int));
    SOPC_ASSERT(NULL != allocatedMemory);
    SOPC_Free(allocatedMemory);
    PRINT("Test3 : ok\n");

    *index += 1;
}
