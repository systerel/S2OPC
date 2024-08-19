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

#include <stdint.h>
#include <windows.h>

#include "sopc_date_time.h"

/***************************************************/
SOPC_DateTime SOPC_Time_GetCurrentTimeUTC(void)
{
    int64_t result = 0;
    FILETIME fileCurrentTime;
    ULARGE_INTEGER currentTime;

    GetSystemTimeAsFileTime(&fileCurrentTime);
    currentTime.LowPart = fileCurrentTime.dwLowDateTime;
    currentTime.HighPart = fileCurrentTime.dwHighDateTime;

    if (currentTime.QuadPart < INT64_MAX)
    {
        result = (int64_t) currentTime.QuadPart;
    }
    else
    {
        result = INT64_MAX;
    }

    return result;
}

/***************************************************/
SOPC_ReturnStatus SOPC_Time_Breakdown_Local(SOPC_Unix_Time t, struct tm* tm)
{
    return (localtime_s(tm, &t) == 0) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

/***************************************************/
SOPC_ReturnStatus SOPC_Time_Breakdown_UTC(SOPC_Unix_Time t, struct tm* tm)
{
    return (gmtime_s(tm, &t) == 0) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}
