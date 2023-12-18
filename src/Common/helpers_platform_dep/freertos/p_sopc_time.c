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

#include "p_sopc_time.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* s2opc includes */
#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_enums.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"

/* freertos includes */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

/* Check mandatory configuration */
#if !configUSE_TICK_HOOK
#error "FreeRTOS parameter 'configUSE_TICK_HOOK' must be equal to 1"
#endif

/* Private time api */

#define SECOND_TO_100NS ((uint64_t) 10000000)
#define MS_TO_US 1000

/* Number of ticks since FreeRTOS' EPOCH, which is 01/01/1970 00:00:00 UTC.
 * There are configTICK_RATE_HZ per second.
 */
static uint64_t gGlobalTimeReference = 0;

// Called if reference tick = 0
void P_TIME_SetInitialDateToBuildTime(void)
{
    // Get today date numerics values
    struct tm today = {};
    static char buffer[12] = {0};

    // Initial date set to build value, always "MMM DD YYYY",
    // DD is left padded with a space if it is less than 10.
    sprintf(buffer, "%s", __DATE__);
    buffer[3] = '\0';
    buffer[6] = '\0';
    char* ptrMonth = buffer;
    char* ptrDay = &buffer[4];
    char* ptrYear = &buffer[7];

    if (strcmp(ptrMonth, "Jan") == 0)
    {
        today.tm_mon = 0; /* Month starts from 0 in C99 */
    }
    else if (strcmp(ptrMonth, "Feb") == 0)
    {
        today.tm_mon = 1;
    }
    else if (strcmp(ptrMonth, "Mar") == 0)
    {
        today.tm_mon = 2;
    }
    else if (strcmp(ptrMonth, "Apr") == 0)
    {
        today.tm_mon = 3;
    }
    else if (strcmp(ptrMonth, "May") == 0)
    {
        today.tm_mon = 4;
    }
    else if (strcmp(ptrMonth, "Jun") == 0)
    {
        today.tm_mon = 5;
    }
    else if (strcmp(ptrMonth, "Jul") == 0)
    {
        today.tm_mon = 6;
    }
    else if (strcmp(ptrMonth, "Aug") == 0)
    {
        today.tm_mon = 7;
    }
    else if (strcmp(ptrMonth, "Sep") == 0)
    {
        today.tm_mon = 8;
    }
    else if (strcmp(ptrMonth, "Oct") == 0)
    {
        today.tm_mon = 9;
    }
    else if (strcmp(ptrMonth, "Nov") == 0)
    {
        today.tm_mon = 10;
    }
    else if (strcmp(ptrMonth, "Dec") == 0)
    {
        today.tm_mon = 11;
    }
    else
    {
        SOPC_ASSERT(false); /* Could not parse compilation date */
    }

    today.tm_year = atoi(ptrYear) - 1900; /* C99 specifies that tm_year begins in 1900 */
    today.tm_mday = atoi(ptrDay);

    // Initial time set to build value, always "HH:MM:SS",
    sprintf(buffer, "%s", __TIME__);
    char* ptrH = strtok(buffer, ":");
    char* ptrM = strtok(NULL, ":");
    char* ptrS = strtok(NULL, ":");

    today.tm_hour = (atoi(ptrH));
    today.tm_min = (atoi(ptrM));
    today.tm_sec = (atoi(ptrS));

    // Newlib uses the same time_t precision and reference as Linux.
    // Compute nb seconds since Unix EPOCH.
    // Warn, mktime use libc malloc
    time_t now = mktime(&today);

    // Set nb ticks
    if (0 <= now)
    {
        gGlobalTimeReference = ((uint64_t) now) * ((uint64_t) configTICK_RATE_HZ);
    }
}

void P_TIME_SetDateOffset(int nbSecOffset)
{
    gGlobalTimeReference += ((int64_t) nbSecOffset) * configTICK_RATE_HZ;
    return;
}
// Hook added in order to use 64 bit tick counter. Used by FreeRTOS
// Called from ISR
void vApplicationTickHook(void)
{
    gGlobalTimeReference += 1;

    return;
}

// Used by SOPC_Time_GetCurrentTimeUTC and SOPC_TimeReference_GetCurrent
static uint64_t P_TIME_Get64BitTickCount(void)
{
    uint64_t xTicks;
    /* Critical section required if running on a 32 or 16 bit processor. */
    portENTER_CRITICAL();
    {
        xTicks = gGlobalTimeReference;
    }
    portEXIT_CRITICAL();
    return xTicks;
}

/* Public s2opc api */

