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
 *  \brief An event dispatcher manager. Once started it runs the provided treatment function on event reception.
 */

#ifndef SOPC_EVENT_DISPATCHER_MANAGER_H_
#define SOPC_EVENT_DISPATCHER_MANAGER_H_

#include <stdint.h>

#include "sopc_toolkit_constants.h"

typedef struct SOPC_EventDispatcherManager SOPC_EventDispatcherManager;

// Function is always in charge to deallocate params in case it is not NULL
typedef void SOPC_EventDispatcherFct(int32_t event, uint32_t eltId, void* params, uintptr_t auxParam);

/**
 *  \brief Create and start the event dispatcher manager treatment, then event can be added in the event queue and
 *  will be treated sequentially with the event dispatcher function as a FIFO queue.
 *
 *  \param fctPointer event dispatcher function pointer to be used to manage a new event
 *  \param name       debug name for the event dispatcher manager
 *
 *  \return  NULL if event dispatcher manager failed, event dispatcher manager in case of success
 */
SOPC_EventDispatcherManager* SOPC_EventDispatcherManager_CreateAndStart(SOPC_EventDispatcherFct fctPointer,
                                                                        const char* name);

/**
 *  \brief Add the event to be treated in the given event dispatcher manager. The event is provided with parameters for
 * event treatment.
 *
 *  \param eventMgr  Pointer of the event dispatcher manager in which action will be added
 *  \param event     The event integer value to add to the event dispatcher manager queue
 *  \param eltId     Identifier of the element on which the event shall be interpreted
 *  \param params    Generic parameter provided with the event
 *  \param auxParam  Auxiliary integer parameter provided with the event (e.g.: status code, etc.)
 *  \param debugName Indicates in a human readable way the event added to the queue
 *  \return             SOPC_STATUS_OK if action added successfully,
 *                      SOPC_STATUS_INVALID_PARAMETER in case event dispatcher manager pointer is NULL,
 *                      SOPC_STATUS_NOK otherwise (event dispatcher manager not started, function pointer and argument
 * both NULL).
 */
SOPC_ReturnStatus SOPC_EventDispatcherManager_AddEvent(SOPC_EventDispatcherManager* eventMgr,
                                                       int32_t event,
                                                       uint32_t eltId,
                                                       void* params,
                                                       uintptr_t auxParam,
                                                       const char* debugName);

/**
 *  \brief Similar to *_AddEvent but the event will be added to be the next one to be treated
 *  IMPORTANT NOTE: to be used only when it is really necessary since event order is not kept
 *
 *  \param eventMgr  Pointer of the event dispatcher manager in which action will be added
 *  \param event     The event integer value to add to the event dispatcher manager queue
 *  \param eltId     Identifier of the element on which the event shall be interpreted
 *  \param params    Generic parameter provided with the event
 *  \param auxParam  Auxiliary integer parameter provided with the event (e.g.: status code, etc.)
 *  \param debugName Indicates in a human readable way the event added to the queue
 *  \return             SOPC_STATUS_OK if action added successfully,
 *                      SOPC_STATUS_INVALID_PARAMETER in case event dispatcher manager pointer is NULL,
 *                      SOPC_STATUS_NOK otherwise (event dispatcher manager not started, function pointer and argument
 * both NULL).
 */
SOPC_ReturnStatus SOPC_EventDispatcherManager_AddEventAsNext(SOPC_EventDispatcherManager* eventMgr,
                                                             int32_t event,
                                                             uint32_t eltId,
                                                             void* params,
                                                             uintptr_t auxParam,
                                                             const char* debugName);

/**
 *  \brief Stops the event dispatcher manager treatment.
 *
 *  \param eventMgr  Address of the pointer of the event dispatcher manager to stop
 *
 *  \return  SOPC_STATUS_OK if it stopped successfully and *eventMgr == NULL,
 *           SOPC_STATUS_INVALID_PARAMETER in case event dispatcher manager pointer is NULL,
 *           SOPC_STATUS_NOK otherwise (already stopped, failed to stop).
 */
SOPC_ReturnStatus SOPC_EventDispatcherManager_StopAndDelete(SOPC_EventDispatcherManager** eventMgr);

/**
 * \brief Internal use only
 */
typedef struct SOPC_EventDispatcherParams
{
    int32_t event;
    uint32_t eltId;
    void* params;
    uintptr_t auxParam;
    const char* debugName;
} SOPC_EventDispatcherParams;

#endif /* SOPC_EVENT_DISPATCHER_MANAGER_H_ */
