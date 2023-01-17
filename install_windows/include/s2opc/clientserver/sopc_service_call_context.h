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
 * \brief Defines abstract context type that can be use to obtain context associated to a service call
 *        (write, callmethod, etc.)
 *
 */

#ifndef SOPC_SERVICE_CALL_CONTEXT_H_
#define SOPC_SERVICE_CALL_CONTEXT_H_

#include "sopc_enum_types.h"
#include "sopc_user.h"

/**
 * \brief Context provided when a service callback is called (write notification, method call).
 *        The context can only be used during the callback call, see getters available.
 */
typedef struct SOPC_CallContext SOPC_CallContext;

/** \brief Returns the user that called the service */
const SOPC_User* SOPC_CallContext_GetUser(const SOPC_CallContext* callContextPtr);

/** \brief Returns the security mode of the connection used to call the service */
OpcUa_MessageSecurityMode SOPC_CallContext_GetSecurityMode(const SOPC_CallContext* callContextPtr);

/** \brief Returns the security policy of the connection used to call the service */
const char* SOPC_CallContext_GetSecurityPolicy(const SOPC_CallContext* callContextPtr);

/** \brief Returns the server endpoint of the connection used to call the service */
uint32_t SOPC_CallContext_GetEndpointConfigIdx(const SOPC_CallContext* callContextPtr);

#endif // SOPC_SERVICE_CALL_CONTEXT_H_
