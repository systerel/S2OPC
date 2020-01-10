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

#include <errno.h>
#include <inttypes.h>
#include <kernel.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __INT32_MAX__
#include <toolchain/xcc_missing_defs.h>
#endif

#ifndef NULL
#define NULL ((void*) 0)
#endif

#include "sopc_enums.h" /* s2opc includes */
#include "sopc_time.h"

/* Private time api */

#define P_TIME_DEBUG (0)

#define SECOND_TO_100NS ((uint64_t) 10000000)
#define MS_TO_100NS ((uint64_t) 10000)
#ifndef CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC
#define CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC 600000000
#endif

static inline uint64_t P_TIME_TimeReference_GetCurrent100ns(void)
{
    uint64_t soft_clock_ms = (SOPC_TimeReference) k_uptime_get();
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

    if (result == SOPC_STATUS_OK)
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
    return (localtime_r(&t, tm) == NULL) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_UTC(time_t t, struct tm* tm)
{
    return (gmtime_r(&t, tm) == NULL) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

void SOPC_Sleep(unsigned int milliseconds)
{
    k_sleep(milliseconds);
    return;
}
