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

/** \file
 *
 * \brief Internal API to manage methods of the ServerConfigurationType according the Push model.
 */

#ifndef SOPC_PUSH_SERVER_CONFIG_METH_
#define SOPC_PUSH_SERVER_CONFIG_METH_

#include "sopc_builtintypes.h"
#include "sopc_service_call_context.h"

SOPC_StatusCode PushSrvCfg_Method_UpdateCertificate(const SOPC_CallContext* callContextPtr,
                                                    const SOPC_NodeId* objectId,
                                                    uint32_t nbInputArgs,
                                                    const SOPC_Variant* inputArgs,
                                                    uint32_t* nbOutputArgs,
                                                    SOPC_Variant** outputArgs,
                                                    void* param);

SOPC_StatusCode PushSrvCfg_Method_ApplyChanges(const SOPC_CallContext* callContextPtr,
                                               const SOPC_NodeId* objectId,
                                               uint32_t nbInputArgs,
                                               const SOPC_Variant* inputArgs,
                                               uint32_t* nbOutputArgs,
                                               SOPC_Variant** outputArgs,
                                               void* param);

SOPC_StatusCode PushSrvCfg_Method_CreateSigningRequest(const SOPC_CallContext* callContextPtr,
                                                       const SOPC_NodeId* objectId,
                                                       uint32_t nbInputArgs,
                                                       const SOPC_Variant* inputArgs,
                                                       uint32_t* nbOutputArgs,
                                                       SOPC_Variant** outputArgs,
                                                       void* param);

SOPC_StatusCode PushSrvCfg_Method_GetRejectedList(const SOPC_CallContext* callContextPtr,
                                                  const SOPC_NodeId* objectId,
                                                  uint32_t nbInputArgs,
                                                  const SOPC_Variant* inputArgs,
                                                  uint32_t* nbOutputArgs,
                                                  SOPC_Variant** outputArgs,
                                                  void* param);

SOPC_StatusCode PushSrvCfg_Method_TOFUNotSuported(const SOPC_CallContext* callContextPtr,
                                                  const SOPC_NodeId* objectId,
                                                  uint32_t nbInputArgs,
                                                  const SOPC_Variant* inputArgs,
                                                  uint32_t* nbOutputArgs,
                                                  SOPC_Variant** outputArgs,
                                                  void* param);

#endif /* SOPC_PUSH_SERVER_CONFIG_METH_ */
