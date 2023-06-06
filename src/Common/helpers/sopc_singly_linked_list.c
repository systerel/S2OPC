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

#include "sopc_singly_linked_list.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

struct SOPC_SLinkedList_Elt
{
    uint32_t id;
    uintptr_t value;
    struct SOPC_SLinkedList_Elt* next;
};

struct SOPC_SLinkedList
{
    SOPC_SLinkedList_Elt* first;
    SOPC_SLinkedList_Elt* last;
    uint32_t length;
    uint32_t maxLength;
};

SOPC_SLinkedList* SOPC_SLinkedList_Create(size_t sizeMax)
{
    SOPC_SLinkedList* result = NULL;

    if (sizeMax <= UINT32_MAX)
    {
        result = SOPC_Calloc(1, sizeof(SOPC_SLinkedList));
        if (result != NULL)
        {
            result->maxLength = (uint32_t) sizeMax;
        }
    }
    return result;
}

uintptr_t SOPC_SLinkedList_Prepend(SOPC_SLinkedList* list, uint32_t id, uintptr_t value)
{
    if (NULL == list || 0 == value)
    {
        return 0;
    }

    SOPC_SLinkedList_Elt* elt = NULL;
    uintptr_t result = 0;

    if (list->length < list->maxLength || list->maxLength == 0)
    {
        elt = SOPC_Malloc(sizeof(SOPC_SLinkedList_Elt));
    }

    if (elt == NULL)
    {
        return 0;
    }

    // Value will be added in list, set result as valid
    result = value;

    elt->id = id;
    elt->value = (uintptr_t) value;
    elt->next = list->first;
    if (NULL == list->last)
    {
        list->last = elt;
    }
    list->first = elt;
    list->length = list->length + 1;

    return result;
}

uintptr_t SOPC_SLinkedList_Append(SOPC_SLinkedList* list, uint32_t id, uintptr_t value)
{
    SOPC_SLinkedList_Elt* elt = NULL;
    uintptr_t result = 0;

    if (NULL != list && 0 != value)
    {
        if (list->length < list->maxLength || list->maxLength == 0)
        {
            elt = SOPC_Malloc(sizeof(SOPC_SLinkedList_Elt));
        }
    }

    if (elt != NULL)
    {
        // Value will be added in list, set result as valid
        result = value;

        elt->id = id;
        elt->value = value;
        elt->next = NULL;
        if (NULL == list->first)
        {
            list->first = elt;
        }
        else
        {
            list->last->next = elt;
        }
        list->last = elt;
        list->length = list->length + 1;
    }

    return result;
}

uintptr_t SOPC_SLinkedList_SortedInsert(SOPC_SLinkedList* list,
                                        uint32_t id,
                                        uintptr_t value,
                                        int8_t (*pCompFn)(uintptr_t left, uintptr_t right))
{
    if (NULL == list)
    {
        return 0;
    }

    SOPC_SLinkedList_Elt* newElt = NULL;
    SOPC_SLinkedList_Elt* elt = NULL;
    SOPC_SLinkedList_Elt* nextElt = NULL;
    uintptr_t result = 0;
    int8_t compareResult = 0;

    if (list->length < list->maxLength || list->maxLength == 0)
    {
        newElt = SOPC_Malloc(sizeof(SOPC_SLinkedList_Elt));
    }

    if (newElt == NULL)
    {
        return 0;
    }

    newElt->id = id;
    newElt->value = value;
    newElt->next = NULL;

    // Value will be added in list, set result as valid
    result = value;

    if (list->first == NULL)
    {
        SOPC_ASSERT(list->last == NULL);
        SOPC_ASSERT(list->length == 0);
        list->first = newElt;
        list->last = newElt;
        list->length = list->length + 1;
    }
    else
    {
        // Not NULL nor empty list
        compareResult = pCompFn(value, list->first->value);
        if (compareResult < 0)
        {
            // New element shall be inserted in place of first element
            list->length = list->length + 1;
            newElt->next = list->first;
            list->first = newElt;
        }
        else
        {
            // Search position to insert new element in rest of list
            elt = list->first;
            if (elt->next != NULL)
            {
                do
                {
                    compareResult = pCompFn(value, elt->next->value);
                    if (compareResult >= 0)
                    {
                        // Position not found yet, continue to search with next element
                        elt = elt->next;
                    } // else < 0: position is before the next element, stop research
                } while (elt->next != NULL && compareResult >= 0);
            }

            if (elt->next != NULL)
            {
                // Insertion position found: insert between current element and next
                list->length = list->length + 1;
                nextElt = elt->next;
                newElt->next = nextElt;
                elt->next = newElt;
            }
            else
            {
                // Insert as last element
                SOPC_ASSERT(list->last == elt);
                list->length = list->length + 1;
                elt->next = newElt;
                list->last = newElt;
            }
        }
    }

    return result;
}

