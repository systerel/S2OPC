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

#include <check.h>

#include "check_helpers.h"
#include "opcua_statuscodes.h"

#include "sopc_atomic.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"

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

START_TEST(test_thread_exec)
{
    Thread thread;
    int32_t cpt = 0;
    // Nominal behavior
    SOPC_ReturnStatus status = SOPC_Thread_Create(&thread, test_thread_exec_fct, &cpt, "test_exec");
    ck_assert(status == SOPC_STATUS_OK);

    ck_assert(wait_value(&cpt, 1));

    status = SOPC_Thread_Join(thread);
    ck_assert(status == SOPC_STATUS_OK);

    // Degraded behavior
    status = SOPC_Thread_Create(NULL, test_thread_exec_fct, &cpt, "test_exec");
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Thread_Create(&thread, NULL, &cpt, "test_exec");
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == SOPC_STATUS_NOK);
}
END_TEST

START_TEST(test_thread_mutex)
{
    Thread thread;
    int32_t cpt = 0;
    // Nominal behavior
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Initialization(&gmutex));
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Lock(&gmutex));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Thread_Create(&thread, test_thread_mutex_fct, &cpt, "test_mutex"));

    // Wait until the thread reaches the "lock mutex" statement
    ck_assert(wait_value(&cpt, 1));

    // Wait a bit, this is not really deterministic anyway as the thread could
    // have been put to sleep by the OS...
    SOPC_Sleep(10);
    ck_assert_int_eq(1, SOPC_Atomic_Int_Get(&cpt));

    // Unlock the mutex and check that the thread can go forward.
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Unlock(&gmutex));
    ck_assert(wait_value(&cpt, 2));

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Thread_Join(thread));
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Clear(&gmutex));

    // Degraded behavior
    SOPC_ReturnStatus status = Mutex_Lock(NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_Unlock(NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_Clear(NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

START_TEST(test_thread_mutex_recursive)
{
    Thread thread;
    int32_t cpt = 0;
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Initialization(&gmutex));
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Lock(&gmutex));
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_Thread_Create(&thread, test_thread_mutex_recursive_fct, &cpt, "mutex_recursive"));

    // Wait until the thread reaches the "lock mutex" statement
    ck_assert(wait_value(&cpt, 1));

    // Wait a bit, this is not really deterministic anyway as the thread could
    // have been put to sleep by the OS...
    SOPC_Sleep(10);
    ck_assert_int_eq(1, SOPC_Atomic_Int_Get(&cpt));

    // Unlock the mutex and check that the thread can go forward.
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Unlock(&gmutex));
    ck_assert(wait_value(&cpt, 2));

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Thread_Join(thread));
    ck_assert_int_eq(SOPC_STATUS_OK, Mutex_Clear(&gmutex));
}
END_TEST

