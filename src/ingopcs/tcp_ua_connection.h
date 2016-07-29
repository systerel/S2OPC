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

typedef enum TCP_Connection_State
{
    TCP_Connection_Connecting,
    TCP_Connection_Connected,
    TCP_Connection_Disconnected,
    TCP_Connection_Error
} TCP_Connection_State;

typedef StatusCode (TCP_UA_Connection_Event_CB) (void*            connection,
                                                 void*            callbackData,
                                                 ConnectionEvent  event,
                                                 UA_Msg_Buffer*   msgBuffer,
                                                 StatusCode       status);

typedef struct TCP_UA_Connection {
    UA_String*                  url;
    uint32_t                    protocolVersion;
    uint32_t                    receiveBufferSize;
    uint32_t                    sendBufferSize;
    uint32_t                    maxMessageSize;
    uint32_t                    maxChunkCount;
    TCP_Connection_State        state;
    Socket_Manager              socketManager;
    Socket                      socket;
    UA_Msg_Buffer*              inputMsgBuffer;
    UA_Msg_Buffer*              outputMsgBuffer;
    UA_Msg_Buffer*              sendingQueue;
    TCP_UA_Connection_Event_CB* callback;
    void*                       callbackData;

} TCP_UA_Connection;

TCP_UA_Connection* Create_Connection(void);
void Delete_Connection(TCP_UA_Connection* connection);

StatusCode Connect_Transport (TCP_UA_Connection*          connection,
                              char*                       uri,
                              TCP_UA_Connection_Event_CB* callback,
                              void*                       callbackData);

#endif /* INGOPCS_TCP_UA_CONNECTION_H_ */
