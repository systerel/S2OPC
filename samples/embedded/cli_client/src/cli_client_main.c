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
#include "sopc_date_time.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

#include "libs2opc_client_config_custom.h"
#include "libs2opc_new_client.h"
#include "libs2opc_request_builder.h"

// project includes
#include "test_config.h"

static int stopSignal = 0;
static SOPC_SecureConnection_Config* gConfiguration = NULL;
static SOPC_ClientConnection* gConnection = NULL;
static char* epURL = NULL;

/***************************************************/
/**               HELPER LOG MACROS                */
/***************************************************/
#define LOG_DEBUG(...) SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
#define LOG_INFO(...) SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
#define LOG_WARNING(...) SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
#define LOG_ERROR(...) SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
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
// Callback for unexpected connection events
static void client_ConnectionEventCallback(SOPC_ClientConnection* config,
                                           SOPC_ClientConnectionEvent event,
                                           SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    PRINT("UNEXPECTED CONNECTION EVENT %d with status 0x%08" PRIX32 "\n", event, status);
    stopSignal = 1;
}

/***************************************************/
// Callback in case of assert failure.
static void assert_UserCallback(const char* context)
{
    PRINT("ASSERT FAILED : <%p>\n", (const void*) context);
    PRINT("Context: <%s>\n", context);
    stopSignal = 1;
}

/***************************************************/
static void log_UserCallback(const char* timestampUtc,
                             const char* category,
                             const SOPC_Log_Level level,
                             const char* const line)
{
    SOPC_UNUSED_ARG(category);
    SOPC_UNUSED_ARG(level);
    SOPC_UNUSED_ARG(timestampUtc);
    if (line != NULL)
    {
        PRINT("%s\n", line);
    }
}

/***************************************************/
static void client_tester(void)
{
    static const char* root_node_id = "ns=0;i=85";
    PRINT("Browse Root.Objects\n");

    OpcUa_BrowseResponse* resp = NULL;
    OpcUa_BrowseRequest* req = SOPC_BrowseRequest_Create(1, 0, NULL);
    SOPC_ReturnStatus status = SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(
        req, 0, root_node_id, OpcUa_BrowseDirection_Forward, NULL, true, 0, OpcUa_BrowseResultMask_All);

    /* Browse specified node */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(gConnection, req, (void**) &resp);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Check response is expected type and not a ServiceFault
        if (&OpcUa_BrowseResponse_EncodeableType == resp->encodeableType)
        {
            SOPC_ASSERT(1 == resp->NoOfResults);
            OpcUa_BrowseResult* browseResult = &resp->Results[0];
            PRINT("status: 0x%08" PRIX32 ", nbOfResults: %d\n", browseResult->StatusCode, browseResult->NoOfReferences);
            for (int32_t i = 0; i < browseResult->NoOfReferences; i++)
            {
                OpcUa_ReferenceDescription* ref = &browseResult->References[i];
                char* nodeIdStr = SOPC_NodeId_ToCString(&ref->NodeId.NodeId);
                PRINT("Item #%d\n", i);
                PRINT("- nodeId: %s\n", nodeIdStr);
                PRINT("- displayName: %s\n", SOPC_String_GetRawCString(&ref->DisplayName.defaultText));

                SOPC_Free(nodeIdStr);
            }
        }
        else
        {
            PRINT("Call to Browse service through failed with return code: 0x%08" PRIX32 "\n",
                  resp->ResponseHeader.ServiceResult);
        }
        SOPC_EncodeableObject_Delete(resp->encodeableType, (void**) &resp);
    }
    else
    {
        PRINT("Call to Browse service through client wrapper failed with return code: %d\n", status);
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

    /* Setup platform-dependant features (network, ...)*/
    SOPC_Platform_Setup();

    epURL = SOPC_strdup(CONFIG_SOPC_ENDPOINT_ADDRESS);
    SOPC_Assert_Set_UserCallback(&assert_UserCallback);
    SOPC_Log_Configuration logCfg = {.logLevel = SOPC_LOG_LEVEL_WARNING,
                                     .logSystem = SOPC_LOG_SYSTEM_USER,
                                     .logSysConfig = {.userSystemLogConfig = {.doLog = &log_UserCallback}}};
    status = SOPC_CommonHelper_Initialize(&logCfg);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_CommonHelper_Initialize failed");

    /* Create thread for Command Line Input management*/
    status = SOPC_Thread_Create(&CLI_thread, &CLI_thread_exec, NULL, "CLI");
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_Thread_Create failed");

    PRINT("========================\n");
    PRINT("Embedded S2OPC client demo\n");

    /* Initialize the toolkit */
    status = SOPC_ClientConfigHelper_Initialize();
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_ClientConfigHelper_Initialize failed");

    while (stopSignal == 0)
    {
        SOPC_Sleep(10);
    }
    PRINT("TEST ended\r\n");
    PRINT("==========\r\n");

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();
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
    PRINT("Client configured     : %s\n", YES_NO(gConfiguration != NULL));

    return 0;
}

/***************************************************/
static int cmd_demo_configure(WordList* pList)
{
    if (NULL != gConfiguration)
    {
        bool configureClient = false;
        PRINT("\nClient already configured!\n");
        PRINT("Do you want to overwrite Configuration ? [y/n]\n");
        char* overwriteBuffer = SOPC_Shell_ReadLine();
        char* wordList = overwriteBuffer;

        const char* word = CLI_GetNextWord(&wordList);
        if (word != NULL && word[0] != 0)
        {
            if (0 == strcmp(word, "y"))
            {
                SOPC_ClientConfigHelper_Clear();
                SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
                if (SOPC_STATUS_OK != status)
                {
                    PRINT("Failed to initialized client toolkit with status %d\n", status);
                }
                else
                {
                    configureClient = true;
                }
            }
            else if (0 == strcmp(word, "n"))
            {
                PRINT("Configuration not modified \n");
            }
            else
            {
                PRINT("Unknown response \n Nothing done !\n");
            }
        }
        SOPC_Free(overwriteBuffer);
        if (!configureClient)
        {
            return 0;
        }
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

    /* configure the connection */
    gConfiguration = SOPC_ClientConfigHelper_CreateSecureConnection("CLI_Client", epURL, OpcUa_MessageSecurityMode_None,
                                                                    SOPC_SecurityPolicy_None);
    if (NULL == gConfiguration)
    {
        PRINT("\nSOPC_ClientConfigHelper_CreateSecureConnection failed \n");
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

    if (NULL == gConfiguration)
    {
        PRINT("\nClient not configured!\n");
        return 0;
    }

    SOPC_ReturnStatus status =
        SOPC_ClientHelperNew_Connect(gConfiguration, client_ConnectionEventCallback, &gConnection);

    if (SOPC_STATUS_OK != status)
    {
        PRINT("\nSOPC_ClientHelper_Connect failed with status %d\r\n", status);
    }
    else
    {
        client_tester();
        status = SOPC_ClientHelperNew_Disconnect(&gConnection);
        if (SOPC_STATUS_OK != status)
        {
            PRINT("\nSOPC_ClientHelper_Disconnect failed with code %d\r\n", status);
        }
    }
    return 0;
}

/***************************************************/
static int cmd_demo_dbg(WordList* pList)
{
    const char* word = CLI_GetNextWord(pList);

    PRINT("S2OPC Client target debug informations:\n");
    SOPC_Platform_Target_Debug(word);
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
