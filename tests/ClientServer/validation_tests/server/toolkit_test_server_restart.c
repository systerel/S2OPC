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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h> /* getenv, exit */
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common_constants.h"
#include "sopc_helper_askpass.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "embedded/sopc_addspace_loader.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"

static const char* default_locale_ids[] = {"en-US", "fr-FR"};

/* ---------------------------------------------------------------------------
 *                          Manage server restart cycles
 * ---------------------------------------------------------------------------*/
// Number of full restart cycle (server stop, server clear, common clear, ..., server start)
const uint8_t NB_FULL_CONFIG = 2;
// Number of restart with server configuration only cycle (server stop, server clear, ..., server start)
const uint8_t NB_SERVER_CONFIG = 2;
// Number of restart with server restart only cycle (server stop, server start)
const uint8_t NB_SERVER_RESTART = 2;

uint8_t nbServerConfigCountCount = 0;

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
 * Server callback definition used for address space modification notification
 */
static void Demo_WriteNotificationCallback(const SOPC_CallContext* callContextPtr,
                                           OpcUa_WriteValue* writeValue,
                                           SOPC_StatusCode writeStatus)
{
    const SOPC_User* user = SOPC_CallContext_GetUser(callContextPtr);
    const char* writeSuccess = (SOPC_STATUS_OK == writeStatus ? "success" : "failure");
    char* sNodeId = SOPC_NodeId_ToCString(&writeValue->NodeId);
    SOPC_Logger_TraceDebug(
        SOPC_LOG_MODULE_CLIENTSERVER,
        "Write notification (%s) on node '%s' by user '%s' from client application URI=%s CertificateTB=%s",
        writeSuccess, sNodeId, SOPC_User_ToCString(user),
        SOPC_String_GetRawCString(&SOPC_CallContext_GetClientApplicationDesc(callContextPtr)->ApplicationUri),
        SOPC_CallContext_GetClientCertThumbprint(callContextPtr));
    SOPC_Free(sNodeId);
}

/*---------------------------------------------------------------------------
 *                          Server initialization
 *---------------------------------------------------------------------------*/

/* Set the log path and create (or keep existing) directory path built on executable path
 *  + first argument of main */
static char* Server_ConfigLogPath(int argc, char* argv[])
{
    const char* logDirName = "toolkit_test_server_restart";
    char* underscore = "_";
    char* suffix = NULL;
    char* logDirPath = NULL;

    if (argc > 1)
    {
        suffix = argv[1];
    }
    else
    {
        suffix = "";
        underscore = "";
    }

    size_t logDirPathSize = 2 + strlen(logDirName) + strlen(underscore) + strlen(suffix) +
                            7; // "./" + logDirName + _ + test_name + _logs/ + '\0'
    if (logDirPathSize < 200)
    {
        logDirPath = SOPC_Malloc(logDirPathSize * sizeof(char));
    }
    if (NULL != logDirPath && (int) (logDirPathSize - 1) != snprintf(logDirPath, logDirPathSize, "./%s%s%s_logs/",
                                                                     logDirName, underscore, suffix))
    {
        SOPC_Free(logDirPath);
        logDirPath = NULL;
    }

    return logDirPath;
}

static SOPC_ReturnStatus Common_Initialize(const char* logDirPath)
{
    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    if (NULL != logDirPath)
    {
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = logDirPath;
    }
    else
    {
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_logs/";
    }
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;

    return SOPC_CommonHelper_Initialize(&logConfiguration, NULL);
}

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

/*
 * Default server configuration loader (without XML configuration)
 */
static SOPC_ReturnStatus Server_SetDefaultConfiguration(void)
{
    // Set locale ids
    SOPC_ReturnStatus status =
        SOPC_ServerConfigHelper_SetLocaleIds(sizeof(default_locale_ids) / sizeof(char*), default_locale_ids);

    // Set application description of server to be returned by discovery services (GetEndpoints, FindServers)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                                   "S2OPC toolkit server example", "en-US",
                                                                   OpcUa_ApplicationType_Server);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_AddApplicationNameLocale("S2OPC toolkit: exemple de serveur", "fr-FR");
    }

    /*
     * Create new endpoint in server
     */
    SOPC_Endpoint_Config* ep = NULL;
    if (SOPC_STATUS_OK == status)
    {
        ep = SOPC_ServerConfigHelper_CreateEndpoint(DEFAULT_ENDPOINT_URL, true);
        status = NULL == ep ? SOPC_STATUS_OUT_OF_MEMORY : status;
    }

    /*
     * Define the certificates, security policies, security modes and user token policies supported by endpoint
     */
    SOPC_SecurityPolicy* sp;
    /*
     * Security policy is None with anonymous and username authentication allowed
     * (for tests only, otherwise users on unsecure channel shall be forbidden
     *  and only discovery endpoint activated on a secured channel configuration)
     */
    if (SOPC_STATUS_OK == status)
    {
        sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_None);
        if (NULL == sp)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SecurityModeMask_None);

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_SecurityConfig_AddUserTokenPolicy(
                    sp, &SOPC_UserTokenPolicy_Anonymous); /* Necessary for tests only */
            }
        }
    }

    return status;
}

