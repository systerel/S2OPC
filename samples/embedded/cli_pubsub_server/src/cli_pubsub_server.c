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

/***
 * @brief
 *  This demo application configures a OPCUA server and a PubSub
 *  - The server endpoint is provided by CONFIG_SOPC_ENDPOINT_ADDRESS
 *  - The Command-Line interface (CLI) provides various interactions with the demo
 *      Type "help" to get more details. Moreover, the "dbg" command might provide
 *      target-related features. Try "dbg help" to see if additional features are available.
 *  - The PubSub configuration is statically defined (see pubsub_config_static.h)
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
// System includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

// S2OPC includes
#include "samples_platform_dep.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common_build_info.h"
#include "sopc_crypto_profiles_lib_itf.h"
#include "sopc_date_time.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_sks.h"
#include "sopc_sk_manager.h"
#include "sopc_sub_scheduler.h"
#include "sopc_threads.h"
#include "sopc_time_reference.h"

// project includes
#include "cache.h"
#include "pubsub_config_static.h"
#include "static_security_data.h"
#include "test_config.h"

/***************************************************/
/**               MISC FUNCTIONS                   */
/***************************************************/
static void log_UserCallback(const char* timestampUtc,
                             const char* category,
                             const SOPC_Log_Level level,
                             const char* const line);

/***************************************************/
/**               SERVER CONFIGURATION             */
/***************************************************/
#define ASYNCH_CONTEXT_PARAM 0x12345678u
#define ASYNCH_CONTEXT_CACHE_SYNC 0x12345679u
#define APPLICATION_URI "urn:S2OPC:localhost"
#define PRODUCT_URI "urn:S2OPC:localhost"
#define SERVER_DESCRIPTION "S2OPC PubSub+Server demo Server"
#define LOCALE_ID "en-US"
static const char* g_userNamespaces[2] = {"urn:S2OPC:sopc_demo", NULL};
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
static SOPC_Thread CLI_thread;

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
static bool Server_LocalWriteSingleNode(const SOPC_NodeId* pNid, SOPC_DataValue* pDv);
static SOPC_DataValue* Server_LocalReadSingleNode(const SOPC_NodeId* pNid);

/***************************************************/
/**               PUBSUB VARIABLES CONTENT         */
/***************************************************/
static SOPC_PubSubConfiguration* pPubSubConfig = NULL;
static SOPC_SubTargetVariableConfig* pTargetConfig = NULL;
static SOPC_PubSourceVariableConfig* pSourceConfig = NULL;
static bool gPubStarted = false;
static bool gSubStarted = false;
static SOPC_PubSubState gSubOperational = SOPC_PubSubState_Disabled;
// Date of last reception on Sub
static SOPC_HighRes_TimeReference* gLastReceptionDateMs = NULL;
SOPC_SKManager* g_skmanager = NULL;

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
typedef char* WordList; // A simple C String
static int cmd_demo_help(WordList* pList);
static int cmd_demo_info(WordList* pList);
static int cmd_demo_dbg(WordList* pList);
static int cmd_demo_log(WordList* pList);
static int cmd_demo_pub(WordList* pList);
static int cmd_demo_sub(WordList* pList);
static int cmd_demo_write(WordList* pList);
static int cmd_demo_date(WordList* pList);
static int cmd_demo_read(WordList* pList);
static int cmd_demo_cache(WordList* pList);
static int cmd_demo_quit(WordList* pList);

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
                                          {"read", cmd_demo_read, "Print content of  <NodeId>"},
                                          {"date", cmd_demo_date, "Print date"},
                                          {"write", cmd_demo_write, "Write value to server"},
                                          {"pub", cmd_demo_pub, "Manage Publisher"},
                                          {"sub", cmd_demo_sub, "Manage Subscriber"},
                                          {"cache", cmd_demo_cache, "Print content of cache"},
                                          {NULL, NULL, NULL}};

#define NB_MAX_SUB_WRITERS 10

/***************************************************/
typedef struct WriterIdState
{
    uint16_t writerId;
    uint16_t groupId;
    SOPC_PubSubState state;
} WriterIdState;
static WriterIdState writerIdStates[NB_MAX_SUB_WRITERS] = {0};

