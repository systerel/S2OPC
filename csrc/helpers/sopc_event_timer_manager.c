/*
 *  Copyright (C) 2018 Systerel and others.
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

#include "sopc_event_timer_manager.h"

#include <assert.h>
#include <inttypes.h>
#include <string.h>

#include "sopc_atomic.h"
#include "sopc_logger.h"
#include "sopc_mutexes.h"
#include "sopc_singly_linked_list.h"

typedef struct SOPC_EventTimer
{
    uint32_t id;
    SOPC_EventDispatcherManager* eventMgr;
    SOPC_EventDispatcherParams eventParams;
    SOPC_TimeReference endTime;
    /* Rest is used only for periodic timers */
    bool isPeriodicTimer;
    uint64_t periodMs;
} SOPC_EventTimer;

#define SOPC_MAX_TIMERS UINT8_MAX /* TODO: avoid static maximum (see monitoredItems Id creation) */

static bool usedTimerIds[SOPC_MAX_TIMERS + 1]; // 0 idx value is invalid (max idx = MAX)
static uint32_t latestTimerId = 0;

static SOPC_SLinkedList* timers = NULL;
static SOPC_SLinkedList* periodicTimersToRestart = NULL;

static Mutex timersMutex;
static int32_t initialized = 0;

static bool is_initialized(void)
{
    return SOPC_Atomic_Int_Get(&initialized) != 0;
}

// Caller should lock the mutex
// 0 result is invalid
static uint32_t SOPC_Internal_GetFreshTimerId_WithoutLock(void)
{
    uint32_t result = 0;

    uint32_t idx = latestTimerId;
    if (SOPC_SLinkedList_GetLength(timers) < SOPC_MAX_TIMERS)
    {
        if (idx < SOPC_MAX_TIMERS) // 0 idx value is invalid (max idx = MAX)
        {
            idx = idx + 1;
        }
        else
        {
            idx = 1;
        }
    } // else idx == latestTimerId which leads to terminate with result == 0

    while (0 == result && idx != latestTimerId)
    {
        if (false == usedTimerIds[idx])
        {
            result = idx;
            usedTimerIds[idx] = true;
        }
        else
        {
            if (idx < SOPC_MAX_TIMERS) // 0 idx value is invalid (max idx = MAX)
            {
                idx = idx + 1;
            }
            else
            {
                idx = 1;                // 0 is invalid value
                if (latestTimerId == 0) // deal with case of init to guarantee no infinite loop
                {
                    latestTimerId = 1;
                }
            }
        }
    }

    if (result != 0)
    {
        latestTimerId = result;
    }

    return result;
}

void SOPC_EventTimer_Initialize()
{
    if (is_initialized())
    {
        return;
    }

    Mutex_Initialization(&timersMutex);
    memset(usedTimerIds, false, sizeof(bool) * (SOPC_MAX_TIMERS + 1)); // 0 idx value is invalid (max idx = MAX + 1)
    timers = SOPC_SLinkedList_Create(SOPC_MAX_TIMERS);
    periodicTimersToRestart = SOPC_SLinkedList_Create(SOPC_MAX_TIMERS);

    if (timers == NULL || periodicTimersToRestart == NULL)
    {
        SOPC_SLinkedList_Delete(timers);
        SOPC_SLinkedList_Delete(periodicTimersToRestart);
        return;
    }

    SOPC_Atomic_Int_Set(&initialized, 1);
}

int8_t SOPC_Internal_SLinkedList_EventTimerCompare(void* left, void* right)
{
    int8_t result;
    if (NULL == left && NULL == right)
    {
        result = 0;
    }
    else if (NULL == left)
    {
        result = -1;
    }
    else if (NULL == right)
    {
        result = 1;
    }
    else
    {
        result = SOPC_TimeReference_Compare(((SOPC_EventTimer*) left)->endTime, ((SOPC_EventTimer*) right)->endTime);
    }
    return result;
}

void SOPC_EventTimer_Clear()
{
    Mutex_Lock(&timersMutex);
    SOPC_SLinkedList_Apply(timers, SOPC_SLinkedList_EltGenericFree);
    SOPC_SLinkedList_Delete(timers);
    timers = NULL;
    // No need to iterate to free elements, elements are temporary added only during timer evaluation
    SOPC_SLinkedList_Delete(periodicTimersToRestart);
    periodicTimersToRestart = NULL;
    SOPC_Atomic_Int_Set(&initialized, 0);
    Mutex_Unlock(&timersMutex);
}

static uint32_t SOPC_InternalEventTimer_Create(SOPC_EventDispatcherManager* eventMgr,
                                               SOPC_EventDispatcherParams eventParams,
                                               uint64_t msDelay,
                                               bool isPeriodic)
{
    if (!is_initialized())
    {
        return 0;
    }

    SOPC_EventTimer* newTimer = NULL;
    SOPC_TimeReference targetTime = 0;
    uint32_t result = 0;
    void* insertResult = NULL;

    // Create target time reference
    targetTime = SOPC_TimeReference_AddMilliseconds(SOPC_TimeReference_GetCurrent(), msDelay);
    // Allocate new timer
    newTimer = calloc(1, sizeof(SOPC_EventTimer));

    if (newTimer == NULL)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    // Configure timeout parameters
    newTimer->endTime = targetTime;
    newTimer->eventMgr = eventMgr;
    newTimer->eventParams = eventParams;
    newTimer->isPeriodicTimer = isPeriodic;
    newTimer->periodMs = msDelay;

    // Set timer
    Mutex_Lock(&timersMutex);
    result = SOPC_Internal_GetFreshTimerId_WithoutLock();
    if (result != 0)
    {
        newTimer->id = result;
        // valid timer Id
        insertResult =
            SOPC_SLinkedList_SortedInsert(timers, result, newTimer, SOPC_Internal_SLinkedList_EventTimerCompare);
        if (insertResult == NULL)
        {
            result = 0;
            free(newTimer);
        }
    } // else 0 is invalid value => no timer available
    else
    {
        free(newTimer);
    }
    Mutex_Unlock(&timersMutex);

    return result;
}

