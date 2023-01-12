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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MIMXRT1064 includes */
#include "fsl_debug_console.h"

/* FreeRTOS include */
#include "FreeRTOS.h"
#include "task.h" // Used for disable task entering in user assert

/* S2OPC includes */
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_common_constants.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "embedded/sopc_addspace_loader.h"

#include "static_security_data.h"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI_2 "urn:S2OPC:localhost_2"

/* Define application namespaces: ns=1 and ns=2 */
static const char* default_app_namespace_uris[] = {DEFAULT_PRODUCT_URI, DEFAULT_PRODUCT_URI_2};
static const char* default_locale_ids[] = {"en-US", "fr-FR"};

static const bool secuActive = true;

/*---------------------------------------------------------------------------
 *                          Log Callback definition
 *---------------------------------------------------------------------------*/

static void log_UserCallback(const char* context, const char* text)
{
    SOPC_UNUSED_ARG(context);
    if (NULL != text)
    {
        PRINTF("%s\r\n", text);
    }
}

/*---------------------------------------------------------------------------
 *                          Assert Callback definition
 *---------------------------------------------------------------------------*/

static void assert_userCallback(const char* context)
{
    PRINTF("ASSERT FAILED : <%p>\r\n", (void*) context);
    PRINTF("Context: <%s>", context);
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

static SOPC_StatusCode SOPC_Method_Func_Test_Generic(const SOPC_CallContext* callContextPtr,
                                                     const SOPC_NodeId* objectId,
                                                     uint32_t nbInputArgs,
                                                     const SOPC_Variant* inputArgs,
                                                     uint32_t* nbOutputArgs,
                                                     SOPC_Variant** outputArgs,
                                                     void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(param);
    *nbOutputArgs = 0;
    *outputArgs = NULL;
    PRINTF("[INFO] SOPC_Method_Func_Test_Generic: Number of input argument %" PRIu32
           " number of output argument %" PRIu32 "\r\n",
           nbInputArgs, *nbOutputArgs);
    return SOPC_STATUS_OK;
}

static SOPC_StatusCode SOPC_Method_Func_Test_CreateSigningRequest(const SOPC_CallContext* callContextPtr,
                                                                  const SOPC_NodeId* objectId,
                                                                  uint32_t nbInputArgs,
                                                                  const SOPC_Variant* inputArgs,
                                                                  uint32_t* nbOutputArgs,
                                                                  SOPC_Variant** outputArgs,
                                                                  void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(objectId);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(param);
    *nbOutputArgs = 1;
    *outputArgs = SOPC_Calloc(1, sizeof(SOPC_Variant));
    assert(NULL != *outputArgs);
    SOPC_Variant_Initialize(&((*outputArgs)[0]));
    (*outputArgs)[0].BuiltInTypeId = SOPC_UInt32_Id;
    (*outputArgs)[0].Value.Uint32 = 42;
    return SOPC_STATUS_OK;
}

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

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

/*----------------------------------------------------
 * Application description and endpoint configuration:
 *---------------------------------------------------*/

/*
 * Configure the cryptographic parameters of the endpoint:
 * - Server certificate and key
 * - Public Key Infrastructure: using a single certificate as Certificate Authority or Trusted Certificate
 */
static SOPC_ReturnStatus Server_SetDefaultCryptographicConfig(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (secuActive)
    {
        SOPC_PKIProvider* pkiProvider = NULL;

        SOPC_SerializedCertificate* serializedCAcert = NULL;
        SOPC_CRLList* serializedCAcrl = NULL;

        /* Load client/server certificates and server key from C source files (no filesystem needed) */
        status = SOPC_HelperConfigServer_SetKeyCertPairFromBytes(sizeof(server_2k_cert), server_2k_cert,
                                                                 sizeof(server_2k_key), server_2k_key);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(cacert, sizeof(cacert), &serializedCAcert);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &serializedCAcrl);
        }

        /* Create the PKI (Public Key Infrastructure) provider */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_Create(serializedCAcert, serializedCAcrl, &pkiProvider);
        }
        SOPC_KeyManager_SerializedCertificate_Delete(serializedCAcert);

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_HelperConfigServer_SetPKIprovider(pkiProvider);
        }

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed loading certificates and key (check paths are valid)\n");
        }
        else
        {
            printf("<Test_Server_Toolkit: Certificates and key loaded\n");
        }
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                          Server initialization
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_Initialize(void)
{
    // Set user assert
    SOPC_Assert_Set_UserCallback(&assert_userCallback);

    // Initialize toolkit and configure logs
    SOPC_Log_Configuration logConfiguration = {.logLevel = SOPC_LOG_LEVEL_WARNING,
                                               .logSystem = SOPC_LOG_SYSTEM_USER,
                                               .logSysConfig = {.userSystemLogConfig = {.doLog = &log_UserCallback}}};

    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_Initialize();
    }
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Toolkit: Failed initializing\n");
    }
    else
    {
        printf("<Test_Server_Toolkit: initialized\n");
    }
    return status;
}

/*-------------------------
 * Method call management :
 *-------------------------*/
