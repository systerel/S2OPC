/*
 * stub_server.h
 *
 *  Created on: Feb 25, 2016
 *      Author: Vincent Monfort (Systerel)
 */

#ifndef STUB_SERVER_STUB_CLIENT_H_
#define STUB_SERVER_STUB_CLIENT_H_

#include <ua_channel.h>

typedef struct {
    OpcUa_Int stub;
} StubClient_CallbackData;

OpcUa_StatusCode StubClient_ConnectionEvent_Callback(UA_Channel       channel,
													 void*            callbackData,
													 UA_Channel_Event event,
													 StatusCode       status);

OpcUa_StatusCode StubClient_ResponseEvent_Callback(UA_Channel         channel,
                                                   void*              response,
												   UA_EncodeableType* responseType,
												   void*              callbackData,
												   StatusCode         status);

#endif /* STUB_SERVER_STUB_CLIENT_H_ */
