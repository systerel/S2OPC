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

#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// S2OPC includes
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "samples_platform_dep.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_common_build_info.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_toolkit_config.h"
#include "sopc_user_app_itf.h"
#include "sopc_user_manager.h"

#include "libs2opc_client_cmds.h"
#include "libs2opc_common_config.h"

// project includes
#include "test_config.h"

static int stopSignal = 0;
static int32_t gConfigurationId = -1;
static char* epURL = NULL;

/***************************************************/
/**               HELPER LOG MACROS                */
/***************************************************/
#define LOG_DEBUG(...) SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
#define LOG_INFO(...) SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
#define LOG_WARNING(...) SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
#define LOG_ERROR(...) SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
#define PRINT printf
#define YES_NO(x) ((x) ? "YES" : "NO")

/***************************************************/
/**          CLIENT LINE INTERFACE                 */
/***************************************************/
static SOPC_Thread CLI_thread;
typedef char* WordList; // A simple C String
static int cmd_demo_help(WordList* pList);
static int cmd_demo_info(WordList* pList);
static int cmd_demo_dbg(WordList* pList);
static int cmd_demo_log(WordList* pList);
static int cmd_demo_quit(WordList* pList);
static int cmd_demo_configure(WordList* pList);
static int cmd_demo_connect(WordList* pList);

/** Configuration of a command line */
typedef struct
{
    const char* name;
    int (*callback)(WordList* pList);
    const char* description;
} CLI_config_t;

static const CLI_config_t CLI_config[] = {
    {"help", cmd_demo_help, "Display help"},      {"quit", cmd_demo_quit, "Quit demo"},
    {"info", cmd_demo_info, "Show demo info"},    {"dbg", cmd_demo_dbg, "Show target debug info"},
    {"log", cmd_demo_log, "Set log level"},       {"conf", cmd_demo_configure, "Configure client [<endpoint>]"},
    {"conn", cmd_demo_connect, "Connect client"}, {NULL, NULL, NULL}};

/***************************************************/
/* This function receives a pointer to a C string.
 * It returns the string pointed to by pList and replaces the
 * next space by a NULL char so that the return value is now a C String
 * containing the first word of the string.
 * pList is modified to point to the next char after the insterted NULL.
 * In case pList reaches the initial NULL char, it is no more modified and
 * an empty string is returned.
 */
static const char* CLI_GetNextWord(WordList* pList)
{
    if (NULL == pList)
    {
        return "";
    }

    const char* result = *pList;

    while (**pList != '\0')
    {
        (*pList)++;
        if (**pList == ' ')
        {
            **pList = 0; // Insert an NULL char to terminate string here
            (*pList)++;
            // next string starts after the first non space char
            while (**pList == ' ')
            {
                (*pList)++;
            }
            break;
        }
    }
    return result;
}

/***************************************************/
static void disconnect_callback(const uint32_t c_id)
{
    PRINT("===> connection #%d has been terminated!\n", c_id);
}

/***************************************************/
// Callback in case of assert failure.
static void assert_UserCallback(const char* context)
{
    PRINT("ASSERT FAILED : <%p>\n", (const void*) context);
    PRINT("Context: <%s>\n", context);
    stopSignal = 1;
    exit(1);
}

/***************************************************/
static void log_UserCallback(const char* context, const char* text)
{
    (void) context;
    if (NULL != text)
    {
        PRINT("%s\n", text);
    }
}

