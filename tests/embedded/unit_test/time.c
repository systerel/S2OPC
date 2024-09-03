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

#include "sopc_assert.h"
#include "sopc_date_time.h"
#include "sopc_enums.h"
#include "sopc_threads.h"
#include "sopc_time_reference.h"
#include "unit_test_include.h"

void suite_test_time(int* index)
{
    PRINT("\n TEST %d: time \n", *index);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_DateTime currentTimeUTC_100ns = SOPC_Time_GetCurrentTimeUTC();
    SOPC_ASSERT(currentTimeUTC_100ns > 0);
    PRINT("Test 1 : ok\n");

    SOPC_TimeReference currentTimeMs = SOPC_TimeReference_GetCurrent();
    SOPC_ASSERT(currentTimeMs > 0);
    PRINT("Test 2: ok\n");

    time_t currentTimeUTCSec = (time_t)(currentTimeUTC_100ns / (1000 * 1000 * 10));
    time_t currentTimeSec = (time_t)(currentTimeMs / 1000);
    struct tm timeStructureUTC;
    struct tm timeStructure;
    status = SOPC_Time_Breakdown_Local(currentTimeSec, &timeStructure);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    PRINT("Test 3: ok\n");

    status = SOPC_Time_Breakdown_UTC(currentTimeUTCSec, &timeStructureUTC);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    PRINT("Test 4: ok\n");

    unsigned int millisecondToSleep = 1000;
    currentTimeMs = SOPC_TimeReference_GetCurrent();
    SOPC_Sleep(millisecondToSleep);
    SOPC_TimeReference currentTimeMsAfterSleep = SOPC_TimeReference_GetCurrent();
    SOPC_ASSERT(currentTimeMs + millisecondToSleep <= currentTimeMsAfterSleep);
    PRINT("Test 5: ok\n");

    /* RealTime interface */
    SOPC_HighRes_TimeReference* pRealTime = SOPC_HighRes_TimeReference_Create();
    SOPC_ASSERT(NULL != pRealTime);
    PRINT("Test 6: ok\n");

    SOPC_HighRes_TimeReference_Delete(&pRealTime);
    SOPC_ASSERT(pRealTime == NULL);
    PRINT("Test 7: ok\n");

    pRealTime = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_GetTime(pRealTime);
    PRINT("Test 8: ok\n");

    uint64_t duration_us = 1000;
    SOPC_HighRes_TimeReference* pRealTimeCopy = SOPC_HighRes_TimeReference_Create();
    SOPC_HighRes_TimeReference_Copy(pRealTimeCopy, pRealTime);
    SOPC_HighRes_TimeReference_AddSynchedDuration(pRealTime, duration_us, 0);
    SOPC_ASSERT(SOPC_HighRes_TimeReference_DeltaUs(pRealTimeCopy, pRealTime) - (duration_us) == 0);
    PRINT("Test 9: ok\n");

    SOPC_HighRes_TimeReference_GetTime(pRealTime);
    duration_us = 100000;
    SOPC_HighRes_TimeReference_AddSynchedDuration(pRealTime, duration_us, 0);
    bool res = SOPC_HighRes_TimeReference_IsExpired(pRealTime, NULL);
    SOPC_ASSERT(!res);
    PRINT("Test 10: ok\n");

    SOPC_HighRes_TimeReference_GetTime(pRealTime);
    duration_us = 1000 * 1000;
    SOPC_HighRes_TimeReference_AddSynchedDuration(pRealTime, duration_us, 0);
    SOPC_HighRes_TimeReference_SleepUntil(pRealTime);
    SOPC_HighRes_TimeReference_GetTime(pRealTimeCopy);
    SOPC_ASSERT(SOPC_HighRes_TimeReference_DeltaUs(pRealTimeCopy, pRealTime) <= 0);
    PRINT("Test 11: ok\n");

    *index += 1;
}