uint32_t SOPC_EventTimer_Create(SOPC_EventDispatcherManager* eventMgr,
                                SOPC_EventDispatcherParams eventParams,
                                uint64_t msDelay)
{
    return SOPC_InternalEventTimer_Create(eventMgr, eventParams, msDelay, false);
}

uint32_t SOPC_EventTimer_CreatePeriodic(SOPC_EventDispatcherManager* eventMgr,
                                        SOPC_EventDispatcherParams eventParams,
                                        uint64_t msPeriod)
{
    return SOPC_InternalEventTimer_Create(eventMgr, eventParams, msPeriod, true);
}

static void SOPC_Internal_EventTimer_Cancel_WithoutLock(uint32_t timerId)
{
    SOPC_EventTimer* timer = NULL;
    if (usedTimerIds[timerId])
    {
        timer = SOPC_SLinkedList_RemoveFromId(timers, timerId);
        if (timer != NULL)
        {
            free(timer);
        }
        usedTimerIds[timerId] = false;
    }
}

void SOPC_EventTimer_Cancel(uint32_t timerId)
{
    if (!is_initialized() || timerId == 0)
    {
        return;
    }

    Mutex_Lock(&timersMutex);
    SOPC_Internal_EventTimer_Cancel_WithoutLock(timerId);
    Mutex_Unlock(&timersMutex);
}

static void SOPC_InternalEventTimer_RestartPeriodicTimer_WithoutLock(SOPC_EventTimer* timer)
{
    SOPC_EventTimer* result = NULL;

    if (usedTimerIds[timer->id])
    {
        result = SOPC_SLinkedList_RemoveFromId(timers, timer->id);
        assert(result == timer);

        // Set timer
        result = SOPC_SLinkedList_SortedInsert(timers, timer->id, timer, SOPC_Internal_SLinkedList_EventTimerCompare);

        if (result != timer)
        {
            usedTimerIds[timer->id] = false;
            SOPC_Logger_TraceError("EventTimerManager: failed to restart the periodic timer on insertion id=%" PRIu32
                                   " with event=%" PRIi32 " and associated id=%" PRIu32,
                                   timer->id, timer->eventParams.event, timer->eventParams.eltId);
            free(timer);
        }
    }
    else
    {
        SOPC_Logger_TraceError("EventTimerManager: failed to restart the disabled periodic timer id=%" PRIu32
                               " with event=%" PRIi32 " and associated id=%" PRIu32,
                               timer->id, timer->eventParams.event, timer->eventParams.eltId);
        free(timer);
    }
}

void SOPC_EventTimer_CyclicTimersEvaluation()
{
    if (!is_initialized())
    {
        return;
    }

    SOPC_SLinkedListIterator timerIt = NULL;
    SOPC_EventTimer* timer = NULL;
    SOPC_TimeReference currentTimeRef = 0;
    int8_t compareResult = 0;
    uint32_t timerId = 0;

    Mutex_Lock(&timersMutex);
    timerIt = SOPC_SLinkedList_GetIterator(timers);
    timer = (SOPC_EventTimer*) SOPC_SLinkedList_Next(&timerIt);
    currentTimeRef = SOPC_TimeReference_GetCurrent();
    if (timer != NULL)
    {
        compareResult = SOPC_TimeReference_Compare(currentTimeRef, timer->endTime);
    } // else ignore and keep precedent result >= 0

    // Trigger timeout if currentTime >= timeoutTime
    while (timer != NULL && compareResult >= 0)
    {
        // Trigger timeout event to dispatch event manager
        timerId = timer->id;
        SOPC_EventDispatcherManager_AddEvent(timer->eventMgr, timer->eventParams.event, timer->eventParams.eltId,
                                             timer->eventParams.params, timer->eventParams.auxParam,
                                             timer->eventParams.debugName);

        if (timer->isPeriodicTimer)
        {
            // Set target time reference
            timer->endTime = SOPC_TimeReference_AddMilliseconds(timer->endTime, timer->periodMs);
            // Add to list of timers to restart it
            if (timer != SOPC_SLinkedList_Append(periodicTimersToRestart, timer->id, (void*) timer))
            {
                SOPC_Logger_TraceError(
                    "EventTimerManager: failed to restart the periodic timer on insertion id=%" PRIu32
                    " with event=%" PRIi32 " and associated id=%" PRIu32,
                    timer->id, timer->eventParams.event, timer->eventParams.eltId);
            }
        }
        else
        {
            // Remove the triggered timer (possible during iteration since we remove already iterated item)
            SOPC_Internal_EventTimer_Cancel_WithoutLock(timerId);
        }

        // Prepare next timeout evaluation
        timer = (SOPC_EventTimer*) SOPC_SLinkedList_Next(&timerIt);
        if (timer != NULL)
        {
            compareResult = SOPC_TimeReference_Compare(currentTimeRef, timer->endTime);
        }
    }
    while (SOPC_SLinkedList_GetLength(periodicTimersToRestart) > 0)
    {
        timer = (SOPC_EventTimer*) SOPC_SLinkedList_PopHead(periodicTimersToRestart);
        if (NULL != timer)
        {
            SOPC_InternalEventTimer_RestartPeriodicTimer_WithoutLock(timer);
        }
    }
    Mutex_Unlock(&timersMutex);
}
