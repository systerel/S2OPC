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
 * \file sopc_circular_queue_autogrow.h
 *
 * \brief Auto-growing usage of a SOPC_CircularQueue.
 *
 * SOPC_CircularQueue is fixed-capacity: SOPC_CircularQueue_Append fails when
 * the queue is full. SOPC_CircularQueueAutoGrow_Append instead doubles the
 * capacity (SOPC_CircularQueue_SetCapacity) and retries, so the queue behaves
 * as unbounded to the caller, matching the SOPC_SLinkedList_Create(0)
 * ("no limit") queues this API replaces. Growth only reallocates the slots:
 * the queue address never changes, so it can be stored as opaque B-model
 * state or as a dictionary value.
 *
 * The capacity is never decreased when elements are removed.
 *
 * Not thread-safe.
 */

#ifndef SOPC_CIRCULAR_QUEUE_AUTOGROW_H_
#define SOPC_CIRCULAR_QUEUE_AUTOGROW_H_

#include <stddef.h>
#include <stdint.h>

#include "sopc_circular_queue.h"

/**
 * \brief Alias of SOPC_CircularQueue, to be manipulated only through the
 *        SOPC_CircularQueueAutoGrow API to keep the auto-growing behavior.
 */
typedef SOPC_CircularQueue SOPC_CircularQueueAutoGrow;

/**
 * \brief Create a new queue with the given initial capacity.
 *
 * \param initialCapacity  Initial capacity; must be > 0. Grown automatically
 *                         (doubling) by ::SOPC_CircularQueueAutoGrow_Append.
 * \return                 Newly allocated queue, or NULL on failure / invalid capacity.
 */
SOPC_CircularQueueAutoGrow* SOPC_CircularQueueAutoGrow_Create(size_t initialCapacity);

/**
 * \brief Append a value, first doubling the capacity if the queue is full.
 *
 * Unlike SOPC_CircularQueue_Append, this only fails on allocation failure or
 * invalid arguments, never because the queue is momentarily full: growth is
 * bounded only by available memory (and the uint32_t capacity representation).
 *
 * \param queue   The queue.
 * \param value   Non-zero value to store.
 * \return        \p value on success, 0 on failure (growth/allocation failed, NULL queue, or value 0).
 */
uintptr_t SOPC_CircularQueueAutoGrow_Append(SOPC_CircularQueueAutoGrow* queue, uintptr_t value);

/**
 * \brief Scan for and remove the first occurrence of \p value.
 *
 * O(n) in queue length (same cost as the SOPC_SLinkedList_RemoveFromId scan
 * this type replaces): acceptable for the small/practically-bounded sets
 * this queue currently backs (the monitored items of one subscription or
 * of one Node).
 *
 * \param queue   The queue.
 * \param value   The value to find and remove; must be non-zero.
 * \return        \p value if found and removed, 0 otherwise (including NULL queue).
 */
uintptr_t SOPC_CircularQueueAutoGrow_RemoveValue(SOPC_CircularQueueAutoGrow* queue, uintptr_t value);

/**
 * \brief Get an iterator positioned at the head (oldest element) of the queue.
 *
 * Do not call ::SOPC_CircularQueueAutoGrow_Append or
 * ::SOPC_CircularQueueAutoGrow_RemoveValue while an iterator obtained before
 * that call is still in use (same restriction as SOPC_CircularQueue's own
 * iterator: the snapshot is only valid as long as the queue is not
 * grown or mutated).
 *
 * \param queue   The queue.
 * \return        Iterator over the queue; empty/invalid if \p queue is NULL.
 */
SOPC_CircularQueueIterator SOPC_CircularQueueAutoGrow_GetIterator(SOPC_CircularQueueAutoGrow* queue);

/**
 * \brief Get the number of elements currently stored.
 *
 * \param queue   The queue.
 * \return        Length, or 0 if NULL.
 */
#define SOPC_CircularQueueAutoGrow_GetLength SOPC_CircularQueue_GetLength

/**
 * \brief Delete the queue (not the stored values).
 *
 * \param queue  The queue; pointer must not be used after this call.
 */
void SOPC_CircularQueueAutoGrow_Delete(SOPC_CircularQueueAutoGrow* queue);

#endif /* SOPC_CIRCULAR_QUEUE_AUTOGROW_H_ */
