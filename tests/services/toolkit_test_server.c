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
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h> /* getenv, exit */
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_encodeable.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"

#include "embedded/loader.h"
#include "runtime_variables.h"

#ifdef WITH_EXPAT
#include "uanodeset_expat/loader.h"
#endif

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#endif

#define ENDPOINT_URL "opc.tcp://localhost:4841"
#define APPLICATION_URI "urn:S2OPC:localhost"
#define PRODUCT_URI "urn:S2OPC:localhost"
#define PRODUCT_URI_2 "urn:S2OPC:localhost_2"

/* Define application namespaces: ns=1 and ns=2 (NULL terminated array) */
static const char* app_namespace_uris[] = {(const char*) PRODUCT_URI, PRODUCT_URI_2, NULL};

static int32_t endpointClosed = 0;
static bool secuActive = true;

volatile sig_atomic_t stopServer = 0;

typedef enum
{
    AS_LOADER_EMBEDDED,
#ifdef WITH_EXPAT
    AS_LOADER_EXPAT,
#endif
} AddressSpaceLoader;

/*---------------------------------------------------------------------------
 *                          Callbacks definition
 *---------------------------------------------------------------------------*/

/*
 * Management of Ctrl-C to stop the server (callback on stop signal)
 */
static void Test_StopSignal(int sig)
{
    /* avoid unused parameter compiler warning */
    (void) sig;

    if (stopServer != 0)
    {
        exit(1);
    }
    else
    {
        stopServer = 1;
    }
}

/*
 * Server callback definition used for endpoint communication events
 * (endpoint closed, local service response)
 */
static void Test_ComEvent_FctServer(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    /* avoid unused parameter compiler warning */
    (void) idOrStatus;
    (void) appContext;

    if (event == SE_CLOSED_ENDPOINT)
    {
        printf("<Test_Server_Toolkit: closed endpoint event: OK\n");
        SOPC_Atomic_Int_Set(&endpointClosed, 1);
    }
    else if (event == SE_LOCAL_SERVICE_RESPONSE)
    {
        SOPC_EncodeableType* message_type = *((SOPC_EncodeableType**) param);

        if (message_type != &OpcUa_WriteResponse_EncodeableType)
        {
            return;
        }

        OpcUa_WriteResponse* write_response = param;

        bool ok = (write_response->ResponseHeader.ServiceResult == SOPC_GoodGenericStatus);

        for (int32_t i = 0; i < write_response->NoOfResults; ++i)
        {
            ok &= (write_response->Results[i] == SOPC_GoodGenericStatus);
        }

        if (!ok)
        {
            printf("<Test_Server_Toolkit: Error while updating address space\n");
        }

        return;
    }

    else
    {
        printf("<Test_Server_Toolkit: unexpected endpoint event %d : NOK\n", event);
    }
}

/*
 * Server callback definition used for address space modification notification
 */
static void Test_AddressSpaceNotif_Fct(SOPC_App_AddSpace_Event event, void* opParam, SOPC_StatusCode opStatus)
{
    /* avoid unused parameter compiler warning */
    (void) opParam;
    (void) opStatus;

    if (event != AS_WRITE_EVENT)
    {
        printf("<Test_Server_Toolkit: unexpected address space event %d : NOK\n", event);
    }
}

/*---------------------------------------------------------------------------
 *                          Server initialization
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_Initialize(void)
{
    // Initialize the toolkit library and define the communication events callback
    SOPC_ReturnStatus status = SOPC_Toolkit_Initialize(Test_ComEvent_FctServer);
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

/*---------------------------------
 * Endpoint security configuration:
 *---------------------------------*/

/*
 * Create an empty endpoint configuration with given URL
 */
static SOPC_Endpoint_Config Server_CreateEnpointConfig(char* endpointURL)
{
    SOPC_Endpoint_Config epConfig;
    memset(&epConfig, 0, sizeof(epConfig));
    // Init unique endpoint structure
    epConfig.endpointURL = endpointURL;

    return epConfig;
}

/*
 * Define the security policies, security modes and user token policies supported by endpoint
 */
