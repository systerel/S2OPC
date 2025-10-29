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

#include "app_cb_call_context_bs.h"
#include "app_cb_call_context_internal.h"

#include "sopc_atomic.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"

#include <string.h>

static SOPC_CallContext currentCtx;
static SOPC_CallContextCopy* currentCopyCtx;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void app_cb_call_context_bs__INITIALISATION(void)
{
    app_cb_call_context_bs__clear_app_call_context();
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void app_cb_call_context_bs__clear_app_call_context(void)
{
    memset(&currentCtx, 0, sizeof(currentCtx));
    SOPC_CallContext_FreeCopy(currentCopyCtx);
    currentCopyCtx = NULL;
}

void app_cb_call_context_bs__set_app_call_context_channel_config(
    const constants__t_channel_config_idx_i app_cb_call_context_bs__p_channel_config,
    const constants__t_endpoint_config_idx_i app_cb_call_context_bs__p_endpoint_config)
{
    SOPC_SecureChannel_Config* scConfigPtr =
        SOPC_ToolkitServer_GetSecureChannelConfig(app_cb_call_context_bs__p_channel_config);
    if (NULL != scConfigPtr)
    {
        currentCtx.msgSecurityMode = scConfigPtr->msgSecurityMode;
        currentCtx.secuPolicyUri = scConfigPtr->reqSecuPolicyUri;
    }

    if (constants__c_endpoint_config_idx_indet == app_cb_call_context_bs__p_endpoint_config)
    {
        // Client connection case => keep secure channel configuration index
        currentCtx.secureChannelConfigIdx = app_cb_call_context_bs__p_channel_config;
    }
    else
    {
        // Server connection case => keep only endpoint configuration index (the secure channel one is internal)
        currentCtx.endpointConfigIdx = app_cb_call_context_bs__p_endpoint_config;
    }
}

void app_cb_call_context_bs__set_app_call_context_session(
    const constants__t_session_i app_cb_call_context_bs__p_session,
    const constants__t_ApplicationDescription_i app_cb_call_context_bs__p_cliAppDesc,
    const constants__t_CertThumbprint_i app_cb_call_context_bs__p_cliCertTb,
    const constants__t_user_i app_cb_call_context_bs__p_user)
{
    currentCtx.sessionId = app_cb_call_context_bs__p_session;
    currentCtx.clientAppDescription = app_cb_call_context_bs__p_cliAppDesc;
    currentCtx.clientCertThumbprint = app_cb_call_context_bs__p_cliCertTb;
    currentCtx.user = SOPC_UserWithAuthorization_GetUser(app_cb_call_context_bs__p_user);
}

const SOPC_CallContext* SOPC_CallContext_GetCurrent(void)
{
    return &currentCtx;
}

SOPC_CallContextCopy* SOPC_CallContext_CreateCurrentCopy(void)
{
    // Allocate new copy to return
    SOPC_CallContextCopy* copy = SOPC_Calloc(1, sizeof(*copy));
    // Allocate a fresh current copy or retrieve existing one
    SOPC_CallContextCopy* freshCurrentCopy =
        (NULL != currentCopyCtx) ? currentCopyCtx : SOPC_Calloc(1, sizeof(*freshCurrentCopy));
    if (NULL != copy && NULL != freshCurrentCopy)
    {
        // If no current copy exists yet, create a new dedicated one
        if (NULL == currentCopyCtx)
        {
            copy->isCopy = true;
            copy->refCopyCount = SOPC_Calloc(1, sizeof(int32_t));
            if (NULL == copy->refCopyCount)
            {
                SOPC_Free(copy);
                copy = NULL;
            }
            else
            {
                // One for currentCopyCtx and one for the returned copy
                // Note: atomic not needed here as copy is not yet shared
                *(copy->refCopyCount) = 2;
                // Do an actual copy of the content for the first copy
                OpcUa_ApplicationDescription* appDescCopy = NULL;
                SOPC_ReturnStatus status =
                    SOPC_EncodeableObject_Create(&OpcUa_ApplicationDescription_EncodeableType, (void**) &appDescCopy);
                SOPC_UNUSED_RESULT(status);
                copy->secureChannelConfigIdx = currentCtx.secureChannelConfigIdx;
                copy->endpointConfigIdx = currentCtx.endpointConfigIdx;
                copy->msgSecurityMode = currentCtx.msgSecurityMode;
                copy->secuPolicyUri = SOPC_strdup(currentCtx.secuPolicyUri);
                copy->sessionId = currentCtx.sessionId;

                if (SOPC_STATUS_OK == status)
                {
                    // Copy ApplicationDescription
                    status = SOPC_EncodeableObject_Copy(&OpcUa_ApplicationDescription_EncodeableType, appDescCopy,
                                                        currentCtx.clientAppDescription);
                }
                if (SOPC_STATUS_OK == status)
                {
                    copy->clientAppDescription = appDescCopy;
                }
                else
                {
                    SOPC_EncodeableObject_Delete(&OpcUa_ApplicationDescription_EncodeableType, (void**) &appDescCopy);
                }
                copy->clientCertThumbprint = SOPC_strdup(currentCtx.clientCertThumbprint);
                copy->user = SOPC_User_Copy(currentCtx.user);
                // Duplicate content for currentCopyCtx
                *freshCurrentCopy = *copy;
                currentCopyCtx = freshCurrentCopy;
            }
        }
        else
        {
            // copy content of current copy
            *copy = *currentCopyCtx;
            // Increase shared reference counter
            // Note: atomic shall be used, it is guaranteed that the last free cannot be done
            //       concurrently since only this thread shall CreateCopy / clear current copy with
            //       clear_app_call_context.
            SOPC_Atomic_Int_Add(copy->refCopyCount, 1);
        }
    }
    else
    {
        SOPC_Free(copy);
        SOPC_Free(freshCurrentCopy);
    }
    return copy;
}

void SOPC_CallContext_FreeCopy(SOPC_CallContextCopy* cc)
{
    if (NULL != cc && cc->isCopy)
    {
        int32_t refCount = SOPC_Atomic_Int_Get(cc->refCopyCount);
        if (refCount > 1)
        {
            // Decrease reference counter
            SOPC_Atomic_Int_Add(cc->refCopyCount, -1);
        }
        else
        {
            SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
            SOPC_Free(cc->refCopyCount);
            SOPC_Free((char*) cc->secuPolicyUri);
            SOPC_EncodeableObject_Delete(&OpcUa_ApplicationDescription_EncodeableType,
                                         (void**) &cc->clientAppDescription);
            SOPC_Free((char*) cc->clientCertThumbprint);
            SOPC_User_Free((SOPC_User**) &cc->user);
            SOPC_GCC_DIAGNOSTIC_RESTORE
        }
        SOPC_Free(cc);
    }
}