/***************************************************/
static const char* subStateToString(SOPC_PubSubState state)
{
    switch (state)
    {
    case SOPC_PubSubState_Disabled:
        return "DISABLED";
    case SOPC_PubSubState_Paused:
        return "PAUSED";
    case SOPC_PubSubState_Operational:
        return "OPERATIONAL";
    case SOPC_PubSubState_Error:
        return "ERROR";
    default:
        return "<Unknown>";
    }
}
/***************************************************/
static const char* subWriterIdStateToString(uint16_t groupId, uint16_t writerId)
{
    SOPC_PubSubState state = SOPC_PubSubState_Error;
    for (int i = 0; i < NB_MAX_SUB_WRITERS; i++)
    {
        if (writerIdStates[i].writerId == writerId && writerIdStates[i].groupId == groupId)
        {
            state = writerIdStates[i].state;
        }
    }
    switch (state)
    {
    case SOPC_PubSubState_Disabled:
        return "DISABLED";
    case SOPC_PubSubState_Paused:
        return "PAUSED";
    case SOPC_PubSubState_Operational:
        return "OPERATIONAL";
    case SOPC_PubSubState_Error:
        return "ERROR";
    default:
        return "<Unknown>";
    }
}

/**************************************************************************/
static void serverStopped_cb(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);

    LOG_DEBUG("serverStopped_cb");
    SOPC_Atomic_Int_Set(&gStopped, 1);
    PRINT("Server stopped!\n");
}

static char logCategory[10];

/***************************************************/
static void log_UserCallback(const char* timestampUtc,
                             const char* category,
                             const SOPC_Log_Level level,
                             const char* const line)
{
    SOPC_UNUSED_ARG(category);
    SOPC_UNUSED_ARG(level);
    SOPC_UNUSED_ARG(timestampUtc);
    if (line != NULL && (category == NULL || logCategory[0] == 0 || strcmp(logCategory, category) == 0))
    {
        PRINT("%s\n", line);
    }
}

/***************************************************/
/* This function can be used to filter incoming connections based on user identification
 * This is an example to allow "user1/pass" authentication request
 **/
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
                PRINT("User <%s> has successfully authenticated", username);
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
            else
            {
                PRINT("User <%s> entered an invalid password", username);
            }
        }
        else
        {
            PRINT("Unknown user <%s>", username);
        }
    }

    return SOPC_STATUS_OK;
}

/***************************************************/
static void cacheSync_WriteToCache(const SOPC_NodeId* pNid, const SOPC_DataValue* pDv)
{
    Cache_Lock();
    SOPC_DataValue* pDvCache = Cache_Get(pNid);

    // Only write values of cache that are already defined
    if (pDvCache != NULL)
    {
        // Replace content of Cache
        SOPC_DataValue_Clear(pDvCache);
        SOPC_DataValue_Copy(pDvCache, pDv);
    }
    Cache_Unlock();
}

/***************************************************/
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
        PRINT("A client updated the content of node <%s> with a value of type %d\n", nodeId,
              writeValue->Value.Value.BuiltInTypeId);
        SOPC_Free(nodeId);
        // Synchronize cache for PubSub
        cacheSync_WriteToCache(&writeValue->NodeId, &writeValue->Value);
    }
    else
    {
        LOG_WARNING("Client write failed on server. returned code 0x%08X", (unsigned int) writeStatus);
    }
}

/***************************************************/
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
                LOG_WARNING("Internal data update[%d/%d] failed with code 0x%08X", (int) i,
                            (int) writeResp->NoOfResults, (unsigned int) status);
            }
        }
    }
}

/***************************************************/
/***
 * Request the server to update a node value (local request)
 * @param pNid The nodeId to modify
 * @param pDv The DataValue to write
 * @post \a localServiceAsyncRespCallback will be called with operation result
 */
static bool Server_LocalWriteSingleNode(const SOPC_NodeId* pNid, SOPC_DataValue* pDv)
{
    OpcUa_WriteRequest* request = SOPC_WriteRequest_Create(1);
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
        // Synchronize cache for PubSub
        cacheSync_WriteToCache(pNid, pDv);

        // param is not used here because this is a static context. In a more complex application,
        // param can be set to any context pointer that way be required by the application
        // We use a simple marker to show data transmission from here to localServiceAsyncRespCallback
        status = SOPC_ServerHelper_LocalServiceAsync(request, ASYNCH_CONTEXT_PARAM);
        if (status != SOPC_STATUS_OK)
        {
            LOG_WARNING("LocalServiceAsync failed with code  (%d)", status);
            SOPC_Free(request);
        }
    }

    return status == SOPC_STATUS_OK;
}

/***************************************************/
/***
 * Send a read request to the server and wait for the response
 * @param pNid The nodeId to read
 * @return a new allocated DataValue containing the result (or NULL in case of failure).
 *      Shall be freed by caller after use.
 */
