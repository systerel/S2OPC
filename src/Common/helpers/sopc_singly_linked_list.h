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
 * \file sopc_singly_linked_list.h
 *
 *  \brief A singly linked list based on elements with unique identifiers and dynamically allocated.
 */

#ifndef SOPC_SINGLY_LINKED_LIST_H_
#define SOPC_SINGLY_LINKED_LIST_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 *  \brief Singly linked list structure
 */
typedef struct SOPC_SLinkedList SOPC_SLinkedList;

typedef struct SOPC_SLinkedList_Elt SOPC_SLinkedList_Elt;

typedef SOPC_SLinkedList_Elt* SOPC_SLinkedListIterator;

/**
 *  \brief            Create and allocate a new singly linked list containing 0 elements with a size limit of the given
 *                    size.
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
 *                  Remove)
 *  \param value    Pointer to the value of the element to prepend
 *
 *  \return         Pointer to the value prepended, provided as parameter, if succeeded, NULL otherwise
 */
void* SOPC_SLinkedList_Prepend(SOPC_SLinkedList* list, uint32_t id, void* value);

/**
 *  \brief          Add a new element (and allocate new list element) to the tail of the given linked list.
 *
 *  \param list     Pointer on the linked list in which new element must be added
 *  \param id       Unique identifier to associate with the element (if not unique append FIFO behavior for Find and
 *                  Remove)
 *  \param value    Pointer to the value of the element to append
 *
 *  \return         Pointer to the value appended, provided as parameter, if succeeded, NULL otherwise
 */
void* SOPC_SLinkedList_Append(SOPC_SLinkedList* list, uint32_t id, void* value);

/**
 * \brief           Insert element in sorted list in correct index regarding compare function.
 *
 *   The element will be inserted before the element for which the compare function return that new element is < to the
 *   existing element (compare returns -1 when new element is left operand and < to right operand).
 *
 * \note            Important: the provided list shall be sorted regarding the same compare function.
 *
 * \param list      Pointer to the linked list
 * \param id        Identifier of the given value
 * \param value     Value to insert in the sorted list
 * \param pCompFn   Compare function pointer returning a int8_t equals to -1 if left value < right value, 0 if left
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
 *  \brief          Get and remove the last element of the list
 *  \note           This function iterate on the whole list to remove the last element
 *
 *  \param list     Pointer on the linked list from which head element must be returned and removed
 *
 *  \return         Pointer to the last element value of the list
 */
void* SOPC_SLinkedList_PopLast(SOPC_SLinkedList* list);

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
 *   An example is the SOPC_SLinkedList_EltGenericFree() function which frees the \p void* \p value
 *   of each element of the list.
 *
 * \param list      Pointer to the linked list
 * \param pFn       Function pointer which takes the id and the value of each element
 */
void SOPC_SLinkedList_Apply(SOPC_SLinkedList* list, void (*pFn)(uint32_t id, void* val));

/**
 *  \brief          Find and remove the first value associated to the given id (FIFO) in the linked list
 *
 *  \param list     Pointer on the linked list in which element must be found
 *  \param id       Unique identifier associated with the element to remove
 *
 *  \return         Pointer to the value removed if succeeded, NULL otherwise
 */
void* SOPC_SLinkedList_RemoveFromId(SOPC_SLinkedList* list, uint32_t id);

/**
 *  \brief          Find and remove the first value pointer equal (FIFO) in the linked list
 *
 *  \param list     Pointer on the linked list in which element must be found
 *  \param value    Pointer on the value to remove from list
 *
 *  \return         Pointer to the value removed if succeeded, NULL otherwise
 */
void* SOPC_SLinkedList_RemoveFromValuePtr(SOPC_SLinkedList* list, void* value);

/**
 *  \brief          Delete all elements of the given linked list
 *
 *  \param list     Pointer to the list of elements to be deleted
 */
void SOPC_SLinkedList_Clear(SOPC_SLinkedList* list);

/**
 *  \brief          Delete and deallocate the given linked list
 *
 *  \param list     Pointer to the list to deallocate (pointer must not be used anymore after operation)
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
 * \brief           Get an iterator on a linked list to iterate on elements (LIFO behavior)
 *
 * \param list      Pointer to the list for which an iterator is requested
 *
 * \return          An iterator on the given linked list
 */
SOPC_SLinkedListIterator SOPC_SLinkedList_GetIterator(SOPC_SLinkedList* list);

/**
 * \brief           Return true if iterator has a non NULL value to provide on next iteration
 *
 * \param it        An iterator on a linked list
 *
 * \return          true if iterator has a non NULL value to provide on next iteration, false otherwise
 */
bool SOPC_SLinkedList_HasNext(const SOPC_SLinkedListIterator* it);

/**
 * \brief           Return the next element pointed by iterator in the linked list  (LIFO behavior)
 *
 * \param it        An iterator on a linked list
 *
 * \return          Pointer on the next value of the linked list
 */
void* SOPC_SLinkedList_Next(SOPC_SLinkedListIterator* it);

/**
 * \brief           Return the next element pointed by iterator in the linked list  (LIFO behavior)
 *
 * \param it        An iterator on a linked list
 * \param pId       Pointer in which the next element id of the linked list is set
 *
 * \return          Pointer on the next value of the linked list
 */
void* SOPC_SLinkedList_NextWithId(SOPC_SLinkedListIterator* it, uint32_t* pId);

/**
 * \brief           Get then number of elements in the linked list
 *
 * \param list      Pointer to the list
 *
 * \return          The number of elements in the list
 */
uint32_t SOPC_SLinkedList_GetLength(SOPC_SLinkedList* list);

#endif /* SOPC_SINGLE_LINKED_LIST_H_ */
