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

#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config_internal.h"

#include <string.h>

static SOPC_CallContext currentCtx;

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

SOPC_CallContext* SOPC_CallContext_Copy(const SOPC_CallContext* cc)
{
    if (NULL == cc)
    {
        return NULL;
    }
    SOPC_CallContext* copy = SOPC_Calloc(1, sizeof(*copy));
    OpcUa_ApplicationDescription* appDescCopy = NULL;
    SOPC_ReturnStatus status =
        SOPC_EncodeableObject_Create(&OpcUa_ApplicationDescription_EncodeableType, (void**) &appDescCopy);
    SOPC_UNUSED_RESULT(status);
    if (NULL != copy)
    {
        copy->isCopy = true;
        copy->secureChannelConfigIdx = cc->secureChannelConfigIdx;
        copy->endpointConfigIdx = cc->endpointConfigIdx;
        copy->msgSecurityMode = cc->msgSecurityMode;
        copy->secuPolicyUri = SOPC_strdup(cc->secuPolicyUri);
        copy->sessionId = cc->sessionId;
        if (SOPC_STATUS_OK == status)
        {
            // Note: we only copy the ApplicationURI to avoid unnecessary allocations,
            //       we might do more in the future if needed
            status = SOPC_String_Copy(&appDescCopy->ApplicationUri, &cc->clientAppDescription->ApplicationUri);
        }
        if (SOPC_STATUS_OK == status)
        {
            copy->clientAppDescription = appDescCopy;
        }
        else
        {
            SOPC_EncodeableObject_Delete(&OpcUa_ApplicationDescription_EncodeableType, (void**) &appDescCopy);
        }
        copy->clientCertThumbprint = SOPC_strdup(cc->clientCertThumbprint);
        copy->user = SOPC_User_Copy(cc->user);
        copy->auxParam = cc->auxParam;
    }
    return copy;
}

void SOPC_CallContext_Free(SOPC_CallContext* cc)
{
    if (NULL != cc && cc->isCopy)
    {
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        SOPC_Free((char*) cc->secuPolicyUri);
        SOPC_EncodeableObject_Delete(&OpcUa_ApplicationDescription_EncodeableType, (void**) &cc->clientAppDescription);
        SOPC_Free((char*) cc->clientCertThumbprint);
        SOPC_User_Free((SOPC_User**) &cc->user);
        SOPC_GCC_DIAGNOSTIC_RESTORE
        SOPC_Free(cc);
    }
}
