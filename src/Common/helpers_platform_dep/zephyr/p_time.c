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
#include <math.h>
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

#include "p_time.h"
#include "sopc_assert.h"
#include "sopc_enums.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"

/* Private time api */

#define P_TIME_DEBUG (0)

#define SECOND_TO_100NS (10000000)
#define MS_TO_100NS (10000)
#define US_TO_100NS (10)

typedef enum P_TIME_STATUS
{
    P_TIME_STATUS_NOT_INITIALIZED,
    P_TIME_STATUS_INITIALIZING,
    P_TIME_STATUS_INITIALIZED
} ePTimeStatus;

static ePTimeStatus gTimeStatus = P_TIME_STATUS_NOT_INITIALIZED;
static uint64_t gUptimeDate_s = 0;

static uint64_t P_TIME_GetBuildDateTime(void)
{
    // Get today date numerics values
    struct tm buildDate = {};
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
        buildDate.tm_mon = 0; /* Month starts from 0 in C99 */
    }
    else if (strcmp(ptrMonth, "Feb") == 0)
    {
        buildDate.tm_mon = 1;
    }
    else if (strcmp(ptrMonth, "Mar") == 0)
    {
        buildDate.tm_mon = 2;
    }
    else if (strcmp(ptrMonth, "Apr") == 0)
    {
        buildDate.tm_mon = 3;
    }
    else if (strcmp(ptrMonth, "May") == 0)
    {
        buildDate.tm_mon = 4;
    }
    else if (strcmp(ptrMonth, "Jun") == 0)
    {
        buildDate.tm_mon = 5;
    }
    else if (strcmp(ptrMonth, "Jul") == 0)
    {
        buildDate.tm_mon = 6;
    }
    else if (strcmp(ptrMonth, "Aug") == 0)
    {
        buildDate.tm_mon = 7;
    }
    else if (strcmp(ptrMonth, "Sep") == 0)
    {
        buildDate.tm_mon = 8;
    }
    else if (strcmp(ptrMonth, "Oct") == 0)
    {
        buildDate.tm_mon = 9;
    }
    else if (strcmp(ptrMonth, "Nov") == 0)
    {
        buildDate.tm_mon = 10;
    }
    else if (strcmp(ptrMonth, "Dec") == 0)
    {
        buildDate.tm_mon = 11;
    }
    else
    {
        assert(false); /* Could not parse compilation date */
    }

    buildDate.tm_year = atoi(ptrYear) - 1900; /* C99 specifies that tm_year begins in 1900 */
    buildDate.tm_mday = atoi(ptrDay);

    // Initial time set to build value, always "HH:MM:SS",
    sprintf(buffer, "%s", __TIME__);
    char* ptrH = strtok(buffer, ":");
    char* ptrM = strtok(NULL, ":");
    char* ptrS = strtok(NULL, ":");

    buildDate.tm_hour = (atoi(ptrH));
    buildDate.tm_min = (atoi(ptrM));
    buildDate.tm_sec = (atoi(ptrS));

    return mktime(&buildDate);
}

