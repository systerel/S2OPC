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

#include <p4.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p_sopc_time.h"
#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_platform_time.h"
#include "sopc_time.h"

#define SECOND_TO_100NS 10000000
#define MS_TO_NS 1000000
#define US_TO_NS 1000
#define SECOND_TO_MS 1000

typedef struct HANDLE_TIME_REFERENCE
{
    time_t buildTime_s; /* Store build time second resolution */
    bool isInit;
    SOPC_Mutex criticalSection;
} handleTimeReference;

/* Handle compilation time as reference for UTC time */
static handleTimeReference gTimeReference = {.buildTime_s = 0, .isInit = false, .criticalSection = {}};

void P_TIME_SetInitialDateToBuildTime(void);

void P_TIME_SetInitialDateToBuildTime(void)
{
    SOPC_Mutex_Initialization(&gTimeReference.criticalSection);
    struct tm today = {};
    static char buffer[12] = {0};

    /*Initial date set to build value, always "MMM DD YYYY",
     DD is left padded with a space if it is less than 10. */
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

    SOPC_Mutex_Lock(&gTimeReference.criticalSection);
    gTimeReference.buildTime_s = mktime(&today);
    gTimeReference.isInit = true;
    SOPC_Mutex_Unlock(&gTimeReference.criticalSection);
}

void SOPC_Sleep(unsigned int milliseconds)
{
    P4_e_t res = p4_sleep((P4_timeout_t) P4_MSEC(milliseconds));
    /* P4_E_CANCEL is sometimes return in debug context. To avoid constant assertions in multithreading context
     * check out this return parameter. But should be add back in production */
    // SOPC_ASSERT(P4_E_OK == res);
    SOPC_ASSERT(P4_E_BADTIMEOUT != res);
}

SOPC_DateTime SOPC_Time_GetCurrentTimeUTC(void)
{
    int64_t datetime = 0;
    uint64_t currentTimeInS = 0;
    uint64_t currentTimeFrac100Ns = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    time_t currentTick_100ns = (time_t)(p4_get_time() / 100); /* SOPC_DateTime has for resolution 100ns */

    if (!gTimeReference.isInit)
    {
        P_TIME_SetInitialDateToBuildTime();
    }

    SOPC_Mutex_Lock(&gTimeReference.criticalSection);
    time_t buildTime_s = gTimeReference.buildTime_s;
    SOPC_Mutex_Unlock(&gTimeReference.criticalSection);

    currentTick_100ns += (buildTime_s * SECOND_TO_100NS);

    currentTimeFrac100Ns = currentTick_100ns % SECOND_TO_100NS;
    currentTimeInS = currentTick_100ns / SECOND_TO_100NS;

    time_t currentTimeT = (time_t) currentTimeInS;
    status = SOPC_Time_FromTimeT(currentTimeT, &datetime);
    if (SOPC_STATUS_OK != status)
    {
        return INT64_MAX;
    }
    datetime += currentTimeFrac100Ns;
    return datetime;
}

SOPC_TimeReference SOPC_TimeReference_GetCurrent(void)
{
    uint64_t timeReference = 0;
    uint64_t currentTimeInS = 0;
    uint64_t currentTimeFracMs = 0;
    time_t currentTick_ms = (time_t)(p4_get_time() / MS_TO_NS); /* timeReference has for resolution millisecond */

    if (!gTimeReference.isInit)
    {
        P_TIME_SetInitialDateToBuildTime();
    }

    SOPC_Mutex_Lock(&gTimeReference.criticalSection);
    time_t buildTime_s = gTimeReference.buildTime_s;
    SOPC_Mutex_Unlock(&gTimeReference.criticalSection);

    currentTick_ms += (buildTime_s * SECOND_TO_MS);

    currentTimeFracMs = currentTick_ms % SECOND_TO_MS;
    currentTimeInS = currentTick_ms / SECOND_TO_MS;

    timeReference = (currentTimeInS * SECOND_TO_MS) + currentTimeFracMs;
    return (SOPC_TimeReference) timeReference;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_Local(time_t t, struct tm* tm)
{
    return (NULL == localtime_r(&t, tm)) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_UTC(time_t t, struct tm* tm)
{
    return SOPC_Time_Breakdown_Local(t, tm);
}

bool SOPC_RealTime_GetTime(SOPC_RealTime* t)
{
    if (t == NULL)
    {
        return false;
    }
    *t = p4_get_time();
    return true;
}

void SOPC_RealTime_AddSynchedDuration(SOPC_RealTime* t, uint64_t duration_us, int32_t offset_us)
{
    SOPC_UNUSED_ARG(offset_us);
    SOPC_ASSERT(t != NULL);
    *t += (P4_time_t)(duration_us * US_TO_NS);
}

bool SOPC_RealTime_IsExpired(const SOPC_RealTime* t, const SOPC_RealTime* now)
{
    SOPC_ASSERT(t != NULL);
    if (now == NULL)
    {
        P4_time_t currentTime = p4_get_time();
        return *t < currentTime;
    }
    return *t < *now;
}

int64_t SOPC_RealTime_DeltaUs(const SOPC_RealTime* tRef, const SOPC_RealTime* t)
{
    P4_time_t t1 = 0;

    if (NULL == t)
    {
        const bool ok = SOPC_RealTime_GetTime(&t1);
        SOPC_ASSERT(ok);
    }
    else
    {
        t1 = *t;
    }

    return (int64_t) t1 - *tRef;
}


bool SOPC_RealTime_SleepUntil(const SOPC_RealTime* date)
{
    P4_time_t currentTime = p4_get_time();
    int64_t timeToWait = (*date - currentTime);
    if (timeToWait < 0)
    {
        p4_thread_yield();
    }
    else
    {
        P4_e_t res = p4_sleep((P4_timeout_t) timeToWait);
        return (P4_E_OK == res);
    }
    return true;
}

size_t strftime(char* strBuffer, size_t maxSize, const char* format, const struct tm* pTime)
{
    char newFormat[100] = {0};
    char year[3] = "%d";
    char month[4] = "%d";
    char day[4] = "%d";
    char hour[4] = "%d";
    char minute[4] = "%d";
    char second[4] = "%d";
    if (pTime->tm_mon < 10)
    {
        snprintf(month, 4, "%s", "0%d");
    }
    if (pTime->tm_mday < 10)
    {
        snprintf(day, 4, "%s", "0%d");
    }
    if (pTime->tm_hour < 10)
    {
        snprintf(hour, 4, "%s", "0%d");
    }
    if (pTime->tm_min < 10)
    {
        snprintf(minute, 4, "%s", "0%d");
    }
    if (pTime->tm_sec < 10)
    {
        snprintf(second, 4, "%s", "0%d");
    }

    if (!strcmp(format, "%Y/%m/%d %H:%M:%S"))
    {
        snprintf(newFormat, 23, "%s/%s/%s %s:%s:%s", year, month, day, hour, minute, second);
    }
    else if (!strcmp(format, "%Y%m%d_%H%M%S"))
    {
        snprintf(newFormat, 23, "%s%s%s_%s%s%s", year, month, day, hour, minute, second);
    }
    else
    {
        return -1;
    }
    return snprintf(strBuffer, maxSize, newFormat, pTime->tm_year + 1900, pTime->tm_mon + 1, pTime->tm_mday,
                    pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
}
