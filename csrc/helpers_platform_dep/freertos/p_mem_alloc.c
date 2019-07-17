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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"

#include "sopc_mem_alloc.h"

#ifndef DEBUG_MEMORY_LEAKS
void* SOPC_Malloc(size_t size)
#else
void* SOPC_Malloc_(size_t size)
#endif
{
    // Minimum size = 4 to avoid NULL pointer
    if (0 == size)
    {
        size = 4;
    }
    return pvPortMalloc(size);
}

#ifndef DEBUG_MEMORY_LEAKS
void SOPC_Free(void* ptr)
#else
void SOPC_Free_(void* ptr)
#endif
{
    vPortFree(ptr);
}

#ifndef DEBUG_MEMORY_LEAKS
void* SOPC_Calloc(size_t nmemb, size_t size)
#else
void* SOPC_Calloc_(size_t nmemb, size_t size)
#endif
{
    if (nmemb > SIZE_MAX / size)
    {
        return NULL;
    }

#ifndef DEBUG_MEMORY_LEAKS
    void* p = SOPC_Malloc(nmemb * size);
#else
    void* p = SOPC_Malloc_(nmemb * size);
#endif
    if (NULL == p)
    {
        return NULL;
    }

    return memset(p, 0, nmemb * size);
}

#ifndef DEBUG_MEMORY_LEAKS
void* SOPC_Realloc(void* ptr, size_t old_size, size_t new_size)
#else
void* SOPC_Realloc_(void* ptr, size_t old_size, size_t new_size)
#endif
{
    /* Do not realloc/copy if size is reduced.
     * This keeps the larger buffer reserved but avoids the copy. */
    if (new_size <= old_size)
    {
        return ptr;
    }

    /* realloc(NULL) shall behave as malloc */
#ifndef DEBUG_MEMORY_LEAKS
    void* new = SOPC_Malloc(new_size);
#else
    void* new = SOPC_Malloc_(new_size);
#endif
    if (NULL == new)
    {
        return NULL;
    }

    if (NULL != ptr)
    {
        memcpy(new, ptr, old_size);
        SOPC_Free(ptr);
    }

    return new;
}
