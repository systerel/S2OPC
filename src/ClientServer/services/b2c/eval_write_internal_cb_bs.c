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

#include "eval_write_internal_cb_bs.h"

#include "libs2opc_server_internal.h"
#include "sopc_builtintypes.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void eval_write_internal_cb_bs__INITIALISATION(void)
{
    // Nothing to do
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void eval_write_internal_cb_bs__eval_write_internal_behavior(
    const constants__t_NodeId_i eval_write_internal_cb_bs__p_nid,
    const constants__t_AttributeId_i eval_write_internal_cb_bs__p_attribute,
    const constants__t_DataValue_i eval_write_internal_cb_bs__p_prev_dataValue,
    const constants__t_Variant_i eval_write_internal_cb_bs__p_new_val,
    const constants__t_RawStatusCode eval_write_internal_cb_bs__p_new_val_sc,
    const constants__t_Timestamp eval_write_internal_cb_bs__p_new_val_ts_src,
    const constants__t_Timestamp eval_write_internal_cb_bs__p_new_val_ts_srv)
{
    if (constants__e_aid_Value != eval_write_internal_cb_bs__p_attribute ||
        !SOPC_IsGoodStatus(eval_write_internal_cb_bs__p_new_val_sc))
    {
        return;
    }

    SOPC_ServerInternal_WriteBehavior_Fct* callback = NULL;
    uintptr_t auxParam = 0;
    if (!SOPC_ServerInternal_GetRegisteredWriteBehaviorCb(eval_write_internal_cb_bs__p_nid, &callback, &auxParam))
    {
        return;
    }

    // Shallow copy of the new value: DoNotClear since the variant is not owned
    SOPC_DataValue newValue;
    SOPC_DataValue_Initialize(&newValue);
    newValue.Value = *eval_write_internal_cb_bs__p_new_val;
    newValue.Value.DoNotClear = true;
    newValue.Status = eval_write_internal_cb_bs__p_new_val_sc;
    newValue.SourceTimestamp = eval_write_internal_cb_bs__p_new_val_ts_src.timestamp;
    newValue.SourcePicoSeconds = eval_write_internal_cb_bs__p_new_val_ts_src.picoSeconds;
    newValue.ServerTimestamp = eval_write_internal_cb_bs__p_new_val_ts_srv.timestamp;
    newValue.ServerPicoSeconds = eval_write_internal_cb_bs__p_new_val_ts_srv.picoSeconds;

    callback(eval_write_internal_cb_bs__p_nid, eval_write_internal_cb_bs__p_prev_dataValue, &newValue, auxParam);
}
