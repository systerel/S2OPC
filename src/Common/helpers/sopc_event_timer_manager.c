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

#include <assert.h>
#include <inttypes.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_event_timer_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_singly_linked_list.h"
#include "sopc_threads.h"

typedef struct SOPC_EventTimer
{
    uint32_t id;
    SOPC_EventHandler* eventHandler;
    SOPC_Event event;
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
static int32_t stop = 0;
static bool timerCreationFailed = false;

static Thread cyclicEvalThread;

static bool is_initialized(void)
{
    return SOPC_Atomic_Int_Get(&initialized) != 0;
}

static bool is_stopped(void)
{
    return SOPC_Atomic_Int_Get(&stop) != 0;
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

static int8_t SOPC_Internal_SLinkedList_EventTimerCompare(void* left, void* right)
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
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                   "EventTimerManager: failed to restart the periodic timer on insertion id=%" PRIu32
                                   " with event=%" PRIi32 " and associated id=%" PRIu32,
                                   timer->id, timer->event.event, timer->event.eltId);
            SOPC_Free(timer);
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                               "EventTimerManager: failed to restart the disabled periodic timer id=%" PRIu32
                               " with event=%" PRIi32 " and associated id=%" PRIu32,
                               timer->id, timer->event.event, timer->event.eltId);
        SOPC_Free(timer);
    }
}

static void SOPC_Internal_EventTimer_Cancel_WithoutLock(uint32_t timerId)
{
    SOPC_EventTimer* timer = NULL;
    if (usedTimerIds[timerId])
    {
        timer = SOPC_SLinkedList_RemoveFromId(timers, timerId);
        if (timer != NULL)
        {
            SOPC_Free(timer);
        }
        usedTimerIds[timerId] = false;
    }
}

static void SOPC_EventTimer_CyclicTimersEvaluation(void)
{
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
        SOPC_ReturnStatus status = SOPC_EventHandler_Post(timer->eventHandler, timer->event.event, timer->event.eltId,
                                                          timer->event.params, timer->event.auxParam);
        SOPC_UNUSED_RESULT(status);
        SOPC_ASSERT(status == SOPC_STATUS_OK);

        if (timer->isPeriodicTimer)
        {
            // Set next target time reference
            assert(timer->periodMs > 0 && "A periodic timer cannot have a period of 0 ms");
            timer->endTime = SOPC_TimeReference_AddMilliseconds(timer->endTime, timer->periodMs);
            compareResult = SOPC_TimeReference_Compare(currentTimeRef, timer->endTime);

            uint16_t loopLimit = SOPC_TIMER_RESOLUTION_MS;
            // Generate missed events until target time is greater than current time
            while (compareResult >= 0 && loopLimit > 0)
            {
                loopLimit--;
                status = SOPC_EventHandler_Post(timer->eventHandler, timer->event.event, timer->event.eltId,
                                                timer->event.params, timer->event.auxParam);
                SOPC_ASSERT(status == SOPC_STATUS_OK);
                // Set next target time reference
                timer->endTime = SOPC_TimeReference_AddMilliseconds(timer->endTime, timer->periodMs);
                compareResult = SOPC_TimeReference_Compare(currentTimeRef, timer->endTime);
            }
            if (compareResult >= 0)
            {
                SOPC_Logger_TraceWarning(
                    SOPC_LOG_MODULE_COMMON,
                    "EventTimerManager: limit number of generated events during 1 timer evaluation "
                    "reached, some expiration events will not be generated: id=%" PRIu32 " with event=%" PRIi32
                    ", period=%" PRIu64 " and associated id=%" PRIu32,
                    timer->id, timer->event.event, timer->periodMs, timer->event.eltId);
            }

            // Add to list of timers to restart it
            if (timer != SOPC_SLinkedList_Append(periodicTimersToRestart, timer->id, (void*) timer))
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_COMMON,
                    "EventTimerManager: failed to restart the periodic timer on insertion id=%" PRIu32
                    " with event=%" PRIi32 " and associated id=%" PRIu32,
                    timer->id, timer->event.event, timer->event.eltId);
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

static void* SOPC_Internal_ThreadLoop(void* arg)
{
    SOPC_UNUSED_ARG(arg);

    if (!is_initialized())
    {
        return NULL;
    }

    while (!is_stopped())
    {
        SOPC_EventTimer_CyclicTimersEvaluation();
        SOPC_Sleep(SOPC_TIMER_RESOLUTION_MS);
    }
    return NULL;
}

void SOPC_EventTimer_Initialize(void)
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
    SOPC_Atomic_Int_Set(&stop, 0);

    if (SOPC_Thread_Create(&cyclicEvalThread, SOPC_Internal_ThreadLoop, NULL, "Timers") != SOPC_STATUS_OK)
    {
        assert(false);
    }
}

