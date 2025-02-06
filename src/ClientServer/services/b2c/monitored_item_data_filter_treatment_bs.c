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

#include "monitored_item_data_filter_treatment_bs.h"

#include <string.h>

#include "address_space_impl.h"
#include "monitored_item_pointer_impl.h"
#include "sopc_address_space_utils_internal.h"
#include "sopc_assert.h"
#include "sopc_mem_alloc.h"

static const SOPC_NodeId Number_DataType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Number);

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void monitored_item_data_filter_treatment_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

#define InputArguments_BrowseName "EURange"

static bool is_EURange(const OpcUa_VariableNode* node)
{
    if (NULL == node || &OpcUa_VariableNode_EncodeableType != node->encodeableType)
    {
        return false;
    }

    /* Type shall be Range */
    if (!(SOPC_IdentifierType_Numeric == node->DataType.IdentifierType && OpcUaId_Range == node->DataType.Data.Numeric))
    {
        return false;
    }

    /* BrowseName shall be EURange (and should be in NS=O but we don't check it) */
    return (strcmp(SOPC_String_GetRawCString(&node->BrowseName.Name), InputArguments_BrowseName) == 0);
}

static bool check_percent_deadband_filter_allowed(SOPC_AddressSpace_Node* variableNode, OpcUa_Range** range)
{
    SOPC_AddressSpace_Node* targetNode = NULL;
    bool found = false;
    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, variableNode);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, variableNode);
    SOPC_Variant* euRangeVariant = NULL;
    OpcUa_Range* euRangeValue = NULL;
    for (int32_t i = 0; i < *n_refs && NULL == euRangeVariant; ++i)
    { /* stop when input argument is found */
        OpcUa_ReferenceNode* ref = &(*refs)[i];
        if (SOPC_AddressSpaceUtil_IsProperty(ref))
        {
            if (ref->TargetId.ServerIndex == 0 && ref->TargetId.NamespaceUri.Length <= 0)
            { // Shall be on same server and shall use only NodeId
                targetNode = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, &ref->TargetId.NodeId, &found);
                if (found && NULL != targetNode && OpcUa_NodeClass_Variable == targetNode->node_class)
                {
                    if (is_EURange(&targetNode->data.variable))
                    {
                        euRangeVariant = SOPC_AddressSpace_Get_Value(address_space_bs__nodes, targetNode);
                    }
                }
            }
        }
    }
    if (euRangeVariant != NULL && SOPC_VariantArrayType_SingleValue == euRangeVariant->ArrayType &&
        SOPC_ExtensionObject_Id == euRangeVariant->BuiltInTypeId &&
        SOPC_ExtObjBodyEncoding_Object == euRangeVariant->Value.ExtObject->Encoding &&
        &OpcUa_Range_EncodeableType == euRangeVariant->Value.ExtObject->Body.Object.ObjType)
    {
        /* Important note: here we make the choice that the EURange for the MI is the one during MI
         * creation/modification, the EURange value / node might be changed in the future but we will ignore it. */
        euRangeValue = (OpcUa_Range*) euRangeVariant->Value.ExtObject->Body.Object.Value;
        if (euRangeValue->High >= euRangeValue->Low)
        {
            *range = euRangeValue;
            return true;
        }
    }
    return false;
}

