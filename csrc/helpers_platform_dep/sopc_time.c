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

static char* get_time_string(bool local, bool compact)
{
    static const char* format_seconds_terse = "%Y/%m/%d %H:%M:%S";
    static const char* format_milliseconds_terse = ".%03lu";
    static const char* format_seconds_compact = "%Y%m%d_%H%M%S";
    static const char* format_milliseconds_compact = "_%03lu";
    static const size_t buf_size = 24;

    char* buf = calloc(buf_size, sizeof(char));

    if (buf == NULL)
    {
        return NULL;
    }

    SOPC_DateTime dt = SOPC_Time_GetCurrentTimeUTC();
    assert(dt != 0);

    time_t seconds;
    SOPC_ReturnStatus status = SOPC_DateTime_ToTimeT(dt, &seconds);
    assert(status == SOPC_STATUS_OK);

    time_t milliseconds = (time_t)((dt / 10000) % 1000);
    size_t res = strftime(buf, buf_size - 1, compact ? format_seconds_compact : format_seconds_terse,
                          local ? localtime(&seconds) : gmtime(&seconds));
    assert(res != 0);

    sprintf(buf + 19, compact ? format_milliseconds_compact : format_milliseconds_terse, milliseconds);

    return buf;
}

char* SOPC_Time_GetStringOfCurrentLocalTime(bool compact)
{
    return get_time_string(true, compact);
}

char* SOPC_Time_GetStringOfCurrentTimeUTC(bool compact)
{
    return get_time_string(false, compact);
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
