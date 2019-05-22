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

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h> /*stdlib*/
#include <string.h>

#include "sopc_enums.h" /*s2opc*/

#include "FreeRTOS.h" /*freeRtos*/
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "p_synchronisation.h" /*synchro*/
#include "p_threads.h"         /*thread*/

/*****Private thread api*****/

static void cbInternalCallback(void* ptr)
{
    tThreadArgs* ptrArgs = (tThreadArgs*) ptr;

    if (ptr != NULL)
    {
        xTaskNotifyWait(ULONG_MAX, ULONG_MAX, NULL, 0); // RAZ notifications

        if (ptrArgs->cbExternalCallback != NULL)
        {
            ptrArgs->cbExternalCallback(ptrArgs->ptrStartArgs);
        }

        if (ptrArgs->pJointure != NULL)
        {
            (void) P_SYNCHRO_SignalAllConditionVariable(ptrArgs->pJointure, JOINTURE_SIGNAL);
        }
    }

    vTaskDelete(NULL);
}

// Non thread safe
void P_THREAD_Destroy(tThreadWks** ptr)
{
    if ((ptr != NULL) && ((*ptr) != NULL))
    {
        if ((*ptr)->args.pJointure != NULL)
        {
            P_SYNCHRO_DestroyConditionVariable(&((*ptr)->args.pJointure));
        }

        if ((*ptr)->lockRecHandle != NULL)
        {
            vQueueDelete((*ptr)->lockRecHandle);
            (*ptr)->lockRecHandle = NULL;
        }

        vPortFree(*ptr);
        *ptr = NULL;
    }
}
// Non thread safe
tThreadWks* P_THREAD_Create(ptrTaskCallback fct, void* args)
{
    tThreadWks* ptrWks = NULL;

    ptrWks = (tThreadWks*) pvPortMalloc(sizeof(tThreadWks));

    if (ptrWks != NULL)
        goto error;
    (void) memset(ptrWks, 0, sizeof(tThreadWks));

    ptrWks->lockRecHandle = xQueueCreateMutex(queueQUEUE_TYPE_MUTEX);

    if (ptrWks->lockRecHandle == NULL)
        goto error;

    ptrWks->args.cbExternalCallback = fct;
    ptrWks->args.ptrStartArgs = args;
    ptrWks->handleTask = 0;
    ptrWks->args.pJointure = P_SYNCHRO_CreateConditionVariable();

    if (ptrWks->args.pJointure == NULL)
        goto error;

    goto success;
error:
    P_THREAD_Destroy(&ptrWks);
success:
    return ptrWks;
}

eThreadResult P_THREAD_Init(tThreadWks* p, unsigned short int wMaxRDV)
{
    eThreadResult resPTHR = E_THREAD_RESULT_OK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;

    if ((p != NULL) && (p->lockRecHandle != NULL))
    {
        xSemaphoreTakeRecursive(p->lockRecHandle, portMAX_DELAY); //******Critical section

        if (p->handleTask == NULL)
        {
            resPSYNC = P_SYNCHRO_InitConditionVariable(p->args.pJointure, wMaxRDV);
            if (resPSYNC == E_COND_VAR_RESULT_OK)
            {
                if (xTaskCreate(cbInternalCallback, "appThread", configMINIMAL_STACK_SIZE, &p->args,
                                configMAX_PRIORITIES - 1, &p->handleTask) != pdPASS)
                {
                    resPTHR = E_THREAD_RESULT_ERROR_NOK;
                }
            }
            else
            {
                resPTHR = E_THREAD_RESULT_ERROR_NOK;
            }
        }
        else
        {
            resPTHR = E_THREAD_RESULT_ERROR_ALREADY_INITIALIZED;
        }

        xSemaphoreGiveRecursive(p->lockRecHandle); //******Leave Critical section
    }

    return resPTHR;
}

eThreadResult P_THREAD_DeInit(tThreadWks* p)
{
    eThreadResult resPTHR = E_THREAD_RESULT_OK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;

    if ((p != NULL) && (p->lockRecHandle != NULL))
    {
        xSemaphoreTakeRecursive(p->lockRecHandle, portMAX_DELAY); //******Critical section

        if (p->handleTask != NULL)
        {
            vTaskDelete(p->handleTask);
            p->handleTask = NULL;
            resPSYNC = P_SYNCHRO_ClearConditionVariable(p->args.pJointure);
            switch (resPSYNC)
            {
            default:
                resPTHR = E_THREAD_RESULT_OK;
                break;
            }
        }
        else
        {
            resPTHR = E_THREAD_RESULT_ERROR_NOT_INITIALIZED;
        }

        xSemaphoreGiveRecursive(p->lockRecHandle); //******Leave Critical section
    }
    return resPTHR;
}

eThreadResult P_THREAD_Join(tThreadWks* p)
{
    eThreadResult result = E_THREAD_RESULT_OK;
    eConditionVariableResult resPSYNC = E_COND_VAR_RESULT_OK;

    if ((p != NULL) && (p->lockRecHandle != NULL))
    {
        xSemaphoreTakeRecursive(p->lockRecHandle, portMAX_DELAY); //******Critical section

        if (p->handleTask != 0)
        {
            resPSYNC = P_SYNCHRO_UnlockAndWaitForConditionVariable(p->args.pJointure, p->lockRecHandle, JOINTURE_SIGNAL,
                                                                   ULONG_MAX);
            switch (resPSYNC)
            {
            case E_COND_VAR_RESULT_OK:
                result = E_THREAD_RESULT_OK;
                break;
            default:
                result = E_THREAD_RESULT_ERROR_NOK;
                break;
            }
        }

        xSemaphoreGiveRecursive(p->lockRecHandle); //******Leave Critical section
    }

    return result;
}

/*****Public s2opc thread api*****/

SOPC_ReturnStatus SOPC_Thread_Create(Thread* thread, void* (*startFct)(void*), void* startArgs)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;

    return status;
}

SOPC_ReturnStatus SOPC_Thread_Join(Thread thread)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    return status;
}

void SOPC_Sleep(unsigned int milliseconds) {}
