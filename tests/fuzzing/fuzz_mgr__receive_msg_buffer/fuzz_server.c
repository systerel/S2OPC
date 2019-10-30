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

#include "embedded/sopc_addspace_loader.h"

#include "fuzz_server.h"
#include "fuzz_client.h"
#include "fuzz_main.h"

#ifdef WITH_STATIC_SECURITY_DATA
#include "static_security_data.h"
#endif

// Function declaration
static SOPC_ReturnStatus GenEndpoingConfig();
static SOPC_ReturnStatus CerAndKeyLoader_serv();

// These variables are global to be accessible from StopSignal_serv
SOPC_AddressSpace* address_space = NULL;
t_CerKey ck_serv;
volatile sig_atomic_t stopServer = 0;
uint32_t epConfigIdx = 0;

// Secu policy configuration:
SOPC_UserAuthentication_Manager* authenticationManager = NULL;
SOPC_UserAuthorization_Manager* authorizationManager = NULL;

void StopSignal_serv(int sig)
{
    /* avoid unused parameter compiler warning */
    (void) sig;

    while (stopServer == 0)
    {
        Teardown_client();
        Teardown_serv();
        stopServer = 1;
    }
    exit(1);
}

static SOPC_ReturnStatus CerAndKeyLoader_serv()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (secuActive)
    {
#ifdef WITH_STATIC_SECURITY_DATA
        status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(server_2k_cert, sizeof(server_2k_cert),
                                                                     &(ck_serv).Certificate);
        output_s2opcConfig.serverConfig.serverCertificate = &(ck_serv).Certificate;

        if (SOPC_STATUS_OK == status)
        {
            status =
                SOPC_KeyManager_SerializedAsymmetricKey_CreateFromData(server_2k_key, sizeof(server_2k_key), &(ck_serv).Key);
            output_s2opcConfig.serverConfig.serverKey = &(ck_serv).Key;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_SerializedCertificate_CreateFromDER(cacert, sizeof(cacert), &(ck_serv).authCertificate);
        }
#else
        if (true == debug)
        {
            printf("<<FUZZ: Must compile with: 'WITH_STATIC_SECURITY_DATA'\n");
        }
#endif

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_PKIProviderStack_Create(ck_serv.authCertificate, NULL, &(ck_serv).pkiProvider);
            output_s2opcConfig.serverConfig.pki = ck_serv.pkiProvider;
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
    return (status);
}

static SOPC_ReturnStatus GenEndpoingConfig()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Application description configuration
    if (SOPC_STATUS_OK == status)
    {
        //  Set application description of server to be returned by discovery services (GetEndpoints, FindServers)
        OpcUa_ApplicationDescription* serverDescription = &(output_s2opcConfig).serverConfig.serverDescription;
        OpcUa_ApplicationDescription_Initialize(serverDescription);
        SOPC_String_AttachFromCstring(&serverDescription->ApplicationUri, APPLICATION_URI);
        SOPC_String_AttachFromCstring(&serverDescription->ProductUri, PRODUCT_URI);
        serverDescription->ApplicationType = OpcUa_ApplicationType_Server;
        SOPC_String_AttachFromCstring(&serverDescription->ApplicationName.Text, "S2OPC fuzzing server");
        SOPC_String_AttachFromCstring(&serverDescription->ApplicationName.Locale, "en-US");

        output_s2opcConfig.serverConfig.endpoints = SOPC_Calloc(sizeof(SOPC_Endpoint_Config), 1);

        if (NULL == output_s2opcConfig.serverConfig.endpoints)
        {
            return false;
        }
    }

    output_s2opcConfig.serverConfig.nbEndpoints = 1;
    epConfig = output_s2opcConfig.serverConfig.endpoints[0];
    epConfig.serverConfigPtr = &(output_s2opcConfig.serverConfig);
    epConfig.endpointURL = ENDPOINT_URL;

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


    if (secuActive)
    {
        // 1st Security policy is Basic256Sha256 with anonymous and username (non encrypted) authentication allowed
        SOPC_String_Initialize(&epConfig.secuConfigurations[0].securityPolicy);
        status = SOPC_String_AttachFromCstring(&epConfig.secuConfigurations[0].securityPolicy,
                                               SOPC_SecurityPolicy_Basic256Sha256_URI);
        epConfig.secuConfigurations[0].securityModes = SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
        epConfig.secuConfigurations[0].nbOfUserTokenPolicies = 1;
        epConfig.secuConfigurations[0].userTokenPolicies[0] = c_userTokenPolicy_Anonymous;
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
        SOPC_String_Initialize(&epConfig.secuConfigurations[NoneSecuConfigIdx].securityPolicy);
        status = SOPC_String_AttachFromCstring(&epConfig.secuConfigurations[NoneSecuConfigIdx].securityPolicy,
                                               SOPC_SecurityPolicy_None_URI);
        epConfig.secuConfigurations[NoneSecuConfigIdx].securityModes = SOPC_SECURITY_MODE_NONE_MASK;
        epConfig.secuConfigurations[NoneSecuConfigIdx].nbOfUserTokenPolicies =
            1; /* Necessary for tests only: it shall be 0 when
                  security is None to avoid any possible session without security */
        epConfig.secuConfigurations[NoneSecuConfigIdx].userTokenPolicies[0] =
            c_userTokenPolicy_Anonymous; /* Necessary for tests only */
    }

    if (secuActive)
    {
        epConfig.nbSecuConfigs = 2;
    }
    else
    {
        epConfig.nbSecuConfigs = 1;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = CerAndKeyLoader_serv();
    }

    if (SOPC_STATUS_OK == status)
    {
        epConfig.authenticationManager = authenticationManager;
        epConfig.authorizationManager = authorizationManager;
    }
    return (status);
}

