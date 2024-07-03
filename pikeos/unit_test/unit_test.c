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

/* ------------------------- FILE INCLUSION -------------------------- */
/* PSSW API. */
#include <vm.h>

#include <stddef.h>

#ifdef PIKEOSDEBUG
/* init_gdbstub(), gdb_breakpoint(). */
#include <vm_debug.h>
#endif

#include <stdbool.h>

#include <assert.h>

#include "unit_test_include.h"
#include "sopc_assert.h"

#include <lwipopts.h>

#include <lwip/inet.h>
#include <lwip/sockets.h>

static bool lwip_init(void)
{
    if (0 != _lwip_init())
    {
        vm_cprintf("Failed to initialized lwip \n");
        return false;
    }
    return true;
}

/***************************************************/
static void userAssertCallback(const char* context)
{

    if (context != NULL)
    {
        vm_cprintf("%s\n", context);
    }
    assert(false);
}

extern int main(void)
{
    vm_cprintf("\n ------ Starting Platform Unit Test ------ \n");
    int index = 1;
#ifdef PIKEOSDEBUG
    gdb_breakpoint();
#endif
    bool isLwipInit = false;
    isLwipInit = lwip_init();

    SOPC_Assert_Set_UserCallback(&userAssertCallback);

    suite_test_alloc_memory(&index);

    suite_test_thread_mutexes(&index);
    suite_test_check_threads(&index);

    suite_test_atomic(&index);

    suite_test_time(&index);

    if (isLwipInit)
    {
        suite_test_raw_sockets(&index);
        suite_test_udp_sockets(&index);
        suite_test_check_sockets(&index);
        suite_test_server_client(&index);
        suite_test_publisher_subscriber(&index);
    }

    vm_cprintf("\n ------ END Unit Test ------ \n");
    for (;;)
    {
        p4_sleep(P4_MSEC(1));
    }
}
