/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
