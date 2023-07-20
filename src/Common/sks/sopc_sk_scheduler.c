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
#include <string.h>

#include "sopc_assert.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_event_handler.h"
#include "sopc_event_timer_manager.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_sk_scheduler.h"

typedef struct SOPC_SKscheduler_Task
{
    SOPC_SKBuilder* builder;
    SOPC_SKProvider* provider;
    SOPC_SKManager* manager;

    bool run;
    uint32_t timerId;
    uint32_t msPeriod;

} SOPC_SKscheduler_Task;

typedef struct SOPC_SKscheduler_DefaultData
{
    bool isInitialized;

    SOPC_Looper* looper;
    SOPC_EventHandler* handler;

    SOPC_SKscheduler_Task task;

    SOPC_Mutex mutex;

} SOPC_SKscheduler_DefaultData;

/*** DEFAULT IMPLEMENTATION FUNCTIONS ***/

static void SOPC_SKscheduler_EventHandler_Callback_Default(SOPC_EventHandler* handler,
                                                           int32_t event,
                                                           uint32_t eltId,
                                                           uintptr_t params,
                                                           uintptr_t auxParam)
{
    // unused variables
    SOPC_UNUSED_ARG(handler);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(eltId);
    SOPC_UNUSED_ARG(auxParam);

    SOPC_SKscheduler_DefaultData* data = (SOPC_SKscheduler_DefaultData*) params;
    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_SKscheduler_Task* task = &data->task;

    SOPC_ASSERT(NULL != task);
    SOPC_ASSERT(NULL != task->builder || NULL != task->provider || NULL != task->manager);

    if (!task->run)
    {
        status = SOPC_Mutex_Unlock(&data->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        return;
    }

    SOPC_SKBuilder_Update(task->builder, task->provider, task->manager);

    /* Get the remaining time to use all available keys */
    uint32_t halfAllKeysLifeTime = SOPC_SKManager_GetAllKeysLifeTime(task->manager) / 2;
    if (halfAllKeysLifeTime < SOPC_SK_SCHEDULER_UPDATE_TIMER_MIN)
    {
        halfAllKeysLifeTime = SOPC_SK_SCHEDULER_UPDATE_TIMER_MIN;
    }
    else if (halfAllKeysLifeTime > SOPC_SK_SCHEDULER_UPDATE_TIMER_MAX)
    {
        halfAllKeysLifeTime = SOPC_SK_SCHEDULER_UPDATE_TIMER_MAX;
    }

    // Replace the timer by a new one with expiration time updated ( half the Keys Life Time )
    if (task->run)
    {
        SOPC_Event timerEvent = {.event = 0, .eltId = 0, .params = (uintptr_t) data};
        uint32_t timerId = SOPC_EventTimer_Create(handler, timerEvent, halfAllKeysLifeTime);
        if (0 != timerId)
        {
            task->msPeriod = halfAllKeysLifeTime;
            task->timerId = timerId;
        }
        else
        {
            task->run = false;
        }
    }

    status = SOPC_Mutex_Unlock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
}

static SOPC_ReturnStatus SOPC_SKscheduler_Initialize_Default(SOPC_SKscheduler* sko)
{
    if (NULL == sko || NULL == sko->data)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SKscheduler_DefaultData* data = (SOPC_SKscheduler_DefaultData*) sko->data;

    if (data->isInitialized)
    {
        return SOPC_STATUS_OK;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Initialize event timer if not done
    SOPC_EventTimer_Initialize();

    // Create an Event Handler in a dedicated Thread.
    data->looper = SOPC_Looper_Create("Security Keys Looper");
    if (NULL == data->looper)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        data->handler = SOPC_EventHandler_Create(data->looper, SOPC_SKscheduler_EventHandler_Callback_Default);
        if (NULL == data->handler)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        data->isInitialized = true;
    }
    else
    {
        data->isInitialized = false;
        if (NULL != data->looper)
        {
            SOPC_Looper_Delete(data->looper);
            data->looper = NULL;
        }
    }
    return status;
}

static SOPC_ReturnStatus SOPC_SKscheduler_AddTask_Default(SOPC_SKscheduler* sko,
                                                          SOPC_SKBuilder* skb,
                                                          SOPC_SKProvider* skp,
                                                          SOPC_SKManager* skm,
                                                          uint32_t msPeriod)
{
    if (NULL == sko || NULL == sko->data || NULL == skb || NULL == skp || NULL == skm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SKscheduler_DefaultData* data = (SOPC_SKscheduler_DefaultData*) sko->data;

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    if (NULL != data->task.builder || NULL != data->task.provider || NULL != data->task.manager)
    {
        status = SOPC_Mutex_Unlock(&data->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        // only one task accepted by this Scheduler
        return SOPC_STATUS_INVALID_STATE;
    }

    data->task.builder = skb;
    data->task.provider = skp;
    data->task.manager = skm;
    data->task.msPeriod = msPeriod;

    status = SOPC_Mutex_Unlock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus SOPC_SKscheduler_Start_Default(SOPC_SKscheduler* sko)
{
    if (NULL == sko || NULL == sko->data)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SKscheduler_DefaultData* data = (SOPC_SKscheduler_DefaultData*) sko->data;

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (NULL == data->task.builder || NULL == data->task.provider || NULL == data->task.manager)
    {
        mutStatus = SOPC_Mutex_Unlock(&data->mutex);
        SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
        // No task added
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_SKscheduler_Initialize_Default(sko);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Event event = {.event = 0, .eltId = 0, .params = (uintptr_t) data};
        data->task.timerId = SOPC_EventTimer_Create(data->handler, event, data->task.msPeriod);
        data->task.run = (0 != data->task.timerId);
        if (!data->task.run)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    mutStatus = SOPC_Mutex_Unlock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    return status;
}

static void SOPC_SKscheduler_StopAndClear_Default(SOPC_SKscheduler* sko)
{
    if (NULL == sko)
    {
        return;
    }

    SOPC_SKscheduler_DefaultData* data = (SOPC_SKscheduler_DefaultData*) sko->data;

    // delete looper => delete handler
    // OK to execute out of the mutex since it will stop any execution of the looper / handler after poison pill event
    SOPC_Looper_Delete(data->looper);
    data->looper = NULL;
    data->handler = NULL;

    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    // Cancel associated timer
    SOPC_EventTimer_Cancel(data->task.timerId);

    // Cancel the task
    data->task.run = false;

    SOPC_SKBuilder_Clear(data->task.builder);
    SOPC_Free(data->task.builder);
    data->task.builder = NULL;

    SOPC_SKProvider_Clear(data->task.provider);
    SOPC_Free(data->task.provider);
    data->task.provider = NULL;

    status = SOPC_Mutex_Unlock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Mutex_Clear(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Free(sko->data);
    sko->data = NULL;
}

/*** API FUNCTIONS ***/

SOPC_SKscheduler* SOPC_SKscheduler_Create(void)
{
    SOPC_SKscheduler* sko = SOPC_Calloc(1, sizeof(SOPC_SKscheduler));
    if (NULL == sko)
    {
        return NULL;
    }

    sko->data = SOPC_Calloc(1, sizeof(SOPC_SKscheduler_DefaultData));
    if (NULL == sko->data)
    {
        SOPC_Free(sko);
        return NULL;
    }
    SOPC_SKscheduler_DefaultData* data = (SOPC_SKscheduler_DefaultData*) sko->data;
    data->isInitialized = false;
    data->task.builder = NULL;
    data->task.provider = NULL;
    data->task.manager = NULL;
    data->task.msPeriod = 0;

    SOPC_Mutex_Initialization(&data->mutex);

    sko->ptrAddTask = SOPC_SKscheduler_AddTask_Default;
    sko->ptrStart = SOPC_SKscheduler_Start_Default;
    sko->ptrClear = SOPC_SKscheduler_StopAndClear_Default;

    return sko;
}

SOPC_ReturnStatus SOPC_SKscheduler_AddTask(SOPC_SKscheduler* sko,
                                           SOPC_SKBuilder* skb,
                                           SOPC_SKProvider* skp,
                                           SOPC_SKManager* skm,
                                           uint32_t msPeriod)
{
    if (NULL == sko)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return sko->ptrAddTask(sko, skb, skp, skm, msPeriod);
}

SOPC_ReturnStatus SOPC_SKscheduler_Start(SOPC_SKscheduler* sko)
{
    if (NULL == sko)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return sko->ptrStart(sko);
}

void SOPC_SKscheduler_StopAndClear(SOPC_SKscheduler* sko)
{
    if (NULL == sko || NULL == sko->ptrClear)
    {
        return;
    }
    sko->ptrClear(sko);
}
