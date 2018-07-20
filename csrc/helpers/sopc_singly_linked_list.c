/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sopc_singly_linked_list.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct SOPC_SLinkedList_Elt
{
    uint32_t id;
    void* value;
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
    SOPC_SLinkedList* result = malloc(sizeof(SOPC_SLinkedList));
    if (result != NULL)
    {
        memset(result, 0, sizeof(SOPC_SLinkedList));
        result->maxLength = sizeMax;
    }
    return result;
}

void* SOPC_SLinkedList_Prepend(SOPC_SLinkedList* list, uint32_t id, void* value)
{
    SOPC_SLinkedList_Elt* elt = NULL;
    void* result = NULL;

    if (NULL != list && NULL != value)
    {
        if (list->length < list->maxLength || list->maxLength == 0)
        {
            elt = malloc(sizeof(SOPC_SLinkedList_Elt));
        }
    }

    if (elt != NULL)
    {
        // Value will be added in list, set result as valid
        result = value;

        elt->id = id;
        elt->value = value;
        elt->next = list->first;
        if (NULL == list->last)
        {
            list->last = elt;
        }
        list->first = elt;
        list->length = list->length + 1;
    }

    return result;
}

void* SOPC_SLinkedList_Append(SOPC_SLinkedList* list, uint32_t id, void* value)
{
    SOPC_SLinkedList_Elt* elt = NULL;
    void* result = NULL;

#ifdef __TRUSTINSOFT_HELPER__
    // use tis_variable_split to handle malloc-returns-null in sockets
#ifdef TEST_SOCKETS
    void tis_variable_split(void *__p, size_t __s, int __limit) __THROW;
    tis_variable_split (&(list), sizeof(list), 20);
#endif
#endif
    if (NULL != list && NULL != value)
    {
        if (list->length < list->maxLength || list->maxLength == 0)
        {
            elt = malloc(sizeof(SOPC_SLinkedList_Elt));
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
#ifdef __TRUSTINSOFT_HELPER__
            // try to remove NULL value
            //@ assert slla_first_not_null: wp: list->first != \null;
#endif
        }
        else
        {
#ifdef __TRUSTINSOFT_HELPER__
            // try to remove NULL value
            //@ assert slla_last_not_null: wp: list->last != \null;
#endif
            list->last->next = elt;
        }
        list->last = elt;
        list->length = list->length + 1;
    }

    return result;
}

void* SOPC_SLinkedList_SortedInsert(SOPC_SLinkedList* list,
                                    uint32_t id,
                                    void* value,
                                    int8_t (*pCompFn)(void* left, void* right))
{
    SOPC_SLinkedList_Elt* newElt = NULL;
    SOPC_SLinkedList_Elt* elt = NULL;
    SOPC_SLinkedList_Elt* nextElt = NULL;
    void* result = NULL;
    int8_t compareResult = 0;

    if (NULL != list && NULL != value)
    {
        if (list->length < list->maxLength || list->maxLength == 0)
        {
            newElt = malloc(sizeof(SOPC_SLinkedList_Elt));
        }
    }

    if (newElt != NULL)
    {
        newElt->id = id;
        newElt->value = value;
        newElt->next = NULL;
    }

    if (newElt != NULL)
    {
        // Value will be added in list, set result as valid
        result = value;

        if (list->first == NULL)
        {
            assert(list->last == NULL);
            assert(list->length == 0);
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
                    assert(list->last == elt);
                    list->length = list->length + 1;
                    elt->next = newElt;
                    list->last = newElt;
                }
            }
        }
    }
    return result;
}

void* SOPC_SLinkedList_PopHead(SOPC_SLinkedList* list)
{
    void* result = NULL;
    SOPC_SLinkedList_Elt* nextElt = NULL;

#ifdef __TRUSTINSOFT_HELPER__
    // use tis_variable_split to handle malloc-returns-null in sockets
#ifdef TEST_SOCKETS
    void tis_variable_split(void *__p, size_t __s, int __limit) __THROW;
    tis_variable_split (&(list), sizeof(list), 20);
#endif
#endif
    if (NULL == list || NULL == list->first)
    {
        return NULL;
    }

    // First element is researched element
    list->length = list->length - 1;
    result = list->first->value;
    nextElt = list->first->next;
    free(list->first);
    list->first = nextElt;
    // If no element remaining, last element to be updated too
    if (NULL == list->first)
    {
        list->last = NULL;
    }

    return result;
}

SOPC_SLinkedList_Elt* SOPC_SLinkedList_InternalFind(SOPC_SLinkedList* list, uint32_t id)
{
    SOPC_SLinkedList_Elt* elt = NULL;
    if (list != NULL)
    {
        elt = list->first;
        while (elt != 0 && elt->id != id)
        {
            elt = elt->next;
        }
    }
    return elt;
}

// Returns null => Not found, otherwise => elt pointer
void* SOPC_SLinkedList_FindFromId(SOPC_SLinkedList* list, uint32_t id)
{
    SOPC_SLinkedList_Elt* elt = SOPC_SLinkedList_InternalFind(list, id);
    void* result = NULL;
    if (elt != NULL)
    {
        result = elt->value;
    }
    return result;
}

void SOPC_SLinkedList_Apply(SOPC_SLinkedList* list, void (*pFn)(uint32_t id, void* val))
{
    SOPC_SLinkedList_Elt* elt = NULL;

    if (NULL == list || NULL == pFn)
        return;

    elt = list->first;
    while (NULL != elt)
    {
        pFn(elt->id, elt->value);
        elt = elt->next;
    }
}

// Returns null => Not found, otherwise => elt pointer
void* SOPC_SLinkedList_RemoveFromId(SOPC_SLinkedList* list, uint32_t id)
{
    SOPC_SLinkedList_Elt* elt = NULL;
    SOPC_SLinkedList_Elt* nextElt = NULL;
    void* result = NULL;
    if (list != NULL && list->first != NULL)
    {
        // Not NULL nor empty list
        if (list->first->id == id)
        {
            // First element is researched element
            list->length = list->length - 1;
            result = list->first->value;
            nextElt = list->first->next;
            free(list->first);
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
            while (elt->next != NULL && elt->next->id != id)
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
                free(elt->next);
                elt->next = nextElt;
            }
        }
    }
    return result;
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
            free(elt);
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
        free(list);
    }
}

void SOPC_SLinkedList_EltGenericFree(uint32_t id, void* val)
{
    (void) (id);
    free(val);
}

SOPC_SLinkedListIterator SOPC_SLinkedList_GetIterator(SOPC_SLinkedList* list)
{
    return list->first;
}

void* SOPC_SLinkedList_NextWithId(SOPC_SLinkedListIterator* it, uint32_t* pId)
{
    SOPC_SLinkedList_Elt* elt = NULL;
    void* value = NULL;
    if (it != NULL && *it != NULL)
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

void* SOPC_SLinkedList_Next(SOPC_SLinkedListIterator* it)
{
    return SOPC_SLinkedList_NextWithId(it, NULL);
}

uint32_t SOPC_SLinkedList_GetLength(SOPC_SLinkedList* list)
{
    uint32_t result = 0;
    if (NULL != list)
    {
        result = list->length;
    } // else 0 elements in the list
    return result;
}
