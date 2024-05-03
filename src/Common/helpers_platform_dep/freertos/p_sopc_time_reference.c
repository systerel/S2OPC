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

/* s2opc includes */
#include "p_sopc_common_time.h"
#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_time_reference.h"

/* freertos includes */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define MS_TO_US 1000

/** Definition of SOPC_HighRes_TimeReference */
struct SOPC_HighRes_TimeReference
{
    /* Internal unit is ticks which is user configurable by setting configTICK_RATE_HZ. Note that
     tick frequency cannot be highter than 1 kHz so precision cannot be highter than ms.
     ticksMs represent time from launch in ms */
    uint64_t ticksMs;
};

SOPC_TimeReference SOPC_TimeReference_GetCurrent(void)
{
    uint64_t currentTimeInMs = 0;
    uint64_t wTimeInTicks = 0;

    wTimeInTicks = P_SOPC_COMMON_TIME_get_tick();
    currentTimeInMs = (wTimeInTicks * (uint64_t) 1000) / (uint64_t) configTICK_RATE_HZ;

    return currentTimeInMs;
}

bool SOPC_HighRes_TimeReference_GetTime(SOPC_HighRes_TimeReference* t)
{
    SOPC_ASSERT(NULL != t);
    t->ticksMs = (uint64_t) SOPC_TimeReference_GetCurrent();
    return true;
}

void SOPC_HighRes_TimeReference_AddSynchedDuration(SOPC_HighRes_TimeReference* t,
                                                   uint64_t duration_us,
                                                   int32_t offset_us)
{
    SOPC_UNUSED_ARG(offset_us);
    SOPC_ASSERT(NULL != t);

    t->ticksMs += (uint64_t)(duration_us / (uint64_t) MS_TO_US);
}

int64_t SOPC_HighRes_TimeReference_DeltaUs(const SOPC_HighRes_TimeReference* tRef, const SOPC_HighRes_TimeReference* t)
{
    SOPC_ASSERT(NULL != tRef);
    SOPC_HighRes_TimeReference t1;
    if (NULL == t)
    {
        t1.ticksMs = (uint64_t) SOPC_TimeReference_GetCurrent();
    }
    else
    {
        t1 = *t;
    }

    return (((int64_t) t1.ticksMs) - ((int64_t) tRef->ticksMs)) * MS_TO_US;
}

bool SOPC_HighRes_TimeReference_IsExpired(const SOPC_HighRes_TimeReference* t, const SOPC_HighRes_TimeReference* now)
{
    SOPC_ASSERT(NULL != t);
    SOPC_HighRes_TimeReference t1;

    if (NULL == now)
    {
        t1.ticksMs = (uint64_t) SOPC_TimeReference_GetCurrent();
    }
    else
    {
        t1 = *now;
    }

    /* t <= t1*/
    return t->ticksMs <= t1.ticksMs;
}

bool SOPC_HighRes_TimeReference_SleepUntil(const SOPC_HighRes_TimeReference* date)
{
    SOPC_ASSERT(NULL != date);
    SOPC_HighRes_TimeReference now;

    now.ticksMs = (uint64_t) SOPC_TimeReference_GetCurrent();

    if (now.ticksMs >= date->ticksMs)
    {
        taskYIELD();
    }
    else
    {
        TickType_t timeToWait = (TickType_t)((date->ticksMs - now.ticksMs) * configTICK_RATE_HZ / 1000);

        /** vTaskDelayUntil works with FreeRTOS absolute Ticks. In order to use this API the absolute
         * time must be calculate other FreeRTOS tick.
         */
        TickType_t nowInternalTick = xTaskGetTickCount();

        vTaskDelayUntil(&nowInternalTick, timeToWait);
    }
    return true;
}

SOPC_HighRes_TimeReference* SOPC_HighRes_TimeReference_Create(void)
{
    SOPC_HighRes_TimeReference* ret = SOPC_Calloc(1, sizeof(SOPC_HighRes_TimeReference));
    if (NULL != ret)
    {
        bool ok = SOPC_HighRes_TimeReference_GetTime(ret);
        if (!ok)
        {
            SOPC_HighRes_TimeReference_Delete(&ret);
        }
    }

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

bool SOPC_HighRes_TimeReference_Copy(SOPC_HighRes_TimeReference* to, const SOPC_HighRes_TimeReference* from)
{
    if (NULL == from || NULL == to)
    {
        return false;
    }
    *to = *from;
    return true;
}