/***************************************************/
static void client_tester(int connectionId)
{
    static const char* root_node_id = "ns=0;i=85";
    int res;
    PRINT("Browse Root.Objects\n");

    SOPC_ClientCmd_BrowseRequest browseRequest;
    SOPC_ClientCmd_BrowseResult browseResult;

    browseRequest.nodeId = root_node_id;                     // Root/Objects/
    browseRequest.direction = OpcUa_BrowseDirection_Forward; // forward
    browseRequest.referenceTypeId = "";                      // all reference types
    browseRequest.includeSubtypes = true;

    /* Browse specified node */
    res = SOPC_ClientCmd_Browse(connectionId, &browseRequest, 1, &browseResult);

    if (0 == res)
    {
        PRINT("status: %d, nbOfResults: %d\n", browseResult.statusCode, browseResult.nbOfReferences);
        for (int32_t i = 0; i < browseResult.nbOfReferences; i++)
        {
            const SOPC_ClientCmd_BrowseResultReference* ref = &browseResult.references[i];
            PRINT("Item #%d\n", i);
            PRINT("- nodeId: %s\n", ref->nodeId);
            PRINT("- displayName: %s\n", ref->displayName);

            SOPC_Free(ref->nodeId);
            SOPC_Free(ref->displayName);
            SOPC_Free(ref->browseName);
            SOPC_Free(ref->referenceTypeId);
        }
        SOPC_Free(browseResult.references);
    }
    else
    {
        PRINT("Call to Browse service through client wrapper failed with return code: %d\n", res);
    }
}

/***************************************************/
static void* CLI_thread_exec(void* arg)
{
    SOPC_UNUSED_ARG(arg);
    PRINT("Command-Line interface ready\n");

    while (SOPC_Atomic_Int_Get(&stopSignal) == 0)
    {
        char* line = SOPC_Shell_ReadLine();
        char* wordList = line;

        bool found = false;
        const char* word = CLI_GetNextWord(&wordList);
        if (word != NULL && word[0] != 0)
        {
            for (const CLI_config_t* pConfig = &CLI_config[0];
                 pConfig->name != NULL && pConfig->description != NULL && !found; pConfig++)
            {
                if (0 == strcmp(word, pConfig->name))
                {
                    pConfig->callback(&wordList);
                    found = true;
                }
            }

            if (!found)
            {
                PRINT("Unknown command <%s>\n", word);
                cmd_demo_help(NULL);
            }
        }
        SOPC_Free(line);
    }

    PRINT("Command-Line interface Terminated\n");
    return NULL;
}
/***************************************************/
void SOPC_Platform_Main(void)
{
    SOPC_ReturnStatus status;

    PRINT("\nBUILD DATE : " __DATE__ " " __TIME__ "\n");

    epURL = SOPC_strdup(CONFIG_SOPC_ENDPOINT_ADDRESS);
    SOPC_Assert_Set_UserCallback(&assert_UserCallback);
    SOPC_Log_Configuration logCfg = {.logLevel = SOPC_LOG_LEVEL_WARNING,
                                     .logSystem = SOPC_LOG_SYSTEM_USER,
                                     .logSysConfig = {.userSystemLogConfig = {.doLog = &log_UserCallback}}};
    status = SOPC_CommonHelper_Initialize(&logCfg);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_CommonHelper_Initialize failed");

    /* Setup platform-dependant features (network, ...)*/
    SOPC_Platform_Setup();

    /* Create thread for Command Line Input management*/
    status = SOPC_Thread_Create(&CLI_thread, &CLI_thread_exec, NULL, "CLI");
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_Thread_Create failed");

    PRINT("========================\n");
    PRINT("Embedded S2OPC client demo\n");

    /* Initialize the toolkit */
    SOPC_ClientCmd_Initialize(disconnect_callback);

    while (stopSignal == 0)
    {
        SOPC_Sleep(10);
    }
    PRINT("TEST ended\r\n");
    PRINT("==========\r\n");

    /* Close the toolkit */
    SOPC_ClientCmd_Finalize();
    SOPC_Free(epURL);

    LOG_INFO("# Info: Client demo stopped.\n");
    SOPC_Platform_Shutdown(true);
}

/*---------------------------------------------------------------------------
 *                            CLI implementation
 *---------------------------------------------------------------------------*/
/***************************************************/
static int cmd_demo_help(WordList* pList)
{
    SOPC_UNUSED_ARG(pList);

    PRINT("S2OPC Client demo commands:\n");

    for (const CLI_config_t* pConfig = &CLI_config[0]; pConfig->name != NULL && pConfig->description != NULL; pConfig++)
    {
        PRINT("  %-16s : %s\n", pConfig->name, pConfig->description);
    }

    return 0;
}

