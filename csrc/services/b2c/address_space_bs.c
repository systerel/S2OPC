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

void address_space_bs__read_AddressSpace_AccessLevel_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable);
    SOPC_Byte accessLevel = address_space_bs__p_node->data.variable.AccessLevel;
    // Note: keep only supported access level flags
    accessLevel = (accessLevel & (SOPC_AccessLevelMask_CurrentRead | SOPC_AccessLevelMask_CurrentWrite |
                                  SOPC_AccessLevelMask_StatusWrite | SOPC_AccessLevelMask_TimestampWrite));
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_Byte(accessLevel);
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_BrowseName_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant =
        util_variant__new_Variant_from_QualifiedName(SOPC_AddressSpace_Item_Get_BrowseName(address_space_bs__p_node));
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_ContainsNoLoops_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_View);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    // Note: always returns false since we do not check this property
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(false);
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_DataType_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable ||
           address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant =
        util_variant__new_Variant_from_NodeId(SOPC_AddressSpace_Item_Get_DataType(address_space_bs__p_node));
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_DisplayName_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant =
        util_variant__new_Variant_from_LocalizedText(SOPC_AddressSpace_Item_Get_DisplayName(address_space_bs__p_node));
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_EventNotifier_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_View ||
           address_space_bs__p_node->node_class == OpcUa_NodeClass_Object);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    // Note: always returns 0 since we do not implement events
    *address_space_bs__variant = util_variant__new_Variant_from_Byte(0);
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_Executable_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Method);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    // Note: always returns false since we do not implement method execution
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(false);
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_Historizing_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    // Note: always returns false since we do not implement historization
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(false);
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_IsAbstract_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType ||
           address_space_bs__p_node->node_class == OpcUa_NodeClass_ObjectType ||
           address_space_bs__p_node->node_class == OpcUa_NodeClass_ReferenceType ||
           address_space_bs__p_node->node_class == OpcUa_NodeClass_DataType);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant =
        util_variant__new_Variant_from_Bool(SOPC_AddressSpace_Item_Get_IsAbstract(address_space_bs__p_node));
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_NodeClass_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_NodeClass(address_space_bs__p_node->node_class);
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_NodeId_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__variant =
        util_variant__new_Variant_from_NodeId(SOPC_AddressSpace_Item_Get_NodeId(address_space_bs__p_node));
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}
void address_space_bs__read_AddressSpace_Symmetric_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_ReferenceType);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant =
        util_variant__new_Variant_from_Bool(address_space_bs__p_node->data.reference_type.Symmetric);
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_UserAccessLevel_value(
    const constants__t_Node_i address_space_bs__p_node,
    const t_bool address_space_bs__p_is_user_read_auth,
    const t_bool address_space_bs__p_is_user_write_auth,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable);
    SOPC_Byte accessLevel = address_space_bs__p_node->data.variable.AccessLevel;
    SOPC_Byte userAccessLevel = 0;
    if (address_space_bs__p_is_user_read_auth)
    {
        // Keep supported read flags
        userAccessLevel = accessLevel & SOPC_AccessLevelMask_CurrentRead;
    }
    if (address_space_bs__p_is_user_write_auth)
    {
        // Keep supported write flags
        uint8_t supportedWriteFlags =
            SOPC_AccessLevelMask_CurrentWrite | SOPC_AccessLevelMask_StatusWrite | SOPC_AccessLevelMask_TimestampWrite;
        userAccessLevel |= (accessLevel & supportedWriteFlags);
    }

    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_Byte(userAccessLevel);
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_UserExecutable_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Method);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    // Note: always returns false since we do not implement method execution
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(false);
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_ValueRank_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable ||
           address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant =
        util_variant__new_Variant_from_int32(*SOPC_AddressSpace_Item_Get_ValueRank(address_space_bs__p_node));
    if (*address_space_bs__variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
    }
}

