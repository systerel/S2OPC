/*
 * stub_server.h
 *
 *  Created on: Feb 25, 2016
 *      Author: Vincent Monfort (Systerel)
 */

#ifndef STUB_SERVER_STUB_SERVER_H_
#define STUB_SERVER_STUB_SERVER_H_

#include <opcua_proxystub.h>
#include <opcua_endpoint.h>
#include <opcua_errorhandling.h>
#include <opcua_trace.h>
#include <opcua_core.h>
#include <opcua_p_socket_interface.h>
#include <opcua_securechannel_types.h>
#include <opcua.h>

OpcUa_ServiceType* StubServer_SupportedServices[0];

typedef struct {
OpcUa_Int Stub;
} StubServer_Endpoint;

OpcUa_StatusCode StubServer_EndpointEvent_Callback(OpcUa_Endpoint          a_hEndpoint,
											       OpcUa_Void             *a_pCallbackData,
											       OpcUa_Endpoint_Event    a_eEvent,
											       OpcUa_StatusCode        a_uStatus,
											       OpcUa_UInt32            a_uSecureChannelId,
											       OpcUa_ByteString       *a_pbsClientCertificate,
											       OpcUa_String           *a_pSecurityPolicy,
											       OpcUa_UInt16            a_uSecurityMode);

#endif /* STUB_SERVER_STUB_SERVER_H_ */
