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
 * \file sopc_circular_queue.c
 * \brief Fixed-capacity circular buffer implementing SOPC_CircularQueue.
 *
 * Layout of the ring (indices into \c slots, modulo \c allocatedCap):
 * - Occupied range is \c length consecutive slots starting at \c head
 * - Append writes at index (head + length) % allocatedCap
 * - Prepend moves head one step backward then writes at the new head
 * - Logical order head -> tail wraps when head + length exceeds allocatedCap
 */

#include "sopc_circular_queue.h"

#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

struct SOPC_CircularQueue
{
    /**
     * Heap-allocated ring buffer of value slots.
     * Size is \c allocatedCap entries; only the \c length entries from \c head
     * hold live values. Remaining slots are unused (cleared to 0 on pop).
     */
    uintptr_t* slots;
    /**
     * Index of the oldest element (logical head / FIFO front).
     * Valid in [0, allocatedCap]. Undefined when \c length is 0 (reset to 0
     * by Clear).
     */
    uint32_t head;
    /**
     * Number of live elements currently stored in the ring.
     * Always <= \c maxLength and <= \c allocatedCap.
     */
    uint32_t length;
    /**
     * Logical capacity: maximum number of elements the caller may store.
     * Enforced by Append / Prepend. May be less than \c allocatedCap after a
     * shrink via SetCapacity (buffer is kept oversized to avoid realloc).
     */
    uint32_t maxLength;
    /**
     * Physical size of \c slots (number of uintptr_t entries allocated).
     * Equals \c maxLength at Create. Grows when SetCapacity raises capacity
     * above the current allocation; never shrinks when capacity is lowered.
     */
    uint32_t allocatedCap;
};

SOPC_CircularQueue* SOPC_CircularQueue_Create(size_t sizeMax)
{
    /* Unlimited capacity (sizeMax == 0) is intentionally unsupported. */
    if (0 == sizeMax || sizeMax > UINT32_MAX)
    {
        return NULL;
    }

    SOPC_CircularQueue* queue = SOPC_Calloc(1, sizeof(SOPC_CircularQueue));
    if (NULL == queue)
    {
        return NULL;
    }

    queue->slots = SOPC_Calloc(sizeMax, sizeof(uintptr_t));
    if (NULL == queue->slots)
    {
        SOPC_Free(queue);
        return NULL;
    }

    /* head and length remain 0 from Calloc; physical size matches capacity. */
    queue->maxLength = (uint32_t) sizeMax;
    queue->allocatedCap = (uint32_t) sizeMax;
    return queue;
}

uintptr_t SOPC_CircularQueue_Prepend(SOPC_CircularQueue* queue, uintptr_t value)
{
    if (NULL == queue || 0 == value || queue->length >= queue->maxLength)
    {
        return 0;
    }

    /* Move head one step backward in the ring (wrap with + allocatedCap - 1). */
    queue->head = (queue->head + queue->allocatedCap - 1) % queue->allocatedCap;
    queue->slots[queue->head] = value;
    queue->length++;
    return value;
}

uintptr_t SOPC_CircularQueue_Append(SOPC_CircularQueue* queue, uintptr_t value)
{
    if (NULL == queue || 0 == value || queue->length >= queue->maxLength)
    {
        return 0;
    }

    /* Next free slot is immediately after the last live element (may wrap). */
    uint32_t idx = (queue->head + queue->length) % queue->allocatedCap;
    queue->slots[idx] = value;
    queue->length++;
    return value;
}

uintptr_t SOPC_CircularQueue_PopHead(SOPC_CircularQueue* queue)
{
    if (NULL == queue || 0 == queue->length)
    {
        return 0;
    }

    uintptr_t value = queue->slots[queue->head];
    queue->slots[queue->head] = 0;
    /* Advance head past the removed element; wrap at allocatedCap. */
    queue->head = (queue->head + 1) % queue->allocatedCap;
    queue->length--;
    return value;
}

uintptr_t SOPC_CircularQueue_PopLast(SOPC_CircularQueue* queue)
{
    if (NULL == queue || 0 == queue->length)
    {
        return 0;
    }

    /* Tail index is the last occupied slot: head + length - 1 (modulo). */
    uint32_t idx = (queue->head + queue->length - 1) % queue->allocatedCap;
    uintptr_t value = queue->slots[idx];
    queue->slots[idx] = 0;
    queue->length--;
    return value;
}

uintptr_t SOPC_CircularQueue_GetHead(SOPC_CircularQueue* queue)
{
    if (NULL == queue || 0 == queue->length)
    {
        return 0;
    }
    return queue->slots[queue->head];
}

uintptr_t SOPC_CircularQueue_GetLast(SOPC_CircularQueue* queue)
{
    if (NULL == queue || 0 == queue->length)
    {
        return 0;
    }
    uint32_t idx = (queue->head + queue->length - 1) % queue->allocatedCap;
    return queue->slots[idx];
}

