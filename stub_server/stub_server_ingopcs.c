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

SOPC_StatusCode StubServer_EndpointEvent_Callback(SOPC_Endpoint      endpoint,
                                                  void*              cbData,
                                                  SOPC_EndpointEvent event,
                                                  SOPC_StatusCode    status,
                                                  uint32_t           secureChannelId,
                                                  const Certificate* clientCertificate,
                                                  const SOPC_String* securityPolicy){
    (void) endpoint;
    (void) cbData;
    (void) secureChannelId;
    (void) clientCertificate;
    (void) securityPolicy;
    printf ("\nEndpoint CALLBACK called with event %d and status %x !\n", event, status);
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

    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 20000;
    // Counter to stop waiting responses after 5 seconds
    uint32_t loopCpt = 0;

    SOPC_Endpoint endpoint;
    // Endpoint URL
    char* endpointUrl = "opc.tcp://localhost:8888/myEndPoint";

    // Paths to client certificate/key and server certificate
    // Server certificate name
    char* certificateSrvLocation = "./server_public/server.der";
    // Server private key
    char* keyLocation = "./server_private/server.key";

    // The certificates: load
    Certificate *crt_srv = NULL;
    status = KeyManager_Certificate_CreateFromFile(certificateSrvLocation, &crt_srv);

    if(STATUS_OK != status)
        printf("Failed to load certificate(s)\n");

    // Private key: load
    AsymmetricKey *priv_srv = NULL;
    status = KeyManager_AsymmetricKey_CreateFromFile(keyLocation, &priv_srv, NULL, 0);
    if(STATUS_OK != status)
        printf("Failed to load private key\n");

    // Certificate Authority: load
    Certificate *crt_ca = NULL;
    if(STATUS_OK != KeyManager_Certificate_CreateFromFile("./trusted/cacert.der", &crt_ca))
        printf("Failed to load CA\n");

    // Init stack configuration
    status = StackConfiguration_Initialize();
    if(STATUS_OK != status) goto Error;

    // Empty callback data
    StubServer_Endpoint Callback_Data;

    // Number of the secu policy configuration: 1
    uint32_t nbOfSecurityPolicyConfigurations = 1;

    // Secu policy configuration: empty
    SOPC_SecurityPolicy secuConfig[1];
    SOPC_String_Initialize(&secuConfig[0].securityPolicy);
    status = SOPC_String_AttachFromCstring(&secuConfig[0].securityPolicy, SecurityPolicy_Basic256Sha256_URI);
    if(STATUS_OK != status) goto Error;
    secuConfig[0].securityModes = SECURITY_MODE_SIGN_MASK | SECURITY_MODE_SIGNANDENCRYPT_MASK;

    status = SOPC_Endpoint_Create (&endpoint, SOPC_EndpointSerializer_Binary, NULL); //Services
    if(STATUS_OK != status) goto Error;

    printf ("%d\n", status);

    // Init PKI provider and parse certificate and private key
    // PKIConfig is just used to create the provider but only configuration of PKIType is useful here (paths not used)
    PKIProvider *pki = NULL;
    if(STATUS_OK != PKIProviderStack_Create(crt_ca, NULL, &pki))
        printf("Failed to create PKI\n");

    // Start the server code (connection and default management on secu channel)
    status = SOPC_Endpoint_Open(endpoint,                          // Endpoint
                                endpointUrl,                       // URL
                                StubServer_EndpointEvent_Callback, // Endpoint Callback
                                &Callback_Data,                    // Endpoint Callback Data
                                crt_srv,                           // Server Certificate
                                priv_srv,                          // Private Key
                                pki,                               // PKI Config
                                nbOfSecurityPolicyConfigurations,  // NoOf SecurityPolicies
	   						    secuConfig);                       // SecurityPolicies
    printf ("%d\n", status);
    if(STATUS_OK != status) goto Error;

    while (connectionClosed == FALSE && STATUS_OK == status && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
#if OPCUA_MULTITHREADED
    	// just wait for callback
    	assert(FALSE);
#else
    	// Retrieve received messages on socket
        status = SOPC_TreatReceivedMessages(sleepTimeout);
        printf ("ServerLoop status: %d\n", status);
        if(STATUS_OK != status) goto Error;
#endif //OPCUA_MULTITHREADED

    }
    loopCpt = 0;
    if(connected == FALSE){
        status = STATUS_NOK;
        goto Error;
    }

    printf ("Final status: %d\n", status);
    PKIProviderStack_Free(pki);
    KeyManager_Certificate_Free(crt_srv);
    KeyManager_Certificate_Free(crt_ca);
    KeyManager_AsymmetricKey_Free(priv_srv);
    SOPC_Endpoint_Delete(&endpoint);
    StackConfiguration_Clear();

    return status;

    Error:
    printf ("Error status: %d\n", status);
    PKIProviderStack_Free(pki);
    KeyManager_Certificate_Free(crt_srv);
    KeyManager_Certificate_Free(crt_ca);
    KeyManager_AsymmetricKey_Free(priv_srv);
    SOPC_Endpoint_Delete(&endpoint);
    StackConfiguration_Clear();
    if(status != 0){
        return -1;
    }
}
