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

#include "sopc_time.h"
#include "sopc_assert.h"

#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"

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
    SOPC_ASSERT(status == SOPC_STATUS_OK);

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
    SOPC_ASSERT(res2 > 0);

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
    SOPC_ASSERT(time >= 0);

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

static bool parseTwoDigitsUint8(const char* startPointer, size_t len, const char endChar, uint8_t* pOut)
{
    SOPC_ASSERT(NULL != startPointer);
    SOPC_ASSERT(NULL != pOut);

    if ((len > 2 && startPointer[2] != endChar) || len < 2)
    {
        return false;
    }

    SOPC_ReturnStatus status = SOPC_strtouint8_t(startPointer, pOut, 10, endChar);
    if (SOPC_STATUS_OK != status)
    {
        return false;
    }

    return true;
}

bool SOPC_tm_FromXsdDateTime(const char* datetime, size_t len, SOPC_tm* tm)
{
    if (NULL == tm)
    {
        return false;
    }

    SOPC_tm res;
    res.year = 0;
    res.month = 0;
    res.day = 0;
    res.hour = 0;
    res.minute = 0;
    res.second = 0;
    res.secondAndFrac = 0.0;
    res.UTC = true;
    res.UTC_neg_off = false;
    res.UTC_hour_off = 0;
    res.UTC_min_off = 0;

    // Check input: minimum length '<YYYY>-<MM>-<DD>T<hh>:<mm>:<ss>'
    if (NULL == datetime || len < 19)
    {
        return false;
    }

    const char* currentPointer = datetime;
    size_t remainingLength = len;

    /*
     * Parse year:
     * -?([1-9][0-9]{3,}|0[0-9]{3})-
     */
    // Check if year is prefixed by '-'
    const char* endPointer = NULL;

    // Manage the case of negative year by searching from next character
    // since at least 4 digits are expected in both cases
    endPointer = memchr(currentPointer + 1, '-', remainingLength - 1);
    if (NULL == endPointer || endPointer - currentPointer < (*currentPointer == '-' ? 5 : 4))
    {
        // End character not found or year < 4 digits year
        return false;
    }

    bool bres = SOPC_strtoint(currentPointer, (size_t)(endPointer - currentPointer), 16, &res.year);
    if (!bres)
    {
        return false;
    }
    endPointer++; // remove '-' end separator
    SOPC_ASSERT(endPointer > currentPointer);
    remainingLength -= (size_t)(endPointer - currentPointer);
    currentPointer = endPointer;

    /*
     * Parse Month:
     * (0[1-9]|1[0-2])-
     */
    bres = parseTwoDigitsUint8(currentPointer, remainingLength, '-', &res.month);
    if (!bres || res.month < 1 || res.month > 12)
    {
        return false;
    }
    remainingLength -= 3;
    currentPointer += 3;

    /*
     * Parse Day:
     * (0[1-9]|[12][0-9]|3[01])T
     */
    bres = parseTwoDigitsUint8(currentPointer, remainingLength, 'T', &res.day);
    if (!bres || res.day < 1 || res.day > 31)
    {
        return false;
    }
    remainingLength -= 3;
    currentPointer += 3;

    /*
     *  Check constraint: Day-of-month Values
     *  The day value must be no more than 30 if month is one of 4, 6, 9, or 11;
     *  no more than 28 if month is 2 and year is not divisible by 4,
     *  or is divisible by 100 but not by 400;
     *  and no more than 29 if month is 2 and year is divisible by 400, or by 4 but not by 100.
     */
    if (res.day > 30 && (4 == res.month || 6 == res.month || 9 == res.month || 11 == res.month))
    {
        return false;
    }
    if (res.day > 28 && (2 == res.month && (res.year % 4 != 0 || (0 == res.year % 100 && 0 != res.year % 400))))
    {
        return false;
    }
    if (res.day > 29 && 2 == res.month) // No more than 29 days in February
    {
        return false;
    }

    /*
     * Parse Hour:
     * ([01][0-9]|2[0-4]):
     */
    bres = parseTwoDigitsUint8(currentPointer, remainingLength, ':', &res.hour);
    // Note: accept hour = 24 for case 24:00:00.0 to be check after parsing minutes and seconds
    if (!bres || res.hour > 24)
    {
        return false;
    }
    remainingLength -= 3;
    currentPointer += 3;

    /*
     * Parse Minutes:
     * ([0-5][0-9]):
     */
    bres = parseTwoDigitsUint8(currentPointer, remainingLength, ':', &res.minute);

    if (!bres || res.minute > 59)
    {
        return false;
    }
    remainingLength -= 3;
    currentPointer += 3;

    /*
     * Parse Seconds (whole number part):
     * ([0-5][0-9]):
     */
    if (remainingLength < 2)
    {
        return false;
    }
    // Use SOPC_strtouint to allow string ending without '\0' and avoid access out of string memory bounds
    bres = SOPC_strtouint(currentPointer, 2, 8, &res.second);
    if (!bres || res.second > 59)
    {
        return false;
    }

    // Initialize seconds without fraction first (in case of no fraction present)
    res.secondAndFrac = (double) res.second;

    // Check 24:00:00 special case
    if (24 == res.hour && (0 != res.minute || 0 != res.second))
    {
        return false;
    }

    if (2 == remainingLength)
    {
        // Whole datetime string consume without any error
        *tm = res;

        return true;
    }

    /*
     * Parse Seconds with fraction (if applicable)
     * [0-5][0-9](\.[0-9]+)?
     */

    // Check if there is a fraction of second
    if ('.' == currentPointer[2])
    {
        // Search for end character starting from character after the '.'
        endPointer = &currentPointer[3];
        size_t localRemLength = remainingLength - 3;

        while (localRemLength > 0 && *endPointer >= '0' && *endPointer <= '9')
        {
            // Check all digits are 0 if hour was 24
            if (24 == res.hour && '0' != *endPointer)
            {
                return false;
            }
            endPointer++;
            localRemLength--;
        }

        // Parse the seconds with fraction as a double value
        bres = SOPC_strtodouble(currentPointer, (size_t)(endPointer - currentPointer), 64, &res.secondAndFrac);
        // Note: we do not need to check for actual value since we already controlled each digit individually
        // If something went wrong it is either due to SOPC_strtodouble or due to double representation
        if (!bres)
        {
            return false;
        }
        remainingLength -= (size_t)(endPointer - currentPointer);
        currentPointer = endPointer;
    }
    else
    {
        remainingLength -= 2;
        currentPointer += 2;
    }

    if (0 != remainingLength && 'Z' != *currentPointer)
    {
        // Parse offset sign
        if ('-' == *currentPointer)
        {
            res.UTC_neg_off = true;
        }
        else if ('+' != *currentPointer)
        {
            return false;
        }

        remainingLength--;
        currentPointer++;

        /*
         * Parse Hour Offset:
         * (0[0-9]|1[0-4]):
         */
        bres = parseTwoDigitsUint8(currentPointer, remainingLength, ':', &res.UTC_hour_off);
        // Note: accept hour = 14 for case 14:00 to be check after parsing minutes and seconds
        if (!bres || res.UTC_hour_off > 14)
        {
            return false;
        }
        remainingLength -= 3;
        currentPointer += 3;

        /*
         * Parse Minute Offset
         * ([0-5][0-9]):
         */
        if (remainingLength < 2)
        {
            return false;
        }
        // Use SOPC_strtouint to allow string ending without '\0' and avoid access out of string memory bounds
        bres = SOPC_strtouint(currentPointer, 2, 8, &res.UTC_min_off);
        // Check for special case of 14:00 which is maximum offset value
        if (!bres || res.UTC_min_off > 59 || (14 == res.UTC_hour_off && 0 != res.UTC_min_off))
        {
            return false;
        }

        remainingLength -= 2;
        currentPointer += 2;

        // Set UTC flag regarding offset
        res.UTC = (0 == res.UTC_hour_off && 0 == res.UTC_min_off);
    }
    else if ('Z' == *currentPointer)
    {
        remainingLength--;
        currentPointer++;
    }

    if (0 != remainingLength)
    {
        return false;
    }

    *tm = res;
    return true;
}