void monitored_item_data_filter_treatment_bs__check_monitored_item_data_filter_valid(
    const constants__t_Node_i monitored_item_data_filter_treatment_bs__p_node,
    const constants__t_monitoringFilter_i monitored_item_data_filter_treatment_bs__p_filter,
    constants_statuscodes_bs__t_StatusCode_i* const monitored_item_data_filter_treatment_bs__StatusCode,
    constants__t_monitoringFilterCtx_i* const monitored_item_data_filter_treatment_bs__filterCtx)
{
    SOPC_ASSERT(constants_bs__c_monitoringFilter_indet != monitored_item_data_filter_treatment_bs__p_filter);
    *monitored_item_data_filter_treatment_bs__filterCtx = constants_bs__c_monitoringFilterCtx_indet;
    *monitored_item_data_filter_treatment_bs__StatusCode = constants_statuscodes_bs__e_sc_bad_filter_not_allowed;
    if (&OpcUa_DataChangeFilter_EncodeableType !=
        monitored_item_data_filter_treatment_bs__p_filter->Body.Object.ObjType)
    {
        return;
    }
    OpcUa_DataChangeFilter* dataChangeFilter =
        (OpcUa_DataChangeFilter*) monitored_item_data_filter_treatment_bs__p_filter->Body.Object.Value;
    OpcUa_NodeClass* ncl = NULL;
    SOPC_NodeId* dataTypeId = NULL;
    bool result = false;
    OpcUa_Range* euRange = NULL;

    SOPC_InternalMonitoredItemFilterCtx* filterCtx = SOPC_Calloc(1, sizeof(*filterCtx));
    if (NULL == filterCtx)
    {
        *monitored_item_data_filter_treatment_bs__StatusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
    else
    {
        filterCtx->isDataFilter = true;
        switch (dataChangeFilter->DeadbandType)
        {
        case OpcUa_DeadbandType_None:
            *monitored_item_data_filter_treatment_bs__StatusCode = constants_statuscodes_bs__e_sc_ok;
            break;
        case OpcUa_DeadbandType_Absolute:
            ncl = SOPC_AddressSpace_Get_NodeClass(address_space_bs__nodes,
                                                  monitored_item_data_filter_treatment_bs__p_node);
            SOPC_ASSERT(NULL != ncl);
            if (OpcUa_NodeClass_Variable == *ncl)
            {
                dataTypeId = SOPC_AddressSpace_Get_DataType(address_space_bs__nodes,
                                                            monitored_item_data_filter_treatment_bs__p_node);
                result = SOPC_NodeId_Equal(dataTypeId, &Number_DataType);
                if (!result)
                {
                    result = SOPC_AddressSpaceUtil_RecursiveIsTransitiveSubtype(
                        address_space_bs__nodes, RECURSION_LIMIT, dataTypeId, dataTypeId, &Number_DataType);
                }

                if (result)
                {
                    /* Keep the absolute deadband value as context */
                    filterCtx->Filter.Data.filterAbsoluteDeadbandContext = dataChangeFilter->DeadbandValue;
                    *monitored_item_data_filter_treatment_bs__StatusCode = constants_statuscodes_bs__e_sc_ok;
                }
            }
            break;
        case OpcUa_DeadbandType_Percent:
            ncl = SOPC_AddressSpace_Get_NodeClass(address_space_bs__nodes,
                                                  monitored_item_data_filter_treatment_bs__p_node);
            SOPC_ASSERT(NULL != ncl);
            if (OpcUa_NodeClass_Variable == *ncl)
            {
                result =
                    check_percent_deadband_filter_allowed(monitored_item_data_filter_treatment_bs__p_node, &euRange);
                if (result)
                {
                    /* Pre-compute the deadband as an absolute value using formula from Part 8 */
                    filterCtx->Filter.Data.filterAbsoluteDeadbandContext =
                        (dataChangeFilter->DeadbandValue / 100.0) * (euRange->High - euRange->Low);
                    *monitored_item_data_filter_treatment_bs__StatusCode = constants_statuscodes_bs__e_sc_ok;
                }
            }
            break;
        default:
            // Already checked when retrieved in message
            SOPC_ASSERT(false && "invalid deadband type");
        }
    }
    if (constants_statuscodes_bs__e_sc_ok == *monitored_item_data_filter_treatment_bs__StatusCode)
    {
        filterCtx->Filter.Data.dataFilter = *dataChangeFilter;
        *monitored_item_data_filter_treatment_bs__filterCtx = filterCtx;
    }
    else
    {
        SOPC_Free(filterCtx);
    }
}
