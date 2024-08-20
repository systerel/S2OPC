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

#include <stdbool.h>
#include <stdint.h>
#include <windows.h>

#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_time_reference.h"

#define US_TO_MS (1000)
/** Definition of SOPC_HighRes_TimeReference */
struct SOPC_HighRes_TimeReference
{
    uint64_t ticksMs;
};

/***************************************************/
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

/***************************************************/
void SOPC_HighRes_TimeReference_GetTime(SOPC_HighRes_TimeReference* t)
{
    SOPC_UNUSED_ARG(t);
}

/***************************************************/
int64_t SOPC_HighRes_TimeReference_DeltaUs(const SOPC_HighRes_TimeReference* tRef, const SOPC_HighRes_TimeReference* t)
{
    SOPC_ASSERT(NULL != tRef);
    SOPC_UNUSED_ARG(t);
    return 0; // not implemented in Windows
}

/***************************************************/
bool SOPC_HighRes_TimeReference_IsExpired(const SOPC_HighRes_TimeReference* t, const SOPC_HighRes_TimeReference* now)
{
    SOPC_ASSERT(NULL != t);
    SOPC_UNUSED_ARG(now);
    return false; // not implemented in Windows
}

/***************************************************/
void SOPC_HighRes_TimeReference_AddSynchedDuration(SOPC_HighRes_TimeReference* t,
                                                   uint64_t duration_us,
                                                   int32_t offset_us)
{
    SOPC_UNUSED_ARG(offset_us);
    SOPC_ASSERT(NULL != t);

    t->ticksMs += (uint64_t)(duration_us / (uint64_t) US_TO_MS);
}

/***************************************************/
void SOPC_HighRes_TimeReference_SleepUntil(const SOPC_HighRes_TimeReference* date)
{
    SOPC_UNUSED_ARG(date);
}

/***************************************************/
SOPC_HighRes_TimeReference* SOPC_HighRes_TimeReference_Create(void)
{
    SOPC_HighRes_TimeReference* ret = SOPC_Calloc(1, sizeof(SOPC_HighRes_TimeReference));
    SOPC_HighRes_TimeReference_GetTime(ret);

    return ret;
}

/***************************************************/
void SOPC_HighRes_TimeReference_Delete(SOPC_HighRes_TimeReference** t)
{
    if (NULL == t)
    {
        return;
    }
    SOPC_Free(*t);
    *t = NULL;
}

/***************************************************/
void SOPC_HighRes_TimeReference_Copy(SOPC_HighRes_TimeReference* to, const SOPC_HighRes_TimeReference* from)
{
    if (NULL != from && NULL != to)
    {
        *to = *from;
    }
}
