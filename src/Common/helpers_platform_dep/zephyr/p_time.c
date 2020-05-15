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

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <inttypes.h>

#include "kernel.h"

#ifndef __INT32_MAX__
#include "toolchain/xcc_missing_defs.h"
#endif

#ifndef NULL
#define NULL ((void*) 0)
#endif
#ifndef K_FOREVER
#define K_FOREVER (-1)
#endif
#ifndef K_NO_WAIT
#define K_NO_WAIT 0
#endif

/* s2opc includes */

#include "sopc_enums.h"
#include "sopc_time.h"

/* Private time api */

#define P_TIME_DEBUG (0)

#define SECOND_TO_100NS ((uint64_t) 10000000)
#define MS_TO_100NS ((uint64_t) 10000)
#ifndef CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC
#define CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC 600000000
#endif

static time_t gBuildDate = 0;

// Temporary workaround while waiting for Zephyr to fix POSIX time.h issue
time_t P_TIME_GetBuildDate(void)
{
    if (0 == gBuildDate)
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
            assert(false); /* Could not parse compilation date */
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
        gBuildDate = mktime(&today);
    }

    return gBuildDate;
}

static inline uint64_t P_TIME_TimeReference_GetCurrent100ns(void)
{
    uint64_t soft_clock_ms = (SOPC_TimeReference)(P_TIME_GetBuildDate() * 1000 + k_uptime_get());
    // Get associated hardware clock counter
    uint64_t kernel_clock_ticks = k_cycle_get_32();

    // compute overflow period for tick counter, kernel millisecond counter point of view

    uint64_t periodOverflowMs = ((uint64_t) UINT32_MAX + 1) * SECOND_TO_100NS / CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;

    // compute number of overflows, kernel millisecond counter point of view
    uint64_t nbOverflows = soft_clock_ms * MS_TO_100NS / periodOverflowMs;

    // compute monotonic high precision value in 100NS
    uint64_t current = ((uint64_t) kernel_clock_ticks * SECOND_TO_100NS) / CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;

    // compute 100NS overflowed cumulated values
    uint64_t overflow = nbOverflows * periodOverflowMs;

    // add to high precision counter
    uint64_t value = current + overflow;

    return value;
}

/* Public s2opc api */

int64_t SOPC_Time_GetCurrentTimeUTC()
{
    int64_t datetime = 0;

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    /*==========================================================*/

    // Get current ms counter

    uint64_t value = P_TIME_TimeReference_GetCurrent100ns();

    // compute value in second, used to compute UTC value
    uint64_t value_in_s = value / SECOND_TO_100NS;
    uint64_t value_frac_in_100ns = value % SECOND_TO_100NS;

#if P_TIME_DEBUG == 1
    printk("\r\n periodOverflowMs = %llu", periodOverflowMs);
    printk("\r\n nbOverflows = %llu", nbOverflows);
    printk("\r\n current = %llu", current);
    printk("\r\n overflow = %llu", overflow);
    printk("\r\n result = %llu", value);
    printk("\r\n %llu", value % SECOND_TO_100NS);
#endif

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
        datetime = INT64_MAX;
        result = SOPC_STATUS_NOK;
        break;
    }

    if (value_in_s > limit)
    {
        result = SOPC_STATUS_NOK;
        datetime = INT64_MAX;
    }

    if (SOPC_STATUS_OK == result)
    {
        currentTimeT = (time_t) value_in_s;
        result = SOPC_Time_FromTimeT(currentTimeT, &datetime);
        if (SOPC_STATUS_OK != result)
        {
            // Time overflow...
            datetime = INT64_MAX;
        }

        // Add to UTC value fractionnal part of value
        datetime += value_frac_in_100ns;
    }

    return datetime;
}

// Return current ms since last power on
SOPC_TimeReference SOPC_TimeReference_GetCurrent()
{
    SOPC_TimeReference currentTimeInMs = 0;

    uint64_t value = P_TIME_TimeReference_GetCurrent100ns();

    currentTimeInMs = (SOPC_TimeReference)(value / MS_TO_100NS);

    return currentTimeInMs;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_Local(time_t t, struct tm* tm)
{
    return (NULL == localtime_r(&t, tm)) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_UTC(time_t t, struct tm* tm)
{
    return (NULL == gmtime_r(&t, tm)) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

void SOPC_Sleep(unsigned int milliseconds)
{

    k_sleep(K_MSEC(milliseconds));
    return;
}
