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

#include <errno.h>
#include <inttypes.h>
#include <kernel.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
//
//#include "sopc_builtintypes.h"
#include "sopc_enums.h" /* s2opc includes */
//#include "sopc_time.h"
//
///* Private time api */
//
//#define SECOND_TO_100NS ((uint64_t) 10000000)
//
///* Public s2opc api */
//
// int64_t SOPC_Time_GetCurrentTimeUTC()
//{
//    uint64_t wTimeInTicks1 = k_cycle_get_32();
//    int64_t datetime = 0;
//    uint64_t currentTimeInS = 0;
//    uint64_t currentTimeInMS = 0;
//    uint64_t currentTimeFrac100NS = 0;
//    SOPC_ReturnStatus status = SOPC_STATUS_OK;
//
//    currentTimeInMS = (SOPC_TimeReference) k_uptime_get();
//
//    // Check for overflow. Note that currentTimeFrac100NS cannot overflow.
//    // Problem: we don't know the limits of time_t, and they are not #defined.
//    time_t currentTimeT = 0;
//    int64_t limit = 0;
//    switch (sizeof(time_t))
//    {
//    case 4:
//        /* Assumes an int32_t */
//        limit = INT32_MAX;
//        break;
//    case 8:
//        /* Assumes an int64_t */
//        limit = INT64_MAX;
//        break;
//    default:
//        return INT64_MAX;
//    }
//
//    if (currentTimeInS > limit)
//    {
//        return INT64_MAX;
//    }
//
//    currentTimeT = (time_t) currentTimeInS;
//    status = SOPC_Time_FromTimeT(currentTimeT, &datetime);
//    if (SOPC_STATUS_OK != status)
//    {
//        // Time overflow...
//        return INT64_MAX;
//    }
//
//    datetime += currentTimeFrac100NS;
//    return datetime;
//}
//
//// Return current ms since last power on
// SOPC_TimeReference SOPC_TimeReference_GetCurrent()
//{
//    SOPC_TimeReference currentTimeInMs = 0;
//
//    currentTimeInMs = (SOPC_TimeReference) k_uptime_get();
//
//    return currentTimeInMs;
//}
//
// SOPC_ReturnStatus SOPC_Time_Breakdown_Local(time_t t, struct tm* tm)
//{
//    return (localtime_r(&t, tm) == NULL) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
//}
//
// SOPC_ReturnStatus SOPC_Time_Breakdown_UTC(time_t t, struct tm* tm)
//{
//    return (gmtime_r(&t, tm) == NULL) ? SOPC_STATUS_NOK : SOPC_STATUS_OK;
//}

SOPC_ReturnStatus SOPC_Sleep(unsigned int milliseconds)
{
    k_sleep(milliseconds);
    return SOPC_STATUS_OK;
}
