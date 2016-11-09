/*
 * stub_server.h
 *
 *  Created on: Feb 25, 2016
 *      Author: Vincent Monfort (Systerel)
 */

#ifndef STUB_CLIENT_INGOPCS_H_
#define STUB_CLIENT_INGOPCS_H_

#include <stdint.h>

#include "sopc_channel.h"

typedef struct {
    uint32_t stub;
} StubClient_CallbackData;

SOPC_StatusCode StubClient_ConnectionEvent_Callback(SOPC_Channel       channel,
                                                    void*            callbackData,
                                                    SOPC_Channel_Event event,
                                                    SOPC_StatusCode       status);

SOPC_StatusCode StubClient_ResponseEvent_Callback(SOPC_Channel         channel,
                                                  void*              response,
                                                  SOPC_EncodeableType* responseType,
 										          void*              callbackData,
 										          SOPC_StatusCode         status);

#endif /* STUB_CLIENT_INGOPCS_H_ */