void SOPC_CircularQueue_Apply(SOPC_CircularQueue* queue, void (*pFn)(uintptr_t val))
{
    if (NULL == queue || NULL == pFn || 0 == queue->length)
    {
        return;
    }

    /* Walk logical order from oldest to newest; id is always 0 (not stored). */
    for (uint32_t i = 0; i < queue->length; i++)
    {
        uint32_t idx = (queue->head + i) % queue->allocatedCap;
        pFn(queue->slots[idx]);
    }
}

SOPC_CircularQueueIterator SOPC_CircularQueue_GetIterator(SOPC_CircularQueue* queue)
{
    SOPC_CircularQueueIterator it = {.queue = queue, .nextOffset = 0};
    return it;
}

bool SOPC_CircularQueue_HasNext(const SOPC_CircularQueueIterator* it)
{
    if (NULL == it || NULL == it->queue)
    {
        return false;
    }
    return it->nextOffset < it->queue->length;
}

uintptr_t SOPC_CircularQueue_Next(SOPC_CircularQueueIterator* it)
{
    return SOPC_CircularQueue_NextWithIndex(it, NULL);
}

uintptr_t SOPC_CircularQueue_NextWithIndex(SOPC_CircularQueueIterator* it, uint32_t* pIndex)
{
    if (!SOPC_CircularQueue_HasNext(it))
    {
        return 0;
    }
    uint32_t logicalIndex = it->nextOffset;
    uint32_t idx = (it->queue->head + logicalIndex) % it->queue->allocatedCap;
    it->nextOffset++;
    if (NULL != pIndex)
    {
        *pIndex = logicalIndex;
    }
    return it->queue->slots[idx];
}

uintptr_t SOPC_CircularQueue_RemoveAt(SOPC_CircularQueue* queue, uint32_t index)
{
    if (NULL == queue || index >= queue->length)
    {
        return 0;
    }

    uint32_t slot = (queue->head + index) % queue->allocatedCap;
    uintptr_t value = queue->slots[slot];

    /* Close the gap: shift later elements one step toward the head. */
    for (uint32_t j = index; j + 1 < queue->length; j++)
    {
        uint32_t cur = (queue->head + j) % queue->allocatedCap;
        uint32_t nxt = (queue->head + j + 1) % queue->allocatedCap;
        queue->slots[cur] = queue->slots[nxt];
    }
    uint32_t last = (queue->head + queue->length - 1) % queue->allocatedCap;
    queue->slots[last] = 0;
    queue->length--;
    return value;
}

void SOPC_CircularQueue_Clear(SOPC_CircularQueue* queue)
{
    if (NULL == queue)
    {
        return;
    }
    /* Drop logical content only; values are not freed (caller Apply first).
     * Stale slot data is ignored while length is 0 and overwritten on next write. */
    queue->head = 0;
    queue->length = 0;
}

void SOPC_CircularQueue_Delete(SOPC_CircularQueue* queue)
{
    if (NULL == queue)
    {
        return;
    }
    SOPC_Free(queue->slots);
    queue->slots = NULL;
    SOPC_Free(queue);
}

uint32_t SOPC_CircularQueue_GetLength(SOPC_CircularQueue* queue)
{
    if (NULL == queue)
    {
        return 0;
    }
    return queue->length;
}

uint32_t SOPC_CircularQueue_GetCapacity(SOPC_CircularQueue* queue)
{
    if (NULL == queue)
    {
        return 0;
    }
    return queue->maxLength;
}

bool SOPC_CircularQueue_SetCapacity(SOPC_CircularQueue* queue, size_t sizeMax)
{
    if (NULL == queue || 0 == sizeMax || sizeMax > UINT32_MAX)
    {
        return false;
    }
    /* Cannot set a capacity below the number of elements already stored. */
    if (queue->length > sizeMax)
    {
        return false;
    }

    uint32_t newMax = (uint32_t) sizeMax;

    if (newMax > queue->allocatedCap)
    {
        /* Grow physical buffer: copy the (possibly wrapped) ring into a
         * contiguous layout starting at index 0 so head can be reset. */
        uintptr_t* newSlots = SOPC_Calloc(newMax, sizeof(uintptr_t));
        if (NULL == newSlots)
        {
            return false;
        }
        for (uint32_t i = 0; i < queue->length; i++)
        {
            newSlots[i] = queue->slots[(queue->head + i) % queue->allocatedCap];
        }
        SOPC_Free(queue->slots);
        queue->slots = newSlots;
        queue->head = 0;
        queue->allocatedCap = newMax;
    }
    /* Shrink path: only lower maxLength; keep allocatedCap so later growth
     * within the previous physical size needs no realloc. */

    queue->maxLength = newMax;
    return true;
}
