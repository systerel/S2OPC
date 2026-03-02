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

#include "libs2opc_client.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "embedded/sopc_addspace_loader.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"

#define CLIENT_APPLICATION_NAME "S2OPC_TestClient"

// Client XML configuration
#define CLIENT_XML_CONFIG "S2OPC_Client_Wrapper_Config.xml"

// Server XML configurations
#define SERVER_XML_CONFIG "S2OPC_Server_UACTT_Config.xml"

static const char* default_locale_ids[] = {"en-US", "fr-FR"};

static const SOPC_NodeId serverCurrentTimeNodeId = SOPC_NODEID_NS0_NUMERIC(2258);

/* ---------------------------------------------------------------------------
 *                          Manage server restart cycles
 * ---------------------------------------------------------------------------*/
// Number of full restart cycle (server stop, server clear, common clear, ..., server start)
const uint8_t NB_FULL_CONFIG = 2;
// Number of restart with server configuration only cycle (server stop, server clear, ..., server start)
const uint8_t NB_SERVER_CONFIG = 2;
// Number of restart with server restart only cycle (server stop, server start)
const uint8_t NB_SERVER_RESTART = 2;

uint8_t nbServerConfigCount = 0;

// Number of restart with server restart only cycle (client disconnect, client connect)
const uint8_t NB_CLIENT_RECONNECT = 2;

uint8_t nbClientConfigCount = 0;

/*---------------------------------------------------------------------------
 *                             Client configuration
 *---------------------------------------------------------------------------*/

static void SOPC_ClientConnectionEventCb(SOPC_ClientConnection* config,
                                         SOPC_ClientConnectionEvent event,
                                         SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
    SOPC_ASSERT(false);
}

static const char* preferred_locale_ids[] = {"en-US", "fr-FR", NULL};

/* Event management global variables and functions */
#if S2OPC_EVENT_MANAGEMENT
/* Notifier used for generated events in the test */
static const SOPC_NodeId serverObjectId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Server);
/* Use a non-base event type to generate events */
static const SOPC_NodeId conditionTypeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_ConditionType);

/* Simple subscription callback printing 1st notification element */
static void SOPC_Client_SubscriptionNotification_Cb(const SOPC_ClientHelper_Subscription* subscription,
                                                    SOPC_StatusCode status,
                                                    SOPC_EncodeableType* notificationType,
                                                    uint32_t nbNotifElts,
                                                    const void* notification,
                                                    uintptr_t* monitoredItemCtxArray)
{
    SOPC_UNUSED_ARG(subscription);
    SOPC_UNUSED_ARG(monitoredItemCtxArray);
    if (SOPC_IsGoodStatus(status) && nbNotifElts > 0)
    {
        if (&OpcUa_EventNotificationList_EncodeableType == notificationType)
        {
            const OpcUa_EventNotificationList* eventList = (const OpcUa_EventNotificationList*) notification;
            printf("Received event notification:\n");
            if (eventList->NoOfEvents > 0 && eventList->Events[0].NoOfEventFields > 0)
            {
                SOPC_Variant_Print(&(eventList->Events[0].EventFields[0]));
            }
        }
        else if (&OpcUa_DataChangeNotification_EncodeableType == notificationType)
        {
            const OpcUa_DataChangeNotification* dataChangeList = (const OpcUa_DataChangeNotification*) notification;
            printf("Received data change notification:\n");
            if (dataChangeList->NoOfMonitoredItems > 0)
            {
                SOPC_Variant_Print(&(dataChangeList->MonitoredItems[0].Value.Value));
            }
        }
    }
}