static SOPC_ReturnStatus Server_SetEnpointSecurityConfig(SOPC_Endpoint_Config* pEpConfig)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (secuActive)
    {
        /*
         * 1st Security policy is Basic256Sha256 with anonymous and username (non encrypted) authentication allowed
         */
        SOPC_String_Initialize(&pEpConfig->secuConfigurations[0].securityPolicy);
        status = SOPC_String_AttachFromCstring(&pEpConfig->secuConfigurations[0].securityPolicy,
                                               SOPC_SecurityPolicy_Basic256Sha256_URI);
        pEpConfig->secuConfigurations[0].securityModes =
            SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
        pEpConfig->secuConfigurations[0].nbOfUserTokenPolicies = 2;
        pEpConfig->secuConfigurations[0].userTokenPolicies[0] = c_userTokenPolicy_Anonymous;
        pEpConfig->secuConfigurations[0].userTokenPolicies[1] = c_userTokenPolicy_UserName_NoneSecurityPolicy;

        /*
         * 2nd Security policy is Basic256 with anonymous and username (non encrypted) authentication allowed
         */
        if (SOPC_STATUS_OK == status)
        {
            SOPC_String_Initialize(&pEpConfig->secuConfigurations[1].securityPolicy);
            status = SOPC_String_AttachFromCstring(&pEpConfig->secuConfigurations[1].securityPolicy,
                                                   SOPC_SecurityPolicy_Basic256_URI);
            pEpConfig->secuConfigurations[1].securityModes =
                SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
            pEpConfig->secuConfigurations[1].nbOfUserTokenPolicies = 2;
            pEpConfig->secuConfigurations[1].userTokenPolicies[0] = c_userTokenPolicy_Anonymous;
            pEpConfig->secuConfigurations[1].userTokenPolicies[1] = c_userTokenPolicy_UserName_NoneSecurityPolicy;
        }
    }

    /*
     * 3rd Security policy is None with anonymous and username (non encrypted) authentication allowed
     * (for tests only, otherwise users on unsecure channel shall be forbidden)
     */
    uint8_t NoneSecuConfigIdx = 2;
    if (!secuActive)
    {
        // Keep only None secu and set it as first secu config in this case
        NoneSecuConfigIdx = 0;
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_String_Initialize(&pEpConfig->secuConfigurations[NoneSecuConfigIdx].securityPolicy);
        status = SOPC_String_AttachFromCstring(&pEpConfig->secuConfigurations[NoneSecuConfigIdx].securityPolicy,
                                               SOPC_SecurityPolicy_None_URI);
        pEpConfig->secuConfigurations[NoneSecuConfigIdx].securityModes = SOPC_SECURITY_MODE_NONE_MASK;
        pEpConfig->secuConfigurations[NoneSecuConfigIdx].nbOfUserTokenPolicies =
            2; /* Necessary for tests only: it shall be 0 when
                  security is None to avoid any possible session without security */
        pEpConfig->secuConfigurations[NoneSecuConfigIdx].userTokenPolicies[0] =
            c_userTokenPolicy_Anonymous; /* Necessary for tests only */
        pEpConfig->secuConfigurations[NoneSecuConfigIdx].userTokenPolicies[1] =
            c_userTokenPolicy_UserName_NoneSecurityPolicy; /* Necessary for UACTT tests only */
    }

    if (secuActive)
    {
        pEpConfig->nbSecuConfigs = 3;
    }
    else
    {
        // Only one security config if no secure endpoint defined
        pEpConfig->nbSecuConfigs = 1;
    }

    return status;
}

/*
 * Configure the cryptographic parameters of the endpoint:
 * - Server certificate and key
 * - Public Key Infrastructure: using a single certificate as Certificate Authority or Trusted Certificate
 */
