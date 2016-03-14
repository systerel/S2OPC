/*
 * stub_server.h
 *
 *  Created on: Feb 25, 2016
 *      Author: vincent
 */

#ifndef STUB_SERVER_STUB_CLIENT_H_
#define STUB_SERVER_STUB_CLIENT_H_

#include <opcua_proxystub.h>
#include <opcua_channel.h>
#include <opcua_clientapi.h>
#include <opcua_string.h>
#include <opcua_core.h>
#include <opcua.h>
#include <opcua_p_socket_interface.h>
#include <unistd.h>

typedef struct {
OpcUa_Int Stub;
} StubClient_CallbackData;

OpcUa_StatusCode StubClient_ConnectionEvent_Callback(OpcUa_Channel                   hChannel,
													 OpcUa_Void*                     pCallbackData,
													 OpcUa_Channel_Event             eEvent,
													 OpcUa_StatusCode                uStatus);
													 //OpcUa_Channel_SecurityToken*    pSecurityToken);


OpcUa_StatusCode StubClient_ResponseEvent_Callback(OpcUa_Channel         hChannel,
												   OpcUa_Void*           pResponse,
												   OpcUa_EncodeableType* pResponseType,
												   OpcUa_Void*           pCallbackData,
												   OpcUa_StatusCode      uStatus);

#endif /* STUB_SERVER_STUB_CLIENT_H_ */
