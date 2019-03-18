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

/** \file
 *
 * Implements the structures behind the address space.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "address_space_bs.h"
#include "b2c.h"

#include "address_space_impl.h"
#include "opcua_identifiers.h"
#include "sopc_builtintypes.h"
#include "sopc_dict.h"
#include "sopc_logger.h"
#include "sopc_numeric_range.h"
#include "sopc_user_manager.h"
#include "util_b2c.h"
#include "util_variant.h"

bool sopc_addressSpace_configured = false;
SOPC_AddressSpace* address_space_bs__nodes = NULL;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_bs__INITIALISATION(void)
{
    if (sopc_addressSpace_configured)
    {
        assert(NULL != address_space_bs__nodes);
    }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

/* This is a_NodeId~ */
void address_space_bs__readall_AddressSpace_Node(const constants__t_NodeId_i address_space_bs__nid,
                                                 t_bool* const address_space_bs__nid_valid,
                                                 constants__t_Node_i* const address_space_bs__node)
{
    SOPC_NodeId* pnid_req;
    bool val_found = false;
    void* val;

    *address_space_bs__nid_valid = false;

    pnid_req = address_space_bs__nid;

    if (NULL == pnid_req)
        return;

    val = SOPC_Dict_Get(address_space_bs__nodes, pnid_req, &val_found);

    if (val_found)
    {
        *address_space_bs__nid_valid = true;
        *address_space_bs__node = val;
    }
}

static constants_statuscodes_bs__t_StatusCode_i index_range_bad_returnstatus_to_service_statuscode(
    SOPC_ReturnStatus status)
{
    switch (status)
    {
    case SOPC_STATUS_OUT_OF_MEMORY:
        return constants_statuscodes_bs__e_sc_bad_out_of_memory;
    default:
        SOPC_Logger_TraceWarning("index_range_statuscode: internal error generated from return status code %d", status);
        return constants_statuscodes_bs__e_sc_bad_internal_error;
    }
}

static constants_statuscodes_bs__t_StatusCode_i read_value_indexed_helper(const SOPC_Variant* value,
                                                                          const SOPC_NumericRange* range,
                                                                          SOPC_Variant* dereferenced)
{
    bool has_range = false;
    SOPC_ReturnStatus status = SOPC_Variant_HasRange(value, range, &has_range);

    if (status != SOPC_STATUS_OK)
    {
        if (SOPC_STATUS_NOT_SUPPORTED == status)
        {
            SOPC_Logger_TraceWarning("read_value_indexed: matrix index range not supported");
        }

        return constants_statuscodes_bs__e_sc_bad_index_range_invalid; // In case we do not support  the range either
                                                                       // (matrix)
    }

    if (!has_range)
    {
        return constants_statuscodes_bs__e_sc_bad_index_range_no_data;
    }

    status = SOPC_Variant_GetRange(dereferenced, value, range);

    if (status != SOPC_STATUS_OK)
    {
        return index_range_bad_returnstatus_to_service_statuscode(status);
    }

    return constants_statuscodes_bs__e_sc_ok;
}

static constants_statuscodes_bs__t_StatusCode_i read_value_indexed(const SOPC_Variant* value,
                                                                   const SOPC_String* range_str,
                                                                   SOPC_Variant* dereferenced)
{
    SOPC_NumericRange* range = NULL;
    SOPC_ReturnStatus status = SOPC_NumericRange_Parse(SOPC_String_GetRawCString(range_str), &range);

    if (status != SOPC_STATUS_OK)
    {
        return (status == SOPC_STATUS_NOK) ? constants_statuscodes_bs__e_sc_bad_index_range_invalid
                                           : index_range_bad_returnstatus_to_service_statuscode(status);
    }

    constants_statuscodes_bs__t_StatusCode_i ret = read_value_indexed_helper(value, range, dereferenced);
    SOPC_NumericRange_Delete(range);

    return ret;
}

/* Reads any attribute and outputs a variant (valid or not)
 * As this function uses the *_2_Variant_i functions, the value must be freed once used
 */
