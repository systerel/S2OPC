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

#include "unit_test_include.h"

#include "sopc_assert.h"
#include "sopc_enums.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"

static int globalCounter;

static void test_thread_fct(int* counter)
{
    int resultExpected = 12;
    int nbIteration = 10;
    for (int i = 0; i < nbIteration; i++)
    {
        *counter += 1;
    }
    SOPC_ASSERT(*counter == resultExpected);
    PRINT("End of test_thread_fct \n");
}

static void test_thread_fct_mutexes(SOPC_Mutex* pMut)
{
    int expectedResult = 1002;
    SOPC_Mutex_Lock(pMut);

    globalCounter = 2;
    for (int i = 0; i < 1000; i++)
    {
        globalCounter++;
    }

    SOPC_ASSERT(globalCounter == expectedResult);
    SOPC_Mutex_Unlock(pMut);
}

void suite_test_thread_mutexes(int* index)
{
    PRINT("\nTEST %d: threads and mutexes \n", *index);
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_Condition cond;
    status = SOPC_Condition_Init(&cond);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    PRINT("Test1 : ok\n");

    status = SOPC_Condition_SignalAll(&cond);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    PRINT("Test2 : ok\n");

    SOPC_Mutex mutex;
    status = SOPC_Mutex_Initialization(&mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    PRINT("Test3 : ok\n");

    status = SOPC_Mutex_Lock(&mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    PRINT("Test4 : ok\n");

    status = SOPC_Mutex_Unlock(&mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    PRINT("Test5 : ok\n");

    SOPC_Thread p1 = 0;
    int count = 2;
    status = SOPC_Thread_CreatePrioritized(&p1, (void*) test_thread_fct, &count, 40, "Test_Thread1");
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    PRINT("Test6 : ok\n");

    status = SOPC_Thread_Join(p1);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    PRINT("Test7 : ok\n");

    SOPC_Thread p2 = 0;
    status = SOPC_Thread_Join(p2);
    SOPC_ASSERT(status != SOPC_STATUS_OK);
    PRINT("Test8 : ok\n");

    SOPC_Thread p3, p4 = 0;
    int count3 = 2;
    int count4 = 2;
    status = SOPC_Thread_CreatePrioritized(&p3, (void*) test_thread_fct, &count3, 40, "Test_Thread2");
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    status = SOPC_Thread_CreatePrioritized(&p4, (void*) test_thread_fct, &count4, 40, "Test_Thread3");
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_Thread_Join(p3);
    SOPC_Thread_Join(p4);
    PRINT("Test9 : ok\n");

    SOPC_Thread p5, p6;
    SOPC_Mutex mut;
    status = SOPC_Mutex_Initialization(&mut);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    status = SOPC_Thread_CreatePrioritized(&p5, (void*) test_thread_fct_mutexes, &mut, 40, "Test_Thread4");
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    status = SOPC_Thread_CreatePrioritized(&p6, (void*) test_thread_fct_mutexes, &mut, 40, "Test_Thread5");
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_Thread_Join(p5);
    SOPC_Thread_Join(p6);
    PRINT("Test10 : ok\n");
    *index += 1;
}
