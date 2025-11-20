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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

#include "sopc_address_space_access.h"
#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_audit.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "toolkit_demo_server_methods.h"

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

/*
 * Server callback definition used for address space modification notification
 */
static void Demo_WriteNotificationCallback(const SOPC_CallContext* callContextPtr,
                                           OpcUa_WriteValue* writeValue,
                                           SOPC_StatusCode writeStatus)
{
    const SOPC_User* user = SOPC_CallContext_GetUser(callContextPtr);
    const char* writeSuccess = (SOPC_STATUS_OK == writeStatus ? "success" : "failure");
    char* sNodeId = SOPC_NodeId_ToCString(&writeValue->NodeId);
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "Write notification (%s) on node '%s' by user '%s'",
                           writeSuccess, sNodeId, SOPC_User_ToCString(user));
    SOPC_Free(sNodeId);
}

#ifdef S2OPC_EXTERNAL_HISTORY_RAW_READ_SERVICE
/*
 * Server callback definition for external history read
 */
static void Demo_ExternalHistoryReadCallback(const OpcUa_ReadRawModifiedDetails* readRawDetails,
                                             bool sourceTsRequired,
                                             bool releaseContinuationPoint,
                                             const OpcUa_HistoryReadValueId* singleValueId,
                                             uintptr_t userContext,
                                             SOPC_StatusCode* outStatusCode,
                                             SOPC_ByteString** outContinuationPoint,
                                             int32_t* outNbDataValues,
                                             SOPC_DataValue** outDataValues)
{
    SOPC_UNUSED_ARG(readRawDetails);
    SOPC_UNUSED_ARG(sourceTsRequired);
    SOPC_UNUSED_ARG(releaseContinuationPoint);
    SOPC_UNUSED_ARG(singleValueId);
    SOPC_UNUSED_ARG(userContext);
    SOPC_UNUSED_ARG(outStatusCode);
    SOPC_UNUSED_ARG(outContinuationPoint);
    SOPC_UNUSED_ARG(outNbDataValues);
    SOPC_UNUSED_ARG(outDataValues);

    /* There is a real example in test server */
}
#endif

/*
 * Server callback definition to ask for private key password during configuration phase
 */
static bool SOPC_PrivateKeyAskPass_FromTerminal(char** outPassword)
{
    bool retAskPass = SOPC_AskPass_CustomPromptFromTerminal("Private key password:\n", outPassword);
    if (!retAskPass)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "> Failed to get password from terminal !");
    }
    return retAskPass;
}

/*-----------------------
 * Logger configuration :
 *-----------------------*/

