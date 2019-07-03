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
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "sopc_builtintypes.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_types.h"

#include "p_logsrv.h"

static char sBuffer[256];
static Mutex m;
static Thread p1 = NULL;
static Thread p2 = NULL;
static Thread p3 = NULL;
static Thread p4 = NULL;
static Thread pX = NULL;

extern void* cbToolkit_test_server(void* arg);

static void* cbS2OPC_Thread_p4(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    unsigned short int cpt = 0;

    Mutex_Lock(&m);
    sprintf((void*) sBuffer, "$$$$ %4X -  Sub task 4   signal all well started : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    status = Condition_SignalAll(pv);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 1  signal all well started status = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    for (cpt = 0; cpt < 10; cpt++)
    {
        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 4 working : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);
        vTaskDelay(100);
    }

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 4 joins ==> Sub task 2  : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    status = SOPC_Thread_Join(p2);
    if (status == 0)
    {
        p2 = NULL;
    }

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 4  try to joins Sub task 2 result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    for (cpt = 0; cpt < 10; cpt++)
    {
        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 4 - 2nd working : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);
        vTaskDelay(100);
    }

    return NULL;
}

static void* cbS2OPC_Thread_p3(void* ptr)
{
    SOPC_ReturnStatus status;

    unsigned short int cpt = 0;

    for (cpt = 0; cpt < 10; cpt++)
    {
        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 3 working : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);
        vTaskDelay(100);
    }

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 3 try joins ==> Sub task 1  : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    status = SOPC_Thread_Join(p1);
    if (status == 0)
    {
        p1 = NULL;
    }

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 3  try to joins Sub task 1 result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    for (cpt = 0; cpt < 10; cpt++)
    {
        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 3 2nd working : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);
        vTaskDelay(100);
    }

    return NULL;
}

static void* cbS2OPC_Thread_pX(void* ptr)
{
    return NULL;
}

