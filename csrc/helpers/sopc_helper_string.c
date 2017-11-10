/*
 *  Copyright (C) 2016 Systerel and others.
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

#include "sopc_helper_string.h"

#include <ctype.h>

int SOPC_strncmp_ignore_case(const char* s1, const char* s2, size_t size)
{
    int lc1, lc2;
    size_t idx;
    int res = -1000;
    if (NULL != s1 && NULL != s2)
    {
        res = 0;
        for (idx = 0; idx < size && res == 0; idx++)
        {
            lc1 = tolower(s1[idx]);
            lc2 = tolower(s2[idx]);
            if (lc1 < lc2)
            {
                res = -1;
            }
            else if (lc1 > lc2)
            {
                res = +1;
            }
            else if (lc1 == '\0')
            {
                // In case we reached end of both strings, stop comparison here
                return res;
            }
        }
    }
    return res;
}
