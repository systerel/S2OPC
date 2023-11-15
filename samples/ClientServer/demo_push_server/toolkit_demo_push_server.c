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
 *
 *         This demo server support the default application group (RsaSha256ApplicationCertificateType)
 *
 *         The server can start in TOFU mode (Trust On First Use). The TOFU mode allows to start the server
 *         with an empty PKI and allows a specific user to configure the TrustList for the first time.
 *         In TOFU mode, when a valid update is completed through TrustList.AddCertificate or TrustList.Write +
 *         TrustList.CloseAndUpdate methods then the server reboot with the new configuration of the TrustList.
 *         The TOFU mode works during a period and if the timeout has elapsed then the server is stopped.
 *
 *         To activate TOFU mode, you must set the TOFU period value from the binary command line eg:
 *         <./toolkit_demo_push_server 1> (server will start in TOFU state with a timeout of 1 minute)
 *         The users allowed in TOFU state are defined by an XML configuration file which is retrieved by
 *         the environemnt variable TEST_TOFU_USERS_XML_CONFIG.
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_enums.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_push_server_config_itf.h"
#include "sopc_user_app_itf.h"

#define DEMO_PUSH_M_TO_MS 60000

/*---------------------------------------------------------------------------
 *                          Server variables content
 *---------------------------------------------------------------------------*/

typedef struct DemoPushSrv_AppCtx
{
    SOPC_Mutex mutex;
    SOPC_Condition cond;
    bool stopFlag;
    bool isTofu;
    bool rebootFlag;
    uint32_t tofuPeriod; // ms
    char* password;
} DemoPushSrv_AppCtx;

static DemoPushSrv_AppCtx gAppCtx = {0};

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