static SOPC_ReturnStatus Server_SetCryptographicConfig(SOPC_Endpoint_Config* pEpConfig,
                                                       SOPC_SerializedCertificate** output_serverCertificate,
                                                       SOPC_SerializedAsymmetricKey** output_serverKey,
                                                       SOPC_SerializedCertificate** output_authCertificate,
                                                       SOPC_PKIProvider** output_pkiProvider)
{
    assert(NULL != output_serverCertificate);
    assert(NULL != output_serverKey);
    assert(NULL != output_authCertificate);
    assert(NULL != output_pkiProvider);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (secuActive)
    {
#ifdef WITH_STATIC_SECURITY_DATA
        /* Load client/server certificates and server key from C source files (no filesystem needed) */
        status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(server_2k_cert, sizeof(server_2k_cert),
                                                                     output_serverCertificate);
        if (SOPC_STATUS_OK == status)
        {
            pEpConfig->serverCertificate = serverCertificate;

            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(server_2k_key, sizeof(server_2k_key),
                                                                            output_serverKey);
        }

        if (SOPC_STATUS_OK == status)
        {
            pEpConfig->serverKey = serverKey;

            status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(cacert, sizeof(cacert), &authCertificate);
        }
#else // WITH_STATIC_SECURITY_DATA == false
        /* Load client/server certificates and server key from files */
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile("./server_public/server_2k_cert.der",
                                                                      output_serverCertificate);
        if (SOPC_STATUS_OK == status)
        {
            pEpConfig->serverCertificate = *output_serverCertificate;

            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile("./server_private/server_2k_key.pem",
                                                                            output_serverKey);
        }
        if (SOPC_STATUS_OK == status)
        {
            pEpConfig->serverKey = *output_serverKey;

            status =
                SOPC_KeyManager_SerializedCertificate_CreateFromFile("./trusted/cacert.der", output_authCertificate);
        }
#endif

        /* Create the PKI (Public Key Infrastructure) provider */
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_Create(*output_authCertificate, NULL, output_pkiProvider);
            pEpConfig->pki = *output_pkiProvider;
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
    else
    {
        pEpConfig->serverCertificate = NULL;
        pEpConfig->serverKey = NULL;
        pEpConfig->pki = NULL;
    }

    return status;
}

/*---------------------------------
 * Server application description :
 *---------------------------------*/

/*
 * Application description of server to be returned by discovery services (GetEndpoints, FindServers)
 */
static void Server_SetApplicationDescriptionConfig(SOPC_Endpoint_Config* pEpConfig)
{
    // Application description configuration
    OpcUa_ApplicationDescription_Initialize(&pEpConfig->serverDescription);
    SOPC_String_AttachFromCstring(&pEpConfig->serverDescription.ApplicationUri, APPLICATION_URI);
    SOPC_String_AttachFromCstring(&pEpConfig->serverDescription.ProductUri, PRODUCT_URI);
    pEpConfig->serverDescription.ApplicationType = OpcUa_ApplicationType_Server;
    SOPC_String_AttachFromCstring(&pEpConfig->serverDescription.ApplicationName.Text, "S2OPC toolkit server example");
    SOPC_String_AttachFromCstring(&pEpConfig->serverDescription.ApplicationName.Locale, "en-US");
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
    /* avoid unused parameter compiler warning */
    (void) (authn);

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
    .pFuncFree = (SOPC_UserAuthentication_Free_Func) SOPC_Free,
    .pFuncValidateUserIdentity = authentication_uactt};

static SOPC_ReturnStatus Server_SetUserManagementConfig(SOPC_Endpoint_Config* pEpConfig,
                                                        SOPC_UserAuthentication_Manager** output_authenticationManager,
                                                        SOPC_UserAuthorization_Manager** output_authorizationManager)
{
    assert(NULL != output_authenticationManager);
    assert(NULL != output_authorizationManager);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Create an user authorization manager which accepts any user.
     * i.e.: UserAccessLevel right == AccessLevel right for any user for a given node of address space */
    *output_authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
    *output_authenticationManager = SOPC_Calloc(1, sizeof(SOPC_UserAuthentication_Manager));
    if (NULL == *output_authenticationManager || NULL == *output_authorizationManager)
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
        printf("<Test_Server_Toolkit: Failed to create the user manager\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Set a user authentication function that complies with UACTT tests expectations */
        (*output_authenticationManager)->pFunctions = &authentication_uactt_functions;
        pEpConfig->authenticationManager = *output_authenticationManager;
        pEpConfig->authorizationManager = *output_authorizationManager;
    }

    return status;
}

/*------------------------------
 * Address space configuraiton :
 *------------------------------*/

/*
 * XML dynamic loader use for parsing an XML address space defintion file
 */
#ifdef WITH_EXPAT
static SOPC_AddressSpace* load_nodeset_from_file(const char* filename)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_AddressSpace* space = NULL;
    FILE* fd = fopen(filename, "r");

    if (fd == NULL)
    {
        printf("<Test_Server_Toolkit: Error while opening %s: %s\n", filename, strerror(errno));
        status = SOPC_STATUS_NOK;
    }

    if (status == SOPC_STATUS_OK)
    {
        space = SOPC_UANodeSet_Parse(fd);

        if (space == NULL)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (status != SOPC_STATUS_OK)
    {
        printf("<Test_Server_Toolkit: Error while parsing XML address space\n");
    }

    if (fd != NULL)
    {
        fclose(fd);
    }

    if (status == SOPC_STATUS_OK)
    {
        printf("<Test_Server_Toolkit: Loaded address space from %s\n", filename);
        return space;
    }
    else
    {
        SOPC_AddressSpace_Delete(space);
        return NULL;
    }
}
#endif

static SOPC_ReturnStatus Server_ConfigureAddressSpace(SOPC_AddressSpace** output_addressSpace)
{
    /* Define server address space loader:
     * If WITH_EXPAT environment variable defined,
     * retrieve XML file path from environment variable TEST_SERVER_XML_ADDRESS_SPACE.
     * In case of success, use the dynamic address space loader from an XML file.
     *
     * Otherwise use the embedded address space (already defined as C code) loader.
     * For this latter case the address space C structure shall have been generated prior to compilation.
     * This should be done using the script ./scripts/generate-s2opc-address-space.py
     */
    assert(NULL != output_addressSpace);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    AddressSpaceLoader address_space_loader = AS_LOADER_EMBEDDED;

#ifdef WITH_EXPAT
    const char* xml_file_path = getenv("TEST_SERVER_XML_ADDRESS_SPACE");

    if (xml_file_path != NULL)
    {
        address_space_loader = AS_LOADER_EXPAT;
    }
#endif

    /* Load the address space using loader */
    switch (address_space_loader)
    {
    case AS_LOADER_EMBEDDED:
        *output_addressSpace = SOPC_Embedded_AddressSpace_Load();
        status = NULL != *output_addressSpace ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        break;
#ifdef WITH_EXPAT
    case AS_LOADER_EXPAT:
        *output_addressSpace = load_nodeset_from_file(xml_file_path);
        status = (NULL != *output_addressSpace) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        break;
#endif
    }

    /* Set the loaded address space as the current server address space configuration */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceConfig(*output_addressSpace);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed to configure the @ space\n");
        }
        else
        {
            printf("<Test_Server_Toolkit: @ space configured\n");
        }
    }

    return status;
}