/*----------------------------------------
 * Users authentication and authorization:
 *----------------------------------------*/

static SOPC_ReturnStatus Server_SetDefaultUserManagementConfig(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_UserAuthorization_Manager* authorizationManager = NULL;
    SOPC_UserAuthentication_Manager* authenticationManager = NULL;

    /* Create an user authorization manager which allow all rights to any user.
     * i.e.: UserAccessLevel right == AccessLevel right for any user for a given node of address space */
    authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
    if (NULL == authorizationManager)
    {
        printf("<Test_Server_Restart: Failed to create the user authorization manager\n");
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        authenticationManager = SOPC_UserAuthentication_CreateManager_AllowAll();
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ServerConfigHelper_SetUserAuthenticationManager(authenticationManager);
        SOPC_ServerConfigHelper_SetUserAuthorizationManager(authorizationManager);
    }
    else
    {
        /* clear */
        SOPC_UserAuthorization_FreeManager(&authorizationManager);
        printf("<Test_Server_Restart: Failed to create the user authentication manager: %d\n", status);
    }

    return status;
}

/*------------------------------
 * Address space configuration :
 *------------------------------*/

static SOPC_ReturnStatus Server_SetDefaultAddressSpace(void)
{
    /* Load embedded default server address space:
     * Use the embedded address space (already defined as C code) loader.
     * The address space C structure shall have been generated prior to compilation.
     * This should be done using the script ./scripts/generate-s2opc-address-space.py
     */

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_AddressSpace* addSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    status = (NULL != addSpace) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetAddressSpace(addSpace);
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Restart: Failed to configure the @ space\n");
    }
    else
    {
        printf("<Test_Server_Restart: @ space configured\n");
    }

    return status;
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
        status = SOPC_ServerConfigHelper_SetMethodCallManager(mcm);
    }
    // Add no methods

    return status;
}

static SOPC_ReturnStatus Server_LoadServerConfiguration(void)
{
    /* Retrieve XML configuration file path from environment variables TEST_SERVER_XML_CONFIG,
     * TEST_SERVER_XML_ADDRESS_SPACE and TEST_USERS_XML_CONFIG.
     *
     * In case of success returns the file path otherwise load default configuration.
     */

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    const char* xml_server_config_path = NULL;
    const char* xml_address_space_config_path = NULL;
    const char* xml_users_config_path = NULL;

    // Define a callback to retrieve the server key password (from environment variable)
    status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);

    if (SOPC_STATUS_OK == status && nbServerConfigCountCount == 2)
    {
        xml_server_config_path = getenv("TEST_SERVER_XML_CONFIG");
        xml_address_space_config_path = getenv("TEST_SERVER_XML_ADDRESS_SPACE");
        xml_users_config_path = getenv("TEST_USERS_XML_CONFIG");
    }

    // Set default server configuration if no XML path provided
    if (SOPC_STATUS_OK == status && NULL == xml_server_config_path)
    {
        status = Server_SetDefaultConfiguration();
    }

    if (SOPC_STATUS_OK == status &&
        (NULL != xml_server_config_path || NULL != xml_address_space_config_path || NULL != xml_users_config_path))
    {
        status = SOPC_ServerConfigHelper_ConfigureFromXML(xml_server_config_path, xml_address_space_config_path,
                                                          xml_users_config_path, NULL);
    }

    if (SOPC_STATUS_OK == status && NULL == xml_address_space_config_path)
    {
        status = Server_SetDefaultAddressSpace();
    }

    if (SOPC_STATUS_OK == status && NULL == xml_users_config_path)
    {
        status = Server_SetDefaultUserManagementConfig();
    }

    return status;
}

