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
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_crypto_profiles.h"
#include "sopc_crypto_provider.h"
#include "sopc_pki_stack.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_encodeable.h"
#include "sopc_time.h"

#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"

#include "embedded/loader.h"

#ifdef WITH_EXPAT
#include "uanodeset_expat/loader.h"
#endif

#define ENDPOINT_URL "opc.tcp://localhost:4841"
#define APPLICATION_URI "urn:INGOPCS:localhost"
#define PRODUCT_URI "urn:INGOPCS:localhost"

static int endpointClosed = false;
static bool secuActive = !false;

volatile sig_atomic_t stopServer = 0;

typedef enum {
    AS_LOADER_EMBEDDED,
#ifdef WITH_EXPAT
    AS_LOADER_EXPAT,
#endif
} AddressSpaceLoader;

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

#ifdef WITH_EXPAT
static SOPC_AddressSpace* load_nodeset_from_file(const char* filename)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_AddressSpace_Description* desc = NULL;
    SOPC_AddressSpace* address_space = NULL;
    FILE* fd = fopen(filename, "r");

    if (fd == NULL)
    {
        printf("<Test_Server_Toolkit: Error while opening %s: %s\n", filename, strerror(errno));
        status = SOPC_STATUS_NOK;
    }

    if (status == SOPC_STATUS_OK)
    {
        desc = SOPC_AddressSpace_Description_Create();

        if (desc == NULL)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (status == SOPC_STATUS_OK)
    {
        status = SOPC_UANodeSet_Parse(fd, desc);
    }

    if (status == SOPC_STATUS_OK)
    {
        address_space = calloc(1, sizeof(SOPC_AddressSpace));

        if (address_space == NULL)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    else
    {
        printf("<Test_Server_Toolkit: Error while parsing XML address space\n");
    }

    if (status == SOPC_STATUS_OK)
    {
        status = SOPC_AddressSpace_Generate(desc, address_space);
    }

    SOPC_AddressSpace_Description_Delete(desc);

    if (fd != NULL)
    {
        fclose(fd);
    }

    if (status == SOPC_STATUS_OK)
    {
        printf("<Test_Server_Toolkit: Loaded address space from %s\n", filename);
        return address_space;
    }
    else
    {
        SOPC_AddressSpace_Clear(address_space);
        free(address_space);
        return NULL;
    }
}
#endif

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

    AddressSpaceLoader address_space_loader = AS_LOADER_EMBEDDED;
    SOPC_AddressSpace* address_space = NULL;
    bool address_space_needs_free = false;

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
            address_space = &addressSpace;
            status = SOPC_STATUS_OK;
            break;
#ifdef WITH_EXPAT
        case AS_LOADER_EXPAT:
            address_space = load_nodeset_from_file(xml_file_path);

            if (address_space != NULL)
            {
                status = SOPC_STATUS_OK;
                address_space_needs_free = true;
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
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

    if (address_space_needs_free)
    {
        SOPC_AddressSpace_Clear(address_space);
        free(address_space);
    }

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

    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
