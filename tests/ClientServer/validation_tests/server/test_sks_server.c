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
#include "sopc_sk_secu_group_managers.h"

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

#define SOPC_PKI_PATH "./S2OPC_Demo_PKI"

static char* default_server_cert = "server_public/server_2k_cert.der";
static char* default_key_cert = "server_private/encrypted_server_2k_key.pem";

// Default certificate paths for X509 Identity tokens

#define SOPC_USR_PKI_PATH "./S2OPC_Users_PKI"

#endif // WITH_STATIC_SECURITY_DATA

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"

static const char* default_locale_ids[] = {"en-US", "fr-FR"};

/* SKS Constants */
// Period to init the scheduler is 1s
#define SKS_SCHEDULER_INIT_MSPERIOD 1000
// Key Lifetime is 5s
#define SKS_KEYLIFETIME 5000
// Number of keys generated randomly
#define SKS_NB_GENERATED_KEYS 2
// Maximum number of Security Keys managed. When the number of keys exceed this limit, only the valid Keys are kept
#define SKS_NB_MAX_KEYS 20

/* SKS Data  */
SOPC_SKscheduler* skScheduler = NULL;

/*---------------------------------------------------------------------------
 *             PubSub Security Key Service specific configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_SKS_Start(uint32_t nbSkMgr, SOPC_SKManager** skMgrArr)
{
    if (nbSkMgr == 0 || NULL == skMgrArr)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

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

        if (NULL != skbAppend)
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
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_sks_server_logs/";
    }
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_Initialize();
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
    SOPC_CertificateList* static_cacert = NULL;
    SOPC_CRLList* static_crl = NULL;

    /* Load client/server certificates and server key from C source files (no filesystem needed) */
    status = SOPC_ServerConfigHelper_SetKeyCertPairFromBytes(sizeof(server_2k_cert), server_2k_cert,
                                                             sizeof(server_2k_key), server_2k_key);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(cacert, sizeof(cacert), &static_cacert);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(cacrl, sizeof(cacrl), &static_crl);
    }

    /* Create the PKI (Public Key Infrastructure) provider */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProvider_CreateFromList(static_cacert, static_crl, NULL, NULL, &pkiProvider);
    }
    SOPC_KeyManager_Certificate_Free(static_cacert);
    SOPC_KeyManager_CRL_Free(static_crl);
#else // WITH_STATIC_SECURITY_DATA == false
    /* Configure the callback */
    status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);

    /* Load client/server certificates and server key from files */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetKeyCertPairFromPath(default_server_cert, default_key_cert, true);
    }

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
    /* Only secuAdmin is authorized to be able to call the GetSecurityKeys method (restricted by role permissions)*/
    if (&OpcUa_UserNameIdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        OpcUa_UserNameIdentityToken* userToken = token->Body.Object.Value;
        SOPC_String* username = &userToken->UserName;
        if (strcmp(SOPC_String_GetRawCString(username), "secuAdmin") == 0)
        {
            SOPC_ByteString* pwd = &userToken->Password;
            if (pwd->Length == strlen("1234") && memcmp(pwd->Data, "1234", strlen("1234")) == 0)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
        }
    }

    if (&OpcUa_X509IdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        SOPC_ASSERT(NULL != authn);
        SOPC_ASSERT(NULL != authn->pUsrPKI);

        SOPC_PKIProvider* pkiProvider = authn->pUsrPKI;
        SOPC_PKI_Profile* pProfile = NULL;
        OpcUa_X509IdentityToken* x509Token = token->Body.Object.Value;
        SOPC_ByteString* rawCert = &x509Token->CertificateData;
        SOPC_CertificateList* pUserCert = NULL;
        SOPC_StatusCode errorStatus;

        status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(rawCert->Data, (uint32_t) rawCert->Length, &pUserCert);

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProvider_CreateMinimalUserProfile(&pProfile);
        }

        if (SOPC_STATUS_OK == status)
        {
            // Verify certificate through PKIProvider callback
            status = SOPC_PKIProvider_ValidateCertificate(pkiProvider, pUserCert, pProfile, &errorStatus, NULL);
            if (SOPC_STATUS_OK == status)
            {
                *authenticated = SOPC_USER_AUTHENTICATION_OK;
            }
            else
            {
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
        if (NULL != authentication->pUsrPKI)
        {
            SOPC_PKIProvider_Free(&authentication->pUsrPKI);
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
    SOPC_ASSERT(NULL != authorizationManager);
    SOPC_ASSERT(NULL != pbOperationAuthorized);

    *pbOperationAuthorized = false;

    bool read = true; // Authorize all users to read
    bool write = false;
    bool exec = false;

    if (SOPC_User_IsUsername(pUser))
    {
        // Authorize some users to execute methods
        const SOPC_String* username = SOPC_User_GetUsername(pUser);
        if (strcmp(SOPC_String_GetRawCString(username), "secuAdmin") == 0)
        {
            exec = true;
        }
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
        SOPC_ASSERT(false && "Unknown operation type.");
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
    SOPC_CertificateList* static_usr_cacert = NULL;
    SOPC_CRLList* static_usr_crl = NULL;

    status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(user_cacert, sizeof(user_cacert), &static_usr_cacert);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_CRL_CreateOrAddFromDER(user_cacrl, sizeof(user_cacrl), &static_usr_crl);
    }

    /* Create the PKI (Public Key Infrastructure) provider */
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_PKIProvider_CreateFromList(static_usr_cacert, static_usr_crl, NULL, NULL, &pX509_UserIdentity_PKI);
    }
    SOPC_KeyManager_Certificate_Free(static_usr_cacert);
    SOPC_KeyManager_CRL_Free(static_usr_crl);
#else
    status = SOPC_PKIProvider_CreateFromStore(SOPC_USR_PKI_PATH, &pX509_UserIdentity_PKI);
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
        /* Set a user authentication function that complies with UACTT tests expectations */
        authenticationManager->pFunctions = &sks_authentication_functions;
        authenticationManager->pData = (void*) NULL;
        authenticationManager->pUsrPKI = pX509_UserIdentity_PKI;
        authorizationManager->pFunctions = &sks_authorization_functions;
        SOPC_ServerConfigHelper_SetUserAuthenticationManager(authenticationManager);
        SOPC_ServerConfigHelper_SetUserAuthorizationManager(authorizationManager);
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

    SOPC_AddressSpace* addSpace = SOPC_Embedded_AddressSpace_LoadWithAlloc(true);
    status = (NULL != addSpace) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetAddressSpace(addSpace);
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

static SOPC_ReturnStatus Server_InitSKScallMethodService(void)
{
    SOPC_NodeId methodTypeId;
    SOPC_NodeId methodId;
    SOPC_NodeId_Initialize(&methodTypeId);
    SOPC_NodeId_Initialize(&methodId);
    methodTypeId.Data.Numeric = OpcUaId_PubSubKeyServiceType_GetSecurityKeys;
    methodId.Data.Numeric = OpcUaId_PublishSubscribe_GetSecurityKeys;
    /* Create and define the method call manager the server will use */
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
        status = SOPC_ServerConfigHelper_ConfigureFromXML(xml_server_config_path, xml_address_space_config_path,
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
        status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }

#endif

    /* Configuration of:
     * - Server endpoints configuration from XML server configuration file (comply with s2opc_clientserver_config.xsd) :
         - Endpoint URL,
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
        printf("<Test_SKS_Server: Server started\n");

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
        printf("<Test_SKS_Server: Terminating with error status, see logs in %s directory for details.\n", logDirPath);
    }

    // Free the string containing log path
    SOPC_Free(logDirPath);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
