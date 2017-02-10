/*
 *  Copyright (C) 2017 Systerel and others.
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

#include "sopc_action_queue.h"

#include <stdlib.h>

#include "singly_linked_list.h"
#include "sopc_mutexes.h"

struct SOPC_ActionQueue {
    SLinkedList* queueList;
    Condition    queueCond;
    Mutex        queueMutex;
    uint32_t     waitingThreads;
};

typedef struct {
    SOPC_ActionFunction* fctPointer;
    void*                fctArgument;
    const char*          debugTxt;
} SOPC_ActionEvent;

SOPC_StatusCode SOPC_ActionQueue_Init(SOPC_ActionQueue** queue){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(NULL != queue){
        *queue = malloc(sizeof(SOPC_ActionQueue));
        if(*queue != NULL){
            status = STATUS_OK;
            (*queue)->waitingThreads = 0;
            (*queue)->queueList = SLinkedList_Create(0);
            if(NULL == (*queue)->queueList){
                status = STATUS_NOK;
            }
            if(STATUS_OK == status){
                status = Condition_Init(&(*queue)->queueCond);
                if(STATUS_OK != status){
                    SLinkedList_Delete((*queue)->queueList);
                    (*queue)->queueList = NULL;
                }
            }
            if(STATUS_OK == status){
                status = Mutex_Initialization(&(*queue)->queueMutex);
                if(STATUS_OK != status){
                    SLinkedList_Delete((*queue)->queueList);
                    (*queue)->queueList = NULL;
                    Condition_Clear(&(*queue)->queueCond);
                }
            }
            if(STATUS_OK != status){
                free(*queue);
                *queue = NULL;
            }
        }
    }
    return status;
}

SOPC_StatusCode SOPC_Action_BlockingEnqueue(SOPC_ActionQueue*    queue,
                                            SOPC_ActionFunction* fctPointer,
                                            void*                fctArgument,
                                            const char*          actionText)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_ActionEvent* event;
    if(NULL != queue && (NULL != fctPointer || NULL != fctArgument)){
        status = STATUS_NOK;
        event = calloc(1, sizeof(SOPC_ActionEvent));
        if(NULL != event){
            event->fctPointer = fctPointer;
            event->fctArgument = fctArgument;
            event->debugTxt = actionText;
            status = Mutex_Lock(&queue->queueMutex);
            if(STATUS_OK == status){
                if(event == SLinkedList_Append(queue->queueList, 0, event)){
                    if(queue->waitingThreads > 0){
                        Condition_SignalAll(&queue->queueCond);
                    }
                }else{
                    status = STATUS_NOK;
                }
                Mutex_Unlock(&queue->queueMutex);
            }
        }
    }
    return status;
}

SOPC_StatusCode SOPC_ActionDequeue(SOPC_ActionQueue*     queue,
                                   uint8_t               isBlocking,
                                   SOPC_ActionFunction** fctPointer,
                                   void**                fctArgument,
                                   const char**          actionText){
    SOPC_ActionEvent* event = NULL;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(NULL != queue && NULL != fctPointer && NULL != fctArgument){
        status = STATUS_NOK;
        Mutex_Lock(&queue->queueMutex);
        event = (SOPC_ActionEvent*) SLinkedList_PopHead(queue->queueList);
        if(NULL == event){
            if(isBlocking == FALSE){
                status = OpcUa_BadWouldBlock;
            }else{
                queue->waitingThreads++;
                event = (SOPC_ActionEvent*) SLinkedList_PopHead(queue->queueList);
                while(NULL == event){
                    Mutex_UnlockAndWaitCond(&queue->queueCond, &queue->queueMutex);
                    event = (SOPC_ActionEvent*) SLinkedList_PopHead(queue->queueList);
                }
                status = STATUS_OK;
                queue->waitingThreads--;
            }
        }else{
            status = STATUS_OK;
        }
        if(STATUS_OK == status && NULL != event){
            *fctPointer = event->fctPointer;
            *fctArgument = event->fctArgument;
            if(NULL != actionText){
                *actionText = event->debugTxt;
            }
            free(event);
            event = NULL;
        }
        Mutex_Unlock(&queue->queueMutex);
    }
    return status;
}

SOPC_StatusCode SOPC_Action_BlockingDequeue(SOPC_ActionQueue*     queue,
                                            SOPC_ActionFunction** fctPointer,
                                            void**                fctArgument,
                                            const char**          actionText)
{
    return SOPC_ActionDequeue(queue, 1, fctPointer, fctArgument, actionText);
}

SOPC_StatusCode SOPC_Action_NonBlockingDequeue(SOPC_ActionQueue*     queue,
                                               SOPC_ActionFunction** fctPointer,
                                               void**                fctArgument,
                                               const char**          actionText)
{
    return SOPC_ActionDequeue(queue, FALSE, fctPointer, fctArgument, actionText);
}

void SOPC_ActionQueue_Free(SOPC_ActionQueue** queue){
    if(NULL != queue){
        if(NULL != *queue && NULL != (*queue)->queueList){
            SLinkedList_Apply((*queue)->queueList, SLinkedList_EltGenericFree);
            SLinkedList_Delete((*queue)->queueList);
        }
        free(*queue);
        *queue = NULL;
    }
}