static SOPC_ReturnStatus Server_InitDefaultCallMethodService(void)
{
    char* sNodeId;
    SOPC_NodeId* methodId;
    SOPC_MethodCallFunc_Ptr* methodFunc;
    /* Create and define the method call manager the server will use*/
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    SOPC_StatusCode status = (NULL != mcm) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetMethodCallManager(mcm);
    }

    /* Add methods implementation in the method call manager used */
    if (SOPC_STATUS_OK == status)
    {
        /* No input, no output */
        sNodeId = "ns=1;s=MethodNoArg";
        methodId = SOPC_NodeId_FromCString(sNodeId, (int32_t) strlen(sNodeId));
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_Test_Generic;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "No input, no output", NULL);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        /* Only input, no output */
        sNodeId = "ns=1;s=MethodI";
        methodId = SOPC_NodeId_FromCString(sNodeId, (int32_t) strlen(sNodeId));
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_Test_Generic;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "Only input, no output", NULL);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* No input, only output */
        sNodeId = "ns=1;s=MethodO";
        methodId = SOPC_NodeId_FromCString(sNodeId, (int32_t) strlen(sNodeId));
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_Test_Generic;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "No input, only output", NULL);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Input, output */
        sNodeId = "ns=1;s=MethodIO";
        methodId = SOPC_NodeId_FromCString(sNodeId, (int32_t) strlen(sNodeId));
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_Test_CreateSigningRequest;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "Input, output", NULL);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
    return status;
}

/*
 * Default server configuration loader (without XML configuration)
 */
static SOPC_ReturnStatus Server_SetDefaultConfiguration(void)
{
    // Set namespaces
    SOPC_ReturnStatus status = SOPC_HelperConfigServer_SetNamespaces(sizeof(default_app_namespace_uris) / sizeof(char*),
                                                                     default_app_namespace_uris);
    // Set locale ids
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetLocaleIds(sizeof(default_locale_ids) / sizeof(char*), default_locale_ids);
    }

    // Set application description of server to be returned by discovery services (GetEndpoints, FindServers)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                                   "S2OPC toolkit server example", "en-US",
                                                                   OpcUa_ApplicationType_Server);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_AddApplicationNameLocale("S2OPC toolkit: exemple de serveur", "fr-FR");
    }

    /*
     * Create new endpoint in server
     */
    SOPC_Endpoint_Config* ep = NULL;
    if (SOPC_STATUS_OK == status)
    {
        ep = SOPC_HelperConfigServer_CreateEndpoint(DEFAULT_ENDPOINT_URL, true);
        status = NULL == ep ? SOPC_STATUS_OUT_OF_MEMORY : status;
    }

    /*
     * Define the certificates, security policies, security modes and user token policies supported by endpoint
     */
    SOPC_SecurityPolicy* sp;
    if (SOPC_STATUS_OK == status && secuActive)
    {
        /*
         * 1st Security policy is Basic256Sha256 with anonymous and username (non encrypted) authentication allowed
         */
        sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Basic256Sha256);
        if (NULL == sp)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_SecurityConfig_SetSecurityModes(
                sp, SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt);

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
            }
            if (SOPC_STATUS_OK == status)
            {
                status =
                    SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy);
            }
        }

        /*
         * 2nd Security policy is Basic256 with anonymous and username (non encrypted) authentication allowed
         */
        if (SOPC_STATUS_OK == status)
        {
            sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Basic256);
            if (NULL == sp)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                status = SOPC_SecurityConfig_SetSecurityModes(
                    sp, SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt);

                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
                }
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(
                        sp, &SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy);
                }
            }
        }

        /*
         * 4th Security policy is Aes128-Sha256-RsaOaep with anonymous and username (non encrypted) authentication
         * allowed
         */
        if (SOPC_STATUS_OK == status)
        {
            sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Aes128Sha256RsaOaep);
            if (NULL == sp)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                status = SOPC_SecurityConfig_SetSecurityModes(
                    sp, SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt);

                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
                }
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(
                        sp, &SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy);
                }
            }
        }

        /*
         * 5th Security policy is Aes256-Sha256-RsaPss with anonymous and username (non encrypted) authentication
         * allowed
         */
        if (SOPC_STATUS_OK == status)
        {
            sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Aes256Sha256RsaPss);
            if (NULL == sp)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else
            {
                status = SOPC_SecurityConfig_SetSecurityModes(
                    sp, SOPC_SecurityModeMask_Sign | SOPC_SecurityModeMask_SignAndEncrypt);

                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
                }
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_SecurityConfig_AddUserTokenPolicy(
                        sp, &SOPC_UserTokenPolicy_UserName_DefaultSecurityPolicy);
                }
            }
        }
    }

    /*
     * 3rd Security policy is None with anonymous and username (non encrypted) authentication allowed
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
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_SecurityConfig_AddUserTokenPolicy(
                    sp,
                    &SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy); /* Necessary for UACTT tests only */
            }
        }
    }

    /**
     * Define server certificate and PKI provider
     */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SetDefaultCryptographicConfig();
    }

    if (!SOPC_EndpointConfig_AddClientToConnect(ep, NULL, "opc.tcp://localhost:4844"))
    {
        status = SOPC_STATUS_NOK;
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

    SOPC_AddressSpace* addSpace = SOPC_Embedded_AddressSpace_Load();
    status = (NULL != addSpace) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetAddressSpace(addSpace);
    }

    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Toolkit: Failed to configure the @ space\n");
    }
    else
    {
        printf("<Test_Server_Toolkit: @ space configured\n");
    }

    return status;
}

