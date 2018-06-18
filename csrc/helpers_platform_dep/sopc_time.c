/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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

    SOPC_DateTime dt = SOPC_Time_GetCurrentTimeUTC();

    if (dt == 0)
    {
        return NULL;
    }

    time_t seconds;
    SOPC_ReturnStatus status = SOPC_DateTime_ToTimeT(dt, &seconds);
    assert(status == SOPC_STATUS_OK);

    time_t milliseconds = (time_t)((dt / 10000) % 1000);
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

    char* buf = calloc(buf_size, sizeof(char));

    if (buf == NULL)
    {
        return NULL;
    }

    size_t res = strftime(buf, buf_size - 1, compact ? format_seconds_compact : format_seconds_terse, &tm);

    if (res == 0)
    {
        free(buf);
        return NULL;
    }

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
