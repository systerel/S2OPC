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

/** \file sopc_askpass.c
 *
 * \brief A platform independent API to handle SOPC_TimeReference
 */

#include "sopc_time_reference.h"

#define SOPC_SECOND_TO_MILLISECONDS 1000
#define SOPC_SECOND_TO_NANOSECONDS 1000000000
#define SOPC_MILLISECOND_TO_NANOSECONDS 1000000

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

SOPC_TimeReference SOPC_TimeReference_FromSecondsAndNanoseconds(int64_t seconds, int64_t nanoseconds)
{
    SOPC_TimeReference result = 0;
    uint64_t nanosecs = 0;

    if (seconds >= 0 && UINT64_MAX / SOPC_SECOND_TO_MILLISECONDS > (uint64_t) seconds)
    {
        result = (uint64_t) seconds * SOPC_SECOND_TO_MILLISECONDS; // Add seconds to results

        // Note: 0 is a valid value (e.g. during the first tick of each second with a tick-based clock)
        if (nanoseconds >= 0 && nanoseconds < SOPC_SECOND_TO_NANOSECONDS)
        {
            nanosecs = (uint64_t) nanoseconds;
        }
        else
        {
            nanosecs = SOPC_SECOND_TO_NANOSECONDS - 1; // Use maximum nanoseconds representable (< 1 second)
        }
        // Since nanosecs < 1 second no overflow possible
        result = result + nanosecs / SOPC_MILLISECOND_TO_NANOSECONDS; // Add milliseconds to result
    }
    else
    {
        result = UINT64_MAX; // Use maximum representable value
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