/*-----------------------
 * Logger configuration :
 *-----------------------*/

/* Set the log path and create (or keep existing) directory path built on executable path
 *  + first argument of main */
static char* Config_SetLogPath(int argc, char* argv[])
{
    char* underscore = "_";
    char* suffix = NULL;
    char* logDirPath = NULL;
    SOPC_StatusCode status = SOPC_STATUS_NOK;

    if (argc > 1)
    {
        suffix = argv[1];
    }
    else
    {
        suffix = "";
        underscore = "";
    }

    size_t logDirPathSize = 2 + strlen(argv[0]) + strlen(underscore) + strlen(suffix) +
                            7; // "./" + exec_name + _ + test_name + _logs/ + '\0'
    if (logDirPathSize < 200)
    {
        logDirPath = SOPC_Malloc(logDirPathSize * sizeof(char));
    }
    if (NULL != logDirPath && (int) (logDirPathSize - 1) ==
                                  snprintf(logDirPath, logDirPathSize, "./%s%s%s_logs/", argv[0], underscore, suffix))
    {
        status = SOPC_ToolkitConfig_SetCircularLogPath(logDirPath, true);
    }
    else
    {
        status = SOPC_ToolkitConfig_SetCircularLogPath("./toolkit_test_server_logs", true);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(logDirPath);
        logDirPath = NULL;
    }

    return logDirPath;
}

