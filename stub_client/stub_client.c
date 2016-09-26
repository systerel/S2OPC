/*
 * stub_server.c
 *
 *  Created on: Feb 25, 2016
 *      Author: Vincent Monfort (Systerel)
 */

#include "stub_client.h"
#include <ua_clientapi.h>

int noEvent = 1;
int noResp = 1;
int disconnect = 0;
int connected = 0;

OpcUa_Handle StubClient_g_pPortLayerHandle = OpcUa_Null;

OpcUa_StatusCode StubClient_ConnectionEvent_Callback(OpcUa_Channel                   hChannel,
												     OpcUa_Void*                     pCallbackData,
													 OpcUa_Channel_Event             eEvent,
													 OpcUa_StatusCode                uStatus){
													 //OpcUa_Channel_SecurityToken*    pSecurityToken){
    (void) pCallbackData;
    (void) hChannel;
    printf ("\nConnection event: channel event=%d and status=%x\n", eEvent, uStatus);
	if (uStatus == OpcUa_Good){
	    connected = 1;
		noEvent = 0;
	}
	if (eEvent == eOpcUa_Channel_Event_Disconnected){
		disconnect = 1;
	}
    return 0;
}

OpcUa_StatusCode StubClient_ResponseEvent_Callback(OpcUa_Channel         hChannel,
												   OpcUa_Void*           pResponse,
												   OpcUa_EncodeableType* pResponseType,
												   OpcUa_Void*           pCallbackData,
												   OpcUa_StatusCode      uStatus){
    (void) pCallbackData;
    (void) hChannel;
    (void) uStatus;
    printf ("\nResponse event\n");
	noResp = 0;

    /* check for fault */
    if (pResponseType->TypeId == OpcUaId_ServiceFault)
    {
    	printf ("\nService fault response\n");
    }

    /* check response type */
    else if (OpcUa_GetEndpointsResponse_EncodeableType.TypeId != pResponseType->TypeId)
    {
    	printf ("\nIncorrect response type !\n");
    }

    /* copy parameters from response object into return parameters. */
    else
    {
    	printf("\n Endpoints response: %d\n",((OpcUa_GetEndpointsResponse*) pResponse)->NoOfEndpoints);
    }

    // Free the allocated response message
    OpcUa_Free(pResponse);

    return 0;
}

void OpcUa_ProxyStubConfiguration_InitializeDefault(OpcUa_ProxyStubConfiguration* a_pProxyStubConfiguration)
{
    /* -1 for integer values means "use stack default" */
    a_pProxyStubConfiguration->bProxyStub_Trace_Enabled              = OpcUa_True;
    a_pProxyStubConfiguration->uProxyStub_Trace_Level                = OPCUA_TRACE_LEVEL_DEBUG;
    //a_pProxyStubConfiguration->uProxyStub_Trace_Output               = OPCUA_TRACE_OUTPUT_CONSOLE;
    a_pProxyStubConfiguration->iSerializer_MaxAlloc                  = -1;
    a_pProxyStubConfiguration->iSerializer_MaxStringLength           = -1;
    a_pProxyStubConfiguration->iSerializer_MaxByteStringLength       = -1;
    a_pProxyStubConfiguration->iSerializer_MaxArrayLength            = -1;
    a_pProxyStubConfiguration->iSerializer_MaxMessageSize            = -1;
    a_pProxyStubConfiguration->iSerializer_MaxRecursionDepth         = -1;
    a_pProxyStubConfiguration->bSecureListener_ThreadPool_Enabled    = OpcUa_False;
    a_pProxyStubConfiguration->iSecureListener_ThreadPool_MinThreads = -1;
    a_pProxyStubConfiguration->iSecureListener_ThreadPool_MaxThreads = -1;
    a_pProxyStubConfiguration->iSecureListener_ThreadPool_MaxJobs    = -1;
    a_pProxyStubConfiguration->bSecureListener_ThreadPool_BlockOnAdd = OpcUa_True;
    a_pProxyStubConfiguration->uSecureListener_ThreadPool_Timeout    = OPCUA_INFINITE;
    a_pProxyStubConfiguration->iTcpListener_DefaultChunkSize         = -1;
    a_pProxyStubConfiguration->iTcpConnection_DefaultChunkSize       = -1;
    a_pProxyStubConfiguration->bTcpListener_ClientThreadsEnabled     = OpcUa_True;
    a_pProxyStubConfiguration->bTcpStream_ExpectWriteToBlock         = OpcUa_True;
    a_pProxyStubConfiguration->iTcpTransport_MaxMessageLength        = -1;
    a_pProxyStubConfiguration->iTcpTransport_MaxChunkCount           = -1;
}


