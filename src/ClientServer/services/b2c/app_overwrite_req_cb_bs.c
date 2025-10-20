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

#include "app_overwrite_req_cb_bs.h"

#include "app_cb_call_context_internal.h"
#include "sopc_assert.h"
#include "sopc_toolkit_config_internal.h"
#include "util_b2c.h"

static SOPC_OverwriteSessionRequestFunc* overwriteRequestFunc = NULL;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void app_overwrite_req_cb_bs__INITIALISATION(void)
{
    /* Translated from B but an intialisation is not needed for this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void app_overwrite_req_cb_bs__has_server_overwrite_req_cb(
    const constants__t_endpoint_config_idx_i app_overwrite_req_cb_bs__p_endpoint_config_idx,
    t_bool* const app_overwrite_req_cb_bs__bres)
{
    SOPC_Endpoint_Config* endpointConfig =
        SOPC_ToolkitServer_GetEndpointConfig(app_overwrite_req_cb_bs__p_endpoint_config_idx);
    SOPC_ASSERT(NULL != endpointConfig && NULL != endpointConfig->serverConfigPtr);
    SOPC_Server_Config* serverConfig = endpointConfig->serverConfigPtr;
    overwriteRequestFunc = serverConfig->overwriteRequestFunc;
    *app_overwrite_req_cb_bs__bres = (overwriteRequestFunc != NULL);
}

void app_overwrite_req_cb_bs__overwrite_service_request(
    const constants__t_msg_i app_overwrite_req_cb_bs__p_req_msg,
    constants_statuscodes_bs__t_StatusCode_i* const app_overwrite_req_cb_bs__p_sc,
    constants__t_msg_i* const app_overwrite_req_cb_bs__new_req_msg)
{
    SOPC_ASSERT(overwriteRequestFunc != NULL);
    const SOPC_CallContext* callContext = SOPC_CallContext_GetCurrent();
    SOPC_EncodeableType* type = *(SOPC_EncodeableType* const*) app_overwrite_req_cb_bs__p_req_msg;
    SOPC_StatusCode retSc = overwriteRequestFunc(callContext, type, app_overwrite_req_cb_bs__p_req_msg);
    SOPC_ASSERT(type == *(SOPC_EncodeableType**) app_overwrite_req_cb_bs__p_req_msg);
    *app_overwrite_req_cb_bs__new_req_msg = app_overwrite_req_cb_bs__p_req_msg;
    util_status_code__C_to_B(retSc, app_overwrite_req_cb_bs__p_sc);
}
