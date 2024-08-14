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

#include "sopc_secure_channels_api.h"
#include "sopc_secure_channels_api_internal.h"

#include <stdbool.h>

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_chunks_mgr.h"
#include "sopc_macros.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_secure_connection_state_mgr.h"
#include "sopc_secure_listener_state_mgr.h"
#include "sopc_sockets_api.h"

void SOPC_SecureChannels_OnInternalEvent(SOPC_EventHandler* handler,
                                         int32_t event,
                                         uint32_t eltId,
                                         uintptr_t params,
                                         uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);
    SOPC_SecureChannels_InternalEvent internalEvent = (SOPC_SecureChannels_InternalEvent) event;

    switch (internalEvent)
    {
    /* SC listener manager -> SC connection state manager */
    case INT_EP_SC_CREATE:
    case INT_EP_SC_CLOSE:
    case INT_EP_SC_REVERSE_CONNECT:
    case INT_SC_RCV_RHE_TRANSITION:
        SOPC_SecureConnectionStateMgr_OnInternalEvent(internalEvent, eltId, params, auxParam);
        break;

    /* SC connection manager -> SC listener state manager */
    case INT_EP_SC_CREATED:
    case INT_EP_SC_RHE_DECODED:
    case INT_EP_SC_DISCONNECTED:
    case INT_REVERSE_EP_REQ_CONNECTION:
        SOPC_SecureListenerStateMgr_OnInternalEvent(internalEvent, eltId, params, auxParam);
        break;

    /* OPC UA chunks message manager -> SC connection manager */
    case INT_SC_RCV_HEL:
    case INT_SC_RCV_ACK:
    case INT_SC_RCV_ERR:
    case INT_SC_RCV_RHE:
    case INT_SC_RCV_OPN:
    case INT_SC_RCV_CLO:
    case INT_SC_RCV_MSG_CHUNKS:
    case INT_SC_RCV_MSG_CHUNK_ABORT:
    case INT_SC_RCV_FAILURE:
    case INT_SC_SND_FATAL_FAILURE:
    case INT_SC_SENT_ABORT_FAILURE:
    case INT_SC_CLOSE:
        SOPC_SecureConnectionStateMgr_OnInternalEvent(internalEvent, eltId, params, auxParam);
        break;

    /* SC connection manager -> OPC UA chunks message manager */
    case INT_SC_SND_HEL:
    case INT_SC_SND_ACK:
    case INT_SC_SND_ERR:
    case INT_SC_SND_RHE:
    case INT_SC_SND_OPN:
    case INT_SC_SND_CLO:
    case INT_SC_SND_MSG_CHUNKS:
        SOPC_ChunksMgr_Dispatcher(internalEvent, eltId, params, auxParam);
        break;
    default:
        SOPC_ASSERT(false && "Unknown internal event.");
        break;
    }
}

void SOPC_SecureChannels_OnSocketsEvent(SOPC_EventHandler* handler,
                                        int32_t event,
                                        uint32_t eltId,
                                        uintptr_t params,
                                        uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);
    SOPC_Sockets_OutputEvent sockEvent = (SOPC_Sockets_OutputEvent) event;

    switch (sockEvent)
    {
    /* Sockets events: */
    /* Sockets manager -> SC listener state manager */
    case SOCKET_LISTENER_OPENED:
    case SOCKET_LISTENER_CONNECTION:
    case SOCKET_LISTENER_FAILURE:
        SOPC_SecureListenerStateMgr_OnSocketEvent(sockEvent, eltId, params, auxParam);
        break;
    /* Sockets manager -> SC connection state manager */
    case SOCKET_CREATED:
    case SOCKET_CONNECTION:
    case SOCKET_FAILURE:
        SOPC_SecureConnectionStateMgr_OnSocketEvent(sockEvent, eltId, params, auxParam);
        break;
    /* Sockets manager -> Chunks manager */
    case SOCKET_RCV_BYTES:
        SOPC_ChunksMgr_OnSocketEvent(sockEvent, eltId, params, auxParam);
        break;
    default:
        SOPC_ASSERT(false && "Unknown socket event.");
    }
}

