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

// System includes
#include <assert.h>
#include <kernel.h>
#include <shell/shell.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>

#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

// S2OPC includes
#include "p_threads.h"
#include "sopc_address_space.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_sub_scheduler.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_toolkit_config.h"

// project includes
#include "cache.h"
#include "cache_sync.h"
#include "network_init.h"
#include "pubsub_config_static.h"
#include "static_security_data.h"
#include "threading_alt.h"

/***
 * @brief
 *  This demo zephyr application configures a OPCUA server and a PubSub
 *  - The server endpoint is provided by CONFIG_SOPC_ENDPOINT_ADDRESS
 *  - The PubSub configuration is staticly defined (see pubsub_config_static.h)
 *  - The server provides nodes to control the PubSub:
 *    - Start
 *    - Stop
 *    - Change publish period
 *    - Expose content of Subscriber-received DataSets (Read-Only)
 *      - These data are refreshed periodically based on CONFIG_SOPC_SUBSCRIBER_SYNCH_TIME_MS
 *      - If 0, then the data are synchronized to the AddressSpace synchronously
 *      (may be used with caution, in particular if PubSub periods are small)
 *    - Expose content of Publisher-emitted DataSets (Read-Write)
 *  - Notes: The content of the DSM CANNOT BE MODIFIED in this demo
 */
/***************************************************/
/**               STATIC CHECKS                    */
/***************************************************/
#ifndef CONFIG_SOPC_ENDPOINT_ADDRESS
#error "CONFIG_SOPC_ENDPOINT_ADDRESS not defined"
#endif
#define DEMO_CYCLE_MS 10

/***************************************************/
/**               MISC FUNCTIONS                   */
/***************************************************/
static void log_UserCallback(const char* context, const char* text);

// Zephyr console interface
static int cmd_demo_kill(const struct shell* shell, size_t argc, char** argv);
static int cmd_demo_info(const struct shell* shell, size_t argc, char** argv);
static int cmd_demo_log(const struct shell* shell, size_t argc, char** argv);
static int cmd_demo_pub(const struct shell* shell, size_t argc, char** argv);
static int cmd_demo_sub(const struct shell* shell, size_t argc, char** argv);
static int cmd_demo_write(const struct shell* shell, size_t argc, char** argv);
static int cmd_demo_read(const struct shell* shell, size_t argc, char** argv);
static int cmd_demo_cache(const struct shell* shell, size_t argc, char** argv);
#if CONFIG_SOPC_HELPER_IMPL_INSTRUM
static void cmd_demo_instrum(const struct shell* shell, size_t argc, char** argv);
#endif

/***************************************************/
/**               SERVER CONFIGURATION             */
/***************************************************/
#define ASYNCH_CONTEXT_PARAM 0x12345678u
#define APPLICATION_URI "urn:S2OPC:localhost"
#define PRODUCT_URI "urn:S2OPC:localhost"
#define SERVER_DESCRIPTION "S2OPC Zephyr demo Server"
#define LOCALE_ID "en-US"
static const char* g_userNamespaces[2] = {"urn:S2OPC:zephyr_demo", NULL};
static const char* g_localesArray[2] = {LOCALE_ID, NULL};

// generated address space.
extern const bool sopc_embedded_is_const_addspace;
extern SOPC_AddressSpace_Node SOPC_Embedded_AddressSpace_Nodes[];
extern const uint32_t SOPC_Embedded_AddressSpace_nNodes;
extern const uint32_t SOPC_Embedded_VariableVariant_nb;
extern SOPC_Variant SOPC_Embedded_VariableVariant[];

/***************************************************/
/**               SERVER VARIABLES CONTENT         */
/***************************************************/
static int32_t gStopped = true;

static SOPC_Endpoint_Config* g_epConfig = NULL;
static SOPC_ReturnStatus authentication_check(SOPC_UserAuthentication_Manager* authn,
                                              const SOPC_ExtensionObject* token,
                                              SOPC_UserAuthentication_Status* authenticated);
/** Configuration of callbacks for authentication */
static const SOPC_UserAuthentication_Functions authentication_functions = {
    .pFuncFree = (SOPC_UserAuthentication_Free_Func*) &SOPC_Free,
    .pFuncValidateUserIdentity = &authentication_check};