static void* cbS2OPC_Thread_p2(void* ptr)
{
    unsigned short int cpt = 0;
    FILE* fd = NULL;
    // static unsigned short int cptKillLogSrvTest = 0;
    uint32_t sizeofSOPCNodeID = sizeof(SOPC_NodeId);
    uint32_t sizeofOpcUa_ReferenceNode = sizeof(OpcUa_ReferenceNode);

    SOPC_ReturnStatus status = SOPC_Thread_Create(&p3, cbS2OPC_Thread_p3, ptr);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task2 creates Sub task 3 created result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 2 JOIN ON --> Sub task 3 : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    status = SOPC_Thread_Join(p3);
    if (status == 0)
    {
        p3 = NULL;
    }

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 2 joins Sub task 3 result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    for (cpt = 0; cpt < 100; cpt++)
    {
        Mutex_Lock(&m);

        sprintf(sBuffer, "$$$$ %2X -  Sub task 2 2nd working : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
        // P_LOG_SRV_Print(pLogSrv,sBuffer,strlen(sBuffer));
        // printf(sBuffer,strlen(sBuffer));
        fd = fopen("path", "w");
        fprintf(fd, "%s", sBuffer);
        sprintf(sBuffer, "Sizeof SOPC_NodeId = %lu | Sizeof OpcUa_ReferenceNode = %lu\r\n", sizeofSOPCNodeID,
                sizeofOpcUa_ReferenceNode);
        fprintf(fd, "%s", sBuffer);
        fclose(fd);

        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        vTaskDelay(100);
    }

    /*cptKillLogSrvTest++;
    if(cptKillLogSrvTest > 3)
    {
        if(pLogSrv!=NULL)
        {
            P_LOG_SRV_StopAndDestroy(&pLogSrv);
        }
    }*/

    return NULL;
}

static void* cbS2OPC_Thread_p1(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = ptr;

    for (;;)
    {
        status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_pX, pv);
        status = SOPC_Thread_Join(pX);
        pX = NULL;

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  main loop task 1 working : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  main loop task 1 working : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        // SOPC_Sleep (1000);

        status = SOPC_Thread_Create(&p2, cbS2OPC_Thread_p2, pv);

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 1 creates sub task 2 result = %d : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        Mutex_Lock(&m); // Test condition variable
        {
            status = SOPC_Thread_Create(&p4, cbS2OPC_Thread_p4, pv);

            sprintf(sBuffer, "$$$$ %2X -  Sub task 1 creates sub task 4 result = %d : current time = %lu\r\n",
                    (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
            PRINTF(sBuffer);

            sprintf(sBuffer,
                    "$$$$ %2X -  Sub task 1 go to wait signal from sub task 4 well started  : current time = %lu\r\n",
                    (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
            PRINTF(sBuffer);

            status = Mutex_UnlockAndWaitCond(pv, &m);

            sprintf(sBuffer, "$$$$ %2X -  Sub task 1 wait signal from sub task 4 result = %d : current time = %lu\r\n",
                    (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
            PRINTF(sBuffer);
        }
        Mutex_Unlock(&m);

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 1 JOIN ON --> Sub task 2 : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        status = SOPC_Thread_Join(p2);
        if (status == 0)
        {
            p2 = NULL;
        }

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 1 joins sub task 2 result = %d : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 1 JOIN ON --> Sub task 4 : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        status = SOPC_Thread_Join(p4);
        if (status == 0)
        {
            p4 = NULL;
        }

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 1 joins sub task 4 result = %d : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
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

    status = SOPC_Thread_Create(&p1, cbS2OPC_Thread_p1, pv);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 1 created result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    status = SOPC_Thread_Join(p1);
    if (status == 0)
    {
        p1 = NULL;
    }
    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Sub task 0 try joined main loop result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);
}

void FREE_RTOS_TEST_S2OPC_SERVER(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    Mutex_Initialization(&m);

    status = SOPC_Thread_Create(&pX, cbToolkit_test_server, pv);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Toolkit test thread init launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);
}

/*================================CHECK_TIME======================================*/

static const int64_t UNIX_EPOCH_01012017_SECS = 1483228800;
static const int64_t UNIX_EPOCH_01012020_SECS = 1577836800;
/*
 * It represents the number of seconds between the OPC-UA (Windows) which starts on 1601/01/01 (supposedly 00:00:00
 * UTC), and Linux times starts on epoch, 1970/01/01 00:00:00 UTC.
 * */
static const int64_t SOPC_SECONDS_BETWEEN_EPOCHS = 11644473600;
static const int64_t SOPC_SECONDS_TO_100_NANOSECONDS = 10000000; // 10^7

static void* cbS2OPC_Thread_TestTime(void* ptr)
{
    int64_t tdate = SOPC_Time_GetCurrentTimeUTC();
    int64_t tdate2;
    uint64_t elapsedMs = 0;
    int8_t compareResult = 0;
    SOPC_TimeReference tref = 0;
    SOPC_TimeReference tref2 = 0;

    int64_t vDate = SOPC_Time_GetCurrentTimeUTC();

    configASSERT(vDate != 0);
    configASSERT(vDate > (UNIX_EPOCH_01012017_SECS + SOPC_SECONDS_BETWEEN_EPOCHS) * SOPC_SECONDS_TO_100_NANOSECONDS);
    configASSERT(vDate < (UNIX_EPOCH_01012020_SECS + SOPC_SECONDS_BETWEEN_EPOCHS) * SOPC_SECONDS_TO_100_NANOSECONDS);

    tref = SOPC_TimeReference_GetCurrent();
    /* Test SOPC_TimeReference_Compare */
    // tref == tref
    compareResult = SOPC_TimeReference_Compare(tref, tref);
    configASSERT(compareResult == 0);

    // (tref2 > tref)
    SOPC_Sleep(20); // Sleep 20 ms to ensure time is different (windows implementation less precise)
    tref2 = SOPC_TimeReference_GetCurrent();

    compareResult = SOPC_TimeReference_Compare(tref2, tref);
    configASSERT(compareResult == 1);

    compareResult = SOPC_TimeReference_Compare(tref, tref2);
    configASSERT(compareResult == -1);

    /* Test SOPC_TimeReference_AddMilliseconds */

    // Nominal case (compute elpase time)
    tref = SOPC_TimeReference_GetCurrent();
    tref2 = SOPC_TimeReference_AddMilliseconds(tref, 1000);

    // Wait 1000 ms elapsed (target time reference <= current time reference)
    while (SOPC_TimeReference_Compare(tref2, tref) > 0)
    {
        SOPC_Sleep(50);
        tref = SOPC_TimeReference_GetCurrent();
    }
    tdate2 = SOPC_Time_GetCurrentTimeUTC();

    configASSERT(tdate >= 0 && tdate2 >= 0 && tdate2 >= tdate);
    elapsedMs = ((uint64_t) tdate2 - (uint64_t) tdate) / 10000; // 100 nanoseconds to milliseconds

    // Check computed elapsed time value (on non monotonic clock) is 1000ms +/- 100ms
    configASSERT(1000 - 100 < elapsedMs && elapsedMs < 1000 + 100);

    return NULL;
}

void FREE_RTOS_TEST_S2OPC_TIME(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    Mutex_Initialization(&m);

    status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_TestTime, pv);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Toolkit test thread init launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);
}
