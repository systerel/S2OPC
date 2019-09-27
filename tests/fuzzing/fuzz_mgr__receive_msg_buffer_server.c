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
#include <unistd.h>

#include "embedded/loader.h"
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
#include "sopc_user_app_itf.h"

#include "fuzz_mgr__receive_msg_buffer.h"
#include "fuzz_mgr__receive_msg_buffer_client.h"
#include "fuzz_mgr__receive_msg_buffer_server.h"

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#endif

// Function declaration
static SOPC_ReturnStatus GenEndpoingConfig(SOPC_Endpoint_Config* epConfig);
static SOPC_ReturnStatus CerAndKeyLoader_serv(SOPC_Endpoint_Config* epConfig);
static char* Config_SetLogPath(void);

// These variables are global to be accessible from StopSignal_serv
volatile sig_atomic_t stopServer = 0;
static int32_t endpointClosed = 0;
static t_CerKey ck;
SOPC_AddressSpace* address_space = NULL;
uint32_t epConfigIdx = 0;

// Secu policy configuration:
SOPC_UserAuthentication_Manager* authenticationManager = NULL;
SOPC_UserAuthorization_Manager* authorizationManager = NULL;

char* logDirPath = NULL;

void StopSignal_serv(int sig)
{
    /* avoid unused parameter compiler warning */
    (void) sig;

    if (stopServer != 0)
    {
        Teardown_serv();
        exit(1);
    }
    else
    {
        stopServer = 1;
    }
}

static char* Config_SetLogPath(void)
{
    char* logDirPathLocal = NULL;
    size_t logDirPathSize = 2 + strlen("toolkit_test_server") + 1 + strlen("./toolkit_test_server_logs/") +
                            7; // "./" + exec_name + _ + test_name + _logs/ + '\0'
    if (logDirPathSize < 200)
    {
        logDirPathLocal = SOPC_Malloc(logDirPathSize * sizeof(char));
    }
    if (NULL != logDirPathLocal &&
        (int) (logDirPathSize - 1) == snprintf(logDirPathLocal, logDirPathSize, "./%s_%s_logs/", "toolkit_test_server",
                                               "./toolkit_test_server_logs/"))
    {
        SOPC_StatusCode status = SOPC_ToolkitConfig_SetCircularLogPath(logDirPathLocal, true);
        if (status == SOPC_STATUS_OK)
        {
            return logDirPathLocal;
        }
    }
    SOPC_Free(logDirPathLocal);
    return NULL;
}

static SOPC_ReturnStatus GenEndpoingConfig(SOPC_Endpoint_Config* epConfig)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (secuActive)
    {
        // 1st Security policy is Basic256Sha256 with anonymous and username (non encrypted) authentication allowed
        SOPC_String_Initialize(&(*epConfig).secuConfigurations[0].securityPolicy);
        status = SOPC_String_AttachFromCstring(&(*epConfig).secuConfigurations[0].securityPolicy,
                                               SOPC_SecurityPolicy_Basic256Sha256_URI);
        (*epConfig).secuConfigurations[0].securityModes = SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
        (*epConfig).secuConfigurations[0].nbOfUserTokenPolicies = 1;
        (*epConfig).secuConfigurations[0].userTokenPolicies[0] = c_userTokenPolicy_Anonymous;
    }

    // 2rd Security policy is None with anonymous and username (non encrypted) authentication allowed
    //(for tests only, otherwise users on unsecure channel shall be forbidden)
    uint8_t NoneSecuConfigIdx = 1;
    if (!secuActive)
    {
        // Keep only None secu and set it as first secu config in this case
        NoneSecuConfigIdx = 0;
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_String_Initialize(&(*epConfig).secuConfigurations[NoneSecuConfigIdx].securityPolicy);
        status = SOPC_String_AttachFromCstring(&(*epConfig).secuConfigurations[NoneSecuConfigIdx].securityPolicy,
                                               SOPC_SecurityPolicy_None_URI);
        (*epConfig).secuConfigurations[NoneSecuConfigIdx].securityModes = SOPC_SECURITY_MODE_NONE_MASK;
        (*epConfig).secuConfigurations[NoneSecuConfigIdx].nbOfUserTokenPolicies =
            1; /* Necessary for tests only: it shall be 0 when
                  security is None to avoid any possible session without security */
        (*epConfig).secuConfigurations[NoneSecuConfigIdx].userTokenPolicies[0] =
            c_userTokenPolicy_Anonymous; /* Necessary for tests only */
    }

    // Init unique endpoint structure
    (*epConfig).endpointURL = ENDPOINT_URL;

    if (secuActive)
    {
        (*epConfig).nbSecuConfigs = 2;
    }
    else
    {
        (*epConfig).nbSecuConfigs = 1;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = CerAndKeyLoader_serv(&epConfig);
    }

    // Application description configuration
    if (SOPC_STATUS_OK == status)
    {
        OpcUa_ApplicationDescription_Initialize(&(*epConfig).serverDescription);
        SOPC_String_AttachFromCstring(&(*epConfig).serverDescription.ApplicationUri, APPLICATION_URI);
        SOPC_String_AttachFromCstring(&(*epConfig).serverDescription.ProductUri, PRODUCT_URI);
        (*epConfig).serverDescription.ApplicationType = OpcUa_ApplicationType_Server;
        SOPC_String_AttachFromCstring(&(*epConfig).serverDescription.ApplicationName.Text,
                                      "S2OPC toolkit server example");
        SOPC_String_AttachFromCstring(&(*epConfig).serverDescription.ApplicationName.Locale, "en-US");
    }

    if (SOPC_STATUS_OK == status)
    {
        authenticationManager = SOPC_Calloc(1, sizeof(SOPC_UserAuthentication_Manager));
        authorizationManager = SOPC_UserAuthorization_CreateManager_AllowAll();
        if (NULL == authenticationManager || NULL == authorizationManager)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
            if (true == debug)
            {
                printf("<Test_Server_Toolkit: Failed to create the user manager\n");
            }
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        (*epConfig).authenticationManager = authenticationManager;
        (*epConfig).authorizationManager = authorizationManager;
    }
    return (status);
}