static void serverWriteEvent(const SOPC_CallContext* callCtxPtr,
                             OpcUa_WriteValue* writeValue,
                             SOPC_StatusCode writeStatus);
static void localServiceAsyncRespCallback(SOPC_EncodeableType* encType, void* response, uintptr_t appContext);
static bool Server_LocalWriteSingleNode(SOPC_NodeId* pNid, SOPC_DataValue* pDv);
static SOPC_DataValue* Server_LocalReadSingleNode(SOPC_NodeId* pNid);

/***************************************************/
/**               PUBSUB VARIABLES CONTENT         */
/***************************************************/
static SOPC_PubSubConfiguration* pPubSubConfig = NULL;
static SOPC_SubTargetVariableConfig* pTargetConfig = NULL;
static SOPC_PubSourceVariableConfig* pSourceConfig = NULL;
static bool gPubStarted = false;
static bool gSubStarted = false;
static bool gSubOperational = false;

/***************************************************/
/**               HELPER LOG MACROS                */
/***************************************************/
#define DEBUG(...) SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
#define INFO(...) SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
#define WARNING(...) SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
#define ERROR(...) SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, __VA_ARGS__)
#define PRINT printk
#define YES_NO(x) ((x) ? "YES" : "NO")

/***************************************************/
SOPC_Build_Info SOPC_ClientServer_GetBuildInfo()
{
    static const SOPC_Build_Info sopc_client_server_build_info = {.buildVersion = SOPC_TOOLKIT_VERSION,
                                                                  .buildSrcCommit = "Not applicable",
                                                                  .buildDockerId = "",
                                                                  .buildBuildDate = ""};

    return sopc_client_server_build_info;
}

/***************************************************/
SOPC_Build_Info SOPC_Common_GetBuildInfo()
{
    static const SOPC_Build_Info sopc_common_build_info = {.buildVersion = SOPC_TOOLKIT_VERSION,
                                                           .buildSrcCommit = "Unknown_Revision",
                                                           .buildDockerId = "",
                                                           .buildBuildDate = ""};

    return sopc_common_build_info;
}

/**************************************************************************/
static void serverStopped_cb(SOPC_ReturnStatus status)
{
    SOPC_Atomic_Int_Set(&gStopped, 1);
    WARNING("Server stopped!\n");
}

/***************************************************/
K_MUTEX_DEFINE(log_mutex);
static void log_UserCallback(const char* context, const char* text)
{
    (void) context;
    if (NULL != text)
    {
        // Protect concurrent accesses to the console
        k_mutex_lock(&log_mutex, K_FOREVER);
        // printk won't print more than 80 chars.
        // This implementation simply prints out the message line by line of 80 characters.
        // In scope a real-time application this is not a relevant implementation because the mutex
        // will be locked for an unknown long time.
        // See example in 'zephyr_ptp' for a non-blocking thread-safe console printing
        printk("%.80s\n", text);
        if (strlen(text) > 80)
        {
            text += 80;
            while (strlen(text) > 78)
            {
                printk("  %.78s\n", text);
                text += 78;
            }
            printk("  %s\n", text);
        }
        k_mutex_unlock(&log_mutex);
    }
}

