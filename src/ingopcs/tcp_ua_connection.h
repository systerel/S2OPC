/*
 * tcp_ua_connection.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_TCP_UA_CONNECTION_H_
#define INGOPCS_TCP_UA_CONNECTION_H_

#include <wrappers.h>

#include <opcua_ingopcs_types.h>
#include <msg_buffer.h>

#define TCP_UA_MIN_BUFFER_SIZE 8192
#define TCP_UA_MAX_URL_LENGTH 4096

typedef enum TCP_Connection_State
{
    TCP_Connection_Connecting,
    TCP_Connection_Connected,
    TCP_Connection_Disconnected,
    TCP_Connection_Error
} TCP_Connection_State;

typedef enum Connection_Event{
    ConnectionEvent_Connected,

//    ConnectionEvent_Reconnecting, => see what to do with #3371, not really clear

    ConnectionEvent_Disconnected,

    ConnectionEvent_Message,

    ConnectionEvent_Error
} Connection_Event;

typedef StatusCode (TCP_UA_Connection_Event_CB) (void*            tcpConnection,
                                                 void*            callbackData,
                                                 Connection_Event event,
                                                 UA_Msg_Buffer*   msgBuffer,
                                                 StatusCode       status);

typedef struct TCP_UA_Connection {
    UA_String*                  url;
    uint32_t                    protocolVersion;
    uint32_t                    receivedProtocolVersion;
    uint32_t                    receiveBufferSize;
    uint32_t                    sendBufferSize;
    uint32_t                    maxMessageSizeRcv;
    uint32_t                    maxChunkCountRcv;
    uint32_t                    maxMessageSizeSnd;
    uint32_t                    maxChunkCountSnd;
    TCP_Connection_State        state;
    Socket_Manager              socketManager;
    Socket                      socket;
    UA_Msg_Buffer*              inputMsgBuffer;
    UA_Msg_Buffer*              outputMsgBuffer;
    UA_Msg_Buffer*              sendingQueue;
    TCP_UA_Connection_Event_CB* callback;
    void*                       callbackData;

} TCP_UA_Connection;

TCP_UA_Connection* Create_Connection(uint32_t scProtocolVersion);
void Delete_Connection(TCP_UA_Connection* connection);

StatusCode Connect_Transport(TCP_UA_Connection*          connection,
                             char*                       uri,
                             TCP_UA_Connection_Event_CB* callback,
                             void*                       callbackData);
void Disconnect_Transport(TCP_UA_Connection* connection);

StatusCode Initiate_Send_Message(TCP_UA_Connection* connection);

StatusCode Send_Hello_Msg(TCP_UA_Connection* connection);
StatusCode Receive_Ack_Msg(TCP_UA_Connection* connection);
StatusCode Receive_Error_Msg(TCP_UA_Connection* connection);

// return UA_FALSE if no protocol version (for non TCP protocol only: must be generic)
uint32_t Get_Rcv_Protocol_Version(TCP_UA_Connection* connection,
                                  uint32_t*          protocolVersion);

#endif /* INGOPCS_TCP_UA_CONNECTION_H_ */
