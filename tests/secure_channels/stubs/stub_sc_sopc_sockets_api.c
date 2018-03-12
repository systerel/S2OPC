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

#include "stub_sc_sopc_sockets_api.h"
#include "sopc_services_api.h"

#include <assert.h>
#include <stdlib.h>

SOPC_AsyncQueue* socketsEvents = NULL;

void SOPC_Sockets_EnqueueEvent(SOPC_Sockets_InputEvent scEvent, uint32_t id, void* params, uintptr_t auxParam)
{
    SOPC_StubSC_SocketsEventParams* scParams = calloc(1, sizeof(SOPC_StubSC_SocketsEventParams));
    assert(scParams != NULL && socketsEvents != NULL);
    scParams->event = scEvent;
    scParams->eltId = id;
    scParams->params = params;
    scParams->auxParam = auxParam;

    assert(SOPC_STATUS_OK == SOPC_AsyncQueue_BlockingEnqueue(socketsEvents, (void*) scParams));
}

void SOPC_Sockets_Initialize()
{
    assert(SOPC_STATUS_OK == SOPC_AsyncQueue_Init(&socketsEvents, "StubsSC_SocketsEventQueue"));
}

void SOPC_Sockets_Clear()
{
    SOPC_AsyncQueue_Free(&socketsEvents);
    socketsEvents = NULL;
}
