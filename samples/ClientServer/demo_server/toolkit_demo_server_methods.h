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

#ifndef TOOLKIT_DEMO_SERVER_METHODS_H_
#define TOOLKIT_DEMO_SERVER_METHODS_H_

#include "sopc_builtintypes.h"
#include "sopc_call_method_manager.h"
#include "sopc_service_call_context.h"

/*---------------------------------------------------------------------------
 *                    Demo Methods for Call service definition
 *---------------------------------------------------------------------------*/

/* \brief Increments the AddressSpace Variable value "ns=1;s=TestObject_Counter" */
SOPC_StatusCode SOPC_Method_Func_IncCounter(const SOPC_CallContext* callContextPtr,
                                            const SOPC_NodeId* objectId,
                                            uint32_t nbInputArgs,
                                            const SOPC_Variant* inputArgs,
                                            uint32_t* nbOutputArgs,
                                            SOPC_Variant** outputArgs,
                                            void* param);

/* \brief Adds the given argument value to the AddressSpace Variable value "ns=1;s=TestObject_Counter" */
SOPC_StatusCode SOPC_Method_Func_AddToCounter(const SOPC_CallContext* callContextPtr,
                                              const SOPC_NodeId* objectId,
                                              uint32_t nbInputArgs,
                                              const SOPC_Variant* inputArgs,
                                              uint32_t* nbOutputArgs,
                                              SOPC_Variant** outputArgs,
                                              void* param);

/* \brief Returns the AddressSpace Variable value "ns=1;s=TestObject_Counter" as output argument */
SOPC_StatusCode SOPC_Method_Func_GetCounterValue(const SOPC_CallContext* callContextPtr,
                                                 const SOPC_NodeId* objectId,
                                                 uint32_t nbInputArgs,
                                                 const SOPC_Variant* inputArgs,
                                                 uint32_t* nbOutputArgs,
                                                 SOPC_Variant** outputArgs,
                                                 void* param);

/* \brief Returns 'Hello <valueOf("ns=1;s=TestObject_HelloNextArg")> !' as output argument
 *        and assign input argument into the AddressSpace Variable value "ns=1;s=TestObject_HelloNextArg"
 */
SOPC_StatusCode SOPC_Method_Func_UpdateAndGetPreviousHello(const SOPC_CallContext* callContextPtr,
                                                           const SOPC_NodeId* objectId,
                                                           uint32_t nbInputArgs,
                                                           const SOPC_Variant* inputArgs,
                                                           uint32_t* nbOutputArgs,
                                                           SOPC_Variant** outputArgs,
                                                           void* param);

SOPC_StatusCode SOPC_Method_Func_AddVariable(const SOPC_CallContext* callContextPtr,
                                             const SOPC_NodeId* objectId,
                                             uint32_t nbInputArgs,
                                             const SOPC_Variant* inputArgs,
                                             uint32_t* nbOutputArgs,
                                             SOPC_Variant** outputArgs,
                                             void* param);

SOPC_StatusCode SOPC_Method_Func_GenEvent(const SOPC_CallContext* callContextPtr,
                                          const SOPC_NodeId* objectId,
                                          uint32_t nbInputArgs,
                                          const SOPC_Variant* inputArgs,
                                          uint32_t* nbOutputArgs,
                                          SOPC_Variant** outputArgs,
                                          void* param);

/* \brief Add a role to the server */
SOPC_StatusCode SOPC_Method_Func_AddRole(const SOPC_CallContext* callContextPtr,
                                         const SOPC_NodeId* objectId,
                                         uint32_t nbInputArgs,
                                         const SOPC_Variant* inputArgs,
                                         uint32_t* nbOutputArgs,
                                         SOPC_Variant** outputArgs,
                                         void* param);

/* \brief Remove a role from the server */
SOPC_StatusCode SOPC_Method_Func_RemoveRole(const SOPC_CallContext* callContextPtr,
                                            const SOPC_NodeId* objectId,
                                            uint32_t nbInputArgs,
                                            const SOPC_Variant* inputArgs,
                                            uint32_t* nbOutputArgs,
                                            SOPC_Variant** outputArgs,
                                            void* param);

/*---------------------------------------------------------------------------
 *                    Demo Methods registration in Method Call Manager
 *---------------------------------------------------------------------------*/

/**
 * \brief Add OPC UA demo methods defined in this module into the given Method Call Manager
 *
 * \param mcm The Method Call Manager into which the method implementations are added
 *
 * \return SOPC_STATUS_OK in case of success.
 */
SOPC_ReturnStatus SOPC_DemoServerConfig_AddMethods(SOPC_MethodCallManager* mcm);

#endif