static void create_monitored_item_event_and_data(const SOPC_ClientHelper_Subscription* subscription)
{
    OpcUa_CreateMonitoredItemsRequest* createMonItReq =
        SOPC_CreateMonitoredItemsRequest_Create(0, 2, OpcUa_TimestampsToReturn_Both);
    if (NULL == createMonItReq)
    {
        return;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // [0] Set the event monitored item to create
    status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId(createMonItReq, 0, &serverObjectId,
                                                                 SOPC_AttributeId_EventNotifier, NULL);

    OpcUa_EventFilter* eventFilter = NULL;
    if (SOPC_STATUS_OK == status)
    {
        size_t nbWhereClauseElts = 0;
        eventFilter = SOPC_MonitoredItem_CreateEventFilter(1, nbWhereClauseElts);
    }

    if (NULL != eventFilter)
    {
        SOPC_QualifiedName messagePath = SOPC_QUALIFIED_NAME(0, "Message");
        status = SOPC_EventFilter_SetSelectClause(eventFilter, 0, &conditionTypeId, 1, &messagePath,
                                                  SOPC_AttributeId_Value, NULL);
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        static const uint32_t queueSizeRequested = 1;
        SOPC_ExtensionObject* extObj = SOPC_MonitoredItem_EventFilter(eventFilter);
        SOPC_ASSERT(NULL != extObj);
        status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemParams(
            createMonItReq, 0, OpcUa_MonitoringMode_Reporting, 0, -1, extObj, queueSizeRequested, true);
    }

    // [1] Set the data monitored item to create
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_CreateMonitoredItemsRequest_SetMonitoredItemId(createMonItReq, 1, &serverCurrentTimeNodeId,
                                                                     SOPC_AttributeId_Value, NULL);
    }

    // Create the monitored items
    OpcUa_CreateMonitoredItemsResponse createMonItResp;
    OpcUa_CreateMonitoredItemsResponse_Initialize(&createMonItResp);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(subscription, createMonItReq, 0, &createMonItResp);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_ASSERT(createMonItResp.NoOfResults == 2 && SOPC_IsGoodStatus(createMonItResp.Results[0].StatusCode) &&
                        SOPC_IsGoodStatus(createMonItResp.Results[1].StatusCode));
        }
    }
    OpcUa_CreateMonitoredItemsResponse_Clear(&createMonItResp);
}
#endif

/*
 * Default client configuration loader (without XML configuration)
 */
static SOPC_ReturnStatus Client_SetDefaultConfiguration(size_t* nbSecConnCfgs,
                                                        SOPC_SecureConnection_Config*** secureConnConfigArray)
{
    // Define client application configuration
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_SetPreferredLocaleIds(
        (sizeof(preferred_locale_ids) / sizeof(preferred_locale_ids[0]) - 1), preferred_locale_ids);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_APPLICATION_URI,
                                                                   CLIENT_APPLICATION_NAME, NULL,
                                                                   OpcUa_ApplicationType_Client);
    }

    // Configure the secure channel connection to use and retrieve channel configuration index
    if (SOPC_STATUS_OK == status)
    {
        SOPC_SecureConnection_Config* secureConnConfig1 = SOPC_ClientConfigHelper_CreateSecureConnection(
            "1", DEFAULT_ENDPOINT_URL, OpcUa_MessageSecurityMode_None, SOPC_SecurityPolicy_None);

        if (secureConnConfig1 == NULL)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_GetSecureConnectionConfigs(nbSecConnCfgs, secureConnConfigArray);
    }

    return status;
}

static bool SOPC_GetClientUserSecuAdminPassword(const SOPC_SecureConnection_Config* secConnConfig,
                                                char** outUserName,
                                                char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);
    const char* user1 = "user1";
    char* userName = SOPC_Calloc(strlen(user1) + 1, sizeof(*userName));
    if (NULL == userName)
    {
        return false;
    }
    memcpy(userName, user1, strlen(user1) + 1);
    bool res = SOPC_TestHelper_AskPassWithContext_FromEnv(user1, outPassword);
    if (!res)
    {
        SOPC_Free(userName);
        return false;
    }
    *outUserName = userName;
    return true;
}

static SOPC_ReturnStatus Client_LoadClientConfiguration(size_t* nbSecConnCfgs,
                                                        SOPC_SecureConnection_Config*** secureConnConfigArray)
{
    /* Retrieve XML configuration file path from environment variables TEST_CLIENT_XML_CONFIG,
     *
     * In case of success returns the file path otherwise load default configuration.
     */

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const char* xml_client_config_path = NULL;

    if (nbClientConfigCount == 2)
    {
        xml_client_config_path = CLIENT_XML_CONFIG;
    }

    if (NULL != xml_client_config_path)
    {
        status = SOPC_ClientConfigHelper_ConfigureFromXML(xml_client_config_path, NULL, nbSecConnCfgs,
                                                          secureConnConfigArray);
    }

    // Set callback necessary to retrieve client key password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }

    // Set callback necessary to retrieve user password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetUserNamePasswordCallback(&SOPC_GetClientUserSecuAdminPassword);
    }

    if (SOPC_STATUS_OK == status && NULL == xml_client_config_path)
    {
        status = Client_SetDefaultConfiguration(nbSecConnCfgs, secureConnConfigArray);
    }

    if (SOPC_STATUS_OK == status)
    {
        printf(">>Test_Client_Restart: Client configured\n");
    }
    else
    {
        printf(">>Test_Client_Restart: Client configuration failed\n");
    }

    return status;
}

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
 *                      Manager server local reads
 * --------------------------------------------------------------------------*/

static SOPC_Thread g_async_local_services_thread = SOPC_INVALID_THREAD;

