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

SOPC_ReturnStatus SOPC_AsyncQueue_Init(SOPC_AsyncQueue** queue, const char* queueName);

/** @brief: enqueue in LILO mode */
SOPC_ReturnStatus SOPC_AsyncQueue_BlockingEnqueue(SOPC_AsyncQueue* queue, void* element);

/** @brief: enqueue in LIFO mode */
SOPC_ReturnStatus SOPC_AsyncQueue_BlockingEnqueueFirstOut(SOPC_AsyncQueue* queue, void* element);

SOPC_ReturnStatus SOPC_AsyncQueue_BlockingDequeue(SOPC_AsyncQueue* queue, void** element);

// Returns SOPC_STATUS_WOULD_BLOCK in case of no action to dequeue
SOPC_ReturnStatus SOPC_AsyncQueue_NonBlockingDequeue(SOPC_AsyncQueue* queue, void** element);

/** @brief: clear the queue by freeing every present element and free the asynchronous queue */
void SOPC_AsyncQueue_Free(SOPC_AsyncQueue** queue);

#endif /* SOPC_ASYNC_QUEUE_H_ */
