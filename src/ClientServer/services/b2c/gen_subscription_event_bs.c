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

#include "gen_subscription_event_bs.h"

#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_services_api_internal.h"
#include "sopc_types.h"
#include "util_b2c.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void gen_subscription_event_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void gen_subscription_event_bs__gen_data_changed_event(
    const constants__t_NodeId_i gen_subscription_event_bs__p_nid,
    const constants__t_AttributeId_i gen_subscription_event_bs__p_attribute,
    const constants__t_DataValue_i gen_subscription_event_bs__p_prev_dataValue,
    const constants__t_Variant_i gen_subscription_event_bs__p_new_val,
    const constants__t_RawStatusCode gen_subscription_event_bs__p_new_val_sc,
    const constants__t_Timestamp gen_subscription_event_bs__p_new_val_ts_src,
    const constants__t_Timestamp gen_subscription_event_bs__p_new_val_ts_srv)
{
    SOPC_ReturnStatus retStatus = SOPC_STATUS_OK;
    OpcUa_WriteValue* newValue = SOPC_Malloc(sizeof(OpcUa_WriteValue));
    OpcUa_WriteValue_Initialize(newValue);
    OpcUa_WriteValue* oldValue = SOPC_Malloc(sizeof(OpcUa_WriteValue));
    OpcUa_WriteValue_Initialize(oldValue);

    if (NULL != oldValue && NULL != newValue)
    {
        /* Copy new data in new WriteValue */
        retStatus = SOPC_Variant_Copy(&newValue->Value.Value, gen_subscription_event_bs__p_new_val);

        /* Copy value meta data */
        newValue->Value.Status = gen_subscription_event_bs__p_new_val_sc;
        newValue->Value.SourceTimestamp = gen_subscription_event_bs__p_new_val_ts_src.timestamp;
        newValue->Value.SourcePicoSeconds = gen_subscription_event_bs__p_new_val_ts_src.picoSeconds;
        newValue->Value.ServerTimestamp = gen_subscription_event_bs__p_new_val_ts_srv.timestamp;
        newValue->Value.ServerPicoSeconds = gen_subscription_event_bs__p_new_val_ts_srv.picoSeconds;

        /* Set old data in old WriteValue */
        // Shallow copy of all fields
        oldValue->Value = *gen_subscription_event_bs__p_prev_dataValue;
        // Move property of Variant: <=> SOPC_Variant_Move but avoiding double shallow copy
        gen_subscription_event_bs__p_prev_dataValue->Value.DoNotClear = true;

        /* Copy common fields with new WriteValue */
        oldValue->AttributeId = gen_subscription_event_bs__p_attribute;
        newValue->AttributeId = gen_subscription_event_bs__p_attribute;

        /* No index ranges */

        if (SOPC_STATUS_OK == retStatus)
        {
            retStatus = SOPC_NodeId_Copy(&oldValue->NodeId, gen_subscription_event_bs__p_nid);
        }

        if (SOPC_STATUS_OK == retStatus)
        {
            retStatus = SOPC_NodeId_Copy(&newValue->NodeId, gen_subscription_event_bs__p_nid);
        }

        /* Generate data changed event with old & new WriteValue */
        if (SOPC_STATUS_OK == retStatus)
        {
            SOPC_EventHandler_Post(SOPC_Services_GetEventHandler(), SE_TO_SE_SERVER_DATA_CHANGED, 0,
                                   (uintptr_t) oldValue, (uintptr_t) newValue);
        }
        else
        {
            // Delete the WriteValues allocated
            OpcUa_WriteValue_Clear(oldValue);
            SOPC_Free(oldValue);
            OpcUa_WriteValue_Clear(newValue);
            SOPC_Free(newValue);
        }
    }
    else
    {
        OpcUa_WriteValue_Clear(oldValue);
        SOPC_Free(oldValue);
        OpcUa_WriteValue_Clear(newValue);
        SOPC_Free(newValue);

        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "gen_subscription_event_bs__gen_data_changed_event: failed to generate a data changed event (out of "
            "memory for wv alloc)");
    }
}

void gen_subscription_event_bs__gen_data_changed_event_failed(void)
{
    SOPC_Logger_TraceError(
        SOPC_LOG_MODULE_CLIENTSERVER,
        "gen_subscription_event_bs__gen_data_changed_event_failed: failed to generate a data changed event (out of "
        "memory ?)");
}
