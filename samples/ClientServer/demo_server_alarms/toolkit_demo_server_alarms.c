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

/*

Alarms behavior principles are the following:

Ai => Alarm Inactive
Aa => Alarm Active
Ak => Alarm active and acKnowledged

TRUE => Condition source is true
FALSE => Condition source is false

Acked => Active alarm is acknowledged (Acknowledge() called)
Confirmed => Active alarm is confirmed (Confirm() called)
_________________________________________________

Alarm states without confirmation (AlarmNoConf):

   |===================Acked==============|
   \/                                    ||
(Ai+FALSE) =TRUE=> (Aa+TRUE) =FALSE=> (Aa+FALSE)
       /\              ||
        \              ||
         \             ||
          \            ||
           \           ||
            \          ||
           FALSE      Acked
              \        ||
               \       ||
                \      ||
                 \     ||
                  \    ||
                   \   ||
                    \  \/
                    (Ak+TRUE)

_________________________________________________

Alarm states with confirmation (AlarmWithConf):

(Ai+FALSE) =TRUE=> (Aa+TRUE) =FALSE=> (Aa+FALSE)
        /\             ||              /
         \            Acked           /
          \            ||            /
           \           \/           /
            \       (Ak+TRUE)      /
             \         ||         /
         Confirmed     ||      Acked
               \       ||       /
                \      ||      /
                 \    FALSE   /
                  \    ||    /
                   \   ||   /
                    \  \/  \/
                   (Ak+FALSE)
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h> /* getenv, exit */
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_alarm_conditions.h"
#include "libs2opc_server_config.h"

#include "demo_server_alarms_mgr.h"

/* ---------------------------------------------------------------------------
 *                          Manage server stop phase
 * ---------------------------------------------------------------------------*/

// Periodic timeout used to check if server should stop or has been stopped
#define UPDATE_STOP_TIMEOUT_MS 500

static int32_t stopRequested = 0;

/*
 * Management of Ctrl-C to stop the server (callback on stop signal)
 */
static void SOPC_Internal_StopSignal(int sig)
{
    /* avoid unused parameter compiler warning */
    SOPC_UNUSED_ARG(sig);

    /*
     * Signal steps:
     * - 1st signal: activate server shutdown phase of OPC UA server
     * - 2rd signal: abrupt exit with error code '1'
     */
    if (stopRequested > 0)
    {
        exit(1);
    }
    else
    {
        stopRequested++;
    }
}

static int32_t atomicStopped = false;

static void SOPC_ServerStopped_Cb(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&atomicStopped, true);
}

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

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
            "TEST_SERVER_XML_ADDRESS_SPACE=./S2OPC_Demo_Alarms_NodeSet.xml "
            "TEST_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml "
            "%s\nThe following environment variables are missing: %s%s%s\n",
            argv0, server_config_missing, addspace_config_missing, users_config_missing);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    return SOPC_ServerConfigHelper_ConfigureFromXML(xml_server_config_path, xml_address_space_config_path,
                                                    xml_users_config_path, NULL);
}

/*-------------------------
 * Method call management :
 *-------------------------*/

static SOPC_MethodCallManager* Server_InitCallMethodService(void)
{
    /* Create and define the method call manager the server will use*/
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    SOPC_ReturnStatus status = (NULL != mcm) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetMethodCallManager(mcm);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_MethodCallManager_Free(mcm);
        mcm = NULL;
    }

    return mcm;
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    // Note: avoid unused parameter warning from compiler
    SOPC_UNUSED_ARG(argc);

    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, SOPC_Internal_StopSignal);
    signal(SIGTERM, SOPC_Internal_StopSignal);

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

    /* Initialize the server library (start library threads) */
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfig, NULL);
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

    /* Define method call manager */
    SOPC_MethodCallManager* mcm = NULL;
    if (SOPC_STATUS_OK == status)
    {
        mcm = Server_InitCallMethodService();
        status = (NULL == mcm ? SOPC_STATUS_NOK : SOPC_STATUS_OK);
    }

    /* Initialize the Alarm and Condition module */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerAlarmConditionMgr_Initialize(mcm);
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Demo_Server_Alarms: Server started\n");

        /* Run the server until error or stop server signal detected (Ctrl-C) */
        status = SOPC_ServerHelper_StartServer(&SOPC_ServerStopped_Cb);

        if (SOPC_STATUS_OK == status)
        {
            status = Demo_Server_InitializeAlarms();
            while (SOPC_STATUS_OK == status && 0 == stopRequested && false == SOPC_Atomic_Int_Get(&atomicStopped))
            {
                SOPC_Sleep(UPDATE_STOP_TIMEOUT_MS);
            }

            Demo_Server_PreStopAlarms();

            status = SOPC_ServerHelper_StopServer();

            while (SOPC_STATUS_OK == status && false == SOPC_Atomic_Int_Get(&atomicStopped))
            {
                SOPC_Sleep(UPDATE_STOP_TIMEOUT_MS);
            }

            Demo_Server_ClearAlarms();
        }

        if (SOPC_STATUS_OK != status)
        {
            printf("<Demo_Server_Alarms: Failed to run the server or end to serve with error = '%d'\n", status);
        }
        else
        {
            printf("<Demo_Server_Alarms: Server ended to serve successfully\n");
        }
    }
    else
    {
        printf("<Demo_Server_Alarms: Error during configuration phase, see logs in %s directory for details.\n",
               logDirPath);
    }

    /* Clear the AlarmCondition manager module */
    SOPC_ServerAlarmConditionMgr_Clear();

    /* Clear the server library (stop all library threads) and server configuration */
    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK != status)
    {
        printf("<Demo_Server_Alarms: Terminating with error status, see logs in %s directory for details.\n",
               logConfig.logSysConfig.fileSystemLogConfig.logDirPath);
    }
    SOPC_Free(logDirPath);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