void address_space_bs__read_AddressSpace_Attribute_value(
    const constants__t_user_i address_space_bs__p_user,
    const constants__t_Node_i address_space_bs__node,
    const constants__t_NodeClass_i address_space_bs__ncl,
    const constants__t_AttributeId_i address_space_bs__aid,
    const constants__t_IndexRange_i address_space_bs__index_range,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    (void) (address_space_bs__p_user); /* Keep for B precondition: User is already authorized for this operation */

    assert(NULL != address_space_bs__node);
    SOPC_AddressSpace_Item* item = address_space_bs__node;
    SOPC_Variant* value = NULL;

    /* Note: conv_* variables are abstract, we must be confident */

    switch (address_space_bs__aid)
    {
    case constants__e_aid_NodeId:
        value = util_variant__new_Variant_from_NodeId(SOPC_AddressSpace_Item_Get_NodeId(item));
        break;
    case constants__e_aid_NodeClass:
        value = util_variant__new_Variant_from_NodeClass(item->node_class);
        break;
    case constants__e_aid_BrowseName:
        value = util_variant__new_Variant_from_QualifiedName(SOPC_AddressSpace_Item_Get_BrowseName(item));
        break;
    case constants__e_aid_DisplayName:
        value = util_variant__new_Variant_from_LocalizedText(SOPC_AddressSpace_Item_Get_DisplayName(item));
        break;
    case constants__e_aid_Value:
        if (constants__e_ncl_Variable == address_space_bs__ncl ||
            constants__e_ncl_VariableType == address_space_bs__ncl)
        {
            value = util_variant__new_Variant_from_Variant(SOPC_AddressSpace_Item_Get_Value(item));
        }
        else
        {
            *address_space_bs__variant = constants__c_Variant_indet;
            *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_attribute_id_invalid;
            return;
        }
        break;
    case constants__e_aid_DataType:
        if (constants__e_ncl_Variable == address_space_bs__ncl)
        {
            value = util_variant__new_Variant_from_NodeId(&item->data.variable.DataType);
        }
        else if (constants__e_ncl_VariableType == address_space_bs__ncl)
        {
            /* TODO: data not parsed for now */
            value = util_variant__new_Variant_from_NodeId(&item->data.variable_type.DataType);
        }
        else
        {
            *address_space_bs__variant = util_variant__new_Variant_from_Indet();
            *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_attribute_id_invalid;
            return;
        }
        break;
    case constants__e_aid_ValueRank:
        if (constants__e_ncl_Variable == address_space_bs__ncl)
        {
            value = util_variant__new_Variant_from_int32(item->data.variable.ValueRank);
        }
        else if (constants__e_ncl_VariableType == address_space_bs__ncl)
        {
            value = util_variant__new_Variant_from_int32(item->data.variable.ValueRank);
        }
        else
        {
            *address_space_bs__variant = util_variant__new_Variant_from_Indet();
            *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_attribute_id_invalid;
            return;
        }
        break;
    case constants__e_aid_EventNotifier:
        if (constants__e_ncl_Object == address_space_bs__ncl || constants__e_ncl_View == address_space_bs__ncl)
        {
            // We do not manage event in any way
            value = util_variant__new_Variant_from_Byte(0);
        }
        else
        {
            *address_space_bs__variant = util_variant__new_Variant_from_Indet();
            *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_attribute_id_invalid;
            return;
        }
        break;
    case constants__e_aid_AccessLevel:
        value =
            util_variant__new_Variant_from_Byte(SOPC_AccessLevelMask_CurrentRead | SOPC_AccessLevelMask_CurrentWrite);
        break;
    case constants__e_aid_UserAccessLevel:
        value =
            util_variant__new_Variant_from_Byte(SOPC_AccessLevelMask_CurrentRead | SOPC_AccessLevelMask_CurrentWrite);
        break;
    default:
        *address_space_bs__variant = constants__c_Variant_indet;
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_attribute_id_invalid;
        return;
    }

    if (value == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
        return;
    }

    if (address_space_bs__index_range == NULL || address_space_bs__index_range->Length <= 0)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
        *address_space_bs__variant = value;
    }
    else
    {
        *address_space_bs__variant = SOPC_Variant_Create();

        if (*address_space_bs__variant == NULL)
        {
            *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        }
        else
        {
            *address_space_bs__sc =
                read_value_indexed(value, address_space_bs__index_range, *address_space_bs__variant);
        }

        SOPC_Variant_Delete(value);
    }
}