uintptr_t SOPC_SLinkedList_PopHead(SOPC_SLinkedList* list)
{
    if (NULL == list)
    {
        return 0;
    }

    uintptr_t result = 0;
    SOPC_SLinkedList_Elt* nextElt = NULL;

    if (NULL == list->first)
    {
        return 0;
    }

    // First element is researched element
    list->length = list->length - 1;
    result = list->first->value;
    nextElt = list->first->next;
    SOPC_Free(list->first);
    list->first = nextElt;
    // If no element remaining, last element to be updated too
    if (NULL == list->first)
    {
        list->last = NULL;
    }

    return result;
}

static bool SOPC_InternalSLinkedList_IsEqualToEltToRemove(SOPC_SLinkedList_Elt* left, SOPC_SLinkedList_Elt* right)
{
    return left == right;
}

static bool SOPC_InternalSLinkedList_IsEltIdEqualToEltToRemove(SOPC_SLinkedList_Elt* left, SOPC_SLinkedList_Elt* right)
{
    SOPC_ASSERT(left != NULL);
    SOPC_ASSERT(right != NULL);
    return left->id == right->id;
}

static bool SOPC_InternalSLinkedList_IsEltValueEqualToEltToRemove(SOPC_SLinkedList_Elt* left,
                                                                  SOPC_SLinkedList_Elt* right)
{
    SOPC_ASSERT(left != NULL);
    SOPC_ASSERT(right != NULL);
    return left->value == right->value;
}

static uintptr_t SOPC_SLinkedList_RemoveFromElt(SOPC_SLinkedList* list,
                                                SOPC_SLinkedList_Elt* eltToRemove,
                                                bool (*isElementFct)(SOPC_SLinkedList_Elt* left,
                                                                     SOPC_SLinkedList_Elt* right))
{
    SOPC_ASSERT(list != NULL);
    SOPC_ASSERT(eltToRemove != NULL);
    SOPC_ASSERT(isElementFct != NULL);

    SOPC_SLinkedList_Elt* elt = NULL;
    SOPC_SLinkedList_Elt* nextElt = NULL;
    uintptr_t result = 0;
    if (list->first != NULL)
    {
        // Not NULL nor empty list
        if (isElementFct(list->first, eltToRemove))
        {
            // First element is researched element
            list->length = list->length - 1;
            result = list->first->value;
            nextElt = list->first->next;
            SOPC_Free(list->first);
            list->first = nextElt;
            // If no element remaining, last element to be updated too
            if (NULL == list->first)
            {
                list->last = NULL;
            }
        }
        else
        {
            // Search element in rest of list
            elt = list->first;
            while (elt->next != NULL && false == isElementFct(elt->next, eltToRemove))
            {
                elt = elt->next;
            }
            if (elt->next != NULL)
            {
                list->length = list->length - 1;
                result = elt->next->value;
                nextElt = elt->next->next;
                // If element is the last element, then set precedent element as precedent
                if (elt->next == list->last)
                {
                    list->last = elt;
                }
                SOPC_Free(elt->next);
                elt->next = nextElt;
            }
        }
    }
    return result;
}

uintptr_t SOPC_SLinkedList_PopLast(SOPC_SLinkedList* list)
{
    if (NULL == list)
    {
        return 0;
    }

    if (list->last == NULL)
    {
        return 0;
    }
    return SOPC_SLinkedList_RemoveFromElt(list, list->last, SOPC_InternalSLinkedList_IsEqualToEltToRemove);
}

uintptr_t SOPC_SLinkedList_GetHead(SOPC_SLinkedList* list)
{
    if (NULL == list)
    {
        return 0;
    }

    if (list->first == NULL)
    {
        return 0;
    }
    return list->first->value;
}

uintptr_t SOPC_SLinkedList_GetLast(SOPC_SLinkedList* list)
{
    if (NULL == list)
    {
        return 0;
    }

    if (list->last == NULL)
    {
        return 0;
    }
    return list->last->value;
}

static SOPC_SLinkedList_Elt* SOPC_SLinkedList_InternalFind(SOPC_SLinkedList* list, uint32_t id)
{
    if (NULL == list)
    {
        return NULL;
    }

    SOPC_SLinkedList_Elt* elt = list->first;

    while (elt != NULL && elt->id != id)
    {
        elt = elt->next;
    }
    return elt;
}

