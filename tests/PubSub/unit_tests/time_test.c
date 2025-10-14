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
#include <stdlib.h>

#include "sopc_date_time.h"
#include "sopc_enums.h"
#include "sopc_threads.h"
#include "sopc_time_reference.h"

START_TEST(test_time)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_DateTime currentTimeUTC_100ns = SOPC_Time_GetCurrentTimeUTC();
    ck_assert_int_gt(currentTimeUTC_100ns, 0);

    SOPC_TimeReference currentTimeMs = SOPC_TimeReference_GetCurrent();
    ck_assert_uint_gt(currentTimeMs, 0);

    time_t currentTimeUTCSec = (time_t)(currentTimeUTC_100ns / (1000 * 1000 * 10));
    time_t currentTimeSec = (time_t)(currentTimeMs / 1000);
    struct tm timeStructureUTC;
    struct tm timeStructure;
    status = SOPC_Time_Breakdown_Local(currentTimeSec, &timeStructure);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_Time_Breakdown_UTC(currentTimeUTCSec, &timeStructureUTC);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    unsigned int millisecondToSleep = 1000;
    currentTimeMs = SOPC_TimeReference_GetCurrent();
    SOPC_Sleep(millisecondToSleep);
    SOPC_TimeReference currentTimeMsAfterSleep = SOPC_TimeReference_GetCurrent();
    ck_assert_uint_le(currentTimeMs + millisecondToSleep, currentTimeMsAfterSleep);

    /* RealTime interface */
    SOPC_HighRes_TimeReference* pRealTime = SOPC_HighRes_TimeReference_Create();
    ck_assert_ptr_nonnull(pRealTime);

    SOPC_HighRes_TimeReference_Delete(&pRealTime);
    ck_assert_ptr_null(pRealTime);

    pRealTime = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_GetTime(pRealTime);

    uint64_t duration_us = 1000;
    SOPC_HighRes_TimeReference* pRealTimeCopy = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_Copy(pRealTimeCopy, pRealTime);
    SOPC_HighRes_TimeReference_AddSynchedDuration(pRealTime, duration_us, -1);
    int64_t deltaUs = SOPC_HighRes_TimeReference_DeltaUs(pRealTimeCopy, pRealTime);
    // If deltaUs < 0, the assertion will also fail.
    ck_assert_uint_eq((uint64_t) deltaUs - duration_us, 0);

    SOPC_HighRes_TimeReference_GetTime(pRealTime);
    duration_us = 100000;
    SOPC_HighRes_TimeReference_AddSynchedDuration(pRealTime, duration_us, 0);
    bool res = SOPC_HighRes_TimeReference_IsExpired(pRealTime, NULL);
    ck_assert(!res);

    SOPC_HighRes_TimeReference_GetTime(pRealTime);
    duration_us = 1000 * 1000;
    SOPC_HighRes_TimeReference_AddSynchedDuration(pRealTime, duration_us, 0);
    SOPC_HighRes_TimeReference_SleepUntil(pRealTime);
    SOPC_HighRes_TimeReference_GetTime(pRealTimeCopy);
    ck_assert_int_le(SOPC_HighRes_TimeReference_DeltaUs(pRealTimeCopy, pRealTime), 0);
}

int main(void)
{
    int number_failed;
    SRunner* sr;

    Suite* suite = suite_create("Time tests");

    TCase* tc_time = tcase_create("Time reference test");
    suite_add_tcase(suite, tc_time);
    tcase_add_test(tc_time, test_time);

    sr = srunner_create(suite);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
