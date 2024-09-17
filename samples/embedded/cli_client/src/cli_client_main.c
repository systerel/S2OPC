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
#include "sopc_encodeable.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

#include "libs2opc_client_config_custom.h"
#include "libs2opc_client.h"
#include "libs2opc_request_builder.h"

// project includes
#include "test_config.h"

static int stopSignal = 0;
static SOPC_SecureConnection_Config* gConfiguration = NULL;
static SOPC_SecureConnection_Config* gMultiConfiguration[MAX_CONFIG] = {NULL};
// static SOPC_ClientConnection* gConnection = NULL;
static SOPC_ClientConnection* gMultiConnection[MAX_CONFIG] = {NULL};
static char* epURL = NULL;

#define IS_INVALID_CFG_IDX(cnxIndex) ((cnxIndex) >= MAX_CONFIG || (cnxIndex) < 0)

#define STR_INVALID_INDEX "\nInvalid index!\n"

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
static int cmd_demo_deconfigure(WordList* pList);
static int cmd_demo_connect(WordList* pList);
static int cmd_demo_read(WordList* pList);
static int cmd_demo_write(WordList* pList);
static int cmd_demo_list(WordList* pList);
static int cmd_demo_disconnect(WordList* pList);

/** Configuration of a command line */
typedef struct
{
    const char* name;
    int (*callback)(WordList* pList);
    const char* description;
} CLI_config_t;

static const CLI_config_t CLI_config[] = {{"help", cmd_demo_help, "Display help"},
                                          {"quit", cmd_demo_quit, "Quit demo"},
                                          {"info", cmd_demo_info, "Show demo info"},
                                          {"dbg", cmd_demo_dbg, "Show target debug info"},
                                          {"log", cmd_demo_log, "Set log level"},
                                          {"conf", cmd_demo_configure, "Configure client [<endpoint>] [<cnxIndex>]"},
                                          {"deconf", cmd_demo_deconfigure, "Deconfigure client [<cnxIndex>]"},
                                          {"read", cmd_demo_read, "Print content of  <NodeId> from server <cnxIndex>"},
                                          {"write", cmd_demo_write, "Write value to server"},
                                          {"disc", cmd_demo_disconnect, "Disconnect client <cnxIndex>"},
                                          {"list", cmd_demo_list, "List all configured endpoints"},
                                          {"conn", cmd_demo_connect, "Connect client <cnxIndex>"},
                                          {NULL, NULL, NULL}};

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
/** Prints on console a human-readable representation of a nid and a Datavalue

 * @param nid The NodeId to display
 * @param dv The DataValue to display
 */
