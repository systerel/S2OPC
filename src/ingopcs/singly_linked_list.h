/*
 * single_linked_list.h
 *
 *  Created on: Sep 14, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_SINGLY_LINKED_LIST_H_
#define INGOPCS_SINGLY_LINKED_LIST_H_

typedef struct SLinkedList SLinkedList;

SLinkedList* SLinkedList_Create();

// Returns null => Not found, otherwise => value pointer
void* SLinkedList_Add(SLinkedList* list, uint32_t id, void* value);

// Returns null => Not found, otherwise => value pointer
void* SLinkedList_Find(SLinkedList* list, uint32_t id);

// Returns null => Not found, otherwise => value pointer
void* SLinkedList_Remove(SLinkedList* list, uint32_t id);

void SLinkedList_Delete(SLinkedList* list);


#endif /* INGOPCS_SINGLE_LINKED_LIST_H_ */
