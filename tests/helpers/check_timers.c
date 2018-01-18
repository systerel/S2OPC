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

/** \file
 *
 * \brief Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include "check_helpers.h"

#include <check.h>
#include <stdlib.h>

#include "sopc_builtintypes.h"
#include "sopc_event_dispatcher_manager.h"
#include "sopc_event_timer_manager.h"

#define NB_TIMERS 5
#define EVENT 1000

// Common and nominal
static uint32_t timersTriggered = 0;
const uint64_t timersDelay[NB_TIMERS] = {1000, 550, 800, 250, 400};
const uint8_t eltsIdNoCancel[NB_TIMERS] = {3, 4, 1, 2, 0}; // position of timers ordered by increasing delay
static SOPC_DateTime dateTimeResults[NB_TIMERS] = {0, 0, 0, 0, 0};
static SOPC_DateTime startTime = 0;

// With cancellation
#define NB_TIMERS_WITH_CANCEL 3
#define NB_TIMERS_TO_CANCEL 2
static uint32_t timersTriggeredWithCancel = 0;
const uint64_t timersDelayWithCancel[NB_TIMERS] = {250, 400, 800, 550, 1000};
// position of (not canceled) timers ordered by increasing delay regarding timersDelay array
static SOPC_DateTime dateTimeResultsWithCancel[NB_TIMERS_WITH_CANCEL] = {0, 0, 0};

uint8_t timersId[NB_TIMERS];

void timeout_event(int32_t event, uint32_t eltId, void* params, uint32_t auxParam)
{
    ck_assert(EVENT == event);
    ck_assert(eltId < NB_TIMERS);
    ck_assert(eltId == eltsIdNoCancel[timersTriggered]);
    ck_assert(auxParam == eltsIdNoCancel[timersTriggered]);
    SOPC_DateTime* dateTime = (SOPC_DateTime*) params;
    ck_assert(NULL != dateTime);
    *dateTime = SOPC_Time_GetCurrentTimeUTC();
    timersTriggered++;
}

START_TEST(test_timers)
{
    uint8_t i = 0;
    uint32_t timerId = 0;
    uint64_t elapsedMs = 0;
    SOPC_EventDispatcherParams eventParams;
    SOPC_EventDispatcherManager* eventMgr =
        SOPC_EventDispatcherManager_CreateAndStart(timeout_event, "Test event timers");
    ck_assert(eventMgr != NULL);

    // Initialize event timers
    SOPC_EventTimer_Initialize();
    // Set event value
    eventParams.event = EVENT;
    // Set start time reference
    startTime = SOPC_Time_GetCurrentTimeUTC();

    for (i = 0; i < NB_TIMERS; i++)
    {
        eventParams.eltId = i;
        eventParams.auxParam = i;
        eventParams.params = &dateTimeResults[i];
        timerId = SOPC_EventTimer_Create(eventMgr, eventParams, timersDelay[i]);
        ck_assert(timerId != 0);
    }

    while (timersTriggered < NB_TIMERS)
    {
        SOPC_Sleep(50);
        // Manually trigger the timers evaluation
        SOPC_EventTimer_CyclicTimersEvaluation();
    }

    for (i = 0; i < NB_TIMERS; i++)
    {
        elapsedMs = (dateTimeResults[i] - startTime) / 10000; // 100 nanoseconds to milliseconds
        // Check computed elapsed time value (on non monotonic clock) is delayMs +/- 50ms
        ck_assert(timersDelay[i] - 50 < elapsedMs && elapsedMs < timersDelay[i] + 50);
    }

    SOPC_EventDispatcherManager_StopAndDelete(&eventMgr);
    SOPC_EventTimer_Clear();
}
END_TEST

void canceled_timeout_event(int32_t event, uint32_t eltId, void* params, uint32_t auxParam)
{
    ck_assert(EVENT == event);
    ck_assert(eltId < NB_TIMERS);
    ck_assert(eltId == timersTriggeredWithCancel);
    ck_assert(auxParam == timersTriggeredWithCancel);
    SOPC_DateTime* dateTime = (SOPC_DateTime*) params;
    ck_assert(NULL != dateTime);
    *dateTime = SOPC_Time_GetCurrentTimeUTC();
    timersTriggeredWithCancel++;
}

START_TEST(test_timers_with_cancellation)
{
    uint8_t i = 0;
    uint64_t elapsedMs = 0;
    SOPC_EventDispatcherParams eventParams;
    SOPC_EventDispatcherManager* eventMgr =
        SOPC_EventDispatcherManager_CreateAndStart(canceled_timeout_event, "Test event timers");
    ck_assert(eventMgr != NULL);

    // Initialize event timers
    SOPC_EventTimer_Initialize();
    // Set event value
    eventParams.event = EVENT;
    // Set start time reference
    startTime = SOPC_Time_GetCurrentTimeUTC();

    for (i = 0; i < NB_TIMERS; i++)
    {
        eventParams.eltId = i;
        eventParams.auxParam = i;
        eventParams.params = &dateTimeResultsWithCancel[i];
        timersId[i] = SOPC_EventTimer_Create(eventMgr, eventParams, timersDelayWithCancel[i]);
        ck_assert(timersId[i] != 0);
    }

    i = 0;
    while (timersTriggeredWithCancel < NB_TIMERS_WITH_CANCEL)
    {
        SOPC_Sleep(50);
        // Manually trigger the timers evaluation
        SOPC_EventTimer_CyclicTimersEvaluation();
        // Delete two last timers
        if (i >= NB_TIMERS - 2 && i < NB_TIMERS)
        {
            // Cancel timers
            SOPC_EventTimer_Cancel(timersId[i]);
        }
        i++;
    }

    for (i = 0; i < NB_TIMERS_WITH_CANCEL; i++)
    {
        elapsedMs = (dateTimeResultsWithCancel[i] - startTime) / 10000; // 100 nanoseconds to milliseconds
        // Check computed elapsed time value (on non monotonic clock) is delayMs +/- 50ms
        ck_assert(timersDelayWithCancel[i] - 50 < elapsedMs && elapsedMs < timersDelayWithCancel[i] + 50);
    }

    SOPC_EventDispatcherManager_StopAndDelete(&eventMgr);
    SOPC_EventTimer_Clear();
}
END_TEST

Suite* tests_make_suite_timers(void)
{
    Suite* s;
    TCase* tc_timers;

    s = suite_create("Event timers");
    tc_timers = tcase_create("Timeouts");
    tcase_add_test(tc_timers, test_timers);
    tcase_add_test(tc_timers, test_timers_with_cancellation);
    suite_add_tcase(s, tc_timers);

    return s;
}
