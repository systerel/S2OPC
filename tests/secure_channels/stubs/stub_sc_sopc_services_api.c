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

#include "stub_sc_sopc_services_api.h"
#include "sopc_services_api.h"

#include <assert.h>
#include <stdlib.h>

SOPC_AsyncQueue* servicesEvents = NULL;

void SOPC_Services_EnqueueEvent(SOPC_Services_Event scEvent, uint32_t id, void* params, uintptr_t auxParam)
{
    SOPC_StubSC_ServicesEventParams* scParams = calloc(1, sizeof(SOPC_StubSC_ServicesEventParams));
    assert(scParams != NULL && servicesEvents != NULL);
    scParams->event = scEvent;
    scParams->eltId = id;
    scParams->params = params;
    scParams->auxParam = auxParam;

    assert(SOPC_STATUS_OK == SOPC_AsyncQueue_BlockingEnqueue(servicesEvents, (void*) scParams));
}

void SOPC_Services_Initialize()
{
    assert(SOPC_STATUS_OK == SOPC_AsyncQueue_Init(&servicesEvents, "StubsSC_ServicesEventQueue"));
}

void SOPC_Services_ToolkitConfigured() {}

void SOPC_Services_PreClear() {}

void SOPC_Services_Clear()
{
    SOPC_AsyncQueue_Free(&servicesEvents);
    servicesEvents = NULL;
}
