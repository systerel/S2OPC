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

#ifndef SKS_DEMO_SERVER_METHODS_H_
#define SKS_DEMO_SERVER_METHODS_H_

#include "sopc_builtintypes.h"
#include "sopc_service_call_context.h"

/*---------------------------------------------------------------------------
 *                 SKS Demo Methods for Call service definition
 *---------------------------------------------------------------------------*/

#ifndef SKS_SECURITY_GROUPID
#define SKS_SECURITY_GROUPID "sgid_1"
#endif

SOPC_StatusCode SOPC_Method_Func_PublishSubscribe_GetSecurityKeys(const SOPC_CallContext* callContextPtr,
                                                                  const SOPC_NodeId* objectId,
                                                                  uint32_t nbInputArgs,
                                                                  const SOPC_Variant* inputArgs,
                                                                  uint32_t* nbOutputArgs,
                                                                  SOPC_Variant** outputArgs,
                                                                  void* param);

#endif
