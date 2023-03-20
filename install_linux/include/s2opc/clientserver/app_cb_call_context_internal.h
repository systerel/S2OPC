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

#ifndef _app_cb_call_context_internal_h
#define _app_cb_call_context_internal_h

#include "sopc_user_app_itf.h"

/* Since we will need to copy it for asynchronous events provided to application code,
 * we should avoid to store structures needing allocation.*/
struct SOPC_CallContext
{
    bool isCopy;

    uint32_t secureChannelConfigIdx; // Only valid for client side
    uint32_t endpointConfigIdx;      // Only valid for server side
    OpcUa_MessageSecurityMode msgSecurityMode;
    const char* secuPolicyUri;
    const SOPC_User* user;

    uintptr_t auxParam; // Used to store initial auxParam in application events (it will be replaced by call context)
};

const SOPC_CallContext* SOPC_CallContext_GetCurrent(void);

SOPC_CallContext* SOPC_CallContext_Copy(const SOPC_CallContext* cc);

void SOPC_CallContext_Free(SOPC_CallContext* cc);

#endif
