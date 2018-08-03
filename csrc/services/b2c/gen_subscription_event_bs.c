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
    const constants__t_Variant_i gen_subscription_event_bs__p_prev_value,
    const constants__t_StatusCode_i gen_subscription_event_bs__p_prev_value_status,
    const constants__t_WriteValuePointer_i gen_subscription_event_bs__p_new_value_pointer)
{
    SOPC_ReturnStatus retStatus = SOPC_STATUS_OK;
    OpcUa_WriteValue* newValue = gen_subscription_event_bs__p_new_value_pointer;
    OpcUa_WriteValue* oldValue = malloc(sizeof(OpcUa_WriteValue));
    OpcUa_WriteValue_Initialize((void*) oldValue);
    SOPC_Variant* oldVariantValue = gen_subscription_event_bs__p_prev_value;

    if (NULL != oldValue)
    {
        /* Set old data in old WriteValue */
        oldValue->Value.Value = *oldVariantValue;
        // Free variant structure container only, content copied in WriteValue
        free(oldVariantValue);
        // TODO: set timestamps (not available: current time always returned)
        util_status_code__B_to_C(gen_subscription_event_bs__p_prev_value_status, &oldValue->Value.Status);

        /* Copy common fields with new WriteValue */
        oldValue->AttributeId = newValue->AttributeId;
        retStatus = SOPC_String_Copy(&oldValue->IndexRange, &newValue->IndexRange);

        if (SOPC_STATUS_OK == retStatus)
        {
            retStatus = SOPC_NodeId_Copy(&oldValue->NodeId, &newValue->NodeId);
        }

        /* Generate data changed event with old & new WriteValue */
        if (SOPC_STATUS_OK == retStatus)
        {
            SOPC_Services_InternalEnqueueEvent(SE_TO_SE_SERVER_DATA_CHANGED, 0, oldValue, (uintptr_t) newValue);
        }
        else
        {
            // Delete the WriteValues allocated
            OpcUa_WriteValue_Clear(oldValue);
            free(oldValue);
            OpcUa_WriteValue_Clear(newValue);
            free(newValue);
        }
    }
    else
    {
        SOPC_Variant_Clear(oldVariantValue);
        free(oldVariantValue);
        OpcUa_WriteValue_Clear(newValue);
        free(newValue);

        SOPC_Logger_TraceError(
            "gen_subscription_event_bs__gen_data_changed_event: failed to generate a data changed event (out of "
            "memory for wv alloc)");
    }
}

void gen_subscription_event_bs__gen_data_changed_event_failed(
    const constants__t_Variant_i gen_subscription_event_bs__p_prev_value)
{
    SOPC_Variant_Clear(gen_subscription_event_bs__p_prev_value);
    free(gen_subscription_event_bs__p_prev_value);

    SOPC_Logger_TraceError(
        "gen_subscription_event_bs__gen_data_changed_event_failed: failed to generate a data changed event (out of "
        "memory for wv copy)");
}