int main(void){
    // Sleep timeout in milliseconds
    const uint32_t sleepTimeout = 500;
    // Loop timeout in milliseconds
    const uint32_t loopTimeout = 5000;
    // Counter to stop waiting responses after 5 seconds
    uint32_t loopCpt = 0;

	// Init proxy stub configuration with default values
	OpcUa_ProxyStubConfiguration_InitializeDefault(&OpcUa_ProxyStub_g_Configuration);
	OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Level = OPCUA_TRACE_OUTPUT_LEVEL_DEBUG;

    OpcUa_Channel hChannel;
    // Endpoint URL
    OpcUa_CharA *sEndpointUrl = "opc.tcp://localhost:8888/myEndPoint";
    // Transport profile
    //const OpcUa_CharA *sTransportProfileUri = OpcUa_TransportProfile_UaTcp;

    // The PKI provider
    // Default directory for certificates
    char* revoked = "./revoked";
    char* untrusted = "./untrusted";
    char* trusted = "./trusted";
    // Init PKI config certificate validation
    OpcUa_P_OpenSSL_CertificateStore_Config pPKIConfig;
    pPKIConfig.PkiType = OpcUa_OpenSSL_PKI;
    pPKIConfig.CertificateTrustListLocation       = trusted;
    pPKIConfig.CertificateRevocationListLocation  = revoked;
    //pPKIConfig.IssuerCertificateStoreLocation         = defaultDir;
    pPKIConfig.CertificateUntrustedListLocation   = untrusted;
    pPKIConfig.Flags = OPCUA_P_PKI_OPENSSL_ADD_UNTRUSTED_LIST_TO_ROOT_CERTIFICATES|
	                   OPCUA_P_PKI_OPENSSL_REQUIRE_CHAIN_CERTIFICATE_IN_TRUST_LIST;
	           	       // OPCUA_P_PKI_OPENSSL_CHECK_REVOCATION_ALL; revocation list needed

    // Paths to client certificate/key and server certificate
    // Client certificate name
    char* certificateLocation = "./client_public/client.der";
    // Server certificate name
    char* certificateSrvLocation = "./server_public/server.der";
    // Client private key
    char* keyLocation = "./client_private/client.key";
    OpcUa_PKIProvider pkiProvider;
    OpcUa_Handle hCertificateStore = OpcUa_Null;

    // The certificates: init
    OpcUa_ByteString ClientCertificate, ServerCertificate;
    OpcUa_ByteString_Initialize (&ClientCertificate);

    // Private key: init
    OpcUa_ByteString ClientPrivateKey;
    OpcUa_ByteString_Initialize (&ClientPrivateKey);


    // Empty callback data
    StubClient_CallbackData Callback_Data;

    // Policy security: None
    OpcUa_String* pRequestedSecurityPolicyUri = OpcUa_String_FromCString(OpcUa_SecurityPolicy_Basic128Rsa15); //OpcUa_String_FromCString(OpcUa_SecurityPolicy_None);


    // Message security mode: None
    OpcUa_MessageSecurityMode messageSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;//OpcUa_MessageSecurityMode_None;

    OpcUa_InitializeStatus(OpcUa_Module_Server, "StubClient_StartUp");

    // Init platform dependent code
	uStatus = OpcUa_P_Initialize(&StubClient_g_pPortLayerHandle);
	OpcUa_GotoErrorIfBad(uStatus);

	// Init proxystub level configuration
	uStatus = OpcUa_ProxyStub_Initialize (StubClient_g_pPortLayerHandle,
									      &OpcUa_ProxyStub_g_Configuration);
	OpcUa_GotoErrorIfBad(uStatus);

    // Create channel object
    uStatus = OpcUa_Channel_Create(&hChannel, OpcUa_Channel_SerializerType_Binary);
	OpcUa_GotoErrorIfBad(uStatus);

    printf ("%d\n", uStatus);

    OpcUa_GotoErrorIfBad(uStatus);

    // Init PKI provider and parse certificate and private key
    // PKIConfig is just used to create the provider but only configuration of PKIType is useful here (paths not used)
    uStatus = OpcUa_PKIProvider_Create(&pPKIConfig, &pkiProvider);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = pkiProvider.OpenCertificateStore (&pkiProvider, &hCertificateStore);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = pkiProvider.LoadCertificate (&pkiProvider,
    									   certificateLocation,
										   hCertificateStore,
										   &ClientCertificate);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = pkiProvider.LoadPrivateKeyFromFile(keyLocation,
                  	  	  	  	  	  	  	  	 OpcUa_Crypto_Encoding_PEM,
												 OpcUa_Null,
												 &ClientPrivateKey);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = pkiProvider.LoadCertificate (&pkiProvider,
    									   certificateSrvLocation,
										   hCertificateStore,
										   &ServerCertificate);
    OpcUa_GotoErrorIfBad(uStatus);

#if OPCUA_MULTITHREADED == OPCUA_CONFIG_NO
    //uStatus = OpcUa_SocketManager_Create (OpcUa_Null, 0, OPCUA_SOCKET_NO_FLAG);
#endif //OPCUA_MULTITHREADED

    // Start connection to server
    uStatus = OpcUa_Channel_BeginConnect(hChannel,
    									 sEndpointUrl,
										 //sTransportProfileUri,
										 &ClientCertificate,           /* Client Certificate       */
										 &ClientPrivateKey,            /* Private Key              */
										 &ServerCertificate,           /* Server Certificate       */
										 &pPKIConfig,                  /* PKI Config               */
										 pRequestedSecurityPolicyUri, /* Request secu policy */
										 5,                            /* Request lifetime */
										 messageSecurityMode,          /* Message secu mode */
										 10,                           /* Network timeout */
										 StubClient_ConnectionEvent_Callback,
										 &Callback_Data);              /* Connect Callback Data   */

    OpcUa_GotoErrorIfBad(uStatus);

    // Request header
    OpcUa_RequestHeader rHeader;
    // Local configuration: empty
    //OpcUa_String localId, profileUri;
    UA_String localId, profileUri;
    // Endpoint URL in OPC UA string format
    //const OpcUa_String* stEndpointUrl = OpcUa_String_FromCString(sEndpointUrl);
    const UA_String* stEndpointUrl = String_CreateFromCString(sEndpointUrl);
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
    	uStatus = OpcUa_SocketManager_Loop (OpcUa_Null, // global socket manager
    				                        sleepTimeout,
											OpcUa_True);
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
    OpcUa_RequestHeader_Initialize (&rHeader);

    // To retrieve response from callaback

    uStatus = UA_ClientApi_BeginGetEndpoints(hChannel,     // Channel
                                             (UA_RequestHeader*) &rHeader,      // Request header
                                             stEndpointUrl, // Endpoint
                                             0,            // No of local id
                                             &localId,           // local id
                                             0,            // No of profile URI
                                             &profileUri,           // profile uri
                                             StubClient_ResponseEvent_Callback, // response call back
                                             &Callback_Data_Get); // call back data

    while (noResp && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
#if OPCUA_MULTITHREADED
    	// just wait for callback
    	OpcUa_Thread_Sleep (sleepTimeout);
#else
    	// Retrieve received messages on socket
    	uStatus = OpcUa_P_SocketManager_ServeLoop (OpcUa_Null, // global socket manager
    				                               sleepTimeout,
												   OpcUa_True);
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
    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;
    printf ("Error status: %d\n", uStatus);
    if(uStatus != 0){
        return -1;
    }
    OpcUa_FinishErrorHandling;
}
