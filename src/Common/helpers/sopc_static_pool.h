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
 * \file sopc_static_pool.h
 * \brief Fixed-block allocator backed by caller-provided static storage.
 *
 * The pool struct and its backing buffer are provided by the caller (typically
 * as file-scope `static` arrays), so this module never allocates memory for
 * itself. Once the buffer's blocks are exhausted, ::SOPC_StaticPool_Alloc
 * transparently falls back to ::SOPC_Malloc for individual blocks; those are
 * returned to the heap by ::SOPC_StaticPool_Free instead of the pool.
 * The free blocks in the memory pool are managed by a linked list mechanism,
 * each free block contains the address of the next one and free list head is
 * stored in the pool struct. Thus either a block is free and stores link to next
 * free block, either the block is used and stores data.
 *
 * Not thread-safe: callers must ensure single-threaded access (or add their
 * own external synchronization).
 *
 */

#ifndef SOPC_STATIC_POOL_H_
#define SOPC_STATIC_POOL_H_

#include <stddef.h>
#include <stdint.h>

/**
 * \brief Pool state. Fields are private: only accessed through the functions
 *        below. Exposed as a full struct (not opaque) so it can be declared
 *        as `static` storage by the caller instead of being heap-allocated.
 */
typedef struct SOPC_StaticPool
{
    /** Backing storage for the fixed blocks (caller-owned, never freed here). */
    uint8_t* buffer;
    /** Size in bytes of one block; must be >= sizeof(void*). */
    size_t block_size;
    /** Number of blocks in \c buffer. */
    size_t capacity;
    /**
     * Head of the LIFO freelist of unused pool blocks. Each free block stores
     * the next pointer in its first sizeof(void*) bytes. NULL when exhausted.
     */
    void* free_list_head;
    /**
     * Cumulative count of ::SOPC_Malloc fallbacks since Init (never decremented).
     */
    size_t nb_fallback_allocs;
} SOPC_StaticPool;

/**
 * \brief Initializes a pool over a caller-provided buffer.
 * \param[in, out] pool  The pool to initialize (caller-owned storage, e.g. `static`).
 * \param buffer         Backing storage, at least `block_size * capacity` bytes
 *                       (caller-owned storage, e.g. `static`).
 *                       The caller is responsible for ensuring `buffer` is suitably
 *                       aligned for the block type it will store: declare it as a
 *                       `static` array of that type (e.g.
 *                       `static SOPC_DataValue buf[CAP];`), not as
 *                       `uint8_t buf[CAP * sizeof(T)]`, which only has
 *                       alignment 1 and can cause undefined behavior (misaligned
 *                       access, potentially a fault) on strict-alignment
 *                       targets once blocks are reinterpreted as `T*`.
 *                       `Init` only asserts a minimum `sizeof(void*)` alignment as a
 *                       defensive sanity check; this does not guarantee correct
 *                       alignment for the actual stored type, which remains the
 *                       caller's responsibility.
 * \param block_size     Size in bytes of each block. Must be >= sizeof(void*).
 * \param capacity       Number of blocks in `buffer`. Must be > 0.
 */
void SOPC_StaticPool_Init(SOPC_StaticPool* pool, void* buffer, size_t block_size, size_t capacity);

/**
 * \brief Allocates one block, zero-initialized. Falls back to ::SOPC_Malloc
 *        if the pool is exhausted.
 * \param[in, out] pool  The pool to allocate from.
 * \return               A zeroed block of `block_size` bytes, or NULL on fallback OOM.
 */
void* SOPC_StaticPool_Alloc(SOPC_StaticPool* pool);

/**
 * \brief Returns a block obtained from ::SOPC_StaticPool_Alloc. Transparently
 *        detects whether the block came from the pool or the malloc fallback.
 *        No-op if `block` is NULL.
 * \param[in, out] pool  The pool that owns the block (if pool-backed).
 * \param block          Block pointer previously returned by ::SOPC_StaticPool_Alloc.
 */
void SOPC_StaticPool_Free(SOPC_StaticPool* pool, void* block);

/**
 * \brief Number of ::SOPC_Malloc fallbacks since the last ::SOPC_StaticPool_Init.
 *
 * This is a cumulative event counter (one increment per fallback Alloc).
 *
 * \param pool  The pool to query.
 * \return      Fallback allocation event count since the last ::SOPC_StaticPool_Init.
 */
size_t SOPC_StaticPool_GetNbFallbackAllocsStat(const SOPC_StaticPool* pool);

#endif /* SOPC_STATIC_POOL_H_ */
