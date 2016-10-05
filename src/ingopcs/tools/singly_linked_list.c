/*
 * single_linked_list.c
 *
 *  Created on: Sep 14, 2016
 *      Author: vincent
 */

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

typedef struct SLinkedList_Elt{
    uint32_t                id;
    void*                   value;
    struct SLinkedList_Elt* next;
} SLinkedList_Elt;

typedef struct SLinkedList {
    SLinkedList_Elt* first;
    size_t           length;
    size_t           maxLength;
} SLinkedList;

SLinkedList* SLinkedList_Create(size_t sizeMax){
    SLinkedList* result = malloc(sizeof(SLinkedList));
    memset(result, 0, sizeof(SLinkedList));
    if(result != NULL){
        result->maxLength = sizeMax;
    }
    return result;
}

void* SLinkedList_Add(SLinkedList* list, uint32_t id, void* value){
    SLinkedList_Elt* elt = NULL;
    if(list->length < list->maxLength){
        elt = malloc(sizeof(SLinkedList_Elt));
    }
    if(elt != NULL){
        elt->id = id;
        elt->value = value;
        elt->next = list->first;
        list->first = elt;
        list->length = list->length + 1;
    }else{
        return NULL;
    }
    return value;
}

SLinkedList_Elt* SLinkedList_InternalFind(SLinkedList* list, uint32_t id){
    SLinkedList_Elt* elt = NULL;
    if(list != NULL){
        elt = list->first;
        while(elt != 0 && elt->id != id){
            elt = elt->next;
        }
    }
    return elt;
}

SLinkedList_Elt* SLinkedList_InternalFindPrec(SLinkedList* list, uint32_t id){
    SLinkedList_Elt* elt = NULL;
    if(list != NULL && list->first != NULL){
        elt = list->first;
        while(elt->next != NULL && elt->next->id != id){
            elt = elt->next;
        }
    }
    return elt;
}

// Returns null => Not found, otherwise => elt pointer
void* SLinkedList_Find(SLinkedList* list, uint32_t id){
    SLinkedList_Elt* elt = SLinkedList_InternalFind(list, id);
    void* result = NULL;
    if(elt != NULL){
        result = elt->value;
    }
    return result;
}

// Returns null => Not found, otherwise => elt pointer
void* SLinkedList_Remove(SLinkedList* list, uint32_t id){
    SLinkedList_Elt* elt = NULL;
    SLinkedList_Elt* nextElt = NULL;
    void* result = NULL;
    if(list != NULL && list->first != NULL){
        // Not NULL nor empty list
        if(list->first->id == id){
            // First element is researched element
            list->length = list->length - 1;
            result = list->first->value;
            nextElt = list->first->next;
            free(list->first);
            list->first = nextElt;
        }else{
            // Search element in rest of list
            elt = list->first;
            while(elt->next != NULL && elt->next->id != id){
                elt = elt->next;
            }
            if(elt->next != NULL){
                list->length = list->length - 1;
                result = elt->next->value;
                nextElt = elt->next->next;
                free(elt->next);
                elt->next = nextElt;
            }
        }
    }
    return result;
}

void SLinkedList_Delete(SLinkedList* list){
    SLinkedList_Elt* elt = NULL;
    SLinkedList_Elt* nextElt = NULL;
    if(list != NULL){
        elt = list->first;
        while(elt != NULL){
            nextElt = elt->next;
            free(elt);
            elt = nextElt;
        }
        free(list);
    }
}
