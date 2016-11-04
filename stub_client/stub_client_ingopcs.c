/*
 * stub_server.c
 *
 *  Created on: Feb 25, 2016
 *      Author: Vincent Monfort (Systerel)
 */

#include "stub_client.h"

#include <stdlib.h>
#include <string.h>

#include <ua_clientapi.h>
#include <ua_stack_config.h>
#include <opcua_identifiers.h>

#include <crypto_profiles.h>
#include <pki.h>
#include <pki_stack.h>
#include <key_manager.h>

int noEvent = 1;
int noResp = 1;
int disconnect = 0;
int connected = 0;

OpcUa_Handle StubClient_g_pPortLayerHandle = OpcUa_Null;

UA_EncodeableType* newEncType;

OpcUa_StatusCode StubClient_ConnectionEvent_Callback(UA_Channel       channel,
                                                     void*            callbackData,
                                                     UA_Channel_Event event,
                                                     StatusCode       status)
{
    (void) callbackData;
    (void) channel;
    printf ("\nConnection event: channel event=%d and status=%x\n", event, status);
	if (status == OpcUa_Good){
	    connected = 1;
		noEvent = 0;
	}
	if (event == ChannelEvent_Disconnected){
		disconnect = 1;
	}
    return 0;
}

OpcUa_StatusCode StubClient_ResponseEvent_Callback(UA_Channel         channel,
                                                   void*              response,
                                                   UA_EncodeableType* responseType,
                                                   void*              callbackData,
                                                   StatusCode         status){
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
    OpcUa_Free(response);

    return 0;
}