/* This function can be used to filter incoming connections based on user identification*/
static SOPC_ReturnStatus authentication_check(SOPC_UserAuthentication_Manager* authn,
                                              const SOPC_ExtensionObject* token,
                                              SOPC_UserAuthentication_Status* authenticated)
{
    SOPC_ASSERT(NULL != token && NULL != authenticated && NULL != authn);

    *authenticated = SOPC_USER_AUTHENTICATION_REJECTED_TOKEN;
    if (SOPC_ExtObjBodyEncoding_Object == token->Encoding &&
        &OpcUa_UserNameIdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        OpcUa_UserNameIdentityToken* userToken = (OpcUa_UserNameIdentityToken*) (token->Body.Object.Value);

        const char* username = SOPC_String_GetRawCString(&userToken->UserName);
        SOPC_ByteString* pwd = &userToken->Password;

        // This is an (unrealistic) example of user authentication check
        if (strcmp(username, "user1") == 0)
        {
            if (pwd->Length == 4 && memcmp(pwd->Data, "pass", 4) == 0)
            {
                INFO("User <%s> has successfully authenticated", username);
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
            else
            {
                WARNING("User <%s> entered an invalid password", username);
            }
        }
        else
        {
            WARNING("Unknown user <%s>", username);
        }
    }

    return SOPC_STATUS_OK;
}

/**
 * Callback for write-event on the server
 */
static void serverWriteEvent(const SOPC_CallContext* callCtxPtr,
                             OpcUa_WriteValue* writeValue,
                             SOPC_StatusCode writeStatus)
{
    SOPC_ASSERT(NULL != callCtxPtr && NULL != writeValue);

    // Here, specific actions can be performed when a client has successfully written
    // into any variable of the server
    if (SOPC_STATUS_OK == writeStatus)
    {
        char* nodeId = SOPC_NodeId_ToCString(&writeValue->NodeId);
        INFO("A client updated the content of node <%s> with a value of type %d", nodeId,
             writeValue->Value.Value.BuiltInTypeId);
        SOPC_Free(nodeId);
        // Synchronize cache for PubSub
        cacheSync_WriteToCache(&writeValue->NodeId, &writeValue->Value);
    }
    else
    {
        WARNING("Client write failed on server. returned code 0x%08X", writeStatus);
    }
}

/***
 * Callback for server local asynch events
 * @param encType The type of result:
 *        - &OpcUa_ReadResponse_EncodeableType for OpcUa_ReadResponse
 *        - &OpcUa_WriteResponse_EncodeableType for OpcUa_WriteResponse
 *        - ...
 * @param response The result of operation. Must be casted to a \a OpcUa_XxxResponse depending on \a encType The
 * @param appContext The user context, as provided in previous call to \a SOPC_ServerHelper_LocalServiceAsync
 */
static void localServiceAsyncRespCallback(SOPC_EncodeableType* encType, void* response, uintptr_t appContext)
{
    if (encType == &OpcUa_WriteResponse_EncodeableType)
    {
        // This check ensures we receive a OpcUa_WriteResponse
        SOPC_ASSERT(appContext == ASYNCH_CONTEXT_PARAM || appContext == ASYNCH_CONTEXT_CACHE_SYNC);

        OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) response;
        // Example: only check that the result is OK
        for (int32_t i = 0; i < writeResp->NoOfResults; i++)
        {
            const SOPC_StatusCode status = writeResp->Results[i];
            if (status != 0)
            {
                WARNING("Internal data update[%d] failed with code 0x%08X", i, status);
            }
        }
    }
}

/***
 * Request the server to update a node value (local request)
 * @param pNid The nodeId to modify
 * @param pDv The DataValue to write
 * @post \a localServiceAsyncRespCallback will be called with operation result
 */
static bool Server_LocalWriteSingleNode(SOPC_NodeId* pNid, SOPC_DataValue* pDv)
{
    OpcUa_WriteRequest* request = SOPC_WriteRequest_Create(1);
    SOPC_ASSERT(NULL != request);

    SOPC_ReturnStatus status;
    status = SOPC_WriteRequest_SetWriteValue(request, 0, pNid, SOPC_AttributeId_Value, NULL, pDv);
    if (status != SOPC_STATUS_OK)
    {
        WARNING("SetWriteValue failed with code  %d", status);
        SOPC_Free(request);
    }
    else
    {
        // Synchronize cache for PubSub
        cacheSync_WriteToCache(pNid, pDv);

        // param is not used here because this is a static context. In a more complex application,
        // param can be set to any context pointer that way be required by the application
        // We use a simple marker to show data transmission from here to localServiceAsyncRespCallback
        status = SOPC_ServerHelper_LocalServiceAsync(request, ASYNCH_CONTEXT_PARAM);
        if (status != SOPC_STATUS_OK)
        {
            WARNING("LocalServiceAsync failed with code  (%d)", status);
            SOPC_Free(request);
        }
    }

    return status == SOPC_STATUS_OK;
}

