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

#include "sopc_services_request_timeout.h"

#include <stddef.h>

/* Reception time of the message currently treated by the Services layer: only accessed from the Services thread */
static bool sopc_hasCurrentMsgReceptionTime = false;
static SOPC_TimeReference sopc_currentMsgReceptionTime = 0;

void SOPC_Services_SetCurrentMsgReceptionTime(SOPC_TimeReference receptionTime)
{
    sopc_currentMsgReceptionTime = receptionTime;
    sopc_hasCurrentMsgReceptionTime = true;
}

void SOPC_Services_ResetCurrentMsgReceptionTime(void)
{
    sopc_currentMsgReceptionTime = 0;
    sopc_hasCurrentMsgReceptionTime = false;
}

bool SOPC_Services_GetCurrentMsgReceptionTime(SOPC_TimeReference* receptionTime)
{
    if (NULL == receptionTime || !sopc_hasCurrentMsgReceptionTime)
    {
        return false;
    }
    *receptionTime = sopc_currentMsgReceptionTime;
    return true;
}

bool SOPC_Services_IsRequestTimeoutHintExpired(SOPC_TimeReference receptionTime,
                                               SOPC_TimeReference currentTime,
                                               uint32_t timeoutHintMs,
                                               uint32_t marginMs)
{
    bool expired = false;
    // TimeoutHint 0 means no timeout. Current time before reception time is not expected (monotonic time).
    if (0 != timeoutHintMs && currentTime >= receptionTime)
    {
        uint64_t elapsed = currentTime - receptionTime;
        // Compare (elapsed + margin > timeoutHint) without overflow risk on the addition
        expired = (elapsed > (uint64_t) timeoutHintMs) || ((uint64_t) timeoutHintMs - elapsed < (uint64_t) marginMs);
    }
    return expired;
}
