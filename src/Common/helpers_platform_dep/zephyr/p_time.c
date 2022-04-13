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

#if CONFIG_NET_GPTP
#include <net/gptp.h>
#include <net/net_core.h>
#include "ethernet/gptp/gptp_messages.h"
// Include MUST follow this order
#include "ethernet/gptp/gptp_data_set.h"
#endif

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

/***************************************************
 * DECLARATION OF LOCAL MACROS
 **************************************************/

/***************************************************
 * DECLARATION OF LOCAL TYPES/VARIABLES
 **************************************************/

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

/***************************************************
 * DECLARATION OF LOCAL FUNCTIONS
 **************************************************/
static uint64_t P_TIME_GetBuildDateTime(void);
/** /brief Get current internal time with 100 ns precision */
static SOPC_RealTime P_TIME_TimeReference_GetInternal100ns(void);

#if CONFIG_NET_GPTP
/** Offset between PTp & internal clock, (or 0 if unknown)*/
static int64_t gPtpSourceSync = 0;
/** /brief Get current internal time with 100 ns precision, corrected by PTP */
static SOPC_RealTime P_TIME_TimeReference_GetCorrected100ns(void);

/** /brief Get current PTP time with 100 ns precision
 * /return 0 if PtpClock is not ready or provides irrelevant value*/
static SOPC_RealTime P_TIME_TimeReference_GetPtp100ns(void);

#else // CONFIG_NET_GPTP
// No time correction is PTP is not available
#define P_TIME_TimeReference_GetCorrected100ns P_TIME_TimeReference_GetInternal100ns
#endif // CONFIG_NET_GPTP

/***************************************************
 * IMPLEMENTATION OF LOCAL FUNCTIONS
 **************************************************/

/***************************************************/
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

/***************************************************/
static SOPC_RealTime P_TIME_TimeReference_GetInternal100ns(void)
{
    SOPC_RealTime result;
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

    result.tick100ns = value_100ns;
    return result;
}

/***************************************************
 * IMPLEMENTATION OF EXTERN FUNCTIONS
 **************************************************/

/***************************************************/
SOPC_DateTime SOPC_Time_GetCurrentTimeUTC()
{
    SOPC_RealTime now100ns = P_TIME_TimeReference_GetCorrected100ns();

    int64_t datetime = 0;

    // Compute value in second, used to compute UTC value
    uint64_t value_in_s = now100ns.tick100ns / SECOND_TO_100NS;
    uint64_t value_frac_in_100ns = now100ns.tick100ns % SECOND_TO_100NS;

    // ZEPHYR time_t is 64 bits.
    SOPC_ReturnStatus result = SOPC_Time_FromTimeT(value_in_s, &datetime);
    if (SOPC_STATUS_OK != result)
    {
        // Time overflow...
        datetime = INT64_MAX;
    }

    // Add to UTC value fractional part of value
    datetime += value_frac_in_100ns;

    return datetime;
}

/***************************************************/
// Return current ms since last power on
SOPC_TimeReference SOPC_TimeReference_GetCurrent()
{
    uint64_t value100ns = P_TIME_TimeReference_GetInternal100ns().tick100ns;

    return (SOPC_TimeReference)(value100ns / MS_TO_100NS);
}

