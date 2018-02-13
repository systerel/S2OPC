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

#include "sopc_secure_channels_api.h"
#include "sopc_secure_channels_api_internal.h"

#include <assert.h>
#include <stdbool.h>

#include "sopc_chunks_mgr.h"
#include "sopc_event_dispatcher_manager.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_secure_connection_state_mgr.h"
#include "sopc_secure_listener_state_mgr.h"

static SOPC_EventDispatcherManager* secureChannelsEventDispatcherMgr = NULL;

static void SOPC_SecureChannelsEventMgr_Dispatcher(int32_t event, uint32_t eltId, void* params, uintptr_t auxParam)
{
    SOPC_SecureChannels_InputEvent scEvent = event;
    switch (scEvent)
    {
    /* Sockets events: */
    /* Sockets manager -> SC listener state manager */
    case SOCKET_LISTENER_OPENED:
    case SOCKET_LISTENER_CONNECTION:
    case SOCKET_LISTENER_FAILURE:
        SOPC_SecureListenerStateMgr_Dispatcher(scEvent, eltId, params, auxParam);
        break;
    /* Sockets manager -> SC connection state manager */
    case SOCKET_CONNECTION:
    case SOCKET_FAILURE:
        SOPC_SecureConnectionStateMgr_Dispatcher(scEvent, eltId, params, auxParam);
        break;
    /* Sockets manager -> Chunks manager */
    case SOCKET_RCV_BYTES:
        SOPC_ChunksMgr_Dispatcher(scEvent, eltId, params, auxParam);
        break;

    /* Services events: */
    /* Services manager -> SC listener state manager */
    case EP_OPEN:
    case EP_CLOSE:
        SOPC_SecureListenerStateMgr_Dispatcher(scEvent, eltId, params, auxParam);
        break;
    /* Services manager -> SC connection state manager */
    case SC_CONNECT:
    case DEBUG_SC_FORCE_OPN_RENEW:
    case SC_DISCONNECT:
    case SC_SERVICE_SND_MSG:
        SOPC_SecureConnectionStateMgr_Dispatcher(scEvent, eltId, params, auxParam);
        break;

    /* Internal events */
    /* SC listener manager -> SC connection state manager */
    case INT_EP_SC_CREATE:
    case INT_EP_SC_CLOSE:
        SOPC_SecureConnectionStateMgr_Dispatcher(scEvent, eltId, params, auxParam);
        break;

    /* SC connection manager -> SC listener state manager */
    case INT_EP_SC_CREATED:
    case INT_EP_SC_DISCONNECTED:
        SOPC_SecureListenerStateMgr_Dispatcher(scEvent, eltId, params, auxParam);
        break;

    /* OPC UA chunks message manager -> SC connection manager */
    case INT_SC_RCV_HEL:
    case INT_SC_RCV_ACK:
    case INT_SC_RCV_ERR:
    case INT_SC_RCV_OPN:
    case INT_SC_RCV_CLO:
    case INT_SC_RCV_MSG_CHUNKS:
    case INT_SC_RCV_FAILURE:
    case INT_SC_SND_FAILURE:
    case INT_SC_CLOSE:
        SOPC_SecureConnectionStateMgr_Dispatcher(scEvent, eltId, params, auxParam);
        break;

    /* SC connection manager -> OPC UA chunks message manager */
    case INT_SC_SND_HEL:
    case INT_SC_SND_ACK:
    case INT_SC_SND_ERR:
    case INT_SC_SND_OPN:
    case INT_SC_SND_CLO:
    case INT_SC_SND_MSG_CHUNKS:
        SOPC_ChunksMgr_Dispatcher(scEvent, eltId, params, auxParam);
        break;
    default:
        assert(false);
    }
}

void SOPC_SecureChannels_EnqueueEvent(SOPC_SecureChannels_InputEvent scEvent,
                                      uint32_t id,
                                      void* params,
                                      uintptr_t auxParam)
{
    if (NULL != secureChannelsEventDispatcherMgr)
    {
        switch (scEvent)
        {
        /* External events */
        case SOCKET_LISTENER_OPENED:
        case SOCKET_LISTENER_CONNECTION:
        case SOCKET_LISTENER_FAILURE:
        case SOCKET_CONNECTION:
        case SOCKET_FAILURE:
        case SOCKET_RCV_BYTES:
        case EP_OPEN:
        case EP_CLOSE:
        case SC_CONNECT:
        case DEBUG_SC_FORCE_OPN_RENEW:
        case SC_DISCONNECT:
        case SC_SERVICE_SND_MSG:
            SOPC_EventDispatcherManager_AddEvent(secureChannelsEventDispatcherMgr, scEvent, id, params, auxParam, NULL);
            break;
        /* Internal or invalid events*/
        case INT_EP_SC_CREATE:
        case INT_EP_SC_CLOSE:
        case INT_EP_SC_CREATED:
        case INT_EP_SC_DISCONNECTED:
        case INT_SC_RCV_HEL:
        case INT_SC_RCV_ACK:
        case INT_SC_RCV_ERR:
        case INT_SC_RCV_OPN:
        case INT_SC_RCV_CLO:
        case INT_SC_RCV_MSG_CHUNKS:
        case INT_SC_RCV_FAILURE:
        case INT_SC_SND_FAILURE:
        case INT_SC_SND_HEL:
        case INT_SC_SND_ACK:
        case INT_SC_SND_ERR:
        case INT_SC_SND_OPN:
        case INT_SC_SND_CLO:
        case INT_SC_SND_MSG_CHUNKS:
        case INT_SC_CLOSE:
        default:
            assert(false);
        }
    }
}

