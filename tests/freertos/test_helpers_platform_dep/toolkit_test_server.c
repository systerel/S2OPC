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
#include <stdlib.h>
#include <string.h>

#include "sopc_atomic.h"
#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_pki_stack.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_encodeable.h"
#include "sopc_time.h"

#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"

#include "p_ethernet_if.h"
#include "p_logsrv.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"

#include "embedded/loader.h"
#include "runtime_variables.h"

extern tLogSrvWks* gLogServer;

#ifdef WITH_EXPAT
#include "uanodeset_expat/loader.h"
#endif

#define ENDPOINT_URL "opc.tcp://localhost:4841"
#define APPLICATION_URI "urn:S2OPC:localhost"
#define PRODUCT_URI "urn:S2OPC:localhost"

static const char* app_namespace_uris[] = {(const char*) PRODUCT_URI, NULL};

static int32_t endpointClosed = 0;
static bool secuActive = false;

volatile sig_atomic_t stopServer = 0;

typedef enum
{
    AS_LOADER_EMBEDDED,
#ifdef WITH_EXPAT
    AS_LOADER_EXPAT,
#endif
} AddressSpaceLoader;

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

/* Set the log path and create (or keep existing) directory path built on executable path
 *  + first argument of main */
static char* Config_SetLogPath(int argc, char* argv[])
{
    if (argc > 1)
    {
        char* logDirPath = NULL;
        size_t logDirPathSize = 2 + strlen("toolkit_test_server") + 1 + strlen(argv[1]) +
                                7; // "./" + exec_name + _ + test_name + _logs/ + '\0'
        if (logDirPathSize < 200)
        {
            logDirPath = malloc(logDirPathSize * sizeof(char));
        }
        if (NULL != logDirPath && (int) (logDirPathSize - 1) == snprintf(logDirPath, logDirPathSize, "./%s_%s_logs/",
                                                                         "toolkit_test_server", argv[1]))
        {
            SOPC_StatusCode status = SOPC_ToolkitConfig_SetCircularLogPath(logDirPath, true);
            if (status == SOPC_STATUS_OK)
            {
                return logDirPath;
            }
        }
        free(logDirPath);
    }
    else
    {
        SOPC_ToolkitConfig_SetCircularLogPath("./toolkit_test_server_logs/", true);
    }
    return NULL;
}

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
    .pFuncFree = (SOPC_UserAuthentication_Free_Func) free,
    .pFuncValidateUserIdentity = authentication_uactt};