SOPC_ReturnStatus Setup_serv(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Get Toolkit Configuration
    SOPC_Build_Info build_info = SOPC_ToolkitConfig_GetBuildInfo();

    if (true == debug)
    {
        printf("toolkitVersion: %s\n", build_info.toolkitVersion);
        printf("toolkitSrcCommit: %s\n", build_info.toolkitSrcCommit);
        printf("toolkitDockerId: %s\n", build_info.toolkitDockerId);
        printf("toolkitBuildDate: %s\n", build_info.toolkitBuildDate);
    }

    status = GenEndpoingConfig();

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
        status = SOPC_ToolkitConfig_SetCircularLogPath("/tmp/mylogs/", true);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ToolkitConfig_SetLogLevel(SOPC_TOOLKIT_LOG_LEVEL_DEBUG);
    }

    // Define server address space
    if (SOPC_STATUS_OK == status)
    {
        address_space = SOPC_Embedded_AddressSpace_Load();
        if (NULL == address_space)
        {
        	status = SOPC_STATUS_NOK;
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

    if (SOPC_STATUS_OK == status)
    {
        // Add endpoint description configuration
        if (true == debug)
        {
        	printf("<<FUZZ_server: Opening endpoint... \n");
        }
        epConfigIdx = SOPC_ToolkitServer_AddEndpointConfig(&epConfig);
        status = (epConfigIdx != 0 ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
    }

    return (status);
}

SOPC_ReturnStatus SOPC_EpConfig_serv()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Asynchronous request to open the endpoint
    if (true == debug && epConfigIdx != 0)
    {
        printf("<<FUZZ_server: Endpoint configured\n");
    }
    else if (true == debug)
    {
        printf("<<FUZZ_server: Failed to configure the endpoint\n");
    }
    SOPC_ToolkitServer_AsyncOpenEndpoint(epConfigIdx);
    return (status);
}

SOPC_ReturnStatus Teardown_serv()
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;


    SOPC_ToolkitServer_AsyncCloseEndpoint(epConfigIdx);

    // Clear the toolkit configuration and stop toolkit threads

    // Deallocate locally allocated data
    SOPC_AddressSpace_Delete(address_space);
    if (true == secuActive)
    {
        SOPC_KeyManager_SerializedCertificate_Delete(ck_serv.Certificate);
        SOPC_KeyManager_SerializedAsymmetricKey_Delete(ck_serv.Key);
        SOPC_KeyManager_SerializedCertificate_Delete(ck_serv.authCertificate);
        SOPC_PKIProvider_Free(&(ck_serv).pkiProvider);
    }

//    SOPC_UserAuthentication_FreeManager(&authenticationManager);
//    SOPC_UserAuthorization_FreeManager(&authorizationManager);

    SOPC_Toolkit_Clear();
    printf("all cleared\n");
    return (status);
}
