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

#ifndef STUB_SERVER_STUB_CLIENT_H_
#define STUB_SERVER_STUB_CLIENT_H_

#include <unistd.h>

#include "opcua_channel.h"

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
