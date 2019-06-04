/*_
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

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h> /* stdlib includes */
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sopc_builtintypes.h"
#include "sopc_enums.h" /* s2opc includes */
#include "sopc_time.h"

#include "FreeRTOS.h"       /* freeRtos includes */
#include "FreeRTOSConfig.h" /* freeRtos includes */
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

/* Private time api */

static SOPC_TimeReference gGlobalTimeReference = 0; // SOPC ticks
static const char* INITIAL_TIME = __TIME__;         // Initial time set to build value
static const char* INITIAL_DATE = __DATE__;
static char gBuffer[16] = {0};

#define ONE_DAY (86400)
#define ONE_HOUR (3600)
#define ONE_MINUTE (60)

// Called if reference tick = 0
static void P_TIME_vSetInitialDateToBuild(void)
{
    uint64_t wIter = 0;
    uint64_t today_month = 1;
    uint64_t today_year = 1970;
    uint64_t today_day = 1;
    uint64_t today_hour = 0;
    uint64_t today_minutes = 0;
    uint64_t today_seconds = 0;
    uint64_t nb_years = 0;
    uint64_t nb_leap_years = 0;
    uint64_t A_ = 0;
    uint64_t B_ = 0;
    uint64_t C_ = 0;
    uint64_t D_ = 0;
    char* ptrMonth = NULL;
    char* ptrDay = NULL;
    char* ptrYear = NULL;
    char* ptrH = NULL;
    char* ptrM = NULL;
    char* ptrS = NULL;

    // Get today date numerics values

    sprintf(gBuffer, "%s", INITIAL_DATE);
    ptrMonth = strtok(gBuffer, " ");
    ptrDay = strtok(NULL, " ");
    ptrYear = strtok(NULL, " ");

    if (strcmp(ptrMonth, "Jan") == 0)
        today_month = 1;
    if (strcmp(ptrMonth, "Feb") == 0)
        today_month = 2;
    if (strcmp(ptrMonth, "Mar") == 0)
        today_month = 3;
    if (strcmp(ptrMonth, "Apr") == 0)
        today_month = 4;
    if (strcmp(ptrMonth, "May") == 0)
        today_month = 5;
    if (strcmp(ptrMonth, "Jun") == 0)
        today_month = 6;
    if (strcmp(ptrMonth, "Jul") == 0)
        today_month = 7;
    if (strcmp(ptrMonth, "Aug") == 0)
        today_month = 8;
    if (strcmp(ptrMonth, "Sep") == 0)
        today_month = 9;
    if (strcmp(ptrMonth, "Oct") == 0)
        today_month = 10;
    if (strcmp(ptrMonth, "Nov") == 0)
        today_month = 11;
    if (strcmp(ptrMonth, "Dec") == 0)
        today_month = 12;

    today_year = atoi(ptrYear);
    today_day = atoi(ptrDay);

    sprintf(gBuffer, "%s", INITIAL_TIME);
    ptrH = strtok(gBuffer, ":");
    ptrM = strtok(NULL, ":");
    ptrS = strtok(NULL, ":");

    today_hour = (atoi(ptrH));
    today_minutes = (atoi(ptrM));
    today_seconds = (atoi(ptrS));

    // Compute nb seconds for nb years since 1970 except current day.
    // February = 28 j in this first compute.
    nb_years = today_year - 1970;
    A_ = (nb_years) *365 * ONE_DAY;

    // Compute nb leap years
    for (wIter = 1970; wIter < (1970 + nb_years); wIter++)
    {
        if ((((wIter % 4) == 0) && ((wIter % 100) != 0)) || ((wIter % 400) == 0))
        {
            nb_leap_years += 1;
        }
    }

    D_ = nb_leap_years * ONE_DAY;

    // Compute nb s between today and 1 january of this year, except today.
    for (wIter = 1; wIter <= (today_month - 1); wIter++)
    {
        switch (wIter)
        {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            B_ += 31;
            break;

        case 2:
            if ((((today_year % 4) == 0) && ((today_year % 100) != 0)) || ((today_year % 400) == 0))
            {
                B_ += 29;
            }
            else
            {
                B_ += 28;
            }
            break;

        case 4:
        case 6:
        case 9:
        case 11:
            B_ += 30;
            break;
        }
    }

    B_ = (B_ + (today_day - 1)) * ONE_DAY;

    // Compute nb s since 00:00 of today
    C_ = today_hour * ONE_HOUR + today_minutes * ONE_MINUTE + today_seconds;

    // Set nb ticks
    gGlobalTimeReference = ((D_ + B_ + C_ + A_)) * ((uint64_t) configTICK_RATE_HZ);
}

// Hook added in order to use 64 bit tick counter. Used by FreeRTOS
// Called from ISR
void vApplicationTickHook(void)
{
    if (gGlobalTimeReference == 0)
    {
        P_TIME_vSetInitialDateToBuild();
    }
    else
    {
        gGlobalTimeReference += 1;
    }
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

SOPC_DateTime SOPC_Time_GetCurrentTimeUTC()
{
    int64_t dt = 0;
    int64_t currentTimeInS = 0;
    int64_t currentTimeIn100NS = 0;
    uint64_t wTimeInTicks = 0;
    // Get tick count
    wTimeInTicks = P_TIME_Get64BitTickCount();
    // Convert to 100NS and S. Warning on type used, intermediate operation on uint64_t
    currentTimeInS = (int64_t)(((uint64_t) wTimeInTicks * (uint64_t) 1) / (uint64_t) configTICK_RATE_HZ);

    currentTimeIn100NS = (int64_t)((((uint64_t) wTimeInTicks * (uint64_t) 10000000) / (uint64_t) configTICK_RATE_HZ) %
                                   (uint64_t) 10000000);

    // Check for overflow
    if ((SOPC_Time_FromTimeT(currentTimeInS, &dt) != SOPC_STATUS_OK) || (INT64_MAX - currentTimeIn100NS < dt))
    {
        // Time overflow...
        return INT64_MAX;
    }

    dt += currentTimeIn100NS;
    return dt;
}

// Return current ms since last power on
SOPC_TimeReference SOPC_TimeReference_GetCurrent()
{
    uint64_t currentTimeInMs = 0;
    uint64_t wTimeInTicks = 0;

    wTimeInTicks = P_TIME_Get64BitTickCount();
    currentTimeInMs = ((uint64_t) wTimeInTicks * (uint64_t) 1000) / (uint64_t) configTICK_RATE_HZ;

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
