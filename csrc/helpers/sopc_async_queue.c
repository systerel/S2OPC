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

#include <stdbool.h>

#include "sopc_async_queue.h"
#include "sopc_mem_alloc.h"
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
        *queue = SOPC_Malloc(sizeof(SOPC_AsyncQueue));
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
                SOPC_Free(*queue);
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
                {
#if 0
                    struct SOPC_SLinkedList_
                    {
                        SOPC_SLinkedList_Elt* first;
                        SOPC_SLinkedList_Elt* last;
                        uint32_t length;
                        uint32_t maxLength;
                    };

                    struct Event_
                    {
                        void* handler;
                        int32_t code;
                        uint32_t id;
                        void* params;
                        uintptr_t auxParam;
                    };

                    {
                        char* sBuffer[50] = {0};
                        QueueHandle_t value = xTaskGetCurrentTaskHandle();
                        snprintf((void*) sBuffer, sizeof(sBuffer) - 1, "|%lu|APPEND|%lu|%s|%2X|%lu|%lu|q%s\r\n",
                                 xTaskGetTickCount(), ((struct Event_*) element)->code, pcTaskGetName(value),
                                 (unsigned int) queue, ((struct SOPC_SLinkedList_*) queue->queueList)->length,
                                 queue->waitingThreads, queue->debugQueueName);
                        SOPC_LogSrv_Print(sBuffer, strlen(sBuffer));
                    }
#endif
                }
                enqueuedElt = SOPC_SLinkedList_Append(queue->queueList, 0, element);
            }
            else
            {
                {
#if 0
                    struct SOPC_SLinkedList_
                    {
                        SOPC_SLinkedList_Elt* first;
                        SOPC_SLinkedList_Elt* last;
                        uint32_t length;
                        uint32_t maxLength;
                    };

                    struct Event_
                    {
                        void* handler;
                        int32_t code;
                        uint32_t id;
                        void* params;
                        uintptr_t auxParam;
                    };

                    {
                        char* sBuffer[50] = {0};
                        QueueHandle_t value = xTaskGetCurrentTaskHandle();
                        snprintf((void*) sBuffer, sizeof(sBuffer) - 1, "|%lu|PREPEND|%lu|%s|%2X|%lu|%lu|q%s\r\n",
                                 xTaskGetTickCount(), ((struct Event_*) element)->code, pcTaskGetName(value),
                                 (unsigned int) queue, ((struct SOPC_SLinkedList_*) queue->queueList)->length,
                                 queue->waitingThreads, queue->debugQueueName);
                        SOPC_LogSrv_Print(sBuffer, strlen(sBuffer));
                    }
#endif
                }
                enqueuedElt = SOPC_SLinkedList_Prepend(queue->queueList, 0, element);
            }
            if (element == enqueuedElt)
            {
                if (queue->waitingThreads > 0)
                {
                    {
#if 0
                        struct SOPC_SLinkedList_
                        {
                            SOPC_SLinkedList_Elt* first;
                            SOPC_SLinkedList_Elt* last;
                            uint32_t length;
                            uint32_t maxLength;
                        };

                        {
                            char* sBuffer[50] = {0};
                            QueueHandle_t value = xTaskGetCurrentTaskHandle();
                            snprintf((void*) sBuffer, sizeof(sBuffer) - 1, "|%lu|SIGALL||%s|%2X|%lu|%lu|q%s\r\n",
                                     xTaskGetTickCount(), pcTaskGetName(value), (unsigned int) queue,
                                     ((struct SOPC_SLinkedList_*) queue->queueList)->length, queue->waitingThreads,
                                     queue->debugQueueName);
                            SOPC_LogSrv_Print(sBuffer, strlen(sBuffer));
                        }
#endif
                    }
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

                {
#if 0
                    struct SOPC_SLinkedList_
                    {
                        SOPC_SLinkedList_Elt* first;
                        SOPC_SLinkedList_Elt* last;
                        uint32_t length;
                        uint32_t maxLength;
                    };

                    struct Event_
                    {
                        void* handler;
                        int32_t code;
                        uint32_t id;
                        void* params;
                        uintptr_t auxParam;
                    };

                    {
                        char* sBuffer[50] = {0};
                        QueueHandle_t value = xTaskGetCurrentTaskHandle();
                        snprintf((void*) sBuffer, sizeof(sBuffer) - 1, "|%lu|POPHEAD LOOPER|%lu|q%s|%2X|%lu|%lu|%s\r\n",
                                 xTaskGetTickCount(), (*((struct Event_**) element))->code, queue->debugQueueName,
                                 (unsigned int) queue, ((struct SOPC_SLinkedList_*) queue->queueList)->length,
                                 queue->waitingThreads, pcTaskGetName(value));
                        SOPC_LogSrv_Print(sBuffer, strlen(sBuffer));
                    }
#endif
                }
            }
        }
        else
        {
            {
#if 0
                struct SOPC_SLinkedList_
                {
                    SOPC_SLinkedList_Elt* first;
                    SOPC_SLinkedList_Elt* last;
                    uint32_t length;
                    uint32_t maxLength;
                };
                struct Event_
                {
                    void* handler;
                    int32_t code;
                    uint32_t id;
                    void* params;
                    uintptr_t auxParam;
                };

                {
                    char* sBuffer[50] = {0};
                    QueueHandle_t value = xTaskGetCurrentTaskHandle();
                    snprintf((void*) sBuffer, sizeof(sBuffer) - 1, "|%lu|POPHEAD|%lu|q%s|%2X|%lu|%lu|%s\r\n",
                             xTaskGetTickCount(), (*((struct Event_**) element))->code, queue->debugQueueName,
                             (unsigned int) queue, ((struct SOPC_SLinkedList_*) queue->queueList)->length,
                             queue->waitingThreads, pcTaskGetName(value));
                    SOPC_LogSrv_Print(sBuffer, strlen(sBuffer));
                }
#endif
            }
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
            Mutex_Clear(&(*queue)->queueMutex);
            Condition_Clear(&(*queue)->queueCond);
        }
        SOPC_Free(*queue);
        *queue = NULL;
    }
}
