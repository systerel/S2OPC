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
#include "sopc_sk_builder.h"
#include "sopc_sk_manager.h"
#include "sopc_sk_provider.h"
#include "sopc_sk_scheduler.h"

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "embedded/sopc_addspace_loader.h"

#include "sks_demo_server_methods.h"

#ifdef WITH_STATIC_SECURITY_DATA
#include "server_static_security_data.h"
#else
// Default certificate paths

static char* default_server_cert = "server_public/server_2k_cert.der";
static char* default_key_cert = "server_private/encrypted_server_2k_key.pem";

static char* default_trusted_root_issuers[] = {"trusted/cacert.der", NULL};
static char* default_trusted_intermediate_issuers[] = {NULL};

static char* default_issued_certs[] = {NULL};
static char* default_untrusted_root_issuers[] = {NULL};
static char* default_untrusted_intermediate_issuers[] = {NULL};
static char* default_revoked_certs[] = {"revoked/cacrl.der", NULL};

// Default certificate paths for X509 Identity tokens

static char* x509_Identity_trusted_root_issuers[] = {"trusted_usr/user_cacert.der", /* Demo CA */ NULL};

static char* x509_Identity_trusted_intermediate_issuers[] = {NULL};

static char* x509_Identity_issued_certs[] = {NULL};
static char* x509_Identity_untrusted_root_issuers[] = {NULL};
static char* x509_Identity_untrusted_intermediate_issuers[] = {NULL};
static char* x509_Identity_revoked_certs[] = {"revoked_usr/user_cacrl.der", NULL};

#endif // WITH_STATIC_SECURITY_DATA

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"

/* Define application namespaces: ns=1 and ns=2 */
static const char* default_app_namespace_uris[] = {DEFAULT_PRODUCT_URI};
static const char* default_locale_ids[] = {"en-US", "fr-FR"};

/* SKS Constants */
// Period to init the scheduler is 1s
#define SKS_SCHEDULER_INIT_MSPERIOD 1000
// Key Lifetime is 10s
#define SKS_KEYLIFETIME 100000
// Number of keys generated randomly
#define SKS_NB_GENERATED_KEYS 5
// Maximum number of Security Keys managed. When the number of keys exceed this limit, only the valid Keys are kept
#define SKS_NB_MAX_KEYS 20

#define SKS_SECURITY_GROUPID "sgid_1"

/* SKS Data  */
SOPC_SKManager* skManager = NULL;
SOPC_SKscheduler* skScheduler = NULL;