void* cbToolkit_test_server(void* arg)
{
    int argc = 0;
    char* argv[1] = {NULL};
    Condition* pv = (Condition*) arg;
    (void) pv;

    SOPC_LogSrv_Start();

    while (P_ETHERNET_IF_IsReady() != 0)
        ;

    SOPC_LogSrv_WaitClient(UINT32_MAX);

    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, Test_StopSignal);
    signal(SIGTERM, Test_StopSignal);

    char* logDirPath = NULL;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t epConfigIdx = 0;
    SOPC_Endpoint_Config epConfig;
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;

    // Secu policy configuration:
    static SOPC_SerializedCertificate* serverCertificate = NULL;
    static SOPC_SerializedAsymmetricKey* serverKey = NULL;
    static SOPC_SerializedCertificate* authCertificate = NULL;
    static SOPC_PKIProvider* pkiProvider = NULL;

    AddressSpaceLoader address_space_loader = AS_LOADER_EMBEDDED;
    SOPC_AddressSpace* address_space = NULL;

    // Get Toolkit Configuration
    SOPC_Build_Info build_info = SOPC_ToolkitConfig_GetBuildInfo();
    printf("toolkitVersion: %s\n", build_info.toolkitVersion);
    printf("toolkitSrcCommit: %s\n", build_info.toolkitSrcCommit);
    printf("toolkitDockerId: %s\n", build_info.toolkitDockerId);
    printf("toolkitBuildDate: %s\n", build_info.toolkitBuildDate);

    if (SOPC_STATUS_OK == status)
    {
        /*
         * 1st Security policy is None with anonymous and username (non encrypted) authentication allowed
         * (for tests only, otherwise users on unsecure channel shall be forbidden)
         */
        SOPC_String_Initialize(&epConfig.secuConfigurations[0].securityPolicy);
        status =
            SOPC_String_AttachFromCstring(&epConfig.secuConfigurations[0].securityPolicy, SOPC_SecurityPolicy_None_URI);
        epConfig.secuConfigurations[0].securityModes = SOPC_SECURITY_MODE_NONE_MASK;
        epConfig.secuConfigurations[0].nbOfUserTokenPolicies = 2;
        epConfig.secuConfigurations[0].userTokenPolicies[0] =
            c_userTokenPolicy_Anonymous; /* Necessary for tests only */
        epConfig.secuConfigurations[0].userTokenPolicies[1] =
            c_userTokenPolicy_UserName_NoneSecurityPolicy; /* Necessary for UACTT tests only */

        /*
         * 2nd Security policy is Basic256 with anonymous and username (non encrypted) authentication allowed
         */
        if (SOPC_STATUS_OK == status)
        {
            SOPC_String_Initialize(&epConfig.secuConfigurations[1].securityPolicy);
            status = SOPC_String_AttachFromCstring(&epConfig.secuConfigurations[1].securityPolicy,
                                                   SOPC_SecurityPolicy_Basic256_URI);
            epConfig.secuConfigurations[1].securityModes =
                SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
            epConfig.secuConfigurations[1].nbOfUserTokenPolicies = 2;
            epConfig.secuConfigurations[1].userTokenPolicies[0] = c_userTokenPolicy_Anonymous;
            epConfig.secuConfigurations[1].userTokenPolicies[1] = c_userTokenPolicy_UserName_NoneSecurityPolicy;
        }
        /*
         * 3rd Security policy is Basic256Sha256 with anonymous and username (non encrypted) authentication allowed
         */
        if (SOPC_STATUS_OK == status)
        {
            SOPC_String_Initialize(&epConfig.secuConfigurations[2].securityPolicy);
            status = SOPC_String_AttachFromCstring(&epConfig.secuConfigurations[2].securityPolicy,
                                                   SOPC_SecurityPolicy_Basic256Sha256_URI);
            epConfig.secuConfigurations[2].securityModes = SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
            epConfig.secuConfigurations[2].nbOfUserTokenPolicies = 2;
            epConfig.secuConfigurations[2].userTokenPolicies[0] = c_userTokenPolicy_Anonymous;
            epConfig.secuConfigurations[2].userTokenPolicies[1] = c_userTokenPolicy_UserName_NoneSecurityPolicy;
        }
    }

    // Init unique endpoint structure
    epConfig.endpointURL = ENDPOINT_URL;

    if (secuActive != false)
    {
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile("./server_public/server_2k_cert.der",
                                                                      &serverCertificate);
        epConfig.serverCertificate = serverCertificate;

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile("./server_private/server_2k_key.pem",
                                                                            &serverKey);
            epConfig.serverKey = serverKey;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromFile("./trusted/cacert.der", &authCertificate);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_Create(authCertificate, NULL, &pkiProvider);
            epConfig.pki = pkiProvider;
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
        epConfig.serverCertificate = NULL;
        epConfig.serverKey = NULL;
        epConfig.pki = NULL;
    }

    if (secuActive)
    {
        epConfig.nbSecuConfigs = 3;
    }
    else
    {
        epConfig.nbSecuConfigs = 1;
    }

    // Application description configuration
    OpcUa_ApplicationDescription_Initialize(&epConfig.serverDescription);
    SOPC_String_AttachFromCstring(&epConfig.serverDescription.ApplicationUri, APPLICATION_URI);
    SOPC_String_AttachFromCstring(&epConfig.serverDescription.ProductUri, PRODUCT_URI);
    epConfig.serverDescription.ApplicationType = OpcUa_ApplicationType_Server;
    SOPC_String_AttachFromCstring(&epConfig.serverDescription.ApplicationName.Text, "S2OPC toolkit server example");

    SOPC_UserAuthentication_Manager* authenticationManager = NULL;
    SOPC_UserAuthorization_Manager* authorizationManager = NULL;
    if (SOPC_STATUS_OK == status)
    {
        authenticationManager = calloc(1, sizeof(SOPC_UserAuthentication_Manager));
        authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
        if (NULL == authenticationManager || NULL == authorizationManager)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
            printf("<Test_Server_Toolkit: Failed to create the user manager\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        authenticationManager->pFunctions = &authentication_uactt_functions;
        epConfig.authenticationManager = authenticationManager;
        epConfig.authorizationManager = authorizationManager;
    }

    // Init stack configuration
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(Test_ComEvent_FctServer);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed initializing\n");
        }
        else
        {
            printf("<Test_Server_Toolkit: initialized\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        logDirPath = Config_SetLogPath(argc, argv);

        status = SOPC_ToolkitConfig_SetLogLevel(SOPC_TOOLKIT_LOG_LEVEL_DEBUG);
    }

    // Define server address space
    if (SOPC_STATUS_OK == status)
    {
#ifdef WITH_EXPAT
        const char* xml_file_path = getenv("TEST_SERVER_XML_ADDRESS_SPACE");

        if (xml_file_path != NULL)
        {
            address_space_loader = AS_LOADER_EXPAT;
        }
#endif

        switch (address_space_loader)
        {
        case AS_LOADER_EMBEDDED:
            address_space = SOPC_Embedded_AddressSpace_Load();
            status = SOPC_STATUS_OK;
            break;
#ifdef WITH_EXPAT
        case AS_LOADER_EXPAT:
            address_space = load_nodeset_from_file(xml_file_path);
            status = (address_space != NULL) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
            break;
#endif
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceConfig(address_space);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed to configure the @ space\n");
        }
        else
        {
            printf("<Test_Server_Toolkit: @ space configured\n");
        }
    }

    // Define address space modification notification callback
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

    // Add endpoint description configuration
    if (SOPC_STATUS_OK == status)
    {
        epConfigIdx = SOPC_ToolkitServer_AddEndpointConfig(&epConfig);
        if (epConfigIdx != 0)
        {
            status = SOPC_Toolkit_Configured();
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Toolkit: Failed to configure the endpoint\n");
        }
        else
        {
            printf("<Test_Server_Toolkit: Endpoint configured\n");
        }
    }

    // Asynchronous request to open the endpoint
    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Toolkit: Opening endpoint... \n");
        SOPC_ToolkitServer_AsyncOpenEndpoint(epConfigIdx);

        RuntimeVariables runtime_vars =
            build_runtime_variables(build_info, PRODUCT_URI, app_namespace_uris, "Systerel");

        if (!set_runtime_variables(epConfigIdx, runtime_vars))
        {
            printf("<Test_Server_Toolkit: Failed to populate Server object");
            status = SOPC_STATUS_NOK;
        }
    }

    // Run the server until notification that endpoint is closed or stop server signal
    while (SOPC_STATUS_OK == status && stopServer == 0 && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        SOPC_Sleep(sleepTimeout);
    }

    if (SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        // Asynchronous request to close the endpoint
        SOPC_ToolkitServer_AsyncCloseEndpoint(epConfigIdx);
    }

    // Wait until endpoint is closed or stop server signal
    while (SOPC_STATUS_OK == status && stopServer == 0 && SOPC_Atomic_Int_Get(&endpointClosed) == 0)
    {
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    // Clear the toolkit configuration and stop toolkit threads
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

    // Deallocate locally allocated data
    SOPC_AddressSpace_Delete(address_space);

    if (secuActive != false)
    {
        SOPC_KeyManager_SerializedCertificate_Delete(serverCertificate);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(serverKey);
        SOPC_KeyManager_SerializedCertificate_Delete(authCertificate);
        SOPC_PKIProvider_Free(&pkiProvider);
    }

    SOPC_UserAuthentication_FreeManager(&authenticationManager);
    SOPC_UserAuthorization_FreeManager(&authorizationManager);
    free(logDirPath);

    return (void*) ((status == SOPC_STATUS_OK) ? 0 : 1);
}