/* Server callback definition to ask for private key password during configuration phase */
static bool DemoPushSrv_PrivateKeyAskPass_FromTerminal(char** outPassword)
{
    bool res = false;
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&gAppCtx.mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    if (NULL == gAppCtx.password)
    {
        res = SOPC_AskPass_CustomPromptFromTerminal("Private key password:\n", outPassword);
        gAppCtx.password = SOPC_strdup(*outPassword);
    }
    else
    {
        *outPassword = SOPC_strdup(gAppCtx.password);
        res = true;
    }
    mutStatus = SOPC_Mutex_Unlock(&gAppCtx.mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    return res;
}

static void DemoPushSrv_StoppedCb(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&gAppCtx.mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    printf("<Demo_Push_Server: Server stop callback\n");
    gAppCtx.stopFlag = true;
    SOPC_Condition_SignalAll(&gAppCtx.cond);
    mutStatus = SOPC_Mutex_Unlock(&gAppCtx.mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
}

static void DemoPushSrv_StopSignal(int arg)
{
    SOPC_UNUSED_ARG(arg);
    SOPC_UNUSED_RESULT(SOPC_ServerHelper_StopServer());
}

static void DemoPushSrv_TrustList_UpdateCompleted(uintptr_t context)
{
    SOPC_UNUSED_ARG(context);
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Lock(&gAppCtx.mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    printf("<Demo_Push_Server: Trustlist update is completed\n");
    gAppCtx.rebootFlag = true;
    SOPC_Condition_SignalAll(&gAppCtx.cond);
    mutStatus = SOPC_Mutex_Unlock(&gAppCtx.mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
}

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus DemoPushSrv_LoadConfigurationFromFiles(void)
{
    /* Retrieve XML configuration file path from environment variables TEST_SERVER_XML_CONFIG,
     * TEST_SERVER_XML_ADDRESS_SPACE, TEST_USERS_XML_CONFIG and TEST_TOFU_USERS_XML_CONFIG
     *
     * In case of success returns the file path otherwise display error message and return failure status.
     */
    const char* xml_server_config_path = getenv("TEST_SERVER_XML_CONFIG");
    const char* xml_address_space_config_path = getenv("TEST_SERVER_XML_ADDRESS_SPACE");
    const char* xml_users_config_path = NULL;

    if (gAppCtx.isTofu)
    {
        xml_users_config_path = getenv("TEST_TOFU_USERS_XML_CONFIG");
    }
    else
    {
        xml_users_config_path = getenv("TEST_USERS_XML_CONFIG");
    }

    if (NULL == xml_server_config_path || NULL == xml_address_space_config_path || NULL == xml_users_config_path)
    {
        const char* server_config_missing = (NULL == xml_server_config_path ? "TEST_SERVER_XML_CONFIG, " : "");
        const char* addspace_config_missing =
            (NULL == xml_address_space_config_path ? "TEST_SERVER_XML_ADDRESS_SPACE, " : "");
        const char* users_config_missing =
            (NULL == xml_users_config_path ? "TEST_USERS_XML_CONFIG or TEST_TOFU_USERS_XML_CONFIG" : "");
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
    if (gAppCtx.isTofu)
    {
        SOPC_PKIProvider_Free(&serverConfig->pki);
        status = SOPC_PKIPermissive_Create(&serverConfig->pki);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProvider_SetStorePath(serverConfig->serverPkiPath, serverConfig->pki);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PushServerConfig_GetTOFUConfiguration(
                serverConfig->pki, SOPC_CERT_TYPE_RSA_SHA256_APPLICATION, SOPC_TRUSTLIST_DEFAULT_MAX_SIZE,
                &DemoPushSrv_TrustList_UpdateCompleted, &pPushConfig);
        }
    }
    else
    {
        status = SOPC_PushServerConfig_GetDefaultConfiguration(
            serverConfig->pki, SOPC_CERT_TYPE_RSA_SHA256_APPLICATION, serverConfig->serverKeyCertPair,
            serverConfig->serverKeyPath, serverConfig->serverCertPath, NULL, SOPC_CERT_TYPE_UNKNOW,
            SOPC_TRUSTLIST_DEFAULT_MAX_SIZE, &pPushConfig);
    }
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
 *                             Server function
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus DemoPushSrv_WaitUntilStop(void)
{
    SOPC_ReturnStatus mutStatus = SOPC_STATUS_OK;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (gAppCtx.isTofu)
    {
        status =
            SOPC_Mutex_UnlockAndTimedWaitCond(&gAppCtx.cond, &gAppCtx.mutex, DEMO_PUSH_M_TO_MS * gAppCtx.tofuPeriod);

        if (SOPC_STATUS_TIMEOUT == status)
        {
            printf("<Demo_Push_Server: TOFU timeout elapsed\n");
        }

        printf("<Demo_Push_Server: Server is leaving TOFU state ...\n");

        SOPC_UNUSED_RESULT(SOPC_ServerHelper_StopServer());
    }
    while (SOPC_STATUS_OK == mutStatus && !gAppCtx.stopFlag)
    {
        mutStatus = SOPC_Mutex_UnlockAndWaitCond(&gAppCtx.cond, &gAppCtx.mutex);
    }

    return status;
}

static SOPC_ReturnStatus DemoPushSrv_MainCommon(const char* logName)
{
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
    char* logDirPath = DemoPushSrv_ConfigLogPath(logName);
    logConfig.logSysConfig.fileSystemLogConfig.logDirPath = logDirPath;

    /* Initialize the server library (start library threads) */
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfig);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_Initialize();
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetKeyPasswordCallback(&DemoPushSrv_PrivateKeyAskPass_FromTerminal);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = DemoPushSrv_LoadConfigurationFromFiles();
        if (SOPC_STATUS_OK != status)
        {
            printf("<Demo_Push_Server: Failed to load configuration files (error = '%d')\n", status);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = DemoPushSrv_AddServerConfigurationType();
        if (SOPC_STATUS_OK != status)
        {
            printf("<Demo_Push_Server: Failed to add the ServerConfigurationType (error = '%d')\n", status);
        }
    }

    /* Signals configuration to stop the server */
    signal(SIGINT, DemoPushSrv_StopSignal);
    signal(SIGTERM, DemoPushSrv_StopSignal);

    if (SOPC_STATUS_OK == status)
    {
        /* Run the server  */
        status = SOPC_ServerHelper_StartServer(&DemoPushSrv_StoppedCb);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Demo_Push_Server: Failed to run the server or end to serve with error = '%d'\n", status);
        }
        else
        {
            char* state = gAppCtx.isTofu ? "and in TOFU state\n" : "\n";
            printf("<Demo_Push_Server: Server started %s", state);
            if (gAppCtx.rebootFlag)
            {
                SOPC_Free(gAppCtx.password);
                gAppCtx.password = NULL;
            }
        }
    }
    else
    {
        printf("<Demo_Push_Server: Error during configuration phase, see logs in %s directory for details.\n",
               logConfig.logSysConfig.fileSystemLogConfig.logDirPath);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = DemoPushSrv_WaitUntilStop();
    }

    /* Clear the server library (stop all library threads) and server configuration */
    SOPC_PushServerConfig_Clear();
    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK != status)
    {
        printf("<Demo_Push_Server: Terminating with error status, see logs in %s directory for details.\n",
               logConfig.logSysConfig.fileSystemLogConfig.logDirPath);
    }
    else
    {
        printf("<Demo_Push_Server: Server ended to serve successfully\n");
    }
    SOPC_Free(logDirPath);
    return status;
}

static SOPC_ReturnStatus DemoPushSrv_SetArg(int argc, char* argv[])
{
    errno = 0;
    bool bBadUsage = true;
    gAppCtx.isTofu = false;
    if (1 == argc)
    {
        bBadUsage = false;
    }
    if (2 == argc)
    {
        char* end = NULL;
        int period = (int) strtol(argv[1], (char**) &end, 0);
        if ('\0' == end[0] && 0 == errno && 0 <= period)
        {
            if (0 != period)
            {
                if ((uint32_t) period < UINT32_MAX / DEMO_PUSH_M_TO_MS)
                {
                    bBadUsage = false;
                    gAppCtx.tofuPeriod = (uint32_t) period;
                    gAppCtx.isTofu = true;
                }
            }
            else
            {
                bBadUsage = false;
            }
        }
    }

    if (bBadUsage)
    {
        printf("toolkit_demo_push_server: Invalid command\n");
        printf("Usage: ./toolkit_demo_push_server <optional integer in range [0;%d[>\n",
               (UINT32_MAX / DEMO_PUSH_M_TO_MS));
        printf("Example: ./toolkit_demo_push_server 1 (Server will start in TOFU state with a timeout of 1 minute)\n");
    }

    return bBadUsage ? SOPC_STATUS_INVALID_PARAMETERS : SOPC_STATUS_OK;
}

static void DemoPushSrv_InitAppCtx(void)
{
    memset(&gAppCtx, 0, sizeof(struct DemoPushSrv_AppCtx));

    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Initialization(&gAppCtx.mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    mutStatus = SOPC_Mutex_Lock(&gAppCtx.mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    mutStatus = SOPC_Condition_Init(&gAppCtx.cond);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
}

static void DemoPushSrv_ClearAppCtx(void)
{
    SOPC_ReturnStatus mutStatus = SOPC_Mutex_Unlock(&gAppCtx.mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    mutStatus = SOPC_Mutex_Clear(&gAppCtx.mutex);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    mutStatus = SOPC_Condition_Clear(&gAppCtx.cond);
    SOPC_ASSERT(SOPC_STATUS_OK == mutStatus);
    SOPC_Free(gAppCtx.password);
    memset(&gAppCtx, 0, sizeof(struct DemoPushSrv_AppCtx));
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    DemoPushSrv_InitAppCtx();
    SOPC_ReturnStatus status = DemoPushSrv_SetArg(argc, argv);
    if (SOPC_STATUS_OK != status)
    {
        return 1;
    }

    status = DemoPushSrv_MainCommon(argv[0]);
    if (SOPC_STATUS_OK == status && gAppCtx.rebootFlag && gAppCtx.isTofu)
    {
        printf("<Demo_Push_Server: Server reboot with a new configuration\n");
        gAppCtx.isTofu = false;
        gAppCtx.stopFlag = false;
        status = DemoPushSrv_MainCommon(argv[0]);
    }

    DemoPushSrv_ClearAppCtx();

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
