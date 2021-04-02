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
#include <stdio.h>
#include <string.h>

#include "MIMXRT1064.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "fsl_debug_console.h"
#include "limits.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"

static char sBuffer[256];
static QueueHandle_t h;
static Mutex m;
static Thread p1 = NULL;
static Thread p2 = NULL;
static Thread p3 = NULL;
static Thread p4 = NULL;
static Thread pX = NULL;

static void* cbS2OPC_Thread_p4(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    unsigned short int cpt = 0;

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 4   signal all well started : current time = %lu\r\n",
            xTaskGetCurrentTaskHandle(), xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    status = Condition_SignalAll(pv);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 1  signal all well started status = %lu : current time = %lu\r\n",
            xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    for (cpt = 0; cpt < 10; cpt++)
    {
        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 4 working : current time = %lu\r\n", xTaskGetCurrentTaskHandle(),
                xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);
        vTaskDelay(100);
    }

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 4 joins ==> Sub task 2  : current time = %lu\r\n",
            xTaskGetCurrentTaskHandle(), xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    status = SOPC_Thread_Join(p2);
    if (status == 0)
    {
        p2 = NULL;
    }

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 4  try to joins Sub task 2 result = %lu : current time = %lu\r\n",
            xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    for (cpt = 0; cpt < 10; cpt++)
    {
        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 4 - 2nd working : current time = %lu\r\n", xTaskGetCurrentTaskHandle(),
                xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);
        vTaskDelay(100);
    }

    return NULL;
}

static void* cbS2OPC_Thread_p3(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    unsigned short int cpt = 0;

    for (cpt = 0; cpt < 10; cpt++)
    {
        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 3 working : current time = %lu\r\n", xTaskGetCurrentTaskHandle(),
                xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);
        vTaskDelay(100);
    }

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 3 try joins ==> Sub task 1  : current time = %lu\r\n",
            xTaskGetCurrentTaskHandle(), xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    status = SOPC_Thread_Join(p1);
    if (status == 0)
    {
        p1 = NULL;
    }

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 3  try to joins Sub task 1 result = %lu : current time = %lu\r\n",
            xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    for (cpt = 0; cpt < 10; cpt++)
    {
        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 3 2nd working : current time = %lu\r\n", xTaskGetCurrentTaskHandle(),
                xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);
        vTaskDelay(100);
    }
    //    Mutex_Lock (&m);
    //    sprintf (sBuffer, "$$$$ %2X -  Sub task 3 try joins ==> Sub task 4  : current time = %lu\r\n",
    //    xTaskGetCurrentTaskHandle (), xTaskGetTickCount ()); PRINTF (sBuffer); Mutex_Unlock (&m);
    //
    //    status = SOPC_Thread_Join (p4);
    //    if (status == 0)
    //    {
    //        p4 = NULL;
    //    }
    //    Mutex_Lock (&m);
    //    sprintf (sBuffer, "$$$$ %2X -  Sub task 3  try to joins Sub task 4 result = %lu : current time = %lu\r\n",
    //    xTaskGetCurrentTaskHandle (), status,
    //             xTaskGetTickCount ());
    //    PRINTF (sBuffer);
    //    Mutex_Unlock (&m);

    return NULL;
}

static void* cbS2OPC_Thread_pX(void* ptr)
{
    return NULL;
}

static void* cbS2OPC_Thread_p2(void* ptr)
{
    unsigned short int cpt = 0;
    Condition* pv = (Condition*) ptr;
    SOPC_ReturnStatus status = SOPC_Thread_Create(&p3, cbS2OPC_Thread_p3, ptr, "Thread_p2");

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task2 creates Sub task 3 created result = %lu : current time = %lu\r\n",
            xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 2 JOIN ON --> Sub task 3 : current time = %lu\r\n",
            xTaskGetCurrentTaskHandle(), xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    status = SOPC_Thread_Join(p3);
    if (status == 0)
    {
        p3 = NULL;
    }

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 2 joins Sub task 3 result = %lu : current time = %lu\r\n",
            xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    for (cpt = 0; cpt < 100; cpt++)
    {
        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 2 2nd working : current time = %lu\r\n", xTaskGetCurrentTaskHandle(),
                xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);
        vTaskDelay(100);
    }

    return NULL;
}

static void* cbS2OPC_Thread_p1(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = ptr;

    for (;;)
    {
        status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_pX, pv, "Thread_p1");
        status = SOPC_Thread_Join(pX);
        pX = NULL;

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  main loop task 1 working : current time = %lu\r\n", xTaskGetCurrentTaskHandle(),
                xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  main loop task 1 working : current time = %lu\r\n", xTaskGetCurrentTaskHandle(),
                xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        // SOPC_Sleep (1000);

        status = SOPC_Thread_Create(&p2, cbS2OPC_Thread_p2, pv, "THREAD_P2");

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 1 creates sub task 2 result = %lu : current time = %lu\r\n",
                xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        Mutex_Lock(&m); // Test condition variable
        {
            status = SOPC_Thread_Create(&p4, cbS2OPC_Thread_p4, pv, "THREAD_P4");

            sprintf(sBuffer, "$$$$ %2X -  Sub task 1 creates sub task 4 result = %lu : current time = %lu\r\n",
                    xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
            PRINTF(sBuffer);

            sprintf(sBuffer,
                    "$$$$ %2X -  Sub task 1 go to wait signal from sub task 4 well started  : current time = %lu\r\n",
                    xTaskGetCurrentTaskHandle(), xTaskGetTickCount());
            PRINTF(sBuffer);

            status = Mutex_UnlockAndWaitCond(pv, &m);

            sprintf(sBuffer, "$$$$ %2X -  Sub task 1 wait signal from sub task 4 result = %lu : current time = %lu\r\n",
                    xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
            PRINTF(sBuffer);
        }
        Mutex_Unlock(&m);

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 1 JOIN ON --> Sub task 2 : current time = %lu\r\n",
                xTaskGetCurrentTaskHandle(), xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        status = SOPC_Thread_Join(p2);
        if (status == 0)
        {
            p2 = NULL;
        }

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 1 joins sub task 2 result = %lu : current time = %lu\r\n",
                xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 1 JOIN ON --> Sub task 4 : current time = %lu\r\n",
                xTaskGetCurrentTaskHandle(), xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        status = SOPC_Thread_Join(p4);
        if (status == 0)
        {
            p4 = NULL;
        }

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 1 joins sub task 4 result = %lu : current time = %lu\r\n",
                xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);
    }

    return NULL;
}

void FREE_RTOS_TEST_API_S2OPC_THREAD(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    Mutex_Initialization(&m);

    status = SOPC_Thread_Create(&p1, cbS2OPC_Thread_p1, pv, "Sub task 1");

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 1 created result = %lu : current time = %lu\r\n",
            xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    status = SOPC_Thread_Join(p1);
    if (status == 0)
    {
        p1 = NULL;
    }
    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 0 try joined main loop result = %lu : current time = %lu\r\n",
            xTaskGetCurrentTaskHandle(), status, xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);
}