static SOPC_DataValue* Server_LocalReadSingleNode(const SOPC_NodeId* pNid)
{
    OpcUa_ReadRequest* request = SOPC_ReadRequest_Create(1, OpcUa_TimestampsToReturn_Neither);
    OpcUa_ReadResponse* response = NULL;
    SOPC_ASSERT(NULL != request);

    SOPC_ReturnStatus status;
    status = SOPC_ReadRequest_SetReadValue(request, 0, pNid, SOPC_AttributeId_Value, NULL);
    if (status != SOPC_STATUS_OK)
    {
        LOG_WARNING("Read Value failed with code  %d", status);
        SOPC_Free(request);
        return NULL;
    }
    status = SOPC_ServerHelper_LocalServiceSync(request, (void**) &response);
    if (status != SOPC_STATUS_OK)
    {
        LOG_WARNING("Read Value failed with code  %d", status);
        SOPC_ASSERT(NULL == response);
        SOPC_Free(request);
        return NULL;
    }

    SOPC_DataValue* result = NULL;
    if (response != NULL && response->NoOfResults == 1)
    {
        // Allocate the result only if the response contains exactly the expected content
        result = SOPC_Malloc(sizeof(*result));
        SOPC_ASSERT(NULL != result);
        SOPC_DataValue_Initialize(result);
        SOPC_DataValue_Copy(result, &response->Results[0]);
    }
    OpcUa_ReadResponse_Clear(response);
    SOPC_Free(response);
    return result;
}

/***************************************************/
static void forEach_CacheInit(const SOPC_NodeId* nid, SOPC_DataValue* dv)
{
    SOPC_DataValue* asDv = Server_LocalReadSingleNode(nid);
    SOPC_DataValue_Copy(dv, asDv);
    SOPC_DataValue_Clear(asDv);
    SOPC_Free(asDv);
}

/***************************************************/
static void initializeCacheFromAddrSpace(void)
{
    Cache_ForEach_Exec exec = {.pExec = &forEach_CacheInit};
    Cache_Lock();
    Cache_ForEach(&exec);
    Cache_Unlock();
}

/***
 * @brief Creates and setup the OPCUA server, using KConfig parameters
 */
