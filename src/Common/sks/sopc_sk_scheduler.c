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

#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_event_handler.h"
#include "sopc_event_timer_manager.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_sk_scheduler.h"

#ifndef SOPC_SK_SCHEDULER_INITIAL_NB_TASKS
#define SOPC_SK_SCHEDULER_INITIAL_NB_TASKS 10
#endif
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

    SOPC_Array* taskArray; // Array of SOPC_SKscheduler_Task*

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
    SOPC_UNUSED_ARG(auxParam);

    SOPC_SKscheduler_DefaultData* data = (SOPC_SKscheduler_DefaultData*) params;
    SOPC_ReturnStatus status = SOPC_Mutex_Lock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_SKscheduler_Task* task = SOPC_Array_Get(data->taskArray, SOPC_SKscheduler_Task*, eltId);
    SOPC_ASSERT(NULL != task);

    if (!task->run)
    {
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
        SOPC_LooperEvent timerEvent = {.event = 0, .eltId = eltId, .params = (uintptr_t) data};
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

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    SOPC_SKscheduler_Task* newTask = SOPC_Calloc(1, sizeof(SOPC_SKscheduler_Task));
    SOPC_ReturnStatus status = NULL != newTask ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;

    if (SOPC_STATUS_OK == status)
    {
        newTask->builder = skb;
        newTask->provider = skp;
        newTask->manager = skm;
        newTask->msPeriod = msPeriod;
        newTask->run = false;

        // Add the new task to the array
        bool res = SOPC_Array_Append(data->taskArray, newTask);
        if (!res)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    mutStatus = SOPC_Mutex_Unlock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);

    if (SOPC_STATUS_OK != status)
    {
        // Delete the task in case of error
        SOPC_Free(newTask);
    }
    else
    {
        newTask->builder->referencesCounter++;
        newTask->provider->referencesCounter++;
    }

    return status;
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

    SOPC_ReturnStatus status = SOPC_SKscheduler_Initialize_Default(sko);

    size_t nbTasks = SOPC_Array_Size(data->taskArray);
    for (size_t i = 0; SOPC_STATUS_OK == status && i < nbTasks; i++)
    {
        SOPC_SKscheduler_Task* task = SOPC_Array_Get(data->taskArray, SOPC_SKscheduler_Task*, i);
        status = NULL != task ? status : SOPC_STATUS_NOK;
        status = i <= UINT32_MAX ? status : SOPC_STATUS_OUT_OF_MEMORY;
        if (SOPC_STATUS_OK == status)
        {
            SOPC_LooperEvent event = {.event = 0, .eltId = (uint32_t) i, .params = (uintptr_t) data};
            task->timerId = SOPC_EventTimer_Create(data->handler, event, task->msPeriod);
            task->run = (0 != task->timerId);
            if (!task->run)
            {
                status = SOPC_STATUS_NOK;
            }
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

    // Clear the task array
    SOPC_Array_Delete(data->taskArray);

    status = SOPC_Mutex_Unlock(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_Mutex_Clear(&data->mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_Free(sko->data);
    sko->data = NULL;
}

/*** API FUNCTIONS ***/

static void free_task(void* vtask)
{
    SOPC_ASSERT(NULL != vtask);
    SOPC_SKscheduler_Task** ppTask = (SOPC_SKscheduler_Task**) vtask;
    SOPC_ASSERT(NULL != *ppTask);
    SOPC_SKscheduler_Task* task = *ppTask;

    // Cancel associated timer
    SOPC_EventTimer_Cancel(task->timerId);

    SOPC_SKBuilder_MayDelete(&task->builder);

    SOPC_SKProvider_MayDelete(&task->provider);

    SOPC_Free(task);
}

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
    data->taskArray = SOPC_Array_Create(sizeof(SOPC_SKscheduler_Task*), SOPC_SK_SCHEDULER_INITIAL_NB_TASKS, free_task);
    if (NULL == data->taskArray)
    {
        SOPC_Free(sko->data);
        SOPC_Free(sko);
        return NULL;
    }

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
