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
 * This file is an excerpt from sopc_call_method_manager.h.
 * It should not be included in a generic project.
 * See s2opc_headers.h
 */

typedef struct SOPC_MethodCallManager SOPC_MethodCallManager;
typedef struct SOPC_MethodCallFunc SOPC_MethodCallFunc;
typedef struct SOPC_CallContext SOPC_CallContext;

typedef SOPC_StatusCode SOPC_MethodCallFunc_Ptr(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param);
typedef void SOPC_MethodCallFunc_Free_Func(void* data);

SOPC_MethodCallManager* SOPC_MethodCallManager_Create(void);
void SOPC_MethodCallManager_Free(SOPC_MethodCallManager* mcm);
SOPC_ReturnStatus SOPC_MethodCallManager_AddMethod(SOPC_MethodCallManager* mcm,
                                                   SOPC_NodeId* methodId,
                                                   SOPC_MethodCallFunc_Ptr* methodFunc,
                                                   void* param,
                                                   SOPC_MethodCallFunc_Free_Func* fnFree);