static void setupServer(void)
{
    SOPC_ReturnStatus status;

    status = SOPC_ServerConfigHelper_Initialize();
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_ServerConfigHelper_Initialize failed");

    PRINT("S2OPC initialization OK\n");

    //////////////////////////////////
    // Namespaces initialization
    status = SOPC_ServerConfigHelper_SetNamespaces(1, g_userNamespaces);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_ServerConfigHelper_SetNamespaces failed");

    status = SOPC_ServerConfigHelper_SetLocaleIds(1, g_localesArray);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_ServerConfigHelper_SetLocaleIds failed");

    //////////////////////////////////
    // Global descriptions initialization
    status = SOPC_ServerConfigHelper_SetApplicationDescription(APPLICATION_URI, PRODUCT_URI, SERVER_DESCRIPTION,
                                                               LOCALE_ID, OpcUa_ApplicationType_Server);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_ServerConfigHelper_SetApplicationDescription failed");

    //////////////////////////////////
    // Create endpoints configuration
    SOPC_SecurityPolicy* sp;

    PRINT("Create endpoint '%s'\n", CONFIG_SOPC_ENDPOINT_ADDRESS);
    g_epConfig = SOPC_ServerConfigHelper_CreateEndpoint(CONFIG_SOPC_ENDPOINT_ADDRESS, true);
    SOPC_ASSERT(NULL != g_epConfig && "SOPC_ServerConfigHelper_CreateEndpoint failed");

    PRINT("Setting up security...\n");

    /* 1st Security policy is None without user (users on unsecure channel shall be forbidden) */
    sp = SOPC_EndpointConfig_AddSecurityConfig(g_epConfig, SOPC_SecurityPolicy_None);
    SOPC_ASSERT(NULL != sp && "SOPC_EndpointConfig_AddSecurityConfig #1 failed");

    status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SECURITY_MODE_NONE_MASK);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_SetSecurityModes #1 failed");

    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
    SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_AddUserTokenPolicy #1/#1 failed");

    if (SOPC_CryptoProfile_Is_Implemented())
    {
        /* 2nd Security policy is Basic256 with anonymous or username authentication allowed
         * (without password encryption) */
        sp = SOPC_EndpointConfig_AddSecurityConfig(g_epConfig, SOPC_SecurityPolicy_Basic256);
        SOPC_ASSERT(NULL != sp && "SOPC_EndpointConfig_AddSecurityConfig #2 failed");

        status = SOPC_SecurityConfig_SetSecurityModes(
            sp, SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK);
        SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_SetSecurityModes #2 failed");

        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
        SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_AddUserTokenPolicy #2/#1 failed");
        status =
            SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy);
        SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_AddUserTokenPolicy #2/#2 failed");

        /* 3rd Security policy is Basic256Sha256 with anonymous or username authentication allowed
         * (without password encryption) */
        sp = SOPC_EndpointConfig_AddSecurityConfig(g_epConfig, SOPC_SecurityPolicy_Basic256Sha256);
        SOPC_ASSERT(NULL != sp && "SOPC_EndpointConfig_AddSecurityConfig #3 failed");

        status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK);
        SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_SetSecurityModes #3 failed");

        status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
        SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_AddUserTokenPolicy #3/#1 failed");
        status =
            SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy);
        SOPC_ASSERT(status == SOPC_STATUS_OK && "SOPC_SecurityConfig_AddUserTokenPolicy #3/#2 failed");

        //////////////////////////////////
        // Server certificates configuration
        SOPC_CertificateList* ca_cert = NULL;
        SOPC_CRLList* crl = NULL;
        SOPC_PKIProvider* pkiProvider = NULL;
        status = SOPC_ServerConfigHelper_SetKeyCertPairFromBytes(sizeof(server_2k_cert), server_2k_cert,
                                                                 sizeof(server_2k_key), server_2k_key);
        SOPC_ASSERT(SOPC_STATUS_OK == status && "SOPC_ServerConfigHelper_SetKeyCertPairFromBytes() failed");

        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(cacert, sizeof(cacert), &ca_cert);
        SOPC_ASSERT(SOPC_STATUS_OK == status && "SOPC_KeyManager_Certificate_CreateOrAddFromDER() failed");
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &crl);
        SOPC_ASSERT(SOPC_STATUS_OK == status && "SOPC_KeyManager_CRL_CreateOrAddFromDER() failed");

        status = SOPC_PKIProvider_CreateFromList(ca_cert, crl, NULL, NULL, &pkiProvider);
        SOPC_ASSERT(SOPC_STATUS_OK == status && "SOPC_PKIProvider_CreateFromList() failed");
        SOPC_KeyManager_Certificate_Free(ca_cert);
        SOPC_KeyManager_CRL_Free(crl);

        status = SOPC_ServerConfigHelper_SetPKIprovider(pkiProvider);
        SOPC_ASSERT(SOPC_STATUS_OK == status && "SOPC_ServerConfigHelper_SetPKIprovider failed");

        PRINT("Test_Server_Client: Certificates and key loaded\n");
    }

    //////////////////////////////////
    // Setup AddressSpace
    (void) sopc_embedded_is_const_addspace;
    SOPC_ASSERT(sopc_embedded_is_const_addspace && "Address space must be constant.");
    LOG_INFO("# Loading AddressSpace (%" PRIu32 " nodes)...\n", SOPC_Embedded_AddressSpace_nNodes);
    SOPC_AddressSpace* addSpace =
        SOPC_AddressSpace_CreateReadOnlyNodes(SOPC_Embedded_AddressSpace_nNodes, SOPC_Embedded_AddressSpace_Nodes,
                                              SOPC_Embedded_VariableVariant_nb, SOPC_Embedded_VariableVariant);
    SOPC_ASSERT(NULL != addSpace && "SOPC_AddressSpace_Create failed");
    LOG_INFO("# Address space loaded\n");

    status = SOPC_ServerConfigHelper_SetAddressSpace(addSpace);
    SOPC_ASSERT(SOPC_STATUS_OK == status && "SOPC_ServerConfigHelper_SetAddressSpace failed");

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
    authenticationManager->pUsrPKI = NULL;
    authenticationManager->pFunctions = &authentication_functions;
    SOPC_ServerConfigHelper_SetUserAuthenticationManager(authenticationManager);
    SOPC_ServerConfigHelper_SetUserAuthorizationManager(authorizationManager);

    status = SOPC_ServerConfigHelper_SetWriteNotifCallback(&serverWriteEvent);
    SOPC_ASSERT(SOPC_STATUS_OK == status && "SOPC_ServerConfigHelper_SetWriteNotifCallback failed");

    //////////////////////////////////
    // Set the asynchronous event callback
    status = SOPC_ServerConfigHelper_SetLocalServiceAsyncResponse(localServiceAsyncRespCallback);
    SOPC_ASSERT(SOPC_STATUS_OK == status && "SOPC_ServerConfigHelper_SetLocalServiceAsyncResponse failed");

    SOPC_ServerConfigHelper_SetShutdownCountdown(1);
}

/***************************************************/
static void clearServer(void)
{
    LOG_DEBUG("SOPC_ServerConfigHelper_Clear");
    SOPC_ServerConfigHelper_Clear();
}

/***************************************************/
static void clearPubSub(void)
{
    SOPC_SubScheduler_Stop();
    gSubStarted = false;
    SOPC_PubScheduler_Stop();
    gSubStarted = false;

    if (NULL != g_skmanager)
    {
        SOPC_SKManager_Clear(g_skmanager);
        SOPC_Free(g_skmanager);
        g_skmanager = NULL;
    }

    SOPC_PubSubSKS_Clear();
    Cache_Clear();
}

