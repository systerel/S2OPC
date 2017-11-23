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

    result = currentTime.QuadPart;

    return result;
}

SOPC_TimeReference* SOPC_TimeReference_GetCurrent()
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
    SOPC_TimeReference* result = calloc(1, sizeof(SOPC_TimeReference));
    if (NULL != result)
    {
        *result = GetTickCount64();
    }
    return result;
}

SOPC_TimeReference* SOPC_TimeReference_AddMilliseconds(const SOPC_TimeReference* timeRef, uint64_t ms)
{
    SOPC_TimeReference* result = NULL;
    if (timeRef != NULL)
    {
        result = calloc(1, sizeof(SOPC_TimeReference));
    }

    if (result != NULL)
    {
        if (ms > UINT64_MAX - *timeRef)
        {
            *result = UINT64_MAX;
        }
        else
        {
            *result = *timeRef + ms;
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
        if (*left == *right)
        {
            result = 0;
        }
        else if (*left < *right)
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
