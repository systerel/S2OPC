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

/** \file
 *
 * \brief Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <inttypes.h>
#include <stdbool.h>

#include "../unit_test_include.h"
#include "opcua_statuscodes.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"

static SOPC_Mutex gmutex;
static SOPC_Condition gcond;

typedef struct CondRes
{
    uint32_t protectedCondition;
    uint32_t waitingThreadStarted;
    uint32_t successCondition;
    uint32_t timeoutCondition;
} CondRes;

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

static void* test_thread_exec_fct(void* args)
{
    SOPC_Atomic_Int_Add((int32_t*) args, 1);
    return NULL;
}

static void* test_thread_mutex_fct(void* args)
{
    SOPC_Atomic_Int_Add((int32_t*) args, 1);
    SOPC_Mutex_Lock(&gmutex);
    SOPC_Atomic_Int_Add((int32_t*) args, 1);
    SOPC_Mutex_Unlock(&gmutex);
    return NULL;
}

static void* test_thread_mutex_recursive_fct(void* args)
{
    SOPC_Atomic_Int_Add((int32_t*) args, 1);
    SOPC_Mutex_Lock(&gmutex);
    SOPC_Mutex_Lock(&gmutex);
    SOPC_Atomic_Int_Add((int32_t*) args, 1);
    SOPC_Mutex_Unlock(&gmutex);
    SOPC_Mutex_Unlock(&gmutex);
    return NULL;
}

static void test_thread_exec(void)
{
    SOPC_Thread thread;
    int32_t cpt = 0;
    // Nominal behavior
    SOPC_ReturnStatus status = SOPC_Thread_Create(&thread, test_thread_exec_fct, &cpt, "test_exec");
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    SOPC_ASSERT(wait_value(&cpt, 1));

    status = SOPC_Thread_Join(&thread);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    // Degraded behavior
    status = SOPC_Thread_Create(NULL, test_thread_exec_fct, &cpt, "test_exec");
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Thread_Create(&thread, NULL, &cpt, "test_exec");
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Thread_Join(&thread);
    SOPC_ASSERT(status == SOPC_STATUS_NOK);
}

static void test_thread_mutex(void)
{
    SOPC_Thread thread;
    int32_t cpt = 0;
    // Nominal behavior
    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_Mutex_Initialization(&gmutex));
    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_Mutex_Lock(&gmutex));
    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_Thread_Create(&thread, test_thread_mutex_fct, &cpt, "test_mutex"));

    // Wait until the thread reaches the "lock mutex" statement
    SOPC_ASSERT(wait_value(&cpt, 1));

    // Wait a bit, this is not really deterministic anyway as the thread could
    // have been put to sleep by the OS...
    SOPC_Sleep(10);
    SOPC_ASSERT(1 == SOPC_Atomic_Int_Get(&cpt));

    // Unlock the mutex and check that the thread can go forward.
    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_Mutex_Unlock(&gmutex));
    SOPC_ASSERT(wait_value(&cpt, 2));

    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_Thread_Join(&thread));
    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_Mutex_Clear(&gmutex));

    // Degraded behavior
    SOPC_ReturnStatus status = SOPC_Mutex_Lock(NULL);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Mutex_Unlock(NULL);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Mutex_Clear(NULL);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
}

static void test_thread_mutex_recursive(void)
{
    SOPC_Thread thread;
    int32_t cpt = 0;
    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_Mutex_Initialization(&gmutex));
    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_Mutex_Lock(&gmutex));
    SOPC_ASSERT(SOPC_STATUS_OK ==
                SOPC_Thread_Create(&thread, test_thread_mutex_recursive_fct, &cpt, "mutex_recursive"));

    // Wait until the thread reaches the "lock mutex" statement
    SOPC_ASSERT(wait_value(&cpt, 1));

    // Wait a bit, this is not really deterministic anyway as the thread could
    SOPC_Sleep(10);
    SOPC_ASSERT(1 == SOPC_Atomic_Int_Get(&cpt));

    // Unlock the mutex and check that the thread can go forward.
    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_Mutex_Unlock(&gmutex));
    SOPC_ASSERT(wait_value(&cpt, 2));

    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_Thread_Join(&thread));
    SOPC_ASSERT(SOPC_STATUS_OK == SOPC_Mutex_Clear(&gmutex));
}

static void* test_thread_condvar_fct(void* args)
{
    CondRes* condRes = (CondRes*) args;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    status = SOPC_Mutex_Lock(&gmutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    condRes->waitingThreadStarted = 1;
    while (condRes->protectedCondition == 0)
    {
        status = SOPC_Mutex_UnlockAndWaitCond(&gcond, &gmutex);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        if (condRes->protectedCondition == 1)
        {
            // Set success
            condRes->successCondition = 1;
        }
    }
    status = SOPC_Mutex_Unlock(&gmutex);
    return NULL;
}

static void* test_thread_condvar_timed_fct(void* args)
{
    CondRes* condRes = (CondRes*) args;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    status = SOPC_Mutex_Lock(&gmutex);
    condRes->waitingThreadStarted = 1;
    status = SOPC_STATUS_NOK;
    while (condRes->protectedCondition == 0 && SOPC_STATUS_TIMEOUT != status)
    {
        status = SOPC_Mutex_UnlockAndTimedWaitCond(&gcond, &gmutex, 1000);
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
    status = SOPC_Mutex_Unlock(&gmutex);
    return NULL;
}

static void test_thread_condvar(void)
{
    SOPC_Thread thread;
    CondRes condRes;
    condRes.protectedCondition = 0;   // FALSE
    condRes.waitingThreadStarted = 0; // FALSE
    condRes.successCondition = 0;     // FALSE
    condRes.timeoutCondition = 0;     // FALSE

    // Nominal behavior (non timed waiting on condition)
    SOPC_ReturnStatus status = SOPC_Mutex_Initialization(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Condition_Init(&gcond);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Create(&thread, test_thread_condvar_fct, &condRes, "Condvar_fct");
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    status = SOPC_Mutex_Lock(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    // Check thread is waiting and mutex is released since we locked it !
    SOPC_ASSERT(condRes.waitingThreadStarted == 1);
    // Trigger the condition now
    condRes.protectedCondition = 1;
    status = SOPC_Mutex_Unlock(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    // Signal condition has changed
    status = SOPC_Condition_SignalAll(&gcond);
    // Wait thread termination
    status = SOPC_Thread_Join(&thread);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    // Check condition status after thread termination
    status = SOPC_Mutex_Lock(&gmutex);
    SOPC_ASSERT(condRes.successCondition == 1);
    status = SOPC_Mutex_Unlock(&gmutex);

    // Clear mutex and Condtion
    status = SOPC_Mutex_Clear(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Condition_Clear(&gcond);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    // Nominal behavior (timed waiting on condition)
    condRes.protectedCondition = 0;   // FALSE
    condRes.waitingThreadStarted = 0; // FALSE
    condRes.successCondition = 0;     // FALSE
    condRes.timeoutCondition = 0;     // FALSE
    status = SOPC_Mutex_Initialization(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Condition_Init(&gcond);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Create(&thread, test_thread_condvar_timed_fct, &condRes, "Condvar_fct");
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    status = SOPC_Mutex_Lock(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    // Check thread is waiting and mutex is released since we locked it !
    SOPC_ASSERT(condRes.waitingThreadStarted == 1);
    // Trigger the condition now
    condRes.protectedCondition = 1;
    status = SOPC_Mutex_Unlock(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    // Signal condition has changed
    status = SOPC_Condition_SignalAll(&gcond);
    // Wait for thread termination
    status = SOPC_Thread_Join(&thread);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    status = SOPC_Mutex_Lock(&gmutex);
    SOPC_ASSERT(condRes.successCondition == 1);
    status = SOPC_Mutex_Unlock(&gmutex);

    // Clear mutex and Condtion
    status = SOPC_Mutex_Clear(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Condition_Clear(&gcond);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    // Degraded behavior (timed waiting on condition)
    condRes.protectedCondition = 0;   // FALSE
    condRes.waitingThreadStarted = 0; // FALSE
    condRes.successCondition = 0;     // FALSE
    condRes.timeoutCondition = 0;     // FALSE
    status = SOPC_Mutex_Initialization(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Condition_Init(&gcond);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Create(&thread, test_thread_condvar_timed_fct, &condRes, "Condvar_fct");
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    status = SOPC_Mutex_Lock(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    // Check thread is waiting and mutex is released since we locked it !
    SOPC_ASSERT(condRes.waitingThreadStarted == 1);
    // DO NOT CHANGE CONDITION
    status = SOPC_Mutex_Unlock(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    // DO NOT SIGNAL CONDITION CHANGE

    // Wait thread termination
    status = SOPC_Thread_Join(&thread);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    // Check timeout on condition occured
    status = SOPC_Mutex_Lock(&gmutex);
    SOPC_ASSERT(condRes.timeoutCondition == 1);
    status = SOPC_Mutex_Unlock(&gmutex);

    // Clear mutex and Condtion
    status = SOPC_Mutex_Clear(&gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Condition_Clear(&gcond);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    // Degraded behavior (invalid parameter)
    status = SOPC_Condition_Init(NULL);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Condition_Clear(NULL);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Condition_SignalAll(NULL);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Mutex_UnlockAndWaitCond(NULL, &gmutex);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Mutex_UnlockAndWaitCond(&gcond, NULL);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Mutex_UnlockAndWaitCond(NULL, NULL);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);

    status = SOPC_Mutex_UnlockAndTimedWaitCond(NULL, &gmutex, 100);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Mutex_UnlockAndTimedWaitCond(&gcond, &gmutex, 0);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Mutex_UnlockAndTimedWaitCond(NULL, &gmutex, 0);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);

    status = SOPC_Mutex_UnlockAndTimedWaitCond(&gcond, NULL, 100);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Mutex_UnlockAndTimedWaitCond(&gcond, &gmutex, 0);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Mutex_UnlockAndTimedWaitCond(&gcond, NULL, 0);

    status = SOPC_Mutex_UnlockAndTimedWaitCond(NULL, NULL, 100);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Mutex_UnlockAndTimedWaitCond(&gcond, NULL, 100);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Mutex_UnlockAndTimedWaitCond(NULL, &gmutex, 100);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);

    status = SOPC_Mutex_UnlockAndTimedWaitCond(NULL, NULL, 0);
    SOPC_ASSERT(status == SOPC_STATUS_INVALID_PARAMETERS);
}

void suite_test_check_threads(int* index)
{
    PRINT("\nTEST: %d check threads\n", *index);
    test_thread_exec();
    PRINT("test 1: ok\n");
    test_thread_mutex();
    PRINT("test 2: ok\n");
    test_thread_mutex_recursive();
    PRINT("test 3: ok\n");
    test_thread_condvar();
    PRINT("test 4: ok\n");
}
