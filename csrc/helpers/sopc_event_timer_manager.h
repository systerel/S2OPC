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

#include "sopc_event_dispatcher_manager.h"
#include "sopc_time.h"

/**
 * \brief Initialize the event timer manager (necessary to could create timers)
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
 * \param eventMgr    the event dispatch manager to which event params will be provided on timeout
 * \param eventParams the event parameters that will be provided on timeout to the event dispatch manager
 * \param msDelay     the delay from current time before timeout in milliseconds
 *
 * \return the timer identifier (or value 0 if operation failed)
 *
 * */
uint32_t SOPC_EventTimer_Create(SOPC_EventDispatcherManager* eventMgr,
                                SOPC_EventDispatcherParams eventParams,
                                uint64_t msDelay);

/**
 * \brief Creates a periodic timer raising an event on a dispatch manager every msPeriod milliseconds.
 *
 * \param eventMgr    the event dispatch manager to which event params will be provided on timeout
 * \param eventParams the event parameters that will be provided on timeout to the event dispatch manager
 * \param msPeriod    the period in milliseconds
 *
 * \return the timer identifier (or value 0 if operation failed)
 *
 * */
uint32_t SOPC_EventTimer_CreatePeriodic(SOPC_EventDispatcherManager* eventMgr,
                                        SOPC_EventDispatcherParams eventParams,
                                        uint64_t msPeriod);

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

/**
 * \brief Evaluation of the started timers, in case of timeout the given event will be triggered to the given event
 * dispatcher manager
 *
 * Note: this function is automatically called by the toolkit
 */
void SOPC_EventTimer_CyclicTimersEvaluation(void);

#endif /* SOPC_EVENT_TIMER_MANAGER_H_ */
