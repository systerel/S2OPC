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
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "event_helpers.h"
#include "event_recorder.h"

#include "opcua_statuscodes.h"
#include "sopc_async_queue.h"
#include "sopc_common.h"
#include "sopc_crypto_profiles.h"
#include "sopc_encoder.h"
#include "sopc_key_manager.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_protocol_constants.h"
#include "sopc_secure_channels_api.h"
#include "sopc_services_api.h"
#include "sopc_time.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_types.h"

static bool cryptoDeactivated = false;

#define NB_SECU_POLICY_CONFIGS 3

int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_PKIProvider* pki = NULL;

    SOPC_SerializedCertificate* crt_srv = NULL;
    SOPC_SerializedAsymmetricKey* priv_srv = NULL;

    // Services side stub code
    SOPC_Server_Config sConfig;
    memset(&sConfig, 0, sizeof(sConfig));
    sConfig.nbEndpoints = 1;
    SOPC_Endpoint_Config epConfig;
    memset(&epConfig, 0, sizeof(epConfig));
    epConfig.serverConfigPtr = &sConfig;
    sConfig.endpoints = &epConfig;
    uint32_t epConfigIdx = 0;
    uint32_t scConfigIdx = 0;
    uint32_t scConnectionId = 0;
    SOPC_Event* serviceEvent = NULL;
    SOPC_EventRecorder* servicesEvents = NULL;

    // Endpoint URL
    char* endpointUrl = "opc.tcp://localhost:4841/myEndPoint";

    // Paths to client certificate/key and server certificate
    // Server certificate name
    char* certificateSrvLocation = "./server_public/server_2k_cert.der";
    // Server private key
    char* keyLocation = "./server_private/server_2k_key.pem";

    if (argc >= 2)
    {
        if (strlen(argv[1]) == strlen("2048") && 0 == memcmp(argv[1], "2048", strlen("2048")))
        {
            certificateSrvLocation = "./server_public/server_2k_cert.der";
            keyLocation = "./server_private/server_2k_key.pem";
        }
        else if (strlen(argv[1]) == strlen("4096") && 0 == memcmp(argv[1], "4096", strlen("4096")))
        {
            certificateSrvLocation = "./server_public/server_4k_cert.der";
            keyLocation = "./server_private/server_4k_key.pem";
        }
        else
        {
            printf("<Stub_Server: Error invalid 1st argument'\n");
            status = SOPC_STATUS_NOK;
        }
    }
    printf("<Stub_Server: used paths for keys and certificates = '%s' and '%s'\n", keyLocation, certificateSrvLocation);

    // The certificates: load
    if (SOPC_STATUS_OK == status && cryptoDeactivated == false)
    {
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(certificateSrvLocation, &crt_srv);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Stub_Server: Failed to load certificate\n");
        }
        else
        {
            printf("<Stub_Server: Server certificate loaded\n");
        }
    }

    // Private key: load
    if (SOPC_STATUS_OK == status && cryptoDeactivated == false)
    {
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(keyLocation, &priv_srv);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Stub_Server: Failed to load private key\n");
        }
        else
        {
            printf("<Stub_Server: Server private key loaded\n");
        }
    }

    epConfig.authenticationManager = NULL;
    epConfig.authorizationManager = NULL;

    // Initialize SOPC_Common
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./test_secure_channel_server_logs/";
        logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
        SOPC_Common_Initialize(logConfiguration);
    }

    // Init toolkit configuration
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(NULL);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Stub_Server: Failed to initialize toolkit\n");
        }
        else
        {
            printf("<Stub_Server: Toolkit initialized\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        servicesEvents = SOPC_EventRecorder_Create();

        status = (servicesEvents != NULL) ? SOPC_STATUS_OK : SOPC_STATUS_OUT_OF_MEMORY;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_SecureChannels_SetEventHandler(servicesEvents->eventHandler);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_String_Initialize(&epConfig.secuConfigurations[0].securityPolicy);
        status = SOPC_String_AttachFromCstring(&epConfig.secuConfigurations[0].securityPolicy,
                                               SOPC_SecurityPolicy_Basic256_URI);
        epConfig.secuConfigurations[0].securityModes =
            SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;

        SOPC_String_Initialize(&epConfig.secuConfigurations[1].securityPolicy);
        status = SOPC_String_AttachFromCstring(&epConfig.secuConfigurations[1].securityPolicy,
                                               SOPC_SecurityPolicy_Basic256Sha256_URI);
        epConfig.secuConfigurations[1].securityModes =
            SOPC_SECURITY_MODE_SIGN_MASK | SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK;

        SOPC_String_Initialize(&epConfig.secuConfigurations[2].securityPolicy);
        status =
            SOPC_String_AttachFromCstring(&epConfig.secuConfigurations[2].securityPolicy, SOPC_SecurityPolicy_None_URI);
        epConfig.secuConfigurations[2].securityModes = SOPC_SECURITY_MODE_NONE_MASK;
    }

    // Init PKI provider and parse certificate and private key
    // PKIConfig is just used to create the provider but only configuration of PKIType is useful here (paths not used)
    if (SOPC_STATUS_OK == status && cryptoDeactivated == false)
    {
        char* lPathsTrustedRoots[] = {"./trusted/cacert.der", NULL};
        char* lPathsTrustedLinks[] = {NULL};
        char* lPathsUntrustedRoots[] = {NULL};
        char* lPathsUntrustedLinks[] = {NULL};
        char* lPathsIssuedCerts[] = {NULL};
        char* lPathsCRL[] = {"./revoked/cacrl.der", NULL};
        status = SOPC_PKIProviderStack_CreateFromPaths(lPathsTrustedRoots, lPathsTrustedLinks, lPathsUntrustedRoots,
                                                       lPathsUntrustedLinks, lPathsIssuedCerts, lPathsCRL, &pki);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Stub_Server: Failed to create PKI\n");
        }
        else
        {
            printf("<Stub_Server: PKI created\n");
        }
    }

    // Start the server code (connection and default management on secu channel)
    if (SOPC_STATUS_OK == status)
    {
        epConfig.endpointURL = endpointUrl;
        epConfig.hasDiscoveryEndpoint = true;
        epConfig.nbSecuConfigs = NB_SECU_POLICY_CONFIGS;
        epConfig.serverConfigPtr->serverCertificate = crt_srv;
        epConfig.serverConfigPtr->serverKey = priv_srv;
        epConfig.serverConfigPtr->pki = pki;

        epConfigIdx = SOPC_ToolkitServer_AddEndpointConfig(&epConfig);

        assert(epConfigIdx != 0);

        status = SOPC_ToolkitServer_Configured();
        assert(status == SOPC_STATUS_OK);

        SOPC_SecureChannels_EnqueueEvent(EP_OPEN, epConfigIdx, (uintptr_t) NULL, 0);

        printf("<Stub_Server: Opening endpoint connection listener ...\n");

        WaitEvent(servicesEvents->events, (void**) &serviceEvent);

        if (serviceEvent->event == EP_CONNECTED)
        {
            if (serviceEvent->eltId == epConfigIdx && (uintptr_t) serviceEvent->params != 0 && // SC config index
                serviceEvent->auxParam != 0)
            { // SC connection index

                SOPC_UNUSED_ARG(scConfigIdx);

                scConfigIdx = (uint32_t)(uintptr_t) serviceEvent->params;
                scConnectionId = (uint32_t) serviceEvent->auxParam;
                printf("<Stub_Server: Connection established from a client\n");
            }
            else
            {
                printf("<Stub_Server: Unexpected client connection parameters values\n");
                status = SOPC_STATUS_NOK;
            }
            SOPC_Free(serviceEvent);
            serviceEvent = NULL;
        }
        else
        {
            printf("<Stub_Server: Unexpected event received '%d'\n", serviceEvent->event);
            status = SOPC_STATUS_NOK;
        }

        // Wait for a service level message reception
        WaitEvent(servicesEvents->events, (void**) &serviceEvent);

        if (serviceEvent->event == SC_SERVICE_RCV_MSG)
        {
            if (serviceEvent->eltId == scConnectionId && (void*) serviceEvent->params != NULL &&
                serviceEvent->auxParam != 0) // request context
            {
                SOPC_EncodeableType* encType = NULL;
                status = SOPC_MsgBodyType_Read((SOPC_Buffer*) serviceEvent->params, &encType);
                assert(status == SOPC_STATUS_OK);
                SOPC_Buffer_Delete((SOPC_Buffer*) serviceEvent->params);
                serviceEvent->params = (uintptr_t) NULL;
                if (encType == &OpcUa_GetEndpointsRequest_EncodeableType)
                {
                    printf("<Stub_Server: Received a GetEndpoint service request => OK\n");

                    OpcUa_ResponseHeader rHeader;
                    OpcUa_ResponseHeader_Initialize(&rHeader);

                    SOPC_Buffer* buffer = SOPC_Buffer_Create(SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH);
                    assert(NULL != buffer);
                    status = SOPC_Buffer_SetDataLength(buffer, SOPC_UA_SYMMETRIC_SECURE_MESSAGE_HEADERS_LENGTH);
                    assert(SOPC_STATUS_OK == status);
                    status = SOPC_Buffer_SetPosition(buffer, SOPC_UA_SYMMETRIC_SECURE_MESSAGE_HEADERS_LENGTH);
                    assert(SOPC_STATUS_OK == status);
                    status =
                        SOPC_EncodeMsg_Type_Header_Body(buffer, &OpcUa_ServiceFault_EncodeableType,
                                                        &OpcUa_ResponseHeader_EncodeableType, (void*) &rHeader, NULL);
                    assert(SOPC_STATUS_OK == status);

                    SOPC_SecureChannels_EnqueueEvent(SC_SERVICE_SND_MSG, scConnectionId, (uintptr_t) buffer,
                                                     serviceEvent->auxParam); // request context
                    printf("<Stub_Server: Responding with a service fault message \n");
                }
                else
                {
                    printf("<Stub_Server: Received an unexpected service request => NOK\n");
                    status = SOPC_STATUS_NOK;
                }
            }
            else
            {
                printf("<Stub_Server: Unexpected service received message parameters values\n");
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            printf("<Stub_Server: Unexpected event received '%d'\n", serviceEvent->event);
            status = SOPC_STATUS_NOK;
        }
        SOPC_Free(serviceEvent);
        serviceEvent = NULL;

        // Wait for client disconnection
        WaitEvent(servicesEvents->events, (void**) &serviceEvent);

        if (serviceEvent->event == SC_DISCONNECTED)
        {
            if (serviceEvent->eltId == scConnectionId)
            {
                printf("<Stub_Server: Secure connection disconnected with status %" PRIx64 "\n",
                       (uint64_t) serviceEvent->auxParam);
            }
            else
            {
                printf("<Stub_Server: Unexpected secure disconnection parameters values\n");
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            printf("<Stub_Server: Unexpected event received '%d'\n", serviceEvent->event);
            status = SOPC_STATUS_NOK;
        }
        SOPC_Free(serviceEvent);
        serviceEvent = NULL;

        // Close the endpoint connection listener
        SOPC_SecureChannels_EnqueueEvent(EP_CLOSE, epConfigIdx, (uintptr_t) NULL, 0);

        WaitEvent(servicesEvents->events, (void**) &serviceEvent);

        if (serviceEvent->event == EP_CLOSED)
        {
            if (serviceEvent->eltId == epConfigIdx)
            {
                printf("<Stub_Server: Secure listener closed\n");
            }
            else
            {
                printf("<Stub_Server: Unexpected secure listener closed parameters values\n");
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            printf("<Stub_Server: Unexpected event received '%d'\n", serviceEvent->event);
            status = SOPC_STATUS_NOK;
        }

        SOPC_Free(serviceEvent);
        serviceEvent = NULL;
    }

    printf("<Stub_Server: Final status: %" PRIu32 "\n", status);
    SOPC_Toolkit_Clear();
    SOPC_PKIProvider_Free(&pki);
    SOPC_KeyManager_SerializedCertificate_Delete(crt_srv);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(priv_srv);
    SOPC_SecureChannels_SetEventHandler(NULL);
    SOPC_EventRecorder_Delete(servicesEvents);
    if (SOPC_STATUS_OK == status)
    {
        printf("<Stub_Server: Stub_Server test: OK\n");
        return 0;
    }
    else
    {
        printf("<Stub_Server: Stub_Server test: NOK\n");
        return 1;
    }
}
