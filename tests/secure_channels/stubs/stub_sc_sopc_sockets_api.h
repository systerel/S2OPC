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

#ifndef STUBS_SC_SOPC_SOCKETS_API_H_
#define STUBS_SC_SOPC_SOCKETS_API_H_

#include "sopc_async_queue.h"
#include "sopc_sockets_api.h"

void SOPC_Sockets_Initialize(void);

void SOPC_Sockets_Clear(void);

typedef struct SOPC_StubSC_SocketsEventParams
{
    SOPC_Sockets_InputEvent event;
    uint32_t eltId;
    void* params;
    uintptr_t auxParam;
} SOPC_StubSC_SocketsEventParams;

// Async queue simulating the services dispatcher and containing SOPC_StubSC_ServicesEventParams elements
extern SOPC_AsyncQueue* socketsEvents;

#endif /* STUBS_SC_SOPC_SOCKETS_API_H_ */
