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

/** \file samples_platform_dep.c
 *
 * \brief Provides the implementation for Zephyr of OS-dependant features required for samples
 */

#include "samples_platform_dep.h"
#include "threading_alt.h"
#include "zephyr_network_init.h"

#include <stdint.h>
#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>

#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_toolkit_config.h"

#if !CONFIG_SHELL
#error "This sample requires CONFIG_SHELL to be set in Zephyr project configuration"
#endif

/***************************************************/
/************ LOCAL FUNCTIONS **********************/
/***************************************************/
#define SHELL_CMD_SIZE 64
static char* shell_command = NULL;
static SOPC_Condition shell_cond;
static SOPC_Mutex shell_mutex;
static int shell_sopc_exec(const struct shell* shell, size_t argc, char** argv);
static char* cstring_array_to_string(size_t argc, char** argv);

/***************************************************/
/************ EXTERN FUNCTIONS *********************/
/***************************************************/

/***************************************************/
void SOPC_Platform_Setup(void)
{
    // Setup network
    bool netInit = Network_Initialize(NULL);
    SOPC_ASSERT(netInit == true);

    /* Initialize MbedTLS */
    tls_threading_initialize();

    // Create a Thread to receive SHELL readline
    SOPC_Condition_Init(&shell_cond);
    SOPC_Mutex_Initialization(&shell_mutex);

    printk("+--------------------------------------------------------------\n");
    printk("| S2OPC sample CLI is integrated into ZEPHYR native SHELL     |\n");
    printk("| Call the ZEPHYR shell command 'sopc' to access demo control |\n");
    printk("+--------------------------------------------------------------\n");
}

/* Creating root (level 0) command "sopc" */
SHELL_CMD_REGISTER(sopc, NULL, "S2OPC demo commands", &shell_sopc_exec);

/***************************************************/
/**
 * \param argv The string array
 * \param argc The size of argv
 * \return a new allocated string containing the concatenation of all argv,
 *  separated by a space
 */
static char* cstring_array_to_string(size_t argc, char** argv)
{
    if (argc < 2)
    {
        return SOPC_Calloc(1, 1);
    }

    size_t len = argc; // argc-1 spaces + 1 char for \0
    for (size_t i = 1; i < argc; i++)
    {
        len += strlen(argv[i]);
    }
    char* result = (char*) SOPC_Calloc(len, 1);
    char* tmp = result;
    int nb = sprintf(tmp, "%s", argv[1]);
    tmp += nb;
    for (size_t i = 2; i < argc; i++)
    {
        nb = sprintf(tmp, " %s", argv[i]);
        tmp += nb;
    }

    return result;
}

/***************************************************/
char* SOPC_Shell_ReadLine(void)
{
    SOPC_Mutex_Lock(&shell_mutex);
    SOPC_Mutex_UnlockAndWaitCond(&shell_cond, &shell_mutex);
    // transfer shell_command to result
    char* result = shell_command;
    shell_command = NULL;
    SOPC_Mutex_Unlock(&shell_mutex);
    printk("\n");
    return result;
}

/***************************************************/
static int shell_sopc_exec(const struct shell* shell, size_t argc, char** argv)
{
    SOPC_Mutex_Lock(&shell_mutex);
    SOPC_Free(shell_command);
    shell_command = cstring_array_to_string(argc, argv);
    SOPC_ASSERT(NULL != shell_command);
    SOPC_Condition_SignalAll(&shell_cond);
    SOPC_Mutex_Unlock(&shell_mutex);
    return 0;
}

/***************************************************/
void SOPC_Platform_Shutdown(const bool reboot)
{
    if (reboot)
    {
        printk("\n# Rebooting in 5 seconds...\n\n");
        SOPC_Sleep(5000);
        sys_reboot(SYS_REBOOT_COLD);
    }
}

/***************************************************/
const char* SOPC_Platform_Get_Default_Net_Itf(void)
{
    struct net_if* iface = net_if_get_default();
    if (NULL != iface && NULL != iface->if_dev && NULL != iface->if_dev->dev && NULL != iface->if_dev->dev->name)
    {
        return iface->if_dev->dev->name;
    }
    return "";
}

/***************************************************/
void SOPC_Platform_Target_Debug(const char* param)
{
    SOPC_UNUSED_ARG(param);
    // Display stack instrumentation status
    const uint32_t nbSec = k_uptime_get_32() / 1000;
    printk("\n=======\nUptime: t=%d m %d s\n", nbSec / 60, nbSec % 60);
#ifdef CONFIG_SYS_HEAP_RUNTIME_STATS
    extern struct k_heap _system_heap;

    struct sys_memory_stats stats;
    int res = sys_heap_runtime_stats_get(&_system_heap.heap, &stats);
    if (res == 0)
    {
        const size_t total = stats.free_bytes + stats.allocated_bytes;
        printk("HEAP Statistics: TOTAL = %u Kb, FREE = %u Kb, maxUsed = %u Kb\n", total / 1024, stats.free_bytes / 1024,
               stats.max_allocated_bytes / 1024);
    }
#else
    printk(
        "HEAP Statistics: Data unavailable. Set flag CONFIG_SYS_HEAP_RUNTIME_STATS to 'y' to enable this feature.\n");
#endif
}

/***************************************************/
void main(void)
{
    SOPC_Platform_Main();
}
