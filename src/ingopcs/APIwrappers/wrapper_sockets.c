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

#include "wrapper_sockets.h"

SOPC_StatusCode OpcUa_SocketManager_Create(SOPC_SocketManager* sManager,
                                           uint32_t            nbSockets,
                                           uint32_t            flags){
    (void) sManager;
    (void) nbSockets;
    (void) flags;
    // Done internally in ingopcs: no need to creation outside of the stack
    return STATUS_OK;
}

void OpcUa_SocketManager_Delete(SOPC_SocketManager* sManager){
    (void) sManager;
}

SOPC_StatusCode OpcUa_SocketManager_Loop(SOPC_SocketManager* sManager,
                                         uint32_t            msTimeout,
                                         uint8_t             runOnce){
    SOPC_StatusCode status = STATUS_OK;
    if(sManager == NULL){
#if OPCUA_MULTITHREADED
        return STATUS_NOK;
#else
        sManager = SOPC_SocketManager_GetGlobal();
#endif //OPCUA_MULTITHREADED
    }
    /* the serving loop */
    do
    {
        status |= SOPC_SocketManager_Loop(sManager, msTimeout);
    } while(!runOnce);
    if(STATUS_OK != status){
        status = STATUS_NOK;
    }
    return status;
}
