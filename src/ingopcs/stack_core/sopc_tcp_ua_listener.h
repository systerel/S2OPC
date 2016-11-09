/*
 * tcp_ua_listener.H
 *
 *  Created on: Jul 26, 2016
 *      Author: vincent
 */

#ifndef SOPC_LISTENER_H_
#define SOPC_LISTENER_H_

typedef void* TCP_SOPC_ListenerEvent_CB;

typedef struct {
    SOPC_String                url;
    uint32_t                 protocolVersion;
    SOPC_SocketManager*        socketManager;
    SOPC_Socket*               socket;
    TCP_SOPC_Connection*       clientConnections;
    SOPC_MsgBuffer*            pendingMsgBuffer;
    TCP_SOPC_ListenerEvent_CB* eventCB;

} TCP_SOPC_Listener;


#endif /* SOPC_LISTENER_H_ */
