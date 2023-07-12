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

#include <stdio.h>
#include <stdlib.h> /* getenv, exit */
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_common_constants.h"
#include "sopc_helper_askpass.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "embedded/sopc_addspace_loader.h"

#include "toolkit_demo_server_methods.h"

#ifdef WITH_STATIC_SECURITY_DATA
#include "server_static_security_data.h"
#else
// Default certificate paths

static char* default_server_cert = "server_public/server_2k_cert.der";
static char* default_key_cert = "server_private/encrypted_server_2k_key.pem";

#endif // WITH_STATIC_SECURITY_DATA

#define SOPC_PKI_PATH "./S2OPC_UACTT_PKI"
#define SOPC_PKI_USERS_PATH "./S2OPC_UACTT_Users_PKI"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI_2 "urn:S2OPC:localhost_2"

#define DEFAULT_CLIENT_REVERSE_ENDPOINT_URL "opc.tcp://localhost:4844"

/* Define application namespaces: ns=1 and ns=2 */
static const char* default_app_namespace_uris[] = {DEFAULT_PRODUCT_URI, DEFAULT_PRODUCT_URI_2};
static const char* default_locale_ids[] = {"en-US", "fr-FR"};

static const bool secuActive = true;

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
    SOPC_ASSERT(res);

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
        status = SOPC_ServerConfigHelper_Initialize();
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
 * Configure the applications authentication parameters of the endpoint:
 * - Server certificate and key
 * - Public Key Infrastructure: using a single certificate as Certificate Authority or Trusted Certificate
 */
static SOPC_ReturnStatus Server_SetDefaultAppsAuthConfig(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (secuActive)
    {
        SOPC_PKIProvider* pkiProvider = NULL;

#ifdef WITH_STATIC_SECURITY_DATA
        SOPC_CertificateList* ca_cert = NULL;
        SOPC_CRLList* crl = NULL;

        /* Load client/server certificates and server key from C source files (no filesystem needed) */
        status = SOPC_ServerConfigHelper_SetKeyCertPairFromBytes(sizeof(server_2k_cert), server_2k_cert,
                                                                 sizeof(server_2k_key), server_2k_key);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(cacert, sizeof(cacert), &ca_cert);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &crl);
        }

        /* Create the PKI (Public Key Infrastructure) provider */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProvider_CreateFromList(ca_cert, crl, NULL, NULL, &pkiProvider);
        }
        SOPC_KeyManager_Certificate_Free(ca_cert);
        SOPC_KeyManager_CRL_Free(crl);
#else // WITH_STATIC_SECURITY_DATA == false
        /* Configure the callback */
        SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
        /* Load client/server certificates and server key from files */
        status = SOPC_ServerConfigHelper_SetKeyCertPairFromPath(default_server_cert, default_key_cert, true);

        /* Create the PKI (Public Key Infrastructure) provider */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProvider_CreateFromStore(SOPC_PKI_PATH, &pkiProvider);
        }