void SOPC_SecureChannels_OnTimerEvent(SOPC_EventHandler* handler,
                                      int32_t event,
                                      uint32_t id,
                                      uintptr_t params,
                                      uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);

    SOPC_SecureChannels_TimerEvent timerEvent = (SOPC_SecureChannels_TimerEvent) event;
    switch (timerEvent)
    {
    /* Secure connection state manager */
    case TIMER_SC_CONNECTION_TIMEOUT:
    case TIMER_SC_SERVER_REVERSE_CONN_RETRY:
    case TIMER_SC_CLIENT_OPN_RENEW:
    case TIMER_SC_REQUEST_TIMEOUT:
        SOPC_SecureConnectionStateMgr_OnTimerEvent(timerEvent, id, params, auxParam);
        break;
    /* Secure listener state manager */
    case TIMER_SC_RHE_RECEPTION_TIMEOUT:
        SOPC_SecureListenerStateMgr_OnTimerEvent(timerEvent, id, params, auxParam);
        break;
    default:
        SOPC_ASSERT(false && "Unknown timer event.");
    }
}

void SOPC_SecureChannels_OnInputEvent(SOPC_EventHandler* handler,
                                      int32_t event,
                                      uint32_t eltId,
                                      uintptr_t params,
                                      uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(handler);

    SOPC_SecureChannels_InputEvent scEvent = (SOPC_SecureChannels_InputEvent) event;
    switch (scEvent)
    {
    /* Services events: */
    /* Services manager -> SC listener state manager */
    case EP_OPEN:
    case EP_CLOSE:
    case REVERSE_EP_OPEN:
    case REVERSE_EP_CLOSE:
        SOPC_SecureListenerStateMgr_Dispatcher(scEvent, eltId, params, auxParam);
        break;
    /* Services manager -> SC connection state manager */
    case SC_CONNECT:
    case SC_REVERSE_CONNECT:
    case SC_DISCONNECT:
    case SC_SERVICE_SND_MSG:
    case SC_SERVICE_SND_ERR:
    case SC_DISCONNECTED_ACK:
    case SCS_REEVALUATE_SCS:
        SOPC_SecureConnectionStateMgr_Dispatcher(scEvent, eltId, params, auxParam);
        break;
    default:
        SOPC_ASSERT(false && "Unknown input event.");
    }
}

SOPC_ReturnStatus SOPC_SecureChannels_EnqueueEvent(SOPC_SecureChannels_InputEvent scEvent,
                                                   uint32_t id,
                                                   uintptr_t params,
                                                   uintptr_t auxParam)
{
    SOPC_ASSERT(secureChannelsInputEventHandler != NULL);
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    switch (scEvent)
    {
    /* External events */
    case EP_OPEN:
    case EP_CLOSE:
    case REVERSE_EP_OPEN:
    case REVERSE_EP_CLOSE:
    case SC_CONNECT:
    case SC_REVERSE_CONNECT:
    case SC_DISCONNECT:
    case SC_SERVICE_SND_MSG:
    case SC_SERVICE_SND_ERR:
    case SC_DISCONNECTED_ACK:
    case SCS_REEVALUATE_SCS:
        status = SOPC_EventHandler_Post(secureChannelsInputEventHandler, (int32_t) scEvent, id, params, auxParam);
        break;
    default:
        SOPC_ASSERT(false && "Unknown event.");
    }
    return status;
}

void SOPC_SecureChannels_EnqueueInternalEventAsNext(SOPC_SecureChannels_InternalEvent event,
                                                    uint32_t id,
                                                    uintptr_t params,
                                                    uintptr_t auxParam)
{
    SOPC_ASSERT(secureChannelsInternalEventHandler != NULL);
    SOPC_EventHandler_PostAsNext(secureChannelsInternalEventHandler, (int32_t) event, id, params, auxParam);
}

uint32_t SOPC_SecureChannels_Get_QueueSize(void)
{
    return SOPC_EventHandler_Get_QueueSize(secureChannelsEventHandler) +
           SOPC_EventHandler_Get_QueueSize(secureChannelsInternalEventHandler);

    /*
        return SOPC_EventHandler_Get_QueueSize(secureChannelsInputEventHandler) +
                SOPC_EventHandler_Get_QueueSize(secureChannelsInternalEventHandler) +
                SOPC_EventHandler_Get_QueueSize(secureChannelsSocketsEventHandler) +
                SOPC_EventHandler_Get_QueueSize(secureChannelsTimerEventHandler) +
                SOPC_EventHandler_Get_QueueSize(secureChannelsEventHandler);*/
}

void SOPC_SecureChannels_Initialize(SOPC_SetListenerFunc* setSocketsListener)
{
    SOPC_SecureChannelsInternalContext_Initialize(setSocketsListener);
}

void SOPC_SecureChannels_SetEventHandler(SOPC_EventHandler* handler)
{
    SOPC_Atomic_Ptr_Set((void**) &secureChannelsEventHandler, handler);
}

void SOPC_SecureChannels_Clear(void)
{
    SOPC_SecureChannelsInternalContext_Clear();
}
