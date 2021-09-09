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
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"

static const int64_t SOPC_SECONDS_BETWEEN_EPOCHS = 11644473600;
static const int64_t SOPC_SECOND_TO_100_NANOSECONDS = 10000000; // 10^7

char* SOPC_Time_GetString(int64_t time, bool local, bool compact)
{
    static const size_t buf_size = 24;

    if (time == 0)
    {
        return NULL;
    }

    time_t seconds = 0;
    SOPC_ReturnStatus status = SOPC_Time_ToTimeT(time, &seconds);
    assert(status == SOPC_STATUS_OK);

    uint32_t milliseconds = (uint32_t)((time / 10000) % 1000);
    struct tm tm;

    if (local)
    {
        status = SOPC_Time_Breakdown_Local(seconds, &tm);
    }
    else
    {
        status = SOPC_Time_Breakdown_UTC(seconds, &tm);
    }

    if (status != SOPC_STATUS_OK)
    {
        return NULL;
    }

    char* buf = SOPC_Calloc(buf_size, sizeof(char));

    if (buf == NULL)
    {
        return NULL;
    }

    size_t res = strftime(buf, buf_size - 1, compact ? "%Y%m%d_%H%M%S" : "%Y/%m/%d %H:%M:%S", &tm);

    if (res == 0)
    {
        SOPC_Free(buf);
        return NULL;
    }

    int res2 = sprintf(buf + 19, compact ? "_%03" PRIu32 : ".%03" PRIu32, milliseconds);
    assert(res2 > 0);

    return buf;
}

static char* get_current_time_string(bool local, bool compact)
{
    return SOPC_Time_GetString(SOPC_Time_GetCurrentTimeUTC(), local, compact);
}

char* SOPC_Time_GetStringOfCurrentLocalTime(bool compact)
{
    return get_current_time_string(true, compact);
}

char* SOPC_Time_GetStringOfCurrentTimeUTC(bool compact)
{
    return get_current_time_string(false, compact);
}

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

SOPC_ReturnStatus SOPC_Time_FromTimeT(time_t time, int64_t* res)
{
    assert(time >= 0);

#if (SOPC_TIME_T_SIZE > 4)
    if (time > INT64_MAX)
    {
        return SOPC_STATUS_NOK;
    }
#endif

    int64_t dt = time;

    if (INT64_MAX - SOPC_SECONDS_BETWEEN_EPOCHS < dt)
    {
        return SOPC_STATUS_NOK;
    }

    dt += SOPC_SECONDS_BETWEEN_EPOCHS;

    if (INT64_MAX / SOPC_SECOND_TO_100_NANOSECONDS < dt)
    {
        return SOPC_STATUS_NOK;
    }

    dt *= SOPC_SECOND_TO_100_NANOSECONDS;
    *res = dt;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_Time_ToTimeT(int64_t dateTime, time_t* res)
{
    int64_t secs = dateTime / SOPC_SECOND_TO_100_NANOSECONDS;

    if (secs < SOPC_SECONDS_BETWEEN_EPOCHS)
    {
        return SOPC_STATUS_NOK;
    }

    secs -= SOPC_SECONDS_BETWEEN_EPOCHS;

    if (secs > LONG_MAX)
    {
        return SOPC_STATUS_NOK;
    }

    if (secs == (int64_t)(time_t) secs)
    {
        *res = (time_t) secs;
        return SOPC_STATUS_OK;
    }
    else
    {
        return SOPC_STATUS_NOK;
    }
}
