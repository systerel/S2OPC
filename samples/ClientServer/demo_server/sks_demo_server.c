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

/**
 * Demo server with Security Key Service (SKS) for Pub/Sub
 *
 * \note In this SKS demo server version any user (username/pwd)
 *       with execution authorization rights are allowed to call GetSecurityKeys
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_address_space_access.h"
#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "sopc_sk_builder.h"
#include "sopc_sk_manager.h"
#include "sopc_sk_provider.h"
#include "sopc_sk_scheduler.h"

#include "sks_demo_server_methods.h"
#include "sopc_sk_secu_group_managers.h"

/* SKS Constants */
// Period to init the scheduler is 1s
#define SKS_SCHEDULER_INIT_MSPERIOD 1000
// Key Lifetime is 10s
#define SKS_KEYLIFETIME 10000
// Number of keys generated randomly
#define SKS_NB_GENERATED_KEYS 5
// Maximum number of Security Keys managed. When the number of keys exceed this limit, only the valid Keys are kept
#define SKS_NB_MAX_KEYS 20

/* SKS Data  */
SOPC_SKscheduler* skScheduler = NULL;

/*---------------------------------------------------------------------------
 *             PubSub Security Key Service specific configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_SKS_Start(uint32_t nbSkMgr, SOPC_SKManager** skMgrArr)
{
    SOPC_ASSERT(nbSkMgr > 0 && NULL != skMgrArr);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Init SK Scheduler */
    if (SOPC_STATUS_OK == status)
    {
        skScheduler = SOPC_SKscheduler_Create();
        if (NULL == skScheduler)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    /* Init SK Builder and SK Provider */
    SOPC_SKBuilder* skBuilder = NULL;
    SOPC_SKProvider* skProvider = NULL;

    /* Init SK Provider : Create Random Keys */
    skProvider = SOPC_SKProvider_RandomPubSub_Create(SKS_NB_GENERATED_KEYS);
    if (NULL == skProvider)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Init SK Builder : adds Keys to Manager and removes obsolete Keys when maximum size is reached */
    SOPC_SKBuilder* skbAppend = NULL;
    if (SOPC_STATUS_OK == status)
    {
        skbAppend = SOPC_SKBuilder_Append_Create();
        if (NULL == skbAppend)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        skBuilder = SOPC_SKBuilder_Truncate_Create(skbAppend, SKS_NB_MAX_KEYS);
        if (NULL == skBuilder)
        {
            SOPC_SKBuilder_Clear(skbAppend);
            SOPC_Free(skbAppend);
            skbAppend = NULL;
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        if (NULL != skProvider)
        {
            SOPC_SKProvider_Clear(skProvider);
            SOPC_Free(skProvider);
            skProvider = NULL;
        }
    }

    /* Configure and register the SK Managers */
    SOPC_SK_SecurityGroup_Managers_Init();
    bool oneTaskAdded = false;
    for (uint32_t i = 0; SOPC_STATUS_OK == status && i < nbSkMgr; i++)
    {
        status = SOPC_SKManager_SetKeyLifetime(skMgrArr[i], SKS_KEYLIFETIME);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_String policy;
            SOPC_String_Initialize(&policy);
            SOPC_String_CopyFromCString(&policy, SOPC_SecurityPolicy_PubSub_Aes256_URI);
            status = SOPC_SKManager_SetSecurityPolicyUri(skMgrArr[i], &policy);
            SOPC_String_Clear(&policy);
        }

        if (SOPC_STATUS_OK == status)
        {
            bool res = SOPC_SK_SecurityGroup_SetSkManager(skMgrArr[i]);

            if (!res)
            {
                printf("<Test_SKS_Server: Failed to set SK Manager for security group %s\n",
                       skMgrArr[i]->securityGroupId);
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            /* Init the tasks with 1s */
            status =
                SOPC_SKscheduler_AddTask(skScheduler, skBuilder, skProvider, skMgrArr[i], SKS_SCHEDULER_INIT_MSPERIOD);
            if (SOPC_STATUS_OK == status)
            {
                oneTaskAdded = true;
            }
        }
    }
    if (oneTaskAdded)
    {
        // At least one task added: ownership transferred to scheduler
        skBuilder = NULL;
        skProvider = NULL;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SKscheduler_Start(skScheduler);
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Security Keys Service: Started\n");
    }
    else
    {
        if (NULL != skScheduler)
        {
            SOPC_SKscheduler_StopAndClear(skScheduler);
            skScheduler = NULL;
        }

        if (NULL != skProvider)
        {
            SOPC_SKProvider_Clear(skProvider);
            SOPC_Free(skProvider);
            skProvider = NULL;
        }

        if (NULL == skbAppend)
        {
            SOPC_SKBuilder_Clear(skbAppend);
            SOPC_Free(skbAppend);
            skbAppend = NULL;
        }

        if (NULL != skBuilder)
        {
            SOPC_SKBuilder_Clear(skBuilder);
            SOPC_Free(skBuilder);
            skBuilder = NULL;
        }

        printf("<Security Keys Service Error: Start failed\n");
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

/*-------------------------
 * Method call management :
 *-------------------------*/

static SOPC_ReturnStatus Server_InitSKScallMethodService(void)
{
    SOPC_NodeId methodTypeId;
    SOPC_NodeId methodId;
    SOPC_NodeId_Initialize(&methodTypeId);
    SOPC_NodeId_Initialize(&methodId);
    methodTypeId.Data.Numeric = OpcUaId_PubSubKeyServiceType_GetSecurityKeys;
    methodId.Data.Numeric = OpcUaId_PublishSubscribe_GetSecurityKeys;
    /* Create and define the method call manager the server will use*/
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    SOPC_ReturnStatus status = (NULL != mcm) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetMethodCallManager(mcm);
    }

    /* Add methods implementation in the method call manager used */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethodWithType(
            mcm, &methodId, &methodTypeId, &SOPC_Method_Func_PublishSubscribe_GetSecurityKeys, NULL, NULL);
    }

    return status;
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
    SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "Write notification (%s) on node '%s' by user '%s'",
                           writeSuccess, sNodeId, SOPC_User_ToCString(user));
    SOPC_Free(sNodeId);
}

