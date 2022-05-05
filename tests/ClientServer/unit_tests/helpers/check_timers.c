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

/** \file
 *
 * \brief Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include "check_helpers.h"

#include <check.h>

#include "sopc_atomic.h"
#include "sopc_builtintypes.h"
#include "sopc_event_timer_manager.h"
#include "sopc_macros.h"

#define NB_TIMERS 5
#define EVENT 1000

// Common and nominal
static int32_t timersTriggered = 0;
const uint64_t timersDelay[NB_TIMERS] = {1000, 550, 800, 250, 400};
const uint8_t eltsIdNoCancel[NB_TIMERS] = {3, 4, 1, 2, 0}; // position of timers ordered by increasing delay
static SOPC_DateTime dateTimeResults[NB_TIMERS] = {0, 0, 0, 0, 0};
static SOPC_DateTime startTime = 0;

// With cancellation
#define NB_TIMERS_WITH_CANCEL 3
#define NB_TIMERS_TO_CANCEL 2
static int32_t timersTriggeredWithCancel = 0;
const uint64_t timersDelayWithCancel[NB_TIMERS] = {250, 400, 800, 550, 1000};
// position of (not canceled) timers ordered by increasing delay regarding timersDelay array
static SOPC_DateTime dateTimeResultsWithCancel[NB_TIMERS_WITH_CANCEL] = {0, 0, 0};

uint32_t timersId[NB_TIMERS];

static void timeout_event(SOPC_EventHandler* handler,
                          int32_t event,
                          uint32_t eltId,
                          uintptr_t params,
                          uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);

    uint32_t triggered = (uint32_t) SOPC_Atomic_Int_Get(&timersTriggered);

    ck_assert(EVENT == event);
    ck_assert(eltId < NB_TIMERS);
    ck_assert(eltId == eltsIdNoCancel[triggered]);
    ck_assert(auxParam == eltsIdNoCancel[triggered]);
    SOPC_DateTime* dateTime = (void*) params;
    ck_assert(NULL != dateTime);
    *dateTime = SOPC_Time_GetCurrentTimeUTC();

    SOPC_Atomic_Int_Add(&timersTriggered, 1);
}

START_TEST(test_timers)
{
    uint8_t i = 0;
    uint32_t timerId = 0;
    uint64_t elapsedMs = 0;
    SOPC_Event event;
    SOPC_Looper* looper = SOPC_Looper_Create("timers");
    ck_assert_ptr_nonnull(looper);

    SOPC_EventHandler* eventHandler = SOPC_EventHandler_Create(looper, timeout_event);
    ck_assert_ptr_nonnull(eventHandler);

    // Initialize event timers
    SOPC_EventTimer_Initialize();
    // Set event value
    memset(&event, 0, sizeof(SOPC_Event));
    event.event = EVENT;
    // Set start time reference
    startTime = SOPC_Time_GetCurrentTimeUTC();

    for (i = 0; i < NB_TIMERS; i++)
    {
        event.eltId = i;
        event.auxParam = i;
        event.params = (uintptr_t) &dateTimeResults[i];
        timerId = SOPC_EventTimer_Create(eventHandler, event, timersDelay[i]);
        ck_assert(timerId != 0);
    }

    while (SOPC_Atomic_Int_Get(&timersTriggered) < NB_TIMERS)
    {
        SOPC_Sleep(50);
    }

    for (i = 0; i < NB_TIMERS; i++)
    {
        ck_assert(dateTimeResults[i] >= 0 && startTime >= 0 && dateTimeResults[i] >= startTime);
        elapsedMs = ((uint64_t) dateTimeResults[i] - (uint64_t) startTime) / 10000; // 100 nanoseconds to milliseconds
        // Check computed elapsed time value (on non monotonic clock) is delayMs +/- 100ms
        ck_assert_uint_le(timersDelay[i] - 100, elapsedMs);
        ck_assert_uint_le(elapsedMs, timersDelay[i] + 100);
    }

    SOPC_Looper_Delete(looper);
    SOPC_EventTimer_Clear();
}
END_TEST

static void canceled_timeout_event(SOPC_EventHandler* handler,
                                   int32_t event,
                                   uint32_t eltId,
                                   uintptr_t params,
                                   uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);

    uint32_t triggeredWithCancel = (uint32_t) SOPC_Atomic_Int_Get(&timersTriggeredWithCancel);

    ck_assert(EVENT == event);
    ck_assert(eltId < NB_TIMERS);
    ck_assert(eltId == triggeredWithCancel);
    ck_assert(auxParam == triggeredWithCancel);
    SOPC_DateTime* dateTime = (void*) params;
    ck_assert(NULL != dateTime);
    *dateTime = SOPC_Time_GetCurrentTimeUTC();

    SOPC_Atomic_Int_Add(&timersTriggeredWithCancel, 1);
}

START_TEST(test_timers_with_cancellation)
{
    uint8_t i = 0;
    uint64_t elapsedMs = 0;
    SOPC_Event event;
    SOPC_Looper* looper = SOPC_Looper_Create("timers");
    ck_assert_ptr_nonnull(looper);

    SOPC_EventHandler* eventHandler = SOPC_EventHandler_Create(looper, canceled_timeout_event);
    ck_assert_ptr_nonnull(eventHandler);

    // Initialize event timers
    SOPC_EventTimer_Initialize();
    // Set event value
    memset(&event, 0, sizeof(SOPC_Event));
    event.event = EVENT;
    // Set start time reference
    startTime = SOPC_Time_GetCurrentTimeUTC();

    for (i = 0; i < NB_TIMERS_WITH_CANCEL; i++)
    {
        event.eltId = i;
        event.auxParam = i;
        event.params = (uintptr_t) &dateTimeResultsWithCancel[i];
        timersId[i] = SOPC_EventTimer_Create(eventHandler, event, timersDelayWithCancel[i]);
        ck_assert(timersId[i] != 0);
    }
    // Create and cancel two last timers
    for (i = NB_TIMERS_WITH_CANCEL; i < NB_TIMERS; i++)
    {
        event.eltId = i;
        event.auxParam = i;
        event.params = (uintptr_t) &dateTimeResults[i];
        timersId[i] = SOPC_EventTimer_Create(eventHandler, event, timersDelayWithCancel[i]);
        ck_assert(timersId[i] != 0);
    }
    for (i = NB_TIMERS_WITH_CANCEL; i < NB_TIMERS; i++)
    {
        // Cancel timers
        SOPC_EventTimer_Cancel(timersId[i]);
    }

    // Wait all timers (even canceled) should have timeout
    SOPC_Sleep(1500);

    // Check only created and non-canceled timers expired
    ck_assert(SOPC_Atomic_Int_Get(&timersTriggeredWithCancel) == NB_TIMERS_WITH_CANCEL);

    for (i = 0; i < NB_TIMERS_WITH_CANCEL; i++)
    {
        ck_assert(dateTimeResultsWithCancel[i] >= 0 && startTime >= 0 && dateTimeResultsWithCancel[i] >= startTime);
        elapsedMs =
            ((uint64_t) dateTimeResultsWithCancel[i] - (uint64_t) startTime) / 10000; // 100 nanoseconds to milliseconds
        // Check computed elapsed time value (on non monotonic clock) is delayMs +/- 100ms
        ck_assert_uint_le(timersDelayWithCancel[i] - 100, elapsedMs);
        ck_assert_uint_le(elapsedMs, timersDelayWithCancel[i] + 100);
    }

    SOPC_Looper_Delete(looper);
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
