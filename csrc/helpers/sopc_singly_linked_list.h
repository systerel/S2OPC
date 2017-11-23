/**
 * \file sopc_singly_linked_list.h
 *
 *  \brief A singly linked list based on elements with unique identifiers and dynamically allocated.
 */
/*
 *  Copyright (C) 2016 Systerel and others.
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

#ifndef SOPC_SINGLY_LINKED_LIST_H_
#define SOPC_SINGLY_LINKED_LIST_H_

#include <stdint.h>
#include <stdlib.h>

/**
 *  \brief Singly linked list structure
 */
typedef struct SOPC_SLinkedList SOPC_SLinkedList;

typedef struct SOPC_SLinkedList_Elt SOPC_SLinkedList_Elt;

typedef SOPC_SLinkedList_Elt* SOPC_SLinkedListIterator;

/**
 *  \brief            Create and allocate a new singly linked list containing 0 elements with a size limit of the given
 * size.
 *
 *  \param sizeMax    The maximum number of elements allowed in the new linked list or 0 if no limit defined
 *  \return           Pointer to the newly allocated singly linked list
 */
SOPC_SLinkedList* SOPC_SLinkedList_Create(size_t sizeMax);

/**
 *  \brief          Add a new element (and allocate new list element) before head of the given linked list.
 *
 *  \param list     Pointer on the linked list in which new element must be added
 *  \param id       Unique identifier to associate with the element (if not unique prepend LIFO behavior for Find and
 * Remove) \param value    Pointer to the value of the element to prepend
 *
 *  \return         Pointer to the value prepended, provided as parameter, if succeeded, NULL otherwise
 */
void* SOPC_SLinkedList_Prepend(SOPC_SLinkedList* list, uint32_t id, void* value);

/**
 *  \brief          Add a new element (and allocate new list element) to the tail of the given linked list.
 *
 *  \param list     Pointer on the linked list in which new element must be added
 *  \param id       Unique identifier to associate with the element (if not unique append FIFO behavior for Find and
 * Remove) \param value    Pointer to the value of the element to append
 *
 *  \return         Pointer to the value appended, provided as parameter, if succeeded, NULL otherwise
 */
void* SOPC_SLinkedList_Append(SOPC_SLinkedList* list, uint32_t id, void* value);

/**
 * \brief           Insert element in sorted list in correct index regarding compare function.
 *                  The element will be inserted before the element for which the compare function return that new
 * element is < to the existing element (compare returns -1 when new element is left operand and < to right operand).
 *
 *                  Important note: the provided list shall be sorted regarding the same compare function.
 *
 * \param list      Pointer to the linked list
 * \param id        Identifier of the given value
 * \param value     Value to insert in the sorted list
 * \param pFn       Compare function pointer returning a int8_t equals to -1 if left value < right value, 0 if left
 * value == right value and 1 if left value > right value
 *
 *  \return         Pointer to the value insterted, provided as parameter, if succeeded, NULL otherwise
 */
void* SOPC_SLinkedList_SortedInsert(SOPC_SLinkedList* list,
                                    uint32_t id,
                                    void* value,
                                    int8_t (*pCompFn)(void* left, void* right));

/**
 *  \brief          Get and remove the head element of the list
 *
 *  \param list     Pointer on the linked list from which head element must be returned and removed
 *
 *  \return         Pointer to the head element value of the list
 */
void* SOPC_SLinkedList_PopHead(SOPC_SLinkedList* list);

/**
 *  \brief          Find the value associated to the given id in the linked list
 *
 *  \param list     Pointer on the linked list in which element must be found
 *  \param id       Unique identifier associated with the element to find
 *
 *  \return         Pointer to the value found if succeeded, NULL otherwise
 */
void* SOPC_SLinkedList_FindFromId(SOPC_SLinkedList* list, uint32_t id);

/**
 * \brief           Apply a function to the value of each element of the list.
 *
 *                  An example is the SOPC_SLinkedList_EltGenericFree() function which frees the \p void* \p value
 *                  of each element of the list.
 *
 * \param list      Pointer to the linked list
 * \param pFn       Function pointer which takes the id and the value of each element
 */
void SOPC_SLinkedList_Apply(SOPC_SLinkedList* list, void (*pFn)(uint32_t id, void* val));

/**
 *  \brief          Find and remove the value associated to the given id in the linked list
 *
 *  \param list     Pointer on the linked list in which element must be found
 *  \param id       Unique identifier associated with the element to remove
 *
 *  \return         Pointer to the value removed if succeeded, NULL otherwise
 */
void* SOPC_SLinkedList_RemoveFromId(SOPC_SLinkedList* list, uint32_t id);

/**
 *  \brief         Delete all elements of the given linked list
 *
 *  \param list    Pointer to the list of elements to be deleted
 */
void SOPC_SLinkedList_Clear(SOPC_SLinkedList* list);

/**
 *  \brief         Delete and deallocate the given linked list
 *
 *  \param list    Pointer to the list to deallocate (pointer must not be used anymore after operation)
 */
void SOPC_SLinkedList_Delete(SOPC_SLinkedList* list);

/**
 * \brief           Frees the value of an element of the SOPC_SLinkedList.
 *
 *  \param id       Unique identifier associated with the element
 *  \param val      Element to be freed
 */
void SOPC_SLinkedList_EltGenericFree(uint32_t id, void* val);

/**
 * \brief           Get an iterator on a linked list to could iterate on elements (LIFO behavior)
 *
 * \param list      Pointer to the list for which an iterator is requested
 *
 * \return          An iterator on the given linked list
 */
SOPC_SLinkedListIterator SOPC_SLinkedList_GetIterator(SOPC_SLinkedList* list);

/**
 * \brief           Return the next element pointed by iterator in the linked list  (LIFO behavior)
 *
 * \param it        An iterator on a linked list
 */
void* SOPC_SLinkedList_Next(SOPC_SLinkedListIterator* it);

/**
 * \brief           Get then number of elements in the linked list
 *
 * \param list      Pointer to the list
 *
 * \return          The number of elements in the list
 */
uint32_t SOPC_SLinkedList_GetLength(SOPC_SLinkedList* list);

#endif /* SOPC_SINGLE_LINKED_LIST_H_ */
