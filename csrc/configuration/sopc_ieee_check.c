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

#include <stdio.h>

#include "sopc_ieee_check.h"

bool SOPC_IEEE_Check()
{
    bool bFltRadixStatus = (FLT_RADIX == 2);
    bool bFltRounds = (FLT_ROUNDS == 1);
    bool bStatus = (bFltRadixStatus && bFltRounds);

    if (false == bStatus)
    {
        printf("ERROR: Compiler floating point support is not IEEE-754 compliant\n");
        if (false == bFltRadixStatus)
        {
            printf("Value for FLT_RADIX is : %d instead of 2\n", FLT_RADIX);
        }
        if (false == bFltRounds)
        {
            printf("Value for FLT_ROUNDS is : %d instead of 1\n", FLT_ROUNDS);
        }
    }

    return bStatus;
}
