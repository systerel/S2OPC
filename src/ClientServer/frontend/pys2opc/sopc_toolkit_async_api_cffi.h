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

/**
 * This file is an excerpt from sopc_toolkit_async_api_cffi.h.
 * It should not be included in a generic project.
 * See s2opc_headers.h
 */

void SOPC_ToolkitServer_AsyncOpenEndpoint(uint32_t endpointConfigIdx);
void SOPC_ToolkitServer_AsyncCloseEndpoint(uint32_t endpointConfigIdx);
void SOPC_ToolkitServer_AsyncLocalServiceRequest(uint32_t endpointConfigIdx,
                                                 void* requestStruct,
                                                 uintptr_t requestContext);

// void SOPC_ToolkitClient_AsyncActivateSession(uint32_t endpointConnectionIdx,
//                                              uintptr_t sessionContext,
//                                              SOPC_ExtensionObject* userToken);
// SOPC_ReturnStatus SOPC_ToolkitClient_AsyncActivateSession_Anonymous(uint32_t endpointConnectionIdx,
//                                                                     uintptr_t sessionContext,
//                                                                     const char* policyId);
// SOPC_ReturnStatus SOPC_ToolkitClient_AsyncActivateSession_UsernamePassword(uint32_t endpointConnectionIdx,
//                                                                            uintptr_t sessionContext,
//                                                                            const char* policyId,
//                                                                            const char* username,
//                                                                            const uint8_t* password,
//                                                                            int32_t length_password);
// void SOPC_ToolkitClient_AsyncSendRequestOnSession(uint32_t sessionId, void* requestStruct, uintptr_t requestContext);
// void SOPC_ToolkitClient_AsyncCloseSession(uint32_t sessionId);
// void SOPC_ToolkitClient_AsyncSendDiscoveryRequest(uint32_t endpointConnectionIdx,
//                                                   void* discoveryReqStruct,
//                                                   uintptr_t requestContext);
