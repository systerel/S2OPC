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

#include <vm.h>

#include "sopc_assert.h"
#include "sopc_enums.h"
#include "sopc_platform_time.h"
#include "unit_test_include.h"

void suite_test_time(int* index)
{
    vm_cprintf("\n TEST %d: time \n", *index);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_DateTime currentTimeUTC_100ns = SOPC_Time_GetCurrentTimeUTC();
    SOPC_ASSERT(currentTimeUTC_100ns > 0);
    vm_cprintf("Test 1 : ok\n");

    SOPC_TimeReference currentTimeMs = SOPC_TimeReference_GetCurrent();
    SOPC_ASSERT(currentTimeMs > 0);
    vm_cprintf("Test 2: ok\n");

    time_t currentTimeUTCSec = (time_t)(currentTimeUTC_100ns / (1000 * 1000 * 10));
    time_t currentTimeSec = (time_t)(currentTimeMs / 1000);
    struct tm timeStructureUTC;
    struct tm timeStructure;
    status = SOPC_Time_Breakdown_Local(currentTimeSec, &timeStructure);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    vm_cprintf("Test 3: ok\n");

    status = SOPC_Time_Breakdown_UTC(currentTimeUTCSec, &timeStructureUTC);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    vm_cprintf("Test 4: ok\n");

    unsigned int millisecondToSleep = 1000;
    currentTimeMs = SOPC_TimeReference_GetCurrent();
    SOPC_Sleep(millisecondToSleep);
    SOPC_TimeReference currentTimeMsAfterSleep = SOPC_TimeReference_GetCurrent();
    SOPC_ASSERT(currentTimeMs + millisecondToSleep <= currentTimeMsAfterSleep);
    vm_cprintf("Test 5: ok\n");

    /* RealTime interface */
    SOPC_RealTime* pRealTime = SOPC_RealTime_Create(NULL);
    SOPC_ASSERT(NULL != pRealTime);
    vm_cprintf("Test 6: ok\n");

    SOPC_RealTime_Delete(&pRealTime);
    SOPC_ASSERT(pRealTime == NULL);
    vm_cprintf("Test 7: ok\n");

    pRealTime = SOPC_RealTime_Create(NULL);
    bool res = SOPC_RealTime_GetTime(pRealTime);
    SOPC_ASSERT(res);
    vm_cprintf("Test 8: ok\n");

    uint64_t duration_us = 1000;
    SOPC_RealTime* pRealTimeCopy = SOPC_RealTime_Create(pRealTime);
    SOPC_RealTime_AddSynchedDuration(pRealTime, duration_us, 0);
    SOPC_ASSERT(*pRealTimeCopy + (duration_us * 1000) == *pRealTime);
    vm_cprintf("Test 9: ok\n");

    SOPC_RealTime_GetTime(pRealTime);
    duration_us = 100000;
    SOPC_RealTime_AddSynchedDuration(pRealTime, duration_us, 0);
    res = SOPC_RealTime_IsExpired(pRealTime, NULL);
    SOPC_ASSERT(!res);
    vm_cprintf("Test 10: ok\n");

    SOPC_RealTime_GetTime(pRealTime);
    duration_us = 1000 * 1000;
    SOPC_RealTime_AddSynchedDuration(pRealTime, duration_us, 0);
    res = SOPC_RealTime_SleepUntil(pRealTime);
    SOPC_ASSERT(res);
    SOPC_RealTime_GetTime(pRealTimeCopy);
    SOPC_ASSERT(*pRealTime <= *pRealTimeCopy);
    vm_cprintf("Test 11: ok\n");

    *index += 1;
}
