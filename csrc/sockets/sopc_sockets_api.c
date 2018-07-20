/*
 *  Copyright (C) 2018 Systerel and others.
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
#ifdef __TRUSTINSOFT_DEBUG__
#include <stdio.h>
#endif

#include "sopc_raw_sockets.h"
#include "sopc_sockets_event_mgr.h"
#include "sopc_sockets_internal_ctx.h"
#include "sopc_sockets_network_event_mgr.h"

static SOPC_EventDispatcherManager* socketsEventDispatcherMgr = NULL;

#ifdef __TRUSTINSOFT_NO_MTHREAD__
// TIS_Sockets_Dispatch function to call EventDispatcherManager
void* SOPC_ThreadStartEventDispatcherManager(void* pEventMgr);
void TIS_Sockets_Dispatch (void) {
  SOPC_ThreadStartEventDispatcherManager (socketsEventDispatcherMgr);
}
#endif
SOPC_EventDispatcherManager* SOPC_Sockets_GetEventDispatcher()
{
    return socketsEventDispatcherMgr;
}

void SOPC_Sockets_EnqueueEvent(SOPC_Sockets_InputEvent socketEvent, uint32_t id, void* params, uintptr_t auxParam)
{
    if (NULL != socketsEventDispatcherMgr)
    {
        SOPC_EventDispatcherManager_AddEvent(socketsEventDispatcherMgr, socketEvent, id, params, auxParam, NULL);
    }
#ifdef __TRUSTINSOFT_DEBUG__
    printf ("TIS:: SOPC_Sockets_EnqueueEvent: id=%u - SOPC_Sockets_InputEvent=%d\n", id, (int)socketEvent);
#endif
}

void SOPC_Sockets_Initialize()
{
    bool init = Socket_Network_Initialize();
    assert(true == init);
    SOPC_SocketsInternalContext_Initialize();
    socketsEventDispatcherMgr =
        SOPC_EventDispatcherManager_CreateAndStart(SOPC_SocketsEventMgr_Dispatcher, "Sockets event manager dispatcher");
    SOPC_SocketsNetworkEventMgr_Initialize();
}

void SOPC_Sockets_Clear()
{
    SOPC_SocketsNetworkEventMgr_Clear();
    if (NULL != socketsEventDispatcherMgr)
    {
        SOPC_EventDispatcherManager_StopAndDelete(&socketsEventDispatcherMgr);
    }
    SOPC_SocketsInternalContext_Clear();
    Socket_Network_Clear();
}
