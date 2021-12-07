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

#include "sopc_atomic.h"
#include "sopc_builtintypes.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_types.h"
#include "sopc_udp_sockets.h"

#include "p_sopc_log_srv.h"

#include "p_channel.h"

#include "p_ethernet_if.h"

#include "config.h"
#include "pubsub.h"
#include "server.h"

// Note: if SOPC_PUBSCHEDULER_HEARTBEAT_FROM_IRQ is set to 1,
//       SOPC_PUBSCHEDULER_TIME_RESOLUTION_MICROSECS shall be set to exact tick resolution
//       the function SOPC_PubScheduler_HeartBeatFromIRQ will be called
#ifndef SOPC_PUBSCHEDULER_TIME_RESOLUTION_MICROSECS
// Use 50 ms as default resolution
#define SOPC_PUBSCHEDULER_TIME_RESOLUTION_MICROSECS 50000
#endif

static char sBuffer[256];
static Mutex m;
static Thread p1 = NULL;
static Thread p2 = NULL;
static Thread p3 = NULL;
static Thread p4 = NULL;
static Thread pX = NULL;
typedef struct message
{
    Mutex mut;
    Condition cond;
} tMessage;

static tChannel channel;

extern void* cbToolkit_test_server(void* arg);
extern void* cbToolkit_test_client(void* arg);

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
    // static unsigned short int cptKillLogSrvTest = 0; //Todo: Test to verify stop log server
    FILE* fd = NULL;

    SOPC_ReturnStatus status = SOPC_Thread_Create(&p3, cbS2OPC_Thread_p3, ptr, NULL);

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

    for (cpt = 0; cpt < 10; cpt++)
    {
        Mutex_Lock(&m);

        sprintf(sBuffer, "$$$$ %2X -  Sub task 2 2nd working : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());

        // Test to check reentrant newlib functions issue (memory leaks)
        fd = fopen("path", "w");
        fprintf(
            stdout,
            "%s "
            "5555AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBB-"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACCCC-"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADDDD-"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAZZZZ",
            sBuffer);

        fflush(stdout); // Test with flush request, if faild, ZZZZ missing for the last logging
        fclose(fd);

        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        vTaskDelay(100);
    }

    //    cptKillLogSrvTest++;          //Todo: Test to verify stop log server
    //    if (cptKillLogSrvTest > 3)
    //    {
    //        SOPC_LogSrv_Stop();
    //    }

    return NULL;
}

