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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_event_handler.h"
#include "sopc_event_timer_manager.h"
#include "sopc_mem_alloc.h"
#include "sopc_sk_ordonnancer.h"

typedef struct SOPC_SKOrdonnancer_Task
{
    SOPC_SKBuilder* builder;
    SOPC_SKProvider* provider;
    SOPC_SKManager* manager;

    bool run;
    uint32_t timerId;
    uint32_t msPeriod;

} SOPC_SKOrdonnancer_Task;

typedef struct SOPC_SKOrdonnancer_DefaultData
{
    bool isInitialized;

    SOPC_Looper* looper;
    SOPC_EventHandler* handler;

    SOPC_SKOrdonnancer_Task task;

} SOPC_SKOrdonnancer_DefaultData;

/*** DEFAULT IMPLEMENTATION FUNCTIONS ***/

static void SOPC_SKOrdonnancer_EventHandler_Callback_Default(SOPC_EventHandler* handler,
                                                             int32_t event,
                                                             uint32_t eltId,
                                                             uintptr_t params,
                                                             uintptr_t auxParam)
{
    // unused variables
    (void) handler;
    (void) event;
    (void) eltId;
    (void) auxParam;

    (void) params;
    SOPC_SKOrdonnancer_Task* task = (SOPC_SKOrdonnancer_Task*) params;
    assert(NULL != task);
    assert(NULL != task->builder || NULL != task->provider || NULL != task->manager);

    SOPC_EventTimer_Cancel(task->timerId);
    task->timerId = 0;

    printf("      => Process Event in SOPC_SKOrdonnancer_EventHandler_Callback_Default\n");

    if (!task->run)
    {
        // SOPC_EventTimer_Cancel(task->timerId);
        // task->timerId = 0;
        return;
    }

    // assert(0 < task->timerId);

    SOPC_SKBuilder_Update(task->builder, task->provider, task->manager);

    /* Get the remaining time to use all available keys */
    uint32_t halfAllKeysLifeTime = SOPC_SKManager_GetAllKeysLifeTime(task->manager) / 2;
    if (halfAllKeysLifeTime < SOPC_SK_ORDONNACER_UPDATE_TIMER_MIN)
    {
        halfAllKeysLifeTime = SOPC_SK_ORDONNACER_UPDATE_TIMER_MIN;
    }
    else if (halfAllKeysLifeTime > SOPC_SK_ORDONNACER_UPDATE_TIMER_MAX)
    {
        halfAllKeysLifeTime = SOPC_SK_ORDONNACER_UPDATE_TIMER_MAX;
    }

    // Replace the timer by a new one with period updated ( half the Keys Life Time )
    if (task->run)
    {
        SOPC_Event periodicEvent = {.event = 0, .eltId = 0, .params = (uintptr_t) task};
        uint32_t timerId = SOPC_EventTimer_CreatePeriodic(handler, periodicEvent, halfAllKeysLifeTime);
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
}

static SOPC_ReturnStatus SOPC_SKOrdonnancer_Initialize_Default(SOPC_SKOrdonnancer* sko)
{
    if (NULL == sko || NULL == sko->data)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SKOrdonnancer_DefaultData* data = (SOPC_SKOrdonnancer_DefaultData*) sko->data;

    if (data->isInitialized)
    {
        return SOPC_STATUS_OK;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    data->looper = NULL;
    data->handler = NULL;

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
        data->handler = SOPC_EventHandler_Create(data->looper, SOPC_SKOrdonnancer_EventHandler_Callback_Default);
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

static SOPC_ReturnStatus SOPC_SKOrdonnancer_AddTask_Default(SOPC_SKOrdonnancer* sko,
                                                            SOPC_SKBuilder* skb,
                                                            SOPC_SKProvider* skp,
                                                            SOPC_SKManager* skm,
                                                            uint32_t msPeriod)
{
    if (NULL == sko || NULL == sko->data || NULL == skb || NULL == skp || NULL == skm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SKOrdonnancer_DefaultData* data = (SOPC_SKOrdonnancer_DefaultData*) sko->data;

    if (NULL != data->task.builder || NULL != data->task.provider || NULL != data->task.manager)
    {
        // only one task accepted by this Ordonnancer
        return SOPC_STATUS_INVALID_STATE;
    }

    data->task.builder = skb;
    data->task.provider = skp;
    data->task.manager = skm;
    data->task.msPeriod = msPeriod;

    return SOPC_STATUS_OK;
}

static SOPC_ReturnStatus SOPC_SKOrdonnancer_Start_Default(SOPC_SKOrdonnancer* sko)
{
    if (NULL == sko || NULL == sko->data)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_SKOrdonnancer_DefaultData* data = (SOPC_SKOrdonnancer_DefaultData*) sko->data;

    if (NULL == data->task.builder || NULL == data->task.provider || NULL == data->task.manager)
    {
        // No task added
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_SKOrdonnancer_Initialize_Default(sko);
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Event event = {.event = 0, .eltId = 0, .params = (uintptr_t) &data->task};
        data->task.timerId = SOPC_EventTimer_CreatePeriodic(data->handler, event, data->task.msPeriod);
        data->task.run = (0 != data->task.timerId);
        if (!data->task.run)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

static void SOPC_SKOrdonnancer_StopAndClear_Default(SOPC_SKOrdonnancer* sko)
{
    if (NULL == sko)
    {
        return;
    }

    SOPC_SKOrdonnancer_DefaultData* data = (SOPC_SKOrdonnancer_DefaultData*) sko->data;

    // Cancel the task
    data->task.run = false;

    // delete looper => delete handler
    SOPC_Looper_Delete(data->looper);

    SOPC_SKBuilder_Clear(data->task.builder);
    SOPC_Free(data->task.builder);
    data->task.builder = NULL;

    SOPC_SKProvider_Clear(data->task.provider);
    SOPC_Free(data->task.provider);
    data->task.provider = NULL;

    SOPC_Free(sko->data);
    sko->data = NULL;
}

/*** API FUNCTIONS ***/

SOPC_SKOrdonnancer* SOPC_SKOrdonnancer_Create()
{
    SOPC_SKOrdonnancer* sko = SOPC_Calloc(1, sizeof(SOPC_SKOrdonnancer));
    if (NULL == sko)
    {
        return NULL;
    }

    sko->data = SOPC_Calloc(1, sizeof(SOPC_SKOrdonnancer_DefaultData));
    if (NULL == sko->data)
    {
        SOPC_Free(sko);
        return NULL;
    }
    SOPC_SKOrdonnancer_DefaultData* data = (SOPC_SKOrdonnancer_DefaultData*) sko->data;
    data->isInitialized = false;
    data->task.builder = NULL;
    data->task.provider = NULL;
    data->task.manager = NULL;
    data->task.msPeriod = 0;

    sko->ptrAddTask = SOPC_SKOrdonnancer_AddTask_Default;
    sko->ptrStart = SOPC_SKOrdonnancer_Start_Default;
    sko->ptrClear = SOPC_SKOrdonnancer_StopAndClear_Default;

    return sko;
}

SOPC_ReturnStatus SOPC_SKOrdonnancer_AddTask(SOPC_SKOrdonnancer* sko,
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

SOPC_ReturnStatus SOPC_SKOrdonnancer_Start(SOPC_SKOrdonnancer* sko)
{
    if (NULL == sko)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return sko->ptrStart(sko);
}

void SOPC_SKOrdonnancer_StopAndClear(SOPC_SKOrdonnancer* sko)
{
    if (NULL == sko || NULL == sko->ptrClear)
    {
        return;
    }
    sko->ptrClear(sko);
}
