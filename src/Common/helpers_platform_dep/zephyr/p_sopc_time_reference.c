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

#include <zephyr/kernel.h>

#include "sopc_assert.h"
#include "sopc_date_time.h"
#include "sopc_mem_alloc.h"
#include "sopc_time_reference.h"

#define UINT32_SHIFT (1llu << 32)
#define UINT31_SHIFT (1llu << 31)

#define SECOND_TO_100NS (10000000)
#define SECOND_TO_US (1000 * 1000)
#define MS_TO_100NS (10000)
#define US_TO_100NS (10)

struct SOPC_HighRes_TimeReference
{
    /* Internal unit is 100ns. 64 bits are enough to store more than 1000 years.
       Reference is boot time*/
    uint64_t tick100ns;
};

typedef enum P_TIME_STATUS
{
    P_TIME_STATUS_NOT_INITIALIZED,
    P_TIME_STATUS_INITIALIZING,
    P_TIME_STATUS_INITIALIZED
} ePTimeStatus;

static ePTimeStatus gTimeStatus = P_TIME_STATUS_NOT_INITIALIZED;

static SOPC_HighRes_TimeReference P_TIME_TimeReference_GetInternal100ns(void);

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
static SOPC_HighRes_TimeReference P_TIME_TimeReference_GetInternal100ns(void)
{
    SOPC_HighRes_TimeReference result;
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

    last_kernel_tick = kernel_clock_ticks;
    last_uptime_ms = uptime_ms;

    k_mutex_unlock(&monotonicMutex);

    result.tick100ns = kernel_clock_100ns;
    return result;
}

/***************************************************/
// Return current ms since last power on
SOPC_TimeReference SOPC_TimeReference_GetCurrent()
{
    uint64_t value100ns = P_TIME_TimeReference_GetInternal100ns().tick100ns;

    return (SOPC_TimeReference)(value100ns / MS_TO_100NS);
}

/***************************************************/
SOPC_TimeReference SOPC_TimeReference_AddMilliseconds(SOPC_TimeReference timeRef, uint64_t ms)
{
    SOPC_TimeReference result = 0;

    if (UINT64_MAX - timeRef > ms)
    {
        result = timeRef + ms;
    }
    else
    {
        // Set maximum representable value
        result = UINT64_MAX;
    }

    return result;
}

/***************************************************/
int8_t SOPC_TimeReference_Compare(SOPC_TimeReference left, SOPC_TimeReference right)
{
    int8_t result = 0;
    if (left < right)
    {
        result = -1;
    }
    else if (left > right)
    {
        result = 1;
    }
    return result;
}

/***************************************************/
bool SOPC_HighRes_TimeReference_GetTime(SOPC_HighRes_TimeReference* t)
{
    SOPC_ASSERT(NULL != t);
    *t = P_TIME_TimeReference_GetInternal100ns();
    return true;
}

/***************************************************/
bool SOPC_HighRes_TimeReference_IsExpired(const SOPC_HighRes_TimeReference* t, const SOPC_HighRes_TimeReference* now)
{
    SOPC_ASSERT(NULL != t);
    SOPC_HighRes_TimeReference t1;

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
bool SOPC_HighRes_TimeReference_SleepUntil(const SOPC_HighRes_TimeReference* date)
{
    SOPC_ASSERT(NULL != date);

    SOPC_HighRes_TimeReference now = P_TIME_TimeReference_GetInternal100ns();

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
    return true;
}

/***************************************************/
int64_t SOPC_HighRes_TimeReference_DeltaUs(const SOPC_HighRes_TimeReference* tRef, const SOPC_HighRes_TimeReference* t)
{
    SOPC_ASSERT(NULL != tRef);
    SOPC_HighRes_TimeReference t1;

    if (NULL == t)
    {
        t1 = P_TIME_TimeReference_GetInternal100ns();
    }
    else
    {
        t1 = *t;
    }

    return (((int64_t) t1.tick100ns) - ((int64_t) tRef->tick100ns)) / US_TO_100NS;
}

/***************************************************/
void SOPC_HighRes_TimeReference_AddSynchedDuration(SOPC_HighRes_TimeReference* t,
                                                   uint64_t duration_us,
                                                   int32_t offset_us)
{
    SOPC_ASSERT(NULL != t);
    uint32_t increment_us = duration_us;

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
    t->tick100ns += (uint64_t) increment_us * US_TO_100NS;
}

/***************************************************/
SOPC_HighRes_TimeReference* SOPC_HighRes_TimeReference_Create(void)
{
    SOPC_HighRes_TimeReference* ret = SOPC_Calloc(1, sizeof(SOPC_HighRes_TimeReference));
    if (NULL != ret)
    {
        bool ok = SOPC_HighRes_TimeReference_GetTime(ret);
        if (!ok)
        {
            SOPC_HighRes_TimeReference_Delete(&ret);
        }
    }

    return ret;
}

/***************************************************/
void SOPC_HighRes_TimeReference_Delete(SOPC_HighRes_TimeReference** t)
{
    if (NULL == t)
    {
        return;
    }
    SOPC_Free(*t);
    *t = NULL;
}

/***************************************************/
bool SOPC_HighRes_TimeReference_Copy(SOPC_HighRes_TimeReference* to, const SOPC_HighRes_TimeReference* from)
{
    if (NULL == from || NULL == to)
    {
        return false;
    }
    *to = *from;
    return true;
}