/***************************************************/
static bool Server_SetTargetVariables(const OpcUa_WriteValue* lwv, const int32_t nbValues)
{
    if (SOPC_Atomic_Int_Get(&gStopped) != 0)
    {
        return true;
    }

    SOPC_HighRes_TimeReference_GetTime(gLastReceptionDateMs);

    /* Encapsulate the WriteValues in a WriteRequest and send it as a local service,
     * acknowledge before the toolkit answers */
    OpcUa_WriteRequest* request = NULL;
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Create(&OpcUa_WriteRequest_EncodeableType, (void**) &request);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    if (NULL == request)
    {
        return false;
    }

    request->NoOfNodesToWrite = nbValues;
    request->NodesToWrite = SOPC_Calloc((size_t) request->NoOfNodesToWrite, sizeof(*request->NodesToWrite));
    SOPC_ASSERT(NULL != request->NodesToWrite);
    for (int i = 0; i < request->NoOfNodesToWrite; i++)
    {
        SOPC_EncodeableObject_Initialize(lwv->encodeableType, &request->NodesToWrite[i]);
        status = SOPC_EncodeableObject_Copy(lwv->encodeableType, &request->NodesToWrite[i], &lwv[i]);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }
    SOPC_ServerHelper_LocalServiceAsync(request, ASYNCH_CONTEXT_CACHE_SYNC);

    return true;
}

static SOPC_SKManager* createSKmanager(void)
{
    /* Create Service Keys manager and set constant keys */
    SOPC_SKManager* skm = SOPC_SKManager_Create();
    SOPC_ASSERT(NULL != skm && "SOPC_SKManager_Create failed");
    uint32_t nbKeys = 0;
    SOPC_Buffer* keysBuffer =
        SOPC_Buffer_Create(sizeof(pubSub_keySign) + sizeof(pubSub_keyEncrypt) + sizeof(pubSub_keyNonce));
    SOPC_ReturnStatus status = (NULL == keysBuffer ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(keysBuffer, pubSub_keySign, (uint32_t) sizeof(pubSub_keySign));
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(keysBuffer, pubSub_keyEncrypt, (uint32_t) sizeof(pubSub_keyEncrypt));
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Write(keysBuffer, pubSub_keyNonce, (uint32_t) sizeof(pubSub_keyNonce));
    }
    SOPC_ByteString keys;
    SOPC_ByteString_Initialize(&keys);
    SOPC_String securityPolicyUri;
    SOPC_String_Initialize(&securityPolicyUri);
    if (SOPC_STATUS_OK == status)
    {
        nbKeys = 1;
        // Set buffer as a byte string for API compatibility
        keys.DoNotClear = true;
        keys.Length = (int32_t) keysBuffer->length;
        keys.Data = (SOPC_Byte*) keysBuffer->data;
        // Set security policy
        status = SOPC_String_AttachFromCstring(&securityPolicyUri, SOPC_SecurityPolicy_PubSub_Aes256_URI);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SKManager_SetKeys(skm, &securityPolicyUri, 1, &keys, nbKeys, UINT32_MAX, UINT32_MAX);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_SKManager_Clear(skm);
        SOPC_Free(skm);
        skm = NULL;
    }
    SOPC_Buffer_Delete(keysBuffer);

    return skm;
}

/*****************************************************
 * @brief Creates and setup the PubSub, using KConfig parameters
 */
static void setupPubSub(void)
{
    // CONFIGURE PUBSUB
    pPubSubConfig = SOPC_PubSubConfig_GetStatic();
    SOPC_ASSERT(NULL != pPubSubConfig && "SOPC_PubSubConfig_GetStatic failed");

    /* Sub target configuration */
    pTargetConfig = SOPC_SubTargetVariableConfig_Create(&Server_SetTargetVariables);
    SOPC_ASSERT(NULL != pTargetConfig && "SOPC_SubTargetVariableConfig_Create failed");

    /* Pub target configuration */
    pSourceConfig = SOPC_PubSourceVariableConfig_Create(&Cache_GetSourceVariables);
    SOPC_ASSERT(NULL != pSourceConfig && "SOPC_PubSourceVariableConfig_Create failed");

    /* PubSub Security Keys configuration */
    g_skmanager = createSKmanager();
    SOPC_ASSERT(NULL != g_skmanager && "SOPC_SKManager_SetKeys failed");
    SOPC_PubSubSKS_Init();
    SOPC_PubSubSKS_SetSkManager(g_skmanager);

    Cache_Initialize(pPubSubConfig, true);
}

