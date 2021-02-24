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

static int64_t daysSince1601(int16_t year, uint8_t month, uint8_t day)
{
    assert(year >= 1601);
    assert(year <= 9999);

    // Number of days until 01/01/1601: 1600 * 365 + 1600 / 4 - 1600 / 100 + 1600 /400
    const int64_t daysUntil1601 = 584388;

    // Month-to-day offset for non-leap-years.
    const int64_t monthDaysElapsed[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    // Number of February months (no year 0)
    int64_t nbFebs = year - (month <= 2 ? 1 : 0);

    // Total number of leap days
    int64_t leapDays = (nbFebs / 4) - (nbFebs / 100) + (nbFebs / 400);

    // Total number of days =
    // 365 * elapsed years + elapsed leap days + elapsed days in year before current month (without leap day)
    // + elapsed days in current month - 1 (current day not elapsed yet)
    int64_t days = 365 * (year - 1) + leapDays + monthDaysElapsed[month - 1] + day - 1;

    return days - daysUntil1601;
}

static int64_t secondsSince1601(int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    assert(year >= 1601);
    assert(year <= 9999);

    const int64_t secsByDay = 86400;
    const int64_t secsCurrentDay = hour * 3600 + minute * 60 + second;
    const int64_t nbDaysSince1601 = daysSince1601(year, month, day);

    return secsByDay * nbDaysSince1601 + secsCurrentDay;
}

SOPC_ReturnStatus SOPC_Time_FromXsdDateTime(const char* dateTime, size_t len, int64_t* res)
{
    // 100ns as a fraction of second
    const float sec_fraction_100ns = (float) 0.0000001;

    int16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    float secondAndFraction = 0.0;
    bool utc = true;
    bool utc_neg_off = false;
    uint8_t utc_hour_off = 0;
    uint8_t utc_min_off = 0;
    bool parseRes = SOPC_stringToDateTime(dateTime, len, &year, &month, &day, &hour, &minute, &second,
                                          &secondAndFraction, &utc, &utc_neg_off, &utc_hour_off, &utc_min_off);

    if (!parseRes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (year < 1601)
    {
        // A date/time value is encoded as 0 if is equal to or earlier than 1601-01-01 12:00AM UTC
        *res = 0;
        return SOPC_STATUS_OK;
    }
    else if (year > 9999)
    {
        // A date/time is encoded as the maximum value for an Int64 if
        // the value is equal to or greater than 9999-12-31 11:59:59PM UTC
        *res = INT64_MAX;
        return SOPC_STATUS_OK;
    }

    // Compute seconds since 1601
    int64_t secsSince1601 = secondsSince1601(year, month, day, hour, minute, second);

    if (!utc)
    {
        int64_t offset = utc_hour_off * 3600 + utc_min_off * 60;
        if (utc_neg_off)
        {
            // Negative offset, add the offset
            secsSince1601 += offset;
        }
        else
        {
            // Positive offset, substract the offset
            secsSince1601 -= offset;
        }
    }

    // Compute seconds fraction if significant
    float sec_fraction = secondAndFraction - (float) second;
    int64_t hundredOfNanoseconds = (int64_t)(sec_fraction / sec_fraction_100ns);

    // Note: no overflow possible for 1601 <= year <= 9999
    *res = secsSince1601 * SOPC_SECOND_TO_100_NANOSECONDS + hundredOfNanoseconds;

    return SOPC_STATUS_OK;
}
