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

#include "sopc_date_time.h"
#include "time.h"

/* This implementation is not POSIX compliant.
 * This time functions return seconds since the Epoch plus UTC offset from building machine.
 * This function is only used to verify x509 certificate date validity. Accordingly to mbedtls 2.28.1 documentation
 * it is is not necessary to have a precise clock only the date is usefull for this verification */
time_t time(time_t* result)
{
    time_t t = -1;
    SOPC_ReturnStatus status = SOPC_Time_ToUnixTime(SOPC_Time_GetCurrentTimeUTC(), &t);
    if (SOPC_STATUS_OK != status)
    {
        t = -1;
    }
    if (NULL != result)
    {
        *result = t;
    }
    return t;
}