/***************************************************/
/* This function receives a pointer to a C string.
 * It returns the string pointed to by pList and replaces the
 * next space by a NULL char so that the return value is now a C String
 * containing the first word of the string.
 * pList is modified to point to the next char after the inserted NULL.
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
static void* CLI_thread_exec(void* arg)
{
    SOPC_UNUSED_ARG(arg);
    PRINT("Command-Line interface ready\n");

    while (SOPC_Atomic_Int_Get(&gStopped) == 0)
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
    PRINT("Build date : " __DATE__ " " __TIME__ "\n");

    /* Setup platform-dependant features (network, ...)*/
    SOPC_Platform_Setup();

    /* Configure the server logger (user logger) */
    SOPC_Log_Configuration logConfig;
    logConfig.logSystem = SOPC_LOG_SYSTEM_USER;
    logConfig.logLevel = SOPC_LOG_LEVEL_WARNING;
    logConfig.logSysConfig.userSystemLogConfig.doLog = &log_UserCallback;
    status = SOPC_CommonHelper_Initialize(&logConfig);
    SOPC_ASSERT(SOPC_STATUS_OK == status && "SOPC_CommonHelper_Initialize failed");

    gLastReceptionDateMs = SOPC_HighRes_TimeReference_Create();

    setupServer();
    setupPubSub();

    //////////////////////////////////
    // Start the server
    SOPC_Atomic_Int_Set(&gStopped, 0);
    status = SOPC_ServerHelper_StartServer(&serverStopped_cb);
    SOPC_ASSERT(SOPC_STATUS_OK == status && "SOPC_ServerHelper_StartServer failed");

    // Check for server status after some time. (Start is asynchronous)
    SOPC_Sleep(100);
    SOPC_ASSERT(SOPC_Atomic_Int_Get(&gStopped) == 0 && "Server failed to start.");

    // Setup default values of Cache using AddressSpace content
    initializeCacheFromAddrSpace();

    /* Create thread for Command Line Input management*/
    status = SOPC_Thread_Create(&CLI_thread, &CLI_thread_exec, NULL, "CLI");
    SOPC_ASSERT(SOPC_STATUS_OK == status && "SOPC_Thread_Create failed");

    // Wait for termination
    while (SOPC_Atomic_Int_Get(&gStopped) == 0)
    {
        // Process command line if any
        SOPC_Sleep(50);
    }

    SOPC_Atomic_Int_Set(&gStopped, 1);

    clearPubSub();
    clearServer();

    LOG_DEBUG("SOPC_CommonHelper_Clear");
    SOPC_CommonHelper_Clear();

    SOPC_HighRes_TimeReference_Delete(&gLastReceptionDateMs);

    LOG_INFO("# Info: Server closed.\n");

    SOPC_Platform_Shutdown(true);
}

/*---------------------------------------------------------------------------
 *                            CLI implementation
 *---------------------------------------------------------------------------*/
/***************************************************/
static int cmd_demo_help(WordList* pList)
{
    SOPC_UNUSED_ARG(pList);

    PRINT("S2OPC PubSub+Server demo commands:\n");

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

    PRINT("S2OPC PubSub+Server demo status\n");
    PRINT("Server endpoint       : %s\n", CONFIG_SOPC_ENDPOINT_ADDRESS);
    PRINT("Server running        : %s\n", YES_NO(gStopped == 0));
    PRINT("Server const@space    : %s\n", YES_NO(sopc_embedded_is_const_addspace));
    PRINT("Server toolkit version: %s\n", SOPC_TOOLKIT_VERSION);
    PRINT("Server src commit     : %s\n", buildInfo.buildSrcCommit);
    PRINT("Server docker Id      : %s\n", buildInfo.buildDockerId);
    PRINT("Server build date     : %s\n", buildInfo.buildBuildDate);
    PRINT("Publisher address     : %s\n", CONFIG_SOPC_PUBLISHER_ADDRESS);
    PRINT("Publisher running     : %s\n", YES_NO(gPubStarted));
    PRINT("Publisher period      : %d ms\n", CONFIG_SOPC_PUBLISHER_PERIOD_US / 1000);
    PRINT("Subscriber            : %s : %s\n", CONFIG_SOPC_SUBSCRIBER_ADDRESS, subStateToString(gSubOperational));
    if (gSubOperational == SOPC_PubSubState_Operational)
    {
        int delta_ms = (int) (SOPC_HighRes_TimeReference_DeltaUs(gLastReceptionDateMs, NULL) / 1000);
        PRINT(" -> Last Rcp: %d ms\n", delta_ms);

        if (pPubSubConfig != NULL)
        {
            uint32_t nbSub = SOPC_PubSubConfiguration_Nb_SubConnection(pPubSubConfig);
            for (uint32_t iSub = 0; iSub < nbSub; iSub++)
            {
                SOPC_PubSubConnection* cnx = SOPC_PubSubConfiguration_Get_SubConnection_At(pPubSubConfig, iSub);
                uint16_t nbReader = SOPC_PubSubConnection_Nb_ReaderGroup(cnx);
                for (uint16_t iReader = 0; iReader < nbReader; iReader++)
                {
                    SOPC_ReaderGroup* group = SOPC_PubSubConnection_Get_ReaderGroup_At(cnx, iReader);
                    uint16_t groupId = SOPC_ReaderGroup_Get_GroupId(group);
                    uint8_t nbDsm = SOPC_ReaderGroup_Nb_DataSetReader(group);
                    for (uint8_t iDsm = 0; iDsm < nbDsm; iDsm++)
                    {
                        SOPC_DataSetReader* reader = SOPC_ReaderGroup_Get_DataSetReader_At(group, iDsm);
                        uint16_t writerId = SOPC_DataSetReader_Get_DataSetWriterId(reader);
                        PRINT(" -> GroupId= %" PRIu16 " (WriterId= %" PRIu16 ") : %s\n", groupId, writerId,
                              subWriterIdStateToString(groupId, writerId));
                    }
                }
            }
        }
    }
    const char* netItf = SOPC_Platform_Get_Default_Net_Itf();
    if (netItf != NULL && netItf[0] != 0)
    {
        PRINT("NET INTERFACE         : %s\n", SOPC_Platform_Get_Default_Net_Itf());
    }

    return 0;
}