static int64_t daysSince1601(int16_t year, uint8_t month, uint8_t day)
{
    SOPC_ASSERT(year >= 1601);
    SOPC_ASSERT(year <= 10000);

    // Years since 1601
    int16_t elapsedYearsSince1601 = (int16_t)(year - 1601);

    // Month-to-day offset for non-leap-years.
    const int64_t monthDaysElapsed[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    // Number of February months since 01/01/1601
    int64_t nbFebs = elapsedYearsSince1601 + (1 - (month <= 2 ? 1 : 0));

    // Total number of leap days since 01/01/1601
    int64_t leapDays = (nbFebs / 4) - (nbFebs / 100) + (nbFebs / 400);

    // Total number of days =
    // 365 * elapsed years + elapsed leap days + elapsed days in current year before current month (without leap day)
    // + elapsed days in current month (- 1 since current day not elapsed yet)
    int64_t days = 365 * elapsedYearsSince1601 + leapDays + monthDaysElapsed[month - 1] + day - 1;

    return days;
}

static int64_t secondsSince1601(int16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    SOPC_ASSERT(year >= 1601 || (year == 1600 && month == 12 && day == 31));
    SOPC_ASSERT(year <= 10000);

    if (year >= 1601) // number of seconds since 1601
    {
        const int64_t secsByDay = 86400;
        const int64_t secsCurrentDay = hour * 3600 + minute * 60 + second;
        const int64_t nbDaysSince1601 = daysSince1601(year, month, day);

        return secsByDay * nbDaysSince1601 + secsCurrentDay;
    }
    else // number of seconds until 01/01/1601 (from day 31/12/1600)
    {
        const int64_t secsUntil1601 = (24 - hour) * 3600 - minute * 60 - second;
        // negative number of seconds since reference is 1601
        return -1 * secsUntil1601;
    }
}

SOPC_ReturnStatus SOPC_Time_FromXsdDateTime(const char* dateTime, size_t len, int64_t* res)
{
    // 100ns as a fraction of second
    const float sec_fraction_100ns = (float) 0.0000001;

    SOPC_tm tm_res;
    memset(&tm_res, 0, sizeof(tm_res));

    bool parseRes = SOPC_tm_FromXsdDateTime(dateTime, len, &tm_res);

    if (!parseRes)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (tm_res.year < 1601 && (tm_res.year != 1600 || tm_res.month != 12 || tm_res.day != 31))
    {
        // A date/time value is encoded as 0 if is equal to or earlier than 1601-01-01 12:00AM UTC.
        // Due to timezone offset to be considered, we keep a 24:00:00 margin as a first approximation.
        // It excludes any date earlier than 31-12-1600.
        *res = 0;
        return SOPC_STATUS_OK;
    }
    else if (tm_res.year > 9999 && (tm_res.year != 10000 || tm_res.month != 1 || tm_res.day != 1))
    {
        // A date/time is encoded as the maximum value for an Int64 if
        // the value is equal to or greater than 9999-12-31 11:59:59PM UTC
        // Due to timezone offset to be considered, we keep a 24:00:00 margin as a first approximation.
        // It excludes any date greater than 01-01-10000.
        *res = INT64_MAX;
        return SOPC_STATUS_OK;
    }

    // Compute seconds since 1601
    int64_t secsSince1601 =
        secondsSince1601(tm_res.year, tm_res.month, tm_res.day, tm_res.hour, tm_res.minute, tm_res.second);

    if (!tm_res.UTC)
    {
        int64_t offset = tm_res.UTC_hour_off * 3600 + tm_res.UTC_min_off * 60;
        if (tm_res.UTC_neg_off)
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

    // Check date >= 1601-01-01 12:00AM UTC
    if (secsSince1601 < 0)
    {
        // A date/time value is encoded as 0 if is equal to or earlier than 1601-01-01 12:00AM UTC.
        // Due to timezone offset to be considered, we keep a 24:00:00 margin as a first approximation.
        // It excludes any date earlier than 31-12-1600.
        *res = 0;
        return SOPC_STATUS_OK;
    }
    else if (secsSince1601 >= 265046774399) // Check date < 9999-12-31 11:59:59PM UTC
    {
        // Note: 265046774399 == secondsSince1601(9999, 12, 31, 23, 59, 59)

        // A date/time is encoded as the maximum value for an Int64 if
        // the value is equal to or greater than 9999-12-31 11:59:59PM UTC
        *res = INT64_MAX;
        return SOPC_STATUS_OK;
    }

    // Compute seconds fraction if significant
    double sec_fraction = tm_res.secondAndFrac - (double) tm_res.second;
    int64_t hundredOfNanoseconds = (int64_t)(sec_fraction / sec_fraction_100ns);

    // Note: no overflow possible for 1601 <= year <= 10000
    *res = secsSince1601 * SOPC_SECOND_TO_100_NANOSECONDS + hundredOfNanoseconds;
    return SOPC_STATUS_OK;
}
