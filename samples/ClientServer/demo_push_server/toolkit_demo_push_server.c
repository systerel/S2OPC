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

/** \file
 *
 * \brief  Demo server implementing the Push Model of the certificate management.
 *         The server shall not be Nano compliant since CallMethod service is not available.
 *         S2OPC shall be build with S2OPC_DYNAMIC_TYPE_RESOLUTION option.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "sopc_askpass.h"
#include "sopc_enums.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_push_server_config_itf.h"
#include "sopc_user_app_itf.h"

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

/* Server callback definition to ask for private key password during configuration phase */
static bool DemoPushSrv_PrivateKeyAskPass_FromTerminal(char** outPassword)
{
    return SOPC_AskPass_CustomPromptFromTerminal("Private key password:\n", outPassword);
}

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus DemoPushSrv_LoadConfigurationFromFiles(void)
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
            "TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo_Push_AddSpace.xml "
            "TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml\n"
            "The following environment variables are missing: %s%s%s\n",
            server_config_missing, addspace_config_missing, users_config_missing);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    return SOPC_HelperConfigServer_ConfigureFromXML(xml_server_config_path, xml_address_space_config_path,
                                                    xml_users_config_path, NULL);
}

static SOPC_ReturnStatus DemoPushSrv_AddServerConfigurationType(void)
{
    SOPC_MethodCallManager* pMcm = NULL;
    SOPC_S2OPC_Config* s2opcConfig = NULL;
    SOPC_Server_Config* serverConfig = NULL;
    SOPC_PushServerConfig_Config* pPushConfig = NULL;

    SOPC_ReturnStatus status = SOPC_PushServerConfig_Initialize();
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    s2opcConfig = SOPC_CommonHelper_GetConfiguration();
    serverConfig = &s2opcConfig->serverConfig;
    status = SOPC_PushServerConfig_GetDefaultConfiguration(
        serverConfig->pki, SOPC_CERT_TYPE_RSA_SHA256_APPLICATION, serverConfig->serverKey,
        serverConfig->serverCertificate, serverConfig->serverKeyPath, serverConfig->serverCertPath, NULL,
        SOPC_CERT_TYPE_UNKNOW, SOPC_TRUSTLIST_DEFAULT_MAX_SIZE, &pPushConfig);

    if (SOPC_STATUS_OK == status)
    {
        pMcm = SOPC_MethodCallManager_Create();
        status = NULL == pMcm ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetMethodCallManager(pMcm);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PushServerConfig_Configure(pPushConfig, pMcm);
    }

    SOPC_PushServerConfig_DeleteConfiguration(&pPushConfig);
    return status;
}

/*---------------------------------------------------------------------------
 *                             Logger configuration
 *---------------------------------------------------------------------------*/

/* Set the log path and set directory path built on executable name prefix */
static char* DemoPushSrv_ConfigLogPath(const char* logDirName)
{
    char* logDirPath = NULL;
    size_t logDirPathSize = strlen(logDirName) + 7; // <logDirName> + "_logs/" + '\0'
    logDirPath = SOPC_Malloc(logDirPathSize * sizeof(char));

    int res = snprintf(logDirPath, logDirPathSize, "%s_logs/", logDirName);
    if (NULL != logDirPath && (int) (logDirPathSize - 1) != res)
    {
        SOPC_Free(logDirPath);
        logDirPath = NULL;
    }
    return logDirPath;
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    SOPC_UNUSED_ARG(argc);

    /* Get the toolkit build information and print it */
    SOPC_Toolkit_Build_Info build_info = SOPC_CommonHelper_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    /* Configure the server logger */
    SOPC_Log_Configuration logConfig = SOPC_Common_GetDefaultLogConfiguration();
    logConfig.logLevel = SOPC_LOG_LEVEL_DEBUG;
    char* logDirPath = DemoPushSrv_ConfigLogPath(argv[0]);
    logConfig.logSysConfig.fileSystemLogConfig.logDirPath = logDirPath;

    /* Initialize the server library (start library threads) */
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfig);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_Initialize();
    }

    if (SOPC_STATUS_OK == status)
    {
        /* This function must be called after the initialization functions of the server library and
           before starting the server and its configuration. */
        status = SOPC_HelperConfigServer_SetKeyPasswordCallback(&DemoPushSrv_PrivateKeyAskPass_FromTerminal);
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
        status = DemoPushSrv_LoadConfigurationFromFiles();
    }

    if (SOPC_STATUS_OK == status)
    {
        status = DemoPushSrv_AddServerConfigurationType();
        if (SOPC_STATUS_OK != status)
        {
            printf("<Demo_Push_Server: Failed to load ServerConfigurationType\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Demo_Push_Server: Server started\n");

        /* Run the server until error  or stop server signal detected (Ctrl-C) */
        status = SOPC_ServerHelper_Serve(true);

        /**
         * Alternative is using SOPC_ServerHelper_StartServer to make server start asynchronously.
         * It is then possible to use local services to access / write data:
         * - Synchronously with SOPC_ServerHelper_LocalServiceSync(...)
         * - Asynchronously with SOPC_ServerHelper_LocalServiceAsync(...): need
         * SOPC_HelperConfigServer_SetLocalServiceAsyncResponse(...) to be configured prior to start server
         */

        if (SOPC_STATUS_OK != status)
        {
            printf("<Demo_Push_Server: Failed to run the server or end to serve with error = '%d'\n", status);
        }
        else
        {
            printf("<Demo_Push_Server: Server ended to serve successfully\n");
        }
    }
    else
    {
        printf("<Demo_Server: Error during configuration phase, see logs in %s directory for details.\n",
               logConfig.logSysConfig.fileSystemLogConfig.logDirPath);
    }

    /* Clear the server library (stop all library threads) and server configuration */
    SOPC_PushServerConfig_Clear();
    SOPC_HelperConfigServer_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK != status)
    {
        printf("<Demo_Server: Terminating with error status, see logs in %s directory for details.\n",
               logConfig.logSysConfig.fileSystemLogConfig.logDirPath);
    }
    SOPC_Free(logDirPath);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
