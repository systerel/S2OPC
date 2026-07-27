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
 * \file sopc_static_pool.c
 * \brief Implementation of the fixed-block static pool allocator.
 */

#include "sopc_static_pool.h"

#include <string.h>

#include "sopc_assert.h"
#include "sopc_mem_alloc.h"

void SOPC_StaticPool_Init(SOPC_StaticPool* pool, void* buffer, size_t block_size, size_t capacity)
{
    SOPC_ASSERT(NULL != pool);
    SOPC_ASSERT(NULL != buffer);
    SOPC_ASSERT(block_size >= sizeof(void*));
    SOPC_ASSERT(capacity > 0);
    SOPC_ASSERT(0 == ((uintptr_t) buffer) % sizeof(void*));

    pool->buffer = (uint8_t*) buffer;
    pool->block_size = block_size;
    pool->capacity = capacity;
    /* Empty freelist: every block in the backing buffer will be pushed below. */
    pool->free_list_head = NULL;
    pool->nb_fallback_allocs = 0;

    /* Build the initial LIFO freelist: each unused block stores a link to the
     * previous head in its first sizeof(void*) bytes (intrusive list, no extra
     * metadata).
     * Iteration order makes the last buffer slot the first Alloc. */
    for (size_t i = 0; i < capacity; ++i)
    {
        void* block = &pool->buffer[i * block_size];
        memcpy(block, &pool->free_list_head, sizeof(void*));
        pool->free_list_head = block;
    }
}

void* SOPC_StaticPool_Alloc(SOPC_StaticPool* pool)
{
    SOPC_ASSERT(NULL != pool);

    void* block = pool->free_list_head;

    if (NULL != block)
    {
        /* Pop freelist head: read next link (free block content) via memcpy (-Wcast-align safe). */
        memcpy(&pool->free_list_head, block, sizeof(void*));
    }
    else
    {
        block = SOPC_Malloc(pool->block_size);
        if (NULL != block)
        {
            /* Count fallback events (not live heap blocks); never decremented. */
            pool->nb_fallback_allocs++;
        }
    }

    if (NULL != block)
    {
        /* Wipe previous payload (and the old freelist link when reusing a pool
         * block) before handing the block to the caller. */
        memset(block, 0, pool->block_size);
    }

    return block;
}

void SOPC_StaticPool_Free(SOPC_StaticPool* pool, void* block)
{
    SOPC_ASSERT(NULL != pool);

    if (NULL == block)
    {
        return;
    }

    uint8_t* byteBlock = (uint8_t*) block;
    uint8_t* bufferEnd = &pool->buffer[pool->capacity * pool->block_size];

    if (byteBlock >= pool->buffer && byteBlock < bufferEnd)
    {
        size_t offset = (size_t)(byteBlock - pool->buffer);
        SOPC_ASSERT(0 == offset % pool->block_size);
        /* Pool-backed block: push back onto the freelist for reuse. The link
         * field overwrites the first sizeof(void*) bytes;
         * payload is cleared on the next Alloc. => TODO: memset 0 before instead ??? */

        memcpy(block, &pool->free_list_head, sizeof(void*));
        pool->free_list_head = block;
    }
    else
    {
        /* Outside the backing buffer: must be a malloc fallback block from a
         * prior Alloc when the freelist was empty. Return it to the heap. */
        SOPC_Free(block);
    }
}

size_t SOPC_StaticPool_GetNbFallbackAllocsStat(const SOPC_StaticPool* pool)
{
    SOPC_ASSERT(NULL != pool);
    return pool->nb_fallback_allocs;
}