/***
 * Send a read requset to the server and wait for the response
 * @param pNid The nodeId to read
 * @return a new allocated DataValue containing the result (or NULL in case of failure).
 *      Shall be freed by caller after use.
 */
static SOPC_DataValue* Server_LocalReadSingleNode(SOPC_NodeId* pNid)
{
    OpcUa_ReadRequest* request = SOPC_ReadRequest_Create(1, OpcUa_TimestampsToReturn_Neither);
    OpcUa_ReadResponse* response = NULL;
    SOPC_ASSERT(NULL != request);

    SOPC_ReturnStatus status;
    status = SOPC_ReadRequest_SetReadValue(request, 0, pNid, SOPC_AttributeId_Value, NULL);
    if (status != SOPC_STATUS_OK)
    {
        WARNING("Read Value failed with code  %d", status);
        SOPC_Free(request);
        return NULL;
    }
    status = SOPC_ServerHelper_LocalServiceSync(request, (void**) &response);
    if (status != SOPC_STATUS_OK)
    {
        WARNING("Read Value failed with code  %d", status);
        SOPC_ASSERT(NULL == response);
        SOPC_Free(request);
        return NULL;
    }

    SOPC_DataValue* result = NULL;
    if (response != NULL && response->NoOfResults == 1)
    {
        //
        result = SOPC_Malloc(sizeof(*result));
        SOPC_ASSERT(NULL != result);
        SOPC_DataValue_Copy(result, &response->Results[0]);
    }
    OpcUa_ReadResponse_Clear(response);
    SOPC_Free(response);
    return result;
}

/***
 * @brief Creates and setup the OPCUA server, using KConfig parameters
 */
