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
 * \file sopc_circular_queue.h
 *
 * \brief Fixed-capacity circular queue API.
 *
 * Stores uintptr_t values (typically pointers) in a pre-allocated slot array so
 * Append / Prepend / Pop do not allocate per element. Intended for bounded
 * FIFO/deque use cases such as monitored-item notification queues.
 *
 * \warning not a thread-safe API.
 */

#ifndef SOPC_CIRCULAR_QUEUE_H_
#define SOPC_CIRCULAR_QUEUE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * \brief Opaque fixed-capacity circular queue of uintptr_t values.
 */
typedef struct SOPC_CircularQueue SOPC_CircularQueue;

/**
 * \brief Iterator over queue elements from head (oldest) to tail (newest).
 *
 * Obtained via ::SOPC_CircularQueue_GetIterator. Do not modify the queue
 * (Append / Prepend / Pop / Remove / Clear / Delete / SetCapacity) while an
 * iterator is in use, except that values may be read.
 */
typedef struct SOPC_CircularQueueIterator
{
    /** Queue being iterated; NULL when the iterator is empty / invalid. */
    SOPC_CircularQueue* queue;
    /**
     * Logical offset from head of the next element to return.
     * Valid range while iterating: [0, length]. Equals length when exhausted.
     */
    uint32_t nextOffset;
} SOPC_CircularQueueIterator;

/**
 * \brief Create a queue with the given maximum number of elements.
 *
 * \param sizeMax  Maximum number of elements; must be > 0 and <= UINT32_MAX.
 * \return         Newly allocated queue, or NULL on failure / invalid sizeMax.
 */
SOPC_CircularQueue* SOPC_CircularQueue_Create(size_t sizeMax);

/**
 * \brief Prepend a value at the head (oldest position).
 *
 * \param queue   The queue.
 * \param value  Non-zero value to store.
 * \return       \p value on success, 0 on failure (full, NULL queue, or value 0).
 */
uintptr_t SOPC_CircularQueue_Prepend(SOPC_CircularQueue* queue, uintptr_t value);

/**
 * \brief Append a value at the tail (newest position).
 *
 * \param queue   The queue.
 * \param value  Non-zero value to store.
 * \return       \p value on success, 0 on failure (full, NULL queue, or value 0).
 */
uintptr_t SOPC_CircularQueue_Append(SOPC_CircularQueue* queue, uintptr_t value);

/**
 * \brief Remove and return the head (oldest) value.
 *
 * \param queue  The queue.
 * \return      Head value, or 0 if empty / NULL.
 */
uintptr_t SOPC_CircularQueue_PopHead(SOPC_CircularQueue* queue);

/**
 * \brief Remove and return the last (newest) value.
 *
 * \param queue  The queue.
 * \return      Last value, or 0 if empty / NULL.
 */
uintptr_t SOPC_CircularQueue_PopLast(SOPC_CircularQueue* queue);

/**
 * \brief Return the head value without removing it.
 *
 * \param queue  The queue.
 * \return      Head value, or 0 if empty / NULL.
 */
uintptr_t SOPC_CircularQueue_GetHead(SOPC_CircularQueue* queue);

/**
 * \brief Return the last value without removing it.
 *
 * \param queue  The queue.
 * \return      Last value, or 0 if empty / NULL.
 */
uintptr_t SOPC_CircularQueue_GetLast(SOPC_CircularQueue* queue);

/**
 * \brief Apply a function to each value from head to tail.
 *
 * \param queue  The queue.
 * \param pFn    Callback to apply a treatment on stored value.
 *               Note: in-place treatment only possible when uintptr_t is an actual pointer.
 */
void SOPC_CircularQueue_Apply(SOPC_CircularQueue* queue, void (*pFn)(uintptr_t val));

/**
 * \brief Get an iterator positioned at the head (oldest element).
 *
 * \param queue  The queue.
 * \return       Iterator; HasNext is false if \p queue is NULL or empty.
 */
SOPC_CircularQueueIterator SOPC_CircularQueue_GetIterator(SOPC_CircularQueue* queue);

/**
 * \brief Return true if ::SOPC_CircularQueue_Next would return a non-zero value.
 *
 * \param it  Iterator from ::SOPC_CircularQueue_GetIterator.
 * \return    true if a next element is available, false otherwise.
 */
bool SOPC_CircularQueue_HasNext(const SOPC_CircularQueueIterator* it);

/**
 * \brief Return the next value and advance the iterator toward the tail.
 *
 * \param it  Iterator from ::SOPC_CircularQueue_GetIterator.
 * \return    Next value, or 0 if exhausted / invalid.
 */
uintptr_t SOPC_CircularQueue_Next(SOPC_CircularQueueIterator* it);

/**
 * \brief Return the next value, its logical index from head, and advance the iterator.
 *
 * The index is valid for ::SOPC_CircularQueue_RemoveAt until the queue is modified.
 *
 * \param it           Iterator from ::SOPC_CircularQueue_GetIterator.
 * \param[out] pIndex  If non-NULL, set to the logical index (0 = head) of the returned value.
 * \return             Next value, or 0 if exhausted / invalid (\p pIndex unchanged then).
 */
uintptr_t SOPC_CircularQueue_NextWithIndex(SOPC_CircularQueueIterator* it, uint32_t* pIndex);

/**
 * \brief Remove the element at the given logical index (0 = head).
 *
 * Remaining elements keep their relative order
 * (but indexes following the removed value change).
 * O(n) in queue length.
 *
 * \param queue   The queue.
 * \param index  Logical offset from head; must be < ::SOPC_CircularQueue_GetLength.
 * \return       Removed value, or 0 if \p queue is NULL or \p index is out of range.
 */
uintptr_t SOPC_CircularQueue_RemoveAt(SOPC_CircularQueue* queue, uint32_t index);

/**
 * \brief Clear all slots without freeing stored values.
 *
 * Caller must Apply a free callback first if values need releasing.
 *
 * \param queue  The queue.
 */
void SOPC_CircularQueue_Clear(SOPC_CircularQueue* queue);

/**
 * \brief Clear and deallocate the queue (not the stored values).
 *
 * \param queue  The queue; pointer must not be used after this call.
 */
void SOPC_CircularQueue_Delete(SOPC_CircularQueue* queue);

/**
 * \brief Get the number of elements currently stored.
 *
 * \param queue  The queue.
 * \return      Length, or 0 if NULL.
 */
uint32_t SOPC_CircularQueue_GetLength(SOPC_CircularQueue* queue);

/**
 * \brief Get the maximum number of elements allowed.
 *
 * \param queue  The queue.
 * \return      Capacity, or 0 if NULL.
 */
uint32_t SOPC_CircularQueue_GetCapacity(SOPC_CircularQueue* queue);

/**
 * \brief Change the maximum number of elements.
 *
 * Fails if current length is greater than \p sizeMax, or if \p sizeMax is 0
 * (unlimited capacity is not supported). Growing beyond the allocated slot
 * array reallocates and linearizes the ring. Shrinking only updates the
 * capacity limit and keeps the oversized buffer.
 *
 * \param queue     The queue.
 * \param sizeMax  New capacity; must be > 0.
 * \return         true on success, false otherwise.
 */
bool SOPC_CircularQueue_SetCapacity(SOPC_CircularQueue* queue, size_t sizeMax);

#endif /* SOPC_CIRCULAR_QUEUE_H_ */
