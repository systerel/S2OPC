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

#include <check.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "sopc_date_time.h"
#include "sopc_enums.h"
#include "sopc_threads.h"
#include "sopc_time_reference.h"

// ============================================================================
// TESTS SOPC_TimeReference_*
// ============================================================================

/*
 * Testing the SOPC_TimeReference_GetCurrent() function.
 * Verify that the function returns an increasing (monotonic) time and not zero.
 */
START_TEST(test_timeReference_getCurrent)
{
    // Non-zero test value
    SOPC_TimeReference t1 = SOPC_TimeReference_GetCurrent();
    ck_assert_uint_gt(t1, 0);

    // Increasing test value
    SOPC_Sleep(1); // 1 ms
    SOPC_TimeReference t2 = SOPC_TimeReference_GetCurrent();
    ck_assert_uint_gt(t2, t1);

    // Monotonicity test on multiple calls
    SOPC_TimeReference prev = t2;
    for (int i = 0; i < 100; i++)
    {
        SOPC_TimeReference now = SOPC_TimeReference_GetCurrent();
        ck_assert_uint_ge(now, prev);
        prev = now;
    }
}
END_TEST

/*
 * Testing the SOPC_TimeReference_AddMilliseconds() function.
 * Verify the correct increment of a reference time.
 */
START_TEST(test_timeReference_addMilliseconds)
{
    // Zero and simple addition test
    SOPC_TimeReference t = 1000;
    ck_assert_uint_eq(SOPC_TimeReference_AddMilliseconds(t, 0), 1000);
    ck_assert_uint_eq(SOPC_TimeReference_AddMilliseconds(t, 500), 1500);

    // Overflow test
    SOPC_TimeReference tMax = UINT64_MAX - 10;
    SOPC_TimeReference result = SOPC_TimeReference_AddMilliseconds(tMax, 100);
    ck_assert_uint_eq(result, UINT64_MAX);

    // Chain of additions test
    SOPC_TimeReference current = 0;
    uint64_t totalMs = 0;

    for (uint64_t i = 0; i < 100; i++)
    {
        uint64_t inc = (i + 1) * 10u;
        totalMs += inc;
        current = SOPC_TimeReference_AddMilliseconds(current, inc);

        ck_assert_uint_eq(current, totalMs);
    }

    current = UINT64_MAX - 100;
    bool saturated = false;
    for (int i = 0; i < 10; i++)
    {
        SOPC_TimeReference previous = current;
        current = SOPC_TimeReference_AddMilliseconds(current, 50);

        ck_assert_uint_ge(current, previous);

        if (current == UINT64_MAX)
        {
            saturated = true;
        }
    }

    // At the end of the chain, we must be saturated
    ck_assert(saturated);
    ck_assert_uint_eq(current, UINT64_MAX);
}
END_TEST

/*
 * Testing the SOPC_TimeReference_Compare() function.
 * Verify the behavior of the comparison.
 */
START_TEST(test_timeReference_compare)
{
    // Equality test
    ck_assert_int_eq(SOPC_TimeReference_Compare(1000, 1000), 0);

    // Lower/higher test
    ck_assert_int_eq(SOPC_TimeReference_Compare(1000, 2000), -1);
    ck_assert_int_eq(SOPC_TimeReference_Compare(2000, 1000), 1);

    // Borderline cases test
    ck_assert_int_eq(SOPC_TimeReference_Compare(0, UINT64_MAX), -1);
    ck_assert_int_eq(SOPC_TimeReference_Compare(UINT64_MAX, 0), 1);
    ck_assert_int_eq(SOPC_TimeReference_Compare(UINT64_MAX - 1, UINT64_MAX), -1);
    ck_assert_int_eq(SOPC_TimeReference_Compare(0, 1), -1);
    ck_assert_int_eq(SOPC_TimeReference_Compare(UINT64_MAX, UINT64_MAX), 0);

    // Symmetry properties test
    int8_t res1 = SOPC_TimeReference_Compare(12345, 67890);
    int8_t res2 = SOPC_TimeReference_Compare(67890, 12345);
    ck_assert_int_eq(res1, -res2);
}
END_TEST

// ============================================================================
// TESTS SOPC_HighRes_TimeReference_*
// ============================================================================

/*
 * Testing the SOPC_HighRes_TimeReference_Create() / GetTime() / Delete() functions.
 * Verify the lifecycle of the high-resolution time object.
 */
START_TEST(test_highRes_timeReference_create_getTime_delete)
{
    // Valid creation
    SOPC_HighRes_TimeReference* t = SOPC_HighRes_TimeReference_Create();
    ck_assert_ptr_nonnull(t);

    SOPC_HighRes_TimeReference_GetTime(t);
    ck_assert_ptr_nonnull(t);

    // GetTime test with NULL parameter
    SOPC_HighRes_TimeReference* tNull = NULL;
    SOPC_HighRes_TimeReference_GetTime(tNull);
    ck_assert_ptr_null(tNull);

    // Valid deletion
    SOPC_HighRes_TimeReference_Delete(&t);
    ck_assert_ptr_null(t);

    // Double delete must be safe
    SOPC_HighRes_TimeReference_Delete(&t);
    ck_assert_ptr_null(t);

    // Delete on explicit NULL pointer
    SOPC_HighRes_TimeReference* nullPtr = NULL;
    SOPC_HighRes_TimeReference_Delete(&nullPtr);
    ck_assert_ptr_null(nullPtr);
}
END_TEST