void address_space_bs__read_AddressSpace_Value_value(
    const constants__t_Node_i address_space_bs__p_node,
    const constants__t_IndexRange_i address_space_bs__index_range,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant,
    constants__t_RawStatusCode* const address_space_bs__val_sc,
    constants__t_Timestamp* const address_space_bs__val_ts_src,
    constants__t_Timestamp* const address_space_bs__val_ts_srv)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable ||
           address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType);
    *address_space_bs__val_sc = OpcUa_BadInvalidState;
    *address_space_bs__val_ts_src = constants_bs__c_Timestamp_null;
    *address_space_bs__val_ts_srv = constants_bs__c_Timestamp_null;

    SOPC_Variant* value =
        util_variant__new_Variant_from_Variant(SOPC_AddressSpace_Item_Get_Value(address_space_bs__p_node));

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
                util_read_value_string_indexed(*address_space_bs__variant, value, address_space_bs__index_range);

            if (constants_statuscodes_bs__e_sc_ok != *address_space_bs__sc)
            {
                SOPC_Variant_Delete(*address_space_bs__variant);
                *address_space_bs__variant = NULL;
            }
        }

        SOPC_Variant_Delete(value);
    }

    if (constants_statuscodes_bs__e_sc_ok == *address_space_bs__sc)
    {
        if (address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable)
        {
            *address_space_bs__val_sc = address_space_bs__p_node->value_status;
            *address_space_bs__val_ts_src = address_space_bs__p_node->value_source_ts;
            address_space_bs__val_ts_srv->timestamp = SOPC_Time_GetCurrentTimeUTC();
            address_space_bs__val_ts_srv->picoSeconds = 0;
        }
        else
        {
            *address_space_bs__val_sc = SOPC_GoodGenericStatus;
        }
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
        return util_return_status__C_to_status_code_B(status);
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

static SOPC_Variant* convertVariantType_ByteArrayByteString(SOPC_Variant* toConvert)
{
    SOPC_Variant* result = SOPC_Variant_Create();

    if (NULL == result)
    {
        return NULL;
    }

    if (SOPC_Byte_Id == toConvert->BuiltInTypeId && SOPC_VariantArrayType_Array == toConvert->ArrayType)
    {
        // Byte[] => ByteString
        result->ArrayType = SOPC_VariantArrayType_SingleValue;
        result->BuiltInTypeId = SOPC_ByteString_Id;
        result->DoNotClear = true; // We do not actually copy the content

        if (toConvert->Value.Array.Length > 0)
        {
            result->Value.Bstring.Length = toConvert->Value.Array.Length;
            result->Value.Bstring.Data = toConvert->Value.Array.Content.ByteArr;
        } // Otherwise NULL ByteString since array of length <= 0
    }
    else if (SOPC_ByteString_Id == toConvert->BuiltInTypeId &&
             SOPC_VariantArrayType_SingleValue == toConvert->ArrayType)
    {
        // ByteString => Byte[]
        result->ArrayType = SOPC_VariantArrayType_Array;
        result->BuiltInTypeId = SOPC_Byte_Id;
        result->DoNotClear = true; // We do not actually copy the content

        if (toConvert->Value.Bstring.Length > 0)
        {
            result->Value.Array.Length = toConvert->Value.Bstring.Length;
            result->Value.Array.Content.ByteArr = toConvert->Value.Bstring.Data;
        } // Otherwise empty Byte[] since ByteString of length <= 0
    }
    else
    {
        // It shall be a Byte[] or a ByteString
        assert(false);
    }

    return result;
}

void address_space_bs__set_Value(const constants__t_user_i address_space_bs__p_user,
                                 const constants__t_Node_i address_space_bs__node,
                                 const constants__t_Variant_i address_space_bs__variant,
                                 const t_bool address_space_bs__toConvert,
                                 const constants__t_IndexRange_i address_space_bs__index_range,
                                 constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__serviceStatusCode,
                                 constants__t_DataValue_i* const address_space_bs__prev_dataValue)
{
    (void) (address_space_bs__p_user); /* Keep for B precondition: user is already authorized for this operation */
    SOPC_AddressSpace_Item* item = address_space_bs__node;
    SOPC_Variant* pvar = SOPC_AddressSpace_Item_Get_Value(item);
    SOPC_Variant* convertedValue = NULL;
    const SOPC_Variant* newValue = address_space_bs__variant;

    *address_space_bs__prev_dataValue = SOPC_Calloc(1, sizeof(SOPC_DataValue));

    if (address_space_bs__toConvert)
    {
        convertedValue = convertVariantType_ByteArrayByteString(address_space_bs__variant);
        newValue = convertedValue;
    }

    if (NULL == *address_space_bs__prev_dataValue || NULL == newValue)
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
    }
    else
    {
        SOPC_DataValue_Clear(*address_space_bs__prev_dataValue);
        free(*address_space_bs__prev_dataValue);
        *address_space_bs__prev_dataValue = NULL;
    }

    if (address_space_bs__toConvert)
    {
        SOPC_Variant_Delete(convertedValue);
    }
}

void address_space_bs__set_Value_SourceTimestamp(const constants__t_user_i address_space_bs__p_user,
                                                 const constants__t_Node_i address_space_bs__p_node,
                                                 const constants__t_Timestamp address_space_bs__p_ts)
{
    (void) (address_space_bs__p_user); /* Keep for B precondition: user is already authorized for this operation */
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    assert(item->node_class == OpcUa_NodeClass_Variable);
    if (address_space_bs__p_ts.timestamp == 0 && address_space_bs__p_ts.picoSeconds == 0)
    {
        // Update source timestamp with current date if no date provided
        item->value_source_ts.timestamp = SOPC_Time_GetCurrentTimeUTC();
    }
    else
    {
        item->value_source_ts = address_space_bs__p_ts;
    }
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

void address_space_bs__is_IndexRangeDefined(const constants__t_IndexRange_i address_space_bs__p_index_range,
                                            t_bool* const address_space_bs__bres)
{
    *address_space_bs__bres = false;
    if (NULL != address_space_bs__p_index_range)
    {
        *address_space_bs__bres = (address_space_bs__p_index_range->Length > 0);
    }
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
    *address_space_bs__p_access_level = SOPC_AddressSpace_Item_Get_AccessLevel(item);
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
    *address_space_bs__p_value_rank = *SOPC_AddressSpace_Item_Get_ValueRank(item);
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
    *address_space_bs__p_ref_index = *n_refs;
}

void address_space_bs__get_RefIndex_Reference(const constants__t_Node_i address_space_bs__p_node,
                                              const t_entier4 address_space_bs__p_ref_index,
                                              constants__t_Reference_i* const address_space_bs__p_ref)
{
    assert(NULL != address_space_bs__p_node);
    assert(address_space_bs__p_ref_index > 0);
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Item_Get_References(item);
    int32_t* n_refs = SOPC_AddressSpace_Item_Get_NoOfReferences(item);
    assert(address_space_bs__p_ref_index <= *n_refs);

    *address_space_bs__p_ref = &(*refs)[address_space_bs__p_ref_index - 1];
}
