/*
 * stub_server.c
 *
 *  Created on: Feb 25, 2016
 *      Author: Vincent Monfort (Systerel)
 */

#include "stub_server.h"
#include <stdint.h>

// indicates if a connection was closed (after being opened)
int connectionClosed = 0;
int connected = 0;

OpcUa_Handle StubServer_g_pPortLayerHandle = OpcUa_Null;

OpcUa_StatusCode StubServer_EndpointEvent_Callback(OpcUa_Endpoint          a_hEndpoint,
                                                   OpcUa_Void             *a_pCallbackData,
                                                   OpcUa_Endpoint_Event    a_eEvent,
                                                   OpcUa_StatusCode        a_uStatus,
                                                   OpcUa_UInt32            a_uSecureChannelId,
                                                   OpcUa_ByteString       *a_pbsClientCertificate,
                                                   OpcUa_String           *a_pSecurityPolicy,
                                                   OpcUa_UInt16            a_uSecurityMode){
    (void) a_hEndpoint;
    (void) a_pCallbackData;
    (void) a_uSecureChannelId;
    (void) a_pbsClientCertificate;
    (void) a_pSecurityPolicy;
    (void) a_uSecurityMode;
    printf ("\nEndpoint CALLBACK called with event %d and status %x !\n", a_eEvent, a_uStatus);
    if (a_eEvent == eOpcUa_Endpoint_Event_SecureChannelClosed){
    	connectionClosed = 1;
    }
    else if (a_eEvent == eOpcUa_Endpoint_Event_SecureChannelOpened){
    	// By pass certificate validation for secure channel in any result case
        connected = 1;
    	//return OpcUa_BadContinue;
    	return 0;
    }
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
    const uint32_t loopTimeout = 20000;
    // Counter to stop waiting responses after 5 seconds
    uint32_t loopCpt = 0;

	// Init proxy stub configuration with default values
	OpcUa_ProxyStubConfiguration_InitializeDefault(&OpcUa_ProxyStub_g_Configuration);
	OpcUa_ProxyStub_g_Configuration.uProxyStub_Trace_Level = OPCUA_TRACE_OUTPUT_LEVEL_DEBUG;

    OpcUa_InitializeStatus(OpcUa_Module_Server, "StubServer_StartUp");

    // Init platform dependent code
	uStatus = OpcUa_P_Initialize(&StubServer_g_pPortLayerHandle);
	OpcUa_GotoErrorIfBad(uStatus);

	// Init proxystub level configuration
	uStatus = OpcUa_ProxyStub_Initialize (StubServer_g_pPortLayerHandle,
			                              &OpcUa_ProxyStub_g_Configuration);
	OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_Endpoint hEndpoint;
    // Endpoint URL
    OpcUa_CharA *sEndpointUrl = "opc.tcp://localhost:8888/myEndPoint";
    // Transport profile
    //const OpcUa_CharA *sTransportProfileUri = OpcUa_TransportProfile_UaTcp;

    // The PKI provider
    // Default directory for certificates
    char* revoked = "./revoked";
    char* untrusted = "./untrusted";
    char* trusted = "./trusted";
    // Init PKI config
    OpcUa_P_OpenSSL_CertificateStore_Config pPKIConfig;
    pPKIConfig.PkiType = OpcUa_OpenSSL_PKI;
    pPKIConfig.CertificateTrustListLocation       = trusted;
    pPKIConfig.CertificateRevocationListLocation  = revoked;
    //pPKIConfig.IssuerCertificateStoreLocation         = defaultDir;
    pPKIConfig.CertificateUntrustedListLocation   = untrusted;
    // Configure the way Open SSL treat or ignores paths !
    // Could allow to consider trusted as root certificates but refuse self signed certificates, etc.
    pPKIConfig.Flags = OPCUA_P_PKI_OPENSSL_ADD_UNTRUSTED_LIST_TO_ROOT_CERTIFICATES|
	                   OPCUA_P_PKI_OPENSSL_REQUIRE_CHAIN_CERTIFICATE_IN_TRUST_LIST;
	           	       // OPCUA_P_PKI_OPENSSL_CHECK_REVOCATION_ALL; revocation list needed

    OpcUa_PKIProvider pkiProvider;
    OpcUa_Handle hCertificateStore = OpcUa_Null;

    // Paths to server certificate/key
    // Server certificate name
    char* certificateSrvLocation = "./server_public/server.der";
    // Client private key
    char* keyLocation = "./server_private/server.key";

    // The server's certificate: empty
    OpcUa_ByteString ServerCertificate;
    OpcUa_ByteString_Initialize (&ServerCertificate);

    // Private key of the server: empty
    OpcUa_Key ServerPrivateKey;
    OpcUa_Key_Initialize(&ServerPrivateKey);
    ServerPrivateKey.Type = OpcUa_Crypto_KeyType_Rsa_Private;

    // Empty callback data
    StubServer_Endpoint Callback_Data;

    // Number of the secu policy configuration: 1
    OpcUa_UInt NbOfSecurityPolicyConfigurations = 1;

    // Policy security: None
    //const OpcUa_String* pSecurityPolicyUri = OpcUa_String_FromCString(OpcUa_SecurityPolicy_Basic128Rsa15);

    // Secu policy configuration: empty
    OpcUa_Endpoint_SecurityPolicyConfiguration* secuConfig = (OpcUa_Endpoint_SecurityPolicyConfiguration*)OpcUa_Alloc(NbOfSecurityPolicyConfigurations * sizeof(OpcUa_Endpoint_SecurityPolicyConfiguration));
    OpcUa_String_AttachReadOnly(&secuConfig[0].sSecurityPolicy, OpcUa_SecurityPolicy_Basic128Rsa15);
    secuConfig[0].uMessageSecurityModes = OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_SIGNANDENCRYPT | OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_NONE |\
    		OPCUA_SECURECHANNEL_MESSAGESECURITYMODE_SIGN;

	// Initialize proxystub level with default services and TCP UA protocol
    uStatus = OpcUa_Endpoint_Create (&hEndpoint, OpcUa_Endpoint_SerializerType_Binary, OpcUa_Null); //StubServer_SupportedServices);
    OpcUa_GotoErrorIfBad(uStatus);

    printf ("%d\n", uStatus);

#if OPCUA_MULTITHREADED == OPCUA_CONFIG_NO
    // In case of multithreaded following statement is not called
    // Still use the global socket manager for now but we should use a different one ?
    uStatus = OpcUa_SocketManager_Create (OpcUa_Null, 10, OPCUA_SOCKET_NO_FLAG);
#endif // OPCUA_MULTITHREADED

    OpcUa_GotoErrorIfBad(uStatus);

    // Init PKI provider and parse certificate and private key
    // PKIConfig is just used to create the provider but only configuration of PKIType is useful here (paths not used)
    uStatus = OpcUa_PKIProvider_Create(&pPKIConfig,
    								   &pkiProvider);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = pkiProvider.OpenCertificateStore (&pkiProvider, &hCertificateStore);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = pkiProvider.LoadCertificate (&pkiProvider,
    									   certificateSrvLocation,
										   hCertificateStore,
										   &ServerCertificate);
    OpcUa_GotoErrorIfBad(uStatus);

    uStatus = pkiProvider.LoadPrivateKeyFromFile(keyLocation,
                  	  	  	  	  	  	  	  	 OpcUa_Crypto_Encoding_PEM,
												 OpcUa_Null,
												 &ServerPrivateKey.Key);
    OpcUa_GotoErrorIfBad(uStatus);

    // Start the server code (connection and default management on secu channel)
    OpcUa_Trace(OPCUA_TRACE_LEVEL_INFO, "Stub Server: Opening Endpoint...\n");
    uStatus = OpcUa_Endpoint_Open(hEndpoint,                         // Endpoint
                                  sEndpointUrl,                      // URL
                                  //sTransportProfileUri,              // Transport profile
								  OpcUa_True, // listen all interfaces
                                  StubServer_EndpointEvent_Callback, // Endpoint Callback
                                  &Callback_Data,                    // Endpoint Callback Data
                                  &ServerCertificate,                // Server Certificate
                                  &ServerPrivateKey,                 // Private Key
                                  &pPKIConfig,                       // PKI Config
                                  NbOfSecurityPolicyConfigurations,  // NoOf SecurityPolicies
								  secuConfig);                       // SecurityPolicies
    printf ("%d\n", uStatus);
    OpcUa_GotoErrorIfBad(uStatus);

    while (connectionClosed == 0 && OpcUa_IsGood(uStatus) && loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
#if OPCUA_MULTITHREADED
    	// just wait for callback
    	OpcUa_Thread_Sleep (sleepTimeout);
#else
    	// Retrieve connections / messages until connection is closed by a client
    	uStatus = OpcUa_SocketManager_Loop(OpcUa_Null, sleepTimeout, OpcUa_True);
#endif //OPCUA_MULTITHREADED

    }
    loopCpt = 0;
    if(connected == 0){
        uStatus = OpcUa_Bad;
    }
    OpcUa_GotoErrorIfBad(uStatus);

    OpcUa_ReturnStatusCode;
    OpcUa_BeginErrorHandling;
    printf ("Error status: %d\n", uStatus);
    if(uStatus != 0){
        return -1;
    }
    OpcUa_FinishErrorHandling;
}
