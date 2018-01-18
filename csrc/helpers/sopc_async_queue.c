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

#include "sopc_async_queue.h"

#include <stdbool.h>
#include <stdlib.h>

#include "sopc_mutexes.h"
#include "sopc_singly_linked_list.h"

struct SOPC_AsyncQueue
{
    const char* debugQueueName;
    SOPC_SLinkedList* queueList;
    Condition queueCond;
    Mutex queueMutex;
    uint32_t waitingThreads;
};

SOPC_ReturnStatus SOPC_AsyncQueue_Init(SOPC_AsyncQueue** queue, const char* queueName)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != queue)
    {
        *queue = malloc(sizeof(SOPC_AsyncQueue));
        if (*queue != NULL)
        {
            status = SOPC_STATUS_OK;
            (*queue)->debugQueueName = queueName;
            (*queue)->waitingThreads = 0;
            (*queue)->queueList = SOPC_SLinkedList_Create(0);
            if (NULL == (*queue)->queueList)
            {
                status = SOPC_STATUS_NOK;
            }
            if (SOPC_STATUS_OK == status)
            {
                status = Condition_Init(&(*queue)->queueCond);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_SLinkedList_Delete((*queue)->queueList);
                    (*queue)->queueList = NULL;
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                status = Mutex_Initialization(&(*queue)->queueMutex);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_SLinkedList_Delete((*queue)->queueList);
                    (*queue)->queueList = NULL;
                    Condition_Clear(&(*queue)->queueCond);
                }
            }
            if (SOPC_STATUS_OK != status)
            {
                free(*queue);
                *queue = NULL;
            }
        }
    }
    return status;
}

static SOPC_ReturnStatus SOPC_AsyncQueue_BlockingEnqueueFirstOrLast(SOPC_AsyncQueue* queue,
                                                                    void* element,
                                                                    bool firstOut)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    void* enqueuedElt = NULL;
    if (NULL != queue && NULL != element)
    {
        status = Mutex_Lock(&queue->queueMutex);
        if (SOPC_STATUS_OK == status)
        {
            if (false == firstOut)
            {
                enqueuedElt = SOPC_SLinkedList_Append(queue->queueList, 0, element);
            }
            else
            {
                enqueuedElt = SOPC_SLinkedList_Prepend(queue->queueList, 0, element);
            }
            if (element == enqueuedElt)
            {
                if (queue->waitingThreads > 0)
                {
                    Condition_SignalAll(&queue->queueCond);
                }
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
            Mutex_Unlock(&queue->queueMutex);
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_AsyncQueue_BlockingEnqueueFirstOut(SOPC_AsyncQueue* queue, void* element)
{
    return SOPC_AsyncQueue_BlockingEnqueueFirstOrLast(queue, element, true);
}

SOPC_ReturnStatus SOPC_AsyncQueue_BlockingEnqueue(SOPC_AsyncQueue* queue, void* element)
{
    return SOPC_AsyncQueue_BlockingEnqueueFirstOrLast(queue, element, false);
}

static SOPC_ReturnStatus SOPC_AsyncQueue_Dequeue(SOPC_AsyncQueue* queue, bool isBlocking, void** element)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL != queue && NULL != element)
    {
        status = SOPC_STATUS_NOK;
        Mutex_Lock(&queue->queueMutex);
        *element = SOPC_SLinkedList_PopHead(queue->queueList);
        if (NULL == *element)
        {
            if (false == isBlocking)
            {
                status = SOPC_STATUS_WOULD_BLOCK;
            }
            else
            {
                queue->waitingThreads++;
                *element = SOPC_SLinkedList_PopHead(queue->queueList);
                while (NULL == *element)
                {
                    Mutex_UnlockAndWaitCond(&queue->queueCond, &queue->queueMutex);
                    *element = SOPC_SLinkedList_PopHead(queue->queueList);
                }
                status = SOPC_STATUS_OK;
                queue->waitingThreads--;
            }
        }
        else
        {
            status = SOPC_STATUS_OK;
        }
        Mutex_Unlock(&queue->queueMutex);
    }
    return status;
}

SOPC_ReturnStatus SOPC_AsyncQueue_BlockingDequeue(SOPC_AsyncQueue* queue, void** element)
{
    return SOPC_AsyncQueue_Dequeue(queue, true, element);
}

SOPC_ReturnStatus SOPC_AsyncQueue_NonBlockingDequeue(SOPC_AsyncQueue* queue, void** element)
{
    return SOPC_AsyncQueue_Dequeue(queue, false, element);
}

void SOPC_AsyncQueue_Free(SOPC_AsyncQueue** queue)
{
    if (NULL != queue)
    {
        if (NULL != *queue && NULL != (*queue)->queueList)
        {
            SOPC_SLinkedList_Apply((*queue)->queueList, SOPC_SLinkedList_EltGenericFree);
            SOPC_SLinkedList_Delete((*queue)->queueList);
        }
        free(*queue);
        *queue = NULL;
    }
}
