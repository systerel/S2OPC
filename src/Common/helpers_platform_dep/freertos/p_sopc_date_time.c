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

#include "p_sopc_common_time.h"
#include "sopc_date_time.h"
#include "sopc_time_reference.h"

#include "FreeRTOS.h"

#include <time.h>

#define SECOND_TO_100NS ((uint64_t) 10000000)

SOPC_DateTime SOPC_Time_GetCurrentTimeUTC(void)
{
    uint64_t wTimeInTicks = P_SOPC_COMMON_TIME_get_tick();
    int64_t datetime = 0;
    uint64_t currentTimeInS = 0;
    uint64_t currentTimeFrac100NS = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Convert to seconds and 100ns fractional part.
    // Warning on type used, intermediate operation on uint64_t.
    currentTimeInS = wTimeInTicks / (uint64_t) configTICK_RATE_HZ;

    currentTimeFrac100NS = ((wTimeInTicks * SECOND_TO_100NS) / (uint64_t) configTICK_RATE_HZ) % SECOND_TO_100NS;

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
        return INT64_MAX;
    }

    if (currentTimeInS > limit)
    {
        return INT64_MAX;
    }

    currentTimeT = (time_t) currentTimeInS;
    status = SOPC_Time_FromUnixTime(currentTimeT, &datetime);
    if (SOPC_STATUS_OK != status)
    {
        // Time overflow...
        return INT64_MAX;
    }

    datetime += currentTimeFrac100NS;
    return datetime;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_Local(SOPC_Unix_Time t, struct tm* tm)
{
    return (localtime_r(&t, tm) == NULL) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_UTC(SOPC_Unix_Time t, struct tm* tm)
{
    return (gmtime_r(&t, tm) == NULL) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
}

#if !S2OPC_CRYPTO_MBEDTLS // stop multiple definition of 'sopc_time_alt'
time_t sopc_time_alt(time_t* timer)
{
    return (time_t) P_SOPC_COMMON_TIME_get_tick() / configTICK_RATE_HZ;
}
#endif // S2OPC_CRYPTO_MBEDTLS
