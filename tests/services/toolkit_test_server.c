/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_pki_stack.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_encodeable.h"
#include "sopc_time.h"

#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"

#include "sopc_addspace.h"

#define ENDPOINT_URL "opc.tcp://localhost:4841"
#define APPLICATION_URI "urn:INGOPCS:localhost"
#define PRODUCT_URI "urn:INGOPCS:localhost"

static int endpointClosed = false;
static bool secuActive = !false;

volatile sig_atomic_t stopServer = 0;

void Test_StopSignal(int sig)
{
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

void Test_ComEvent_FctServer(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext)
{
    (void) idOrStatus;
    (void) param;
    (void) appContext;
    if (event == SE_CLOSED_ENDPOINT)
    {
        printf("<Test_Server_Toolkit: closed endpoint event: OK\n");
        endpointClosed = !false;
    }
    else
    {
        printf("<Test_Server_Toolkit: unexpected endpoint event %d : NOK\n", event);
    }
}

void Test_AddressSpaceNotif_Fct(SOPC_App_AddSpace_Event event, void* opParam, SOPC_StatusCode opStatus)
{
    OpcUa_WriteValue* wv = NULL;
    if (event == AS_WRITE_EVENT)
    {
        printf("<Test_Server_Toolkit: address space WRITE event: OK\n");
        wv = (OpcUa_WriteValue*) opParam;
        if (wv != NULL)
        {
            switch (wv->NodeId.IdentifierType)
            {
            case SOPC_IdentifierType_Numeric:
                printf("  NodeId: Namespace %" PRIu16 ", ID %" PRIu32 "\n", wv->NodeId.Namespace,
                       wv->NodeId.Data.Numeric);
                break;
            case SOPC_IdentifierType_String:
                printf("  NodeId: Namespace %" PRIu16 ", ID %s\n", wv->NodeId.Namespace,
                       SOPC_String_GetRawCString(&wv->NodeId.Data.String));
                break;
            default:
                break;
            }
            printf("  AttributeId: %" PRIu32 "\n", wv->AttributeId);
            printf("  Write status: %" PRIX32 "\n", opStatus);
        }
    }
    else
    {
        printf("<Test_Server_Toolkit: unexpected address space event %d : NOK\n", event);
    }
}

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, Test_StopSignal);
    signal(SIGTERM, Test_StopSignal);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint32_t epConfigIdx = 0;
    SOPC_Endpoint_Config epConfig;
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;

    // Secu policy configuration:
    static SOPC_Certificate* serverCertificate = NULL;
    static SOPC_AsymmetricKey* asymmetricKey = NULL;
    static SOPC_Certificate* authCertificate = NULL;
    static SOPC_PKIProvider* pkiProvider = NULL;

    SOPC_SecurityPolicy secuConfig[3];
    SOPC_String_Initialize(&secuConfig[0].securityPolicy);
    SOPC_String_Initialize(&secuConfig[1].securityPolicy);
    SOPC_String_Initialize(&secuConfig[2].securityPolicy);

    // Get Toolkit Configuration
    SOPC_Build_Info build_info = SOPC_ToolkitConfig_GetBuildInfo();
    printf("toolkitVersion: %s\n", build_info.toolkitVersion);
    printf("toolkitSrcCommit: %s\n", build_info.toolkitSrcCommit);
    printf("toolkitDockerId: %s\n", build_info.toolkitDockerId);
    printf("toolkitBuildDate: %s\n", build_info.toolkitBuildDate);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_AttachFromCstring(&secuConfig[0].securityPolicy, SOPC_SecurityPolicy_None_URI);
        secuConfig[0].securityModes = SOPC_SECURITY_MODE_NONE_MASK;
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_String_AttachFromCstring(&secuConfig[1].securityPolicy, SOPC_SecurityPolicy_Basic256_URI);
            secuConfig[1].securityModes = SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
        }
        if (SOPC_STATUS_OK == status)
        {
            status =
                SOPC_String_AttachFromCstring(&secuConfig[2].securityPolicy, SOPC_SecurityPolicy_Basic256Sha256_URI);
            secuConfig[2].securityModes = SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;
        }
    }

    // Init unique endpoint structure
    epConfig.endpointURL = ENDPOINT_URL;

    if (secuActive != false)
    {
        status = SOPC_KeyManager_Certificate_CreateFromFile("./server_public/server_2k.der", &serverCertificate);
        epConfig.serverCertificate = serverCertificate;

        if (SOPC_STATUS_OK == status)
        {
            status =
                SOPC_KeyManager_AsymmetricKey_CreateFromFile("./server_private/server_2k.key", &asymmetricKey, NULL, 0);
            epConfig.serverKey = asymmetricKey;
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_Certificate_CreateFromFile("./trusted/cacert.der", &authCertificate);
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

    epConfig.secuConfigurations = secuConfig;
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
    SOPC_String_AttachFromCstring(&epConfig.serverDescription.ApplicationName.Text, "INGOPCS toolkit server example");

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

    // Define server address space
    if (SOPC_STATUS_OK == status)
    {
        assert(SOPC_STATUS_OK == SOPC_ToolkitConfig_SetLogLevel(SOPC_TOOLKIT_LOG_LEVEL_DEBUG));

        status = SOPC_ToolkitServer_SetAddressSpaceConfig(&addressSpace);
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
    }

    // Run the server until notification that endpoint is closed or stop server signal
    while (SOPC_STATUS_OK == status && stopServer == 0 && endpointClosed == false)
    {
        SOPC_Sleep(sleepTimeout);
    }

    if (SOPC_STATUS_OK == status && endpointClosed == false)
    {
        // Asynchronous request to close the endpoint
        SOPC_ToolkitServer_AsyncCloseEndpoint(epConfigIdx);
    }

    // Wait until endpoint is closed or stop server signal
    while (SOPC_STATUS_OK == status && stopServer == 0 && endpointClosed == false)
    {
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    // Clear the toolkit configuration and stop toolkit threads
    SOPC_Toolkit_Clear();

    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Toolkit final result: OK\n");
    }
    else
    {
        printf("<Test_Server_Toolkit final result: NOK with status = '%d'\n", status);
    }

    // Deallocate locally allocated data

    SOPC_String_Clear(&secuConfig[0].securityPolicy);
    SOPC_String_Clear(&secuConfig[1].securityPolicy);
    SOPC_String_Clear(&secuConfig[2].securityPolicy);

    if (secuActive != false)
    {
        SOPC_KeyManager_Certificate_Free(serverCertificate);
        SOPC_KeyManager_AsymmetricKey_Free(asymmetricKey);
        SOPC_KeyManager_Certificate_Free(authCertificate);
        SOPC_PKIProviderStack_Free(pkiProvider);
    }

    return status;
}
