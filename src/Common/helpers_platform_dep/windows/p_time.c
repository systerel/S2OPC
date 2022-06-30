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

#include "sopc_builtintypes.h"
#include "sopc_time.h"

int64_t SOPC_Time_GetCurrentTimeUTC(void)
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

SOPC_TimeReference SOPC_TimeReference_GetCurrent(void)
{
    /* Extract of GetTickCount64 function documentation:
     *
     * The resolution of the GetTickCount64 function is limited to the resolution of the system timer, which is
     * typically in the range of 10 milliseconds to 16 milliseconds. The resolution of the GetTickCount64 function is
     * not affected by adjustments made by the GetSystemTimeAdjustment function.
     *
     * Note: more precise counter could be used if necessary:
     * https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408%28v=vs.85%29.aspx
     */
    return GetTickCount64();
}

SOPC_ReturnStatus SOPC_Time_Breakdown_Local(time_t t, struct tm* tm)
{
    return (localtime_s(tm, &t) == 0) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

SOPC_ReturnStatus SOPC_Time_Breakdown_UTC(time_t t, struct tm* tm)
{
    return (gmtime_s(tm, &t) == 0) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}