static void print_VarValue(const SOPC_NodeId* nid, const SOPC_DataValue* dv)
{
    char* nidStr = SOPC_NodeId_ToCString(nid);
    SOPC_ASSERT(NULL != nid && NULL != nidStr);

    PRINT("- %.25s", nidStr);

    if (NULL != dv)
    {
        int result = -1;
        static char status[22];
        if (dv->Status & SOPC_BadStatusMask)
        {
            result = sprintf(status, "BAD 0x%08" PRIX32, dv->Status);
        }
        else if (dv->Status & SOPC_UncertainStatusMask)
        {
            result = sprintf(status, "UNCERTAIN 0x%08" PRIX32, dv->Status);
        }
        else
        {
            result = sprintf(status, "GOOD");
        }
        SOPC_ASSERT(result > 0);

        const char* type = "";
        switch (dv->Value.ArrayType)
        {
        case SOPC_VariantArrayType_Matrix:
            type = " ; [MAT]";
            break;
        case SOPC_VariantArrayType_Array:
            type = " ; [ARR]";
            break;
        case SOPC_VariantArrayType_SingleValue:
        default:
            break;
        }
        PRINT(" ; Status = %s%s", status, type);
        if (type[0] == 0)
        {
            static const char* typeName[] = {
                "Null",           "Boolean",       "SByte",         "Byte",          "Int16",           "UInt16",
                "Int32",          "UInt32",        "Int64",         "UInt64",        "Float",           "Double",
                "String",         "DateTime",      "Guid",          "ByteString",    "XmlElement",      "NodeId",
                "ExpandedNodeId", "StatusCode",    "QualifiedName", "LocalizedText", "ExtensionObject", "DataValue",
                "Variant",        "DiagnosticInfo"};

            const SOPC_BuiltinId typeId = dv->Value.BuiltInTypeId;
            if (typeId <= sizeof(typeName) / sizeof(*typeName) - 1)
            {
                PRINT(" ; Type=%.12s ; Val = ", typeName[typeId]);
            }
            else
            {
                PRINT(" ; Type=%.12s ; Val = ", "<Invalid>");
            }

            switch (typeId)
            {
            case SOPC_Boolean_Id:
                PRINT("%" PRId32, (int32_t) dv->Value.Value.Boolean);
                break;
            case SOPC_SByte_Id:
                PRINT("%" PRId32, (uint32_t) dv->Value.Value.Sbyte);
                break;
            case SOPC_Byte_Id:
                PRINT("%" PRId32, (int32_t) dv->Value.Value.Byte);
                break;
            case SOPC_UInt16_Id:
                PRINT("%" PRIu16, dv->Value.Value.Uint16);
                break;
            case SOPC_Int16_Id:
                PRINT("%" PRId16, dv->Value.Value.Int16);
                break;
            case SOPC_Int32_Id:
                PRINT("%" PRId32, dv->Value.Value.Int32);
                break;
            case SOPC_UInt32_Id:
                PRINT("%" PRIu32, dv->Value.Value.Uint32);
                break;
            case SOPC_Int64_Id:
                PRINT("%" PRId64, dv->Value.Value.Int64);
                break;
            case SOPC_UInt64_Id:
                PRINT("%" PRIu64, dv->Value.Value.Uint64);
                break;
            case SOPC_Float_Id:
                PRINT("%f", (double) dv->Value.Value.Floatv);
                break;
            case SOPC_Double_Id:
                PRINT("%f", (double) dv->Value.Value.Doublev);
                break;
            case SOPC_String_Id:
                PRINT("<%s>", SAFE_STRING(SOPC_String_GetRawCString(&dv->Value.Value.String)));
                break;
            case SOPC_StatusCode_Id:
                PRINT("0x%08" PRIX32, dv->Value.Value.Status);
                break;
            default:
                PRINT("(...)");
                break;
            }
        }
    }
    else
    {
        PRINT("<no value>");
    }
    PRINT("\n");

    SOPC_Free(nidStr);
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
static void client_tester(int cnxIndex)
{
    if (IS_INVALID_CFG_IDX(cnxIndex))
    {
        PRINT(STR_INVALID_INDEX);
        return;
    }

    static const char* root_node_id = "ns=0;i=85";
    PRINT("Browse Root.Objects\n");

    OpcUa_BrowseResponse* resp = NULL;
    OpcUa_BrowseRequest* req = SOPC_BrowseRequest_Create(1, 0, NULL);
    SOPC_ReturnStatus status = SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(
        req, 0, root_node_id, OpcUa_BrowseDirection_Forward, NULL, true, 0, OpcUa_BrowseResultMask_All);

    /* Browse specified node */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelperNew_ServiceSync(gMultiConnection[cnxIndex], req, (void**) &resp);
        SOPC_ASSERT(NULL != resp);
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
        SOPC_Encodeable_Delete(resp->encodeableType, (void**) &resp);
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

/*****************************************************************/
static SOPC_DataValue* Server_ReadSingleNode(const SOPC_NodeId* pNid, int cnxIndex)
{
    if (IS_INVALID_CFG_IDX(cnxIndex))
    {
        PRINT(STR_INVALID_INDEX);
        return 0;
    }

    OpcUa_ReadRequest* request = SOPC_ReadRequest_Create(1, OpcUa_TimestampsToReturn_Neither);
    OpcUa_ReadResponse* response = NULL;
    SOPC_ASSERT(NULL != request);

    SOPC_ReturnStatus status;
    status = SOPC_ReadRequest_SetReadValue(request, 0, pNid, SOPC_AttributeId_Value, NULL);
    if (status != SOPC_STATUS_OK)
    {
        LOG_WARNING("Read Value failed from SetReadValue with code  %d", (int) status);
        SOPC_Free(request);
        return NULL;
    }
    status = SOPC_ClientHelperNew_ServiceSync(gMultiConnection[cnxIndex], request, (void**) &response);
    if (status != SOPC_STATUS_OK)
    {
        LOG_WARNING("Read Value failed from Service sync with code  %d", (int) status);
        SOPC_ASSERT(NULL == response);
        return NULL;
    }

    SOPC_DataValue* result = NULL;
    if (response != NULL && response->NoOfResults == 1)
    {
        // Allocate the result only if the response contains exactly the expected content
        result = (SOPC_DataValue*) SOPC_Malloc(sizeof(*result));
        SOPC_ASSERT(NULL != result);
        SOPC_DataValue_Initialize(result);
        SOPC_DataValue_Copy(result, &response->Results[0]);
    }
    OpcUa_ReadResponse_Clear(response);
    SOPC_Free(response);
    return result;
}

/***************************************************/
/***
 * Request the server to update a node value
 * @param pNid The nodeId to modify
 * @param pDv The DataValue to write
 *
 */
static bool Server_WriteSingleNode(const SOPC_NodeId* pNid, SOPC_DataValue* pDv, int cnxIndex)
{
    if (IS_INVALID_CFG_IDX(cnxIndex))
    {
        PRINT(STR_INVALID_INDEX);
        return 0;
    }

    OpcUa_WriteRequest* request = SOPC_WriteRequest_Create(1);
    OpcUa_WriteResponse* response = NULL;
    SOPC_ASSERT(NULL != request);

    SOPC_ReturnStatus status;
    status = SOPC_WriteRequest_SetWriteValue(request, 0, pNid, SOPC_AttributeId_Value, NULL, pDv);
    if (status != SOPC_STATUS_OK)
    {
        LOG_WARNING("SetWriteValue failed with code  %d", status);
        SOPC_Free(request);
    }
    else
    {
        status = SOPC_ClientHelperNew_ServiceSync(gMultiConnection[cnxIndex], request, (void**) &response);

        if (status != SOPC_STATUS_OK)
        {
            LOG_WARNING("ServiceAsync failed with code  (%d)", status);
            SOPC_Free(request);
        }
        else
        {
            PRINT("Write sucessful\n");
        }
    }

    return status == SOPC_STATUS_OK;
}

/*****************************************************************/
static void Server_GetEndpoint(const SOPC_SecureConnection_Config* Config)
{
    PRINT(" Url : %s \r\n ", Config->scConfig.url);
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
    const char* word = CLI_GetNextWord(pList);
    const char* cnxStr = CLI_GetNextWord(pList);

    if (cnxStr[0] == 0 || word[0] == 0)
    {
        PRINT("usage: demo conf <address> <cnxIndex>\n");
        PRINT("<cnxIndex> must be < %d and > 0\n", MAX_CONFIG);
        PRINT("<cnxIndex = 0> is used for default configuration\r\n");
        return 0;
    }
    int cnxIndex;
    int n = sscanf(cnxStr, "%d", &cnxIndex);
    if (n == 0 || IS_INVALID_CFG_IDX(cnxIndex))
    {
        PRINT(STR_INVALID_INDEX);
        return 0;
    }
    if (NULL != gMultiConfiguration[cnxIndex])
    {
        PRINT("\nClient cnxIndex %d already configured\n", cnxIndex);
        return 0;
    }
    OpcUa_MessageSecurityMode security_mode;
    security_mode = OpcUa_MessageSecurityMode_None;

    SOPC_Free(epURL);
    if (word[0] != 0)
    {
        epURL = SOPC_strdup(word);
    }
    else
    {
        PRINT("Using default endpoint address '%s'\n", CONFIG_SOPC_ENDPOINT_ADDRESS);
        epURL = SOPC_strdup(CONFIG_SOPC_ENDPOINT_ADDRESS);
        cnxIndex = 0;
    }
    SOPC_ASSERT(epURL != NULL);

    /* configure the connection */
    gConfiguration =
        SOPC_ClientConfigHelper_CreateSecureConnection("CLI_Client", epURL, security_mode, SOPC_SecurityPolicy_None);
    if (NULL == gConfiguration)
    {
        PRINT("\nSOPC_ClientConfigHelper_CreateSecureConnection failed \n");
    }
    else
    {
        gMultiConfiguration[cnxIndex] = gConfiguration;
        PRINT("\nCreated connection to %s\n", epURL);
    }

    return 0;
}

/***************************************************/
static int cmd_demo_deconfigure(WordList* pList)
{
    const char* cnxStr = CLI_GetNextWord(pList);
    if (cnxStr[0] == 0)
    {
        PRINT("usage: demo deconf <cnxIndex>\n");
        return 0;
    }
    int cnxIndex;
    int n = sscanf(cnxStr, "%d", &cnxIndex);
    if (n == 0 || IS_INVALID_CFG_IDX(cnxIndex))
    {
        PRINT(STR_INVALID_INDEX);
        return 0;
    }
    if (NULL == gMultiConfiguration[cnxIndex])
    {
        PRINT("\nNo configuration on %d\n", cnxIndex);
        return 0;
    }
    else
    {
        SOPC_ReturnStatus status = SOPC_ClientHelperNew_Disconnect(&gMultiConnection[cnxIndex]);
        if (SOPC_STATUS_OK != status)
        {
            PRINT("\nSOPC_ClientHelper_Disconnect failed with code %d\r\n", status);
        }
        else
        {
            PRINT("\nDisconnected\r\n");
        }
        // SOPC_Free(gMultiConfiguration[cnxIndex]);
        gMultiConfiguration[cnxIndex] = NULL;

        PRINT("\nCleared configuration on cnxIndex %d\n", cnxIndex);
    }

    return 0;
}

/***************************************************/
static int cmd_demo_read(WordList* pList)
{
    const char* cnxStr = CLI_GetNextWord(pList);
    const char* nodeIdC = CLI_GetNextWord(pList);
    if (nodeIdC[0] == 0)
    {
        PRINT("usage: demo read <cnx_index> <nodeid> \n");
        return 0;
    }
    int cnxIndex;
    int n = sscanf(cnxStr, "%d", &cnxIndex);
    if (n == 0 || IS_INVALID_CFG_IDX(cnxIndex))
    {
        PRINT(STR_INVALID_INDEX);
        return 0;
    }

    SOPC_NodeId nid;
    SOPC_ReturnStatus status = SOPC_NodeId_InitializeFromCString(&nid, nodeIdC, (int32_t) strlen(nodeIdC));
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_DataValue* dv = Server_ReadSingleNode(&nid, cnxIndex);

    if (NULL == dv)
    {
        PRINT("Failed to read node '%s'\n", nodeIdC);
        SOPC_NodeId_Clear(&nid);
        return 1;
    }

    print_VarValue(&nid, dv);

    SOPC_NodeId_Clear(&nid);
    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);
    return 0;
}

/***************************************************/
static int cmd_demo_write(WordList* pList)
{
    const char* cnxStr = CLI_GetNextWord(pList);
    const char* nodeIdC = CLI_GetNextWord(pList);
    const char* dvC = CLI_GetNextWord(pList);
    if (dvC[0] == 0)
    {
        PRINT("usage: demo write <cnx_index> <nodeid> <value>\n");
        PRINT("<value> must be prefixed by b for a BOOL, s for a String, B for a byte,\n");
        PRINT("        i for a INT32, u for a UINT32\n");
        PRINT("Other formats not implemented here.\n");
        return 0;
    }
    int cnxIndex;
    int n = sscanf(cnxStr, "%d", &cnxIndex);
    if (n == 0 || IS_INVALID_CFG_IDX(cnxIndex))
    {
        PRINT(STR_INVALID_INDEX);
        return 0;
    }
    SOPC_NodeId nid;
    SOPC_ReturnStatus status = SOPC_NodeId_InitializeFromCString(&nid, nodeIdC, (int32_t) strlen(nodeIdC));
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_DataValue dv;
    SOPC_DataValue_Initialize(&dv);

    dv.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    dv.Value.DoNotClear = false;
    if (dvC[0] == 's')
    {
        dv.Value.BuiltInTypeId = SOPC_String_Id;
        status = SOPC_String_InitializeFromCString(&dv.Value.Value.String, dvC + 1);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }
    else if (dvC[0] == 'b')
    {
        dv.Value.BuiltInTypeId = SOPC_Boolean_Id;

        dv.Value.Value.Boolean = (bool) atoi(dvC + 1);
    }
    else if (dvC[0] == 'B')
    {
        dv.Value.BuiltInTypeId = SOPC_Byte_Id;

        dv.Value.Value.Byte = (SOPC_Byte) atoi(dvC + 1);
    }
    else if (dvC[0] == 'i')
    {
        dv.Value.BuiltInTypeId = SOPC_Int32_Id;

        dv.Value.Value.Int32 = (int32_t) atoi(dvC + 1);
    }
    else if (dvC[0] == 'u')
    {
        dv.Value.BuiltInTypeId = SOPC_UInt32_Id;

        dv.Value.Value.Uint32 = (uint32_t) atoi(dvC + 1);
    }
    else
    {
        PRINT("Invalid format for <value>\n");
        return 0;
    }

    Server_WriteSingleNode(&nid, &dv, cnxIndex);
    SOPC_NodeId_Clear(&nid);
    SOPC_DataValue_Clear(&dv);
    return 0;
}

/***************************************************/
static int cmd_demo_connect(WordList* pList)
{
    const char* cnxStr = CLI_GetNextWord(pList);

    if (cnxStr[0] == 0)
    {
        PRINT("usage: demo conn <cnxIndex>\n");
        return 0;
    }
    int cnxIndex;
    int n = sscanf(cnxStr, "%d", &cnxIndex);
    if (n == 0 || IS_INVALID_CFG_IDX(cnxIndex))
    {
        PRINT(STR_INVALID_INDEX);
        return 0;
    }
    SOPC_ReturnStatus status = SOPC_ClientHelperNew_Disconnect(&gMultiConnection[cnxIndex]);

    if (NULL == gMultiConfiguration[cnxIndex])
    {
        PRINT("\nClient not configured!\n");

        return 0;
    }
    status = SOPC_ClientHelperNew_Connect(gMultiConfiguration[cnxIndex], client_ConnectionEventCallback,
                                          &gMultiConnection[cnxIndex]);

    if (SOPC_STATUS_OK != status)
    {
        PRINT("\nSOPC_ClientHelper_Connect failed with status %d\r\n", status);
    }
    else
    {
        client_tester(cnxIndex);
    }
    return 0;
}

/***************************************************/
static int cmd_demo_list(WordList* pList)
{
    SOPC_UNUSED_ARG(pList);
    PRINT("\nList of configured clients:\r\n");
    for (int i = 0; i < MAX_CONFIG; i++)
    {
        if (NULL != gMultiConfiguration[i])
        {
            PRINT("\ncnxIndex : %d", i);
            Server_GetEndpoint(gMultiConfiguration[i]);
        }
    }
    return 0;
}

/***************************************************/
static int cmd_demo_disconnect(WordList* pList)
{
    const char* cnxStr = CLI_GetNextWord(pList);

    if (cnxStr[0] == 0)
    {
        PRINT("usage: demo disc <cnxIndex>\n");
        return 0;
    }
    int cnxIndex;
    int n = sscanf(cnxStr, "%d", &cnxIndex);
    if (n == 0 || IS_INVALID_CFG_IDX(cnxIndex))
    {
        PRINT(STR_INVALID_INDEX);
        return 0;
    }

    if (NULL == gMultiConfiguration[cnxIndex])
    {
        PRINT("\nClient not configured!\n");
        return 0;
    }

    if (NULL == gMultiConnection[cnxIndex])
    {
        PRINT("\nClient already disconnected!\n");
        return 0;
    }
    SOPC_ReturnStatus status = SOPC_ClientHelperNew_Disconnect(&gMultiConnection[cnxIndex]);
    if (SOPC_STATUS_OK != status)
    {
        PRINT("\nSOPC_ClientHelper_Disconnect failed with code %d\r\n", status);
    }
    else
    {
        PRINT("\nDisconnected\r\n");
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
