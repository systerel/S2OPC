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
