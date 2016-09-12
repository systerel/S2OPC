/*
 * tcp_ua_connection.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_TCP_UA_CONNECTION_H_
#define INGOPCS_TCP_UA_CONNECTION_H_

#include <wrappers.h>
#include <ua_builtintypes.h>

#include <ua_msg_buffer.h>

#define TCP_UA_MIN_BUFFER_SIZE 8192
#define TCP_UA_MAX_URL_LENGTH 4096

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

typedef StatusCode (TCP_UA_Connection_Event_CB) (void*           tcpConnection,
                                                 void*           callbackData,
                                                 ConnectionEvent event,
                                                 UA_MsgBuffer*   msgBuffer,
                                                 StatusCode      status);

typedef struct {
    UA_String*                  url;
    uint32_t                    protocolVersion;
    uint32_t                    receivedProtocolVersion;
    uint32_t                    receiveBufferSize;
    uint32_t                    sendBufferSize;
    uint32_t                    maxMessageSizeRcv;
    uint32_t                    maxChunkCountRcv;
    uint32_t                    maxMessageSizeSnd;
    uint32_t                    maxChunkCountSnd;
    TCP_ConnectionState         state;
    SocketManager               socketManager;
    Socket                      socket;
    UA_MsgBuffer*               inputMsgBuffer;
    UA_MsgBuffer*               outputMsgBuffer;
    UA_MsgBuffer*               sendingQueue;
    TCP_UA_Connection_Event_CB* callback;
    void*                       callbackData;

} TCP_UA_Connection;

TCP_UA_Connection* TCP_UA_Connection_Create(uint32_t scProtocolVersion);
void TCP_UA_Connection_Delete(TCP_UA_Connection* connection);

StatusCode TCP_UA_Connection_Connect(TCP_UA_Connection*          connection,
                                     char*                       uri,
                                     TCP_UA_Connection_Event_CB* callback,
                                     void*                       callbackData);
void TCP_UA_Connection_Disconnect(TCP_UA_Connection* connection);

// return UA_FALSE if no protocol version (for non TCP protocol only: must be generic)
uint32_t TCP_UA_Connection_GetReceiveProtocolVersion(TCP_UA_Connection* connection,
                                                     uint32_t*          protocolVersion);

#endif /* INGOPCS_TCP_UA_CONNECTION_H_ */