void SOPC_SecureChannels_EnqueueInternalEvent(SOPC_SecureChannels_InputEvent scEvent,
                                              uint32_t id,
                                              void* params,
                                              uintptr_t auxParam)
{
    if (NULL != secureChannelsEventDispatcherMgr)
    {
        switch (scEvent)
        {
        /* Internal events*/
        case INT_EP_SC_CREATE:
        case INT_EP_SC_CLOSE:
        case INT_EP_SC_CREATED:
        case INT_EP_SC_DISCONNECTED:
        case INT_SC_RCV_HEL:
        case INT_SC_RCV_ACK:
        case INT_SC_RCV_ERR:
        case INT_SC_RCV_OPN:
        case INT_SC_RCV_CLO:
        case INT_SC_RCV_MSG_CHUNKS:
        case INT_SC_RCV_FAILURE:
        case INT_SC_SND_FAILURE:
        case INT_SC_SND_HEL:
        case INT_SC_SND_ACK:
        case INT_SC_SND_ERR:
        case INT_SC_SND_OPN:
        case INT_SC_SND_CLO:
        case INT_SC_SND_MSG_CHUNKS:
        case INT_SC_CLOSE:
            SOPC_EventDispatcherManager_AddEvent(secureChannelsEventDispatcherMgr, scEvent, id, params, auxParam, NULL);
            break;
        /* External or invalid events */
        case SOCKET_LISTENER_OPENED:
        case SOCKET_LISTENER_CONNECTION:
        case SOCKET_LISTENER_FAILURE:
        case SOCKET_CONNECTION:
        case SOCKET_FAILURE:
        case SOCKET_RCV_BYTES:
        case EP_OPEN:
        case EP_CLOSE:
        case SC_CONNECT:
        case DEBUG_SC_FORCE_OPN_RENEW:
        case SC_DISCONNECT:
        case SC_SERVICE_SND_MSG:
        default:
            assert(false);
        }
    }
}

void SOPC_SecureChannels_EnqueueInternalEventAsNext(SOPC_SecureChannels_InputEvent scEvent,
                                                    uint32_t id,
                                                    void* params,
                                                    uintptr_t auxParam)
{
    if (NULL != secureChannelsEventDispatcherMgr)
    {
        switch (scEvent)
        {
        /* Internal events*/
        case INT_EP_SC_CREATE:
        case INT_EP_SC_CLOSE:
        case INT_EP_SC_CREATED:
        case INT_EP_SC_DISCONNECTED:
        case INT_SC_RCV_HEL:
        case INT_SC_RCV_ACK:
        case INT_SC_RCV_ERR:
        case INT_SC_RCV_OPN:
        case INT_SC_RCV_CLO:
        case INT_SC_RCV_MSG_CHUNKS:
        case INT_SC_RCV_FAILURE:
        case INT_SC_SND_FAILURE:
        case INT_SC_SND_HEL:
        case INT_SC_SND_ACK:
        case INT_SC_SND_ERR:
        case INT_SC_SND_OPN:
        case INT_SC_SND_CLO:
        case INT_SC_SND_MSG_CHUNKS:
        case INT_SC_CLOSE:
            SOPC_EventDispatcherManager_AddEventAsNext(secureChannelsEventDispatcherMgr, scEvent, id, params, auxParam,
                                                       NULL);
            break;
        /* External or invalid events */
        case SOCKET_LISTENER_OPENED:
        case SOCKET_LISTENER_CONNECTION:
        case SOCKET_LISTENER_FAILURE:
        case SOCKET_CONNECTION:
        case SOCKET_FAILURE:
        case SOCKET_RCV_BYTES:
        case EP_OPEN:
        case EP_CLOSE:
        case SC_CONNECT:
        case DEBUG_SC_FORCE_OPN_RENEW:
        case SC_DISCONNECT:
        case SC_SERVICE_SND_MSG:
        default:
            assert(false);
        }
    }
}

void SOPC_SecureChannels_Initialize()
{
    SOPC_SecureChannelsInternalContext_Initialize();
    secureChannelsEventDispatcherMgr = SOPC_EventDispatcherManager_CreateAndStart(
        SOPC_SecureChannelsEventMgr_Dispatcher, "Secure channels event manager dispatcher");
}

void SOPC_SecureChannels_Clear()
{
    if (NULL != secureChannelsEventDispatcherMgr)
    {
        SOPC_EventDispatcherManager_StopAndDelete(&secureChannelsEventDispatcherMgr);
    }
    SOPC_SecureChannelsInternalContext_Clear();
}