static void* test_thread_condvar_fct(void* args)
{
    CondRes* condRes = (CondRes*) args;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    status = Mutex_Lock(&gmutex);
    ck_assert(SOPC_STATUS_OK == status);
    condRes->waitingThreadStarted = 1;
    while (condRes->protectedCondition == 0)
    {
        status = Mutex_UnlockAndWaitCond(&gcond, &gmutex);
        ck_assert(SOPC_STATUS_OK == status);
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

START_TEST(test_thread_condvar)
{
    Thread thread;
    CondRes condRes;
    condRes.protectedCondition = 0;   // FALSE
    condRes.waitingThreadStarted = 0; // FALSE
    condRes.successCondition = 0;     // FALSE
    condRes.timeoutCondition = 0;     // FALSE

    // Nominal behavior (non timed waiting on condition)
    SOPC_ReturnStatus status = Mutex_Initialization(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    status = Condition_Init(&gcond);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Create(&thread, test_thread_condvar_fct, &condRes, "Condvar_fct");
    ck_assert(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    status = Mutex_Lock(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    // Check thread is waiting and mutex is released since we locked it !
    ck_assert(condRes.waitingThreadStarted == 1);
    // Trigger the condition now
    condRes.protectedCondition = 1;
    status = Mutex_Unlock(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    // Signal condition has changed
    status = Condition_SignalAll(&gcond);
    // Wait thread termination
    status = SOPC_Thread_Join(thread);
    ck_assert(status == SOPC_STATUS_OK);
    // Check condition status after thread termination
    status = Mutex_Lock(&gmutex);
    ck_assert(condRes.successCondition == 1);
    status = Mutex_Unlock(&gmutex);

    // Clear mutex and Condtion
    status = Mutex_Clear(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    status = Condition_Clear(&gcond);
    ck_assert(status == SOPC_STATUS_OK);

    // Nominal behavior (timed waiting on condition)
    condRes.protectedCondition = 0;   // FALSE
    condRes.waitingThreadStarted = 0; // FALSE
    condRes.successCondition = 0;     // FALSE
    condRes.timeoutCondition = 0;     // FALSE
    status = Mutex_Initialization(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    status = Condition_Init(&gcond);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Create(&thread, test_thread_condvar_timed_fct, &condRes, "Condvar_fct");
    ck_assert(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    status = Mutex_Lock(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    // Check thread is waiting and mutex is released since we locked it !
    ck_assert(condRes.waitingThreadStarted == 1);
    // Trigger the condition now
    condRes.protectedCondition = 1;
    status = Mutex_Unlock(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    // Signal condition has changed
    status = Condition_SignalAll(&gcond);
    // Wait for thread termination
    status = SOPC_Thread_Join(thread);
    ck_assert(status == SOPC_STATUS_OK);

    status = Mutex_Lock(&gmutex);
    ck_assert(condRes.successCondition == 1);
    status = Mutex_Unlock(&gmutex);

    // Clear mutex and Condtion
    status = Mutex_Clear(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    status = Condition_Clear(&gcond);
    ck_assert(status == SOPC_STATUS_OK);

    // Degraded behavior (timed waiting on condition)
    condRes.protectedCondition = 0;   // FALSE
    condRes.waitingThreadStarted = 0; // FALSE
    condRes.successCondition = 0;     // FALSE
    condRes.timeoutCondition = 0;     // FALSE
    status = Mutex_Initialization(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    status = Condition_Init(&gcond);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Create(&thread, test_thread_condvar_timed_fct, &condRes, "Condvar_fct");
    ck_assert(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    status = Mutex_Lock(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    // Check thread is waiting and mutex is released since we locked it !
    ck_assert(condRes.waitingThreadStarted == 1);
    // DO NOT CHANGE CONDITION
    status = Mutex_Unlock(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    // DO NOT SIGNAL CONDITION CHANGE

    // Wait thread termination
    status = SOPC_Thread_Join(thread);
    ck_assert(status == SOPC_STATUS_OK);

    // Check timeout on condition occured
    status = Mutex_Lock(&gmutex);
    ck_assert(condRes.timeoutCondition == 1);
    status = Mutex_Unlock(&gmutex);

    // Clear mutex and Condtion
    status = Mutex_Clear(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    status = Condition_Clear(&gcond);
    ck_assert(status == SOPC_STATUS_OK);

    // Degraded behavior (invalid parameter)
    status = Condition_Init(NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Condition_Clear(NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Condition_SignalAll(NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndWaitCond(NULL, &gmutex);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndWaitCond(&gcond, NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndWaitCond(NULL, NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);

    status = Mutex_UnlockAndTimedWaitCond(NULL, &gmutex, 100);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(&gcond, &gmutex, 0);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(NULL, &gmutex, 0);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);

    status = Mutex_UnlockAndTimedWaitCond(&gcond, NULL, 100);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(&gcond, &gmutex, 0);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(&gcond, NULL, 0);

    status = Mutex_UnlockAndTimedWaitCond(NULL, NULL, 100);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(&gcond, NULL, 100);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_UnlockAndTimedWaitCond(NULL, &gmutex, 100);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);

    status = Mutex_UnlockAndTimedWaitCond(NULL, NULL, 0);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

Suite* tests_make_suite_threads(void)
{
    Suite* s;
    TCase* tc_thread_mutex;

    s = suite_create("Threads");
    tc_thread_mutex = tcase_create("Thread execution");
    tcase_add_test(tc_thread_mutex, test_thread_exec);
    suite_add_tcase(s, tc_thread_mutex);
    tc_thread_mutex = tcase_create("Threads and mutexes");
    tcase_add_test(tc_thread_mutex, test_thread_mutex);
    tcase_add_test(tc_thread_mutex, test_thread_mutex_recursive);
    suite_add_tcase(s, tc_thread_mutex);
    tc_thread_mutex = tcase_create("Threads and condition variables");
    tcase_add_test(tc_thread_mutex, test_thread_condvar);
    suite_add_tcase(s, tc_thread_mutex);

    return s;
}