/* Set the log path and set directory path built on executable name prefix */
static char* Server_ConfigLogPath(const char* logDirName)
{
    char* logDirPath = NULL;

    size_t logDirPathSize = strlen(logDirName) + 7; // <logDirName> + "_logs/" + '\0'

    logDirPath = SOPC_Malloc(logDirPathSize * sizeof(char));

    if (NULL != logDirPath &&
        (int) (logDirPathSize - 1) != snprintf(logDirPath, logDirPathSize, "%s_logs/", logDirName))
    {
        SOPC_Free(logDirPath);
        logDirPath = NULL;
    }

    return logDirPath;
}

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_LoadServerConfigurationFromFiles(char* argv0)
{
    /* Retrieve XML configuration file path from environment variables TEST_SERVER_XML_CONFIG,
     * TEST_SERVER_XML_ADDRESS_SPACE and TEST_USERS_XML_CONFIG.
     *
     * In case of success returns the file path otherwise display error message and return failure status.
     */

    const char* xml_server_config_path = getenv("TEST_SERVER_XML_CONFIG");
    const char* xml_address_space_config_path = getenv("TEST_SERVER_XML_ADDRESS_SPACE");
    const char* xml_users_config_path = getenv("TEST_USERS_XML_CONFIG");

    if (NULL == xml_server_config_path || NULL == xml_address_space_config_path || NULL == xml_users_config_path)
    {
        const char* server_config_missing = (NULL == xml_server_config_path ? "TEST_SERVER_XML_CONFIG, " : "");
        const char* addspace_config_missing =
            (NULL == xml_address_space_config_path ? "TEST_SERVER_XML_ADDRESS_SPACE, " : "");
        const char* users_config_missing = (NULL == xml_users_config_path ? "TEST_USERS_XML_CONFIG" : "");

        printf(
            "Error: an XML server configuration file path shall be provided, e.g.: "
            "TEST_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml "
            "TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo%s_NodeSet.xml "
            "TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml "
            "%s\nThe following environment variables are missing: %s%s%s\n",
            NULL == strstr(argv0, "nano") ? "" : "_Nano", argv0, server_config_missing, addspace_config_missing,
            users_config_missing);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return SOPC_ServerConfigHelper_ConfigureFromXML(xml_server_config_path, xml_address_space_config_path,
                                                    xml_users_config_path, NULL);
}

/*-------------------------
 * Method call management :
 *-------------------------*/

static SOPC_ReturnStatus Server_InitDefaultCallMethodService(void)
{
    /* Create and define the method call manager the server will use*/
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    SOPC_ReturnStatus status = (NULL != mcm) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        // Note: in case of Nano compliant server this manager will never be used
        // since CallMethod service is not available
        status = SOPC_ServerConfigHelper_SetMethodCallManager(mcm);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_DemoServerConfig_AddMethods(mcm);
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    // Note: avoid unused parameter warning from compiler
    SOPC_UNUSED_ARG(argc);

    /* Get the toolkit build information and print it */
    SOPC_Toolkit_Build_Info build_info = SOPC_CommonHelper_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    /* Configure the server logger:
     * DEBUG traces generated in ./<argv[0]_logs/ */
    SOPC_Log_Configuration logConfig = SOPC_Common_GetDefaultLogConfiguration();
    logConfig.logLevel = SOPC_LOG_LEVEL_DEBUG;
    char* logDirPath = Server_ConfigLogPath(argv[0]);
    logConfig.logSysConfig.fileSystemLogConfig.logDirPath = logDirPath;

    SOPC_Audit_Configuration* pAuditCfg = NULL;
#ifdef S2OPC_HAS_AUDITING
    // Initialize the toolkit library and define the log configuration
    SOPC_Audit_Configuration auditCfg;
    auditCfg.auditEntryPath = NULL;
    auditCfg.options = SOPC_Audit_DefaultSecuOptions;
    pAuditCfg = &auditCfg;
#endif

    /* Initialize the server library (start library threads) */
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfig, pAuditCfg);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_Initialize();
    }

    if (SOPC_STATUS_OK == status)
    {
        /* This function must be called after the initialization functions of the server library and
           before starting the server and its configuration. */
        status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_PrivateKeyAskPass_FromTerminal);
    }

    /* Configuration of:
     * - Server endpoints configuration from XML server configuration file (comply with s2opc_clientserver_config.xsd) :
         - Enpoint URL,
         - Security endpoint properties,
         - Cryptographic parameters,
         - Application description
       - Server address space initial content from XML configuration file (comply with UANodeSet.xsd)
       - User authentication and authorization management from XML configuration file
         (comply with s2opc_clientserver_users_config.xsd):
         - User credentials
         - User access rights
    */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_LoadServerConfigurationFromFiles(argv[0]);
    }

    /* Define demo implementation of functions called for method call service */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_InitDefaultCallMethodService();
    }

    /* Define address space write notification callback */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetWriteNotifCallback(Demo_WriteNotificationCallback);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Failed to configure the @ space modification notification callback");
        }
    }

#ifdef S2OPC_EXTERNAL_HISTORY_RAW_READ_SERVICE
    /* Define history read external callback */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetExternalHistoryRawReadCallback(Demo_ExternalHistoryReadCallback, 0);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Failed to configure the external history read callback");
        }
    }
#endif

    if (SOPC_STATUS_OK == status)
    {
        printf("<Demo_Server: Server started\n");

        /* Run the server until error  or stop server signal detected (Ctrl-C) */
        status = SOPC_ServerHelper_Serve(true);

        /**
         * Alternative is using SOPC_ServerHelper_StartServer to make server start asynchronously
         * (and then SOPC_ServerHelper_StopServer to stop it).
         * It is then possible to use local services to access / write data:
         * - Synchronously with SOPC_ServerHelper_LocalServiceSync(...)
         * - Asynchronously with SOPC_ServerHelper_LocalServiceAsync(...): need
         * SOPC_ServerConfigHelper_SetLocalServiceAsyncResponse(...) to be configured prior to start server
         */

        if (SOPC_STATUS_OK != status)
        {
            printf("<Demo_Server: Failed to run the server or end to serve with error = '%d'\n", status);
        }
        else
        {
            printf("<Demo_Server: Server ended to serve successfully\n");
        }
    }
    else
    {
        printf("<Demo_Server: Error during configuration phase, see logs in %s directory for details.\n",
               logConfig.logSysConfig.fileSystemLogConfig.logDirPath);
    }

    /* Clear the server library (stop all library threads) and server configuration */
    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK != status)
    {
        printf("<Demo_Server: Terminating with error status, see logs in %s directory for details.\n",
               logConfig.logSysConfig.fileSystemLogConfig.logDirPath);
    }
    SOPC_Free(logDirPath);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
