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

// ============================================================================
// INCLUDE
// ============================================================================
#include <check.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

// The macro used in this Unit Test: triggers an error if requested.
static bool g_fail_clock_gettime = false;
#define clock_gettime(clk_id, tp) (g_fail_clock_gettime ? (errno = EFAULT, -1) : clock_gettime(clk_id, tp))

#include "sopc_date_time.h"
#include "sopc_enums.h"
#include "sopc_threads.h"
#include "sopc_time_reference.h"

#undef clock_gettime

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
    ck_assert_msg(t1 > 0, "TimeReference should be > 0");

    // Increasing test value
    SOPC_Sleep(1); // 1 ms
    SOPC_TimeReference t2 = SOPC_TimeReference_GetCurrent();
    ck_assert_msg(t2 > t1, "TimeReference should increase over time");

    // Monotonicity test on multiple calls
    SOPC_TimeReference prev = t2;
    for (int i = 0; i < 100; i++)
    {
        SOPC_TimeReference now = SOPC_TimeReference_GetCurrent();
        ck_assert_msg(now >= prev, "Monotonic clock violated");
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
    SOPC_TimeReference t_max = UINT64_MAX - 10;
    SOPC_TimeReference result = SOPC_TimeReference_AddMilliseconds(t_max, 100);
    ck_assert_msg(result == UINT64_MAX, "Expected saturation on overflow");

    // Chain of additions test
    SOPC_TimeReference current = 0;
    uint64_t total_ms = 0;

    for (int i = 0; i < 100; i++)
    {
        uint64_t inc = (uint64_t)(i + 1) * 10u;
        total_ms += inc;
        current = SOPC_TimeReference_AddMilliseconds(current, inc);

        ck_assert_msg(current == total_ms, "After cumulative additions, expected %llu but got %llu (iter %d)",
                      (unsigned long long) total_ms, (unsigned long long) current, i);
    }

    current = UINT64_MAX - 100;
    bool saturated = false;
    for (int i = 0; i < 10; i++)
    {
        SOPC_TimeReference previous = current;
        current = SOPC_TimeReference_AddMilliseconds(current, 50);

        ck_assert_msg(current >= previous, "Expected monotonic increase or saturation (iter %d)", i);

        if (current == UINT64_MAX)
        {
            saturated = true;
        }
    }

    // At the end of the chain, we must be saturated
    ck_assert_msg(saturated && current == UINT64_MAX, "Expected saturation after overflow chain (final value %llu)",
                  (unsigned long long) current);
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

    // GetTime fills in a valid structure
    SOPC_HighRes_TimeReference_GetTime(t);
    ck_assert_ptr_nonnull(t);
    SOPC_HighRes_TimeReference_GetTime(NULL);

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
    SOPC_HighRes_TimeReference_Copy(t2, NULL);

    SOPC_HighRes_TimeReference_Delete(&t1);
    SOPC_HighRes_TimeReference_Delete(&t2);
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
    ck_assert_msg(delta > 0, "Delta should be positive");

    // Negative delta test
    int64_t delta_neg = SOPC_HighRes_TimeReference_DeltaUs(t2, t1);
    ck_assert_msg(delta_neg < 0, "Delta should be negative");

    // Zero delta test
    int64_t delta_zero = SOPC_HighRes_TimeReference_DeltaUs(t1, t1);
    ck_assert_int_eq(delta_zero, 0);

    // NULL delta test
    int64_t delta_auto = SOPC_HighRes_TimeReference_DeltaUs(t1, NULL);
    ck_assert_msg(delta_auto > 0, "Delta should be positive when comparing to current time");

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
    SOPC_HighRes_TimeReference* t_init = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_GetTime(t_init);

    SOPC_HighRes_TimeReference* t_test = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_Copy(t_test, t_init);

    int64_t delta_before = SOPC_HighRes_TimeReference_DeltaUs(t_init, t_test);
    ck_assert_int_eq(delta_before, 0);

    // Simple addition without offset
    SOPC_HighRes_TimeReference_AddSynchedDuration(t_test, 100000, -1); // +100 ms
    int64_t delta_us = SOPC_HighRes_TimeReference_DeltaUs(t_init, t_test);
    ck_assert_msg(delta_us > 0, "Duration should increase time reference");

    // Add with offset in the window (offset < duration)
    SOPC_HighRes_TimeReference_Copy(t_test, t_init);
    SOPC_HighRes_TimeReference_AddSynchedDuration(t_test, 100000, 20000); // période 100 ms, offset 20 ms
    delta_us = SOPC_HighRes_TimeReference_DeltaUs(t_init, t_test);
    ck_assert_msg(delta_us >= 0, "Offset +20ms must still move forward");

    // Borderline case: offset = 0
    SOPC_HighRes_TimeReference_Copy(t_test, t_init);
    SOPC_HighRes_TimeReference_AddSynchedDuration(t_test, 100000, 0);
    delta_us = SOPC_HighRes_TimeReference_DeltaUs(t_init, t_test);
    ck_assert_msg(delta_us >= 0, "Offset 0 should be valid and positive");

    // Borderline case: offset = duration (should return to the beginning of the next period)
    SOPC_HighRes_TimeReference_Copy(t_test, t_init);
    SOPC_HighRes_TimeReference_AddSynchedDuration(t_test, 100000, 100000);
    delta_us = SOPC_HighRes_TimeReference_DeltaUs(t_init, t_test);
    ck_assert_msg(delta_us >= 0, "Offset equal to duration should still move forward");

    // Borderline case: negative offset (equivalent to “no window,” simple addition)
    SOPC_HighRes_TimeReference_Copy(t_test, t_init);
    SOPC_HighRes_TimeReference_AddSynchedDuration(t_test, 100000, -100);
    delta_us = SOPC_HighRes_TimeReference_DeltaUs(t_init, t_test);
    ck_assert_msg(delta_us > 0, "Negative offset should be treated as simple duration addition");

    // Duration not a multiple of 1 second (undefined behavior, check stability)
    SOPC_HighRes_TimeReference_Copy(t_test, t_init);
    SOPC_HighRes_TimeReference_AddSynchedDuration(t_test, 123456, -1);
    delta_us = SOPC_HighRes_TimeReference_DeltaUs(t_init, t_test);
    ck_assert_msg(delta_us > 0, "Non-multiple of 1s should not crash and still advance");

    SOPC_HighRes_TimeReference_Delete(&t_init);
    SOPC_HighRes_TimeReference_Delete(&t_test);
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
    ck_assert_msg(SOPC_HighRes_TimeReference_IsExpired(t, NULL));

    // Test future date
    SOPC_HighRes_TimeReference_GetTime(t);
    SOPC_HighRes_TimeReference_AddSynchedDuration(t, 200000, -1);
    ck_assert_msg(!SOPC_HighRes_TimeReference_IsExpired(t, NULL));

    SOPC_Sleep(250);
    ck_assert(SOPC_HighRes_TimeReference_IsExpired(t, NULL));

    // Explicit comparison with a provided "now"
    SOPC_HighRes_TimeReference_GetTime(t);
    SOPC_HighRes_TimeReference_GetTime(now);

    SOPC_HighRes_TimeReference_AddSynchedDuration(t, 100000, -1);
    ck_assert_msg(!SOPC_HighRes_TimeReference_IsExpired(t, now));

    SOPC_HighRes_TimeReference_GetTime(t);
    SOPC_Sleep(3);
    SOPC_HighRes_TimeReference_GetTime(now);
    ck_assert_msg(SOPC_HighRes_TimeReference_IsExpired(t, now));

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
    SOPC_HighRes_TimeReference* t_future = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_GetTime(t_future);
    SOPC_HighRes_TimeReference_AddSynchedDuration(t_future, 200000, -1);

    SOPC_TimeReference start = SOPC_TimeReference_GetCurrent();
    SOPC_HighRes_TimeReference_SleepUntil(t_future);
    SOPC_TimeReference end = SOPC_TimeReference_GetCurrent();

    ck_assert_msg((end - start) >= 150);

    SOPC_HighRes_TimeReference_Delete(&t_future);

    // Test date passed
    SOPC_HighRes_TimeReference* t_past = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_GetTime(t_past);
    SOPC_Sleep(5);

    start = SOPC_TimeReference_GetCurrent();
    SOPC_HighRes_TimeReference_SleepUntil(t_past);
    end = SOPC_TimeReference_GetCurrent();

    ck_assert_msg((end - start) < 20);

    SOPC_HighRes_TimeReference_Delete(&t_past);
}
END_TEST

