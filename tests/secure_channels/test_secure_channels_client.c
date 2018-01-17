/*
 *  Copyright (C) 2017 Systerel and others.
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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_crypto_profiles.h"
#include "sopc_encoder.h"
#include "sopc_pki_stack.h"
#include "sopc_secure_channels_api.h"
#include "sopc_services_api.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_constants.h"
#include "sopc_types.h"
#include "stub_sc_sopc_services_api.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

#include "sopc_time.h"

/*
 * Expected arguments (based on arguments order, last arguments can be unused (default value used)):
 * 1) Security mode (default encrypt): "none", "sign" or "encrypt"
 * 2) Security policy  (default Basic256Sha256 or None if Security mode == None): "B256", "B256Sha256"
 * 3) Client (and Server default) key size(s) (default 2048 for both): "2048" or "4096"
 * 4) Server key size (default based on precedent argument):  "2048" or "4096"
 * */
int main(int argc, char* argv[])
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 10000;
    // Counter to stop waiting responses after 5 seconds
    uint32_t loopCpt = 0;

    SOPC_SecureChannel_Config scConfig;
    uint32_t scConfigIdx = 0;
    SOPC_StubSC_ServicesEventParams* serviceEvent = NULL;
    uint32_t scConnectionId = 0;

    SOPC_PKIProvider* pki = NULL;
    SOPC_Certificate *crt_cli = NULL, *crt_srv = NULL;
    SOPC_Certificate* crt_ca = NULL;
    SOPC_AsymmetricKey* priv_cli = NULL;

    // Endpoint URL
    SOPC_String stEndpointUrl;
    SOPC_String_Initialize(&stEndpointUrl);
    char* sEndpointUrl = "opc.tcp://localhost:8888/myEndPoint";

    // Policy security:
    char* pRequestedSecurityPolicyUri = SOPC_SecurityPolicy_Basic256Sha256_URI;

    // Message security mode: None
    OpcUa_MessageSecurityMode messageSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;

    // Manage change of security policy mode (none, sign or encrypt)
    if (argc >= 2)
    {
        if (strlen(argv[1]) == strlen("none") && 0 == memcmp(argv[1], "none", strlen("none")))
        {
            messageSecurityMode = OpcUa_MessageSecurityMode_None;
            pRequestedSecurityPolicyUri = SOPC_SecurityPolicy_None_URI;
            printf(">>Stub_Client: Security mode ='None'\n");
        }
        else if (strlen(argv[1]) == strlen("sign") && 0 == memcmp(argv[1], "sign", strlen("sign")))
        {
            messageSecurityMode = OpcUa_MessageSecurityMode_Sign;
            printf(">>Stub_Client: Security mode ='Sign'\n");
        }
        else if (strlen(argv[1]) == strlen("encrypt") && 0 == memcmp(argv[1], "encrypt", strlen("encrypt")))
        {
            printf(">>Stub_Client: Security mode ='SignAndEncrypt'\n");
        }
    }
    else
    {
        printf(">>Stub_Client: Security mode ='SignAndEncrypt'\n");
    }

    // Manage security policy URI (B256 or B256Sha256)
    if (argc >= 3)
    {
        if (strlen(argv[2]) == strlen("B256") && 0 == memcmp(argv[2], "B256", strlen("B256")))
        {
            pRequestedSecurityPolicyUri = SOPC_SecurityPolicy_Basic256_URI;
        }
        else if (strlen(argv[2]) == strlen("B256Sha256") && 0 == memcmp(argv[2], "B256Sha256", strlen("B256Sha256")))
        {
            pRequestedSecurityPolicyUri = SOPC_SecurityPolicy_Basic256Sha256_URI;
        }
        else
        {
            printf(">>Stub_Client: Error invalid 2nd argument'\n");
            status = SOPC_STATUS_NOK;
        }
    }
    printf(">>Stub_Client: Security policy ='%s'\n", pRequestedSecurityPolicyUri);

    // Paths to client certificate/key and server certificate
    // Client certificate name
    char* certificateLocation = "./client_public/client_2k.der";
    // Server certificate name
    char* certificateSrvLocation = "./server_public/server_2k.der";
    // Client private key
    char* keyLocation = "./client_private/client_2k.key";

    // Manage key length used for the client (and default for server) 2048 or 4096
    if (argc >= 4)
    {
        if (strlen(argv[3]) == strlen("2048") && 0 == memcmp(argv[3], "2048", strlen("2048")))
        {
            certificateLocation = "./client_public/client_2k.der";
            certificateSrvLocation = "./server_public/server_2k.der";
            keyLocation = "./client_private/client_2k.key";
        }
        else if (strlen(argv[3]) == strlen("4096") && 0 == memcmp(argv[3], "4096", strlen("4096")))
        {
            certificateLocation = "./client_public/client_4k.der";
            certificateSrvLocation = "./server_public/server_4k.der";
            keyLocation = "./client_private/client_4k.key";
        }
        else
        {
            printf(">>Stub_Client: Error invalid 3rd argument'\n");
            status = SOPC_STATUS_NOK;
        }
    }

    // Manage key length used for the client (and default for server) 2048 or 4096
    if (argc >= 5)
    {
        if (strlen(argv[4]) == strlen("2048") && 0 == memcmp(argv[4], "2048", strlen("2048")))
        {
            certificateSrvLocation = "./server_public/server_2k.der";
        }
        else if (strlen(argv[4]) == strlen("4096") && 0 == memcmp(argv[4], "4096", strlen("4096")))
        {
            certificateSrvLocation = "./server_public/server_4k.der";
        }
        else
        {
            printf(">>Stub_Client: Error invalid 4th argument'\n");
            status = SOPC_STATUS_NOK;
        }
    }

    if (messageSecurityMode != OpcUa_MessageSecurityMode_None && SOPC_STATUS_OK == status)
    {
        printf(">>Stub_Client: used paths for keys and certificates ='%s', '%s' and '%s'\n", keyLocation,
               certificateLocation, certificateSrvLocation);

        // The certificates: load
        status = SOPC_KeyManager_Certificate_CreateFromFile(certificateLocation, &crt_cli);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed to load client certificate\n");
        }
        else
        {
            printf(">>Stub_Client: Client certificate loaded\n");
        }
    }

    if (messageSecurityMode != OpcUa_MessageSecurityMode_None && SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_Certificate_CreateFromFile(certificateSrvLocation, &crt_srv);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed to load server certificate\n");
        }
        else
        {
            printf(">>Stub_Client: Server certificate loaded\n");
        }
    }

    if (messageSecurityMode != OpcUa_MessageSecurityMode_None && SOPC_STATUS_OK == status)
    {
        // Private key: load
        status = SOPC_KeyManager_AsymmetricKey_CreateFromFile(keyLocation, &priv_cli, NULL, 0);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed to load private key\n");
        }
        else
        {
            printf(">>Stub_Client: Client private key loaded\n");
        }
    }

    if (messageSecurityMode != OpcUa_MessageSecurityMode_None && SOPC_STATUS_OK == status)
    {
        // Certificate Authority: load
        status = SOPC_KeyManager_Certificate_CreateFromFile("./trusted/cacert.der", &crt_ca);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed to load CA\n");
        }
        else
        {
            printf(">>Stub_Client: CA certificate loaded\n");
        }
    }

    // Init PKI provider and parse certificate and private key
    // PKIConfig is just used to create the provider but only configuration of PKIType is useful here (paths not used)
    if (messageSecurityMode != OpcUa_MessageSecurityMode_None && SOPC_STATUS_OK == status)
    {
        status = SOPC_PKIProviderStack_Create(crt_ca, NULL, &pki);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed to create PKI\n");
        }
        else
        {
            printf(">>Stub_Client: PKI created\n");
        }
    }

    // Init toolkit configuration
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Toolkit_Initialize(NULL);
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: Failed initializing toolkit\n");
        }
        else
        {
            printf(">>Stub_Client: Toolkit initialized\n");
        }
    }

    // Start connection to server
    if (SOPC_STATUS_OK == status)
    {
        scConfig.isClientSc = true;
        scConfig.msgSecurityMode = messageSecurityMode;
        scConfig.reqSecuPolicyUri = pRequestedSecurityPolicyUri;
        scConfig.crt_cli = crt_cli;
        scConfig.crt_srv = crt_srv;
        scConfig.key_priv_cli = priv_cli;
        scConfig.pki = pki;
        scConfig.requestedLifetime = 100000;
        scConfig.url = sEndpointUrl;

        scConfigIdx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
        assert(scConfigIdx != 0);
        assert(SOPC_STATUS_OK == SOPC_Toolkit_Configured());

        SOPC_SecureChannels_EnqueueEvent(SC_CONNECT, scConfigIdx, NULL, 0);
        printf(">>Stub_Client: Establishing connection to server...\n");
    }

    while ((SOPC_STATUS_OK == status || SOPC_STATUS_WOULD_BLOCK == status) && serviceEvent == NULL &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        status = SOPC_AsyncQueue_NonBlockingDequeue(servicesEvents, (void**) &serviceEvent);
        if (SOPC_STATUS_OK != status)
        {
            loopCpt++;
            SOPC_Sleep(sleepTimeout);
        }
    }

    if (SOPC_STATUS_OK != status && loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (SOPC_STATUS_OK == status && serviceEvent == NULL)
    {
        status = SOPC_STATUS_NOK;
    }
    loopCpt = 0;

    if (SOPC_STATUS_OK == status)
    {
        if (serviceEvent->event == SC_TO_SE_SC_CONNECTED && serviceEvent->auxParam == scConfigIdx)
        {
            scConnectionId = serviceEvent->eltId;
            printf(">>Stub_Client: Connection to server established\n");
        }
        else
        {
            printf(">>Stub_Client: Unexpected event received '%d'\n", serviceEvent->event);
            status = SOPC_STATUS_CLOSED;
        }
        free(serviceEvent);
        serviceEvent = NULL;
    }
    if (SOPC_STATUS_OK != status)
    {
        printf(">>Stub_Client: Failed to establish connection to server\n");
    }

    // Request
    OpcUa_RequestHeader rHeader;
    OpcUa_GetEndpointsRequest cRequest;

    if (SOPC_STATUS_OK == status)
    {
        // Endpoint URL in OPC UA string format
        status = SOPC_String_AttachFromCstring(&stEndpointUrl, sEndpointUrl);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Initialization of empty request
        OpcUa_RequestHeader_Initialize(&rHeader);
        OpcUa_GetEndpointsRequest_Initialize(&cRequest);

        cRequest.EndpointUrl = stEndpointUrl;
        cRequest.NoOfLocaleIds = 0;
        cRequest.LocaleIds = NULL;
        cRequest.NoOfProfileUris = 0;
        cRequest.ProfileUris = NULL;

        SOPC_Buffer* buffer = SOPC_Buffer_Create(SOPC_MAX_MESSAGE_LENGTH);
        assert(NULL != buffer);
        status = SOPC_Buffer_SetDataLength(buffer, SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH +
                                                       SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH +
                                                       SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
        assert(SOPC_STATUS_OK == status);
        status = SOPC_Buffer_SetPosition(buffer, SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH +
                                                     SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH +
                                                     SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
        assert(SOPC_STATUS_OK == status);
        status =
            SOPC_EncodeMsg_Type_Header_Body(buffer, &OpcUa_GetEndpointsRequest_EncodeableType,
                                            &OpcUa_RequestHeader_EncodeableType, (void*) &rHeader, (void*) &cRequest);
        assert(SOPC_STATUS_OK == status);

        SOPC_SecureChannels_EnqueueEvent(SC_SERVICE_SND_MSG, scConnectionId, (void*) buffer, 0);

        printf(">>Stub_Client: Calling GetEndpoint service...\n");
    }

    while ((SOPC_STATUS_OK == status || SOPC_STATUS_WOULD_BLOCK == status) && serviceEvent == NULL &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        status = SOPC_AsyncQueue_NonBlockingDequeue(servicesEvents, (void**) &serviceEvent);
        if (SOPC_STATUS_OK != status)
        {
            loopCpt++;
            SOPC_Sleep(sleepTimeout);
        }
    }

    if (SOPC_STATUS_OK != status && loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (SOPC_STATUS_OK == status && serviceEvent == NULL)
    {
        status = SOPC_STATUS_NOK;
    }
    loopCpt = 0;

    if (SOPC_STATUS_OK == status)
    {
        if (serviceEvent->event == SC_TO_SE_SC_SERVICE_RCV_MSG)
        {
            if (serviceEvent->eltId == scConnectionId && serviceEvent->params != NULL && serviceEvent->auxParam == 0)
            {
                SOPC_EncodeableType* encType = NULL;
                SOPC_MsgBodyType_Read((SOPC_Buffer*) serviceEvent->params, &encType);
                SOPC_Buffer_Delete((SOPC_Buffer*) serviceEvent->params);
                serviceEvent->params = NULL;

                if (encType == &OpcUa_ServiceFault_EncodeableType)
                {
                    printf(">>Stub_Client: GetEndpoint service call failed with a service fault => OK\n");
                }
                else if (encType == &OpcUa_GetEndpointsResponse_EncodeableType)
                {
                    printf(">>Stub_Client: GetEndpoint service call succeeded => OK\n");
                }
                else
                {
                    status = SOPC_STATUS_NOT_SUPPORTED;
                }
            }
            else
            {
                printf(">>Stub_Client: Unexpected service received message parameters values\n");
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            printf(">>Stub_Client: Unexpected event received '%d'\n", serviceEvent->event);
            status = SOPC_STATUS_NOK;
        }
        free(serviceEvent);
        serviceEvent = NULL;
    }
    if (SOPC_STATUS_OK != status)
    {
        printf(">>Stub_Client: GetEndpoint service call failed\n");
    }

    // CLOSE THE SECURE CHANNEL
    if (SOPC_STATUS_OK == status)
    {
        SOPC_SecureChannels_EnqueueEvent(SC_DISCONNECT, scConnectionId, NULL, 0);
        printf(">>Stub_Client: Closing secure connection\n");
    }

    while ((SOPC_STATUS_OK == status || SOPC_STATUS_WOULD_BLOCK == status) && serviceEvent == NULL &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        status = SOPC_AsyncQueue_NonBlockingDequeue(servicesEvents, (void**) &serviceEvent);
        if (SOPC_STATUS_OK != status)
        {
            loopCpt++;
            SOPC_Sleep(sleepTimeout);
        }
    }

    if (SOPC_STATUS_OK != status && loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }
    else if (SOPC_STATUS_OK == status && serviceEvent == NULL)
    {
        status = SOPC_STATUS_NOK;
    }
    loopCpt = 0;

    if (SOPC_STATUS_OK == status)
    {
        if (serviceEvent->event == SC_TO_SE_SC_DISCONNECTED && serviceEvent->eltId == scConnectionId)
        {
            printf(">>Stub_Client: secure connection closed => OK \n");
        }
        else
        {
            printf(">>Stub_Client: Unexpected event received '%d'\n", serviceEvent->event);
            status = SOPC_STATUS_CLOSED;
        }
        free(serviceEvent);
        serviceEvent = NULL;
        if (SOPC_STATUS_OK != status)
        {
            printf(">>Stub_Client: close secure connection failed\n");
        }
    }

    printf(">>Stub_Client: Final status: %x\n", status);
    SOPC_Toolkit_Clear();

    SOPC_PKIProviderStack_Free(pki);
    SOPC_String_Clear(&stEndpointUrl);
    SOPC_KeyManager_Certificate_Free(crt_cli);
    SOPC_KeyManager_Certificate_Free(crt_srv);
    SOPC_KeyManager_Certificate_Free(crt_ca);
    SOPC_KeyManager_AsymmetricKey_Free(priv_cli);

    if (SOPC_STATUS_OK == status)
    {
        printf(">>Stub_Client: Stub_Client test: OK\n");
    }
    else
    {
        printf(">>Stub_Client: Stub_Client test: NOK\n");
    }
    if (status != 0)
    {
        return -1;
    }
    return status;
}
