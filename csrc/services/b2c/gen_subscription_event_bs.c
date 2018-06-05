/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gen_subscription_event_bs.h"

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
        // TODO: logger ?
        SOPC_Variant_Clear(oldVariantValue);
        free(oldVariantValue);
        OpcUa_WriteValue_Clear(newValue);
        free(newValue);
    }
}
