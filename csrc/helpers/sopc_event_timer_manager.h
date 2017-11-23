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

#ifndef SOPC_EVENT_TIMER_MANAGER_H
#define SOPC_EVENT_TIMER_MANAGER_H

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
 * \param startTime   the time reference to be used as start time
 * \param msDelay     the delay from time reference before timeout in milliseconds
 *
 * \return the timer identifier
 *
 * */
uint32_t SOPC_EventTimer_Create(SOPC_EventDispatcherManager* eventMgr,
                                SOPC_EventDispatcherParams eventParams,
                                SOPC_TimeReference* startTime,
                                uint64_t msDelay);

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
 */
void SOPC_EventTimer_CyclicTimersEvaluation(void);

#endif /* SOPC_EVENT_TIMER_MANAGER_H */