#endif

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ServerConfigHelper_SetPKIprovider(pkiProvider);
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
    SOPC_ReturnStatus status = SOPC_ServerConfigHelper_SetNamespaces(sizeof(default_app_namespace_uris) / sizeof(char*),
                                                                     default_app_namespace_uris);
    // Set locale ids
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetLocaleIds(sizeof(default_locale_ids) / sizeof(char*), default_locale_ids);
    }

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
    if (SOPC_STATUS_OK == status && secuActive)
    {
        /*
         * 1st Security policy is Basic256Sha256 with anonymous and username authentication allowed
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
                status = SOPC_SecurityConfig_AddUserTokenPolicy(
                    sp, &SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_X509_DefaultSecurityPolicy);
            }
        }

        /*
         * 2nd Security policy is Basic256 with anonymous and username authentication allowed
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
                        sp, &SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy);
                }
                if (SOPC_STATUS_OK == status)
                {
                    status =
                        SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_X509_DefaultSecurityPolicy);
                }
            }
        }

        /*
         * 4th Security policy is Aes128-Sha256-RsaOaep with anonymous and username authentication
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
                        sp, &SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy);
                }
                if (SOPC_STATUS_OK == status)
                {
                    status =
                        SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_X509_DefaultSecurityPolicy);
                }
            }
        }

        /*
         * 5th Security policy is Aes256-Sha256-RsaPss with anonymous and username authentication
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
                        sp, &SOPC_UserTokenPolicy_UserName_Basic256Sha256SecurityPolicy);
                }
                if (SOPC_STATUS_OK == status)
                {
                    status =
                        SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_X509_DefaultSecurityPolicy);
                }
            }
        }
    }

    /*
     * 6th Security policy is None with anonymous and username authentication allowed
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
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_SecurityConfig_AddUserTokenPolicy(
                    sp, &SOPC_UserTokenPolicy_X509_Basic256Sha256SecurityPolicy); /* Necessary for UACTT tests only */
            }
        }
    }

    /**
     * Define server certificate and PKI provider
     */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SetDefaultAppsAuthConfig();
    }

    if (!SOPC_EndpointConfig_AddClientToConnect(ep, NULL, DEFAULT_CLIENT_REVERSE_ENDPOINT_URL))
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
    SOPC_ASSERT(NULL != token);
    SOPC_ASSERT(NULL != authenticated);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    SOPC_ASSERT(NULL != token && NULL != authenticated);

    *authenticated = SOPC_USER_AUTHENTICATION_REJECTED_TOKEN;
    SOPC_ASSERT(SOPC_ExtObjBodyEncoding_Object == token->Encoding);
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

    if (&OpcUa_X509IdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        SOPC_ASSERT(NULL != authn);
        SOPC_ASSERT(NULL != authn->pData);

        const SOPC_PKIProvider* pkiProvider = authn->pData;
        OpcUa_X509IdentityToken* x509Token = token->Body.Object.Value;
        SOPC_ByteString* rawCert = &x509Token->CertificateData;
        SOPC_CertificateList* pUserCert = NULL;
        SOPC_StatusCode errorStatus;

        SOPC_PKI_Profile* pProfile = NULL;
        status = SOPC_PKIProvider_CreateMinimalUserProfile(&pProfile);
        /* TODO:
            After the peer review, start the refactor and add SOPC_PKIProvider_CheckLeafCertificate in
            user_authentication_bs__is_valid_user_x509_authentication::is_cert_comply_with_security_policy
        */
        if (SOPC_STATUS_OK == status)
        {
            status =
                SOPC_KeyManager_Certificate_CreateOrAddFromDER(rawCert->Data, (uint32_t) rawCert->Length, &pUserCert);
        }

        if (SOPC_STATUS_OK == status)
        {
            // Verify certificate through PKIProvider callback
            status = SOPC_PKIProvider_ValidateCertificate(pkiProvider, pUserCert, pProfile, &errorStatus);
            if (SOPC_STATUS_OK == status)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
            else
            {
                /* UACTT expected BadIdentityTokenRejected */
                *authenticated = SOPC_USER_AUTHENTICATION_REJECTED_TOKEN;
                char* tpr = SOPC_KeyManager_Certificate_GetCstring_SHA1(pUserCert);
                if (NULL == tpr)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "authentication: Validation of User Certificate failed with error: %X",
                                           errorStatus);
                }
                else
                {
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "authentication: Validation of User Certificate with SHA-1 thumbprint %s failed with error: %X",
                        tpr, errorStatus);
                    SOPC_Free(tpr);
                }
            }
            /* The certificate validation failed but not the authentication function itself*/
            status = SOPC_STATUS_OK;
        }

        /* Clear */
        SOPC_KeyManager_Certificate_Free(pUserCert);
        SOPC_PKIProvider_DeleteProfile(&pProfile);
    }

    return status;
}

static void UserAuthentication_Free(SOPC_UserAuthentication_Manager* authentication)
{
    if (NULL != authentication)
    {
        if (NULL != authentication->pData)
        {
            SOPC_PKIProvider_Free((SOPC_PKIProvider**) &authentication->pData);
        }
        SOPC_Free(authentication);
    }
}

static const SOPC_UserAuthentication_Functions authentication_uactt_functions = {
    .pFuncFree = (SOPC_UserAuthentication_Free_Func*) &UserAuthentication_Free,
    .pFuncValidateUserIdentity = authentication_uactt};

