/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "sopc_sockets_api.h"

#include <assert.h>

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_raw_sockets.h"
#include "sopc_sockets_event_mgr.h"
#include "sopc_sockets_internal_ctx.h"
#include "sopc_sockets_network_event_mgr.h"

void SOPC_Sockets_EnqueueEvent(SOPC_Sockets_InputEvent socketEvent, uint32_t id, uintptr_t params, uintptr_t auxParam)
{
    if (SOPC_STATUS_OK == SOPC_Sockets_EnqueueInputEvent(socketEvent, id, params, auxParam))
    {
        SOPC_SocketsNetworkEventMgr_InterruptForInputEvent();
    }
}

void SOPC_Sockets_Initialize(void)
{
    bool init = SOPC_Socket_Network_Initialize();
    SOPC_ASSERT(true == init);
    SOPC_SocketsInternalContext_Initialize();
    SOPC_SocketsNetworkEventMgr_Initialize();
}

void SOPC_Sockets_SetEventHandler(SOPC_EventHandler* handler)
{
    SOPC_Atomic_Ptr_Set((void**) &socketsEventHandler, handler);
}

void SOPC_Sockets_Clear(void)
{
    SOPC_SocketsNetworkEventMgr_Clear();
    SOPC_SocketsInternalContext_Clear();
    SOPC_Socket_Network_Clear();
}