static uint64_t P_TIME_TimeReference_GetCurrent100ns(void)
{
    ePTimeStatus expectedStatus = P_TIME_STATUS_NOT_INITIALIZED;
    ePTimeStatus desiredStatus = P_TIME_STATUS_INITIALIZING;

    // Note: avoid u64 overflow by reducing factors
    static const uint64_t tick_reduce_factor = 10e5;
    static const uint64_t tick_to_100ns_d = (SECOND_TO_100NS / tick_reduce_factor);
    static uint64_t tick_to_100ns_n = 0;
    static struct k_mutex monotonicMutex;

    bool bTransition = __atomic_compare_exchange(&gTimeStatus, &expectedStatus, &desiredStatus, false, __ATOMIC_SEQ_CST,
                                                 __ATOMIC_SEQ_CST);

    if (bTransition)
    {
        tick_to_100ns_n = (sys_clock_hw_cycles_per_sec() / tick_reduce_factor);
        // Check that rounding assumptions for tick_to_100ns_d and tick_to_100ns_n are correct
        SOPC_ASSERT((SECOND_TO_100NS % tick_reduce_factor) == 0);
        SOPC_ASSERT((sys_clock_hw_cycles_per_sec() % tick_reduce_factor) == 0);
        SOPC_ASSERT(0 < tick_to_100ns_n);

        gUptimeDate_s = P_TIME_GetBuildDateTime();

        k_mutex_init(&monotonicMutex);

        desiredStatus = P_TIME_STATUS_INITIALIZED;
        __atomic_store(&gTimeStatus, &desiredStatus, __ATOMIC_SEQ_CST);
    }

    __atomic_load(&gTimeStatus, &expectedStatus, __ATOMIC_SEQ_CST);

    while (expectedStatus != P_TIME_STATUS_INITIALIZED)
    {
        k_yield();
        __atomic_load(&gTimeStatus, &expectedStatus, __ATOMIC_SEQ_CST);
    }

    static uint64_t overflow_offset = 0;
    static uint64_t last_kernel_tick = 0;

    k_mutex_lock(&monotonicMutex, K_FOREVER);
    // Get associated hardware clock counter
    uint64_t kernel_clock_ticks = k_cycle_get_32();
    if (kernel_clock_ticks < last_kernel_tick)
    {
        // We assume that this function is called with sufficient frequency so
        //  that there cannot be 2 overflows between 2 successive calls. This could be improved
        // by comparing the "P_TIME_CurrentTimeMs" evolution
        overflow_offset += (1llu << 32);
    }
    // kernel_clock_ticks now contains the overflow-corrected tick count

    const uint64_t kernel_clock_100ns = ((kernel_clock_ticks + overflow_offset) * tick_to_100ns_d) / tick_to_100ns_n;

    const uint64_t value_100ns = gUptimeDate_s * SECOND_TO_100NS + kernel_clock_100ns;

#if P_TIME_DEBUG == 1
    static uint64_t last_date = 0;
    if (last_date > value_100ns)
    {
        printk(
            "Last_date = %llu > new = %llu\n"
            "last_kernel_tick=%llu, kernel_clock_ticks=%llu, kernel_clock_100ns=%llu\n",
            last_date, value_100ns, last_kernel_tick, kernel_clock_ticks, kernel_clock_100ns);
        printk("\r\n kernel_clock_ticks = %llu", kernel_clock_ticks);
        printk("\r\n last_kernel_tick = %llu", last_kernel_tick);
        printk("\r\n overflow_offset = %llu", overflow_offset);
        printk("\r\n kernel_clock_100ns = %llu", kernel_clock_100ns);
        printk("\r\n value_100ns = %llu", value_100ns);
        SOPC_ASSERT(false);
    }
    last_date = value_100ns;
#endif
    last_kernel_tick = kernel_clock_ticks;

    k_mutex_unlock(&monotonicMutex);

    return value_100ns;
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
    uint64_t value = P_TIME_TimeReference_GetCurrent100ns();

    return (SOPC_TimeReference)(value / MS_TO_100NS);
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
    if (milliseconds > 0)
    {
        k_sleep(K_MSEC(milliseconds));
    }
    else
    {
        k_yield();
    }
    return;
}

SOPC_RealTime* SOPC_RealTime_Create(const SOPC_RealTime* copy)
{
    SOPC_RealTime* ret = SOPC_Calloc(1, sizeof(SOPC_RealTime));
    if (NULL != copy && NULL != ret)
    {
        *ret = *copy;
    }
    else if (NULL != ret)
    {
        bool ok = SOPC_RealTime_GetTime(ret);
        if (!ok)
        {
            SOPC_RealTime_Delete(&ret);
        }
    }

    return ret;
}
void SOPC_RealTime_Delete(SOPC_RealTime** t)
{
    if (NULL == t)
    {
        return;
    }
    SOPC_Free(*t);
    *t = NULL;
}

bool SOPC_RealTime_GetTime(SOPC_RealTime* t)
{
    if (NULL == t)
    {
        return false;
    }

    *t = P_TIME_TimeReference_GetCurrent100ns();
    return true;
}

void SOPC_RealTime_AddDuration(SOPC_RealTime* t, double duration_ms)
{
    assert(NULL != t);

    *t += duration_ms * MS_TO_100NS;
}

bool SOPC_RealTime_IsExpired(const SOPC_RealTime* t, const SOPC_RealTime* now)
{
    SOPC_ASSERT(NULL != t);
    SOPC_RealTime t1 = 0;
    bool ok = true;

    if (NULL == now)
    {
        ok = SOPC_RealTime_GetTime(&t1);
    }
    else
    {
        t1 = *now;
    }

    /* t <= t1 */
    return ok && ((*t) <= t1);
}

bool SOPC_RealTime_SleepUntil(const SOPC_RealTime* date)
{
#if (CONFIG_SYS_CLOCK_TICKS_PER_SEC < 1000)
#warning CONFIG_SYS_CLOCK_TICKS_PER_SEC is insufficient to handle durations under 1 ms
#endif
    if (NULL == date)
    {
        return false;
    }

    const int64_t now = P_TIME_TimeReference_GetCurrent100ns();
    const int64_t expDate = (int64_t) *date;
    int64_t toWait_us = (expDate - now) / US_TO_100NS;

    if (toWait_us <= 0)
    {
        return false;
    }

    while (toWait_us > 0)
    {
        toWait_us = k_usleep(toWait_us);
    }
    return false;
}
