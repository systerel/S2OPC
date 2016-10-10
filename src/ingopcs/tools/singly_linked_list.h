/*
 * \file single_linked_list.h
 *
 *  \brief A singly linked list based on elements with unique identifiers and dynamically allocated.
 *
 *  Created on: Sep 14, 2016
 *      Author: VMO (Systerel)
 */

#ifndef INGOPCS_SINGLY_LINKED_LIST_H_
#define INGOPCS_SINGLY_LINKED_LIST_H_

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
 *  \brief         Delete and deallocate the given linked list
 *
 *  \param list    Pointer to the list to deallocate (pointer must not be used anymore after operation)
 */
void SLinkedList_Delete(SLinkedList* list);


#endif /* INGOPCS_SINGLE_LINKED_LIST_H_ */
