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

#include "sopc_async_queue.h"

#include <stdlib.h>
#include <stdbool.h>

#include "sopc_singly_linked_list.h"
#include "sopc_mutexes.h"

struct SOPC_AsyncQueue {
    const char*       debugQueueName;
    SOPC_SLinkedList* queueList;
    Condition         queueCond;
    Mutex             queueMutex;
    uint32_t          waitingThreads;
};

SOPC_StatusCode SOPC_AsyncQueue_Init(SOPC_AsyncQueue** queue, const char*  queueName){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(NULL != queue){
        *queue = malloc(sizeof(SOPC_AsyncQueue));
        if(*queue != NULL){
            status = STATUS_OK;
            (*queue)->debugQueueName = queueName;
            (*queue)->waitingThreads = 0;
            (*queue)->queueList = SOPC_SLinkedList_Create(0);
            if(NULL == (*queue)->queueList){
                status = STATUS_NOK;
            }
            if(STATUS_OK == status){
                status = Condition_Init(&(*queue)->queueCond);
                if(STATUS_OK != status){
                    SOPC_SLinkedList_Delete((*queue)->queueList);
                    (*queue)->queueList = NULL;
                }
            }
            if(STATUS_OK == status){
                status = Mutex_Initialization(&(*queue)->queueMutex);
                if(STATUS_OK != status){
                    SOPC_SLinkedList_Delete((*queue)->queueList);
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

static SOPC_StatusCode SOPC_AsyncQueue_BlockingEnqueueFirstOrLast(SOPC_AsyncQueue* queue,
                                                                  void*            element,
                                                                  bool             firstOut)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    void* enqueuedElt = NULL;
    if(NULL != queue && NULL != element){
        status = Mutex_Lock(&queue->queueMutex);
        if(STATUS_OK == status){
            if(firstOut == false){
                enqueuedElt = SOPC_SLinkedList_Append(queue->queueList, 0, element);
            }else{
                enqueuedElt = SOPC_SLinkedList_Prepend(queue->queueList, 0, element);
            }
            if(element == enqueuedElt){
                if(queue->waitingThreads > 0){
                    Condition_SignalAll(&queue->queueCond);
                }
            }else{
                status = STATUS_NOK;
            }
            Mutex_Unlock(&queue->queueMutex);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_AsyncQueue_BlockingEnqueueFirstOut(SOPC_AsyncQueue* queue,
                                                        void*            element)
{
    return SOPC_AsyncQueue_BlockingEnqueueFirstOrLast(queue,
                                                      element,
                                                      true);
}

SOPC_StatusCode SOPC_AsyncQueue_BlockingEnqueue(SOPC_AsyncQueue* queue,
                                                void*            element)
{
    return SOPC_AsyncQueue_BlockingEnqueueFirstOrLast(queue,
                                                      element,
                                                      false);
}

static SOPC_StatusCode SOPC_AsyncQueue_Dequeue(SOPC_AsyncQueue* queue,
                                               uint8_t          isBlocking,
                                               void**           element){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(NULL != queue && NULL != element){
        status = STATUS_NOK;
        Mutex_Lock(&queue->queueMutex);
        *element = SOPC_SLinkedList_PopHead(queue->queueList);
        if(NULL == *element){
            if(isBlocking == FALSE){
                status = OpcUa_BadWouldBlock;
            }else{
                queue->waitingThreads++;
                *element = SOPC_SLinkedList_PopHead(queue->queueList);
                while(NULL == *element){
                    Mutex_UnlockAndWaitCond(&queue->queueCond, &queue->queueMutex);
                    *element = SOPC_SLinkedList_PopHead(queue->queueList);
                }
                status = STATUS_OK;
                queue->waitingThreads--;
            }
        }else{
            status = STATUS_OK;
        }
        Mutex_Unlock(&queue->queueMutex);
    }
    return status;
}

SOPC_StatusCode SOPC_AsyncQueue_BlockingDequeue(SOPC_AsyncQueue* queue,
                                                void**           element)
{
    return SOPC_AsyncQueue_Dequeue(queue, !FALSE, element);
}

SOPC_StatusCode SOPC_AsyncQueue_NonBlockingDequeue(SOPC_AsyncQueue* queue,
                                                   void**           element)
{
    return SOPC_AsyncQueue_Dequeue(queue, FALSE, element);
}

void SOPC_AsyncQueue_Free(SOPC_AsyncQueue** queue){
    if(NULL != queue){
        if(NULL != *queue && NULL != (*queue)->queueList){
            SOPC_SLinkedList_Apply((*queue)->queueList, SOPC_SLinkedList_EltGenericFree);
            SOPC_SLinkedList_Delete((*queue)->queueList);
        }
        free(*queue);
        *queue = NULL;
    }
}
