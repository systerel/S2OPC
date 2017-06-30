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

#ifndef SOPC_ACTION_QUEUE_MANAGER_H_
#define SOPC_ACTION_QUEUE_MANAGER_H_

#include "sopc_action_queue.h"

typedef struct SOPC_ActionQueueManager SOPC_ActionQueueManager;

// Initialization done by stack config module
extern SOPC_ActionQueueManager* stackActionQueueMgr; // stack actions manager
extern SOPC_ActionQueueManager* appCallbackQueueMgr; // applicative callbacks manager
extern SOPC_ActionQueueManager* condSignalQueueMgr; // condition signal change (unblocking sync calls) manager


/**
 *  \brief Create and start the action queue manager treatment, then actions can be added in the action queue and
 *  will be treated sequentially as a FIFO queue.
 *
 *  \param name  debug name for the action queue manager
 *
 *  \return  NULL if action queue manager failed, action queue manager in case of success
 */
SOPC_ActionQueueManager* SOPC_ActionQueueManager_CreateAndStart(const char* name);

/**
 *  \brief Add the action to be run in the action queue. The action is provided as a function pointer and parameter to be provided.
 *
 *  \param queueMgr     Pointer of the action queue manager in which action will be added
 *  \param fctPointer   Pointer of the function to run for executing the action
 *  \param fctArgument  Argument to be used as parameter when calling the provided function
 *  \param actionText   Indicates in a human readable string the action added to the queue
 *  \return             STATUS_OK if action added successfully,
 *                      STATUS_INVALID_PARAMETER in case action queue manager pointer is NULL,
 *                      STATUS_NOK otherwise (action queue manager not started, function pointer and argument both NULL).
 */
SOPC_StatusCode SOPC_ActionQueueManager_AddAction(SOPC_ActionQueueManager* queueMgr,
                                                  SOPC_ActionFunction*     fctPointer,
                                                  void*                    fctArgument,
                                                  const char*              actionText);
/**
 *  \brief Stops the action queue manager treatment.
 *
 *  \param queueMgr  Address of the pointer of the action queue manager to stop
 *
 *  \return  STATUS_OK if it stopped successfully and *queueMgr == NULL,
 *           STATUS_INVALID_PARAMETER in case action queue manager pointer is NULL,
 *           STATUS_NOK otherwise (already stopped, failed to stop).
 */
SOPC_StatusCode SOPC_ActionQueueManager_StopAndDelete(SOPC_ActionQueueManager** queueMgr);

#endif /* SOPC_ACTION_QUEUE_MANAGER_H_ */
