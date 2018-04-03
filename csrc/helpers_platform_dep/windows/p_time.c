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

#include <stdint.h>
#include <windows.h>

#include "sopc_time.h"

SOPC_DateTime SOPC_Time_GetCurrentTimeUTC()
{
    SOPC_DateTime result = 0;
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

SOPC_TimeReference SOPC_TimeReference_GetCurrent()
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
