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

#include <check.h>
#include <time.h>

#include "check_helpers.h"

#include "sopc_time.h"

/*
 * It represents the number of seconds between the OPC-UA (Windows) which starts on 1601/01/01 (supposedly 00:00:00
 * UTC), and Linux times starts on epoch, 1970/01/01 00:00:00 UTC.
 * */
#define SOPC_SECONDS_BETWEEN_EPOCHS 11644473600
/*
 * It represents the number of seconds between the 1900/01/01 00:00:00 UTC
 * and Linux times starts on epoch, 1970/01/01 00:00:00 UTC.
 * */
#define SOPC_SECONDS_SINCE_1900 2208988800

static const int64_t SOPC_SECONDS_BETWEEN_1601_1900 = SOPC_SECONDS_BETWEEN_EPOCHS - SOPC_SECONDS_SINCE_1900;

static const int64_t SOPC_SECONDS_TO_100_NANOSECONDS = 10000000; // 10^7
static const int64_t SOPC_SECONDS_IN_ONE_YEAR = 31556926;        // approximation: 365.24 days

START_TEST(test_current_time)
{
    time_t cTime;
    cTime = time(&cTime);
    struct tm* cStructTime = gmtime(&cTime);
    /* Check date is between 01/01/<current_year> and 01/01/<current_year + 1> */
    int64_t vDate = SOPC_Time_GetCurrentTimeUTC();
    ck_assert(vDate != 0);
    // tm_year field is the number of years since 1900: obtain the number of seconds and add the delta with year 1601
    ck_assert(vDate > (cStructTime->tm_year * SOPC_SECONDS_IN_ONE_YEAR + SOPC_SECONDS_BETWEEN_1601_1900) *
                          SOPC_SECONDS_TO_100_NANOSECONDS);
    ck_assert(vDate < ((cStructTime->tm_year + 1) * SOPC_SECONDS_IN_ONE_YEAR + SOPC_SECONDS_BETWEEN_1601_1900) *
                          SOPC_SECONDS_TO_100_NANOSECONDS);
}
END_TEST

START_TEST(test_elapsed_time)
{
    int64_t tdate = SOPC_Time_GetCurrentTimeUTC();
    int64_t tdate2;
    uint64_t elapsedMs = 0;
    int8_t compareResult = 0;
    SOPC_TimeReference tref = 0;
    SOPC_TimeReference tref2 = 0;

    tref = SOPC_TimeReference_GetCurrent();
    /* Test SOPC_TimeReference_Compare */
    // tref == tref
    compareResult = SOPC_TimeReference_Compare(tref, tref);
    ck_assert(compareResult == 0);

    // (tref2 > tref)
    SOPC_Sleep(20); // Sleep 20 ms to ensure time is different (windows implementation less precise)
    tref2 = SOPC_TimeReference_GetCurrent();

    compareResult = SOPC_TimeReference_Compare(tref2, tref);
    ck_assert(compareResult == 1);

    compareResult = SOPC_TimeReference_Compare(tref, tref2);
    ck_assert(compareResult == -1);

    /* Test SOPC_TimeReference_AddMilliseconds */

    // Nominal case (compute elpase time)
    tref = SOPC_TimeReference_GetCurrent();
    tref2 = SOPC_TimeReference_AddMilliseconds(tref, 1000);

    // Wait 1000 ms elapsed (target time reference <= current time reference)
    while (SOPC_TimeReference_Compare(tref2, tref) > 0)
    {
        SOPC_Sleep(50);
        tref = SOPC_TimeReference_GetCurrent();
    }
    tdate2 = SOPC_Time_GetCurrentTimeUTC();

    ck_assert(tdate >= 0 && tdate2 >= 0 && tdate2 >= tdate);
    elapsedMs = ((uint64_t) tdate2 - (uint64_t) tdate) / 10000; // 100 nanoseconds to milliseconds

    // Check computed elapsed time value (on non monotonic clock) is 1000ms +/- 100ms
    ck_assert(1000 - 100 < elapsedMs && elapsedMs < 1000 + 100);
}
END_TEST

Suite* tests_make_suite_time(void)
{
    Suite* s;
    TCase *tc_time, *tc_elapsed_time;

    s = suite_create("Time tests");
    tc_time = tcase_create("Time");
    tcase_add_test(tc_time, test_current_time);
    suite_add_tcase(s, tc_time);
    tc_elapsed_time = tcase_create("Elapsed Time");
    tcase_add_test(tc_elapsed_time, test_elapsed_time);
    suite_add_tcase(s, tc_elapsed_time);

    return s;
}
