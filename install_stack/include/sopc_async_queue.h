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

#ifndef SOPC_ASYNC_QUEUE_H_
#define SOPC_ASYNC_QUEUE_H_

#include "sopc_base_types.h"

typedef struct SOPC_AsyncQueue SOPC_AsyncQueue;

SOPC_StatusCode SOPC_AsyncQueue_Init(SOPC_AsyncQueue** queue, const char*  queueName);

SOPC_StatusCode SOPC_AsyncQueue_BlockingEnqueue(SOPC_AsyncQueue* queue,
                                                void*            element);

SOPC_StatusCode SOPC_AsyncQueue_BlockingDequeue(SOPC_AsyncQueue* queue,
                                                void**           element);

// Returns OpcUa_BadWouldBlock in case of no action to dequeue
SOPC_StatusCode SOPC_AsyncQueue_NonBlockingDequeue(SOPC_AsyncQueue* queue,
                                                   void**           element);

void SOPC_AsyncQueue_Free(SOPC_AsyncQueue** queue);

#endif /* SOPC_ASYNC_QUEUE_H_ */
