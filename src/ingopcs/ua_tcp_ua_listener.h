/*
 * tcp_ua_listener.H
 *
 *  Created on: Jul 26, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_TCP_UA_LISTENER_H_
#define INGOPCS_TCP_UA_LISTENER_H_

#include <wrappers.h>

typedef struct TCP_UA_Listener {
    UA_String                url;
    uint32_t                 protocolVersion;
    SocketManager            socketManager;
    Socket                   socket;
    TCP_UA_Connection*       clientConnections;
    UA_MsgBuffer*            pendingMsgBuffer;
    TCP_UA_ListenerEvent_CB* eventCB;

} TCP_UA_Listener;


#endif /* INGOPCS_TCP_UA_LISTENER_H_ */