/***************************************************/
static int cmd_demo_dbg(WordList* pList)
{
    const char* word = CLI_GetNextWord(pList);

    PRINT("S2OPC PubSub+Server target debug informations:\n");
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
    case '0':
        logCategory[0] = 0;
        break;
    case 'c':
    {
        memset(logCategory, 0, sizeof(logCategory));
        const char* category = CLI_GetNextWord(pList);
        strncpy(logCategory, category, sizeof(logCategory) - 1);
    }
    break;
    default:
        PRINT("usage: log <c <category>|D|I|W|E>\n");
        break;
    }
    return 0;
}

/***************************************************/
static int cmd_demo_pub(WordList* pList)
{
    const char* word = CLI_GetNextWord(pList);

    if (0 == strcmp(word, "start"))
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
    if (0 == strcmp(word, "stop"))
    {
        SOPC_PubScheduler_Stop();
        gPubStarted = false;
        return 0;
    }
    PRINT("usage: pub [start|stop] (found %s)\n", word);
    return 0;
}

/***************************************************/
static void cb_SetSubStatus(const SOPC_Conf_PublisherId* pubId,
                            uint16_t groupId,
                            uint16_t writerId,
                            SOPC_PubSubState state)
{
    if (pubId == NULL)
    {
        PRINT("New Sub state: %d\n", (int) state);
        gSubOperational = state;
    }
    else
    {
        int freeIndex = -1;
        for (int i = 0; i < NB_MAX_SUB_WRITERS; i++)
        {
            if (freeIndex == -1 && writerIdStates[i].writerId == 0 && writerIdStates[i].groupId == 0)
            {
                freeIndex = i;
            }
            if (writerIdStates[i].writerId == writerId && writerIdStates[i].groupId == groupId)
            {
                writerIdStates[i].state = state;
                freeIndex = -2;
            }
        }
        if (freeIndex >= 0)
        {
            writerIdStates[freeIndex].state = state;
            writerIdStates[freeIndex].writerId = writerId;
            writerIdStates[freeIndex].groupId = groupId;
        }
        PRINT("New Sub state for GroupId %d WriterId %d: %d\n", (int) groupId, (int) writerId, (int) state);
    }
}

/***************************************************/
static void cb_ReceiveGapDsmSequenceNumber(SOPC_Conf_PublisherId pubId,
                                           uint16_t groupId,
                                           uint16_t writerId,
                                           uint16_t prevSN,
                                           uint16_t receivedSN)
{
    if (SOPC_UInteger_PublisherId == pubId.type)
    {
        PRINT("Gap detected in sequence numbers of DataSetMessage for PublisherId=%" PRIu64 " GroupId =%" PRIu16
              " DataSetWriterId=%" PRIu16 ", missing SNs: [%" PRIu16 ", %" PRIu16 "]\n",
              pubId.data.uint, groupId, writerId, prevSN + 1, receivedSN - 1);
    }
    else
    {
        PRINT("Gap detected in sequence numbers of DataSetMessage for PublisherId=%s GroupId =%" PRIu16
              " DataSetWriterId=%" PRIu16 ", missing SNs: [%" PRIu16 ", %" PRIu16 "]\n",
              SOPC_String_GetRawCString(&pubId.data.string), groupId, writerId, prevSN + 1, receivedSN - 1);
    }
}

