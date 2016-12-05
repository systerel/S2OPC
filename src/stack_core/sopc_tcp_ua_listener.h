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

typedef void* TCP_UA_ListenerEvent_CB;

typedef struct {
    SOPC_String              url;
    uint32_t                 protocolVersion;
    SOPC_SocketManager*      socketManager;
    SOPC_Socket*             socket;
    TCP_UA_Connection*       clientConnections;
    SOPC_MsgBuffer*          pendingMsgBuffer;
    TCP_UA_ListenerEvent_CB* eventCB;

} TCP_UA_Listener;


#endif /* SOPC_TCP_UA_LISTENER_H_ */
