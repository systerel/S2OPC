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

#define UINT32_SHIFT (1llu << 32)
#define UINT31_SHIFT (1llu << 31)

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

#include "sopc_assert.h"
#include "sopc_enums.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"
#include "zephyr/p_time.h"
#include "zephyr/sopc_zephyr_time.h"

/***************************************************
 * DECLARATION OF LOCAL MACROS
 **************************************************/

/***************************************************
 * DECLARATION OF LOCAL TYPES/VARIABLES
 **************************************************/

#define SECOND_TO_100NS (10000000)
#define SECOND_TO_US (1000 * 1000)
#define MS_TO_100NS (10000)
#define US_TO_100NS (10)

#define CLOCK_IS_IMPRECISE 0.0f

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
/***************************************************
 * GPTP SPECIFIC DECLARATIONS
 **************************************************/
/*
 * PTP implementation details:
 * - PtP clock is used to read actual time received. This time is used to:
 *   - Compute gPtpSourceSync, which is constantly updated to resynchronize local clock
 *          to PtP time (In calls to ::SOPC_Time_GetCurrentTimeUTC)
 *   - Compute gLocalClockCorrFactor, which is updated every CONFIG_SOPC_PTP_SYNCH_DURATION seconds
 *          by measuring the elapsing difference between internal and PtP clocks. If the correction is
 *          lower than CLOCK_CORRECTION_RANGE, then this correction is accepted and applied to :
 *      - Continue the correction of internal clock deviation in the case the PtP is lost.
 *      - Adjust actual requested ticks in call to SOPC_Sleep
 *      - Adjust actual requested ticks in call to SOPC_RealTime-related features. As they rely on
 *          internal clock, and as the time reference is also used by caller, the correction shall be
 *          applied on SOPC_RealTime operations (SOPC_RealTime_AddSynchedDuration).
 * SOPC_TimeReference-related features are not resynchronized and remain on internal MONOTONIC clock.
 */

#define GPTP_ASSERT(x) SOPC_ASSERT(x)
#define GPTP_CORRECT(x) ((x) *gLocalClockCorrFactor)
#define CLOCK_CORRECTION_RANGE 0.1

/** Offset between PtP & internal clock, (or 0 if unknown)*/
static int64_t gPtpSourceSync = 0;
/** date of local clock at last PtP synchronized time (last update of gPtpSourceSync) */
static uint64_t gLastLocSyncDate = 0;

/** Local clock correction factor
 * Range [1 - CLOCK_CORRECTION_RANGE .. 1 + CLOCK_CORRECTION_RANGE]
 *  */
static float gLocalClockCorrFactor = 1.0;

/** See \SOPC_RealTime_GetClockImprecision */
static float gLocalClockPrecision = 0.0;

/** Next clcock correction synchro point */
static uint64_t gMeasureNextSynch = 0;

/** \brief Get current internal time with 100 ns precision, corrected by PTP */
static SOPC_RealTime P_TIME_TimeReference_GetCorrected100ns(void);

/** \brief Get current PTP time with 100 ns precision
 * \return
 *   - Current time (unit = 100ns, from 1970, Jan. 1st)
 *   - 0 if PtpClock is not ready or provides irrelevant value*/
static SOPC_RealTime P_TIME_TimeReference_GetPtp100ns(void);

#else // CONFIG_NET_GPTP
#define GPTP_ASSERT(x)
#define GPTP_CORRECT(x) (x)
// No time correction if PTP is not available
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
static uint64_t uint64_gcd(uint64_t a, uint64_t b)
{
    uint64_t temp;
    while (b != 0)
    {
        temp = a % b;

        a = b;
        b = temp;
    }
    return a;
}

