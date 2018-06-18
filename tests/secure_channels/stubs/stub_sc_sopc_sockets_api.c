/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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

#include "stub_sc_sopc_sockets_api.h"
#include "sopc_services_api.h"

#include <assert.h>
#include <stdlib.h>

SOPC_AsyncQueue* socketsEvents = NULL;

void SOPC_Sockets_EnqueueEvent(SOPC_Sockets_InputEvent scEvent, uint32_t id, void* params, uintptr_t auxParam)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StubSC_SocketsEventParams* scParams = calloc(1, sizeof(SOPC_StubSC_SocketsEventParams));
    assert(scParams != NULL && socketsEvents != NULL);
    scParams->event = scEvent;
    scParams->eltId = id;
    scParams->params = params;
    scParams->auxParam = auxParam;

    status = SOPC_AsyncQueue_BlockingEnqueue(socketsEvents, (void*) scParams);
    (void) status; // status is not used if asserts are not compiled in
    assert(status == SOPC_STATUS_OK);
}

void SOPC_Sockets_Initialize()
{
    SOPC_ReturnStatus status = SOPC_AsyncQueue_Init(&socketsEvents, "StubsSC_SocketsEventQueue");
    (void) status; // status is not used if asserts are not compiled in
    assert(status == SOPC_STATUS_OK);
}

void SOPC_Sockets_Clear()
{
    SOPC_AsyncQueue_Free(&socketsEvents);
    socketsEvents = NULL;
}