/***************************************************/
static int cmd_demo_sub(WordList* pList)
{
    const char* word = CLI_GetNextWord(pList);

    if (0 == strcmp(word, "start"))
    {
        // start subscriber (will fail if already started)
        bool bResult;
        bResult = SOPC_SubScheduler_Start(pPubSubConfig, pTargetConfig, cb_SetSubStatus, cb_ReceiveGapDsmSequenceNumber,
                                          NULL, CONFIG_SOPC_SUBSCRIBER_PRIORITY);
        if (!bResult)
        {
            PRINT("\r\nFailed to start Subscriber!\r\n");
            return 1;
        }
        else
        {
            gSubStarted = true;
            PRINT("\r\nSubscriber started\r\n");
            return 0;
        }
    }
    if (0 == strcmp(word, "stop"))
    {
        SOPC_SubScheduler_Stop();
        gSubStarted = false;
        return 0;
    }

    PRINT("usage: sub [start|stop]\n");
    return 0;
}

/***************************************************/
/** Allow parsing of a nodeId but tries with prepending prefix "ns=1;s=" in case of
 * failure to ease user input */
static SOPC_ReturnStatus createNodeIdFromInput(SOPC_NodeId* pNid, const char* input)
{
    const size_t len = strlen(input);
    if (len == 0)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ReturnStatus status = SOPC_NodeId_InitializeFromCString(pNid, input, (int32_t) len);
    if (SOPC_STATUS_OK != status)
    {
        static const char* prefix = "ns=1;s=";
        char* nodeId = NULL;

        status = SOPC_StrConcat(prefix, input, &nodeId);
        if (SOPC_STATUS_OK == status)
        {
            PRINT("Could not find nodeId '%s', trying with '%s'\n", input, nodeId);
            status = SOPC_NodeId_InitializeFromCString(pNid, nodeId, (int32_t) strlen(nodeId));
            SOPC_Free(nodeId);
        }
    }
    return status;
}

/***************************************************/
static int cmd_demo_write(WordList* pList)
{
    const char* nodeIdC = CLI_GetNextWord(pList);
    const char* dvC = CLI_GetNextWord(pList);

    SOPC_NodeId nid;
    SOPC_ReturnStatus status = createNodeIdFromInput(&nid, nodeIdC);
    if (SOPC_STATUS_OK != status || dvC[0] == 0)
    {
        PRINT("usage: demo write <nodeid> <value>\n");
        PRINT("<value> must be prefixed by b for a BOOL, s for a String, B for a byte,\n");
        PRINT("        i for a INT32, u for a UINT32\n");
        PRINT("Other formats not implemented here.\n");
        return 0;
    }

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

    Server_LocalWriteSingleNode(&nid, &dv);

    SOPC_NodeId_Clear(&nid);
    SOPC_DataValue_Clear(&dv);
    return 0;
}

/***************************************************/
static int cmd_demo_date(WordList* pList)
{
    SOPC_UNUSED_ARG(pList);

    char* date = SOPC_Time_GetStringOfCurrentLocalTime(false);
    PRINT("%s\n", date);
    SOPC_Free(date);
    return 0;
}

/***************************************************/
static int cmd_demo_read(WordList* pList)
{
    const char* nodeIdC = CLI_GetNextWord(pList);

    SOPC_NodeId nid;
    SOPC_ReturnStatus status = createNodeIdFromInput(&nid, nodeIdC);

    if (SOPC_STATUS_OK != status)
    {
        PRINT("usage: demo read <nodeid>\n");
        return 0;
    }

    SOPC_DataValue* dv = Server_LocalReadSingleNode(&nid);

    if (NULL == dv)
    {
        PRINT("Failed to read node '%s'\n", nodeIdC);
        SOPC_NodeId_Clear(&nid);
        return 1;
    }

    Cache_Dump_VarValue(&nid, dv);

    SOPC_NodeId_Clear(&nid);
    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);
    return 0;
}

/***************************************************/
static int cmd_demo_cache(WordList* pList)
{
    SOPC_UNUSED_ARG(pList);

    Cache_Dump();
    return 0;
}

/***************************************************/
static int cmd_demo_quit(WordList* pList)
{
    SOPC_UNUSED_ARG(pList);
    if (gPubStarted)
    {
        SOPC_PubScheduler_Stop();
        gPubStarted = false;
    }
    if (gSubStarted)
    {
        SOPC_SubScheduler_Stop();
        gSubStarted = false;
    }
    SOPC_ServerHelper_StopServer();
    LOG_WARNING("Server manually stopped!\n");

    return 0;
}
