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

/**
 *  \file
 *
 *  \brief An asynchronous and thread-safe queue implementation
 */

#ifndef SOPC_ASYNC_QUEUE_H_
#define SOPC_ASYNC_QUEUE_H_

#include "sopc_toolkit_constants.h"

typedef struct SOPC_AsyncQueue SOPC_AsyncQueue;

SOPC_ReturnStatus SOPC_AsyncQueue_Init(SOPC_AsyncQueue** queue, const char* queueName);

/** @brief: enqueue in FIFO mode */
SOPC_ReturnStatus SOPC_AsyncQueue_BlockingEnqueue(SOPC_AsyncQueue* queue, void* element);

/** @brief: enqueue in LIFO mode */
SOPC_ReturnStatus SOPC_AsyncQueue_BlockingEnqueueFirstOut(SOPC_AsyncQueue* queue, void* element);

SOPC_ReturnStatus SOPC_AsyncQueue_BlockingDequeue(SOPC_AsyncQueue* queue, void** element);

// Returns SOPC_STATUS_WOULD_BLOCK in case of no action to dequeue
SOPC_ReturnStatus SOPC_AsyncQueue_NonBlockingDequeue(SOPC_AsyncQueue* queue, void** element);

/** @brief: clear the queue by freeing every present element and free the asynchronous queue */
void SOPC_AsyncQueue_Free(SOPC_AsyncQueue** queue);

#endif /* SOPC_ASYNC_QUEUE_H_ */
