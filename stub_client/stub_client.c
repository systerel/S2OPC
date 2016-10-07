/*
 * stub_server.c
 *
 *  Created on: Feb 25, 2016
 *      Author: Vincent Monfort (Systerel)
 */

#include "stub_client.h"

#include <stdlib.h>

#include <ua_clientapi.h>
#include <ua_stack_config.h>
#include <wrappers.h>

#include <opcua_pkifactory.h>

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
    if (responseType->typeId == OpcUaId_ServiceFault)
    {
    	printf ("\nService fault response\n");
    }

    /* check response type */
    else if (UA_GetEndpointsResponse_EncodeableType.typeId != responseType->typeId)
    {
    	printf ("\nIncorrect response type !\n");
    }

    /* copy parameters from response object into return parameters. */
    else
    {
    	printf("\n Endpoints response: %d\n",((UA_GetEndpointsResponse*) response)->NoOfEndpoints);
    }

    // Free the allocated response message
    OpcUa_Free(response);

    return 0;
}

void OpcUa_ProxyStubConfiguration_InitializeDefault(OpcUa_ProxyStubConfiguration* a_pProxyStubConfiguration)
{
    /* -1 for integer values means "use stack default" */
    a_pProxyStubConfiguration->bProxyStub_Trace_Enabled              = OpcUa_True;
    a_pProxyStubConfiguration->uProxyStub_Trace_Level                = OPCUA_TRACE_OUTPUT_LEVEL_DEBUG;
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

    OpcUa_InitializeStatus(OpcUa_Module_Server, "StubClient_StartUp");

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
    OpcUa_ByteString_Initialize (&ServerCertificate);

    // Private key: init
    OpcUa_ByteString ClientPrivateKey;
    OpcUa_ByteString_Initialize (&ClientPrivateKey);


    // Empty callback data
    StubClient_CallbackData Callback_Data;

    // Policy security: None
    char* pRequestedSecurityPolicyUri = OpcUa_SecurityPolicy_Basic256Sha256;
    OpcUa_GotoErrorIfBad(uStatus);

    // Message security mode: None
    OpcUa_MessageSecurityMode messageSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;//OpcUa_MessageSecurityMode_None;

    // Init platform dependent code
	uStatus = UA_P_Initialize(&StubClient_g_pPortLayerHandle);
	OpcUa_GotoErrorIfBad(uStatus);

    // TODO: remove
    // Init proxy stub configuration with default values (only for socket now)
    OpcUa_ProxyStubConfiguration_InitializeDefault(&OpcUa_ProxyStub_g_Configuration);
    uStatus = OpcUa_ProxyStub_Initialize(StubClient_g_pPortLayerHandle,
                                         &OpcUa_ProxyStub_g_Configuration);
    OpcUa_GotoErrorIfBad(uStatus);

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
    uStatus = OPCUA_P_PKIFACTORY_CREATEPKIPROVIDER(&pPKIConfig, &pkiProvider);
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
    uStatus = UA_Channel_BeginConnect(hChannel,
                                      sEndpointUrl,
                                      //sTransportProfileUri,
                                      (UA_ByteString*) &ClientCertificate,           /* Client Certificate       */
                                      (UA_ByteString*) &ClientPrivateKey,            /* Private Key              */
                                      (UA_ByteString*) &ServerCertificate,           /* Server Certificate       */
                                      &pPKIConfig,                  /* PKI Config               */
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
    UA_RequestHeader rHeader;

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
    	uStatus = UA_SocketManager_Loop (OpcUa_Null, // global socket manager
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
    // TODO: wrappers for managing OpcUa_String (inside request header ...)
    UA_RequestHeader_Initialize (&rHeader);

    // To retrieve response from callaback

    uStatus = UA_ClientApi_BeginGetEndpoints(hChannel,     // Channel
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
    	uStatus = UA_SocketManager_Loop (OpcUa_Null, // global socket manager
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
    pkiProvider.CloseCertificateStore(&pkiProvider, &hCertificateStore);
    OPCUA_P_PKIFACTORY_DELETEPKIPROVIDER(&pkiProvider);
    String_Clear(&stEndpointUrl);
    OpcUa_ByteString_Clear(&ClientCertificate);
    OpcUa_ByteString_Clear(&ClientPrivateKey);
    OpcUa_ByteString_Clear(&ServerCertificate);
    StackConfiguration_Clear();
    // TODO: only for socket now
    OpcUa_ProxyStub_Clear();

    OpcUa_ReturnStatusCode;

    OpcUa_BeginErrorHandling;
    pkiProvider.CloseCertificateStore(&pkiProvider, &hCertificateStore);
    OPCUA_P_PKIFACTORY_DELETEPKIPROVIDER(&pkiProvider);
    String_Clear(&stEndpointUrl);
    OpcUa_ByteString_Clear(&ClientCertificate);
    OpcUa_ByteString_Clear(&ClientPrivateKey);
    OpcUa_ByteString_Clear(&ServerCertificate);
    StackConfiguration_Clear();
    // TODO: only for socket now
    OpcUa_ProxyStub_Clear();

    printf ("Error status: %d\n", uStatus);
    if(uStatus != 0){
        return -1;
    }
    OpcUa_FinishErrorHandling;
}