static void setupServer(void)
{
    SOPC_ReturnStatus status;

    status = SOPC_HelperConfigServer_Initialize();
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_HelperConfigServer_Initialize failed");

    log_UserCallback(NULL, "S2OPC initialization OK");

    //////////////////////////////////
    // Namespaces initialization
    status = SOPC_HelperConfigServer_SetNamespaces(1, g_userNamespaces);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_HelperConfigServer_SetNamespaces failed");

    status = SOPC_HelperConfigServer_SetLocaleIds(1, g_localesArray);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_HelperConfigServer_SetLocaleIds failed");

    //////////////////////////////////
    // Global descriptions initialization
    status = SOPC_HelperConfigServer_SetApplicationDescription(APPLICATION_URI, PRODUCT_URI, SERVER_DESCRIPTION,
                                                               LOCALE_ID, OpcUa_ApplicationType_Server);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_HelperConfigServer_SetApplicationDescription failed");

    //////////////////////////////////
    // Create endpoints configuration
    SOPC_SecurityPolicy* sp;

    g_epConfig = SOPC_HelperConfigServer_CreateEndpoint(CONFIG_SOPC_ENDPOINT_ADDRESS, true);
    SOPC_ASSERT(NULL != g_epConfig && "SOPC_HelperConfigServer_CreateEndpoint failed");

    log_UserCallback(NULL, "Setting up security...");

    /* 1st Security policy is None without user (users on unsecure channel shall be forbidden) */
    sp = SOPC_EndpointConfig_AddSecurityConfig(g_epConfig, SOPC_SecurityPolicy_None);
    SOPC_ASSERT(NULL != sp && "SOPC_EndpointConfig_AddSecurityConfig #1 failed");

    status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SECURITY_MODE_NONE_MASK);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_SetSecurityModes #1 failed");

    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_AddUserTokenPolicy #1/#1 failed");

    /* 2nd Security policy is Basic256 with anonymous or username authentication allowed
     * (without password encryption) */
    sp = SOPC_EndpointConfig_AddSecurityConfig(g_epConfig, SOPC_SecurityPolicy_Basic256);
    SOPC_ASSERT(NULL != sp && "SOPC_EndpointConfig_AddSecurityConfig #2 failed");

    status =
        SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_SetSecurityModes #2 failed");

    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_AddUserTokenPolicy #2/#1 failed");
    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_AddUserTokenPolicy #2/#2 failed");

    /* 3rd Security policy is Basic256Sha256 with anonymous or username authentication allowed
     * (without password encryption) */
    sp = SOPC_EndpointConfig_AddSecurityConfig(g_epConfig, SOPC_SecurityPolicy_Basic256Sha256);
    SOPC_ASSERT(NULL != sp && "SOPC_EndpointConfig_AddSecurityConfig #3 failed");

    status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_SetSecurityModes #3 failed");

    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_AddUserTokenPolicy #3/#1 failed");
    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_AddUserTokenPolicy #3/#2 failed");

    //////////////////////////////////
    // Server certificates configuration
    SOPC_SerializedCertificate* serializedCAcert = NULL;
    SOPC_CRLList* serializedCAcrl = NULL;
    SOPC_PKIProvider* pkiProvider = NULL;
    status = SOPC_HelperConfigServer_SetKeyCertPairFromBytes(sizeof(server_2k_cert), server_2k_cert,
                                                             sizeof(server_2k_key), server_2k_key);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_HelperConfigServer_SetKeyCertPairFromBytes() failed");

    status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(cacert, sizeof(cacert), &serializedCAcert);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_KeyManager_SerializedCertificate_CreateFromDER() failed");
    status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &serializedCAcrl);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_KeyManager_CRL_CreateOrAddFromDER() failed");

    status = SOPC_PKIProviderStack_Create(serializedCAcert, serializedCAcrl, &pkiProvider);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_PKIProviderStack_Create() failed");
    SOPC_KeyManager_SerializedCertificate_Delete(serializedCAcert);

    status = SOPC_HelperConfigServer_SetPKIprovider(pkiProvider);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_HelperConfigServer_SetPKIprovider failed");

    log_UserCallback(NULL, "Test_Server_Client: Certificates and key loaded");

    //////////////////////////////////
    // Setup AddressSpace
    (void) sopc_embedded_is_const_addspace;
    SOPC_ASSERT(sopc_embedded_is_const_addspace && "Address space must be constant.");
    INFO("# Loading AddressSpace (%u nodes)...\n", SOPC_Embedded_AddressSpace_nNodes);
    SOPC_AddressSpace* addSpace =
        SOPC_AddressSpace_CreateReadOnlyNodes(SOPC_Embedded_AddressSpace_nNodes, SOPC_Embedded_AddressSpace_Nodes,
                                              SOPC_Embedded_VariableVariant_nb, SOPC_Embedded_VariableVariant);
    SOPC_ASSERT(NULL != addSpace && "SOPC_AddressSpace_Create failed");
    INFO("# Address space loaded\n");

    status = SOPC_HelperConfigServer_SetAddressSpace(addSpace);
    SOPC_ASSERT(NULL != addSpace && "SOPC_HelperConfigServer_SetAddressSpace failed");

    SOPC_UserAuthorization_Manager* authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
    SOPC_ASSERT(NULL != authorizationManager && "Failed to allocate SOPC_UserAuthentication_Manager");

    //////////////////////////////////
    // User Management configuration
    SOPC_UserAuthentication_Manager* authenticationManager =
        (SOPC_UserAuthentication_Manager*) SOPC_Malloc(sizeof(SOPC_UserAuthentication_Manager));
    SOPC_ASSERT(NULL != authenticationManager && "Failed to allocate SOPC_UserAuthentication_Manager");

    memset(authenticationManager, 0, sizeof(*authenticationManager));

    // It is possible to store any user value in pData, which can provide some context while in
    // callback event (see function authentication_check)
    authenticationManager->pData = (void*) NULL;

    authenticationManager->pFunctions = &authentication_functions;
    SOPC_HelperConfigServer_SetUserAuthenticationManager(authenticationManager);
    SOPC_HelperConfigServer_SetUserAuthorizationManager(authorizationManager);

    status = SOPC_HelperConfigServer_SetWriteNotifCallback(&serverWriteEvent);
    SOPC_ASSERT(NULL != authenticationManager && "SOPC_HelperConfigServer_SetWriteNotifCallback failed");

    //////////////////////////////////
    // Set the asynchronous event callback
    status = SOPC_HelperConfigServer_SetLocalServiceAsyncResponse(localServiceAsyncRespCallback);
    SOPC_ASSERT(NULL != authenticationManager && "SOPC_HelperConfigServer_SetLocalServiceAsyncResponse failed");

    SOPC_HelperConfigServer_SetShutdownCountdown(1);

    //////////////////////////////////
    // Start the server
    SOPC_Atomic_Int_Set(&gStopped, 0);
    status = SOPC_ServerHelper_StartServer(&serverStopped_cb);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_ServerHelper_StartServer failed");

    // Check for server status after some time. (Start is asynchronous)
    k_sleep(K_MSEC(100));
    SOPC_ASSERT(SOPC_Atomic_Int_Get(&gStopped) == 0 && "Server failed to start.");
    PRINT("########################\n");
    PRINT("# Server started on <%s>\n", g_epConfig->endpointURL);
}

