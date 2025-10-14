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
#include "sopc_common_constants.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_time_reference.h"

#define SOPC_SECONDS_TO_MICROSECONDS 1000000

static LARGE_INTEGER g_freq;
static bool g_freq_initialized = false;

static void Initialize_QPFrequency(void)
{
    if (!g_freq_initialized)
    {
        QueryPerformanceFrequency(&g_freq);
        g_freq_initialized = true;
    }
}

/** Definition of SOPC_HighRes_TimeReference */
struct SOPC_HighRes_TimeReference
{
    LONGLONG timeUs;
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
    return (SOPC_TimeReference) GetTickCount64();
}

/***************************************************/
void SOPC_HighRes_TimeReference_GetTime(SOPC_HighRes_TimeReference* t)
{
    if (NULL != t)
    {
        Initialize_QPFrequency();
        LARGE_INTEGER tick;
        QueryPerformanceCounter(&tick);
        // We now have the elapsed number of ticks, along with the number of ticks-per-second.
        // We use these values to convert to the number of elapsed microseconds.
        // To guard against loss-of-precision, we convert to microseconds *before* dividing by ticks-per-second.
        LONGLONG tickUs = (tick.QuadPart * SOPC_SECONDS_TO_MICROSECONDS);
        SOPC_ASSERT(tickUs <= MAXLONGLONG && "tickUs greater than MAXLONGLONG");
        t->timeUs = tickUs / g_freq.QuadPart;
    }
}

/***************************************************/
int64_t SOPC_HighRes_TimeReference_DeltaUs(const SOPC_HighRes_TimeReference* tRef, const SOPC_HighRes_TimeReference* t)
{
    SOPC_ASSERT(NULL != tRef);
    SOPC_HighRes_TimeReference t1 = {0};
    if (NULL == t)
    {
        SOPC_HighRes_TimeReference_GetTime(&t1);
    }
    else
    {
        t1 = *t;
    }
    return (int64_t)(t1.timeUs - tRef->timeUs);
}

/***************************************************/
static void SOPC_HighRes_TimeReference_AddDuration(SOPC_HighRes_TimeReference* t, uint64_t duration_us)
{
    SOPC_ASSERT(NULL != t);
    t->timeUs += (LONGLONG) duration_us;
}

/**
 * \warning The offset_us argument is not used for the Windows implementation.
 */
void SOPC_HighRes_TimeReference_AddSynchedDuration(SOPC_HighRes_TimeReference* t,
                                                   uint64_t duration_us,
                                                   int32_t offset_us)
{
    SOPC_UNUSED_ARG(offset_us);
    SOPC_ASSERT(NULL != t);
    SOPC_HighRes_TimeReference_AddDuration(t, duration_us);
}

/***************************************************/
bool SOPC_HighRes_TimeReference_IsExpired(const SOPC_HighRes_TimeReference* t, const SOPC_HighRes_TimeReference* now)
{
    SOPC_ASSERT(NULL != t);
    SOPC_HighRes_TimeReference t1 = {0};
    if (NULL == now)
    {
        SOPC_HighRes_TimeReference_GetTime(&t1);
    }
    else
    {
        t1 = *now;
    }

    return t->timeUs <= t1.timeUs;
}

/***************************************************/
/**
 * \brief Sleep during \p microseconds. Split the waiting time in two:
 *        The millisecond part (passive waiting, not very precise)
 *        The microsecond part (active waiting, very precise)
 *
 * \note TimeSlice > 10ms for non-RT Windows.
 *       I tried WaitableTimer, but I observed the same latency as with Sleep().
 *       In theory, WaitableTimer has a precision of 100 ns (perhaps with a Windows RT specification).
 */
static void SleepUs(uint64_t microseconds)
{
    if (microseconds == 0)
    {
        return;
    }

    Initialize_QPFrequency();
    LARGE_INTEGER start, now;

    // Split waiting
    DWORD sleepMs = (DWORD)(microseconds / 1000);
    LONGLONG remainingUs = (LONGLONG)(microseconds % 1000);

    // Passive sleep
    if (sleepMs > 0)
        Sleep(sleepMs);

    // Active sleep (QPC) for the remaining us
    QueryPerformanceCounter(&start);
    LONGLONG elapsed_us = 0;
    bool timeEnded = false;
    while (!timeEnded)
    {
        QueryPerformanceCounter(&now);
        LONGLONG elapsed_us = (now.QuadPart - start.QuadPart) * SOPC_SECONDS_TO_MICROSECONDS / g_freq.QuadPart;
        if (elapsed_us >= remainingUs)
        {
            timeEnded = true;
        }
    }
}

void SOPC_HighRes_TimeReference_SleepUntil(const SOPC_HighRes_TimeReference* date)
{
    SOPC_ASSERT(NULL != date);
    int64_t TimeUsToWait = SOPC_HighRes_TimeReference_DeltaUs(date, NULL); // NULL => delta with current time
    SleepUs((uint64_t) -TimeUsToWait);
}

/***************************************************/
SOPC_HighRes_TimeReference* SOPC_HighRes_TimeReference_Create(void)
{
    SOPC_HighRes_TimeReference* ret = SOPC_Calloc(1, sizeof(SOPC_HighRes_TimeReference));
    SOPC_HighRes_TimeReference_GetTime(ret);
    return ret;
}

/***************************************************/
void SOPC_HighRes_TimeReference_Copy(SOPC_HighRes_TimeReference* to, const SOPC_HighRes_TimeReference* from)
{
    if (NULL != from && NULL != to)
    {
        *to = *from;
    }
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