static constants_statuscodes_bs__t_StatusCode_i set_value_full(SOPC_Variant* node_value,
                                                               const SOPC_Variant* new_value,
                                                               SOPC_Variant* previous_value)
{
    SOPC_Variant_Move(previous_value, node_value);
    SOPC_Variant_Clear(node_value);
    SOPC_Variant_Initialize(node_value);
    SOPC_ReturnStatus status = SOPC_Variant_Copy(node_value, new_value);

    return (status == SOPC_STATUS_OK) ? constants_statuscodes_bs__e_sc_ok
                                      : constants_statuscodes_bs__e_sc_bad_internal_error;
}

static constants_statuscodes_bs__t_StatusCode_i set_value_indexed_helper(SOPC_Variant* node_value,
                                                                         const SOPC_Variant* new_value,
                                                                         const SOPC_NumericRange* range,
                                                                         SOPC_Variant* previous_value)
{
    bool has_range = false;
    SOPC_ReturnStatus status = SOPC_Variant_HasRange(node_value, range, &has_range);

    if (status != SOPC_STATUS_OK)
    {
        if (SOPC_STATUS_NOT_SUPPORTED == status)
        {
            SOPC_Logger_TraceWarning("set_value_indexed: matrix index range not supported");
        }

        return constants_statuscodes_bs__e_sc_bad_index_range_invalid; // In case we do not support  the range either
                                                                       // (matrix)
    }

    if (!has_range)
    {
        return constants_statuscodes_bs__e_sc_bad_index_range_no_data;
    }

    status = SOPC_Variant_Copy(previous_value, node_value);

    if (status != SOPC_STATUS_OK)
    {
        return constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }

    status = SOPC_Variant_SetRange(node_value, new_value, range);

    if (status != SOPC_STATUS_OK)
    {
        return index_range_bad_returnstatus_to_service_statuscode(status);
    }

    return constants_statuscodes_bs__e_sc_ok;
}

static constants_statuscodes_bs__t_StatusCode_i set_value_indexed(SOPC_Variant* node_value,
                                                                  const SOPC_Variant* new_value,
                                                                  const SOPC_String* range_str,
                                                                  SOPC_Variant* previous_value)
{
    SOPC_NumericRange* range = NULL;
    SOPC_ReturnStatus status = SOPC_NumericRange_Parse(SOPC_String_GetRawCString(range_str), &range);

    if (status != SOPC_STATUS_OK)
    {
        return (status == SOPC_STATUS_NOK) ? constants_statuscodes_bs__e_sc_bad_index_range_invalid
                                           : constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }

    constants_statuscodes_bs__t_StatusCode_i ret =
        set_value_indexed_helper(node_value, new_value, range, previous_value);
    SOPC_NumericRange_Delete(range);

    return ret;
}

