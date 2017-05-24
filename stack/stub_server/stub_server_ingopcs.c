/*
 *  Copyright (C) 2016 Systerel and others.
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

#include "stub_server_ingopcs.h"

#include <stdint.h>
#include <stdio.h>

#include "sopc_stack_config.h"
#include "sopc_run.h"
#include "crypto_profiles.h"
#include "key_manager.h"
#include "pki_stack.h"

// indicates if a connection was closed (after being opened)
int connectionClosed = 0;
int connected = 0;

SOPC_StatusCode StubServer_EndpointEvent_Callback(SOPC_Endpoint             endpoint,
                                                  void*                     cbData,
                                                  SOPC_EndpointEvent        event,
                                                  SOPC_StatusCode           status,
                                                  uint32_t                  secureChannelId,
                                                  const Certificate*        clientCertificate,
                                                  const SOPC_String*        securityPolicy,
                                                  OpcUa_MessageSecurityMode securityMode){
    (void) endpoint;
    (void) cbData;
    (void) secureChannelId;
    (void) clientCertificate;
    (void) securityPolicy;
    (void) securityMode;
    char* cevent = NULL;
    switch(event){
        case SOPC_EndpointEvent_Invalid:
            cevent = "SOPC_EndpointEvent_Invalid";
            break;
        case SOPC_EndpointEvent_SecureChannelOpened:
            cevent = "SOPC_EndpointEvent_SecureChannelOpened";
            break;
        case SOPC_EndpointEvent_SecureChannelClosed:
            cevent = "SOPC_EndpointEvent_SecureChannelClosed";
            break;
        case SOPC_EndpointEvent_Renewed:
            cevent = "SOPC_EndpointEvent_Renewed";
            break;
        case SOPC_EndpointEvent_UnsupportedServiceRequested:
            cevent = "SOPC_EndpointEvent_UnsupportedServiceRequested";
            break;
        case SOPC_EndpointEvent_DecoderError:
            cevent = "SOPC_EndpointEvent_Renewed";
            break;
        case SOPC_EndpointEvent_EndpointClosed:
            cevent = "SOPC_EndpointEvent_UnsupportedServiceRequested";
            break;
    }
    printf("<Stub_Server: Endpoint CALLBACK called with event '%s' and status '%x' !\n", cevent, status);
    if (event == SOPC_EndpointEvent_SecureChannelClosed){
    	connectionClosed = 1;
    }
    else if (event == SOPC_EndpointEvent_SecureChannelOpened){
    	// By pass certificate validation for secure channel in any result case
        connected = 1;
    	//return OpcUa_BadContinue;
    	return 0;
    }
	return 0;
}

int main(void){
    SOPC_StatusCode status = STATUS_OK;
    PKIProvider *pki = NULL;

    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 20000;
    // Counter to stop waiting responses after 5 seconds
    uint32_t loopCpt = 0;

    SOPC_Endpoint endpoint = NULL;
    Certificate *crt_srv = NULL;
    AsymmetricKey *priv_srv = NULL;
    Certificate *crt_ca = NULL;

    // Empty callback data
    StubServer_Endpoint Callback_Data;

    // Number of the secu policy configuration: 1
    uint32_t nbOfSecurityPolicyConfigurations = 1;

    // Secu policy configuration: empty
    SOPC_SecurityPolicy secuConfig[1];
    SOPC_String_Initialize(&secuConfig[0].securityPolicy);

    // Endpoint URL
    char* endpointUrl = "opc.tcp://localhost:8888/myEndPoint";

    // Paths to client certificate/key and server certificate
    // Server certificate name
    char* certificateSrvLocation = "./server_public/server.der";
    // Server private key
    char* keyLocation = "./server_private/server.key";

    // The certificates: load
    status = KeyManager_Certificate_CreateFromFile(certificateSrvLocation, &crt_srv);

    if(STATUS_OK != status){
        printf("<Stub_Server: Failed to load certificate\n");
    }else{
        printf("<Stub_Server: Server certificate loaded\n");
    }

    // Private key: load
    if(STATUS_OK == status){
        status = KeyManager_AsymmetricKey_CreateFromFile(keyLocation, &priv_srv, NULL, 0);
        if(STATUS_OK != status){
            printf("<Stub_Server: Failed to load private key\n");
        }else{
            printf("<Stub_Server: Server private key loaded\n");
        }
    }

    // Certificate Authority: load
    if(STATUS_OK == status){
        status = KeyManager_Certificate_CreateFromFile("./trusted/cacert.der", &crt_ca);
        if(STATUS_OK != status){
            printf("<Stub_Server: Failed to load CA\n");
        }else{
            printf("<Stub_Server: CA certificate loaded\n");
        }
    }

    // Init stack configuration
    if(STATUS_OK == status){
        status = StackConfiguration_Initialize();
        if(STATUS_OK != status){
            printf("<Stub_Server: Failed to initialize stack\n");
        }else{
            printf("<Stub_Server: Stack initialized\n");
        }
    }

    if(STATUS_OK == status){
        status = SOPC_String_AttachFromCstring(&secuConfig[0].securityPolicy, SecurityPolicy_Basic256Sha256_URI);
        secuConfig[0].securityModes = SECURITY_MODE_SIGN_MASK | SECURITY_MODE_SIGNANDENCRYPT_MASK;
    }

    if(STATUS_OK == status){
        status = SOPC_Endpoint_Create (&endpoint, SOPC_EndpointSerializer_Binary, NULL); //Services
    }

    // Init PKI provider and parse certificate and private key
    // PKIConfig is just used to create the provider but only configuration of PKIType is useful here (paths not used)
    if(STATUS_OK == status){
        if(STATUS_OK != PKIProviderStack_Create(crt_ca, NULL, &pki)){
            printf("<Stub_Server: Failed to create PKI\n");
        }else{
            printf("<Stub_Server: PKI created\n");
        }
    }

    // Start the server code (connection and default management on secu channel)
    if(STATUS_OK == status){
        status = SOPC_Endpoint_Open(endpoint,                          // Endpoint
                                    endpointUrl,                       // URL
                                    StubServer_EndpointEvent_Callback, // Endpoint Callback
                                    &Callback_Data,                    // Endpoint Callback Data
                                    crt_srv,                           // Server Certificate
                                    priv_srv,                          // Private Key
                                    pki,                               // PKI Config
                                    nbOfSecurityPolicyConfigurations,  // NoOf SecurityPolicies
                                    secuConfig);                       // SecurityPolicies
        if(STATUS_OK != status){
            printf("<Stub_Server: Failed to open the endpoint\n");
        }else{
            printf("<Stub_Server: Opening endpoint with success\n");
        }
    }

    while (STATUS_OK == status && connectionClosed == FALSE && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
#if OPCUA_MULTITHREADED
    	// just wait for callback
    	assert(FALSE);
#else
    	// Retrieve received messages on socket
        status = SOPC_TreatReceivedMessages(sleepTimeout);
#endif //OPCUA_MULTITHREADED

    }
    loopCpt = 0;
    if(STATUS_OK == status){
        if(connected == FALSE){
            status = STATUS_NOK;
            printf("<Stub_Server: No connection established from client\n");
        }else if(connectionClosed != FALSE){
            printf("<Stub_Server: Client connection established and then closed\n");
        }else{
            printf("<Stub_Server: Timeout before client closed connection\n");
        }
    }

    printf ("<Stub_Server: Final status: %x\n", status);
    PKIProviderStack_Free(pki);
    KeyManager_Certificate_Free(crt_srv);
    KeyManager_Certificate_Free(crt_ca);
    KeyManager_AsymmetricKey_Free(priv_srv);
    SOPC_Endpoint_Delete(&endpoint);
    StackConfiguration_Clear();
    if(STATUS_OK == status){
        printf("<Stub_Server: Stub_Server test: OK\n");
    }else{
        printf("<Stub_Server: Stub_Server test: NOK\n");
    }
    if(status != 0){
        return -1;
    }else{

    }
    return status;
}