/*----------------------------------------
 * Users authentication and authorization:
 *----------------------------------------*/

/* The toolkit test servers shall pass the UACTT tests. Hence it shall authenticate
 * (ids and passwords can be changed in the UACTT settings/Server Test/Session):
 *  - anonymous users
 *  - user1:password
 *  - user2:password1
 * Then it shall accept username:password, but return "access denied".
 * Otherwise it shall be "identity token rejected".
 */
static SOPC_ReturnStatus authentication_uactt(SOPC_UserAuthentication_Manager* authn,
                                              const SOPC_ExtensionObject* token,
                                              SOPC_UserAuthentication_Status* authenticated)
{
    SOPC_UNUSED_ARG(authn);

    assert(NULL != token && NULL != authenticated);

    *authenticated = SOPC_USER_AUTHENTICATION_REJECTED_TOKEN;
    assert(SOPC_ExtObjBodyEncoding_Object == token->Encoding);
    if (&OpcUa_UserNameIdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        OpcUa_UserNameIdentityToken* userToken = token->Body.Object.Value;
        SOPC_String* username = &userToken->UserName;
        if (strcmp(SOPC_String_GetRawCString(username), "user1") == 0)
        {
            SOPC_ByteString* pwd = &userToken->Password;
            if (pwd->Length == strlen("password") && memcmp(pwd->Data, "password", strlen("password")) == 0)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
        }
        else if (strcmp(SOPC_String_GetRawCString(username), "user2") == 0)
        {
            SOPC_ByteString* pwd = &userToken->Password;
            if (pwd->Length == strlen("password1") && memcmp(pwd->Data, "password1", strlen("password1")) == 0)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
        }
        else if (strcmp(SOPC_String_GetRawCString(username), "username") == 0)
        {
            SOPC_ByteString* pwd = &userToken->Password;
            if (pwd->Length == strlen("password") && memcmp(pwd->Data, "password", strlen("password")) == 0)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_ACCESS_DENIED;
            }
        }
    }

    return SOPC_STATUS_OK;
}

static const SOPC_UserAuthentication_Functions authentication_uactt_functions = {
    .pFuncFree = (SOPC_UserAuthentication_Free_Func*) &SOPC_Free,
    .pFuncValidateUserIdentity = authentication_uactt};

static SOPC_ReturnStatus Server_SetDefaultUserManagementConfig(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Create an user authorization manager which accepts any user.
     * i.e.: UserAccessLevel right == AccessLevel right for any user for a given node of address space */
    SOPC_UserAuthorization_Manager* authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
    SOPC_UserAuthentication_Manager* authenticationManager = SOPC_Calloc(1, sizeof(SOPC_UserAuthentication_Manager));
    if (NULL == authenticationManager || NULL == authorizationManager)
    {
        SOPC_UserAuthorization_FreeManager(&authorizationManager);
        SOPC_UserAuthentication_FreeManager(&authenticationManager);
        status = SOPC_STATUS_OUT_OF_MEMORY;
        printf("<Test_Server_Toolkit: Failed to create the user manager\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Set a user authentication function that complies with UACTT tests expectations */
        authenticationManager->pFunctions = &authentication_uactt_functions;
        SOPC_HelperConfigServer_SetUserAuthenticationManager(authenticationManager);
        SOPC_HelperConfigServer_SetUserAuthorizationManager(authorizationManager);
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_LoadServerConfiguration(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    status = Server_SetDefaultConfiguration();

    if (SOPC_STATUS_OK == status)
    {
        status = Server_SetDefaultAddressSpace();
    }

    if (SOPC_STATUS_OK == status)
    {
        status = Server_SetDefaultUserManagementConfig();
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

void cbToolkit_test_server(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Get the toolkit build information and print it */
    SOPC_Toolkit_Build_Info build_info = SOPC_CommonHelper_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    status = Server_Initialize();

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
        status = SOPC_HelperConfigServer_SetWriteNotifCallback(Demo_WriteNotificationCallback);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "Failed to configure the @ space modification notification callback");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Demo_Server: Server started\n");

        /* Run the server until error  or stop server signal detected (Ctrl-C) */
        status = SOPC_ServerHelper_Serve(true);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed to run the server or end to serve with error = '%d'\n", status);
        }
        else
        {
            printf("<Test_Server_Toolkit: Server ended to serve successfully\n");
        }
    }
    else
    {
        printf("<Test_Server_Toolkit: Error during configuration phase, see logs for details.\n");
    }

    /* Clear the server library (stop all library threads) and server configuration */
    SOPC_HelperConfigServer_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Toolkit: Terminating with error status, see logs for details.\n");
    }
}
