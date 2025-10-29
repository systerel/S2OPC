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
    int32_t* refCopyCount; // Number of references to this copy (only allocated if isCopy set to true)

    uint32_t secureChannelConfigIdx; // Only valid for client side (type ::SOPC_SecureChannelConfigIdx)
    uint32_t endpointConfigIdx;      // Only valid for server side (type ::SOPC_EndpointConfigIdx)
    OpcUa_MessageSecurityMode msgSecurityMode;
    const char* secuPolicyUri;
    const OpcUa_ApplicationDescription* clientAppDescription;
    char* clientCertThumbprint;
    SOPC_SessionId sessionId;
    const SOPC_User* user;

    /* Specific use of call context that is never copied (in ::SOPC_CallContext_CreateCurrentCopy) */
    SOPC_AddressSpaceAccess* addressSpaceForMethodCall;
    uintptr_t auxParam; // Used to store initial auxParam in application events (it will be replaced by call context)
};

typedef SOPC_CallContext SOPC_CallContextCopy;

const SOPC_CallContext* SOPC_CallContext_GetCurrent(void);

/*
 * It shall be used only when call context is used asynchronously and or when specific context content is added.
 * A reference counter is used to avoid unecessary allocations are done.
 * The copy shall still be const as it might share content between several copies of same current context.
 *
 * Note: this function SHALL only be called from the service layer thread (i.e. only form *bs.c modules)
 */
SOPC_CallContextCopy* SOPC_CallContext_CreateCurrentCopy(void);

void SOPC_CallContext_FreeCopy(SOPC_CallContextCopy* cc);

#endif
