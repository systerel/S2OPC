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

#ifndef SOPC_ACTION_QUEUE_H_
#define SOPC_ACTION_QUEUE_H_

#include "sopc_base_types.h"

typedef struct SOPC_ActionQueue SOPC_ActionQueue;

typedef void SOPC_ActionFunction (void* arg);

SOPC_StatusCode SOPC_ActionQueue_Init(SOPC_ActionQueue** queue, const char*  queueName);

// queue non NULL, fctPointer or fctArgument non NULL, actionText optional
SOPC_StatusCode SOPC_Action_BlockingEnqueue(SOPC_ActionQueue*    queue,
                                            SOPC_ActionFunction* fctPointer,
                                            void*                fctArgument,
                                            const char*          actionText);

// actionText optional, others non NULL
SOPC_StatusCode SOPC_Action_BlockingDequeue(SOPC_ActionQueue*     queue,
                                            SOPC_ActionFunction** fctPointer,
                                            void**                fctArgument,
                                            const char**                actionText);

// Returns OpcUa_BadWouldBlock in case of no action to dequeue
SOPC_StatusCode SOPC_Action_NonBlockingDequeue(SOPC_ActionQueue*     queue,
                                               SOPC_ActionFunction** fctPointer,
                                               void**                fctArgument,
                                               const char**                actionText);

void SOPC_ActionQueue_Free(SOPC_ActionQueue** queue);

#endif /* SOPC_ACTION_QUEUE_H_ */