void SOPC_EventTimer_Clear(void)
{
    SOPC_Atomic_Int_Set(&stop, 1);
    Mutex_Lock(&timersMutex);
    SOPC_SLinkedList_Apply(timers, SOPC_SLinkedList_EltGenericFree);
    SOPC_SLinkedList_Delete(timers);
    timers = NULL;
    // No need to iterate to free elements, elements are temporary added only during timer evaluation
    SOPC_SLinkedList_Delete(periodicTimersToRestart);
    periodicTimersToRestart = NULL;
    Mutex_Unlock(&timersMutex);
    SOPC_Thread_Join(cyclicEvalThread);
    SOPC_Atomic_Int_Set(&initialized, 0);
    Mutex_Clear(&timersMutex);
}

static uint32_t SOPC_InternalEventTimer_Create(SOPC_EventHandler* eventHandler,
                                               SOPC_Event event,
                                               uint64_t msDelay,
                                               bool isPeriodic)
{
    if (!is_initialized() || NULL == eventHandler || 0 == msDelay)
    {
        return 0;
    }

    if (isPeriodic && msDelay < 2 * SOPC_TIMER_RESOLUTION_MS)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_COMMON,
            "EventTimerManager: creating an event timer with a period value less than 2 times the event timers "
            "resolution (%" PRIu64 " < 2*%u) with event=%" PRIi32,
            msDelay, SOPC_TIMER_RESOLUTION_MS, event.event);
        return 0;
    }

    SOPC_EventTimer* newTimer = NULL;
    SOPC_TimeReference targetTime = 0;
    uint32_t result = 0;
    void* insertResult = NULL;

    // Create target time reference
    targetTime = SOPC_TimeReference_AddMilliseconds(SOPC_TimeReference_GetCurrent(), msDelay);
    // Allocate new timer
    newTimer = SOPC_Calloc(1, sizeof(SOPC_EventTimer));

    if (newTimer == NULL)
    {
        return 0;
    }

    // Configure timeout parameters
    newTimer->endTime = targetTime;
    newTimer->eventHandler = eventHandler;
    newTimer->event = event;
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
            SOPC_Free(newTimer);
        }
    } // else 0 is invalid value => no timer available
    else
    {
        if (!timerCreationFailed) // Log only on first failure due to no timer available
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON,
                                   "EventTimerManager: failed to create a new timer since no timer available");
        }
        SOPC_Free(newTimer);
    }
    timerCreationFailed = (0 == result);
    Mutex_Unlock(&timersMutex);

    return result;
}

uint32_t SOPC_EventTimer_Create(SOPC_EventHandler* eventHandler, SOPC_Event event, uint64_t msDelay)
{
    return SOPC_InternalEventTimer_Create(eventHandler, event, msDelay, false);
}

uint32_t SOPC_EventTimer_CreatePeriodic(SOPC_EventHandler* eventHandler, SOPC_Event event, uint64_t msPeriod)
{
    return SOPC_InternalEventTimer_Create(eventHandler, event, msPeriod, true);
}

bool SOPC_EventTimer_ModifyPeriodic(uint32_t timerId, uint64_t msPeriod)
{
    if (!is_initialized() || 0 == timerId || 0 == msPeriod)
    {
        return false;
    }

    bool result = false;
    SOPC_EventTimer* timer = NULL;
    Mutex_Lock(&timersMutex);
    timer = (SOPC_EventTimer*) SOPC_SLinkedList_FindFromId(timers, timerId);
    if (timer != NULL && timer->isPeriodicTimer)
    {
        if (msPeriod < 2 * SOPC_TIMER_RESOLUTION_MS)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_COMMON,
                "EventTimerManager: modifying an event timer with a period value less than 2 times the event timers "
                "resolution (%" PRIu64 " < 2*%u) with id=%" PRIu32 "event=%" PRIi32,
                msPeriod, SOPC_TIMER_RESOLUTION_MS, timerId, timer->event.event);
        }
        else
        {
            result = true;
            timer->periodMs = msPeriod;
        }
    }
    Mutex_Unlock(&timersMutex);
    return result;
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
