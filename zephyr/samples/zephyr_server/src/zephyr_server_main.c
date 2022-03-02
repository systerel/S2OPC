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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>

#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

#include "sopc_address_space.h"
#include "sopc_atomic.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config.h"

#include "server.h"
#include "network_init.h"
#include "threading_alt.h"

// Global test variables
static SOPC_S2OPC_Config s2opcConfig;

static void log_UserCallback(const char* context, const char* text);

/***************************************************/
SOPC_Build_Info SOPC_ClientServer_GetBuildInfo()
{
    static const SOPC_Build_Info sopc_client_server_build_info =
    {
            .buildVersion = SOPC_TOOLKIT_VERSION,
            .buildSrcCommit = "Not applicable",
            .buildDockerId = "",
            .buildBuildDate = ""
    };

    return sopc_client_server_build_info;
}

/***************************************************/
SOPC_Build_Info SOPC_Common_GetBuildInfo()
{
    static const SOPC_Build_Info sopc_common_build_info =
    {
            .buildVersion = SOPC_TOOLKIT_VERSION,
            .buildSrcCommit = "Unknown_Revision",
            .buildDockerId = "",
            .buildBuildDate = ""
    };

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
    }
}
/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    // Note: avoid unused parameter warning from compiler
    (void) argc;
    (void) argv;

    /* Configure the server logger:
     * DEBUG traces generated in ./<argv[0]_logs/ */
    SOPC_Log_Configuration logConfig;
    logConfig.logSystem = SOPC_LOG_SYSTEM_USER;
    logConfig.logLevel = SOPC_LOG_LEVEL_WARNING;
    logConfig.logSysConfig.userSystemLogConfig.doLog = &log_UserCallback;

    bool netInit = Network_Initialize();
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
