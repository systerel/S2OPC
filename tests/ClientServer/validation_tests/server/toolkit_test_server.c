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
#include <stdlib.h> /* getenv, exit */
#include <string.h>

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

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#else
// Default certificate paths

static char* default_server_cert = "server_public/server_2k_cert.der";
static char* default_key_cert = "server_private/server_2k_key.pem";

static char* default_trusted_root_issuers[] = {
    "trusted/cacert.der",    /* Demo CA */
    "trusted/ctt_ca1TC.der", /* Will be ignored because no CRL associated. Tests 042, 043 */
    "trusted/ctt_ca1T.der",  /* Tests 029, 037 */
    NULL};
static char* default_trusted_intermediate_issuers[] = {"trusted/ctt_ca1I_ca2T.der", NULL};

static char* default_issued_certs[] = {"issued/ctt_appT.der",  /* Test 048 */
                                       "issued/ctt_appTE.der", /* Test 007 */
                                       "issued/ctt_appTSha1_1024.der",
                                       "issued/ctt_appTSha1_2048.der",
                                       "issued/ctt_appTSha256_2048.der", /* Test 051 */
                                       "issued/ctt_appTSha256_4096.der", /* Test 052 still fails */
                                       "issued/ctt_appTSincorrect.der",  /* Test 010 */
                                       "issued/ctt_appTSip.der",
                                       "issued/ctt_appTSuri.der",
                                       "issued/ctt_appTV.der",     /* Test 008 */
                                       "issued/ctt_ca1I_appT.der", /* Test 044 */
                                       "issued/ctt_ca1I_appTR.der",
                                       "issued/ctt_ca1I_ca2T_appT.der",
                                       "issued/ctt_ca1I_ca2T_appTR.der",
                                       "issued/ctt_ca1IC_appT.der",
                                       "issued/ctt_ca1IC_appTR.der",
                                       "issued/ctt_ca1T_appT.der",
                                       "issued/ctt_ca1T_appTR.der",
                                       "issued/ctt_ca1T_ca2U_appT.der",
                                       "issued/ctt_ca1T_ca2U_appTR.der",
                                       "issued/ctt_ca1TC_appT.der",
                                       "issued/ctt_ca1TC_appTR.der",
                                       "issued/ctt_ca1TC_ca2I_appT.der", /* Test 002 */
                                       "issued/ctt_ca1TC_ca2I_appTR.der",
                                       "issued/ctt_ca1U_appT.der", /* Test 046 */
                                       "issued/ctt_ca1U_appTR.der",
                                       NULL};
static char* default_untrusted_root_issuers[] = {
    "untrusted/ctt_ca1IC.der", /* Will be ignored because no CRL associated */
    "untrusted/ctt_ca1I.der",  /* Test 044 */
    NULL};
static char* default_untrusted_intermediate_issuers[] = {"untrusted/ctt_ca1TC_ca2I.der", /* Test 002 */
                                                         NULL};
static char* default_revoked_certs[] = {"revoked/cacrl.der",          "revoked/ctt_ca1T.crl",
                                        "revoked/ctt_ca1I.crl",       "revoked/ctt_ca1I_ca2T.crl",
                                        "revoked/ctt_ca1TC_ca2I.crl", NULL};

#endif // WITH_STATIC_SECURITY_DATA

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI_2 "urn:S2OPC:localhost_2"

/* Define application namespaces: ns=1 and ns=2 (NULL terminated array) */
static char* default_app_namespace_uris[] = {DEFAULT_PRODUCT_URI, DEFAULT_PRODUCT_URI_2};
static char* default_locale_ids[] = {"en-US", "fr-FR"};

static const bool secuActive = true;

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
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);
    SOPC_UNUSED_ARG(param);
    *nbOutputArgs = 0;
    *outputArgs = NULL;
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
 *                          Server initialization
 *---------------------------------------------------------------------------*/

