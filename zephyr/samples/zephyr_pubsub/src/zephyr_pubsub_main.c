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
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_sub_scheduler.h"
#include "sopc_toolkit_config.h"
#include "sopc_user_app_itf.h"
#include "sopc_user_manager.h"

#include "cache.h"
#include "network_init.h"
#include "pubsub_config_static.h"
#include "static_security_data.h"
#include "threading_alt.h"

static SOPC_PubSubConfiguration* pPubSubConfig = NULL;
static SOPC_SubTargetVariableConfig* pTargetConfig = NULL;
static SOPC_PubSourceVariableConfig* pSourceConfig = NULL;
static bool gPubStarted = false;
static bool gSubStarted = false;

volatile int stopSignal = 0;
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

static void cb_SetSubStatus(SOPC_PubSubState state)
{
    printk("New Pub/sub state: %d\n", (int) state);
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
    SOPC_ASSERT(netInit == true);

    /* Initialize MbedTLS */
    tls_threading_initialize();

    /* Initialize S2OPC Server */
    const SOPC_Log_Configuration logCfg = {.logLevel = SOPC_LOG_LEVEL_WARNING,
                                           .logSystem = SOPC_LOG_SYSTEM_USER,
                                           .logSysConfig = {.userSystemLogConfig = {.doLog = &log_UserCallback}}};

    SOPC_ReturnStatus status = SOPC_Common_Initialize(logCfg);

    if (SOPC_STATUS_OK != status)
    {
        printf("Error while initializing logs\n");
    }

    // CONFIGURE PUBSUB
    pPubSubConfig = SOPC_PubSubConfig_GetStatic();
    if (NULL == pPubSubConfig)
    {
        return SOPC_STATUS_NOK;
    }

    /* Sub target configuration */
    if (SOPC_STATUS_OK == status)
    {
        pTargetConfig = SOPC_SubTargetVariableConfig_Create(&Cache_SetTargetVariables);
        if (NULL == pTargetConfig)
        {
            printf("# Error: Cannot create Sub configuration.\n");
            status = SOPC_STATUS_NOK;
        }
    }

    /* Pub target configuration */
    if (SOPC_STATUS_OK == status)
    {
        pSourceConfig = SOPC_PubSourceVariableConfig_Create(&Cache_GetSourceVariables);
        if (NULL == pSourceConfig)
        {
            printf("# Error: Cannot create Pub configuration.\n");
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        // Configure SKS for PubSub
        SOPC_LocalSKS_init_static(pubSub_keySign, sizeof(pubSub_keySign),       //
                                  pubSub_keyEncrypt, sizeof(pubSub_keyEncrypt), //
                                  pubSub_keyNonce, sizeof(pubSub_keyNonce));

        Cache_Initialize(pPubSubConfig);
    }

    while (!stopSignal)
    {
        k_sleep(K_MSEC(50));
    }

    SOPC_PubScheduler_Stop();
    SOPC_SubScheduler_Stop();
    Cache_Clear();
    SOPC_PubSourceVariableConfig_Delete(pSourceConfig);
    SOPC_SubTargetVariableConfig_Delete(pTargetConfig);
    SOPC_PubSubConfiguration_Delete(pPubSubConfig);

    printk("\r\nTEST ended\r\n");
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

    printk("\nZephyr S2OPC PubSub demo status\n");
    printk("Server toolkit version: %s\n", SOPC_TOOLKIT_VERSION);
    if (pPubSubConfig != NULL)
    {
        uint32_t nbConn = SOPC_PubSubConfiguration_Nb_PubConnection(pPubSubConfig);
        printk("Pub connections: %d\n", nbConn);
        for (uint32_t iConn = 0; iConn < nbConn; iConn++)
        {
            SOPC_PubSubConnection* connx = SOPC_PubSubConfiguration_Get_PubConnection_At(pPubSubConfig, iConn);
            printk("  - RUNNING=%d\n", gPubStarted);
            printk("  - ADDR :%s\n", SOPC_PubSubConnection_Get_Address(connx));
        }

        nbConn = SOPC_PubSubConfiguration_Nb_SubConnection(pPubSubConfig);
        printk("Sub connections: %d\n", nbConn);
        for (uint32_t iConn = 0; iConn < nbConn; iConn++)
        {
            SOPC_PubSubConnection* connx = SOPC_PubSubConfiguration_Get_SubConnection_At(pPubSubConfig, iConn);
            connx = SOPC_PubSubConfiguration_Get_PubConnection_At(pPubSubConfig, 0);
            printk("  - RUNNING=%d\n", gSubStarted);
            printk("  - ADDR :%s\n", SOPC_PubSubConnection_Get_Address(connx));
        }
    }

    return 0;
}

/***************************************************/
static int cmd_demo_log(const struct shell* shell, size_t argc, char** argv)
{
    const char* param = (argc < 2 ? "" : argv[1]);
    if (0 == SOPC_strcmp_ignore_case(param, "debug"))
    {
        SOPC_Logger_SetTraceLogLevel(SOPC_LOG_LEVEL_DEBUG);
    }
    else if (0 == SOPC_strcmp_ignore_case(param, "info"))
    {
        SOPC_Logger_SetTraceLogLevel(SOPC_LOG_LEVEL_INFO);
    }
    else if (0 == SOPC_strcmp_ignore_case(param, "warning"))
    {
        SOPC_Logger_SetTraceLogLevel(SOPC_LOG_LEVEL_WARNING);
    }
    else if (0 == SOPC_strcmp_ignore_case(param, "error"))
    {
        SOPC_Logger_SetTraceLogLevel(SOPC_LOG_LEVEL_ERROR);
    }
    else
    {
        printk("\n Invalid log level (debug/info/warning/error).\n");
    }

    return 0;
}

/***************************************************/
static int cmd_demo_set(const struct shell* shell, size_t argc, char** argv)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    const char* param1 = (argc < 3 ? "" : argv[1]);
    const char* param2 = (argc < 3 ? "" : argv[2]);

    SOPC_NodeId nid;
    status = SOPC_NodeId_InitializeFromCString(&nid, param1, strlen(param1));
    if (SOPC_STATUS_OK == status && param2[0] != 0)
    {
        bool result = false;
        Cache_Lock();
        SOPC_DataValue* dv = Cache_Get(&nid);
        if (NULL != dv)
        {
            result = Cache_UpdateVariant(dv->Value.BuiltInTypeId, &dv->Value.Value, param2);
        }
        Cache_Unlock();

        if (result)
        {
            printk("\n..OK!\n");
        }
        else
        {
            printk("\nUpdate failed.\n");
        }
    }
    else
    {
        printk("\n Invalid parameters.");
    }
    SOPC_NodeId_Clear(&nid);

    return 0;
}