int64_t SOPC_Time_GetCurrentTimeUTC()
{
    uint64_t wTimeInTicks = P_TIME_Get64BitTickCount();
    int64_t datetime = 0;
    uint64_t currentTimeInS = 0;
    uint64_t currentTimeFrac100NS = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Convert to seconds and 100ns fractional part.
    // Warning on type used, intermediate operation on uint64_t.
    currentTimeInS = wTimeInTicks / (uint64_t) configTICK_RATE_HZ;

    currentTimeFrac100NS = ((wTimeInTicks * SECOND_TO_100NS) / (uint64_t) configTICK_RATE_HZ) % SECOND_TO_100NS;

    // Check for overflow. Note that currentTimeFrac100NS cannot overflow.
    // Problem: we don't know the limits of time_t, and they are not #defined.
    time_t currentTimeT = 0;
    int64_t limit = 0;
    switch (sizeof(time_t))
    {
    case 4:
        /* Assumes an int32_t */
        limit = INT32_MAX;
        break;
    case 8:
        /* Assumes an int64_t */
        limit = INT64_MAX;
        break;
    default:
        return INT64_MAX;
    }

    if (currentTimeInS > limit)
    {
        return INT64_MAX;
    }

    currentTimeT = (time_t) currentTimeInS;
    status = SOPC_Time_FromTimeT(currentTimeT, &datetime);
    if (SOPC_STATUS_OK != status)
    {
        // Time overflow...
        return INT64_MAX;
    }

    datetime += currentTimeFrac100NS;
    return datetime;
}

// Return current ms since last power on
SOPC_TimeReference SOPC_TimeReference_GetCurrent()
{
    uint64_t currentTimeInMs = 0;
    uint64_t wTimeInTicks = 0;

    wTimeInTicks = P_TIME_Get64BitTickCount();
    currentTimeInMs = (wTimeInTicks * (uint64_t) 1000) / (uint64_t) configTICK_RATE_HZ;

    return currentTimeInMs;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_Local(time_t t, struct tm* tm)
{
    return (localtime_r(&t, tm) == NULL) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_UTC(time_t t, struct tm* tm)
{
    return (gmtime_r(&t, tm) == NULL) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

bool SOPC_RealTime_GetTime(SOPC_RealTime* t)
{
    SOPC_ASSERT(NULL != t);
    t->ticksMs = (uint64_t) SOPC_TimeReference_GetCurrent();
    return true;
}

void SOPC_RealTime_AddSynchedDuration(SOPC_RealTime* t, uint64_t duration_us, int32_t offset_us)
{
    SOPC_UNUSED_ARG(offset_us);
    SOPC_ASSERT(NULL != t);

    t->ticksMs += (uint64_t)(duration_us / (uint64_t) MS_TO_US);
}

int64_t SOPC_RealTime_DeltaUs(const SOPC_RealTime* tRef, const SOPC_RealTime* t)
{
    SOPC_ASSERT(NULL != tRef);
    SOPC_RealTime t1;
    if (NULL == t)
    {
        t1.ticksMs = (uint64_t) SOPC_TimeReference_GetCurrent();
    }
    else
    {
        t1 = *t;
    }

    return (((int64_t) t1.ticksMs) - ((int64_t) tRef->ticksMs)) * MS_TO_US;
}

bool SOPC_RealTime_IsExpired(const SOPC_RealTime* t, const SOPC_RealTime* now)
{
    SOPC_ASSERT(NULL != t);
    SOPC_RealTime t1;

    if (NULL == now)
    {
        t1.ticksMs = (uint64_t) SOPC_TimeReference_GetCurrent();
    }
    else
    {
        t1 = *now;
    }

    /* t <= t1*/
    return t->ticksMs <= t1.ticksMs;
}

bool SOPC_RealTime_SleepUntil(const SOPC_RealTime* date)
{
    SOPC_ASSERT(NULL != date);
    SOPC_RealTime now;

    now.ticksMs = (uint64_t) SOPC_TimeReference_GetCurrent();

    if (now.ticksMs >= date->ticksMs)
    {
        taskYIELD();
    }
    else
    {
        TickType_t timeToWait = (TickType_t)((date->ticksMs - now.ticksMs) * configTICK_RATE_HZ / 1000);

        /** vTaskDelayUntil works with FreeRTOS absolute Ticks. In order to use this API the absolute
         * time must be calculate other FreeRTOS tick.
         */
        TickType_t nowInternalTick = xTaskGetTickCount();

        vTaskDelayUntil(&nowInternalTick, timeToWait);
    }
    return true;
}

/* Return the POSIX time (number of seconds since since 00:00, Jan 1 1970 UTC) */
time_t sopc_time_alt(time_t* timer)
{
    return (time_t)(gGlobalTimeReference / configTICK_RATE_HZ);
}

/* TODO: assert that mktime works correctly on the targeted platform
 * struct tm tmc = {.tm_year = 119,
 *                  .tm_mon = 5, // in 0-11
 *                  .tm_mday = 11,
 *                  .tm_hour = 15,
 *                  .tm_min = 50,
 *                  .tm_sec = 42,
 *                  .tm_isdst = 0};
 * time_t nsec = mktime(&tmc);
 * SOPC_ASSERT(1560268242 == nsec);
 */