/***************************************************/
static int cmd_demo_info(WordList* pList)
{
    SOPC_UNUSED_ARG(pList);

    const SOPC_Build_Info buildInfo = SOPC_CommonHelper_GetBuildInfo().commonBuildInfo;

    PRINT("\nEmbedded S2OPC client demo status\n");
    PRINT("Server endpoint       : %s\n", epURL);
    PRINT("Client toolkit version: %s\n", SOPC_TOOLKIT_VERSION);
    PRINT("Server src commit     : %s\n", buildInfo.buildSrcCommit);
    PRINT("Server docker Id      : %s\n", buildInfo.buildDockerId);
    PRINT("Server build date     : %s\n", buildInfo.buildBuildDate);
    PRINT("Client configured     : %s\n", YES_NO(gConfigurationId > 0));

    return 0;
}

/***************************************************/
static int cmd_demo_configure(WordList* pList)
{
    SOPC_ClientCmd_Security security = {
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
        .path_cert_x509_token = NULL,
        .path_key_x509_token = NULL,
    };

    if (gConfigurationId > 0)
    {
        PRINT("\nClient already configured!\n");
        return 0;
    }

    const char* word = CLI_GetNextWord(pList);
    SOPC_Free(epURL);
    if (word[0] != 0)
    {
        epURL = SOPC_strdup(word);
    }
    else
    {
        PRINT("Using default endpoint address '%s'\n", CONFIG_SOPC_ENDPOINT_ADDRESS);
        epURL = SOPC_strdup(CONFIG_SOPC_ENDPOINT_ADDRESS);
    }
    SOPC_ASSERT(epURL != NULL);

    SOPC_ClientCmd_EndpointConnection endpoint = {
        .endpointUrl = epURL,
        .serverUri = NULL,
        .reverseConnectionConfigId = 0,
    };

    /* connect to the endpoint */
    gConfigurationId = SOPC_ClientCmd_CreateConfiguration(&endpoint, &security, NULL);
    if (gConfigurationId <= 0)
    {
        PRINT("\nSOPC_ClientHelper_CreateConfiguration failed with code %d\r\n", gConfigurationId);
    }
    else
    {
        PRINT("\nCreated connection to %s\n", epURL);
    }
    return 0;
}

/***************************************************/
static int cmd_demo_connect(WordList* pList)
{
    SOPC_UNUSED_ARG(pList);

    int32_t connectionId = SOPC_ClientCmd_CreateConnection(gConfigurationId);

    if (connectionId <= 0)
    {
        PRINT("\nSOPC_ClientHelper_CreateConnection failed with code %d\r\n", connectionId);
    }
    else
    {
        client_tester(connectionId);
        int32_t discoRes = SOPC_ClientCmd_Disconnect(connectionId);
        if (discoRes != 0)
        {
            PRINT("\nSOPC_ClientHelper_Disconnect failed with code %d\r\n", discoRes);
        }
    }
    return 0;
}

/***************************************************/
static int cmd_demo_dbg(WordList* pList)
{
    SOPC_UNUSED_ARG(pList);

    PRINT("S2OPC Client target debug informations:\n");
    SOPC_Platform_Target_Debug();
    return 0;
}

/***************************************************/
static int cmd_demo_log(WordList* pList)
{
    const char* word = CLI_GetNextWord(pList);
    switch (word[0])
    {
    case 'E':
        SOPC_Logger_SetTraceLogLevel(SOPC_LOG_LEVEL_ERROR);
        break;
    case 'W':
        SOPC_Logger_SetTraceLogLevel(SOPC_LOG_LEVEL_WARNING);
        break;
    case 'I':
        SOPC_Logger_SetTraceLogLevel(SOPC_LOG_LEVEL_INFO);
        break;
    case 'D':
        SOPC_Logger_SetTraceLogLevel(SOPC_LOG_LEVEL_DEBUG);
        break;
    default:
        PRINT("usage: log <D|I|W|E>\n");
        break;
    }
    return 0;
}

/***************************************************/
static int cmd_demo_quit(WordList* pList)
{
    SOPC_UNUSED_ARG(pList);
    stopSignal = 1;

    return 0;
}
