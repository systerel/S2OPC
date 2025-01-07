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

#include "libs2opc_internal_eventid_list.h"

#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_singly_linked_list.h"

/**
 * Internal function to free a SOPC_ByteString stored in the linked list
 */
static void SOPC_EventIdList_FreeByteString(uint32_t id, uintptr_t val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_ByteString* bs = (SOPC_ByteString*) val;
    if (NULL != bs)
    {
        SOPC_ByteString_Clear(bs);
        SOPC_Free(bs);
    }
}

SOPC_EventIdList* SOPC_EventIdList_Create(size_t maxSize)
{
    return SOPC_SLinkedList_Create(maxSize);
}

SOPC_ReturnStatus SOPC_EventIdList_Add(SOPC_EventIdList* list, const SOPC_ByteString* eventId)
{
    if (NULL == list || NULL == eventId)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Create a copy of the EventId
    SOPC_ByteString* eventIdCopy = SOPC_Calloc(1, sizeof(SOPC_ByteString));
    if (NULL == eventIdCopy)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    SOPC_ByteString_Initialize(eventIdCopy);

    SOPC_ReturnStatus status = SOPC_ByteString_Copy(eventIdCopy, eventId);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(eventIdCopy);
        return status;
    }

    // If list is at capacity, remove oldest element (head of list)
    if (SOPC_SLinkedList_GetLength(list) == SOPC_SLinkedList_GetCapacity(list))
    {
        uintptr_t oldestVal = SOPC_SLinkedList_PopHead(list);
        if (0 != oldestVal)
        {
            SOPC_EventIdList_FreeByteString(0, oldestVal);
        }
    }

    // Append new EventId to list
    if (0 == SOPC_SLinkedList_Append(list, 0, (uintptr_t) eventIdCopy))
    {
        SOPC_ByteString_Clear(eventIdCopy);
        SOPC_Free(eventIdCopy);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    return SOPC_STATUS_OK;
}

bool SOPC_EventIdList_Contains(SOPC_EventIdList* list, const SOPC_ByteString* eventId)
{
    if (NULL == list || NULL == eventId)
    {
        return false;
    }

    SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(list);
    while (SOPC_SLinkedList_HasNext(&it))
    {
        const SOPC_ByteString* currentEventId = (SOPC_ByteString*) SOPC_SLinkedList_Next(&it);
        if (SOPC_ByteString_Equal(currentEventId, eventId))
        {
            return true;
        }
    }

    return false;
}

void SOPC_EventIdList_Clear(SOPC_EventIdList* list)
{
    if (NULL != list)
    {
        SOPC_SLinkedList_Apply(list, SOPC_EventIdList_FreeByteString);
        SOPC_SLinkedList_Clear(list);
    }
}

void SOPC_EventIdList_Delete(SOPC_EventIdList** list)
{
    if (NULL != list)
    {
        SOPC_EventIdList_Clear(*list);
        SOPC_Free(*list);
        *list = NULL;
    }
}