static SOPC_ReturnStatus Server_SetDefaultUserManagementConfig(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_PKIProvider* pX509_UserIdentity_PKI = NULL;
    SOPC_UserAuthorization_Manager* authorizationManager = NULL;
    SOPC_UserAuthentication_Manager* authenticationManager = NULL;

    /* Create an user authorization manager which allow all rights to any user.
     * i.e.: UserAccessLevel right == AccessLevel right for any user for a given node of address space */
    authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
    if (NULL == authorizationManager)
    {
        printf("<Test_Server_Toolkit: Failed to create the user authorization manager\n");
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

#ifdef WITH_STATIC_SECURITY_DATA
    SOPC_CertificateList* userCAcert = NULL;
    SOPC_CRLList* userCAcrl = NULL;

    status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(user_cacert, sizeof(user_cacert), &userCAcert);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(user_cacrl, sizeof(user_cacrl), &userCAcrl);
    }

    /* Create the PKI (Public Key Infrastructure) provider */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromList(userCAcert, userCAcrl, NULL, NULL, &pX509_UserIdentity_PKI);
    }
    SOPC_KeyManager_Certificate_Free(userCAcert);
    SOPC_KeyManager_CRL_Free(userCAcrl);
#else
    status = SOPC_PKIProvider_CreateFromStore(SOPC_PKI_USERS_PATH, &pX509_UserIdentity_PKI);

#endif

    if (SOPC_STATUS_OK == status)
    {
        authenticationManager = SOPC_Calloc(1, sizeof(SOPC_UserAuthentication_Manager));
        status = NULL == authenticationManager ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Set a user authentication function that complies with UACTT tests expectations */
        authenticationManager->pFunctions = &authentication_uactt_functions;
        authenticationManager->pData = pX509_UserIdentity_PKI;
        SOPC_ServerConfigHelper_SetUserAuthenticationManager(authenticationManager);
        SOPC_ServerConfigHelper_SetUserAuthorizationManager(authorizationManager);
    }
    else
    {
        /* clear */
        SOPC_PKIProvider_Free(&pX509_UserIdentity_PKI);
        SOPC_UserAuthorization_FreeManager(&authorizationManager);
        printf("<Test_Server_Toolkit: Failed to create the user authentication manager: %d\n", status);
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
        status = SOPC_ServerConfigHelper_SetAddressSpace(addSpace);
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
    SOPC_ReturnStatus status = (NULL != mcm) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetMethodCallManager(mcm);
    }

    /* Add methods implementation in the method call manager used */
    if (SOPC_STATUS_OK == status)
    {
        /* No input, no output */
        sNodeId = "ns=1;s=MethodNoArg";
        methodId = SOPC_NodeId_FromCString(sNodeId, (int32_t) strlen(sNodeId));
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_IncCounter;
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
            methodFunc = &SOPC_Method_Func_AddToCounter;
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
            methodFunc = &SOPC_Method_Func_GetCounterValue;
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
            methodFunc = &SOPC_Method_Func_UpdateAndGetPreviousHello;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "Input, output", NULL);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        sNodeId = "ns=1;s=AddVariableMethod";
        methodId = SOPC_NodeId_FromCString(sNodeId, (int32_t) strlen(sNodeId));
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_AddVariable;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "AddVariable", NULL);
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