/*
 * Server callback definition to ask for private key password during configuration phase
 */
static bool SOPC_PrivateKeyAskPass_FromTerminal(char** outPassword)
{
    return SOPC_AskPass_CustomPromptFromTerminal("Private key password:\n", outPassword);
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

    if (NULL != logDirPath)
    {
        int res = snprintf(logDirPath, logDirPathSize, "%s_logs/", logDirName);
        if (res != (int) (logDirPathSize - 1))
        {
            SOPC_Free(logDirPath);
            logDirPath = NULL;
        }
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

    const char* xml_server_config_path = getenv("SKS_SERVER_XML_CONFIG");
    const char* xml_address_space_config_path = getenv("SKS_SERVER_XML_ADDRESS_SPACE");
    const char* xml_users_config_path = getenv("SKS_USERS_XML_CONFIG");

    if (NULL == xml_server_config_path || NULL == xml_address_space_config_path || NULL == xml_users_config_path)
    {
        const char* server_config_missing = (NULL == xml_server_config_path ? "SKS_SERVER_XML_CONFIG, " : "");
        const char* addspace_config_missing =
            (NULL == xml_address_space_config_path ? "SKS_SERVER_XML_ADDRESS_SPACE, " : "");
        const char* users_config_missing = (NULL == xml_users_config_path ? "SKS_USERS_XML_CONFIG" : "");

        printf(
            "Error: an XML server configuration file path shall be provided, e.g.: "
            "SKS_SERVER_XML_CONFIG=./S2OPC_Server_Demo_Config.xml SKS_SERVER_XML_ADDRESS_SPACE=./s2opc_sks.xml "
            "SKS_USERS_XML_CONFIG=./S2OPC_Users_Demo_Config.xml "
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

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    if (argc > 1 && (0 == strcmp(argv[1], "help") || 0 == strcmp(argv[1], "-h") || 0 == strcmp(argv[1], "--help")))
    {
        printf("Usage: %s [security_group_id_1] [security_group_id_2] ...\n", argv[0]);
        printf("If no security group id is provided, a single SK Manager with empty security group id is created.\n");
        return 0;
    }

    /* Retrieve the security group ids and create the associated SK managers */
    uint32_t nbSecuGroups = 1;
    if (argc > 1)
    {
        nbSecuGroups = (uint32_t) argc - 1;
    }
    SOPC_SKManager** skMgrArr = SOPC_Calloc(nbSecuGroups, sizeof(SOPC_SKManager*));
    if (NULL == skMgrArr)
    {
        printf("<Test_SKS_Server: Failed to allocate SK Managers array\n");
        return -1;
    }
    bool freeSksManagers = true;
    if (argc > 1)
    {
        for (uint32_t i = 0; i < nbSecuGroups; i++)
        {
            // Create SK Manager with security group id from command line arguments
            skMgrArr[i] = SOPC_SKManager_Create(argv[i + 1], 0);
            if (NULL == skMgrArr[i])
            {
                printf("<Test_SKS_Server: Failed to create SK Manager for security group %s\n", argv[i + 1]);
                SOPC_Free(skMgrArr);
                return -2;
            }
        }
    }
    else
    {
        // Single SK Manager with empty security group id
        *skMgrArr = SOPC_SKManager_Create("", 0);
        if (NULL == *skMgrArr)
        {
            printf("<Test_SKS_Server: Failed to create SK Manager for security group empty id\n");
            return -2;
        }
    }

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
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfig);
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
         - Endpoint URL,
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

    // Define sks implementation of functions called for method call service
    if (SOPC_STATUS_OK == status)
    {
        status = Server_InitSKScallMethodService();
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

    /* Start SKS Scheduler */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SKS_Start(nbSecuGroups, skMgrArr);
        if (SOPC_STATUS_OK == status)
        {
            freeSksManagers = false; // Ownership of SK Managers is transferred to the security group managers
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<SKS_Demo_Server: Server started\n");

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
            printf("<SKS_Demo_Server: Failed to run the server or end to serve with error = '%d'\n", status);
        }
        else
        {
            printf("<SKS_Demo_Server: Server ended to serve successfully\n");
        }
    }
    else
    {
        printf("<SKS_Demo_Server: Error during configuration phase, see logs in %s directory for details.\n",
               logConfig.logSysConfig.fileSystemLogConfig.logDirPath);
    }

    /* Stop and clear SKS related modules */
    SOPC_SKscheduler_StopAndClear(skScheduler);
    SOPC_Free(skScheduler);
    SOPC_SK_SecurityGroup_Managers_Clear(); // skManagers are cleared here if freeSksManagers is false
    for (uint32_t i = 0; freeSksManagers && i < nbSecuGroups; i++) // otherwise do it here
    {
        SOPC_SKManager_Clear(skMgrArr[i]);
        SOPC_Free(skMgrArr[i]);
    }
    SOPC_Free(skMgrArr);

    /* Clear the server library (stop all library threads) and server configuration */
    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK != status)
    {
        printf("<SKS_Demo_Server: Terminating with error status, see logs in %s directory for details.\n",
               logConfig.logSysConfig.fileSystemLogConfig.logDirPath);
    }
    SOPC_Free(logDirPath);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