static t_CerKey CerAndKeyLoader_serv(SOPC_Endpoint_Config* epConfig)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (secuActive)
    {
#ifdef WITH_STATIC_SECURITY_DATA
        status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(server_2k_cert, sizeof(server_2k_cert),
                                                                     &(ck).Certificate);
        epConfig.serverCertificate = &(ck).Certificate;

        if (SOPC_STATUS_OK == status)
        {
            status =
                SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(server_2k_key, sizeof(server_2k_key), &(ck).Key);
            epConfig.serverKey = &(ck).Key;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(cacert, sizeof(cacert), &(ck).authCertificate);
        }
#else
        if (true == debug)
        {
            Printf("<<FUZZ: Must compile with: 'WITH_STATIC_SECURITY_DATA'\n");
        }
#endif

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_Create(&(ck).authCertificate, NULL, &(ck).pkiProvider);
            (*epConfig).pki = &(ck).pkiProvider;
        }
        if (SOPC_STATUS_OK == status)
        {
            if (true == debug)
            {
                printf(">>FUZZ_server: Certificates and key loaded\n");
            }
        }
        else
        {
            if (true == debug)
            {
                printf(">>FUZZ_server: Failed loading certificates and key (check paths are valid)\n");
            }
        }
    }
    else
    {
        (*epConfig).serverCertificate = NULL;
        (*epConfig).serverKey = NULL;
        (*epConfig).pki = NULL;
    }
    return (status);
}

SOPC_ReturnStatus Setup_serv(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Endpoint_Config epConfig;
    AddressSpaceLoader address_space_loader = AS_LOADER_EMBEDDED;
    // Get Toolkit Configuration
    SOPC_Build_Info build_info = SOPC_ToolkitConfig_GetBuildInfo();
    const uint32_t sleepTimeout = 500;

    printf("toolkitVersion: %s\n", build_info.toolkitVersion);
    printf("toolkitSrcCommit: %s\n", build_info.toolkitSrcCommit);
    printf("toolkitDockerId: %s\n", build_info.toolkitDockerId);
    printf("toolkitBuildDate: %s\n", build_info.toolkitBuildDate);

    status = GenEndpoingConfig(&epConfig);

    // Init stack configuration
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(Fuzz_Event_Fct);
        if (SOPC_STATUS_OK == status)
        {
            if (true == debug)
            {
                printf(">>FUZZ_server: initialized\n");
            }
        }
        else
        {
            if (true == debug)
            {
                printf(">>FUZZ_server: Failed initializing\n");
            }
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        logDirPath = Config_SetLogPath();

        status = SOPC_ToolkitConfig_SetLogLevel(SOPC_TOOLKIT_LOG_LEVEL_ERROR);
    }

    // Define server address space
    if (SOPC_STATUS_OK == status)
    {
        switch (address_space_loader)
        {
        case AS_LOADER_EMBEDDED:
            address_space = SOPC_Embedded_AddressSpace_Load();
            status = SOPC_STATUS_OK;
            break;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitServer_SetAddressSpaceConfig(address_space);

        if (SOPC_STATUS_OK != status)
        {
            if (true == debug)
            {
                printf("<<FUZZ_server: Failed to configure the @ space\n");
            }
        }
        else
        {
            if (true == debug)
            {
                printf("<<FUZZ_server: @ space configured\n");
            }
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
        if (SOPC_STATUS_OK == status)
        {
            if (true == debug)
            {
                printf("<<FUZZ_server: Endpoint configured\n");
            }
        }
        else
        {
            if (true == debug)
            {
                printf("<<FUZZ_server: Failed to configure the endpoint\n");
            }
        }
    }

    // Asynchronous request to open the endpoint
    if (SOPC_STATUS_OK == status)
    {
        if (true == debug)
        {
            printf("<<FUZZ_server: Opening endpoint... \n");
        }
        SOPC_ToolkitServer_AsyncOpenEndpoint(epConfigIdx);
    }

    return (status);
}

void Teardown_serv()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    const uint32_t sleepTimeout = 500;

    // Run the server until notification that endpoint is closed or stop server signal
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
        if (true == debug)
        {
            printf("<<FUZZ: final result: OK\n");
        }
    }
    else
    {
        if (true == debug)
        {
            printf("<<FUZZ: final result: NOK with status = '%d'\n", status);
        }
    }

    // Deallocate locally allocated data
    SOPC_AddressSpace_Delete(address_space);
    if (true == secuActive)
    {
        SOPC_KeyManager_SerializedCertificate_Delete(&(ck).Certificate);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(&(ck).Key);
        SOPC_KeyManager_SerializedCertificate_Delete(&(ck).authCertificate);
        SOPC_PKIProvider_Free(&(ck).pkiProvider);
    }

    SOPC_UserAuthentication_FreeManager(&authenticationManager);
    SOPC_UserAuthorization_FreeManager(&authorizationManager);
    SOPC_Free(logDirPath);
}