static void clearServer(void)
{
    /// Note : in current example, the server is already down when reaching this point
    SOPC_ServerHelper_StopServer();

    SOPC_HelperConfigServer_Clear();
}

/***
 * @brief Creates and setup the PubSub, using KConfig parameters
 */
static void setupPubSub(void)
{
    // CONFIGURE PUBSUB
    pPubSubConfig = SOPC_PubSubConfig_GetStatic();
    SOPC_ASSERT(NULL != pPubSubConfig && "SOPC_PubSubConfig_GetStatic failed");

    /* Sub target configuration */
    pTargetConfig = SOPC_SubTargetVariableConfig_Create(&cacheSync_SetTargetVariables);
    SOPC_ASSERT(NULL != pTargetConfig && "SOPC_SubTargetVariableConfig_Create failed");

    /* Pub target configuration */
    pSourceConfig = SOPC_PubSourceVariableConfig_Create(&Cache_GetSourceVariables);
    SOPC_ASSERT(NULL != pSourceConfig && "SOPC_PubSourceVariableConfig_Create failed");

    // Configure SKS for PubSub
    SOPC_KeyBunch_init_static(pubSub_keySign, sizeof(pubSub_keySign), pubSub_keyEncrypt, sizeof(pubSub_keyEncrypt),
                              pubSub_keyNonce, sizeof(pubSub_keyNonce));

    Cache_Initialize(pPubSubConfig);
}

/***************************************************/
/**             Server main function               */
/***************************************************/
int main(int argc, char* argv[])
{
    // Note: avoid unused parameter warning from compiler
    (void) argc;
    (void) argv;
    PRINT("\nBUILD DATE : " __DATE__ " " __TIME__ "\n");

    // Setup network
    bool netInit = Network_Initialize(NULL);
    assert(netInit == true);

    /* Initialize MbedTLS */
    tls_threading_initialize();

    /* Configure the server logger (user logger) */
    SOPC_Log_Configuration logConfig;
    logConfig.logSystem = SOPC_LOG_SYSTEM_USER;
    logConfig.logLevel = SOPC_LOG_LEVEL_INFO;
    logConfig.logSysConfig.userSystemLogConfig.doLog = &log_UserCallback;

    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfig);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_CommonHelper_Initialize failed");

    setupPubSub();
    setupServer();

    PRINT("# Type 'demo help' in console for help\n");

    while (SOPC_Atomic_Int_Get(&gStopped) == 0)
    {
        k_sleep(K_MSEC(DEMO_CYCLE_MS));
    }

    clearServer();
    SOPC_CommonHelper_Clear();
    INFO("# Info: Server closed.\n");
    WARNING("# Rebooting in 5 seconds...\n\n");
    k_sleep(K_MSEC(5000));

    sys_reboot(SYS_REBOOT_COLD);
    return 0;
}

/*---------------------------------------------------------------------------
 *                             NET SHELL CONFIGURATION
 *---------------------------------------------------------------------------*/