static void* cbS2OPC_Thread_p1(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = ptr;

    SOPC_LogSrv_Start(60, 4023);
    // SOPC_LogSrv_WaitClient(UINT32_MAX); // Todo: Test to verify server start stop memory leaks.
    // SOPC_LogSrv_Stop();
    // SOPC_LogSrv_Start(60, 4023);
    for (;;)
    {
        status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_pX, pv, NULL);
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

        status = SOPC_Thread_Create(&p2, cbS2OPC_Thread_p2, pv, NULL);

        Mutex_Lock(&m);
        sprintf(sBuffer, "$$$$ %2X -  Sub task 1 creates sub task 2 result = %d : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
        PRINTF(sBuffer);
        Mutex_Unlock(&m);

        Mutex_Lock(&m); // Test condition variable
        {
            status = SOPC_Thread_Create(&p4, cbS2OPC_Thread_p4, pv, NULL);

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

    status = SOPC_Thread_Create(&p1, cbS2OPC_Thread_p1, pv, NULL);

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

    status = SOPC_Thread_Create(&pX, cbToolkit_test_server, pv, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Toolkit test thread init launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);
}

void FREE_RTOS_TEST_S2OPC_CLIENT(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    Mutex_Initialization(&m);

    status = SOPC_Thread_Create(&pX, cbToolkit_test_client, pv, NULL);

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

    status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_TestTime, pv, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Toolkit test thread init launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);
}

/*================================CHECK_THREAD======================================*/

static bool wait_value(int32_t* atomic, int32_t val)
{
    for (int i = 0; i < 100; ++i)
    {
        int32_t x = SOPC_Atomic_Int_Get(atomic);

        if (x == val)
        {
            return true;
        }

        SOPC_Sleep(10);
    }

    return false;
}

static Mutex gmutex;
static Condition gcond;

typedef struct CondRes
{
    uint32_t protectedCondition;
    uint32_t waitingThreadStarted;
    uint32_t successCondition;
    uint32_t timeoutCondition;
} CondRes;

static void* test_thread_exec_fct(void* args)
{
    SOPC_Atomic_Int_Add((int32_t*) args, 1);
    return NULL;
}

static void* test_thread_mutex_fct(void* args)
{
    SOPC_Atomic_Int_Add((int32_t*) args, 1);
    Mutex_Lock(&gmutex);
    SOPC_Atomic_Int_Add((int32_t*) args, 1);
    Mutex_Unlock(&gmutex);
    return NULL;
}

static void* test_thread_mutex_recursive_fct(void* args)
{
    SOPC_Atomic_Int_Add((int32_t*) args, 1);
    Mutex_Lock(&gmutex);
    Mutex_Lock(&gmutex);
    SOPC_Atomic_Int_Add((int32_t*) args, 1);
    Mutex_Unlock(&gmutex);
    Mutex_Unlock(&gmutex);
    return NULL;
}

static void* cbS2OPC_Thread_Test_Thread(void* ptr)
{
    Thread thread;
    int32_t cpt = 0;
    // Nominal behavior
    thread = NULL;
    SOPC_ReturnStatus status = SOPC_Thread_Create(&thread, test_thread_exec_fct, &cpt, NULL);
    configASSERT(status == SOPC_STATUS_OK);

    configASSERT(wait_value(&cpt, 1));

    status = SOPC_Thread_Join(thread);
    configASSERT(status == SOPC_STATUS_OK);
    thread = NULL;

    // Degraded behavior
    status = SOPC_Thread_Create(NULL, test_thread_exec_fct, &cpt, NULL);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Thread_Create(&thread, NULL, &cpt, NULL);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Thread_Join(thread);
    configASSERT(status != SOPC_STATUS_OK); // Todo: expected NOK, returned INVALID_STATE
    return NULL;
}

static void* cbS2OPC_Thread_Test_Thread_Mutex(void* ptr)
{
    Thread thread = NULL; // Todo: current implem: handle must be set to NULL before called by create
    int32_t cpt = 0;
    // Nominal behavior
    configASSERT(SOPC_STATUS_OK == Mutex_Initialization(&gmutex));
    configASSERT(SOPC_STATUS_OK == Mutex_Lock(&gmutex));
    configASSERT(SOPC_STATUS_OK == SOPC_Thread_Create(&thread, test_thread_mutex_fct, &cpt, NULL));

    // Wait until the thread reaches the "lock mutex" statement
    configASSERT(wait_value(&cpt, 1));

    // Wait a bit, this is not really deterministic anyway as the thread could
    // have been put to sleep by the OS...
    SOPC_Sleep(10);
    configASSERT(1 == SOPC_Atomic_Int_Get(&cpt));

    // Unlock the mutex and check that the thread can go forward.
    configASSERT(SOPC_STATUS_OK == Mutex_Unlock(&gmutex));
    configASSERT(wait_value(&cpt, 2));

    configASSERT(SOPC_STATUS_OK == SOPC_Thread_Join(thread));
    configASSERT(SOPC_STATUS_OK == Mutex_Clear(&gmutex));

    // Degraded behavior
    SOPC_ReturnStatus status = Mutex_Lock(NULL);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_Unlock(NULL);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_Clear(NULL);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    return NULL;
}

static void* cbS2OPC_Thread_Test_Mutex_Recursive(void* ptr)
{
    Thread thread = NULL; // Todo: current implem: handle must be set to NULL before called by create
    int32_t cpt = 0;
    configASSERT(SOPC_STATUS_OK == Mutex_Initialization(&gmutex));
    configASSERT(SOPC_STATUS_OK == Mutex_Lock(&gmutex));
    configASSERT(SOPC_STATUS_OK == SOPC_Thread_Create(&thread, test_thread_mutex_recursive_fct, &cpt, NULL));

    // Wait until the thread reaches the "lock mutex" statement
    configASSERT(wait_value(&cpt, 1));

    // Wait a bit, this is not really deterministic anyway as the thread could
    // have been put to sleep by the OS...
    SOPC_Sleep(10);
    configASSERT(1 == SOPC_Atomic_Int_Get(&cpt));

    // Unlock the mutex and check that the thread can go forward.
    configASSERT(SOPC_STATUS_OK == Mutex_Unlock(&gmutex));
    configASSERT(wait_value(&cpt, 2));

    configASSERT(SOPC_STATUS_OK == SOPC_Thread_Join(thread));
    configASSERT(SOPC_STATUS_OK == Mutex_Clear(&gmutex));
    return NULL;
}

static void* test_thread_condvar_fct(void* args)
{
    CondRes* condRes = (CondRes*) args;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    status = Mutex_Lock(&gmutex);
    configASSERT(SOPC_STATUS_OK == status);
    condRes->waitingThreadStarted = 1;
    while (condRes->protectedCondition == 0)
    {
        status = Mutex_UnlockAndWaitCond(&gcond, &gmutex);
        configASSERT(SOPC_STATUS_OK == status);
        if (condRes->protectedCondition == 1)
        {
            // Set success
            condRes->successCondition = 1;
        }
    }
    status = Mutex_Unlock(&gmutex);
    return NULL;
}

static void* test_thread_condvar_timed_fct(void* args)
{
    CondRes* condRes = (CondRes*) args;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    status = Mutex_Lock(&gmutex);
    condRes->waitingThreadStarted = 1;
    status = SOPC_STATUS_NOK;
    while (condRes->protectedCondition == 0 && SOPC_STATUS_TIMEOUT != status)
    {
        status = Mutex_UnlockAndTimedWaitCond(&gcond, &gmutex, 1000);
    }
    if (SOPC_STATUS_OK == status && condRes->protectedCondition == 1)
    {
        // Set success on condition
        condRes->successCondition = 1;
    }
    else if (SOPC_STATUS_TIMEOUT == status && condRes->protectedCondition == 0)
    {
        condRes->timeoutCondition = 1;
    }
    status = Mutex_Unlock(&gmutex);
    return NULL;
}

static void* cbS2OPC_Thread_Test_CondVar(void* ptr)
{
    Thread thread = NULL; // Todo: current implem: handle must be set to NULL before called by create
    CondRes condRes;
    condRes.protectedCondition = 0;   // FALSE
    condRes.waitingThreadStarted = 0; // FALSE
    condRes.successCondition = 0;     // FALSE
    condRes.timeoutCondition = 0;     // FALSE

    // Nominal behavior (non timed waiting on condition)
    SOPC_ReturnStatus status = Mutex_Initialization(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    status = Condition_Init(&gcond);
    configASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Create(&thread, test_thread_condvar_fct, &condRes, NULL);
    configASSERT(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    status = Mutex_Lock(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    // Check thread is waiting and mutex is released since we locked it !
    configASSERT(condRes.waitingThreadStarted == 1);
    // Trigger the condition now
    condRes.protectedCondition = 1;
    status = Mutex_Unlock(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    // Signal condition has changed
    status = Condition_SignalAll(&gcond);
    // Wait thread termination
    status = SOPC_Thread_Join(thread);
    configASSERT(status == SOPC_STATUS_OK);
    thread = NULL;
    // Check condition status after thread termination
    status = Mutex_Lock(&gmutex);
    configASSERT(condRes.successCondition == 1);
    status = Mutex_Unlock(&gmutex);

    // Clear mutex and Condtion
    status = Mutex_Clear(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    status = Condition_Clear(&gcond);
    configASSERT(status == SOPC_STATUS_OK);

    // Nominal behavior (timed waiting on condition)
    condRes.protectedCondition = 0;   // FALSE
    condRes.waitingThreadStarted = 0; // FALSE
    condRes.successCondition = 0;     // FALSE
    condRes.timeoutCondition = 0;     // FALSE
    status = Mutex_Initialization(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    status = Condition_Init(&gcond);
    configASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Create(&thread, test_thread_condvar_timed_fct, &condRes, NULL);
    configASSERT(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    status = Mutex_Lock(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    // Check thread is waiting and mutex is released since we locked it !
    configASSERT(condRes.waitingThreadStarted == 1);
    // Trigger the condition now
    condRes.protectedCondition = 1;
    status = Mutex_Unlock(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    // Signal condition has changed
    status = Condition_SignalAll(&gcond);
    // Wait for thread termination
    status = SOPC_Thread_Join(thread);
    thread = NULL;
    configASSERT(status == SOPC_STATUS_OK);

    status = Mutex_Lock(&gmutex);
    configASSERT(condRes.successCondition == 1);
    status = Mutex_Unlock(&gmutex);

    // Clear mutex and Condtion
    status = Mutex_Clear(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    status = Condition_Clear(&gcond);
    configASSERT(status == SOPC_STATUS_OK);

    // Degraded behavior (timed waiting on condition)
    condRes.protectedCondition = 0;   // FALSE
    condRes.waitingThreadStarted = 0; // FALSE
    condRes.successCondition = 0;     // FALSE
    condRes.timeoutCondition = 0;     // FALSE
    status = Mutex_Initialization(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    status = Condition_Init(&gcond);
    configASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Create(&thread, test_thread_condvar_timed_fct, &condRes, NULL);
    configASSERT(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    status = Mutex_Lock(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    // Check thread is waiting and mutex is released since we locked it !
    configASSERT(condRes.waitingThreadStarted == 1);
    // DO NOT CHANGE CONDITION
    status = Mutex_Unlock(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    // DO NOT SIGNAL CONDITION CHANGE

    // Wait thread termination
    status = SOPC_Thread_Join(thread);
    configASSERT(status == SOPC_STATUS_OK);
    thread = NULL;
    // Check timeout on condition occured
    status = Mutex_Lock(&gmutex);
    configASSERT(condRes.timeoutCondition == 1);
    status = Mutex_Unlock(&gmutex);

    // Clear mutex and Condtion
    status = Mutex_Clear(&gmutex);
    configASSERT(status == SOPC_STATUS_OK);
    status = Condition_Clear(&gcond);
    configASSERT(status == SOPC_STATUS_OK);

    // Degraded behavior (invalid parameter)
    status = Condition_Init(NULL);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Condition_Clear(NULL);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Condition_SignalAll(NULL);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    // status = Condition_Init(&gcond); // Todo: Initialized to test gCond valid and other parameters invalid else valid
    // parameters with gCond not initialized returns INVALID STATE

    status = Mutex_UnlockAndWaitCond(NULL, &gmutex);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndWaitCond(&gcond, NULL);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndWaitCond(NULL, NULL);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);

    status = Mutex_UnlockAndTimedWaitCond(NULL, &gmutex, 100);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(&gcond, &gmutex, 0);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(NULL, &gmutex, 0);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);

    status = Mutex_UnlockAndTimedWaitCond(&gcond, NULL, 100);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(&gcond, &gmutex, 0);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(&gcond, NULL, 0);

    status = Mutex_UnlockAndTimedWaitCond(NULL, NULL, 100);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(&gcond, NULL, 100);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(NULL, &gmutex, 100);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);

    status = Mutex_UnlockAndTimedWaitCond(NULL, NULL, 0);
    configASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);

    // status = Condition_Clear(&gcond);
    // configASSERT(status == SOPC_STATUS_OK);

    return NULL;
}

static void* cbS2OPC_Thread_CheckThread(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;

    SOPC_LogSrv_Start(60, 4023);
    SOPC_LogSrv_WaitClient(UINT32_MAX);

    pX = NULL;
    status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_Test_Thread, pv, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X - Test thread launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    printf(sBuffer);
    Mutex_Unlock(&m);

    configASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Join(pX);
    pX = NULL;

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X - Test thread joined result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    printf(sBuffer);
    Mutex_Unlock(&m);

    configASSERT(status == SOPC_STATUS_OK);

    pX = NULL;
    status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_Test_Thread_Mutex, pv, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X - Test thread mutex launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    printf(sBuffer);
    Mutex_Unlock(&m);

    configASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Join(pX);
    pX = NULL;

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X - Test thread mutex joined result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    printf(sBuffer);
    Mutex_Unlock(&m);

    configASSERT(status == SOPC_STATUS_OK);

    pX = NULL;
    status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_Test_Mutex_Recursive, pv, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X - Test thread mutex recursive launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    printf(sBuffer);
    Mutex_Unlock(&m);

    configASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Join(pX);
    pX = NULL;

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X - Test thread mutex recursive joined result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    printf(sBuffer);
    Mutex_Unlock(&m);

    configASSERT(status == SOPC_STATUS_OK);

    pX = NULL;
    status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_Test_CondVar, pv, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X - Test thread cond var launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    printf(sBuffer);
    Mutex_Unlock(&m);

    configASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Join(pX);
    pX = NULL;

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X - Test thread cond var joined result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    printf(sBuffer);
    Mutex_Unlock(&m);

    configASSERT(status == SOPC_STATUS_OK);
    return NULL;
}

void FREE_RTOS_TEST_S2OPC_CHECK_THREAD(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    Mutex_Initialization(&m);

    status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_CheckThread, pv, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Toolkit check thread launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);
}

static void* cbS2OPC_Thread_UDP_Socket_API(void* ptr)
{
    SOPC_Buffer* pBuffer;
    SOPC_Socket_AddressInfo* adresse = NULL;
    Socket sock = -1;
    pBuffer = SOPC_Buffer_Create(256);
    SOPC_Buffer_Write(pBuffer, (const uint8_t*) "Hello world !!!\r\n", (uint32_t) strlen("Hello world !!!\r\n"));
    SOPC_Buffer_SetPosition(pBuffer, 0);
    SOPC_LogSrv_Start(60, 4023);

    P_ETHERNET_IF_IsReady(UINT32_MAX);

    while (1)
    {
        {
            adresse = SOPC_UDP_SocketAddress_Create(false, "224.1.2.3", "4000");
            SOPC_UDP_Socket_CreateToSend(adresse, NULL, true, &sock);
            SOPC_UDP_Socket_Set_MulticastTTL(sock, 255);
            SOPC_UDP_Socket_SendTo(sock, adresse, pBuffer);
            SOPC_UDP_Socket_Close(&sock);
            SOPC_UDP_SocketAddress_Delete(&adresse);

            adresse = SOPC_UDP_SocketAddress_Create(false, "224.1.2.3", "5000");
            SOPC_UDP_Socket_CreateToReceive(adresse, NULL, true, true, &sock);
            SOPC_UDP_Socket_AddMembership(sock, NULL, adresse, NULL);
            SOPC_UDP_Socket_ReceiveFrom(sock, pBuffer);

            if (pBuffer->length > 0)
            {
                pBuffer->data[pBuffer->length < (pBuffer->maximum_size - 2) ? pBuffer->length : pBuffer->length - 1] =
                    '\r';
                pBuffer
                    ->data[pBuffer->length < (pBuffer->maximum_size - 2) ? pBuffer->length + 1 : pBuffer->length - 1] =
                    '\n';
                pBuffer
                    ->data[pBuffer->length < (pBuffer->maximum_size - 2) ? pBuffer->length + 2 : pBuffer->length - 1] =
                    0;

                printf((void*) pBuffer->data);
                fflush(stdout);
            }

            SOPC_UDP_Socket_Close(&sock);
            SOPC_UDP_SocketAddress_Delete(&adresse);
        }
    }

    return NULL;
}

static void* cbS2OPC_Thread_UDP_Socket_API_LB(void* ptr)
{
    SOPC_Buffer* pBuffer;
    SOPC_Socket_AddressInfo* adresseR;
    adresseR = NULL;
    Socket sockR = -1;
    pBuffer = SOPC_Buffer_Create(256);
    SOPC_Buffer_Write(pBuffer, (const uint8_t*) "Hello world !!!\r\n", (uint32_t) strlen("Hello world !!!\r\n"));
    SOPC_Buffer_SetPosition(pBuffer, 0);
    SOPC_LogSrv_Start(60, 4023);

    P_ETHERNET_IF_IsReady(UINT32_MAX);

    adresseR = SOPC_UDP_SocketAddress_Create(false, "232.1.2.101", "4840");
    configASSERT(SOPC_UDP_Socket_CreateToReceive(adresseR, NULL, true, true, &sockR) == SOPC_STATUS_OK);
    configASSERT(SOPC_UDP_Socket_AddMembership(sockR, NULL, adresseR, NULL) == SOPC_STATUS_OK);
    SOPC_SocketSet rdset;
    SOPC_SocketSet wrset;
    SOPC_SocketSet exset;
    SOPC_SocketSet_Clear(&rdset);
    SOPC_SocketSet_Clear(&wrset);
    SOPC_SocketSet_Clear(&exset);
    SOPC_SocketSet_Add(sockR, &rdset);
    SOPC_SocketSet_Add(sockR, &wrset);
    SOPC_SocketSet_Add(sockR, &exset);

    while (1)
    {
        {
            SOPC_SocketSet_Clear(&rdset);
            SOPC_SocketSet_Clear(&wrset);
            SOPC_SocketSet_Clear(&exset);
            SOPC_SocketSet_Add(sockR, &rdset);
            SOPC_SocketSet_Add(sockR, &wrset);
            SOPC_SocketSet_Add(sockR, &exset);

            if (SOPC_Socket_WaitSocketEvents(&rdset, &rdset, &exset, 1000) > 0)
            {
                configASSERT(SOPC_UDP_Socket_ReceiveFrom(sockR, pBuffer) == SOPC_STATUS_OK);

                if (pBuffer->length > 0)
                {
                    pBuffer
                        ->data[pBuffer->length < (pBuffer->maximum_size - 2) ? pBuffer->length : pBuffer->length - 1] =
                        '\r';
                    pBuffer->data[pBuffer->length < (pBuffer->maximum_size - 2) ? pBuffer->length + 1
                                                                                : pBuffer->length - 1] = '\n';
                    pBuffer->data[pBuffer->length < (pBuffer->maximum_size - 2) ? pBuffer->length + 2
                                                                                : pBuffer->length - 1] = 0;

                    printf((void*) pBuffer->data);
                    fflush(stdout);
                }
            }
        }
    }

    return NULL;
}

void FREE_RTOS_TEST_S2OPC_UDP_SOCKET_API(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    Mutex_Initialization(&m);

    status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_UDP_Socket_API, pv, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Toolkit test udp socket api init launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);
}

void FREE_RTOS_TEST_S2OPC_UDP_SOCKET_API_LB(void* ptr)
{
    SOPC_ReturnStatus status;
    Condition* pv = (Condition*) ptr;
    Mutex_Initialization(&m);

    status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_UDP_Socket_API_LB, pv, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Toolkit test udp socket api init launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);
}

static void* cbS2OPC_Thread_PubSub(void* ptr)
{
    SOPC_LogSrv_Start(60, 4023);
    // SOPC_LogSrv_WaitClient(UINT32_MAX);

    P_ETHERNET_IF_IsReady(UINT32_MAX);

    ENDPOINT_URL = DEFAULT_ENDPOINT_URL;

    /* Initialize S2OPC Server */
    SOPC_ReturnStatus status = Server_Initialize();
    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Could not initialize the server.\n");
    }

    /* Configure the Server */
    SOPC_S2OPC_Config s2opcConfig;
    SOPC_S2OPC_Config_Initialize(&s2opcConfig);

    if (SOPC_STATUS_OK == status)
    {
        status = Server_CreateServerConfig(&s2opcConfig);
        if (SOPC_STATUS_OK != status)
        {
            printf("# Error: Could not create the server configuration.\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = Server_LoadAddressSpace();
    }

    /* Configuration of the PubSub module is done upon PubSub start through the local service */

    /* Start the Server */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_ConfigureStartServer(&s2opcConfig.serverConfig.endpoints[0]);
    }

    /* Write in PubSub nodes, which starts the PubSub */
    if (SOPC_STATUS_OK == status)
    {
        // status = Server_WritePubSubNodes();
    }

    /* Wait for a signal */
    while (SOPC_STATUS_OK == status && Server_IsRunning())
    {
        SOPC_Sleep(SLEEP_TIMEOUT);
        if (Server_PubSubStop_Requested())
        {
            PubSub_Stop();
            printf("# Info: PubSub stopped through Start/Stop Command.\n");
        }
        if (Server_PubSubStart_Requested())
        {
            status = PubSub_Configure();
            if (SOPC_STATUS_OK == status)
            {
                printf("# Info: PubSub configured through Start/Stop Command.\n");
            }
            else
            {
                printf("# Warning: Start/Stop Command failed to configure the PubSub module.\n");
            }

            if (SOPC_STATUS_OK == status)
            {
                status = PubSub_Start(SOPC_PUBSCHEDULER_TIME_RESOLUTION_MICROSECS) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
                if (SOPC_STATUS_NOK == status)
                {
                    PubSub_Stop(); // Ensure Pub & Sub are stopped in this case
                }
            }
            if (SOPC_STATUS_OK == status)
            {
                printf("# Info: PubSub started through Start/Stop Command.\n");
            }
            else
            {
                printf("# Warning: Start/Stop Command failed to start the PubSub module.\n");
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            printf("# Warning: waiting for a new configure + start command !\n");
            status = SOPC_STATUS_OK;
        }
    }

    /* Clean and quit */
    PubSub_StopAndClear();
    Server_StopAndClear(&s2opcConfig);
    printf("# Info: Server closed.\n");
    return NULL;
}

void FREE_RTOS_TEST_S2OPC_PUBSUB(void* ptr)
{
    SOPC_ReturnStatus status;
    Mutex_Initialization(&m);

    status = SOPC_Thread_Create(&pX, cbS2OPC_Thread_PubSub, NULL, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  Toolkit test pubsub init launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);
}

static void* cbS2OPC_Thread_prod(void* ptr)
{
    SOPC_ReturnStatus status;
    eChannelResult resChannel;

    tMessage* ctx;
    uint32_t cptLoop = 0;
    char lclBuffer[256];
    SOPC_LogSrv_Start(60, 4023);
    SOPC_LogSrv_WaitClient(UINT32_MAX); // Todo: Test to verify server start stop memory leaks.
    // SOPC_LogSrv_Stop();
    // SOPC_LogSrv_Start(60, 4023);
    for (;;)
    {
        cptLoop += 1;
        if ((cptLoop % 1024) == 0)
        {
            sprintf(lclBuffer, "$$$$ %2X -  PROD - toujours en vie cptLoop = %lu : current time = %lu\r\n",
                    (unsigned int) xTaskGetCurrentTaskHandle(), cptLoop, (uint32_t) xTaskGetTickCount());
            printf(lclBuffer);
        }
        ctx = pvPortMalloc(sizeof(tMessage));
        status = Mutex_Initialization(&ctx->mut);
        configASSERT(status == SOPC_STATUS_OK);
        status = Condition_Init(&ctx->cond);
        configASSERT(status == SOPC_STATUS_OK);

        status = Mutex_Lock(&ctx->mut);
        configASSERT(status == SOPC_STATUS_OK);
#if 1

        sprintf(lclBuffer, "$$$$ %2X -  PROD - Envoi requete %2X : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (unsigned int) ctx, (uint32_t) xTaskGetTickCount());
        printf(lclBuffer);

#endif

        resChannel = P_CHANNEL_Send(&channel, (uint8_t*) &ctx, sizeof(ctx), NULL, 0);
        configASSERT(resChannel == E_CHANNEL_RESULT_OK);

        status = Mutex_UnlockAndWaitCond(&ctx->cond, &ctx->mut);
        configASSERT(status == SOPC_STATUS_OK);

#if 1

        sprintf(lclBuffer, "$$$$ %2X -  PROD - Reponse arrivee : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (uint32_t) xTaskGetTickCount());
        printf(lclBuffer);

#endif

        SOPC_Sleep(1);

        status = Condition_SignalAll(&ctx->cond);
        configASSERT(status == SOPC_STATUS_OK);
        status = Mutex_Unlock(&ctx->mut);
        configASSERT(status == SOPC_STATUS_OK);
    }
    return NULL;
}

static void* cbS2OPC_Thread_cons(void* ptr)
{
    SOPC_ReturnStatus status;
    tMessage* ctx;
    char lclBuffer[256];
    for (;;)
    {
        SOPC_Sleep(1);

        P_CHANNEL_Receive(&channel, NULL, (uint8_t*) &ctx, NULL, sizeof(ctx), portMAX_DELAY, 0);
#if 1

        sprintf(lclBuffer, "$$$$ %2X -  CONS - Reception contexte %2X : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (unsigned int) ctx, (uint32_t) xTaskGetTickCount());
        printf(lclBuffer);

#endif

        status = Mutex_Lock(&ctx->mut);
        configASSERT(status == SOPC_STATUS_OK);
        // Sinal à prod reponse arrivee
        status = Condition_SignalAll(&ctx->cond);
        configASSERT(status == SOPC_STATUS_OK);
        status = Mutex_UnlockAndWaitCond(&ctx->cond, &ctx->mut);
        configASSERT(status == SOPC_STATUS_OK);
#if 1

        sprintf(lclBuffer, "$$$$ %2X -  CONS - Autorisation destruction contexte %2X : current time = %lu\r\n",
                (unsigned int) xTaskGetCurrentTaskHandle(), (unsigned int) ctx, (uint32_t) xTaskGetTickCount());
        printf(lclBuffer);

#endif
        status = Mutex_Unlock(&ctx->mut);
        configASSERT(status == SOPC_STATUS_OK);
        status = Mutex_Clear(&ctx->mut);
        configASSERT(status == SOPC_STATUS_OK);
        status = Condition_Clear(&ctx->cond);
        configASSERT(status == SOPC_STATUS_OK);
        vPortFree(ctx);
        ctx = NULL;
    }
    return NULL;
}

void FREE_RTOS_TEST_S2OPC_USECASE_PUBSUB_SYNCHRO(void* ptr)
{
    SOPC_ReturnStatus status;
    Mutex_Initialization(&m);

    P_CHANNEL_Init(&channel, 16 * sizeof(tMessage), sizeof(tMessage), 16);

    status = SOPC_Thread_Create(&p1, cbS2OPC_Thread_prod, NULL, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  cbS2OPC_Thread_prod init launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);

    status = SOPC_Thread_Create(&p2, cbS2OPC_Thread_cons, NULL, NULL);

    Mutex_Lock(&m);
    sprintf(sBuffer, "$$$$ %2X -  cbS2OPC_Thread_cons init launching result = %d : current time = %lu\r\n",
            (unsigned int) xTaskGetCurrentTaskHandle(), status, (uint32_t) xTaskGetTickCount());
    PRINTF(sBuffer);
    Mutex_Unlock(&m);
}
