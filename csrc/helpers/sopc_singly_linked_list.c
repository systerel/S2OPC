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

#include "sopc_singly_linked_list.h"

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

struct SOPC_SLinkedList_Elt{
    uint32_t                id;
    void*                   value;
    struct SOPC_SLinkedList_Elt* next;
};

struct SOPC_SLinkedList {
    SOPC_SLinkedList_Elt* first;
    SOPC_SLinkedList_Elt* last;
    size_t           length;
    size_t           maxLength;
};

SOPC_SLinkedList* SOPC_SLinkedList_Create(size_t sizeMax){
    SOPC_SLinkedList* result = malloc(sizeof(SOPC_SLinkedList));
    if(result != NULL){
        memset(result, 0, sizeof(SOPC_SLinkedList));
        result->maxLength = sizeMax;
    }
    return result;
}

void* SOPC_SLinkedList_Prepend(SOPC_SLinkedList* list, uint32_t id, void* value){
    SOPC_SLinkedList_Elt* elt = NULL;
    if(NULL == list){
        return NULL;
    }

    if(list->length < list->maxLength || list->maxLength == 0){
        elt = malloc(sizeof(SOPC_SLinkedList_Elt));
    }

    if(elt != NULL){
        elt->id = id;
        elt->value = value;
        elt->next = list->first;
        if(NULL == list->last){
            list->last = elt;
        }
        list->first = elt;
        list->length = list->length + 1;
    }else{
        return NULL;
    }

    return value;
}

void* SOPC_SLinkedList_Append(SOPC_SLinkedList* list, uint32_t id, void* value){
    SOPC_SLinkedList_Elt* elt = NULL;
    if(NULL == list){
        return NULL;
    }

    if(list->length < list->maxLength || list->maxLength == 0){
        elt = malloc(sizeof(SOPC_SLinkedList_Elt));
    }

    if(elt != NULL){
        elt->id = id;
        elt->value = value;
        elt->next = NULL;
        if(NULL == list->first){
            list->first = elt;
        }else{
            list->last->next = elt;
        }
        list->last = elt;
        list->length = list->length + 1;
    }else{
        return NULL;
    }

    return value;
}

void* SOPC_SLinkedList_PopHead(SOPC_SLinkedList* list){
    void* result = NULL;
    SOPC_SLinkedList_Elt* nextElt = NULL;

    if(NULL == list || NULL == list->first){
       return NULL;
    }

    // First element is researched element
    list->length = list->length - 1;
    result = list->first->value;
    nextElt = list->first->next;
    free(list->first);
    list->first = nextElt;
    // If no element remaining, last element to be updated too
    if(NULL == list->first){
        list->last = NULL;
    }

    return result;
}

SOPC_SLinkedList_Elt* SOPC_SLinkedList_InternalFind(SOPC_SLinkedList* list, uint32_t id){
    SOPC_SLinkedList_Elt* elt = NULL;
    if(list != NULL){
        elt = list->first;
        while(elt != 0 && elt->id != id){
            elt = elt->next;
        }
    }
    return elt;
}

SOPC_SLinkedList_Elt* SOPC_SLinkedList_InternalFindPrec(SOPC_SLinkedList* list, uint32_t id){
    SOPC_SLinkedList_Elt* elt = NULL;
    if(list != NULL && list->first != NULL){
        elt = list->first;
        while(elt->next != NULL && elt->next->id != id){
            elt = elt->next;
        }
    }
    return elt;
}

// Returns null => Not found, otherwise => elt pointer
void* SOPC_SLinkedList_FindFromId(SOPC_SLinkedList* list, uint32_t id){
    SOPC_SLinkedList_Elt* elt = SOPC_SLinkedList_InternalFind(list, id);
    void* result = NULL;
    if(elt != NULL){
        result = elt->value;
    }
    return result;
}

void SOPC_SLinkedList_Apply(SOPC_SLinkedList* list, void (*pFn)(uint32_t id, void *val))
{
    SOPC_SLinkedList_Elt* elt = NULL;

    if(NULL == list || NULL == pFn)
        return;

    elt = list->first;
    while(NULL != elt)
    {
        pFn(elt->id, elt->value);
        elt = elt->next;
    }
}

// Returns null => Not found, otherwise => elt pointer
void* SOPC_SLinkedList_RemoveFromId(SOPC_SLinkedList* list, uint32_t id){
    SOPC_SLinkedList_Elt* elt = NULL;
    SOPC_SLinkedList_Elt* nextElt = NULL;
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
            // If no element remaining, last element to be updated too
            if(NULL == list->first){
                list->last = NULL;
            }
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
                // If element is the last element, then set precedent element as precedent
                if(elt->next == list->last){
                    list->last = elt;
                }
                free(elt->next);
                elt->next = nextElt;
            }
        }
    }
    return result;
}

void SOPC_SLinkedList_Clear(SOPC_SLinkedList* list){
    SOPC_SLinkedList_Elt* elt = NULL;
    SOPC_SLinkedList_Elt* nextElt = NULL;
    if(list != NULL){
        elt = list->first;
        while(elt != NULL){
            nextElt = elt->next;
            free(elt);
            elt = nextElt;
        }
        list->first = NULL;
        list->last = NULL;
        list->length = 0;
    }
}

void SOPC_SLinkedList_Delete(SOPC_SLinkedList* list){
    if(list != NULL){
        SOPC_SLinkedList_Clear(list);
        free(list);
    }
}


void SOPC_SLinkedList_EltGenericFree(uint32_t id, void *val)
{
    (void)(id);
    free(val);
}

SOPC_SLinkedListIterator SOPC_SLinkedList_GetIterator(SOPC_SLinkedList* list){
    return list->first;
}

void* SOPC_SLinkedList_Next(SOPC_SLinkedListIterator* it){
    SOPC_SLinkedList_Elt* elt = NULL;
    void* value = NULL;
    if(it != NULL && *it != NULL){
        elt = *it;
        value = elt->value;
        *it = elt->next;
    }
    return value;
}