int main(void){
    StatusCode status = STATUS_OK;

    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 10000;
    // Counter to stop waiting responses after 5 seconds
    uint32_t loopCpt = 0;

    OpcUa_Channel hChannel;
    // Endpoint URL
    UA_String stEndpointUrl;
    String_Initialize(&stEndpointUrl);
    OpcUa_CharA *sEndpointUrl = "opc.tcp://localhost:8888/myEndPoint";
    // Transport profile
    //const OpcUa_CharA *sTransportProfileUri = OpcUa_TransportProfile_UaTcp;

    // The PKI provider
    // Default directory for certificates
    //char* revoked = "./revoked";
    //char* untrusted = "./untrusted";
    //char* trusted = "./trusted";
    // Loads certificates, then create PKI

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
    status = KeyManager_AsymmetricKey_CreateFromFile(keyLocation, &priv_cli);
    if(STATUS_OK != status)
        printf("Failed to load private key\n");

    // Certificate Authority: load
    Certificate *crt_ca = NULL;
    if(STATUS_OK != KeyManager_Certificate_CreateFromFile("./trusted/cacert.der", &crt_ca))
        printf("Failed to load CA\n");

    // Empty callback data
    StubClient_CallbackData Callback_Data;

    // Policy security: None
    char* pRequestedSecurityPolicyUri = OpcUa_SecurityPolicy_Basic256Sha256;
    OpcUa_GotoErrorIfBad(uStatus);

    // Message security mode: None
    OpcUa_MessageSecurityMode messageSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;//OpcUa_MessageSecurityMode_None;

	// Init stack configuration
	StackConfiguration_Initialize();
	OpcUa_GotoErrorIfBad(uStatus);

	// Add types
	newEncType = malloc(sizeof(UA_EncodeableType));
	if(newEncType == NULL){
	    return STATUS_NOK;
	}
	memset(newEncType, 0, sizeof(UA_EncodeableType));
	StackConfiguration_AddTypes(&newEncType, 1);

    // Create channel object
    uStatus = UA_Channel_Create(&hChannel, OpcUa_Channel_SerializerType_Binary);
	OpcUa_GotoErrorIfBad(uStatus);

    printf ("%d\n", uStatus);

    OpcUa_GotoErrorIfBad(uStatus);

    // Init PKI provider and parse certificate and private key
    // PKIConfig is just used to create the provider but only configuration of PKIType is useful here (paths not used)
    PKIProvider *pki = NULL;
    if(STATUS_OK != PKIProviderStack_Create(crt_ca, NULL, &pki))
        printf("Failed to create PKI\n");

#if OPCUA_MULTITHREADED == OPCUA_CONFIG_NO
    //uStatus = OpcUa_SocketManager_Create (OpcUa_Null, 0, OPCUA_SOCKET_NO_FLAG);
#endif //OPCUA_MULTITHREADED

    // Start connection to server
    uStatus = UA_Channel_BeginConnect(hChannel,
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
                                      (UA_Channel_PfnConnectionStateChanged*) StubClient_ConnectionEvent_Callback,
                                      &Callback_Data);              /* Connect Callback Data   */

    OpcUa_GotoErrorIfBad(uStatus);

    // Request header
    // TODO: wrappers for managing OpcUa_String (inside request header ...)
    //OpcUa_RequestHeader rHeader;
    OpcUa_RequestHeader rHeader;

    // Local configuration: empty
    // TODO: wrappers for managing OpcUa_String
    //OpcUa_String localId, profileUri;
    UA_String localId, profileUri;
    // Endpoint URL in OPC UA string format
    // TODO: wrappers for managing OpcUa_String
    //const OpcUa_String* stEndpointUrl = OpcUa_String_FromCString(sEndpointUrl);
    uStatus = String_AttachFromCstring(&stEndpointUrl, sEndpointUrl);
    OpcUa_GotoErrorIfBad(uStatus);

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
    	uStatus = UA_SocketManager_Loop (UA_SocketManager_GetGlobal(),
    				                     sleepTimeout);
        printf ("ServerLoop status: %d\n", uStatus);
        OpcUa_GotoErrorIfBad(uStatus);
#endif //OPCUA_MULTITHREADED
    }
    loopCpt = 0;

    if (disconnect != 0 || connected == 0){
        uStatus = STATUS_NOK;
    	goto Error;
    }

    // Initialization of empty request header
    // TODO: wrappers for managing OpcUa_String (inside request header ...)
    OpcUa_RequestHeader_Initialize (&rHeader);

    // To retrieve response from callaback

    uStatus = OpcUa_ClientApi_BeginGetEndpoints(hChannel,     // Channel
                                                &rHeader,      // Request header
                                                &stEndpointUrl, // Endpoint
                                                0,            // No of local id
                                                &localId,           // local id
                                                0,            // No of profile URI
                                                &profileUri,           // profile uri
                                                (UA_Channel_PfnRequestComplete*) StubClient_ResponseEvent_Callback, // response call back
                                                &Callback_Data_Get); // call back data

    while (noResp && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
#if OPCUA_MULTITHREADED
    	// just wait for callback
    	OpcUa_Thread_Sleep (sleepTimeout);
#else
    	// Retrieve received messages on socket
    	uStatus = UA_SocketManager_Loop (UA_SocketManager_GetGlobal(),
    				                     sleepTimeout);
        printf ("ServerLoop status: %d\n", uStatus);
        OpcUa_GotoErrorIfBad(uStatus);
#endif //OPCUA_MULTITHREADED
    }
    loopCpt = 0;

    if (disconnect != 0 || noResp != 0){
        uStatus = STATUS_NOK;
    	goto Error;
    }

// Get Endpoints multithread ONLY:
//    OpcUa_ResponseHeader pResponseHeader;
//	OpcUa_Int32 pNoOfEndpoints;
//    OpcUa_EndpointDescription* pEndpoints;
//
//    uStatus = OpcUa_ClientApi_GetEndpoints(hChannel,     // Channel
//    									   &rHeader,      // Request header
//										   stEndpointUrl, // Endpoint
//										   0,            // No of local id
//										   &localId,           // local id
//										   0,            // No of profile URI
//										   &profileUri,           // profile uri
//										   &pResponseHeader, // response call back
//										   &pNoOfEndpoints,
//										   &pEndpoints); // call back data

    printf ("Final status: %d\n", uStatus);
    PKIProviderStack_Free(pki);
    String_Clear(&stEndpointUrl);
    KeyManager_Certificate_Free(crt_cli);
    KeyManager_Certificate_Free(crt_srv);
    KeyManager_Certificate_Free(crt_ca);
    KeyManager_AsymmetricKey_Free(priv_cli);
    UA_Channel_Delete(&hChannel);
    StackConfiguration_Clear();

    OpcUa_ReturnStatusCode;

    OpcUa_BeginErrorHandling;
    String_Clear(&stEndpointUrl);
    UA_Channel_Delete(&hChannel);
    StackConfiguration_Clear();

    printf ("Error status: %d\n", uStatus);
    if(uStatus != 0){
        return -1;
    }
    OpcUa_FinishErrorHandling;
}
