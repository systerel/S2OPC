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
static Condition shell_cond;
static Mutex shell_mutex;
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
    Condition_Init(&shell_cond);
    Mutex_Initialization(&shell_mutex);

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
    Mutex_Lock(&shell_mutex);
    Mutex_UnlockAndWaitCond(&shell_cond, &shell_mutex);
    // transfer shell_command to result
    char* result = shell_command;
    shell_command = NULL;
    Mutex_Unlock(&shell_mutex);
    printk("\n");
    return result;
}

/***************************************************/
static int shell_sopc_exec(const struct shell* shell, size_t argc, char** argv)
{
    Mutex_Lock(&shell_mutex);
    SOPC_Free(shell_command);
    shell_command = cstring_array_to_string(argc, argv);
    SOPC_ASSERT(NULL != shell_command);
    Condition_SignalAll(&shell_cond);
    Mutex_Unlock(&shell_mutex);
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
SOPC_Build_Info SOPC_ClientServer_GetBuildInfo() // TODO : generate automatically
{
    static const SOPC_Build_Info sopc_client_server_build_info = {.buildVersion = SOPC_TOOLKIT_VERSION,
                                                                  .buildSrcCommit = "Not applicable",
                                                                  .buildDockerId = "",
                                                                  .buildBuildDate = ""};

    return sopc_client_server_build_info;
}

/***************************************************/
SOPC_Build_Info SOPC_Common_GetBuildInfo() // TODO : generate automatically
{
    static const SOPC_Build_Info sopc_common_build_info = {.buildVersion = SOPC_TOOLKIT_VERSION,
                                                           .buildSrcCommit = "Unknown_Revision",
                                                           .buildDockerId = "",
                                                           .buildBuildDate = ""};

    return sopc_common_build_info;
}

/***************************************************/
void SOPC_Platform_Target_Debug(void)
{
#if CONFIG_SOPC_HELPER_IMPL_INSTRUM
    // Display stack instrumentation status
    const uint32_t nbSec = k_uptime_get_32() / 1000;
    printk("\n=======\nUptime: t=%d m %d s\n", nbSec / 60, nbSec % 60);
    printk("Nb Allocs: %u\n", SOPC_MemAlloc_Nb_Allocs());

    const SOPC_Thread_Info* pInfos = SOPC_Thread_GetAllThreadsInfo();
    size_t idx = 0;
    while (NULL != pInfos && pInfos->stack_size > 0)
    {
        idx++;
        printk("Thr #%02u (%.08s): %05u / %05u bytes used (%02d%%)\n", idx, pInfos->name, pInfos->stack_usage,
               pInfos->stack_size, (100 * pInfos->stack_usage) / pInfos->stack_size);
        pInfos++;
    }
#else
    printk("Data unavailable. Set flag CONFIG_SOPC_HELPER_IMPL_INSTRUM to 'y' to enable this feature.\n");
#endif
}

/***************************************************/
void main(void)
{
    SOPC_Platform_Main();
}
