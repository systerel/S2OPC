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

#include <kernel.h>
#include <limits.h>
#include <shell/shell.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_common.h"
#include "sopc_common_build_info.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_sub_scheduler.h"
#include "sopc_time.h"
#include "sopc_toolkit_config.h"
#include "sopc_user_app_itf.h"
#include "sopc_user_manager.h"

#include "libs2opc_client_cmds.h"
#include "libs2opc_common_config.h"

#include "cache.h"
#include "network_init.h"
#include "static_security_data.h"
#include "threading_alt.h"

volatile int stopSignal = 0;
static int32_t gConfigurationId = -1;
static const char* epURL = CONFIG_SOPC_ENDPOINT_ADDRESS;

/***************************************************/
static void signal_stop_server(int sig)
{
    (void) sig;

    if (stopSignal != 0)
    {
        exit(1);
    }
    else
    {
        stopSignal = 1;
    }
}

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

/***************************************************/
static void disconnect_callback(const uint32_t c_id)
{
    printf("===> connection #%d has been terminated!\n", c_id);
}

/***************************************************/
// Callback in case of assert failure.
static void assert_UserCallback(const char* context)
{
    printk("ASSERT FAILED : <%p>\n", (void*) context);
    printk("Context: <%s>\n", context);
    stopSignal = 1;
    exit(1);
}

/***************************************************/
static void log_UserCallback(const char* context, const char* text)
{
    (void) context;
    if (NULL != text)
    {
        printk("%s\n", text);
    }
}

/***************************************************/
static void client_tester(int connectionId)
{
    static const char* root_node_id = "ns=0;i=85";
    int res;
    printk("Browse Root.Objects\n");

    SOPC_ClientHelper_BrowseRequest browseRequest;
    SOPC_ClientHelper_BrowseResult browseResult;

    browseRequest.nodeId = "ns=0;i=85";                      // Root/Objects/
    browseRequest.direction = OpcUa_BrowseDirection_Forward; // forward
    browseRequest.referenceTypeId = "";                      // all reference types
    browseRequest.includeSubtypes = true;

    res = SOPC_ClientHelper_Browse(connectionId, &browseRequest, 1, &browseResult);
    if (0 == res)
    {
        SOPC_ClientHelper_BrowseRequest browseRequest;
        SOPC_ClientHelper_BrowseResult browseResult;

        browseRequest.nodeId = root_node_id;                     // Root/Objects/
        browseRequest.direction = OpcUa_BrowseDirection_Forward; // forward
        browseRequest.referenceTypeId = "";                      // all reference types
        browseRequest.includeSubtypes = true;

        /* Browse specified node */
        res = SOPC_ClientHelper_Browse(connectionId, &browseRequest, 1, &browseResult);

        if (0 == res)
        {
            printf("status: %d, nbOfResults: %d\n", browseResult.statusCode, browseResult.nbOfReferences);
            for (int32_t i = 0; i < browseResult.nbOfReferences; i++)
            {
                const SOPC_ClientHelper_BrowseResultReference* ref = &browseResult.references[i];
                printf("Item #%d\n", i);
                printf("- nodeId: %s\n", ref->nodeId);
                printf("- displayName: %s\n", ref->displayName);

                free(ref->nodeId);
                free(ref->displayName);
                free(ref->browseName);
                free(ref->referenceTypeId);
            }
            free(browseResult.references);
        }
        else
        {
            printf("Call to Browse service through client wrapper failed with return code: %d\n", res);
        }
    }
}