static void SOPC_LocalServiceAsyncResp(SOPC_EncodeableType* encType, void* response, uintptr_t appContext)
{
    SOPC_Free((uintptr_t*) appContext); // Deallocate empty context
    if (NULL != encType && NULL != response)
    {
        if (encType == &OpcUa_ReadResponse_EncodeableType)
        {
            OpcUa_ReadResponse* readResp = (OpcUa_ReadResponse*) response;
            if (SOPC_IsGoodStatus(readResp->ResponseHeader.ServiceResult) && 1 == readResp->NoOfResults)
            {
                printf("Received local read response with value:\n");
                SOPC_Variant_Print(&(readResp->Results[0].Value));
            }
        }
    }
}

static void* AsyncReadThread(void* args)
{
    SOPC_UNUSED_ARG(args);
    while (0 == stopRequested && false == SOPC_Atomic_Int_Get(&atomicStopped))
    {
        OpcUa_ReadRequest* readReq = SOPC_ReadRequest_Create(1, OpcUa_TimestampsToReturn_Both);
        uintptr_t* emptyContext = SOPC_Calloc(1, sizeof(uintptr_t)); // Just to check deallocation is always done
        if (NULL != readReq)
        {
            SOPC_ReturnStatus st =
                SOPC_ReadRequest_SetReadValue(readReq, 0, &serverCurrentTimeNodeId, SOPC_AttributeId_Value, NULL);
            if (SOPC_STATUS_OK == st)
            {
                st = SOPC_ServerHelper_LocalServiceAsync(readReq, (uintptr_t) emptyContext);
            }
            if (SOPC_STATUS_OK != st)
            {
                SOPC_Free(emptyContext);
                st = SOPC_EncodeableObject_Delete(&OpcUa_ReadRequest_EncodeableType, (void**) &readReq);
            }
        }
#if S2OPC_EVENT_MANAGEMENT
        // Generate an event
        SOPC_Event* event = NULL;
        SOPC_ReturnStatus eventStatus = SOPC_ServerHelper_CreateEvent(&conditionTypeId, &event);
        if (SOPC_STATUS_OK == eventStatus)
        {
            SOPC_LocalizedText lt = SOPC_LOCALIZED_TEXT("en-US", "EVENT GENERATED BY SERVER");
            eventStatus = SOPC_Event_SetMessage(event, &lt);
            SOPC_ASSERT(SOPC_STATUS_OK == eventStatus);
            eventStatus = SOPC_ServerHelper_TriggerEvent(&serverObjectId, event, 0, 0, 0);
        }
#endif
        SOPC_Sleep(UPDATE_STOP_TIMEOUT_MS);
    }
    return NULL;
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
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_restart_logs/";
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

    if (SOPC_STATUS_OK == status && nbServerConfigCount == 2)
    {
        xml_server_config_path = SERVER_XML_CONFIG;
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

    /* Start a dedicated thread to do local services */
    status = SOPC_Thread_Create(&g_async_local_services_thread, &AsyncReadThread, NULL, "AsyncLocalServices");

    /* Client secure channel configurations */
    SOPC_SecureConnection_Config** secureConnConfigArray = NULL;
    size_t nbSecConnCfgs = 0;

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
        nbServerConfigCount = 0;
        while (SOPC_STATUS_OK == status && nbServerConfigCount < NB_SERVER_CONFIG)
        {
            nbServerConfigCount++;

            /* Only configure the client the first time server is configured,
               second time do not reconfigure */
            if (nbServerConfigCount % NB_SERVER_CONFIG != 0)
            {
                nbClientConfigCount++;
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_ClientConfigHelper_Initialize();
                    if (SOPC_STATUS_OK == status)
                    {
                        printf("<Test_Server_Restart: CLIENT initialized #%" PRIu8 ".%" PRIu8 "\n", nbFullConfigCount,
                               nbClientConfigCount);
                    }
                    else
                    {
                        printf("<Test_Server_Restart: Failed CLIENT initialization #%" PRIu8 ".%" PRIu8 "\n",
                               nbFullConfigCount, nbClientConfigCount);
                    }
                }

                if (SOPC_STATUS_OK == status)
                {
                    status = Client_LoadClientConfiguration(&nbSecConnCfgs, &secureConnConfigArray);
                }
            }

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ServerConfigHelper_Initialize();
                if (SOPC_STATUS_OK != status)
                {
                    printf("<Test_Server_Restart: Failed SERVER initializing #%" PRIu8 ".%" PRIu8 "\n",
                           nbFullConfigCount, nbServerConfigCount);
                }
                else
                {
                    printf("<Test_Server_Restart: SERVER initialized #%" PRIu8 ".%" PRIu8 "\n", nbFullConfigCount,
                           nbServerConfigCount);
                }
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

            /* Define local service asynchronous response callback */
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ServerConfigHelper_SetLocalServiceAsyncResponse(&SOPC_LocalServiceAsyncResp);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "Failed to configure the local service asynchronous response callback");
                }
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
                    /* Run the server until error, stop server signal detected (Ctrl-C) or StopServer called */
                    status = SOPC_ServerHelper_StartServer(&SOPC_ServerStopped_Cb);
                    if (SOPC_STATUS_OK == status)
                    {
                        SOPC_Atomic_Int_Set(&atomicStopped, false);
                        printf("<Test_Server_Restart: Server STARTED #%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
                               nbFullConfigCount, nbServerConfigCount, nbRestartCountRem);
                    }
                    else
                    {
                        printf("<Test_Server_Restart: Failed to START server #%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
                               nbFullConfigCount, nbServerConfigCount, nbRestartCountRem);
                    }

                    uint8_t nbReConnectCountRem = 0;
                    while (nbReConnectCountRem < NB_CLIENT_RECONNECT)
                    {
                        SOPC_ClientConnection* secureConnection = NULL;
                        status = SOPC_ClientHelper_Connect(secureConnConfigArray[0], &SOPC_ClientConnectionEventCb,
                                                           &secureConnection);
                        if (SOPC_STATUS_OK == status)
                        {
                            printf("<Test_Server_Restart: Client CONNECTED #%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8
                                   "\n",
                                   nbFullConfigCount, nbServerConfigCount, nbRestartCountRem, nbReConnectCountRem);
                            SOPC_Sleep(UPDATE_STOP_TIMEOUT_MS);
                        }
                        else
                        {
                            printf("<Test_Server_Restart: Failed to CONNECT client #%" PRIu8 ".%" PRIu8 ".%" PRIu8
                                   ".%" PRIu8 "\n",
                                   nbFullConfigCount, nbServerConfigCount, nbRestartCountRem, nbReConnectCountRem);
                        }
#if S2OPC_EVENT_MANAGEMENT
                        // Create a subscription to receive events
                        if (SOPC_STATUS_OK == status)
                        {
                            OpcUa_CreateSubscriptionRequest* createSubParams =
                                SOPC_CreateSubscriptionRequest_CreateDefault();

                            const SOPC_ClientHelper_Subscription* subscription = SOPC_ClientHelper_CreateSubscription(
                                secureConnection, createSubParams, SOPC_Client_SubscriptionNotification_Cb, 0);
                            if (SOPC_STATUS_OK == status)
                            {
                                create_monitored_item_event_and_data(subscription);
                            }
                            SOPC_Sleep(UPDATE_STOP_TIMEOUT_MS);
                        }
#endif
                        if (SOPC_STATUS_OK == status)
                        {
                            status = SOPC_ClientHelper_Disconnect(&secureConnection);
                            if (SOPC_STATUS_OK == status)
                            {
                                printf("<Test_Server_Restart: Client DISCONNECTED #%" PRIu8 ".%" PRIu8 ".%" PRIu8
                                       ".%" PRIu8 "\n",
                                       nbFullConfigCount, nbServerConfigCount, nbRestartCountRem, nbReConnectCountRem);
                            }
                            else
                            {
                                printf("<Test_Server_Restart: Failed to DISCONNECT client #%" PRIu8 ".%" PRIu8
                                       ".%" PRIu8 ".%" PRIu8 "\n",
                                       nbFullConfigCount, nbServerConfigCount, nbRestartCountRem, nbReConnectCountRem);
                            }
                        }
                        nbReConnectCountRem++;
                    }

                    if (SOPC_STATUS_OK == status)
                    {
                        status = SOPC_ServerHelper_StopServer();
                    }

                    if (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&atomicStopped) == true)
                    {
                        printf("<Test_Server_Restart: Server STOPPED #%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
                               nbFullConfigCount, nbServerConfigCount, nbRestartCountRem);
                    }
                    else
                    {
                        printf("<Test_Server_Restart: Failed to STOP server #%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
                               nbFullConfigCount, nbServerConfigCount, nbRestartCountRem);
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
            /* Only clear the client at the end of the second time server is cleared
             * as we do not reconfigure client for the second time */
            if (nbServerConfigCount % NB_SERVER_CONFIG == 0)
            {
                SOPC_ClientConfigHelper_Clear();
            }
            /* Clear the server library (stop all library threads) and server configuration */
            SOPC_ServerConfigHelper_Clear();
            printf("<Test_Server_Restart: SERVER clearing #%" PRIu8 ".%" PRIu8 "\n", nbFullConfigCount,
                   nbServerConfigCount);
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
