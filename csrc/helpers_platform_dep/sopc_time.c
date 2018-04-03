/*
 *  Copyright (C) 2018 Systerel and others.
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sopc_time.h"

char* SOPC_Time_GetStringOfCurrentLocalTime(bool compact)
{
    // 19 characters for date with second precision + 4 for milliseconds (.000)
    char* stime = malloc(24 * sizeof(char));
    time_t timer;
    SOPC_DateTime dt = 0;
    uint16_t ms = 0;
    if (NULL != stime)
    {
        timer = time(NULL);
        if (compact == false)
        {
            // Max size = 19 characters for date + '\0' terminating => 20
            strftime(stime, 20, "%Y/%m/%d %H:%M:%S", localtime(&timer)); // => 19 used
            dt = SOPC_Time_GetCurrentTimeUTC();
            if (dt > 0)
            {
                ms = (uint16_t)(dt / 10000) % 1000; // 100 nanosecs => millisecs
            }
            else
            {
                ms = 0;
            }
            // . + 3 digits = 19 + 4 => 23 used
            sprintf(&stime[19], ".%03u", ms);
            // '\0' terminating => 24 used
            stime[23] = '\0';
        }
        else
        {
            // Max size = 15 characters for date + '\0' terminating => 16
            strftime(stime, 16, "%Y%m%d_%H%M%S", localtime(&timer)); // => 15 used
            dt = SOPC_Time_GetCurrentTimeUTC();
            if (dt > 0)
            {
                ms = (uint16_t)(dt / 10000) % 1000; // 100 nanosecs => millisecs
            }
            else
            {
                ms = 0;
            }
            // . + 3 digits = 15 + 4 => 19 used
            sprintf(&stime[15], "_%03u", ms);
            // '\0' terminating => 20 used
            stime[20] = '\0';
        }
    }
    return stime;
}

char* SOPC_Time_GetStringOfCurrentTimeUTC(bool compact)
{
    // 19 characters for date with second precision + 4 for milliseconds (.000)
    char* stime = malloc(24 * sizeof(char));
    time_t timer;
    SOPC_DateTime dt = 0;
    uint16_t ms = 0;
    if (NULL != stime)
    {
        timer = time(NULL);
        if (compact == false)
        {
            // Max size = 19 characters for date + '\0' terminating => 20
            strftime(stime, 20, "%Y/%m/%d %H:%M:%S", gmtime(&timer)); // => 19 used
            dt = SOPC_Time_GetCurrentTimeUTC();
            if (dt > 0)
            {
                ms = (uint16_t)(dt / 10000) % 1000; // 100 nanosecs => millisecs
            }
            else
            {
                ms = 0;
            }
            // . + 3 digits = 19 + 4 => 23 used
            sprintf(&stime[19], ".%03u", ms);
            // '\0' terminating => 24 used
            stime[23] = '\0';
        }
        else
        {
            // Max size = 15 characters for date + '\0' terminating => 16
            strftime(stime, 16, "%Y%m%d_%H%M%S", gmtime(&timer)); // => 15 used
            dt = SOPC_Time_GetCurrentTimeUTC();
            if (dt > 0)
            {
                ms = (uint16_t)(dt / 10000) % 1000; // 100 nanosecs => millisecs
            }
            else
            {
                ms = 0;
            }
            // . + 3 digits = 15 + 4 => 19 used
            sprintf(&stime[15], "_%03u", ms);
            // '\0' terminating => 20 used
            stime[19] = '\0';
        }
    }
    return stime;
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
