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
#include <string.h>

#include "sopc_mutexes.h"
#include "sopc_singly_linked_list.h"

typedef struct SOPC_EventTimer
{
    uint32_t id;
    SOPC_EventDispatcherManager* eventMgr;
    SOPC_EventDispatcherParams eventParams;
    SOPC_TimeReference endTime;
} SOPC_EventTimer;

#define SOPC_MAX_TIMERS UINT16_MAX

static bool usedTimerIds[SOPC_MAX_TIMERS + 1]; // 0 idx value is invalid (max idx = MAX + 1)
static uint32_t latestTimerId = 0;

static SOPC_SLinkedList* timers = NULL;

static Mutex timersMutex;
static bool initialized = false;

// Caller should lock the mutex
// 0 result is invalid
static uint32_t SOPC_Internal_GetFreshTimerId_WithoutLock(void)
{
    uint32_t result = 0;

    uint32_t idx = latestTimerId;
    if (SOPC_SLinkedList_GetLength(timers) < SOPC_MAX_TIMERS)
    {
        if (result < SOPC_MAX_TIMERS + 1) // 0 idx value is invalid (max idx = MAX + 1)
        {
            idx = latestTimerId + 1;
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
            if (idx < SOPC_MAX_TIMERS + 1) // 0 idx value is invalid (max idx = MAX + 1)
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
    if (false == initialized)
    {
        Mutex_Initialization(&timersMutex);
        memset(usedTimerIds, false, sizeof(bool) * (SOPC_MAX_TIMERS + 1)); // 0 idx value is invalid (max idx = MAX + 1)
        timers = SOPC_SLinkedList_Create(SOPC_MAX_TIMERS);
        initialized = true;
    }
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
    Mutex_Unlock(&timersMutex);
    initialized = false;
}

uint32_t SOPC_EventTimer_Create(SOPC_EventDispatcherManager* eventMgr,
                                SOPC_EventDispatcherParams eventParams,
                                uint64_t msDelay)
{
    SOPC_EventTimer* newTimer = NULL;
    SOPC_TimeReference targetTime = 0;
    uint32_t result = 0;
    void* insertResult = NULL;

    if (false != initialized)
    {
        // Create target time reference
        targetTime = SOPC_TimeReference_AddMilliseconds(SOPC_TimeReference_GetCurrent(), msDelay);
        // Allocate new timer
        newTimer = calloc(1, sizeof(SOPC_EventTimer));

        if (NULL != newTimer)
        {
            // Configure timeout parameters
            newTimer->endTime = targetTime;
            newTimer->eventMgr = eventMgr;
            newTimer->eventParams = eventParams;

            // Set timer
            Mutex_Lock(&timersMutex);
            result = SOPC_Internal_GetFreshTimerId_WithoutLock();
            if (result != 0)
            {
                newTimer->id = result;
                // valid timer Id
                insertResult = SOPC_SLinkedList_SortedInsert(timers, result, newTimer,
                                                             SOPC_Internal_SLinkedList_EventTimerCompare);
                if (insertResult == NULL)
                {
                    result = 0;
                    free(newTimer);
                }
            } // else 0 is invalid value => no timer available
            Mutex_Unlock(&timersMutex);
        }
    }
    return result;
}

static void SOPC_Internal_EventTimer_Cancel_WithoutLock(uint32_t timerId)
{
    SOPC_EventTimer* timer = NULL;
    if (usedTimerIds[timerId] != false)
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
    if (false != initialized && timerId > 0)
    {
        Mutex_Lock(&timersMutex);
        SOPC_Internal_EventTimer_Cancel_WithoutLock(timerId);
        Mutex_Unlock(&timersMutex);
    }
}

void SOPC_EventTimer_CyclicTimersEvaluation()
{
    SOPC_SLinkedListIterator timerIt = NULL;
    SOPC_EventTimer* timer = NULL;
    SOPC_TimeReference currentTimeRef = 0;
    int8_t compareResult = 0;
    uint32_t timerId = 0;

    if (false != initialized)
    {
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
            // Remove the triggered timer (possible during iteration since we remove already iterated item)
            SOPC_Internal_EventTimer_Cancel_WithoutLock(timerId);

            // Prepare next timeout evaluation
            timer = (SOPC_EventTimer*) SOPC_SLinkedList_Next(&timerIt);
            if (timer != NULL)
            {
                compareResult = SOPC_TimeReference_Compare(currentTimeRef, timer->endTime);
            }
        }
        Mutex_Unlock(&timersMutex);
    }
}
