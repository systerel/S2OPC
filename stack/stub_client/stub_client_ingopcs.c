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

#include "stub_client_ingopcs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "opcua_identifiers.h"
#include "sopc_stack_config.h"
#include "sopc_time.h"
#include "crypto_profiles.h"
#include "pki_stack.h"
#include "sopc_clientapi.h"

int noEvent = 1;
int noResp = 1;
int disconnect = 0;
int connected = 0;

SOPC_EncodeableType* newEncType = NULL;

SOPC_StatusCode StubClient_ConnectionEvent_Callback(SOPC_Channel       channel,
                                                    void*              callbackData,
                                                    SOPC_Channel_Event event,
                                                    SOPC_StatusCode    status)
{
    (void) callbackData;
    (void) channel;
    char* cevent = NULL;
    switch(event){
        case SOPC_ChannelEvent_Invalid:
            cevent = "SOPC_ChannelEvent_Invalid";
            break;
        case SOPC_ChannelEvent_Connected:
            cevent = "SOPC_ChannelEvent_Connected";
            break;
        case SOPC_ChannelEvent_Disconnected:
            cevent = "SOPC_ChannelEvent_Disconnected";
            break;
    }
    printf(">>Stub_Client: Connection event: channel event '%s' and status '%x'\n", cevent, status);
	if (status == STATUS_OK){
	    connected = 1;
		noEvent = 0;
	}
	if (event == SOPC_ChannelEvent_Disconnected){
		disconnect = 1;
        noEvent = 0;
	}
    return 0;
}

SOPC_StatusCode StubClient_ResponseEvent_Callback(SOPC_Channel         channel,
                                                  void*                response,
                                                  SOPC_EncodeableType* responseType,
                                                  void*                callbackData,
                                                  SOPC_StatusCode      status){
    (void) callbackData;
    (void) channel;
    (void) status;
	noResp = 0;

    /* check for fault */
    if (responseType->TypeId == OpcUaId_ServiceFault)
    {
    	printf(">>Stub_Client: Service fault response: OK (not implemented in server)\n");
    }

    /* check response type */
    else if (OpcUa_GetEndpointsResponse_EncodeableType.TypeId != responseType->TypeId)
    {
    	printf(">>Stub_Client: Incorrect response type !\n");
    }

    /* copy parameters from response object into return parameters. */
    else
    {
    	printf(">>Stub_Client: Endpoints response: %d\n",((OpcUa_GetEndpointsResponse*) response)->NoOfEndpoints);
    }

    // Free the allocated response message
    free(response);

    return 0;
}

