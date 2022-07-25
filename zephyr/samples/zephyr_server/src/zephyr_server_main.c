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

#include <assert.h>
#include <kernel.h>
#include <shell/shell.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

#include "sopc_address_space.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_toolkit_config.h"

#include "network_init.h"
#include "server.h"
#include "threading_alt.h"

// Global test variables
static SOPC_S2OPC_Config s2opcConfig;

static void log_UserCallback(const char* context, const char* text);
static int cmd_demo_info(const struct shell* shell, size_t argc, char** argv);

/***************************************************/
SOPC_Build_Info SOPC_ClientServer_GetBuildInfo()
{
    static const SOPC_Build_Info sopc_client_server_build_info = {.buildVersion = SOPC_TOOLKIT_VERSION,
                                                                  .buildSrcCommit = "Not applicable",
                                                                  .buildDockerId = "",
                                                                  .buildBuildDate = ""};

    return sopc_client_server_build_info;
}

/***************************************************/
SOPC_Build_Info SOPC_Common_GetBuildInfo()
{
    static const SOPC_Build_Info sopc_common_build_info = {.buildVersion = SOPC_TOOLKIT_VERSION,
                                                           .buildSrcCommit = "Unknown_Revision",
                                                           .buildDockerId = "",
                                                           .buildBuildDate = ""};

    return sopc_common_build_info;
}

/*-----------------------
 * Logger configuration :
 *-----------------------*/

/***************************************************/
static void log_UserCallback(const char* context, const char* text)
{
    (void) context;
    if (NULL != text)
    {
        printk("%s\n", text);
        if (strlen(text) > 80)
        {
            printf("%s\n", text + 80);
        }
    }
}
/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    printk("\nBUILD DATE : " __DATE__ " " __TIME__ "\n");
    // Note: avoid unused parameter warning from compiler
    (void) argc;
    (void) argv;

    /* Configure the server logger:
     * DEBUG traces generated in ./<argv[0]_logs/ */
    SOPC_Log_Configuration logConfig;
    logConfig.logSystem = SOPC_LOG_SYSTEM_USER;
    logConfig.logLevel = SOPC_LOG_LEVEL_WARNING;
    logConfig.logSysConfig.userSystemLogConfig.doLog = &log_UserCallback;

    bool netInit = Network_Initialize(NULL);
    assert(netInit == true);

    /* Initialize MbedTLS */
    tls_threading_initialize();

    SOPC_ReturnStatus status = Server_Initialize(&logConfig);
    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Could not initialize the server.\n");
        return -1;
    }

    /* Configure the Server */
    SOPC_S2OPC_Config_Initialize(&s2opcConfig);

    status = Server_CreateServerConfig(&s2opcConfig);
    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Could not create the server configuration.\n");
        return -1;
    }

    // The demo configuration is supposed to have only one endpoint
    assert(s2opcConfig.serverConfig.nbEndpoints == 1);
    SOPC_Endpoint_Config* epConfig = &s2opcConfig.serverConfig.endpoints[0];
    printf("# Initializing server at EP: %s\n", epConfig->endpointURL);

    status = Server_LoadAddressSpace();
    if (SOPC_STATUS_OK != status)
    {
        printf("# Error: Could not load address space\n");
        return -1;
    }
    printf("# Address space loaded\n");

    status = Server_ConfigureStartServer(epConfig);
    if (SOPC_STATUS_OK == status)
    {
        printf("# Server started on <%s>\n", epConfig->endpointURL);
    }

    while (SOPC_STATUS_OK == status && Server_IsRunning())
    {
        k_sleep(K_MSEC(50));
    }

    Server_StopAndClear(&s2opcConfig);
    printf("# Info: Server closed.\n");
    return 0;
}

/*---------------------------------------------------------------------------
 *                             NET SHELL CONFIGURATION
 *---------------------------------------------------------------------------*/
/***************************************************/
static int cmd_demo_info(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    printk("Zephyr S2OPC Server demo status\n");
    printk("Server endpoint       : %s\n", CONFIG_SOPC_ENDPOINT_ADDRESS);
    printk("Server running        : %s\n", (Server_IsRunning() ? "YES" : "NO"));
    printk("Server toolkit version: %s\n", SOPC_TOOLKIT_VERSION);

    return 0;
}

/***************************************************/
static int cmd_demo_write(const struct shell* shell, size_t argc, char** argv)
{
    if (argc < 3)
    {
        printk("usage: demo write <nodeid> <value>\n");
        printk("<value> must be prefixed by b for a BOOL s for a String.\n");
        printk("Other formats not implemented here.\n");
        return 0;
    }

    const char* nodeIdC = argv[1];
    const char* dvC = argv[2];

    SOPC_NodeId nid;
    SOPC_NodeId_InitializeFromCString(&nid, nodeIdC, strlen(nodeIdC));
    SOPC_DataValue dv;
    SOPC_DataValue_Initialize(&dv);

    dv.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    dv.Value.DoNotClear = false;
    if (dvC[0] == 's')
    {
        dv.Value.BuiltInTypeId = SOPC_String_Id;
        SOPC_String_InitializeFromCString(&dv.Value.Value.String, dvC + 1);
    }
    else if (dvC[0] == 'b')
    {
        dv.Value.BuiltInTypeId = SOPC_Boolean_Id;

        dv.Value.Value.Boolean = atoi(dvC + 1);
    }
    else
    {
        printk("Invalid format for <value>\n");
        return 0;
    }

    Server_LocalWriteSingleNode(&nid, &dv);

    SOPC_NodeId_Clear(&nid);
    SOPC_DataValue_Clear(&dv);
    return 0;
}

/***************************************************/
static int cmd_demo_kill(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    Server_Interrupt();

    return 0;
}

/* Creating subcommands (level 1 command) array for command "demo". */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_demo,
                               SHELL_CMD(info, NULL, "Show demo info", cmd_demo_info),
                               SHELL_CMD(kill, NULL, "Kill server", cmd_demo_kill),
                               SHELL_CMD(write, NULL, "Write value to server", cmd_demo_write),
                               SHELL_SUBCMD_SET_END);

/* Creating root (level 0) command "demo" */
SHELL_CMD_REGISTER(demo, &sub_demo, "Demo commands", NULL);