/*
 * Testing the SOPC_HighRes_TimeReference_Copy() function.
 * Verify deep copying of structures.
 */
START_TEST(test_highRes_timeReference_copy)
{
    SOPC_HighRes_TimeReference* t1 = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference* t2 = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_GetTime(t1);

    // Normal copy test
    SOPC_HighRes_TimeReference_Copy(t2, t1);
    int64_t delta = SOPC_HighRes_TimeReference_DeltaUs(t1, t2);
    ck_assert_int_eq(delta, 0);

    // Copy ignored if pointers are NULL
    SOPC_HighRes_TimeReference_Copy(NULL, t1);

    SOPC_HighRes_TimeReference* t2Before = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_Copy(t2Before, t2);
    SOPC_HighRes_TimeReference_Copy(t2, NULL);
    int64_t deltaAfter = SOPC_HighRes_TimeReference_DeltaUs(t2Before, t2);
    ck_assert_int_eq(deltaAfter, 0);

    SOPC_HighRes_TimeReference_Delete(&t1);
    SOPC_HighRes_TimeReference_Delete(&t2);
    SOPC_HighRes_TimeReference_Delete(&t2Before);
}
END_TEST

/*
 * Testing the SOPC_HighRes_TimeReference_DeltaUs() function.
 * Verify the time difference between two references.
 */
START_TEST(test_highRes_timeReference_deltaUs)
{
    SOPC_HighRes_TimeReference* t1 = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_GetTime(t1);

    SOPC_Sleep(2); // 2 ms

    SOPC_HighRes_TimeReference* t2 = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_GetTime(t2);

    // Positive delta test
    int64_t delta = SOPC_HighRes_TimeReference_DeltaUs(t1, t2);
    ck_assert_int_gt(delta, 0);

    // Negative delta test
    int64_t deltaNeg = SOPC_HighRes_TimeReference_DeltaUs(t2, t1);
    ck_assert_int_lt(deltaNeg, 0);

    // Zero delta test
    int64_t deltaZero = SOPC_HighRes_TimeReference_DeltaUs(t1, t1);
    ck_assert_int_eq(deltaZero, 0);

    // NULL delta test
    int64_t deltaAuto = SOPC_HighRes_TimeReference_DeltaUs(t1, NULL);
    ck_assert_int_gt(deltaAuto, 0);

    SOPC_HighRes_TimeReference_Delete(&t1);
    SOPC_HighRes_TimeReference_Delete(&t2);
}
END_TEST

/*
 * Testing the SOPC_HighRes_TimeReference_AddSynchedDuration() function.
 * Verify the update of t, respecting the period and offset.
 */
START_TEST(test_highRes_timeReference_addSynchedDuration)
{
    SOPC_HighRes_TimeReference* tInit = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_GetTime(tInit);

    SOPC_HighRes_TimeReference* tTest = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_Copy(tTest, tInit);

    int64_t deltaBefore = SOPC_HighRes_TimeReference_DeltaUs(tInit, tTest);
    ck_assert_int_eq(deltaBefore, 0);

    // Simple addition without offset
    SOPC_HighRes_TimeReference_AddSynchedDuration(tTest, 100000, -1); // +100 ms
    int64_t deltaUs = SOPC_HighRes_TimeReference_DeltaUs(tInit, tTest);
    ck_assert_int_gt(deltaUs, 0);

    // Add with offset in the window (offset < duration)
    SOPC_HighRes_TimeReference_Copy(tTest, tInit);
    SOPC_HighRes_TimeReference_AddSynchedDuration(tTest, 100000, 20000); // period 100 ms, offset 20 ms
    deltaUs = SOPC_HighRes_TimeReference_DeltaUs(tInit, tTest);
    ck_assert_int_ge(deltaUs, 0);

    // Borderline case: offset = 0
    SOPC_HighRes_TimeReference_Copy(tTest, tInit);
    SOPC_HighRes_TimeReference_AddSynchedDuration(tTest, 100000, 0);
    deltaUs = SOPC_HighRes_TimeReference_DeltaUs(tInit, tTest);
    ck_assert_int_ge(deltaUs, 0);

    // Borderline case: offset = duration (should return to the beginning of the next period)
    SOPC_HighRes_TimeReference_Copy(tTest, tInit);
    SOPC_HighRes_TimeReference_AddSynchedDuration(tTest, 100000, 100000);
    deltaUs = SOPC_HighRes_TimeReference_DeltaUs(tInit, tTest);
    ck_assert_int_ge(deltaUs, 0);

    // Borderline case: negative offset (equivalent to “no window,” simple addition)
    SOPC_HighRes_TimeReference_Copy(tTest, tInit);
    SOPC_HighRes_TimeReference_AddSynchedDuration(tTest, 100000, -100);
    deltaUs = SOPC_HighRes_TimeReference_DeltaUs(tInit, tTest);
    ck_assert_int_gt(deltaUs, 0);

    // Duration not a multiple of 1 second (undefined behavior, check stability)
    SOPC_HighRes_TimeReference_Copy(tTest, tInit);
    SOPC_HighRes_TimeReference_AddSynchedDuration(tTest, 123456, -1);
    deltaUs = SOPC_HighRes_TimeReference_DeltaUs(tInit, tTest);
    ck_assert_int_gt(deltaUs, 0);

    SOPC_HighRes_TimeReference_Delete(&tInit);
    SOPC_HighRes_TimeReference_Delete(&tTest);
}
END_TEST