void address_space_bs__set_Value(const constants__t_user_i address_space_bs__p_user,
                                 const constants__t_Node_i address_space_bs__node,
                                 const constants__t_Variant_i address_space_bs__variant,
                                 const constants__t_IndexRange_i address_space_bs__index_range,
                                 constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__serviceStatusCode,
                                 constants__t_DataValue_i* const address_space_bs__prev_dataValue)
{
    (void) (address_space_bs__p_user); /* Keep for B precondition: user is already authorized for this operation */
    SOPC_AddressSpace_Item* item = address_space_bs__node;
    SOPC_Variant* pvar = SOPC_AddressSpace_Item_Get_Value(item);
    *address_space_bs__prev_dataValue = calloc(1, sizeof(SOPC_DataValue));

    if (*address_space_bs__prev_dataValue == NULL)
    {
        *address_space_bs__serviceStatusCode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        return;
    }

    if (address_space_bs__index_range->Length <= 0)
    {
        *address_space_bs__serviceStatusCode =
            set_value_full(pvar, address_space_bs__variant, &(*address_space_bs__prev_dataValue)->Value);
    }
    else
    {
        *address_space_bs__serviceStatusCode =
            set_value_indexed(pvar, address_space_bs__variant, address_space_bs__index_range,
                              &(*address_space_bs__prev_dataValue)->Value);
    }

    if (*address_space_bs__serviceStatusCode == constants_statuscodes_bs__e_sc_ok)
    {
        (*address_space_bs__prev_dataValue)->Status = item->value_status;
        (*address_space_bs__prev_dataValue)->SourceTimestamp = item->value_source_ts.timestamp;
        (*address_space_bs__prev_dataValue)->SourcePicoSeconds = item->value_source_ts.picoSeconds;
        (*address_space_bs__prev_dataValue)->ServerTimestamp = item->value_server_ts.timestamp;
        (*address_space_bs__prev_dataValue)->ServerPicoSeconds = item->value_server_ts.picoSeconds;

        item->value_server_ts.timestamp = SOPC_Time_GetCurrentTimeUTC();
        item->value_server_ts.picoSeconds = 0;
    }
    else
    {
        SOPC_DataValue_Clear(*address_space_bs__prev_dataValue);
        free(*address_space_bs__prev_dataValue);
        *address_space_bs__prev_dataValue = NULL;
    }
}

void address_space_bs__set_Value_SourceTimestamp(const constants__t_user_i address_space_bs__p_user,
                                                 const constants__t_Node_i address_space_bs__p_node,
                                                 const constants__t_Timestamp address_space_bs__p_ts)
{
    (void) (address_space_bs__p_user); /* Keep for B precondition: user is already authorized for this operation */
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    assert(item->node_class == OpcUa_NodeClass_Variable);
    item->value_source_ts = address_space_bs__p_ts;
}

void address_space_bs__set_Value_StatusCode(const constants__t_user_i address_space_bs__p_user,
                                            const constants__t_Node_i address_space_bs__p_node,
                                            const constants__t_RawStatusCode address_space_bs__p_sc)
{
    (void) (address_space_bs__p_user); /* Keep for B precondition: user is already authorized for this operation */
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    assert(item->node_class == OpcUa_NodeClass_Variable);
    item->value_status = address_space_bs__p_sc;
}

void address_space_bs__get_Value_StatusCode(const constants__t_user_i address_space_bs__p_user,
                                            const constants__t_Node_i address_space_bs__node,
                                            constants__t_RawStatusCode* const address_space_bs__sc)
{
    (void) (address_space_bs__p_user); /* User is already authorized for this operation */
    SOPC_AddressSpace_Item* item = address_space_bs__node;
    *address_space_bs__sc = item->value_status;
}

void address_space_bs__is_NodeId_equal(const constants__t_NodeId_i address_space_bs__nid1,
                                       const constants__t_NodeId_i address_space_bs__nid2,
                                       t_bool* const address_space_bs__bres)
{
    *address_space_bs__bres = SOPC_NodeId_Equal(address_space_bs__nid1, address_space_bs__nid2);
}

void address_space_bs__read_AddressSpace_clear_value(const constants__t_Variant_i address_space_bs__val)
{
    SOPC_Variant_Clear(address_space_bs__val);
}

void address_space_bs__read_AddressSpace_free_variant(const constants__t_Variant_i address_space_bs__val)
{
    SOPC_Variant_Delete(address_space_bs__val);
}

void address_space_bs__write_AddressSpace_free_dataValue(const constants__t_DataValue_i address_space_bs__data)
{
    SOPC_DataValue_Clear(address_space_bs__data);
    free(address_space_bs__data);
}

void address_space_bs__get_AccessLevel(const constants__t_Node_i address_space_bs__p_node,
                                       constants__t_access_level* const address_space_bs__p_access_level)
{
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    *address_space_bs__p_access_level = *SOPC_AddressSpace_Item_Get_AccessLevel(item);
}

void address_space_bs__get_BrowseName(const constants__t_Node_i address_space_bs__p_node,
                                      constants__t_QualifiedName_i* const address_space_bs__p_browse_name)
{
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    *address_space_bs__p_browse_name = SOPC_AddressSpace_Item_Get_BrowseName(item);
}

