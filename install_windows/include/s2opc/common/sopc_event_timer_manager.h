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
 *  \brief An event timer manager which allow to associate an event to enqueue in an event dispatcher manager on timer
 * expiration.
 *
 *  \note  SOPC_EventTimer_CyclicTimersEvaluation function shall be called to evaluate timers expiration (already
 * integrated in toolkit).
 */

#ifndef SOPC_EVENT_TIMER_MANAGER_H_
#define SOPC_EVENT_TIMER_MANAGER_H_

#include "sopc_event_handler.h"
#include "sopc_time.h"

/**
 * Maximum resolution time for the event timers evaluation
 *
 */
#ifndef SOPC_TIMER_RESOLUTION_MS
#define SOPC_TIMER_RESOLUTION_MS 50
#endif

#if SOPC_TIMER_RESOLUTION_MS <= 0
#error "Timer resolution cannot be <= 0"
#endif

/**
 * \brief Initialize the event timer manager (necessary to create timers)
 *
 */
void SOPC_EventTimer_Initialize(void);

/**
 * \brief Clear the event timer manager (cancel all timers not already triggered)
 *
 */
void SOPC_EventTimer_Clear(void);

/**
 * \brief Create a timer which will raise the given event parameters to the given event
 * dispatch manager
 *
 * \param eventHandler  the event handler where to dispatch the event on timeout
 * \param event         the event to dispatch on timeout
 * \param msDelay       the delay from current time before timeout in milliseconds
 *
 * \return the timer identifier (or value 0 if operation failed)
 *
 * */
uint32_t SOPC_EventTimer_Create(SOPC_EventHandler* eventHandler, SOPC_Event event, uint64_t msDelay);

/**
 * \brief Creates a periodic timer raising an event on a dispatch manager every msPeriod milliseconds.
 *
 * \param eventHandler  the event handler where to dispatch the event on timeout
 * \param event         the event to dispatch on timeout
 * \param msPeriod    the period in milliseconds
 *
 * \return the timer identifier (or value 0 if operation failed)
 *
 * */
uint32_t SOPC_EventTimer_CreatePeriodic(SOPC_EventHandler* eventHandler, SOPC_Event event, uint64_t msPeriod);

/**
 * \brief Modifies an existing periodic timer period
 *
 * \note The new period value is applied from next timer expiration
 *
 * \param timerId the identifier of the started periodic timer to modify
 * \param msPeriod the new period to apply in milliseconds
 *
 * \return true if the modification was successfully done, false otherwise
 */
bool SOPC_EventTimer_ModifyPeriodic(uint32_t timerId, uint64_t msPeriod);

/**
 * \brief Cancel a started timer
 *
 * \param timerId the identifier of the started timer to cancel
 *
 */
void SOPC_EventTimer_Cancel(uint32_t timerId);

#endif /* SOPC_EVENT_TIMER_MANAGER_H_ */