static SOPC_StatusCode Test_OverwriteClientRequestCallback(const SOPC_CallContext* callCtxPtr,
                                                           SOPC_EncodeableType* type,
                                                           void* request)
{
    /* Do nothing callback */
    SOPC_UNUSED_ARG(callCtxPtr);
    SOPC_UNUSED_ARG(type);
    SOPC_UNUSED_ARG(request);
    return SOPC_GoodGenericStatus;
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, SOPC_Internal_StopSignal);
    signal(SIGTERM, SOPC_Internal_StopSignal);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Get the toolkit build information and print it */
    SOPC_Toolkit_Build_Info build_info = SOPC_CommonHelper_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    /* Configure the server logger:
     * DEBUG traces generated in ./toolkit_server_<argv[1]>_logs/ */
    char* logDirPath = Server_ConfigLogPath(argc, argv);

    /* Make a full config (common + server) 2 times*/
    uint8_t nbFullConfigCount = 0;
    while (nbFullConfigCount < NB_FULL_CONFIG)
    {
        nbFullConfigCount++;

        /* Initialize the server library (start library threads) */
        status = Common_Initialize(logDirPath);
        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Restart: COMMON initialized #%" PRIu8 "\n", nbFullConfigCount);
        }
        else
        {
            printf("<Test_Server_Restart: Failed COMMON initialization #%" PRIu8 "\n", nbFullConfigCount);
        }

        /* Make a server config only 2 times */
        nbServerConfigCountCount = 0;
        while (SOPC_STATUS_OK == status && nbServerConfigCountCount < NB_SERVER_CONFIG)
        {
            nbServerConfigCountCount++;

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ServerConfigHelper_Initialize();
            }
            if (SOPC_STATUS_OK != status)
            {
                printf("<Test_Server_Restart: Failed SERVER initializing #%" PRIu8 ".%" PRIu8 "\n", nbFullConfigCount,
                       nbServerConfigCountCount);
            }
            else
            {
                printf("<Test_Server_Restart: SERVER initialized #%" PRIu8 ".%" PRIu8 "\n", nbFullConfigCount,
                       nbServerConfigCountCount);
            }

            if (SOPC_STATUS_OK == status)
            {
                status = Server_LoadServerConfiguration();
            }

            // Define demo implementation of functions called for method call service
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

            /* Define overwrite client request callback */
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ServerConfigHelper_SetOverwriteRequestCb(Test_OverwriteClientRequestCallback);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "Failed to configure the overwrite client request callback");
                }
            }

            /* Reduce shutdown phase duration for tests */
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ServerConfigHelper_SetShutdownCountdown(1);
            }

            /* Start the server: 2 times */
            uint8_t nbRestartCountRem = 0;
            while (nbRestartCountRem < NB_SERVER_RESTART)
            {
                nbRestartCountRem++;
                if (SOPC_STATUS_OK == status)
                {
                    /* Run the server until error or stop server signal detected (Ctrl-C) */
                    status = SOPC_ServerHelper_StartServer(&SOPC_ServerStopped_Cb);

                    if (SOPC_STATUS_OK == status)
                    {
                        printf("<Test_Server_Restart: Server STARTED #%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
                               nbFullConfigCount, nbServerConfigCountCount, nbRestartCountRem);
                        SOPC_Atomic_Int_Set(&atomicStopped, false);
                        SOPC_Sleep(UPDATE_STOP_TIMEOUT_MS);
                    }
                    else
                    {
                        printf("<Test_Server_Restart: Failed to START server #%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
                               nbFullConfigCount, nbServerConfigCountCount, nbRestartCountRem);
                    }
                    if (SOPC_STATUS_OK == status)
                    {
                        status = SOPC_ServerHelper_StopServer();
                    }

                    if (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&atomicStopped) == true)
                    {
                        printf("<Test_Server_Restart: Server STOPPED #%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
                               nbFullConfigCount, nbServerConfigCountCount, nbRestartCountRem);
                    }
                    else
                    {
                        printf("<Test_Server_Restart: Failed to STOP server #%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
                               nbFullConfigCount, nbServerConfigCountCount, nbRestartCountRem);
                    }
                }
                else
                {
                    printf(
                        "<Test_Server_Restart: Error during configuration phase, see logs in %s directory for "
                        "details.\n",
                        logDirPath);
                }
            }

            /* Clear the server library (stop all library threads) and server configuration */
            SOPC_ServerConfigHelper_Clear();
            printf("<Test_Server_Restart: SERVER clearing #%" PRIu8 ".%" PRIu8 "\n", nbFullConfigCount,
                   nbServerConfigCountCount);
        }
        SOPC_CommonHelper_Clear();
        printf("<Test_Server_Restart: COMMON clearing #%" PRIu8 "\n", nbFullConfigCount);
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Restart: Terminating with error status, see logs in %s directory for details.\n",
               logDirPath);
    }

    // Free the string containing log path
    SOPC_Free(logDirPath);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
