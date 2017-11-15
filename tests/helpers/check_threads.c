/*
 * Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 *
 *
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <check.h>
#include <stdlib.h>

#include "check_helpers.h"
#include "opcua_statuscodes.h"

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

void* test_thread_exec_fct(void* args)
{
    uint32_t* addr_i = (uint32_t*) args;
    while (*addr_i < 100)
    {
        *addr_i = *addr_i + 1;
        SOPC_Sleep(1);
    }
    return NULL;
}

void* test_thread_mutex_fct(void* args)
{
    Mutex_Lock(&gmutex);
    test_thread_exec_fct(args);
    Mutex_Unlock(&gmutex);
    return NULL;
}

START_TEST(test_thread_exec)
{
    Thread thread;
    uint32_t cpt = 0;
    // Nominal behavior
    SOPC_ReturnStatus status = SOPC_Thread_Create(&thread, test_thread_exec_fct, &cpt);
    ck_assert(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    ck_assert(cpt > 0 && cpt < 100);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(cpt == 100);

    // Degraded behavior
    status = SOPC_Thread_Create(NULL, test_thread_exec_fct, &cpt);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Thread_Create(&thread, NULL, &cpt);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == SOPC_STATUS_NOK);
}
END_TEST

START_TEST(test_thread_mutex)
{
    Thread thread;
    uint32_t cpt = 0;
    // Nominal behavior
    SOPC_ReturnStatus status = Mutex_Initialization(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    status = Mutex_Lock(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);
    status = SOPC_Thread_Create(&thread, test_thread_mutex_fct, &cpt);
    ck_assert(status == SOPC_STATUS_OK);
    SOPC_Sleep(10);
    ck_assert(cpt == 0);
    Mutex_Unlock(&gmutex);
    SOPC_Sleep(10);
    ck_assert(cpt > 0 && cpt < 100);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == SOPC_STATUS_OK);
    ck_assert(cpt == 100);
    status = Mutex_Clear(&gmutex);
    ck_assert(status == SOPC_STATUS_OK);

    // Degraded behavior
    status = Mutex_Initialization(NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_Lock(NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_Unlock(NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
    status = Mutex_Clear(NULL);
    ck_assert(status == SOPC_STATUS_INVALID_PARAMETERS);
}
END_TEST

void* test_thread_condvar_fct(void* args)
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

void* test_thread_condvar_timed_fct(void* args)
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
    status = SOPC_Thread_Create(&thread, test_thread_condvar_fct, &condRes);
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
    // Check thread successfully terminated on condition
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
    status = SOPC_Thread_Create(&thread, test_thread_condvar_timed_fct, &condRes);
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
    status = SOPC_Thread_Create(&thread, test_thread_condvar_timed_fct, &condRes);
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
    suite_add_tcase(s, tc_thread_mutex);
    tc_thread_mutex = tcase_create("Threads and condition variables");
    tcase_add_test(tc_thread_mutex, test_thread_condvar);
    suite_add_tcase(s, tc_thread_mutex);

    return s;
}