static SOPC_ReturnStatus Server_SetMatrixVariablesProperties(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_STATE;
    SOPC_AddressSpace* addSpace = SOPC_ServerConfigHelper_GetAddressSpace();
    if (NULL == addSpace)
    {
        return status;
    }
    bool found = false;
    const char* boolMatrixStr = "ns=1;s=Bool_Matrix";
    SOPC_NodeId* boolMatrixId = SOPC_NodeId_FromCString(boolMatrixStr, (int32_t) strlen(boolMatrixStr));
    SOPC_AddressSpace_Node* boolMatrix = SOPC_AddressSpace_Get_Node(addSpace, boolMatrixId, &found);
    SOPC_NodeId_Clear(boolMatrixId);
    SOPC_Free(boolMatrixId);
    if (found)
    {
        SOPC_Variant* boolMatrixVar = SOPC_AddressSpace_Get_Value(addSpace, boolMatrix);
        SOPC_ASSERT(NULL != boolMatrixVar);
        SOPC_Variant_Clear(boolMatrixVar);
        boolMatrixVar->BuiltInTypeId = SOPC_Boolean_Id;
        boolMatrixVar->ArrayType = SOPC_VariantArrayType_Matrix;
        boolMatrixVar->Value.Matrix.ArrayDimensions =
            SOPC_Calloc(2, sizeof(*boolMatrixVar->Value.Matrix.ArrayDimensions));
        if (NULL == boolMatrixVar->Value.Matrix.ArrayDimensions)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            boolMatrixVar->Value.Matrix.Dimensions = 2;
            boolMatrixVar->Value.Matrix.ArrayDimensions[0] = 5;
            boolMatrixVar->Value.Matrix.ArrayDimensions[1] = 10;
            size_t arrayLength = (size_t)(boolMatrixVar->Value.Matrix.ArrayDimensions[0] *
                                          boolMatrixVar->Value.Matrix.ArrayDimensions[1]);
            boolMatrixVar->Value.Matrix.Content.BooleanArr =
                SOPC_Calloc(arrayLength, sizeof(*boolMatrixVar->Value.Matrix.Content.BooleanArr));
            if (NULL == boolMatrixVar->Value.Matrix.Content.BooleanArr)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else if (!SOPC_AddressSpace_AreReadOnlyNodes(addSpace))
            {
                bool res = SOPC_AddressSpace_Set_StatusCode(addSpace, boolMatrix, SOPC_GoodGenericStatus);
                status = res ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
            }
            else
            {
                status = SOPC_STATUS_OK;
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Variant_Clear(boolMatrixVar);
        }
    }

    const char* byteMatrixIdStr = "ns=1;s=Byte_Matrix3D";
    SOPC_NodeId* byteMatrixId = SOPC_NodeId_FromCString(byteMatrixIdStr, (int32_t) strlen(byteMatrixIdStr));
    SOPC_AddressSpace_Node* byteMatrix = SOPC_AddressSpace_Get_Node(addSpace, byteMatrixId, &found);
    SOPC_NodeId_Clear(byteMatrixId);
    SOPC_Free(byteMatrixId);
    if (found)
    {
        SOPC_Variant* byteMatrixVar = SOPC_AddressSpace_Get_Value(addSpace, byteMatrix);
        SOPC_ASSERT(NULL != byteMatrixVar);
        SOPC_Variant_Clear(byteMatrixVar);
        byteMatrixVar->BuiltInTypeId = SOPC_Byte_Id;
        byteMatrixVar->ArrayType = SOPC_VariantArrayType_Matrix;
        byteMatrixVar->Value.Matrix.ArrayDimensions =
            SOPC_Calloc(3, sizeof(*byteMatrixVar->Value.Matrix.ArrayDimensions));
        if (NULL == byteMatrixVar->Value.Matrix.ArrayDimensions)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            byteMatrixVar->Value.Matrix.Dimensions = 3;
            byteMatrixVar->Value.Matrix.ArrayDimensions[0] = 5;
            byteMatrixVar->Value.Matrix.ArrayDimensions[1] = 7;
            byteMatrixVar->Value.Matrix.ArrayDimensions[2] = 9;
            size_t arrayLength = (size_t)(byteMatrixVar->Value.Matrix.ArrayDimensions[0] *
                                          byteMatrixVar->Value.Matrix.ArrayDimensions[1] *
                                          byteMatrixVar->Value.Matrix.ArrayDimensions[2]);
            byteMatrixVar->Value.Matrix.Content.ByteArr =
                SOPC_Calloc(arrayLength, sizeof(*byteMatrixVar->Value.Matrix.Content.ByteArr));
            if (NULL == byteMatrixVar->Value.Matrix.Content.ByteArr)
            {
                status = SOPC_STATUS_OUT_OF_MEMORY;
            }
            else if (!SOPC_AddressSpace_AreReadOnlyNodes(addSpace))
            {
                bool res = SOPC_AddressSpace_Set_StatusCode(addSpace, byteMatrix, SOPC_GoodGenericStatus);
                status = res ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
            }
            else
            {
                status = SOPC_STATUS_OK;
            }
        }
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Variant_Clear(byteMatrixVar);
        }
    }

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

#ifndef WITH_STATIC_SECURITY_DATA
    // Define a callback to retrieve the server key password (from environment variable)
    status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);

    xml_server_config_path = getenv("TEST_SERVER_XML_CONFIG");
    xml_address_space_config_path = getenv("TEST_SERVER_XML_ADDRESS_SPACE");
    xml_users_config_path = getenv("TEST_USERS_XML_CONFIG");
#endif

    if (SOPC_STATUS_OK == status &&
        (NULL != xml_server_config_path || NULL != xml_address_space_config_path || NULL != xml_users_config_path))
    {
#ifdef WITH_EXPAT
        status = SOPC_ServerConfigHelper_ConfigureFromXML(xml_server_config_path, xml_address_space_config_path,
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

    if (SOPC_STATUS_OK == status)
    {
        // Set multi-dimensional properties (not available yet in parsers)
        status = Server_SetMatrixVariablesProperties();
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
        status = SOPC_ServerConfigHelper_SetWriteNotifCallback(Demo_WriteNotificationCallback);
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
    SOPC_ServerConfigHelper_Clear();
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