/*---------------------------------------------------------------------------
 *             PubSub Security Key Service specific configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_SKS_Start(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Set KeyLifeTime only if SK Manager didn't get Keys from Slave
    if (0 == SOPC_SKManager_Size(skManager))
    {
        status = SOPC_SKManager_SetKeyLifetime(skManager, SKS_KEYLIFETIME);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_String policy;
            SOPC_String_Initialize(&policy);
            SOPC_String_CopyFromCString(&policy, SOPC_SecurityPolicy_PubSub_Aes256_URI);
            status = SOPC_SKManager_SetSecurityPolicyUri(skManager, &policy);
            SOPC_String_Clear(&policy);
        }
    }

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

    if (SOPC_STATUS_OK == status)
    {
        /* Init the task with 1s */
        status = SOPC_SKscheduler_AddTask(skScheduler, skBuilder, skProvider, skManager, SKS_SCHEDULER_INIT_MSPERIOD);
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
        printf("<Security Keys Service Error: Start failed\n");
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                      OPC UA Methods callback implementation
 *---------------------------------------------------------------------------*/

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
        status = SOPC_HelperConfigServer_Initialize();
    }
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_SKS_Server: Failed initializing\n");
    }
    else
    {
        printf("<Test_SKS_Server: initialized\n");
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
    /* Configure the callback */
    status = SOPC_HelperConfigServer_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);

    /* Load client/server certificates and server key from files */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetKeyCertPairFromPath(default_server_cert, default_key_cert, true);
    }

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
        printf("<Test_SKS_Server: Failed loading certificates and key (check paths are valid)\n");
    }
    else
    {
        printf("<Test_SKS_Server: Certificates and key loaded\n");
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
    if (SOPC_STATUS_OK == status)
    {
        /*
         * Security policy is Basic256Sha256 with anonymous and username authentication allowed
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
    }

    /**
     * Define server certificate and PKI provider
     */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SetDefaultAppsAuthConfig();
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
 * Then it shall accept username:password, but return "access denied".
 * Otherwise it shall be "identity token rejected".
 */
static SOPC_ReturnStatus authentication_test_sks(SOPC_UserAuthentication_Manager* authn,
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

        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(rawCert->Data, (uint32_t) rawCert->Length, &pUserCert);

        if (SOPC_STATUS_OK == status)
        {
            // Verify certificate through PKIProvider callback
            status = pkiProvider->pFnValidateCertificate(pkiProvider, pUserCert, &errorStatus);
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

static SOPC_ReturnStatus authorization_test_sks(SOPC_UserAuthorization_Manager* authorizationManager,
                                                SOPC_UserAuthorization_OperationType operationType,
                                                const SOPC_NodeId* nodeId,
                                                uint32_t attributeId,
                                                const SOPC_User* pUser,
                                                bool* pbOperationAuthorized)
{
    // We use global user rights only and do not check user rights for a specific node
    (void) (nodeId);
    (void) (attributeId);
    assert(NULL != authorizationManager);
    assert(NULL != pbOperationAuthorized);

    *pbOperationAuthorized = false;

    bool read = true; // Authorize
    bool write = false;
    bool exec = false;

    if (SOPC_User_IsCertificate(pUser))
    {
        read = true;
        exec = true;
    }
    else if (SOPC_User_IsUsername(pUser))
    {
        // Authorize some users to execute methods
        const SOPC_String* username = SOPC_User_GetUsername(pUser);
        if (strcmp(SOPC_String_GetRawCString(username), "user1") == 0)
        {
            read = true;
            exec = true;
        }
    }
    else if (SOPC_User_IsAnonymous(pUser))
    {
        // Anonymous has read only rights
        read = true;
    }

    switch (operationType)
    {
    case SOPC_USER_AUTHORIZATION_OPERATION_READ:
        *pbOperationAuthorized = read;
        break;
    case SOPC_USER_AUTHORIZATION_OPERATION_WRITE:
        *pbOperationAuthorized = write;
        break;
    case SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE:
        *pbOperationAuthorized = exec;
        break;
    default:
        assert(false && "Unknown operation type.");
        break;
    }

    return SOPC_STATUS_OK;
}

static const SOPC_UserAuthentication_Functions sks_authentication_functions = {
    .pFuncFree = UserAuthentication_Free,
    .pFuncValidateUserIdentity = authentication_test_sks};

static void UserAuthorization_Free(SOPC_UserAuthorization_Manager* authorization)
{
    SOPC_Free(authorization);
}

static const SOPC_UserAuthorization_Functions sks_authorization_functions = {
    .pFuncFree = UserAuthorization_Free,
    .pFuncAuthorizeOperation = authorization_test_sks};

static SOPC_ReturnStatus Server_SetDefaultUserManagementConfig(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_PKIProvider* pX509_UserIdentity_PKI = NULL;
    SOPC_UserAuthorization_Manager* authorizationManager = NULL;
    SOPC_UserAuthentication_Manager* authenticationManager = NULL;

#ifdef WITH_STATIC_SECURITY_DATA
    SOPC_SerializedCertificate* serializedUserCAcert = NULL;
    SOPC_CRLList* serializedUserCAcrl = NULL;

    status =
        SOPC_KeyManager_SerializedCertificate_CreateFromDER(user_cacert, sizeof(user_cacert), &serializedUserCAcert);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(user_cacrl, sizeof(user_cacrl), &serializedUserCAcrl);
    }

    /* Create the PKI (Public Key Infrastructure) provider */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProviderStack_Create(serializedUserCAcert, serializedUserCAcrl, &pX509_UserIdentity_PKI);
    }
    SOPC_KeyManager_SerializedCertificate_Delete(serializedUserCAcert);
#else
    status = SOPC_PKIProviderStack_CreateFromPaths(
        x509_Identity_trusted_root_issuers, x509_Identity_trusted_intermediate_issuers,
        x509_Identity_untrusted_root_issuers, x509_Identity_untrusted_intermediate_issuers, x509_Identity_issued_certs,
        x509_Identity_revoked_certs, &pX509_UserIdentity_PKI);
#endif

    if (SOPC_STATUS_OK == status)
    {
        authenticationManager = SOPC_Calloc(1, sizeof(SOPC_UserAuthentication_Manager));
        authorizationManager = SOPC_Calloc(1, sizeof(SOPC_UserAuthorization_Manager));

        if (NULL == authenticationManager || NULL == authorizationManager)
        {
            SOPC_Free(authenticationManager);
            SOPC_Free(authorizationManager);
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Set PKI for user */
        SOPC_PKIProviderStack_SetUserCert(pX509_UserIdentity_PKI, true);
        /* Set a user authentication function that complies with UACTT tests expectations */
        authenticationManager->pFunctions = &sks_authentication_functions;
        authenticationManager->pData = pX509_UserIdentity_PKI;
        authorizationManager->pFunctions = &sks_authorization_functions;
        SOPC_HelperConfigServer_SetUserAuthenticationManager(authenticationManager);
        SOPC_HelperConfigServer_SetUserAuthorizationManager(authorizationManager);
    }
    else
    {
        /* clear */
        SOPC_PKIProvider_Free(&pX509_UserIdentity_PKI);
        SOPC_UserAuthorization_FreeManager(&authorizationManager);
        printf("<Test_SKS_Server: Failed to create the user authentication manager: %d\n", status);
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
        printf("<Test_SKS_Server: Failed to configure the @ space\n");
    }
    else
    {
        printf("<Test_SKS_Server: @ space configured\n");
    }

    return status;
}

/*-------------------------
 * Method call management :
 *-------------------------*/

static SOPC_ReturnStatus Server_InitSKScallMethodService(SOPC_SKManager* skm)
{
    SOPC_NodeId* methodId;
    SOPC_MethodCallFunc_Ptr* methodFunc;
    /* Create and define the method call manager the server will use*/
    SOPC_MethodCallManager* mcm = SOPC_MethodCallManager_Create();
    SOPC_ReturnStatus status = (NULL != mcm) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetMethodCallManager(mcm);
    }

    /* Add methods implementation in the method call manager used */
    if (SOPC_STATUS_OK == status)
    {
        // getSecurityKeys method node
        methodId = SOPC_Calloc(1, sizeof(*methodId));
        if (NULL != methodId)
        {
            methodId->Data.Numeric = OpcUaId_PublishSubscribe_GetSecurityKeys;
            methodFunc = &SOPC_Method_Func_PublishSubscribe_GetSecurityKeys;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, skm, NULL);
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

    const char* xml_server_config_path = getenv("SKS_SERVER_XML_CONFIG");
    const char* xml_address_space_config_path = getenv("SKS_SERVER_XML_ADDRESS_SPACE");
    const char* xml_users_config_path = getenv("SKS_USERS_XML_CONFIG");

    if (NULL != xml_server_config_path || NULL != xml_address_space_config_path || NULL != xml_users_config_path)
    {
#ifdef WITH_EXPAT
        status = SOPC_HelperConfigServer_ConfigureFromXML(xml_server_config_path, xml_address_space_config_path,
                                                          xml_users_config_path, NULL);
#else
        printf(
            "Error: an XML server configuration file path provided whereas XML library not available (Expat).\n"
            "Do not define environment variables SKS_SERVER_XML_CONFIG, SKS_SERVER_XML_ADDRESS_SPACE and "
            "SKS_USERS_XML_CONFIG.\n"
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
    SOPC_UNUSED_ARG(argc);

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
    char* logDirPath = Server_ConfigLogPath(argv[0]);

    /* Initialize the server library (start library threads) */
    status = Server_Initialize(logDirPath);

#ifdef WITH_EXPAT

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }

#endif

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

    /* Init SK Manager */
    if (SOPC_STATUS_OK == status)
    {
        skManager = SOPC_SKManager_Create();
        if (NULL == skManager)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    // Define sks implementation of functions called for method call service
    if (SOPC_STATUS_OK == status)
    {
        status = Server_InitSKScallMethodService(skManager);
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

    /* Start SKS Scheduler */
    if (SOPC_STATUS_OK == status)
    {
        status = Server_SKS_Start();
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Demo_Server: Server started\n");

        /* Run the server until error  or stop server signal detected (Ctrl-C) */
        status = SOPC_ServerHelper_Serve(true);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_SKS_Server: Failed to run the server or end to serve with error = '%d'\n", status);
        }
        else
        {
            printf("<Test_SKS_Server: Server ended to serve successfully\n");
        }
    }
    else
    {
        printf("<Test_SKS_Server: Error during configuration phase, see logs in %s directory for details.\n",
               logDirPath);
    }

    /* Stop and clear SKS related modules */
    SOPC_SKscheduler_StopAndClear(skScheduler);
    SOPC_Free(skScheduler);
    SOPC_SKManager_Clear(skManager);
    SOPC_Free(skManager);

    /* Clear the server library (stop all library threads) and server configuration */
    SOPC_HelperConfigServer_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_SKS_Server: Terminating with error status, see logs in %s directory for details.\n", logDirPath);
    }

    // Free the string containing log path
    SOPC_Free(logDirPath);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
