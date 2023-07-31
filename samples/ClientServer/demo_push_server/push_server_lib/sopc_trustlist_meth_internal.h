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
 * \brief Internal API to manage methods of the TrustListType according the Push model.
 */

#ifndef SOPC_TRUSTLIST_METH_INTERNAL_
#define SOPC_TRUSTLIST_METH_INTERNAL_

#include "sopc_builtintypes.h"
#include "sopc_service_call_context.h"

SOPC_StatusCode TrustList_Method_OpenWithMasks(const SOPC_CallContext* callContextPtr,
                                               const SOPC_NodeId* objectId,
                                               uint32_t nbInputArgs,
                                               const SOPC_Variant* inputArgs,
                                               uint32_t* nbOutputArgs,
                                               SOPC_Variant** outputArgs,
                                               void* param);

SOPC_StatusCode TrustList_Method_Open(const SOPC_CallContext* callContextPtr,
                                      const SOPC_NodeId* objectId,
                                      uint32_t nbInputArgs,
                                      const SOPC_Variant* inputArgs,
                                      uint32_t* nbOutputArgs,
                                      SOPC_Variant** outputArgs,
                                      void* param);

SOPC_StatusCode TrustList_Method_Close(const SOPC_CallContext* callContextPtr,
                                       const SOPC_NodeId* objectId,
                                       uint32_t nbInputArgs,
                                       const SOPC_Variant* inputArgs,
                                       uint32_t* nbOutputArgs,
                                       SOPC_Variant** outputArgs,
                                       void* param);

SOPC_StatusCode TrustList_Method_CloseAndUpdate(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param);

SOPC_StatusCode TrustList_Method_AddCertificate(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param);

SOPC_StatusCode TrustList_Method_RemoveCertificate(const SOPC_CallContext* callContextPtr,
                                                   const SOPC_NodeId* objectId,
                                                   uint32_t nbInputArgs,
                                                   const SOPC_Variant* inputArgs,
                                                   uint32_t* nbOutputArgs,
                                                   SOPC_Variant** outputArgs,
                                                   void* param);

SOPC_StatusCode TrustList_Method_Read(const SOPC_CallContext* callContextPtr,
                                      const SOPC_NodeId* objectId,
                                      uint32_t nbInputArgs,
                                      const SOPC_Variant* inputArgs,
                                      uint32_t* nbOutputArgs,
                                      SOPC_Variant** outputArgs,
                                      void* param);

SOPC_StatusCode TrustList_Method_Write(const SOPC_CallContext* callContextPtr,
                                       const SOPC_NodeId* objectId,
                                       uint32_t nbInputArgs,
                                       const SOPC_Variant* inputArgs,
                                       uint32_t* nbOutputArgs,
                                       SOPC_Variant** outputArgs,
                                       void* param);

SOPC_StatusCode TrustList_Method_GetPosition(const SOPC_CallContext* callContextPtr,
                                             const SOPC_NodeId* objectId,
                                             uint32_t nbInputArgs,
                                             const SOPC_Variant* inputArgs,
                                             uint32_t* nbOutputArgs,
                                             SOPC_Variant** outputArgs,
                                             void* param);

SOPC_StatusCode TrustList_Method_SetPosition(const SOPC_CallContext* callContextPtr,
                                             const SOPC_NodeId* objectId,
                                             uint32_t nbInputArgs,
                                             const SOPC_Variant* inputArgs,
                                             uint32_t* nbOutputArgs,
                                             SOPC_Variant** outputArgs,
                                             void* param);

#endif /* SOPC_TRUSTLIST_METH_INTERNAL_ */