/*
 * Testing the SOPC_HighRes_TimeReference_IsExpired() function.
 * Verify the timeout test.
 */
START_TEST(test_highRes_timeReference_isExpired)
{
    SOPC_HighRes_TimeReference* t = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference* now = SOPC_HighRes_TimeReference_Create();

    // Test date passed
    SOPC_HighRes_TimeReference_GetTime(t);
    SOPC_Sleep(2);
    ck_assert(SOPC_HighRes_TimeReference_IsExpired(t, NULL));

    // Test future date
    SOPC_HighRes_TimeReference_GetTime(t);
    SOPC_HighRes_TimeReference_AddSynchedDuration(t, 200000, -1);
    ck_assert(!SOPC_HighRes_TimeReference_IsExpired(t, NULL));

    SOPC_Sleep(250);
    ck_assert(SOPC_HighRes_TimeReference_IsExpired(t, NULL));

    // Explicit comparison with a provided "now"
    SOPC_HighRes_TimeReference_GetTime(t);
    SOPC_HighRes_TimeReference_GetTime(now);

    SOPC_HighRes_TimeReference_AddSynchedDuration(t, 100000, -1);
    ck_assert(!SOPC_HighRes_TimeReference_IsExpired(t, now));

    SOPC_HighRes_TimeReference_GetTime(t);
    SOPC_Sleep(3);
    SOPC_HighRes_TimeReference_GetTime(now);
    ck_assert(SOPC_HighRes_TimeReference_IsExpired(t, now));

    SOPC_HighRes_TimeReference_Delete(&t);
    SOPC_HighRes_TimeReference_Delete(&now);
}
END_TEST

/*
 * Testing the SOPC_HighRes_TimeReference_SleepUntil() function.
 * Verify that the function waits until a given date.
 */
START_TEST(test_highRes_timeReference_sleepUntil)
{
    // Test future date
    SOPC_HighRes_TimeReference* tFuture = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_GetTime(tFuture);
    SOPC_HighRes_TimeReference_AddSynchedDuration(tFuture, 200000, -1);

    SOPC_HighRes_TimeReference_SleepUntil(tFuture);

    // After SleepUntil, the current time must be >= tFuture
    ck_assert(SOPC_HighRes_TimeReference_IsExpired(tFuture, NULL));

    SOPC_HighRes_TimeReference_Delete(&tFuture);

    // Test date passed
    SOPC_HighRes_TimeReference* tPast = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_GetTime(tPast);
    SOPC_Sleep(5);

    SOPC_TimeReference start = SOPC_TimeReference_GetCurrent();
    SOPC_HighRes_TimeReference_SleepUntil(tPast);
    SOPC_TimeReference end = SOPC_TimeReference_GetCurrent();

    ck_assert_uint_lt((end - start), 100);

    SOPC_HighRes_TimeReference_Delete(&tPast);
}
END_TEST

// ============================================================================
// MAIN
// ============================================================================
int main(void)
{
    int number_failed;
    Suite* suite = suite_create("SOPC Time Reference Tests");
    SRunner* sr = srunner_create(suite);

    TCase* tc_time_ref = tcase_create("TimeReference");
    tcase_add_test(tc_time_ref, test_timeReference_getCurrent);
    tcase_add_test(tc_time_ref, test_timeReference_addMilliseconds);
    tcase_add_test(tc_time_ref, test_timeReference_compare);
    suite_add_tcase(suite, tc_time_ref);

    TCase* tc_highres = tcase_create("HighResTimeReference");
    tcase_add_test(tc_highres, test_highRes_timeReference_create_getTime_delete);
    tcase_add_test(tc_highres, test_highRes_timeReference_copy);
    tcase_add_test(tc_highres, test_highRes_timeReference_deltaUs);
    tcase_add_test(tc_highres, test_highRes_timeReference_addSynchedDuration);
    tcase_add_test(tc_highres, test_highRes_timeReference_isExpired);
    tcase_add_test(tc_highres, test_highRes_timeReference_sleepUntil);
    suite_add_tcase(suite, tc_highres);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