/***************************************************/
SOPC_ReturnStatus SOPC_Time_Breakdown_Local(time_t t, struct tm* tm)
{
    return (NULL == localtime_r(&t, tm)) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

/***************************************************/
SOPC_ReturnStatus SOPC_Time_Breakdown_UTC(time_t t, struct tm* tm)
{
    return (NULL == gmtime_r(&t, tm)) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

/***************************************************/
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

/***************************************************/
SOPC_RealTime* SOPC_RealTime_Create(const SOPC_RealTime* copy)
{
    SOPC_RealTime* ret = SOPC_Calloc(1, sizeof(SOPC_RealTime));
    if (NULL != copy && NULL != ret)
    {
        *ret = *copy;
    }
    else if (NULL != ret)
    {
        *ret = P_TIME_TimeReference_GetInternal100ns();
    }

    return ret;
}

/***************************************************/
void SOPC_RealTime_Delete(SOPC_RealTime** t)
{
    if (NULL == t)
    {
        return;
    }
    SOPC_Free(*t);
    *t = NULL;
}

/***************************************************/
bool SOPC_RealTime_GetTime(SOPC_RealTime* t)
{
    if (NULL == t)
    {
        return false;
    }

    *t = P_TIME_TimeReference_GetInternal100ns();
    return true;
}

/***************************************************/
void SOPC_RealTime_AddDuration(SOPC_RealTime* t, double duration_ms)
{
    assert(NULL != t);

    t->tick100ns += duration_ms * MS_TO_100NS;
}

/***************************************************/
bool SOPC_RealTime_IsExpired(const SOPC_RealTime* t, const SOPC_RealTime* now)
{
    SOPC_ASSERT(NULL != t);
    SOPC_RealTime t1;

    if (NULL == now)
    {
        t1 = P_TIME_TimeReference_GetInternal100ns();
    }
    else
    {
        t1 = *now;
    }

    /* t <= t1 */
    return t->tick100ns <= t1.tick100ns;
}

/***************************************************/
bool SOPC_RealTime_SleepUntil(const SOPC_RealTime* date)
{
#if (CONFIG_SYS_CLOCK_TICKS_PER_SEC < 1000)
#warning CONFIG_SYS_CLOCK_TICKS_PER_SEC is insufficient to handle durations under 1 ms
#endif
    if (NULL == date)
    {
        return false;
    }

    SOPC_RealTime now = P_TIME_TimeReference_GetInternal100ns();

    const int64_t expDateUs = (int64_t) date->tick100ns / US_TO_100NS;
    const int64_t nowDateUs = (int64_t) now.tick100ns / US_TO_100NS;
    // No overflow possible thanks to conversion to microseconds
    int64_t toWait_us = (expDateUs - nowDateUs);

    if (toWait_us <= 0)
    {
        k_yield();
    }

    while (toWait_us > 0)
    {
        toWait_us = k_usleep(toWait_us);
    }
    return false;
}

#if CONFIG_NET_GPTP

/***************************************************
 * PTP-SPECIFIC SECTION
 **************************************************/
static struct gptp_global_ds* globalDs = NULL;

/***************************************************/
static SOPC_RealTime P_TIME_TimeReference_GetCorrected100ns(void)
{
    SOPC_RealTime nowPtp = P_TIME_TimeReference_GetPtp100ns();
    const uint64_t nowInt = P_TIME_TimeReference_GetInternal100ns().tick100ns;

    if (nowPtp.tick100ns != 0)
    {
        // PTP is available. Recalculate Internal clock offset.
        // It will be used if PTP gets unsynchronized
        if (nowPtp.tick100ns < INT64_MAX && nowInt < INT64_MAX)
        {
            gPtpSourceSync = ((int64_t) nowPtp.tick100ns) - ((int64_t) nowInt);
        }
    }
    else
    {
        nowPtp.tick100ns = nowInt + gPtpSourceSync;
    }

    return nowPtp;
}

/***************************************************/
static SOPC_RealTime P_TIME_TimeReference_GetPtp100ns(void)
{
    SOPC_RealTime result;
    struct net_ptp_time slave_time;
    bool gm_present = false;
    int errCode = gptp_event_capture(&slave_time, &gm_present);

    static const uint64_t minTimeOffset = 365 * (2021 - 1970);

    // If time is irrelevant, ignore it
    if (errCode == 0 && slave_time.second > minTimeOffset)
    {
        /* Note : 64 bits with 100 ns precision from 1970 can hold much more than 1000 years*/
        result.tick100ns = slave_time.second * SECOND_TO_100NS;
        result.tick100ns += slave_time.nanosecond / 100;
    }
    else
    {
        result.tick100ns = 0;
    }

    return result;
}

/***************************************************/
SOPC_Time_TimeSource SOPC_Time_GetTimeSource(void)
{
    SOPC_Time_TimeSource result = SOPC_TIME_TIMESOURCE_INTERNAL;
    static const int port = 1;
    // Make sure things are computed once only when possible.
    if (NULL == globalDs)
    {
        struct gptp_domain* domain = gptp_get_domain();
        struct gptp_port_ds* port_ds;

        const int ret = gptp_get_port_data(domain, port, &port_ds, NULL, NULL, NULL, NULL);

        if (ret >= 0 && NULL != port_ds && port == port_ds->port_id.port_number)
        {
            globalDs = GPTP_GLOBAL_DS();
        }
    }

    if (NULL != globalDs)
    {
        switch (globalDs->selected_role[port])
        {
        case GPTP_PORT_MASTER:
            result = SOPC_TIME_TIMESOURCE_PTP_MASTER;
            break;
        case GPTP_PORT_SLAVE:
            result = SOPC_TIME_TIMESOURCE_PTP_SLAVE;
            break;
        default:
            break;
        }
    }
    return result;
}

#else // CONFIG_NET_GPTP

/***************************************************
 * NON-PTP-SPECIFIC SECTION
 **************************************************/
/***************************************************/
SOPC_Time_TimeSource SOPC_Time_GetTimeSource(void)
{
    return SOPC_TIME_TIMESOURCE_INTERNAL;
}

#endif // CONFIG_NET_GPTP