/* Set the log path and create (or keep existing) directory path built on executable path
 *  + first argument of main */
static char* Server_ConfigLogPath(int argc, char* argv[])
{
    const char* logDirName = "toolkit_test_server";
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

static SOPC_ReturnStatus Server_Initialize(const char* logDirPath)
{
    // Due to issue in certification tool for View Basic 005/015/020 number of chunks shall be the same and at least 12
    SOPC_Common_EncodingConstants encConf = SOPC_Common_GetDefaultEncodingConstants();
    encConf.receive_max_nb_chunks = 12;
    encConf.send_max_nb_chunks = 12;
    bool res = SOPC_Common_SetEncodingConstants(encConf);
    assert(res);

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

#ifdef WITH_STATIC_SECURITY_DATA
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
#else // WITH_STATIC_SECURITY_DATA == false
        /* Load client/server certificates and server key from files */
        status = SOPC_HelperConfigServer_SetKeyCertPairFromPath(default_server_cert, default_key_cert);

        /* Create the PKI (Public Key Infrastructure) provider */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_CreateFromPaths(
                default_trusted_root_issuers, default_trusted_intermediate_issuers, default_untrusted_root_issuers,
                default_untrusted_intermediate_issuers, default_issued_certs, default_revoked_certs, &pkiProvider);
        }
#endif

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

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_LoadServerConfiguration(void)
{
    /* Retrieve XML configuration file path from environment variables TEST_SERVER_XML_CONFIG,
     * TEST_SERVER_XML_ADDRESS_SPACE and TEST_USERS_XML_CONFIG.
     *
     * In case of success returns the file path otherwise load default configuration.
     */

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    const char* xml_server_config_path = getenv("TEST_SERVER_XML_CONFIG");
    const char* xml_address_space_config_path = getenv("TEST_SERVER_XML_ADDRESS_SPACE");
    const char* xml_users_config_path = getenv("TEST_USERS_XML_CONFIG");

    if (NULL != xml_server_config_path || NULL != xml_address_space_config_path || NULL != xml_users_config_path)
    {
#ifdef WITH_EXPAT
        status = SOPC_HelperConfigServer_ConfigureFromXML(xml_server_config_path, xml_address_space_config_path,
                                                          xml_users_config_path, NULL);
#else
        printf(
            "Error: an XML server configuration file path provided whereas XML library not available (Expat).\n"
            "Do not define environment variables TEST_SERVER_XML_CONFIG, TEST_SERVER_XML_ADDRESS_SPACE and "
            "TEST_USERS_XML_CONFIG.\n"
            "Or compile with XML library available.\n");
        status = SOPC_STATUS_INVALID_PARAMETERS;
#endif
    }

    if (SOPC_STATUS_OK == status && NULL == xml_server_config_path)
    {
        status = Server_SetDefaultConfiguration();
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

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
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

    /* Configure the server logger:
     * DEBUG traces generated in ./toolkit_server_<argv[1]>_logs/ */
    char* logDirPath = Server_ConfigLogPath(argc, argv);

    /* Initialize the server library (start library threads) */
    status = Server_Initialize(logDirPath);

    /* Configuration of:
     * - Server endpoints configuration from XML server configuration file (comply with s2opc_clientserver_config.xsd) :
         - Enpoint URL,
         - Security endpoint properties,
         - Cryptographic parameters,
         - Application description
       - Server address space initial content from XML configuration file (comply with UANodeSet.xsd)
       - User authentication and authorization management from XML configuration file
         (comply with s2opc_clientserver_users_config.xsd)
    */
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
        printf("<Test_Server_Toolkit: Error during configuration phase, see logs in %s directory for details.\n",
               logDirPath);
    }

    /* Clear the server library (stop all library threads) and server configuration */
    SOPC_HelperConfigServer_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Toolkit: Terminating with error status, see logs in %s directory for details.\n",
               logDirPath);
    }

    // Free the string containing log path
    SOPC_Free(logDirPath);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
