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

#include <stdio.h>

#include "sopc_common_constants.h"
#include "sopc_ieee_check.h"

bool SOPC_IEEE_Check(void)
{
    bool bFltRadixStatus = (FLT_RADIX == 2);
    bool bFltRounds = (FLT_ROUNDS == 1);
    bool bStatus = (bFltRadixStatus && bFltRounds);

    if (false == bStatus)
    {
        SOPC_CONSOLE_PRINTF("ERROR: Compiler floating point support is not IEEE-754 compliant\n");
        if (false == bFltRadixStatus)
        {
            SOPC_CONSOLE_PRINTF("Value for FLT_RADIX is : %d instead of 2\n", FLT_RADIX);
        }
        if (false == bFltRounds)
        {
            SOPC_CONSOLE_PRINTF("Value for FLT_ROUNDS is : %d instead of 1\n", FLT_ROUNDS);
        }
    }

    return bStatus;
}
