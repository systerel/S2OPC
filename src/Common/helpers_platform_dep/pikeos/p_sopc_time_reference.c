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

#include <p4.h>
#include <stdlib.h>

#include "p_time_c99.h"
#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_time_reference.h"

#define SOPC_MILLISECONDS_TO_NANOSECONDS 1000000
#define SOPC_MICROSECONDS_TO_NANOSECONDS 1000

struct SOPC_HighRes_TimeReference
{
    P4_time_t tp;
};

SOPC_TimeReference SOPC_TimeReference_GetCurrent(void)
{
    return (SOPC_TimeReference)(p4_get_time() /
                                SOPC_MILLISECONDS_TO_NANOSECONDS); /* timeReference has for resolution millisecond */
}

SOPC_HighRes_TimeReference* SOPC_HighRes_TimeReference_Create(void)
{
    SOPC_HighRes_TimeReference* ret = SOPC_Calloc(1, sizeof(SOPC_HighRes_TimeReference));
    SOPC_HighRes_TimeReference_GetTime(ret);

    return ret;
}

void SOPC_HighRes_TimeReference_Delete(SOPC_HighRes_TimeReference** t)
{
    if (NULL == t)
    {
        return;
    }
    SOPC_Free(*t);
    *t = NULL;
}

void SOPC_HighRes_TimeReference_Copy(SOPC_HighRes_TimeReference* to, const SOPC_HighRes_TimeReference* from)
{
    if (NULL != from && NULL != to)
    {
        *to = *from;
    }
}

void SOPC_HighRes_TimeReference_GetTime(SOPC_HighRes_TimeReference* t)
{
    if (t != NULL)
    {
        t->tp = p4_get_time();
    }
}

void SOPC_HighRes_TimeReference_AddSynchedDuration(SOPC_HighRes_TimeReference* t,
                                                   uint64_t duration_us,
                                                   int32_t offset_us)
{
    SOPC_UNUSED_ARG(offset_us);
    SOPC_ASSERT(t != NULL);
    t->tp += (P4_time_t)(duration_us * SOPC_MICROSECONDS_TO_NANOSECONDS);
}

int64_t SOPC_HighRes_TimeReference_DeltaUs(const SOPC_HighRes_TimeReference* tRef, const SOPC_HighRes_TimeReference* t)
{
    SOPC_HighRes_TimeReference t1 = {0};

    if (NULL == t)
    {
        SOPC_HighRes_TimeReference_GetTime(&t1);
    }
    else
    {
        t1.tp = t->tp;
    }

    return (int64_t)((int64_t)(t1.tp - tRef->tp) / SOPC_MICROSECONDS_TO_NANOSECONDS);
}

bool SOPC_HighRes_TimeReference_IsExpired(const SOPC_HighRes_TimeReference* t, const SOPC_HighRes_TimeReference* now)
{
    SOPC_ASSERT(t != NULL);
    if (now == NULL)
    {
        P4_time_t currentTime = p4_get_time();
        return t->tp < currentTime;
    }
    return t->tp < now->tp;
}

void SOPC_HighRes_TimeReference_SleepUntil(const SOPC_HighRes_TimeReference* date)
{
    SOPC_ASSERT(NULL != date);
    static bool warned = false;

    P4_time_t currentTime = p4_get_time();
    int64_t timeToWait = (date->tp - currentTime);
    if (timeToWait < 0)
    {
        p4_thread_yield();
    }
    else
    {
        P4_e_t res = p4_sleep((P4_timeout_t) timeToWait);
        if (P4_E_OK != res && !warned)
        {
            /* TODO: strerror is not thread safe: is it possible to find a thread safe work-around? */
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_COMMON, "p4_sleep failed (warn once): %d", res);
            SOPC_ASSERT(false);
        }
    }
}