int main(int argc, char *argv[]){
    SOPC_StatusCode status = STATUS_OK;

    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 10000;
    // Counter to stop waiting responses after 5 seconds
    uint32_t loopCpt = 0;

    SOPC_Channel hChannel = NULL;
    PKIProvider *pki = NULL;
    Certificate *crt_cli = NULL, *crt_srv = NULL;
    Certificate *crt_ca = NULL;
    AsymmetricKey *priv_cli = NULL;

    // Empty callback data
    StubClient_CallbackData Callback_Data;

    // Endpoint URL
    SOPC_String stEndpointUrl;
    SOPC_String_Initialize(&stEndpointUrl);
    char* sEndpointUrl = "opc.tcp://localhost:8888/myEndPoint";

    // Policy security:
    char* pRequestedSecurityPolicyUri = SecurityPolicy_Basic256Sha256_URI;

    // Message security mode: None
    OpcUa_MessageSecurityMode messageSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;

    // Manage change of security policy and mode (none = None, None | sign = Basic256Sha256)
    if(argc == 2){
        if(strlen(argv[1]) == strlen("none") && 0 == memcmp(argv[1], "none", strlen("none"))){
            messageSecurityMode = OpcUa_MessageSecurityMode_None;
            pRequestedSecurityPolicyUri = SecurityPolicy_None_URI;
            printf(">>Stub_Client: Security mode ='None'\n");
        }else if(strlen(argv[1]) == strlen("sign") && 0 == memcmp(argv[1], "sign", strlen("sign"))){
            messageSecurityMode = OpcUa_MessageSecurityMode_Sign;
            printf(">>Stub_Client: Security mode ='Sign'\n");
        }else{
            printf(">>Stub_Client: Security mode ='SignAndEncrypt'\n");
        }
    }else{
        printf(">>Stub_Client: Security mode ='SignAndEncrypt'\n");
    }
    printf(">>Stub_Client: Security policy ='%s'\n", pRequestedSecurityPolicyUri);

    // Paths to client certificate/key and server certificate
    // Client certificate name
    char* certificateLocation = "./client_public/client.der";
    // Server certificate name
    char* certificateSrvLocation = "./server_public/server.der";
    // Client private key
    char* keyLocation = "./client_private/client.key";

    // The certificates: load
    status = KeyManager_Certificate_CreateFromFile(certificateLocation, &crt_cli);
    if(STATUS_OK != status){
        printf(">>Stub_Client: Failed to load client certificate\n");
    }else{
        printf(">>Stub_Client: Client certificate loaded\n");
    }

    if(STATUS_OK == status){
        status = KeyManager_Certificate_CreateFromFile(certificateSrvLocation, &crt_srv);
        if(STATUS_OK != status){
            printf(">>Stub_Client: Failed to load server certificate\n");
        }else{
            printf(">>Stub_Client: Server certificate loaded\n");
        }
    }

    if(STATUS_OK == status){
        // Private key: load
        status = KeyManager_AsymmetricKey_CreateFromFile(keyLocation, &priv_cli, NULL, 0);
        if(STATUS_OK != status){
            printf(">>Stub_Client: Failed to load private key\n");
        }else{
            printf(">>Stub_Client: Server private key loaded\n");
        }
    }

    // Certificate Authority: load
    if(STATUS_OK == status){
        if(STATUS_OK != KeyManager_Certificate_CreateFromFile("./trusted/cacert.der", &crt_ca)){
            printf(">>Stub_Client: Failed to load CA\n");
        }else{
            printf(">>Stub_Client: CA certificate loaded\n");
        }
    }

	// Init stack configuration
    if(STATUS_OK == status){
        status = SOPC_StackConfiguration_Initialize();
        if(STATUS_OK != status){
            printf(">>Stub_Client: Failed initializing stack\n");
        }else{
            printf(">>Stub_Client: Stack initialized\n");
        }
    }

    if(STATUS_OK == status){
        // Add types
        newEncType = malloc(sizeof(SOPC_EncodeableType));
        if(newEncType == NULL){
            return STATUS_NOK;
        }
        memset(newEncType, 0, sizeof(SOPC_EncodeableType));
        status = SOPC_StackConfiguration_AddTypes(&newEncType, 1);
    }

    // Create channel object
    if(STATUS_OK == status){
        status = SOPC_Channel_Create(&hChannel, SOPC_ChannelSerializer_Binary);
    }

    // Init PKI provider and parse certificate and private key
    // PKIConfig is just used to create the provider but only configuration of PKIType is useful here (paths not used)
    if(STATUS_OK == status){
        if(STATUS_OK != PKIProviderStack_Create(crt_ca, NULL, &pki)){
            printf(">>Stub_Client: Failed to create PKI\n");
        }else{
            printf(">>Stub_Client: PKI created\n");
        }
    }

    // Start connection to server
    if(STATUS_OK == status){
        status = SOPC_Channel_BeginConnect(hChannel,
                                          sEndpointUrl,
                                          //sTransportProfileUri,
                                          crt_cli,                      /* Client Certificate       */
                                          priv_cli,                     /* Private Key              */
                                          crt_srv,                      /* Server Certificate       */
                                          pki,                          /* PKI Config               */
                                          pRequestedSecurityPolicyUri, /* Request secu policy */
                                          5,                            /* Request lifetime */
                                          messageSecurityMode,          /* Message secu mode */
                                          10,                           /* Network timeout */
                                          (SOPC_Channel_PfnConnectionStateChanged*) StubClient_ConnectionEvent_Callback,
                                          &Callback_Data);              /* Connect Callback Data   */
        if(STATUS_OK != status){
            printf(">>Stub_Client: Failed to start connection to server\n");
        }else{
            printf(">>Stub_Client: Establishing connection to server...\n");
        }
    }

    // Empty callback data
    StubClient_CallbackData Callback_Data_Get;

    while (STATUS_OK == status && noEvent && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
    	SOPC_Sleep(sleepTimeout);
    }
    loopCpt = 0;

    if(STATUS_OK == status){
        if (disconnect != 0 || connected == 0){
            status = STATUS_NOK;
            printf(">>Stub_Client: Failed to establish connection to server\n");
        }else{
            printf(">>Stub_Client: Connection to server established\n");
        }
    }

    // Request header
    OpcUa_RequestHeader rHeader;

    // Local configuration: empty
    SOPC_String localId, profileUri;

    if(STATUS_OK == status){
        // Endpoint URL in OPC UA string format
        status = SOPC_String_AttachFromCstring(&stEndpointUrl, sEndpointUrl);
    }

    if(STATUS_OK == status){
        // Initialization of empty request header
        OpcUa_RequestHeader_Initialize (&rHeader);

        // To retrieve response from callback

        status = OpcUa_ClientApi_BeginGetEndpoints(hChannel,     // Channel
                                                    &rHeader,      // Request header
                                                    &stEndpointUrl, // Endpoint
                                                    0,            // No of local id
                                                    &localId,           // local id
                                                    0,            // No of profile URI
                                                    &profileUri,           // profile uri
                                                    (SOPC_Channel_PfnRequestComplete*) StubClient_ResponseEvent_Callback, // response call back
                                                    &Callback_Data_Get); // call back data

        if(STATUS_OK != status){
            printf(">>Stub_Client: Failed to call GetEndpoint service\n");
        }else{
            printf(">>Stub_Client: Calling GetEndpoint service...\n");
        }
    }

    while (STATUS_OK == status && noResp != FALSE && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
    	SOPC_Sleep(sleepTimeout);
    }
    loopCpt = 0;

    if (STATUS_OK == status){
        if(disconnect != 0 || noResp != 0){
            status = STATUS_NOK;
            printf(">>Stub_Client: GetEndpoint service call failed\n");
        }else{
            printf(">>Stub_Client: GetEndpoint service call succeeded\n");
        }
    }

    printf(">>Stub_Client: Final status: %x\n", status);
    PKIProviderStack_Free(pki);
    SOPC_String_Clear(&stEndpointUrl);
    KeyManager_Certificate_Free(crt_cli);
    KeyManager_Certificate_Free(crt_srv);
    KeyManager_Certificate_Free(crt_ca);
    KeyManager_AsymmetricKey_Free(priv_cli);
    SOPC_Channel_Delete(&hChannel);
    SOPC_StackConfiguration_Clear();
    if(newEncType != NULL)
        free(newEncType);
    if(STATUS_OK == status){
        printf(">>Stub_Client: Stub_Client test: OK\n");
    }else{
        printf(">>Stub_Client: Stub_Client test: NOK\n");
    }
    if(status != 0){
        return -1;
    }
    return status;
}
