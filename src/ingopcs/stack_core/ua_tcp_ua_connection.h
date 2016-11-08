/*
 * tcp_ua_connection.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_TCP_SOPC_CONNECTION_H_
#define INGOPCS_TCP_SOPC_CONNECTION_H_

#include <ua_builtintypes.h>
#include <ua_msg_buffer.h>
#include <ua_sockets.h>

#define TCP_SOPC_MIN_BUFFER_SIZE 8192
#define TCP_SOPC_MAX_URL_LENGTH 4096

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

typedef SOPC_StatusCode (TCP_SOPC_Connection_Event_CB) (void*           tcpConnection,
                                                 void*           callbackData,
                                                 ConnectionEvent event,
                                                 SOPC_MsgBuffer*   msgBuffer,
                                                 SOPC_StatusCode      status);

typedef struct {
    SOPC_String                   url;
    uint32_t                    protocolVersion;
    uint32_t                    receivedProtocolVersion;
    uint32_t                    receiveBufferSize;
    uint32_t                    sendBufferSize;
    uint32_t                    maxMessageSizeRcv;
    uint32_t                    maxChunkCountRcv;
    uint32_t                    maxMessageSizeSnd;
    uint32_t                    maxChunkCountSnd;
    TCP_ConnectionState         state;
    SOPC_SocketManager*           socketManager;
    SOPC_Socket*                  socket;
    SOPC_MsgBuffer*               inputMsgBuffer;
    SOPC_MsgBuffer*               outputMsgBuffer;
    SOPC_MsgBuffer*               sendingQueue;
    TCP_SOPC_Connection_Event_CB* callback;
    void*                       callbackData;

} TCP_SOPC_Connection;

TCP_SOPC_Connection* TCP_SOPC_Connection_Create(uint32_t scProtocolVersion);
void TCP_SOPC_Connection_Delete(TCP_SOPC_Connection* connection);

SOPC_StatusCode TCP_SOPC_Connection_Connect(TCP_SOPC_Connection*          connection,
                                     const char*                 uri,
                                     TCP_SOPC_Connection_Event_CB* callback,
                                     void*                       callbackData);
void TCP_SOPC_Connection_Disconnect(TCP_SOPC_Connection* connection);

// return SOPC_FALSE if no protocol version (for non TCP protocol only: must be generic)
uint32_t TCP_SOPC_Connection_GetReceiveProtocolVersion(TCP_SOPC_Connection* connection,
                                                     uint32_t*          protocolVersion);

#endif /* INGOPCS_TCP_SOPC_CONNECTION_H_ */