/***************************************************/
static int cmd_demo_info(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    PRINT("Zephyr S2OPC Server demo status\n");
    PRINT("Server endpoint       : %s\n", CONFIG_SOPC_ENDPOINT_ADDRESS);
    PRINT("Server running        : %s\n", YES_NO(gStopped == 0));
    PRINT("Server const@space    : %s\n", YES_NO(sopc_embedded_is_const_addspace));
    PRINT("Server toolkit version: %s\n", SOPC_TOOLKIT_VERSION);
    PRINT("Publisher address     : %s\n", CONFIG_SOPC_PUBLISHER_ADDRESS);
    PRINT("Publisher running     : %s\n", YES_NO(gPubStarted));
    PRINT("Publisher period      : %d ms\n", CONFIG_SOPC_PUBLISHER_PERIOD_US / 1000);
    PRINT("Subscriber address    : %s\n", CONFIG_SOPC_SUBSCRIBER_ADDRESS);
    PRINT("Subscriber running    : %s\n", YES_NO(gSubStarted));
    PRINT("Subscriber operat.    : %s\n", YES_NO(gSubOperational));

    return 0;
}

/***************************************************/
static int cmd_demo_log(const struct shell* shell, size_t argc, char** argv)
{
    if (argc < 2)
    {
        PRINT("usage: demo log <D|I|W|E>\n");
        return 0;
    }
    switch (argv[1][0])
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
    }
    return 0;
}

/***************************************************/
static int cmd_demo_pub(const struct shell* shell, size_t argc, char** argv)
{
    const char* param = (argc < 2 ? "" : argv[1]);

    if (0 == strcmp(param, "start"))
    {
        // start publisher (will fail if already started)
        bool bResult;
        bResult = SOPC_PubScheduler_Start(pPubSubConfig, pSourceConfig, CONFIG_SOPC_PUBLISHER_PRIORITY);
        if (!bResult)
        {
            PRINT("\r\nFailed to start Publisher!\r\n");
            return 1;
        }
        else
        {
            gPubStarted = true;
            PRINT("\r\nPublisher started\r\n");
            return 0;
        }
    }
    if (0 == strcmp(param, "stop"))
    {
        SOPC_PubScheduler_Stop();
        gPubStarted = false;
        return 0;
    }
    PRINT("usage: %s [start|stop]\n", argv[0]);
    return 0;
}

/***************************************************/
static void cb_SetSubStatus(SOPC_PubSubState state)
{
    printk("New Sub state: %d\n", (int) state);
    gSubOperational = (SOPC_PubSubState_Operational == state);
}

/***************************************************/
static int cmd_demo_sub(const struct shell* shell, size_t argc, char** argv)
{
    const char* param = (argc < 2 ? "" : argv[1]);

    if (0 == strcmp(param, "start"))
    {
        // start subscriber (will fail if already started)
        bool bResult;
        bResult =
            SOPC_SubScheduler_Start(pPubSubConfig, pTargetConfig, cb_SetSubStatus, CONFIG_SOPC_SUBSCRIBER_PRIORITY);
        if (!bResult)
        {
            printk("\r\nFailed to start Subscriber!\r\n");
            return 1;
        }
        else
        {
            gSubStarted = true;
            PRINT("\r\nSubscriber started\r\n");
            return 0;
        }
    }
    if (0 == strcmp(param, "stop"))
    {
        SOPC_PubScheduler_Stop();
        gSubStarted = false;
        return 0;
    }

    PRINT("usage: %s [start|stop]\n", argv[0]);
    return 0;
}

/***************************************************/
static int cmd_demo_write(const struct shell* shell, size_t argc, char** argv)
{
    if (argc < 3)
    {
        PRINT("usage: demo write <nodeid> <value>\n");
        PRINT("<value> must be prefixed by b for a BOOL s for a String; i for a 32 bit INT.\n");
        PRINT("Other formats not implemented here.\n");
        return 0;
    }

    const char* nodeIdC = argv[1];
    const char* dvC = argv[2];

    PRINT("nodeIdC='%s', dvC='%s'\n", nodeIdC, dvC);

    SOPC_NodeId nid;
    SOPC_NodeId_InitializeFromCString(&nid, nodeIdC, strlen(nodeIdC));
    SOPC_DataValue dv;
    SOPC_DataValue_Initialize(&dv);

    dv.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    dv.Value.DoNotClear = false;
    if (dvC[0] == 's')
    {
        dv.Value.BuiltInTypeId = SOPC_String_Id;
        SOPC_String_InitializeFromCString(&dv.Value.Value.String, dvC + 1);
    }
    else if (dvC[0] == 'b')
    {
        dv.Value.BuiltInTypeId = SOPC_Boolean_Id;

        dv.Value.Value.Boolean = atoi(dvC + 1);
    }
    else if (dvC[0] == 'i')
    {
        dv.Value.BuiltInTypeId = SOPC_Int32_Id;

        dv.Value.Value.Int32 = atoi(dvC + 1);
    }
    else if (dvC[0] == 'u')
    {
        dv.Value.BuiltInTypeId = SOPC_UInt32_Id;

        dv.Value.Value.Uint32 = atoi(dvC + 1);
    }
    else
    {
        PRINT("Invalid format for <value>\n");
        return 0;
    }

    Server_LocalWriteSingleNode(&nid, &dv);

    SOPC_NodeId_Clear(&nid);
    SOPC_DataValue_Clear(&dv);
    return 0;
}

