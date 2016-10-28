/*
 * stub_server.h
 *
 *  Created on: Feb 25, 2016
 *      Author: Vincent Monfort (Systerel)
 */

#ifndef STUB_SERVER_STUB_CLIENT_H_
#define STUB_SERVER_STUB_CLIENT_H_

#include <opcua_channel.h>
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
