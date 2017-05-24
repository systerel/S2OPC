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

#ifndef SOPC_TCP_UA_LISTENER_H_
#define SOPC_TCP_UA_LISTENER_H_

#include "sopc_sockets.h"
#include "sopc_tcp_ua_connection.h"
#include "sopc_msg_buffer.h"


typedef enum TCP_ListenerState
{
    TCP_Listener_Closed,
    TCP_Listener_Opened,
    TCP_Listener_Error
} TCP_ListenerState;

typedef enum {
    TCP_ListenerEvent_Closed,
    TCP_ListenerEvent_Opened,
    TCP_ListenerEvent_Connect,
    TCP_ListenerEvent_Error
} TCP_ListenerEvent;

struct SC_ServerEndpoint;
typedef SOPC_StatusCode (TCP_UA_ListenerEvent_CB) (struct SC_ServerEndpoint* sEndpoint,
                                                   TCP_ListenerEvent         event,
                                                   SOPC_StatusCode           status,
                                                   TCP_UA_Connection*        newTcpConnection);


typedef struct TCP_UA_Listener{
    SOPC_String                   url;
    uint32_t                      protocolVersion;
    TCP_ListenerState             state;
    SOPC_SocketManager*           socketManager;
    SOPC_Socket*                  socket;
//    SOPC_MsgBuffer*               pendingMsgBuffer;
    TCP_UA_ListenerEvent_CB*      callback;
    struct SC_ServerEndpoint*     callbackData;
} TCP_UA_Listener;

TCP_UA_Listener* TCP_UA_Listener_Create(uint32_t scProtocolVersion);
void TCP_UA_Listener_Delete(TCP_UA_Listener* listener);

SOPC_StatusCode TCP_UA_Listener_Open(TCP_UA_Listener*             listener,
                                     const char*                  uri,
                                     TCP_UA_ListenerEvent_CB*     callback,
                                     struct SC_ServerEndpoint*    callbackData);

void TCP_UA_Listener_Close(TCP_UA_Listener* listener);

#endif /* SOPC_TCP_UA_LISTENER_H_ */
