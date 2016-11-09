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
#include <string.h>

#include "opcua_identifiers.h"
#include "sopc_stack_config.h"
#include "crypto_profiles.h"
#include "pki_stack.h"
#include "sopc_clientapi.h"
#include "sopc_run.h"

int noEvent = 1;
int noResp = 1;
int disconnect = 0;
int connected = 0;

SOPC_EncodeableType* newEncType;

SOPC_StatusCode StubClient_ConnectionEvent_Callback(SOPC_Channel       channel,
                                                    void*              callbackData,
                                                    SOPC_Channel_Event event,
                                                    SOPC_StatusCode    status)
{
    (void) callbackData;
    (void) channel;
    printf ("\nConnection event: channel event=%d and status=%x\n", event, status);
	if (status == STATUS_OK){
	    connected = 1;
		noEvent = 0;
	}
	if (event == SOPC_ChannelEvent_Disconnected){
		disconnect = 1;
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
    printf ("\nResponse event\n");
	noResp = 0;

    /* check for fault */
    if (responseType->TypeId == OpcUaId_ServiceFault)
    {
    	printf ("\nService fault response\n");
    }

    /* check response type */
    else if (OpcUa_GetEndpointsResponse_EncodeableType.TypeId != responseType->TypeId)
    {
    	printf ("\nIncorrect response type !\n");
    }

    /* copy parameters from response object into return parameters. */
    else
    {
    	printf("\n Endpoints response: %d\n",((OpcUa_GetEndpointsResponse*) response)->NoOfEndpoints);
    }

    // Free the allocated response message
    free(response);

    return 0;
}

int main(void){
    SOPC_StatusCode status = STATUS_OK;

    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 10000;
    // Counter to stop waiting responses after 5 seconds
    uint32_t loopCpt = 0;

    SOPC_Channel hChannel;
    // Endpoint URL
    SOPC_String stEndpointUrl;
    SOPC_String_Initialize(&stEndpointUrl);
    char* sEndpointUrl = "opc.tcp://localhost:8888/myEndPoint";

    // Paths to client certificate/key and server certificate
    // Client certificate name
    char* certificateLocation = "./client_public/client.der";
    // Server certificate name
    char* certificateSrvLocation = "./server_public/server.der";
    // Client private key
    char* keyLocation = "./client_private/client.key";

    // The certificates: load
    Certificate *crt_cli = NULL, *crt_srv = NULL;
    status = KeyManager_Certificate_CreateFromFile(certificateLocation, &crt_cli);
    if(STATUS_OK == status)
        status = KeyManager_Certificate_CreateFromFile(certificateSrvLocation, &crt_srv);
    if(STATUS_OK != status)
        printf("Failed to load certificate(s)\n");

    // Private key: load
    AsymmetricKey *priv_cli = NULL;
    status = KeyManager_AsymmetricKey_CreateFromFile(keyLocation, &priv_cli, NULL, 0);
    if(STATUS_OK != status)
        printf("Failed to load private key\n");

    // Certificate Authority: load
    Certificate *crt_ca = NULL;
    if(STATUS_OK != KeyManager_Certificate_CreateFromFile("./trusted/cacert.der", &crt_ca))
        printf("Failed to load CA\n");

    // Empty callback data
    StubClient_CallbackData Callback_Data;

    // Policy security: None
    char* pRequestedSecurityPolicyUri = SecurityPolicy_Basic256Sha256_URI;
    if(STATUS_OK != status) goto Error;

    // Message security mode: None
    OpcUa_MessageSecurityMode messageSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;//OpcUa_MessageSecurityMode_None;

	// Init stack configuration
	StackConfiguration_Initialize();
	if(STATUS_OK != status) goto Error;

	// Add types
	newEncType = malloc(sizeof(SOPC_EncodeableType));
	if(newEncType == NULL){
	    return STATUS_NOK;
	}
	memset(newEncType, 0, sizeof(SOPC_EncodeableType));
	StackConfiguration_AddTypes(&newEncType, 1);

    // Create channel object
    status = SOPC_Channel_Create(&hChannel, SOPC_ChannelSerializer_Binary);
	if(STATUS_OK != status) goto Error;

    printf ("%d\n", status);

    if(STATUS_OK != status) goto Error;

    // Init PKI provider and parse certificate and private key
    // PKIConfig is just used to create the provider but only configuration of PKIType is useful here (paths not used)
    PKIProvider *pki = NULL;
    if(STATUS_OK != PKIProviderStack_Create(crt_ca, NULL, &pki))
        printf("Failed to create PKI\n");

    // Start connection to server
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

    if(STATUS_OK != status) goto Error;

    // Request header
    OpcUa_RequestHeader rHeader;

    // Local configuration: empty
    SOPC_String localId, profileUri;
    // Endpoint URL in OPC UA string format
    status = SOPC_String_AttachFromCstring(&stEndpointUrl, sEndpointUrl);
    if(STATUS_OK != status) goto Error;

    // Empty callback data
    StubClient_CallbackData Callback_Data_Get;

    while (noEvent && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
#if OPCUA_MULTITHREADED
    	// just wait for callback called
    	OpcUa_Thread_Sleep (sleepTimeout);
#else
    	// Retrieve received messages on socket
    	status = SOPC_TreatReceivedMessages(sleepTimeout);
        printf ("ServerLoop status: %d\n", status);
        if(STATUS_OK != status) goto Error;
#endif //OPCUA_MULTITHREADED
    }
    loopCpt = 0;

    if (disconnect != 0 || connected == 0){
        status = STATUS_NOK;
    	goto Error;
    }

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

    while (noResp && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
#if OPCUA_MULTITHREADED
    	// just wait for callback
    	OpcUa_Thread_Sleep (sleepTimeout);
#else
    	// Retrieve received messages on socket
    	status = SOPC_TreatReceivedMessages(sleepTimeout);
        printf ("ServerLoop status: %d\n", status);
        if(STATUS_OK != status) goto Error;
#endif //OPCUA_MULTITHREADED
    }
    loopCpt = 0;

    if (disconnect != 0 || noResp != 0){
        status = STATUS_NOK;
    	goto Error;
    }

    printf ("Final status: %d\n", status);
    PKIProviderStack_Free(pki);
    SOPC_String_Clear(&stEndpointUrl);
    KeyManager_Certificate_Free(crt_cli);
    KeyManager_Certificate_Free(crt_srv);
    KeyManager_Certificate_Free(crt_ca);
    KeyManager_AsymmetricKey_Free(priv_cli);
    SOPC_Channel_Delete(&hChannel);
    StackConfiguration_Clear();

    return status;

    Error:
    SOPC_String_Clear(&stEndpointUrl);
    SOPC_Channel_Delete(&hChannel);
    StackConfiguration_Clear();

    printf ("Error status: %d\n", status);
    if(status != 0){
        return -1;
    }
    return status;
}
