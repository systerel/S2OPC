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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "p_sopc_common_time.h"
#include "sopc_assert.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

/* Check mandatory configuration */
#if !configUSE_TICK_HOOK
#error "FreeRTOS parameter 'configUSE_TICK_HOOK' must be equal to 1"
#endif

/* Number of ticks since FreeRTOS' EPOCH, which is 01/01/1970 00:00:00 UTC.
 * There are configTICK_RATE_HZ per second.
 */
static uint64_t gGlobalTimeReference = 0;

// Hook added in order to use 64 bit tick counter. Used by FreeRTOS
// Called from ISR
void vApplicationTickHook(void)
{
    gGlobalTimeReference += 1;

    return;
}

/*
 * Since ticks start from 0 and ticks are supposed to represent time from Epoch.
 * Apply an offset to aproximate real date. This offset is equal to the number of ticks spent since Epoch to the build
 * date.
 */
void setInitialDateToBuildTime(void)
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
        gGlobalTimeReference += ((uint64_t) now) * ((uint64_t) configTICK_RATE_HZ);
    }
}

uint64_t P_SOPC_COMMON_TIME_get_tick(void)
{
    static bool isTimeInitialized = false;

    if (!isTimeInitialized)
    {
        setInitialDateToBuildTime();
        isTimeInitialized = true;
    }
    uint64_t xTicks;
    /* Critical section required if running on a 32 or 16 bit processor. */
    portENTER_CRITICAL();
    {
        xTicks = gGlobalTimeReference;
    }
    portEXIT_CRITICAL();
    return xTicks;
}

void P_SOPC_COMMON_TIME_SetDateOffset(int64_t nbSecOffset)
{
    gGlobalTimeReference += ((int64_t) nbSecOffset) * configTICK_RATE_HZ;
    return;
}