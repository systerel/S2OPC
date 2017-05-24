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


#include <stdlib.h>
#include <check.h>

#include "check_stack.h"

#include "sopc_base_types.h"
#include "sopc_threads.h"
#include "sopc_mutexes.h"

static Mutex gmutex;

void* test_thread_exec_fct(void* args){
    uint32_t* addr_i = (uint32_t*) args;
    while(*addr_i < 100){
        *addr_i = *addr_i + 1;
        SOPC_Sleep(1);
    }
    return NULL;
}

void* test_thread_mutex_fct(void* args){
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
    SOPC_StatusCode status = SOPC_Thread_Create(&thread, test_thread_exec_fct, &cpt);
    ck_assert(status == STATUS_OK);
    SOPC_Sleep(10);
    ck_assert(cpt > 0 && cpt < 100);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == STATUS_OK);
    ck_assert(cpt == 100);

    // Degraded behavior
    status = SOPC_Thread_Create(NULL, test_thread_exec_fct, &cpt);
    ck_assert(status == STATUS_INVALID_PARAMETERS);
    status = SOPC_Thread_Create(&thread, NULL, &cpt);
    ck_assert(status == STATUS_INVALID_PARAMETERS);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == STATUS_NOK);
}
END_TEST

START_TEST(test_thread_mutex)
{
    Thread thread;
    uint32_t cpt = 0;
    // Nominal behavior
    SOPC_StatusCode status = Mutex_Initialization(&gmutex);
    ck_assert(status == STATUS_OK);
    status = Mutex_Lock(&gmutex);
    ck_assert(status == STATUS_OK);
    status = SOPC_Thread_Create(&thread, test_thread_mutex_fct, &cpt);
    ck_assert(status == STATUS_OK);
    SOPC_Sleep(10);
    ck_assert(cpt == 0);
    Mutex_Unlock(&gmutex);
    SOPC_Sleep(10);
    ck_assert(cpt > 0 && cpt < 100);
    status = SOPC_Thread_Join(thread);
    ck_assert(status == STATUS_OK);
    ck_assert(cpt == 100);
    status = Mutex_Clear(&gmutex);
    ck_assert(status == STATUS_OK);

    // Degraded behavior
    status = Mutex_Initialization(NULL);
    ck_assert(status == STATUS_INVALID_PARAMETERS);
    status = Mutex_Lock(NULL);
    ck_assert(status == STATUS_INVALID_PARAMETERS);
    status = Mutex_Unlock(NULL);
    ck_assert(status == STATUS_INVALID_PARAMETERS);
    status = Mutex_Clear(NULL);
    ck_assert(status == STATUS_INVALID_PARAMETERS);
}
END_TEST

Suite *tests_make_suite_threads(void)
{
    Suite *s;
    TCase *tc_thread_mutex;

    s = suite_create("Threads");
    tc_thread_mutex = tcase_create("Thread execution");
    tcase_add_test(tc_thread_mutex, test_thread_exec);
    suite_add_tcase(s, tc_thread_mutex);
    tc_thread_mutex = tcase_create("Threads and mutexes");
    tcase_add_test(tc_thread_mutex, test_thread_mutex);
    suite_add_tcase(s, tc_thread_mutex);

    return s;
}
