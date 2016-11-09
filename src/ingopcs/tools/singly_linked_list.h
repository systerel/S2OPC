/**
 * \file single_linked_list.h
 *
 *  \brief A singly linked list based on elements with unique identifiers and dynamically allocated.
 *
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

/**
 *  \brief Singly linked list structure
 */
typedef struct SLinkedList SLinkedList;

/**
 *  \brief            Create and allocate a new singly linked list containing 0 elements with a size limit of the given size.
 *
 *  \param sizeMax    The maximum number of elements allowed in the new linked list
 *  \return           Pointer to the newly allocated singly linked list
 */
SLinkedList* SLinkedList_Create(size_t sizeMax);

/**
 *  \brief          Add a new element (and allocate new list element) in the given linked list.
 *
 *  \param list     Pointer on the linked list in which new element must be added
 *  \param id       Unique identifier to associate with the element (if not unique LIFO mode is applied for Find and Remove)
 *  \param value    Pointer to the value of the element to add
 *
 *  \return         Pointer to the value added, provided as parameter, if succeeded, NULL otherwise
 */
void* SLinkedList_Add(SLinkedList* list, uint32_t id, void* value);

/**
 *  \brief          Find the value associated to the given id in the linked list
 *
 *  \param list     Pointer on the linked list in which element must be found
 *  \param id       Unique identifier associated with the element to find
 *
 *  \return         Pointer to the value found if succeeded, NULL otherwise
 */
void* SLinkedList_Find(SLinkedList* list, uint32_t id);

/**
 *  \brief          Find and remove the value associated to the given id in the linked list
 *
 *  \param list     Pointer on the linked list in which element must be found
 *  \param id       Unique identifier associated with the element to remove
 *
 *  \return         Pointer to the value removed if succeeded, NULL otherwise
 */
void* SLinkedList_Remove(SLinkedList* list, uint32_t id);

/**
 *  \brief         Delete all elements of the given linked list
 *
 *  \param list    Pointer to the list of elements to be deleted
 */
void SLinkedList_Clear(SLinkedList* list);

/**
 *  \brief         Delete and deallocate the given linked list
 *
 *  \param list    Pointer to the list to deallocate (pointer must not be used anymore after operation)
 */
void SLinkedList_Delete(SLinkedList* list);


#endif /* SOPC_SINGLE_LINKED_LIST_H_ */