/***************************************************/
int main(int argc, char* const argv[])
{
    printk("\nBUILD DATE : " __DATE__ " " __TIME__ "\n");
    /* Signal handling: close the server gracefully when interrupted */
    signal(SIGINT, signal_stop_server);
    signal(SIGTERM, signal_stop_server);

    /* Parse command line arguments ? */
    (void) argc;
    (void) argv;
    SOPC_Assert_Set_UserCallback(&assert_UserCallback);

    bool netInit = Network_Initialize(NULL);
    SOPC_ReturnStatus status;
    SOPC_Log_Configuration logCfg = {.logLevel = SOPC_LOG_LEVEL_WARNING,
                                     .logSystem = SOPC_LOG_SYSTEM_USER,
                                     .logSysConfig = {.userSystemLogConfig = {.doLog = &log_UserCallback}}};

    SOPC_ASSERT(netInit == true);

    /* Initialize MbedTLS */
    tls_threading_initialize();

    printk("========================\n");
    printk("ZEPHYR S2OPC client demo\n");

    /* Initialize the toolkit */
    status = SOPC_CommonHelper_Initialize(&logCfg);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_ClientHelper_Initialize(disconnect_callback);

    while (stopSignal == 0)
    {
        SOPC_Sleep(10);
    }
    printk("TEST ended\r\n");
    printk("==========\r\n");

    /* Close the toolkit */
    SOPC_ClientHelper_Finalize();
    return 0;
}

/*---------------------------------------------------------------------------
 *                             NET SHELL CONFIGURATION
 *---------------------------------------------------------------------------*/
#define BOOL_TO_TEXT(b) ((b) ? "YES" : "NO")

/***************************************************/
static int cmd_demo_info(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    printk("\nZephyr S2OPC client demo status\n");
    printk("Server endpoint       : %s\n", epURL);
    printk("Client toolkit version: %s\n", SOPC_TOOLKIT_VERSION);
    printk("Client configured     : %s\n", BOOL_TO_TEXT(gConfigurationId > 0));

    return 0;
}

/***************************************************/
static int cmd_demo_configure(const struct shell* shell, size_t argc, char** argv)
{
    SOPC_ClientHelper_Security security = {
        .security_policy = SOPC_SecurityPolicy_None_URI,
        .security_mode = OpcUa_MessageSecurityMode_None,
        .path_cert_auth = NULL,
        .path_crl = NULL,
        .path_cert_srv = NULL,
        .path_cert_cli = NULL,
        .path_key_cli = NULL,
        .policyId = "anonymous",
        .username = NULL,
        .password = NULL,
    }; // TODO !! no FS

    if (gConfigurationId > 0)
    {
        printk("\nClient already configured!\n");
        return 0;
    }

    if (argc > 1)
    {
        epURL = argv[1];
    }
    SOPC_ASSERT(epURL != NULL);

    /* connect to the endpoint */
    gConfigurationId = SOPC_ClientHelper_CreateConfiguration(epURL, &security, NULL);
    if (gConfigurationId <= 0)
    {
        printk("\nSOPC_ClientHelper_CreateConfiguration failed with code %d\r\n", gConfigurationId);
    }
    else
    {
        printk("\nCreated connection to %s\n", epURL);
    }
    return 0;
}

/***************************************************/
static int cmd_demo_connect(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    int32_t connectionId = SOPC_ClientHelper_CreateConnection(gConfigurationId);

    if (connectionId <= 0)
    {
        printk("\nSOPC_ClientHelper_CreateConnection failed with code %d\r\n", connectionId);
    }
    else
    {
        client_tester(connectionId);
        int32_t discoRes = SOPC_ClientHelper_Disconnect(connectionId);
        if (discoRes != 0)
        {
            printk("\nSOPC_ClientHelper_Disconnect failed with code %d\r\n", discoRes);
        }
    }
    return 0;
}

/***************************************************/
static int cmd_demo_kill(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    stopSignal = 1;

    return 0;
}
/* Creating subcommands (level 1 command) array for command "demo". */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_demo,
                               SHELL_CMD(info, NULL, "Show demo info", cmd_demo_info),
                               SHELL_CMD(conf, NULL, "Configure client [<endpoint>]", cmd_demo_configure),
                               SHELL_CMD(conn, NULL, "Connect client", cmd_demo_connect),
                               SHELL_CMD(kill, NULL, "Quit", cmd_demo_kill),
                               SHELL_SUBCMD_SET_END);

/* Creating root (level 0) command "demo" */
SHELL_CMD_REGISTER(demo, &sub_demo, "Demo commands", NULL);
