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

#ifndef SOPC_EVENT_DISPATCHER_MANAGER_H_
#define SOPC_EVENT_DISPATCHER_MANAGER_H_

#include "sopc_async_queue.h"

typedef struct SOPC_EventDispatcherManager SOPC_EventDispatcherManager;

// Function is always in charge to deallocate params in case it is not NULL
typedef void SOPC_EventDispatcherFct (int32_t event, uint32_t eltId, void* params, int32_t auxParam);

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
                                                                        const char*             name);

/**
 *  \brief Add the event to be treated in the given event dispatcher manager. The event is provided with parameters for event treatment.
 *
 *  \param eventMgr  Pointer of the event dispatcher manager in which action will be added
 *  \param event     The event integer value to add to the event dispatcher manager queue
 *  \param eltId     Identifier of the element on which the event shall be interpreted
 *  \param params    Generic parameter provided with the event
 *  \param auxParam  Auxiliary integer parameter provided with the event (e.g.: status code, etc.)
 *  \param debugName Indicates in a human readable way the event added to the queue
 *  \return             STATUS_OK if action added successfully,
 *                      STATUS_INVALID_PARAMETER in case event dispatcher manager pointer is NULL,
 *                      STATUS_NOK otherwise (event dispatcher manager not started, function pointer and argument both NULL).
 */
SOPC_StatusCode SOPC_EventDispatcherManager_AddEvent(SOPC_EventDispatcherManager* eventMgr,
                                                     int32_t                      event,
                                                     uint32_t                     eltId,
                                                     void*                        params,
                                                     int32_t                      auxParam,
                                                     const char*                  debugName);

/**
 *  \brief Similar to *_AddEvent but the event will be added to be the next one to be treated
 *  IMPORTANT NOTE: to be used only when it is really necessary since event order is not kept
 *
 *  \param eventMgr  Pointer of the event dispatcher manager in which action will be added
 *  \param eltId     Identifier of the element on which the event shall be interpreted
 *  \param params    Generic parameter provided with the event
 *  \param auxParam  Auxiliary integer parameter provided with the event (e.g.: status code, etc.)
 *  \param debugName Indicates in a human readable way the event added to the queue
 *  \return             STATUS_OK if action added successfully,
 *                      STATUS_INVALID_PARAMETER in case event dispatcher manager pointer is NULL,
 *                      STATUS_NOK otherwise (event dispatcher manager not started, function pointer and argument both NULL).
 */
SOPC_StatusCode SOPC_EventDispatcherManager_AddEventAsNext(SOPC_EventDispatcherManager* eventMgr,
                                                           int32_t                      event,
                                                           uint32_t                     eltId,
                                                           void*                        params,
                                                           int32_t                      auxParam,
                                                           const char*                  debugName);

/**
 *  \brief Stops the event dispatcher manager treatment.
 *
 *  \param eventMgr  Address of the pointer of the event dispatcher manager to stop
 *
 *  \return  STATUS_OK if it stopped successfully and *eventMgr == NULL,
 *           STATUS_INVALID_PARAMETER in case event dispatcher manager pointer is NULL,
 *           STATUS_NOK otherwise (already stopped, failed to stop).
 */
SOPC_StatusCode SOPC_EventDispatcherManager_StopAndDelete(SOPC_EventDispatcherManager** eventMgr);

#endif /* SOPC_EVENT_DISPATCHER_MANAGER_H_ */
