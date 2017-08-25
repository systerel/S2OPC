/*
 *  Copyright (C) 2017 Systerel and others.
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

#include "sopc_sockets_api.h"

#include <assert.h>

#include "sopc_event_dispatcher_manager.h"
#include "sopc_raw_sockets.h"
#include "sopc_sockets_internal_ctx.h"
#include "sopc_sockets_network_event_mgr.h"
#include "sopc_sockets_event_mgr.h"

static SOPC_EventDispatcherManager* socketsEventDispatcherMgr = NULL;


void SOPC_Sockets_EnqueueEvent(SOPC_Sockets_InputEvent socketEvent,
                               uint32_t                id,
                               void*                   params,
                               int32_t                 auxParam){
    if(NULL != socketsEventDispatcherMgr){
        SOPC_EventDispatcherManager_AddEvent(socketsEventDispatcherMgr,
                                             socketEvent,
                                             id,
                                             params,
                                             auxParam,
                                             NULL);
    }
}

void SOPC_Sockets_Initialize(){
    assert(STATUS_OK == Socket_Network_Initialize());
    SOPC_SocketsInternalContext_Initialize();
    socketsEventDispatcherMgr =
            SOPC_EventDispatcherManager_CreateAndStart(SOPC_SocketsEventMgr_Dispatcher,
                                                       "Sockets event manager dispatcher");
    SOPC_SocketsNetworkEventMgr_Initialize();
}

void SOPC_Sockets_Clear(){
    SOPC_SocketsNetworkEventMgr_Clear();
    if(NULL != socketsEventDispatcherMgr){
        SOPC_EventDispatcherManager_StopAndDelete(&socketsEventDispatcherMgr);
    }
    SOPC_SocketsInternalContext_Clear();
    Socket_Network_Clear();
}
