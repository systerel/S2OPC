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

#include "sopc_toolkit_async_api.h"

#include "sopc_services_api.h"

uint32_t uniqueUserId = 1;

void SOPC_ToolkitServer_AsyncOpenEndpoint(uint32_t endpointConfigIdx)
{
    // TODO: check valid config and return bool
    SOPC_Services_EnqueueEvent(APP_TO_SE_OPEN_ENDPOINT, endpointConfigIdx, NULL, 0);
}

void SOPC_ToolkitServer_AsyncCloseEndpoint(uint32_t endpointConfigIdx)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_CLOSE_ENDPOINT, endpointConfigIdx, NULL, 0);
}

void SOPC_ToolkitServer_AsyncLocalServiceRequest(uint32_t endpointConfigIdx,
                                                 void* requestStruct,
                                                 uintptr_t requestContext)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_LOCAL_SERVICE_REQUEST, endpointConfigIdx, requestStruct, requestContext);
}

void SOPC_ToolkitClient_AsyncActivateSession(uint32_t endpointConnectionIdx, uintptr_t sessionContext)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_ACTIVATE_SESSION, endpointConnectionIdx, &uniqueUserId, sessionContext);
}

void SOPC_ToolkitClient_AsyncSendRequestOnSession(uint32_t sessionId, void* requestStruct, uintptr_t requestContext)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_SEND_SESSION_REQUEST, sessionId, requestStruct, requestContext);
}

void SOPC_ToolkitClient_AsyncCloseSession(uint32_t sessionId)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_CLOSE_SESSION, sessionId, NULL, 0);
}

void SOPC_ToolkitClient_AsyncSendDiscoveryRequest(uint32_t endpointConnectionIdx,
                                                  void* discoveryReqStruct,
                                                  uintptr_t requestContext)
{
    SOPC_Services_EnqueueEvent(APP_TO_SE_SEND_DISCOVERY_REQUEST, endpointConnectionIdx, discoveryReqStruct,
                               requestContext);
}
