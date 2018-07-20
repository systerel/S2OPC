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

#include "stub_sc_sopc_services_api.h"
#include "sopc_services_api.h"

#include <assert.h>
#include <stdlib.h>

SOPC_AsyncQueue* servicesEvents = NULL;

void SOPC_Services_EnqueueEvent(SOPC_Services_Event scEvent, uint32_t id, void* params, uintptr_t auxParam)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StubSC_ServicesEventParams* scParams = calloc(1, sizeof(SOPC_StubSC_ServicesEventParams));
    assert(scParams != NULL && servicesEvents != NULL);
    scParams->event = scEvent;
    scParams->eltId = id;
    scParams->params = params;
    scParams->auxParam = auxParam;

    status = SOPC_AsyncQueue_BlockingEnqueue(servicesEvents, (void*) scParams);
    (void) status; // status is not used if asserts are not compiled in
    assert(status == SOPC_STATUS_OK);
}

void SOPC_Services_Initialize()
{
    SOPC_ReturnStatus status = SOPC_AsyncQueue_Init(&servicesEvents, "StubsSC_ServicesEventQueue");
    (void) status; // status is not used if asserts are not compiled in
    assert(status == SOPC_STATUS_OK);
}

void SOPC_Services_ToolkitConfigured() {}

void SOPC_Services_PreClear() {}

void SOPC_Services_Clear()
{
    SOPC_AsyncQueue_Free(&servicesEvents);
    servicesEvents = NULL;
}