/***************************************************/
static int cmd_demo_print(const struct shell* shell, size_t argc, char** argv)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    const char* param = (argc < 2 ? NULL : argv[1]);

    if (param == NULL)
    {
        printk("\nDump full cache:\n");
        Cache_Dump();
    }
    else
    {
        SOPC_NodeId nid;
        status = SOPC_NodeId_InitializeFromCString(&nid, param, strlen(param));
        if (SOPC_STATUS_OK == status)
        {
            SOPC_DataValue dvc;
            SOPC_DataValue_Initialize(&dvc);

            // Only copy DV during Cache_Lock , to avoid too long Cache blocking
            Cache_Lock();
            SOPC_DataValue* dv = Cache_Get(&nid);
            if (NULL != dv)
            {
                status = SOPC_DataValue_Copy(&dvc, dv);
            }
            Cache_Unlock();

            // Now process with copy
            if (status == SOPC_STATUS_OK)
            {
                Cache_Dump_VarValue(&nid, &dvc);
            }

            SOPC_DataValue_Clear(&dvc);
        }
        else
        {
            printk("\n Invalid node id.\n");
        }
        SOPC_NodeId_Clear(&nid);
    }

    return 0;
}

/***************************************************/
static int cmd_demo_start(const struct shell* shell, size_t argc, char** argv)
{
    const char* param = (argc < 2 ? "" : argv[1]);

    if (0 == strcmp(param, "pub"))
    {
        // start pub
        bool bResult;
        bResult = SOPC_PubScheduler_Start(pPubSubConfig, pSourceConfig, CONFIG_SOPC_PUBLISHER_PRIORITY);
        if (!bResult)
        {
            printk("\r\nFailed to start Publisher!\r\n");
        }
        else
        {
            gPubStarted = true;
            printk("\r\nPublisher started\r\n");
        }
    }
    else if (0 == strcmp(param, "sub"))
    {
        // start sub
        bool bResult;
        bResult =
            SOPC_SubScheduler_Start(pPubSubConfig, pTargetConfig, cb_SetSubStatus, CONFIG_SOPC_SUBSCRIBER_PRIORITY);
        if (!bResult)
        {
            printk("\r\nFailed to start Subscriber!\r\n");
        }
        else
        {
            gSubStarted = true;
            printk("\r\nSubscriber started\r\n");
        }
    }
    else
    {
        printk("usage: %s [pub|sub]\n", argv[0]);
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
                               SHELL_CMD(start, NULL, "Start [pub|sub]", cmd_demo_start),
                               SHELL_CMD(print, NULL, "Print content of  <NodeId>", cmd_demo_print),
                               SHELL_CMD(set, NULL, "Set content of  <NodeId>", cmd_demo_set),
                               SHELL_CMD(log, NULL, "Set log level", cmd_demo_log),
                               SHELL_CMD(kill, NULL, "Kill demo", cmd_demo_kill),
                               SHELL_SUBCMD_SET_END);

/* Creating root (level 0) command "demo" */
SHELL_CMD_REGISTER(demo, &sub_demo, "Demo commands", NULL);
