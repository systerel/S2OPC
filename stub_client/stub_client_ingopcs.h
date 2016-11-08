/*
 * stub_server.h
 *
 *  Created on: Feb 25, 2016
 *      Author: Vincent Monfort (Systerel)
 */

#ifndef STUB_SERVER_STUB_CLIENT_H_
#define STUB_SERVER_STUB_CLIENT_H_

#include <stdint.h>
#include <ua_channel.h>

typedef struct {
    uint32_t stub;
} StubClient_CallbackData;

SOPC_StatusCode StubClient_ConnectionEvent_Callback(UA_Channel       channel,
                                               void*            callbackData,
                                               SOPC_Channel_Event event,
                                               SOPC_StatusCode       status);

SOPC_StatusCode StubClient_ResponseEvent_Callback(UA_Channel         channel,
                                             void*              response,
                                             SOPC_EncodeableType* responseType,
 										     void*              callbackData,
 										     SOPC_StatusCode         status);

#endif /* STUB_SERVER_STUB_CLIENT_H_ */
