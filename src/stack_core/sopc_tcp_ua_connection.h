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

#ifndef SOPC_TCP_UA_CONNECTION_H_
#define SOPC_TCP_UA_CONNECTION_H_

#include "sopc_msg_buffer.h"
#include "sopc_sockets.h"
#include "sopc_builtintypes.h"

#define TCP_UA_MIN_BUFFER_SIZE 8192

typedef enum TCP_ConnectionState
{
    TCP_Connection_Connecting,
    TCP_Connection_Connected,
    TCP_Connection_Disconnected,
    TCP_Connection_Error
} TCP_ConnectionState;

typedef enum {
    ConnectionEvent_Connected,

//    ConnectionEvent_Reconnecting, => see what to do with #3371, not really clear

    ConnectionEvent_Disconnected,

    ConnectionEvent_Message,

    ConnectionEvent_Error
} ConnectionEvent;

typedef SOPC_StatusCode (TCP_UA_Connection_Event_CB) (void*           tcpConnection,
                                                      void*           callbackData,
                                                      ConnectionEvent event,
                                                      SOPC_MsgBuffer* msgBuffer,
                                                      SOPC_StatusCode status);

typedef struct {
    SOPC_String                 url;
    uint32_t                    protocolVersion;
    uint32_t                    receivedProtocolVersion;
    uint32_t                    receiveBufferSize;
    uint32_t                    sendBufferSize;
    uint32_t                    maxMessageSizeRcv;
    uint32_t                    maxChunkCountRcv;
    uint32_t                    maxMessageSizeSnd;
    uint32_t                    maxChunkCountSnd;
    TCP_ConnectionState         state;
    SOPC_SocketManager*         socketManager;
    SOPC_Socket*                socket;
    SOPC_MsgBuffer*             inputMsgBuffer;
    SOPC_MsgBuffer*             outputMsgBuffer;
    SOPC_MsgBuffer*             sendingQueue;
    TCP_UA_Connection_Event_CB* callback;
    void*                       callbackData;

} TCP_UA_Connection;

TCP_UA_Connection* TCP_UA_Connection_Create(uint32_t scProtocolVersion);
void TCP_UA_Connection_Delete(TCP_UA_Connection* connection);

SOPC_StatusCode TCP_UA_Connection_Connect(TCP_UA_Connection*          connection,
                                          const char*                 uri,
                                          TCP_UA_Connection_Event_CB* callback,
                                          void*                       callbackData);
void TCP_UA_Connection_Disconnect(TCP_UA_Connection* connection);

// return FALSE if no protocol version (for non TCP protocol only: must be generic)
uint32_t TCP_UA_Connection_GetReceiveProtocolVersion(TCP_UA_Connection* connection,
                                                     uint32_t*          protocolVersion);

#endif /* SOPC_TCP_UA_CONNECTION_H_ */
