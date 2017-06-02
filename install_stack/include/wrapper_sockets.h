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

#ifndef SOPC_WRAPPER_SOCKETS_H_
#define SOPC_WRAPPER_SOCKETS_H_

#include "sopc_base_types.h"
#include "sopc_sockets.h"

BEGIN_EXTERN_C

SOPC_StatusCode OpcUa_P_SocketManager_Create(SOPC_SocketManager* sManager,
                                             uint32_t            nbSockets,
                                             uint32_t            flags);

void OpcUa_P_SocketManager_Delete(SOPC_SocketManager* sManager);

SOPC_StatusCode OpcUa_P_SocketManager_ServeLoop(SOPC_SocketManager* sManager,
                                                uint32_t            msTimeout,
                                                uint8_t             runOnce);


void OpcUa_P_Socket_InetAddr();
void OpcUa_P_SocketManager_CreateServer();
void OpcUa_P_SocketManager_CreateClient();
void OpcUa_P_SocketManager_CreateSslServer();
void OpcUa_P_SocketManager_CreateSslClient();
void OpcUa_P_SocketManager_SignalEvent();
void OpcUa_P_Socket_Read();
void OpcUa_P_Socket_Write();
void OpcUa_P_Socket_Close();
void OpcUa_P_Socket_GetPeerInfo();
void OpcUa_P_Socket_GetLastError();
void OpcUa_P_Socket_SetUserData();
void OpcUa_P_Socket_InitializeNetwork();
void OpcUa_P_Socket_CleanupNetwork();

END_EXTERN_C

#endif /* SOPC_WRAPPER_SOCKETS_H_ */
