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
#include <stdlib.h>

#include <inttypes.h>

#include "kernel.h"

#include "sopc_atomic.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#ifdef CONFIG_SOPC_HELPER_IMPL_INSTRUM

static size_t gNbAllocs = 0;
static inline void INCREASE_NB_ALLOCS(void* res, int i)
{
    if (res != NULL)
    {
        SOPC_Atomic_Int_Add(&gNbAllocs, i);
    }
}

static inline void CHECK_ALLOC_RESULT(void* res, size_t size)
{
    if (res == NULL)
    {
        printk("\r\n !!! P_MEM_ALLOC (%u) returned NULL\r\n", size);
    }
}

const size_t SOPC_MemAlloc_Nb_Allocs(void)
{
    return gNbAllocs;
}
#else
#define INCREASE_NB_ALLOCS(res, i)
#define CHECK_ALLOC_RESULT(res, size)
#endif

void* SOPC_Malloc(size_t size)
{
    // Minimum size = 4 to avoid NULL pointer
    if (0 == size)
    {
        size = 4;
    }
    void* result = malloc(size);
    CHECK_ALLOC_RESULT(result, size);
    INCREASE_NB_ALLOCS(result, 1);
    return result;
}

void SOPC_Free(void* ptr)
{
    INCREASE_NB_ALLOCS(ptr, -1);
    free(ptr);
}

void* SOPC_Calloc(size_t nmemb, size_t size)
{
    if (nmemb > SIZE_MAX / size)
    {
        return NULL;
    }
    // Minimum size = 4 to avoid NULL pointer
    size_t total_size = nmemb * size;
    if (0 == total_size)
    {
        total_size = 4;
    }
    void* result = calloc(total_size, 1);
    CHECK_ALLOC_RESULT(result, size);
    INCREASE_NB_ALLOCS(result, 1);
    return result;
}

void* SOPC_Realloc(void* ptr, size_t old_size, size_t new_size)
{
    SOPC_UNUSED_ARG(old_size);
    void* result = realloc(ptr, new_size);
    CHECK_ALLOC_RESULT(result, new_size);
    return result;
}
