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

/**
 *  \file sopc_sockets_api_internal.h
 *
 *  \brief Event oriented API of the Services layer for internal use only (from Services layer).
 */

#ifndef SOPC_SERVICES_API_INTERNAL_H
#define SOPC_SERVICES_API_INTERNAL_H

#include "sopc_services_api.h"

/* API to enqueue an internal event for services */
void SOPC_Services_InternalEnqueueEvent(SOPC_Services_Event seEvent, uint32_t id, void* params, uintptr_t auxParam);

/* API to enqueue an internal event in priority for services */
void SOPC_Services_InternalEnqueuePrioEvent(SOPC_Services_Event seEvent, uint32_t id, void* params, uintptr_t auxParam);

// Internal use only (timers)
SOPC_EventDispatcherManager* SOPC_Services_GetEventDispatcher(void);

typedef struct SOPC_Internal_AsyncSendMsgData
{
    uint32_t requestId;     // t_request_context
    uint32_t requestHandle; // t_request_handle
    void* msgToSend;        // OpcUa_<Msg> *
} SOPC_Internal_AsyncSendMsgData;

#endif /* SOPC_SERVICES_API_INTERNAL_H */
