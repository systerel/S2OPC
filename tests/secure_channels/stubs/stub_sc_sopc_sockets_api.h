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
