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

#include "sopc_assert.h"
#include "sopc_date_time.h"
#include "sopc_mutexes.h"

#define SOPC_SECONDS_TO_100NANOSECONDS 10000000
#define SOPC_MILLISECONDS_TO_NANOSECONDS 1000000
#define SOPC_MICROSECONDS_TO_NANOSECONDS 1000
#define SOPC_SECONDS_TO_MILLISECONDS 1000

typedef struct HANDLE_TIME_REFERENCE
{
    time_t buildTime_s; /* Store build time second resolution */
    bool isInit;
    SOPC_Mutex criticalSection;
} handleTimeReference;

/* Handle compilation time as reference for UTC time */
static handleTimeReference gTimeReference = {.buildTime_s = 0, .isInit = false, .criticalSection = SOPC_INVALID_MUTEX};

static void P_TIME_SetInitialDateToBuildTime(void);

static void P_TIME_SetInitialDateToBuildTime(void)
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

    currentTick_100ns += (buildTime_s * SOPC_SECONDS_TO_100NANOSECONDS);

    currentTimeFrac100Ns = currentTick_100ns % SOPC_SECONDS_TO_100NANOSECONDS;
    currentTimeInS = currentTick_100ns / SOPC_SECONDS_TO_100NANOSECONDS;

    time_t currentTimeT = (time_t) currentTimeInS;
    status = SOPC_Time_FromUnixTime(currentTimeT, &datetime);
    if (SOPC_STATUS_OK != status)
    {
        return INT64_MAX;
    }
    datetime += currentTimeFrac100Ns;
    return datetime;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_Local(SOPC_Unix_Time t, struct tm* tm)
{
    return (NULL == localtime_r(&t, tm)) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_UTC(SOPC_Unix_Time t, struct tm* tm)
{
    return SOPC_Time_Breakdown_Local(t, tm);
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