static char* Server_ConfigureLogger(int argc, char* argv[])
{
    // Define directoy path for log traces: ./<exec_name>_argv[1]_logs/
    char* logDirPath = Config_SetLogPath(argc, argv);

    if (NULL != logDirPath)
    {
        // Set log traces to DEBUG level: displays DEBUG, INFO, WARNING and ERROR
        if (SOPC_STATUS_OK != SOPC_ToolkitConfig_SetLogLevel(SOPC_TOOLKIT_LOG_LEVEL_DEBUG))
        {
            SOPC_Free(logDirPath);
            logDirPath = NULL;
        }
    }

    return logDirPath;
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, Test_StopSignal);
    signal(SIGTERM, Test_StopSignal);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    char* logDirPath = NULL;

    SOPC_Endpoint_Config epConfig;
    uint32_t epConfigIdx = 0;

    SOPC_SerializedCertificate* serverCertificate = NULL;
    SOPC_SerializedAsymmetricKey* serverKey = NULL;
    SOPC_SerializedCertificate* authCertificate = NULL;
    SOPC_PKIProvider* pkiProvider = NULL;

    SOPC_UserAuthentication_Manager* authenticationManager = NULL;
    SOPC_UserAuthorization_Manager* authorizationManager = NULL;

    SOPC_AddressSpace* address_space = NULL;

    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;

    /* Get the toolkit build information and print it */
    SOPC_Build_Info build_info = SOPC_ToolkitConfig_GetBuildInfo();
    printf("toolkitVersion: %s\n", build_info.toolkitVersion);
    printf("toolkitSrcCommit: %s\n", build_info.toolkitSrcCommit);
    printf("toolkitDockerId: %s\n", build_info.toolkitDockerId);
    printf("toolkitBuildDate: %s\n", build_info.toolkitBuildDate);

    /* Initialize the server library (start library threads)
     * and define communication events callback */
    status = Server_Initialize();

    /* Configure the server logger:
     * DEBUG traces generated in ./<argv[0]>_<argv[1]>_logs/ */
    logDirPath = Server_ConfigureLogger(argc, argv);

    /* Prepare configuration of server endpoint:
       - Enpoint URL,
       - Security endpoint properties,
       - Cryptographic parameters,
       - User authentication and authorization management,
       - Application description
    */
    if (SOPC_STATUS_OK == status)
    {
        epConfig = Server_CreateEnpointConfig(ENDPOINT_URL);

        status = Server_SetEnpointSecurityConfig(&epConfig);

        if (SOPC_STATUS_OK == status)
        {
            status = Server_SetCryptographicConfig(&epConfig, &serverCertificate, &serverKey, &authCertificate,
                                                   &pkiProvider);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = Server_SetUserManagementConfig(&epConfig, &authenticationManager, &authorizationManager);
        }

        if (SOPC_STATUS_OK == status)
        {
            Server_SetApplicationDescriptionConfig(&epConfig);
        }
    }

    /* Set endpoint configuration: keep endpoint configuration identifier for opening it later */
    if (SOPC_STATUS_OK == status)
    {
        epConfigIdx = SOPC_ToolkitServer_AddEndpointConfig(&epConfig);
        status = (epConfigIdx != 0 ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
    }

    /* Configure the address space of the server */
    if (SOPC_STATUS_OK == status)
    {
        /* Address space loaded dynamically from XML file
           or from pre-generated C structure */
        status = Server_ConfigureAddressSpace(&address_space);
    }

    /* Define address space modification notification callback */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceNotifCb(&Test_AddressSpaceNotif_Fct);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed to configure the @ space modification notification callback \n");
        }
        else
        {
            printf("<Test_Server_Toolkit: @ space modification notification callback configured\n");
        }
    }

    /* Finalize the server configuration */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Configured();

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed to configure the endpoint\n");
        }
        else
        {
            printf("<Test_Server_Toolkit: Endpoint configured\n");
        }
    }

    /* Asynchronous request to open the configured endpoint using endpoint configuration index */
    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Toolkit: Opening endpoint... \n");
        SOPC_ToolkitServer_AsyncOpenEndpoint(epConfigIdx);
    }

    /* Update server information runtime variables in address space */
    if (SOPC_STATUS_OK == status)
    {
        RuntimeVariables runtime_vars =
            build_runtime_variables(build_info, PRODUCT_URI, app_namespace_uris, "Systerel");

        if (!set_runtime_variables(epConfigIdx, runtime_vars))
        {
            printf("<Test_Server_Toolkit: Failed to populate Server object");
            status = SOPC_STATUS_NOK;
        }
    }

    /* Run the server until notification that endpoint is closed received
     *  or stop server signal detected (Ctrl-C) */
    while (SOPC_STATUS_OK == status && stopServer == 0 && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        SOPC_Sleep(sleepTimeout);
    }

    /* Asynchronous request to close the endpoint */
    if (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        SOPC_ToolkitServer_AsyncCloseEndpoint(epConfigIdx);
    }

    /* Wait until endpoint is closed or stop server signal */
    while (SOPC_STATUS_OK == status && stopServer == 0 && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    /* Clear the server library (stop all library threads) */
    SOPC_Toolkit_Clear();

    // Check that endpoint closed due to stop signal
    if (SOPC_STATUS_OK == status && stopServer == 0)
    {
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Toolkit final result: OK\n");
    }
    else
    {
        printf("<Test_Server_Toolkit final result: NOK with status = '%d'\n", status);
    }

    /* Deallocate all locally created structures: */

    SOPC_AddressSpace_Delete(address_space);

    if (secuActive)
    {
        SOPC_KeyManager_SerializedCertificate_Delete(serverCertificate);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(serverKey);
        SOPC_KeyManager_SerializedCertificate_Delete(authCertificate);
        SOPC_PKIProvider_Free(&pkiProvider);
    }

    SOPC_UserAuthentication_FreeManager(&authenticationManager);
    SOPC_UserAuthorization_FreeManager(&authorizationManager);

    SOPC_Free(logDirPath);

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
