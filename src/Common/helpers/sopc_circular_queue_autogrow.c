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
 * \file sopc_circular_queue_autogrow.c
 * \brief Auto-growing usage of a SOPC_CircularQueue.
 */

#include "sopc_circular_queue_autogrow.h"

SOPC_CircularQueueAutoGrow* SOPC_CircularQueueAutoGrow_Create(size_t initialCapacity)
{
    return SOPC_CircularQueue_Create(initialCapacity);
}

uintptr_t SOPC_CircularQueueAutoGrow_Append(SOPC_CircularQueueAutoGrow* queue, uintptr_t value)
{
    if (NULL == queue || 0 == value)
    {
        return 0;
    }

    uintptr_t res = SOPC_CircularQueue_Append(queue, value);
    if (0 == res)
    {
        uint32_t length = SOPC_CircularQueue_GetLength(queue);
        uint32_t capacity = SOPC_CircularQueue_GetCapacity(queue);
        if (length >= capacity)
        {
            /* Double the capacity; saturate instead of overflowing uint32_t. */
            uint32_t newCapacity = (capacity > UINT32_MAX / 2) ? UINT32_MAX : capacity * 2;
            if (newCapacity == capacity || !SOPC_CircularQueue_SetCapacity(queue, newCapacity))
            {
                return 0;
            }
            res = SOPC_CircularQueue_Append(queue, value);
        }
    }

    return res;
}

uintptr_t SOPC_CircularQueueAutoGrow_RemoveValue(SOPC_CircularQueueAutoGrow* queue, uintptr_t value)
{
    if (NULL == queue || 0 == value)
    {
        return 0;
    }

    SOPC_CircularQueueIterator it = SOPC_CircularQueue_GetIterator(queue);
    while (SOPC_CircularQueue_HasNext(&it))
    {
        uint32_t index = 0;
        uintptr_t val = SOPC_CircularQueue_NextWithIndex(&it, &index);
        if (val == value)
        {
            return SOPC_CircularQueue_RemoveAt(queue, index);
        }
    }
    return 0;
}

SOPC_CircularQueueIterator SOPC_CircularQueueAutoGrow_GetIterator(SOPC_CircularQueueAutoGrow* queue)
{
    return SOPC_CircularQueue_GetIterator(queue);
}

void SOPC_CircularQueueAutoGrow_Delete(SOPC_CircularQueueAutoGrow* queue)
{
    SOPC_CircularQueue_Delete(queue);
}