// ============================================================================
// TESTS SOPC_ASSERT
// ============================================================================

/*
 * Testing the SOPC_HighRes_TimeReference_GetTime() function.
 * Verify that the function triggers a SOPC_ASSERT if clock_gettime fails.
 */
START_TEST(test_highRes_timeReference_getTime_sopcAssert)
{
    g_fail_clock_gettime = true; // Force clock_gettime() to fail
    SOPC_HighRes_TimeReference* t = SOPC_HighRes_TimeReference_Create();

    // Must raise an assertion (SIGABRT expected)
    SOPC_HighRes_TimeReference_GetTime(t);
    ck_abort_msg("SOPC_ASSERT was not triggered even though clock_gettime failed");

    g_fail_clock_gettime = false;
    SOPC_HighRes_TimeReference_Delete(&t);
}
END_TEST

/*
 * Testing the SOPC_HighRes_TimeReference_SleepUntil() function.
 * Verify that the function triggers a SOPC_ASSERT if a NULL value is passed as a parameter.
 */
START_TEST(test_highRes_timeReference_sleepUntil_sopcAssert)
{
    SOPC_HighRes_TimeReference_SleepUntil(NULL);
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

    TCase* tc_highres_assert = tcase_create("HighResTimeReferenceSOPCAssert");
    tcase_add_exit_test(tc_highres_assert, test_highRes_timeReference_getTime_sopcAssert, EXIT_FAILURE);
    tcase_add_test_raise_signal(tc_highres_assert, test_highRes_timeReference_sleepUntil_sopcAssert, SIGABRT);
    suite_add_tcase(suite, tc_highres_assert);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
