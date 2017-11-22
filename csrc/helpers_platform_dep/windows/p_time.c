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
