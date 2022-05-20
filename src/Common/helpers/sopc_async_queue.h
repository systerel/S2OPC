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
 *  \file
 *
 *  \brief An asynchronous and thread-safe queue implementation
 */

#ifndef SOPC_ASYNC_QUEUE_H_
#define SOPC_ASYNC_QUEUE_H_

#include "sopc_enums.h"

typedef struct SOPC_AsyncQueue SOPC_AsyncQueue;

/**
 *  \brief          Initialize the queue, the queue must be freed at the end of it's use with SOPC_AsyncQueue_Free
 *
 *  \param queue     Pointer on the address of the queue
 *  \param queueName Pointer on a string, useful to debug
 *
 *  \return         SOPC_STATUS_OK if the queue is successfully initialized. SOPC_STATUS_OK or
 * SOPC_STATUS_INVALID_PARAMETERS otherwise
 */
SOPC_ReturnStatus SOPC_AsyncQueue_Init(SOPC_AsyncQueue** queue, const char* queueName);

/**
 *  \brief          Add a new element (and allocate new queue element) to the head of the given linked queue.
 *                  Can be used as FIFO queue with SOPC_AsyncQueue_BlokingDequeue or SOPC_AsyncQueue_NonBlockingDequeue.
 *
 *
 *  \param queue     Pointer on the linked queue in which new element must be added
 *  \param element    Pointer to the element to append, must be dynamically created by user.
 *
 *  \return         SOPC_ReturnStatus value, if the element is successfully added SOPC_STATUS_OK, otherwise if queue or
 *                  element is NULL SOPC_STATUS_INVALID_PARAMETERS, and SOPC_STATUS_NOK in others failed cases
 */
SOPC_ReturnStatus SOPC_AsyncQueue_BlockingEnqueue(SOPC_AsyncQueue* queue, void* element);

/**
 *  \brief          Add a new element (and allocate new queue element) to the tail of the given linked queue.
 *                  Can be used as LIFO queue with SOPC_AsyncQueue_BlokingDequeue or SOPC_AsyncQueue_NonBlockingDequeue.
 *
 *
 *  \param queue     Pointer on the linked queue in which new element must be added
 *  \param element    Pointer to the element to append, must be dynamically created by user.
 *
 *  \return         SOPC_ReturnStatus value, if the element is successfully added SOPC_STATUS_OK, otherwise if queue or
 *                  element is NULL SOPC_STATUS_INVALID_PARAMETERS, and SOPC_STATUS_NOK in others failed cases
 */
SOPC_ReturnStatus SOPC_AsyncQueue_BlockingEnqueueFirstOut(SOPC_AsyncQueue* queue, void* element);

/**
 *  \brief          Get and remove the head element of the queue. If the queue is empty the function will block
 *                  until an element is added in the queue.
 *
 *  \param queue     Pointer on the linked queue in which the element will be removed
 *  \param element    Pointer to the address of the element to remove. element can be freed by caller after use
 *
 *  \return         SOPC_ReturnStatus value, if the element is successfully removed SOPC_STATUS_OK, if queue or
 *                  element is NULL SOPC_STATUS_INVALID_PARAMETERS
 */
SOPC_ReturnStatus SOPC_AsyncQueue_BlockingDequeue(SOPC_AsyncQueue* queue, void** element);

/**
 *  \brief          Get and remove the head element of the queue. If the queue is empty the function will block
 *                  until an element is added in the queue. Element must be freed after used.
 *
 *  \param queue     Pointer on the linked queue in which the element will be removed
 *  \param element    Pointer to the addres of the element to remove, element can be freed by caller after use
 *
 *  \return         SOPC_ReturnStatus value, if the element is successfully removed SOPC_STATUS_OK, if queue or
 *                  element is NULL SOPC_STATUS_INVALID_PARAMETERS, if no element in
 *                  the queue SOPC_STATUS_WOULD_BLOCK
 */
SOPC_ReturnStatus SOPC_AsyncQueue_NonBlockingDequeue(SOPC_AsyncQueue* queue, void** element);

/** @brief: clear the queue by freeing every present element and free the asynchronous queue */
void SOPC_AsyncQueue_Free(SOPC_AsyncQueue** queue);

#endif /* SOPC_ASYNC_QUEUE_H_ */