/***************************************************/
static int cmd_demo_read(const struct shell* shell, size_t argc, char** argv)
{
    if (argc < 2)
    {
        PRINT("usage: demo read <nodeid>\n");
        return 0;
    }

    const char* nodeIdC = argv[1];

    SOPC_NodeId nid;
    SOPC_NodeId_InitializeFromCString(&nid, nodeIdC, strlen(nodeIdC));

    SOPC_DataValue* dv = Server_LocalReadSingleNode(&nid);

    if (NULL == dv)
    {
        PRINT("Failed to read node '%s'\n", nodeIdC);
        return 1;
    }

    Cache_Dump_VarValue(&nid, dv);

    SOPC_NodeId_Clear(&nid);
    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);
    return 0;
}

/***************************************************/
static int cmd_demo_cache(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    Cache_Dump();
    return 0;
}

// TODO JCH #warning "TODO : match cache to addressspace"

/***************************************************/
static int cmd_demo_kill(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    SOPC_Atomic_Int_Set(&gStopped, 1);
    WARNING("Server manually stopped!\n");

    return 0;
}

/* Creating subcommands (level 1 command) array for command "demo". */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_demo,
                               SHELL_CMD(info, NULL, "Show demo info", cmd_demo_info),
#if CONFIG_SOPC_HELPER_IMPL_INSTRUM
                               SHELL_CMD(instrum, NULL, "Show instrum. result", cmd_demo_instrum),
#endif
                               SHELL_CMD(log, NULL, "Set log level", cmd_demo_log),
                               SHELL_CMD(kill, NULL, "Kill server", cmd_demo_kill),
                               SHELL_CMD(read, NULL, "Print content of  <NodeId>", cmd_demo_read),
                               SHELL_CMD(cache, NULL, "Print content of cache", cmd_demo_cache),
                               SHELL_CMD(write, NULL, "Write value to server", cmd_demo_write),
                               SHELL_CMD(pub, NULL, "Manage Publisher", cmd_demo_pub),
                               SHELL_CMD(sub, NULL, "Manage Subscriber", cmd_demo_sub),
                               SHELL_SUBCMD_SET_END);

/* Creating root (level 0) command "demo" */
SHELL_CMD_REGISTER(demo, &sub_demo, "Demo commands", NULL);

#if CONFIG_SOPC_HELPER_IMPL_INSTRUM
extern const size_t SOPC_MemAlloc_Nb_Allocs(void);
static void cmd_demo_instrum(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    const uint32_t nbSec = k_uptime_get_32() / 1000;
    PRINT("----- t=%d m %d s-------------\n", nbSec / 60, nbSec % 60);
    PRINT("Nb Allocs: %u\n", SOPC_MemAlloc_Nb_Allocs());

    const SOPC_Thread_Info* pInfos = SOPC_Thread_GetAllThreadsInfo();
    size_t idx = 0;
    while (NULL != pInfos && pInfos->stack_size > 0)
    {
        idx++;
        PRINT("Thr #%02u (%.08s): (%05u / %05u bytes used) = %02d%%\n", idx, pInfos->name, pInfos->stack_usage,
              pInfos->stack_size, (100 * pInfos->stack_usage) / pInfos->stack_size);
        pInfos++;
    }
}
#endif