void address_space_bs__get_DisplayName(const constants__t_Node_i address_space_bs__p_node,
                                       constants__t_LocalizedText_i* const address_space_bs__p_display_name)
{
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    *address_space_bs__p_display_name = SOPC_AddressSpace_Item_Get_DisplayName(item);
}

void address_space_bs__get_NodeClass(const constants__t_Node_i address_space_bs__p_node,
                                     constants__t_NodeClass_i* const address_space_bs__p_node_class)
{
    assert(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;

    bool res = util_NodeClass__C_to_B(item->node_class, address_space_bs__p_node_class);
    if (false == res)
    {
        *address_space_bs__p_node_class = constants__c_NodeClass_indet;
    }
}

void address_space_bs__get_DataType(const constants__t_Node_i address_space_bs__p_node,
                                    constants__t_NodeId_i* const address_space_bs__p_data_type)
{
    assert(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    *address_space_bs__p_data_type = SOPC_AddressSpace_Item_Get_DataType(item);
}

void address_space_bs__get_ValueRank(const constants__t_Node_i address_space_bs__p_node,
                                     t_entier4* const address_space_bs__p_value_rank)
{
    assert(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    *address_space_bs__p_value_rank = SOPC_AddressSpace_Item_Get_ValueRank(item);
}

static bool is_type_definition(const OpcUa_ReferenceNode* ref)
{
    if (ref->IsInverse)
    {
        return false;
    }

    return ref->ReferenceTypeId.IdentifierType == SOPC_IdentifierType_Numeric &&
           ref->ReferenceTypeId.Data.Numeric == OpcUaId_HasTypeDefinition;
}

void address_space_bs__get_TypeDefinition(const constants__t_Node_i address_space_bs__p_node,
                                          constants__t_ExpandedNodeId_i* const address_space_bs__p_type_def)
{
    assert(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    int32_t* n_refs = SOPC_AddressSpace_Item_Get_NoOfReferences(item);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Item_Get_References(item);
    *address_space_bs__p_type_def = constants__c_ExpandedNodeId_indet;

    for (int32_t i = 0; i < *n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &(*refs)[i];

        if (is_type_definition(ref))
        {
            *address_space_bs__p_type_def = &ref->TargetId;
            break;
        }
    }
}

void address_space_bs__get_Reference_ReferenceType(const constants__t_Reference_i address_space_bs__p_ref,
                                                   constants__t_NodeId_i* const address_space_bs__p_RefType)
{
    OpcUa_ReferenceNode* ref = address_space_bs__p_ref;
    *address_space_bs__p_RefType = &ref->ReferenceTypeId;
}

void address_space_bs__get_Reference_TargetNode(const constants__t_Reference_i address_space_bs__p_ref,
                                                constants__t_ExpandedNodeId_i* const address_space_bs__p_TargetNode)
{
    OpcUa_ReferenceNode* ref = address_space_bs__p_ref;
    *address_space_bs__p_TargetNode = &ref->TargetId;
}

void address_space_bs__get_Reference_IsForward(const constants__t_Reference_i address_space_bs__p_ref,
                                               t_bool* const address_space_bs__p_IsForward)
{
    OpcUa_ReferenceNode* ref = address_space_bs__p_ref;
    *address_space_bs__p_IsForward = !ref->IsInverse;
}

void address_space_bs__get_Node_RefIndexEnd(const constants__t_Node_i address_space_bs__p_node,
                                            t_entier4* const address_space_bs__p_ref_index)
{
    assert(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    int32_t* n_refs = SOPC_AddressSpace_Item_Get_NoOfReferences(item);
    *address_space_bs__p_ref_index = *n_refs - 1;
}

void address_space_bs__get_RefIndex_Reference(const constants__t_Node_i address_space_bs__p_node,
                                              const t_entier4 address_space_bs__p_ref_index,
                                              constants__t_Reference_i* const address_space_bs__p_ref)
{
    assert(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Item_Get_References(item);
    int32_t* n_refs = SOPC_AddressSpace_Item_Get_NoOfReferences(item);
    assert(address_space_bs__p_ref_index < *n_refs);
    *address_space_bs__p_ref = &(*refs)[address_space_bs__p_ref_index];
}
