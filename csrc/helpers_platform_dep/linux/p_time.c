/*
 *  Copyright (C) 2017 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "sopc_time.h"

/*
 * It represents the number of seconds between the OPC-UA (Windows) which starts on 1601/01/01 (supposedly 00:00:00
 * UTC), and Linux times starts on epoch, 1970/01/01 00:00:00 UTC.
 * */
static const uint64_t SOPC_SECONDS_BETWEEN_EPOCHS = 11644473600;
static const uint64_t SOPC_SECONDS_TO_100_NANOSECONDS = 10000000; // 10^7
static const uint64_t SOPC_MILLISECONDS_TO_NANOSECONDS = 1000000; // 10^6

SOPC_DateTime SOPC_Time_GetCurrentTimeUTC()
{
    struct timespec currentTime;
    int gettimeResult;
    uint64_t intermediateResult = 0;
    SOPC_DateTime result = 0;

    /*
     * Extract from clock_gettime documentation:
     * All implementations support the system-wide real-time clock, which is identified by CLOCK_REALTIME.  Its time
     * represents seconds and nanoseconds since the Epoch.  When its time is changed, timers for a relative interval are
     * unaffected, but timers for an absolute point in time are affected.
     * */
    gettimeResult = clock_gettime(CLOCK_REALTIME, &currentTime);
    if (gettimeResult == 0)
    {
        intermediateResult = currentTime.tv_sec;
        if (UINT64_MAX - SOPC_SECONDS_BETWEEN_EPOCHS > intermediateResult)
        {
            intermediateResult += SOPC_SECONDS_BETWEEN_EPOCHS;
            if (UINT64_MAX / SOPC_SECONDS_TO_100_NANOSECONDS > intermediateResult)
            {
                intermediateResult *= SOPC_SECONDS_TO_100_NANOSECONDS;
                result = currentTime.tv_nsec / 100; // set nanosecs in 100 nanosecs
                if ((uint64_t) INT64_MAX >= intermediateResult + (uint64_t) result)
                {
                    result += intermediateResult;
                }
                else
                {
                    // Maximum value to be used as result since value does not fit in INT64 value
                    result = INT64_MAX;
                }
            }
        }
        else
        {
            // Maximum value to be used as result since value does not fit in INT64 value
            result = INT64_MAX;
        }

    } // else return the minimum time value which is 0

    return result;
}

SOPC_TimeReference* SOPC_TimeReference_GetCurrent()
{
    /* Extract of clock_gettime documentation:
     *
     *   CLOCK_REALTIME
     *          System-wide  clock  that measures real (i.e., wall-clock) time.  Setting this clock requires appropriate
     * privileges.  This clock is affected by discontinuous jumps in the system time (e.g., if the system administrator
     * manually changes the clock), and by the incremental adjustments performed by adjtime(3) and NTP.
     *
     *   CLOCK_MONOTONIC
     *          Clock that cannot be set and represents monotonic time since some unspecified starting point.  This
     * clock is not affected by discontinuous jumps in the system time (e.g., if the  system administrator manually
     * changes the clock), but is affected by the incremental adjustments performed by adjtime(3) and NTP.
     *
     *   CLOCK_MONOTONIC_RAW (since Linux 2.6.28; Linux-specific)
     *          Similar to CLOCK_MONOTONIC, but provides access to a raw hardware-based time that is not subject to NTP
     * adjustments or the incremental adjustments performed by adjtime(3).
     */

    int gettimeResult;
    SOPC_TimeReference* result = calloc(1, sizeof(SOPC_TimeReference));

    if (result != NULL)
    {
        gettimeResult = clock_gettime(CLOCK_MONOTONIC_RAW, result);
        if (gettimeResult != 0)
        {
            gettimeResult = clock_gettime(CLOCK_MONOTONIC, result);
        }
        if (gettimeResult != 0)
        {
            /* If the system does not support monotonic clock,
             * it is necessary to set SOPC_MONOTONIC_CLOCK to false.
             * In this latter case the real time is used which is not monotonic
             * and there is no guarantee on elapsed duration computation.
             * */
            assert(false == SOPC_MONOTONIC_CLOCK);
            gettimeResult = clock_gettime(CLOCK_REALTIME, result);
            // Realtime clock shall always be available
            assert(0 == gettimeResult);
        }
    }
    return result;
}

SOPC_TimeReference* SOPC_TimeReference_AddMilliseconds(const SOPC_TimeReference* timeRef, uint64_t ms)
{
    SOPC_TimeReference* result = NULL;
    uint64_t seconds;
    uint64_t nanoseconds;

    if (timeRef != NULL)
    {
        result = calloc(1, sizeof(SOPC_TimeReference));
    }

    if (result != NULL)
    {
        seconds = ms / 1000; // seconds

        if (seconds > (uint64_t) INT64_MAX - (uint64_t) timeRef->tv_sec)
        {
            result->tv_sec = INT64_MAX;
        }
        else
        {
            result->tv_sec = timeRef->tv_sec + (int64_t) seconds; // Add seconds
        }

        // milliseconds rest
        nanoseconds = ms % 1000;
        // milliseconds to nanoseconds
        if (nanoseconds > (uint64_t) INT64_MAX / SOPC_MILLISECONDS_TO_NANOSECONDS)
        {
            nanoseconds = UINT64_MAX; // > INT64_MAX - timeRef->tv_nsec
        }
        else
        {
            nanoseconds = nanoseconds * SOPC_MILLISECONDS_TO_NANOSECONDS;
        }

        if (nanoseconds > (uint64_t) INT64_MAX - (uint64_t) timeRef->tv_nsec)
        {
            result->tv_nsec = INT64_MAX;
        }
        else
        {
            result->tv_nsec = timeRef->tv_nsec + (int64_t) nanoseconds;
        }
    }

    return result;
}

int8_t SOPC_TimeReference_Compare(SOPC_TimeReference* left, SOPC_TimeReference* right)
{
    int8_t result = 0;
    if (NULL == left && NULL == right)
    {
        result = 0;
    }
    else if (NULL == left)
    {
        result = -1;
    }
    else if (NULL == right)
    {
        result = 1;
    }
    else
    {
        if (left->tv_sec == right->tv_sec)
        {
            if (left->tv_nsec == right->tv_nsec)
            {
                result = 0;
            }
            else if (left->tv_nsec < right->tv_nsec)
            {
                result = -1;
            }
            else
            {
                result = 1;
            }
        }
        else if (left->tv_sec < right->tv_sec)
        {
            result = -1;
        }
        else
        {
            result = 1;
        }
    }
    return result;
}

void SOPC_TimeReference_Free(SOPC_TimeReference* tref)
{
    if (NULL != tref)
    {
        free(tref);
    }
}
