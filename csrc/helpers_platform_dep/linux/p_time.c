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

#include <stdint.h>
#include <time.h>

#include "sopc_time.h"

/*
 * It represents the number of seconds between the OPC-UA (Windows) which starts on 1601/01/01 (supposedly 00:00:00
 * UTC), and Linux times starts on epoch, 1970/01/01 00:00:00 UTC.
 * */
static const uint64_t SOPC_SECONDS_BETWEEN_EPOCHS = 11644473600;
static const uint64_t SOPC_SECONDS_TO_100_NANOSECONDS = 10000000; // 10^7

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