/***************************************************/
static SOPC_RealTime P_TIME_TimeReference_GetInternal100ns(void)
{
    SOPC_RealTime result;
    ePTimeStatus expectedStatus = P_TIME_STATUS_NOT_INITIALIZED;
    ePTimeStatus desiredStatus = P_TIME_STATUS_INITIALIZING;

    // Note: avoid u64 overflow by reducing factors
    static uint64_t tick_to_100ns_n = 0;
    static uint64_t tick_to_100ns_d = 0;
    static struct k_mutex monotonicMutex;
    static int64_t hw_clocks_per_sec = 0;

    bool bTransition = __atomic_compare_exchange(&gTimeStatus, &expectedStatus, &desiredStatus, false, __ATOMIC_SEQ_CST,
                                                 __ATOMIC_SEQ_CST);

    if (bTransition)
    {
        hw_clocks_per_sec = sys_clock_hw_cycles_per_sec();
        const uint64_t tick_reduce_factor = uint64_gcd(SECOND_TO_100NS, hw_clocks_per_sec);

        // tick_to_100ns_n and tick_to_100ns_d are numerator and denominator of (SECOND_TO_100NS / hw_clocks_per_sec)
        // So as to avoid overflows, they are reduced using their GCD
        tick_to_100ns_d = (hw_clocks_per_sec / tick_reduce_factor);
        tick_to_100ns_n = (SECOND_TO_100NS / tick_reduce_factor);

        SOPC_ASSERT(0 < tick_to_100ns_d);

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

    static uint64_t overflow_ticks = 0;
    static uint32_t last_kernel_tick = 0;
    static int64_t last_uptime_ms = 0;

    k_mutex_lock(&monotonicMutex, K_FOREVER);

    /**
     * Note: algorithm principle:
     * - read hw clock (32 bits, overflows typically every 9 seconds)
     * - get uptime (64 bits, in ms, imprecise)
     * - compute the number of full HW cycles missing to match both clocks.
     * - Add the exact number of ticks to HW clock (now is 64 bits wide)
     * - Convert the resulting tick counts into 100ns unit for final result.
     * Note that if \a k_cycle_get_64 is available, all this is not required, however,
     * this function is typically not available on 32 bits platforms.
     */
    // Get associated hardware clock counter
    uint32_t kernel_clock_ticks = k_cycle_get_32();
    const int64_t uptime_ms = k_uptime_get();

    if (last_uptime_ms > 0 && last_uptime_ms <= uptime_ms)
    {
        // Compute time since last call (approximate in ms)
        const int64_t delta_uptime_ms = uptime_ms - last_uptime_ms;
        const int64_t delta_uptime_ticks = delta_uptime_ms * (hw_clocks_per_sec / 1000);

        // Compute time since last call (modulo 32 bits in system TICKS)
        const int64_t delta_cycle_ticks = ((int64_t) kernel_clock_ticks) - ((int64_t) last_kernel_tick);

        // Compute how many new times the hw clock has made loops
        const int64_t missing_ticks = (delta_uptime_ticks - delta_cycle_ticks);
        const int64_t missing_loops = ((missing_ticks + (UINT31_SHIFT)) / UINT32_SHIFT);
        overflow_ticks += (missing_loops << 32);
    }

    const uint64_t kernel_clock_100ns = ((kernel_clock_ticks + overflow_ticks) * tick_to_100ns_n) / tick_to_100ns_d;

    const uint64_t value_100ns = gUptimeDate_s * SECOND_TO_100NS + kernel_clock_100ns;

    last_kernel_tick = kernel_clock_ticks;
    last_uptime_ms = uptime_ms;

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
    else
    {
        // Add to UTC value fractional part of value
        datetime += value_frac_in_100ns;
    }

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
        GPTP_ASSERT(fabs(1.0 - gLocalClockCorrFactor) <= CLOCK_CORRECTION_RANGE);
        k_sleep(K_MSEC(GPTP_CORRECT(milliseconds)));
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
void SOPC_RealTime_AddSynchedDuration(SOPC_RealTime* t, uint64_t duration_us, int32_t offset_us)
{
    SOPC_ASSERT(NULL != t);
    uint32_t increment_us = duration_us;
    GPTP_ASSERT(fabs(1.0 - gLocalClockCorrFactor) <= CLOCK_CORRECTION_RANGE);

    if (offset_us >= 0)
    {
        const uint64_t minIncrement = duration_us / 5;
        /**
         * Window offset principle.
         * - Find out current position in window [0 .. duration_us -1]
         * - Add the missing time up to next position "offset_us" in that window
         * - In the case the current position is close BEFORE "offset_us", then add a full cycle to
         *      wait time. This case means that this function was called a little too early due to
         *      clock discrepancies:
         *       - position in window is given by system DateTime clock (which is continuously
         *           corrected by PTP/NTP)
         *       - actual wait times are provided by monotonic realtime clock
         */

        SOPC_ASSERT(duration_us > 0 && duration_us <= UINT32_MAX);

        uint64_t currentWinTime_us = (SOPC_Time_GetCurrentTimeUTC() / 10);
        const uint32_t duration32_us = (uint32_t) duration_us;
        const uint32_t currentWinTime32_us = (uint32_t)(currentWinTime_us % SECOND_TO_US);

        // Current Position of clock within window given by duration_us
        const uint32_t currentWinPos_us = currentWinTime32_us % duration32_us;
        // consider the remainder relatively to a window starting at offset_us rather than 0
        // windowOffset_us is 0 if current time matches offset_us
        // windowOffset_us is small positive if current time is past offset_us
        // windowOffset_us is large positive (from duration_us -1) if current time is right before offset_us
        const uint32_t windowOffset_us = (duration32_us + currentWinPos_us - offset_us) % duration32_us;

        SOPC_ASSERT(increment_us > windowOffset_us);
        // reomve the part of the cycle that already elapsed
        increment_us -= windowOffset_us;

        // Consider that event is in the "past" if windowOffset_us is close to next event (>80% of cycle)
        if (increment_us < minIncrement)
        {
            increment_us += duration32_us;
        }
    }
    t->tick100ns += (uint64_t) GPTP_CORRECT(increment_us * US_TO_100NS);
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
    static uint64_t connectionStartLoc = 0;
    static uint64_t connectionStartPtp = 0;

    SOPC_RealTime nowPtp = P_TIME_TimeReference_GetPtp100ns();
    const uint64_t nowPtpInt = nowPtp.tick100ns;
    const uint64_t nowLocInt = P_TIME_TimeReference_GetInternal100ns().tick100ns;

    // During synchronization phase, invalid time laps may appear
    bool invalidTimes = false;

    // Check that PtP is currently SLAVE to avoid re-synchronizing clock on local measure
    if (nowPtpInt != 0)
    {
        if (connectionStartLoc == 0)
        {
            // Start correction measure
            connectionStartLoc = nowLocInt;
            connectionStartPtp = nowPtpInt;
        }
        else if (nowPtpInt < connectionStartPtp || nowLocInt < connectionStartLoc)
        {
            invalidTimes = true;
        }
        else
        {
            // Measured time on PTP clock (external)
            const float measurePtpDuration = (float) (nowPtpInt - connectionStartPtp);
            // Measured time on RT clock (internal)
            const float measureLocDuration = (float) (nowLocInt - connectionStartLoc);
            // newFactor represents the divergence of RT clock towards PTP clock, which is supposed
            // of good quality
            float newFactor = measureLocDuration / measurePtpDuration;

            if (fabs(1.0 - newFactor) >= CLOCK_CORRECTION_RANGE)
            {
                invalidTimes = true;
            }
            else if (measurePtpDuration / SECOND_TO_100NS > CONFIG_SOPC_PTP_SYNCH_DURATION + gMeasureNextSynch)
            {
                gMeasureNextSynch += CONFIG_SOPC_PTP_SYNCH_DURATION;

                // Compute precision, based on difference between current and new correction
                gLocalClockPrecision = 1.0 - fabs((newFactor - gLocalClockCorrFactor) / CLOCK_CORRECTION_RANGE);
                // Accept this correction factor
                gLocalClockCorrFactor = newFactor;
            }
        }
        // Check if correction measure is ready

        // PTP is available. Recalculate Internal clock offset.
        // It will be used if PTP gets unsynchronized
        if (nowPtpInt < INT64_MAX && nowLocInt < INT64_MAX)
        {
            gPtpSourceSync = ((int64_t) nowPtpInt) - ((int64_t) nowLocInt);
        }
        gLastLocSyncDate = nowLocInt;
    }
    else
    {
        // PTP is not available. Apply deviation correction since PtP disconnection
        nowPtp.tick100ns = nowLocInt + gPtpSourceSync;
        if (gLastLocSyncDate > 0 && gLastLocSyncDate < nowLocInt)
        {
            const uint64_t delta = (nowLocInt - gLastLocSyncDate);

            const float corrFact = (1.0 - gLocalClockCorrFactor) / gLocalClockCorrFactor;
            const int64_t corr = (int64_t)(delta * corrFact);
            nowPtp.tick100ns += corr;
        }

        invalidTimes = true;
    }
    if (invalidTimes)
    {
        connectionStartLoc = 0;
        connectionStartPtp = 0;
        gMeasureNextSynch = 0;
        gLocalClockPrecision = CLOCK_IS_IMPRECISE;
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

    static const uint64_t minTimeOffset = 365 * (2021 - 1970) * 24 * 60 * 60;

    // If time is irrelevant or provided locally, ignore it
    // a "PtP MASTER" state can be observed in transitions (connection/disconnection)
    if (errCode == 0 && slave_time.second > minTimeOffset &&
        SOPC_Time_GetTimeSource() == SOPC_TIME_TIMESOURCE_PTP_SLAVE)
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
float SOPC_RealTime_GetClockCorrection(void)
{
    return gLocalClockCorrFactor;
}

/***************************************************/
float SOPC_RealTime_GetClockPrecision(void)
{
    return gLocalClockPrecision;
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