// Returns null => Not found, otherwise => elt pointer
uintptr_t SOPC_SLinkedList_FindFromId(SOPC_SLinkedList* list, uint32_t id)
{
    SOPC_SLinkedList_Elt* elt = SOPC_SLinkedList_InternalFind(list, id);
    uintptr_t result = 0;
    if (elt != NULL)
    {
        result = elt->value;
    }
    return result;
}

void SOPC_SLinkedList_Apply(SOPC_SLinkedList* list, void (*pFn)(uint32_t id, uintptr_t val))
{
    if (NULL == list || NULL == pFn)
    {
        return;
    }

    SOPC_SLinkedList_Elt* elt = NULL;

    elt = list->first;
    while (NULL != elt)
    {
        pFn(elt->id, elt->value);
        elt = elt->next;
    }
}

// Returns null => Not found, otherwise => elt pointer
uintptr_t SOPC_SLinkedList_RemoveFromId(SOPC_SLinkedList* list, uint32_t id)
{
    if (NULL == list)
    {
        return 0;
    }

    SOPC_SLinkedList_Elt eltToRemove = {id, 0, NULL};

    return SOPC_SLinkedList_RemoveFromElt(list, &eltToRemove, SOPC_InternalSLinkedList_IsEltIdEqualToEltToRemove);
}

uintptr_t SOPC_SLinkedList_RemoveFromValuePtr(SOPC_SLinkedList* list, uintptr_t value)
{
    if (NULL == list || 0 == value)
    {
        return 0;
    }

    SOPC_SLinkedList_Elt eltToRemove = {0, value, NULL};

    return SOPC_SLinkedList_RemoveFromElt(list, &eltToRemove, SOPC_InternalSLinkedList_IsEltValueEqualToEltToRemove);
}

void SOPC_SLinkedList_Clear(SOPC_SLinkedList* list)
{
    SOPC_SLinkedList_Elt* elt = NULL;
    SOPC_SLinkedList_Elt* nextElt = NULL;
    if (list != NULL)
    {
        elt = list->first;
        while (elt != NULL)
        {
            nextElt = elt->next;
            SOPC_Free(elt);
            elt = nextElt;
        }
        list->first = NULL;
        list->last = NULL;
        list->length = 0;
    }
}

void SOPC_SLinkedList_Delete(SOPC_SLinkedList* list)
{
    if (list != NULL)
    {
        SOPC_SLinkedList_Clear(list);
        SOPC_Free(list);
    }
}

void SOPC_SLinkedList_EltGenericFree(uint32_t id, uintptr_t val)
{
    SOPC_UNUSED_ARG(id);
    SOPC_Free((void*) val);
}

SOPC_SLinkedListIterator SOPC_SLinkedList_GetIterator(SOPC_SLinkedList* list)
{
    if (NULL == list)
    {
        return NULL;
    }
    return list->first;
}

uintptr_t SOPC_SLinkedList_NextWithId(SOPC_SLinkedListIterator* it, uint32_t* pId)
{
    if (NULL == it)
    {
        return 0;
    }
    SOPC_SLinkedList_Elt* elt = NULL;
    uintptr_t value = 0;
    if (*it != NULL)
    {
        elt = *it;
        value = elt->value;
        if (NULL != pId)
        {
            *pId = elt->id;
        }
        *it = elt->next;
    }
    return value;
}

bool SOPC_SLinkedList_HasNext(const SOPC_SLinkedListIterator* it)
{
    if (NULL == it)
    {
        return false;
    }
    SOPC_SLinkedList_Elt* elt = NULL;
    bool result = false;
    if (*it != NULL)
    {
        elt = *it;
        result = elt->value != 0;
    }
    return result;
}

uintptr_t SOPC_SLinkedList_Next(SOPC_SLinkedListIterator* it)
{
    return SOPC_SLinkedList_NextWithId(it, NULL);
}

uint32_t SOPC_SLinkedList_GetLength(SOPC_SLinkedList* list)
{
    if (NULL == list)
    {
        return 0;
    }
    return list->length;
}

uint32_t SOPC_SLinkedList_GetCapacity(SOPC_SLinkedList* list)
{
    if (NULL == list)
    {
        return 0;
    }
    return list->maxLength;
}

bool SOPC_SLinkedList_SetCapacity(SOPC_SLinkedList* list, size_t sizeMax)
{
    if (NULL == list)
    {
        return false;
    }
    if (sizeMax > 0 && list->length > sizeMax)
    {
        return false;
    }
    list->maxLength = (uint32_t) sizeMax;
    return true;
}
