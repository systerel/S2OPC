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

#include "stub_sockets_sopc_secure_channels_api.h"
#include "sopc_secure_channels_api.h"

#include <assert.h>
#include <stdlib.h>

SOPC_AsyncQueue* secureChannelsEvents = NULL;

void SOPC_SecureChannels_EnqueueEvent(SOPC_SecureChannels_InputEvent scEvent,
                                      uint32_t id,
                                      void* params,
                                      uint32_t auxParam)
{
    SOPC_StubSockets_SecureChannelsEventParams* scParams =
        calloc(1, sizeof(SOPC_StubSockets_SecureChannelsEventParams));
    assert(scParams != NULL && secureChannelsEvents != NULL);
    scParams->event = scEvent;
    scParams->eltId = id;
    scParams->params = params;
    scParams->auxParam = auxParam;

    assert(SOPC_STATUS_OK == SOPC_AsyncQueue_BlockingEnqueue(secureChannelsEvents, (void*) scParams));
}

void SOPC_SecureChannels_Initialize()
{
    assert(SOPC_STATUS_OK == SOPC_AsyncQueue_Init(&secureChannelsEvents, "StubsSockets_SecureChannelEventQueue"));
}

void SOPC_SecureChannels_Clear()
{
    SOPC_AsyncQueue_Free(&secureChannelsEvents);
}
