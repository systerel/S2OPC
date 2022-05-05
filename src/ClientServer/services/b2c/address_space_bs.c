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
#include <string.h>

#include "address_space_bs.h"
#include "b2c.h"
#include "util_address_space.h"

#include "address_space_impl.h"
#include "opcua_identifiers.h"
#include "sopc_builtintypes.h"
#include "sopc_dict.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_numeric_range.h"
#include "sopc_user_manager.h"
#include "util_b2c.h"
#include "util_variant.h"

bool sopc_addressSpace_configured = false;
SOPC_AddressSpace* address_space_bs__nodes = NULL;

#define sopc_address_space_bs__InputArguments_BrowseName "InputArguments"

static bool is_inputArgument(const OpcUa_VariableNode* node);

void SOPC_AddressSpace_Check_Configured(void)
{
    if (sopc_addressSpace_configured)
    {
        assert(NULL != address_space_bs__nodes);
    }
}

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_bs__INITIALISATION(void) {}

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

    val = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, pnid_req, &val_found);

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
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_ArrayDimensions_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable ||
           address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    int32_t* valueRank = SOPC_AddressSpace_Get_ValueRank(address_space_bs__nodes, address_space_bs__p_node);
    SOPC_Variant* variant = SOPC_Variant_Create();
    if (variant == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
    else
    {
        // See table 1 of part 5, we return unknown size or NULL value
        if (*valueRank > 0)
        {
            uint32_t* arrayDimensionsArray = SOPC_Calloc((size_t) *valueRank, sizeof(uint32_t));
            if (NULL != arrayDimensionsArray)
            {
                variant->BuiltInTypeId = SOPC_UInt32_Id;
                variant->ArrayType = SOPC_VariantArrayType_Array;
                variant->Value.Array.Length = *valueRank;
                variant->Value.Array.Content.Uint32Arr = arrayDimensionsArray; // All items already set to 0 by alloc
            }
            else
            {
                *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
                SOPC_Variant_Delete(variant);
                variant = NULL;
            }
        } // else NULL variant content (nothing to do in variant content)
    }
    *address_space_bs__variant = variant;
}

void address_space_bs__read_AddressSpace_BrowseName_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_QualifiedName(
        SOPC_AddressSpace_Get_BrowseName(address_space_bs__nodes, address_space_bs__p_node));
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
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
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
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
    *address_space_bs__variant = util_variant__new_Variant_from_NodeId(
        SOPC_AddressSpace_Get_DataType(address_space_bs__nodes, address_space_bs__p_node));
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_DisplayName_value(
    const constants__t_LocaleIds_i address_space_bs__p_locales,
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_LocalizedText(
        SOPC_AddressSpace_Get_DisplayName(address_space_bs__nodes, address_space_bs__p_node));
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
    else
    {
        *address_space_bs__variant = util_variant__set_PreferredLocalizedText_from_LocalizedText_Variant(
            address_space_bs__variant, address_space_bs__p_locales);
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
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_Executable_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Method);
    bool executable;
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    address_space_bs__get_Executable(address_space_bs__p_node, &executable);
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(executable);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
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
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
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
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(
        SOPC_AddressSpace_Get_IsAbstract(address_space_bs__nodes, address_space_bs__p_node));
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_NodeClass_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    *address_space_bs__variant = util_variant__new_Variant_from_NodeClass(address_space_bs__p_node->node_class);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_NodeId_value(
    const constants__t_Node_i address_space_bs__p_node,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    *address_space_bs__variant = util_variant__new_Variant_from_NodeId(
        SOPC_AddressSpace_Get_NodeId(address_space_bs__nodes, address_space_bs__p_node));
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
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
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
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
    /* UserAccess Level can be only more restrictive than access level  */
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
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_UserExecutable_value(
    const constants__t_Node_i address_space_bs__p_node,
    const t_bool address_space_bs__p_is_user_executable_auth,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Method);
    *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
    /* UserExecutable Level can be only more restrictive than executable attribute */
    *address_space_bs__variant = util_variant__new_Variant_from_Bool(address_space_bs__p_node->data.method.Executable &&
                                                                     address_space_bs__p_is_user_executable_auth);
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
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
    *address_space_bs__variant = util_variant__new_Variant_from_int32(
        *SOPC_AddressSpace_Get_ValueRank(address_space_bs__nodes, address_space_bs__p_node));
    if (NULL == *address_space_bs__variant)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    }
}

void address_space_bs__read_AddressSpace_Value_value(
    const constants__t_LocaleIds_i address_space_bs__p_locales,
    const constants__t_Node_i address_space_bs__p_node,
    const constants__t_IndexRange_i address_space_bs__index_range,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc,
    constants__t_Variant_i* const address_space_bs__variant,
    constants__t_RawStatusCode* const address_space_bs__val_sc,
    constants__t_Timestamp* const address_space_bs__val_ts_src)
{
    assert(address_space_bs__p_node->node_class == OpcUa_NodeClass_Variable ||
           address_space_bs__p_node->node_class == OpcUa_NodeClass_VariableType);
    *address_space_bs__val_sc = OpcUa_BadInvalidState;
    *address_space_bs__val_ts_src = constants_bs__c_Timestamp_null;

    SOPC_Variant* value = util_variant__new_Variant_from_Variant(
        SOPC_AddressSpace_Get_Value(address_space_bs__nodes, address_space_bs__p_node));

    if (value == NULL)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
        *address_space_bs__variant = NULL;
        return;
    }
    else
    {
        if (SOPC_LocalizedText_Id == value->BuiltInTypeId)
        {
            // Get preferred localized text(s) (single value, array or matrix)
            value = util_variant__set_PreferredLocalizedText_from_LocalizedText_Variant(&value,
                                                                                        address_space_bs__p_locales);
        }
    }

    if (address_space_bs__index_range == NULL || address_space_bs__index_range->Length <= 0)
    {
        *address_space_bs__sc = constants_statuscodes_bs__e_sc_ok;
        *address_space_bs__variant = value;
    }
    else
    {
        *address_space_bs__variant = SOPC_Variant_Create();

        if (NULL == *address_space_bs__variant)
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
            *address_space_bs__val_sc =
                SOPC_AddressSpace_Get_StatusCode(address_space_bs__nodes, address_space_bs__p_node);
            *address_space_bs__val_ts_src =
                SOPC_AddressSpace_Get_SourceTs(address_space_bs__nodes, address_space_bs__p_node);
        }
        else
        {
            *address_space_bs__val_sc = SOPC_GoodGenericStatus;
        }
    }
}

static SOPC_ReturnStatus modify_localized_text(char** supportedLocales,
                                               SOPC_Variant* node_value,
                                               const SOPC_Variant* new_value,
                                               SOPC_Variant* previous_value)
{
    assert(SOPC_LocalizedText_Id == new_value->BuiltInTypeId);
    assert(SOPC_LocalizedText_Id == node_value->BuiltInTypeId);
    assert(node_value->ArrayType == new_value->ArrayType);
    previous_value->ArrayType = node_value->ArrayType;
    previous_value->BuiltInTypeId = node_value->BuiltInTypeId;
    /* Important note: we shall use Move because the initial variant value might be part of initial address space
     * (DoNotClear flag set to true if constant declaration). Since we will modify variant content it will lead to an
     * hybrid content (constant and not constant) if we do not make a copy before modification.
     * 1. Move current value to previous value variant
     * 2. Copy previous value content to node value
     * 3. Modify node value with new content
     */

    // 1. Move current value to previous value
    SOPC_Variant_Move(previous_value, node_value);
    SOPC_Variant_Clear(node_value); // Reset DoNotClear flag
    // 2. Make a copy of the variant to modify it and ensure clear (DoNotClear flag ensured to be false)
    SOPC_ReturnStatus status = SOPC_Variant_Copy(node_value, previous_value);
    if (SOPC_STATUS_OK != status)
    {
        // Restore node value
        SOPC_Variant_Move(node_value, previous_value);
        return status;
    }

    // 3. Modify node value with new content
    if (SOPC_VariantArrayType_SingleValue == node_value->ArrayType)
    {
        status = SOPC_LocalizedText_AddOrSetLocale(node_value->Value.LocalizedText, supportedLocales,
                                                   new_value->Value.LocalizedText);
    }
    else if (SOPC_VariantArrayType_Array == node_value->ArrayType)
    {
        assert(node_value->Value.Array.Length == new_value->Value.Array.Length);
        for (int32_t i = 0; SOPC_STATUS_OK == status && i < new_value->Value.Array.Length; i++)
        {
            status = SOPC_LocalizedText_AddOrSetLocale(&node_value->Value.Array.Content.LocalizedTextArr[i],
                                                       supportedLocales,
                                                       &new_value->Value.Array.Content.LocalizedTextArr[i]);
        }
    }
    else if (SOPC_VariantArrayType_Matrix == node_value->ArrayType)
    {
        assert(node_value->Value.Matrix.Dimensions == new_value->Value.Matrix.Dimensions);
        int32_t matrixLength = 1;
        for (int32_t i = 0; i < new_value->Value.Matrix.Dimensions; i++)
        {
            assert(node_value->Value.Matrix.ArrayDimensions[i] == new_value->Value.Matrix.ArrayDimensions[i]);
            matrixLength *= node_value->Value.Matrix.ArrayDimensions[i];
        }
        for (int32_t i = 0; SOPC_STATUS_OK == status && i < matrixLength; i++)
        {
            status = SOPC_LocalizedText_AddOrSetLocale(&node_value->Value.Matrix.Content.LocalizedTextArr[i],
                                                       supportedLocales,
                                                       &new_value->Value.Matrix.Content.LocalizedTextArr[i]);
        }
    }
    else
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Variant_Clear(node_value);
        SOPC_Variant_Move(node_value, previous_value);
        SOPC_Variant_Clear(previous_value);
    }
    return status;
}

static bool is_localized_text_and_modifiable(SOPC_Variant* node_value, const SOPC_Variant* new_value)
{
    bool modifyLocalizedText = false;
    if (SOPC_LocalizedText_Id != node_value->BuiltInTypeId || SOPC_LocalizedText_Id != new_value->BuiltInTypeId)
    {
        // it is not a localized text or it is possible to have different types (new value can be NULL)
        return false;
    }
    if (node_value->ArrayType != new_value->ArrayType)
    {
        // we should overwrite and not modify since it is not same array type
        return false;
    }

    if (SOPC_VariantArrayType_SingleValue == node_value->ArrayType)
    {
        modifyLocalizedText = true;
    }
    else if (SOPC_VariantArrayType_Array == node_value->ArrayType)
    {
        modifyLocalizedText = node_value->Value.Array.Length == new_value->Value.Array.Length;
    }
    else if (SOPC_VariantArrayType_Matrix == node_value->ArrayType)
    {
        if (node_value->Value.Matrix.Dimensions == new_value->Value.Matrix.Dimensions)
        {
            modifyLocalizedText = true;
            for (int32_t i = 0; i < node_value->Value.Matrix.Dimensions; i++)
            {
                if (node_value->Value.Matrix.ArrayDimensions[i] != new_value->Value.Matrix.ArrayDimensions[i])
                {
                    modifyLocalizedText = false;
                }
            }
        }
    }
    else
    {
        return constants_statuscodes_bs__e_sc_bad_write_not_supported;
    }

    return modifyLocalizedText;
}

static constants_statuscodes_bs__t_StatusCode_i set_value_full(char** supportedLocales,
                                                               SOPC_Variant* node_value,
                                                               const SOPC_Variant* new_value,
                                                               SOPC_Variant* previous_value)
{
    bool modifyLocalizedText = is_localized_text_and_modifiable(node_value, new_value);

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    if (modifyLocalizedText)
    {
        status = modify_localized_text(supportedLocales, node_value, new_value, previous_value);
    }
    else
    {
        // Overwrite current value
        SOPC_Variant_Move(previous_value, node_value);
        SOPC_Variant_Clear(node_value);
        SOPC_Variant_Initialize(node_value);
        status = SOPC_Variant_Copy(node_value, new_value);
    }

    switch (status)
    {
    case SOPC_STATUS_OK:
        return constants_statuscodes_bs__e_sc_ok;
    case SOPC_STATUS_NOT_SUPPORTED:
        // Note: should be BadLocaleNotSupported regarding spec 1.03 but does not exist in 1.03 schemas / uactt
        return constants_statuscodes_bs__e_sc_bad_invalid_argument;
    default:
        return constants_statuscodes_bs__e_sc_bad_internal_error;
    }
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
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "set_value_indexed: matrix index range not supported");
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
                                 const constants__t_LocaleIds_i address_space_bs__p_locales,
                                 const constants__t_Node_i address_space_bs__node,
                                 const constants__t_Variant_i address_space_bs__variant,
                                 const t_bool address_space_bs__toConvert,
                                 const constants__t_IndexRange_i address_space_bs__index_range,
                                 constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__serviceStatusCode,
                                 constants__t_DataValue_i* const address_space_bs__prev_dataValue)
{
    SOPC_UNUSED_ARG(
        address_space_bs__p_user); /* Keep for B precondition: user is already authorized for this operation */
    SOPC_AddressSpace_Node* node = address_space_bs__node;
    SOPC_Variant* pvar = SOPC_AddressSpace_Get_Value(address_space_bs__nodes, node);
    SOPC_Variant* convertedValue = NULL;
    const SOPC_Variant* newValue = address_space_bs__variant;

    *address_space_bs__prev_dataValue = SOPC_Malloc(sizeof(SOPC_DataValue));
    SOPC_DataValue_Initialize(*address_space_bs__prev_dataValue);

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
        *address_space_bs__serviceStatusCode = set_value_full(
            address_space_bs__p_locales, pvar, address_space_bs__variant, &(*address_space_bs__prev_dataValue)->Value);
    }
    else
    {
        // Note: set_value_indexed on single value will always fail except on ByteString/String
        // Note2: set_value_indexed does not support partial update of localized text (whole overwrite)
        *address_space_bs__serviceStatusCode =
            set_value_indexed(pvar, address_space_bs__variant, address_space_bs__index_range,
                              &(*address_space_bs__prev_dataValue)->Value);
    }

    if (*address_space_bs__serviceStatusCode == constants_statuscodes_bs__e_sc_ok)
    {
        (*address_space_bs__prev_dataValue)->Status = SOPC_AddressSpace_Get_StatusCode(address_space_bs__nodes, node);
        SOPC_Value_Timestamp ts = SOPC_AddressSpace_Get_SourceTs(address_space_bs__nodes, node);
        (*address_space_bs__prev_dataValue)->SourceTimestamp = ts.timestamp;
        (*address_space_bs__prev_dataValue)->SourcePicoSeconds = ts.picoSeconds;
    }
    else
    {
        SOPC_DataValue_Clear(*address_space_bs__prev_dataValue);
        SOPC_Free(*address_space_bs__prev_dataValue);
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
    SOPC_UNUSED_ARG(
        address_space_bs__p_user); /* Keep for B precondition: user is already authorized for this operation */
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    assert(node->node_class == OpcUa_NodeClass_Variable);
    bool result = true;
    if (address_space_bs__p_ts.timestamp == 0 && address_space_bs__p_ts.picoSeconds == 0)
    {
        // Update source timestamp with current date if no date provided
        SOPC_Value_Timestamp ts;
        ts.timestamp = SOPC_Time_GetCurrentTimeUTC();
        ts.picoSeconds = 0;
        result = SOPC_AddressSpace_Set_SourceTs(address_space_bs__nodes, node, ts);
    }
    else
    {
        result = SOPC_AddressSpace_Set_SourceTs(address_space_bs__nodes, node, address_space_bs__p_ts);
    }

    if (!result)
    {
        static bool warned = false;
        if (!warned)
        {
            char* nodeId = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(address_space_bs__nodes, node));
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "SourceTimestamp write on NodeId=%s failed due to constant metadata in address space. "
                "It should be forbidden by AccessLevel.",
                nodeId);
            SOPC_Free(nodeId);
            warned = true;
        }
    }
}

void address_space_bs__set_Value_StatusCode(const constants__t_user_i address_space_bs__p_user,
                                            const constants__t_Node_i address_space_bs__p_node,
                                            const constants__t_RawStatusCode address_space_bs__p_sc)
{
    SOPC_UNUSED_ARG(
        address_space_bs__p_user); /* Keep for B precondition: user is already authorized for this operation */
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    assert(node->node_class == OpcUa_NodeClass_Variable);
    bool result = SOPC_AddressSpace_Set_StatusCode(address_space_bs__nodes, node, address_space_bs__p_sc);

    if (!result)
    {
        static bool warned = false;
        if (!warned)
        {
            char* nodeId = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(address_space_bs__nodes, node));
            SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                     "StatusCode write on NodeId=%s failed due to constant metadata in address space."
                                     "It should be forbidden by AccessLevel.",
                                     nodeId);
            SOPC_Free(nodeId);
            warned = true;
        }
    }
}

void address_space_bs__get_Value_StatusCode(const constants__t_user_i address_space_bs__p_user,
                                            const constants__t_Node_i address_space_bs__node,
                                            constants__t_RawStatusCode* const address_space_bs__sc)
{
    SOPC_UNUSED_ARG(address_space_bs__p_user); /* User is already authorized for this operation */
    *address_space_bs__sc = SOPC_AddressSpace_Get_StatusCode(address_space_bs__nodes, address_space_bs__node);
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
    SOPC_Free(address_space_bs__data);
}

void address_space_bs__get_AccessLevel(const constants__t_Node_i address_space_bs__p_node,
                                       constants__t_access_level* const address_space_bs__p_access_level)
{
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_access_level = SOPC_AddressSpace_Get_AccessLevel(address_space_bs__nodes, node);
}

void address_space_bs__get_BrowseName(const constants__t_Node_i address_space_bs__p_node,
                                      constants__t_QualifiedName_i* const address_space_bs__p_browse_name)
{
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_browse_name = SOPC_AddressSpace_Get_BrowseName(address_space_bs__nodes, node);
}

void address_space_bs__get_DisplayName(const constants__t_Node_i address_space_bs__p_node,
                                       constants__t_LocalizedText_i* const address_space_bs__p_display_name)
{
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_display_name = SOPC_AddressSpace_Get_DisplayName(address_space_bs__nodes, node);
}

void address_space_bs__get_Executable(const constants__t_Node_i address_space_bs__p_node,
                                      t_bool* const address_space_bs__p_bool)
{
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_bool = SOPC_AddressSpace_Get_Executable(address_space_bs__nodes, node);
}

void address_space_bs__get_NodeClass(const constants__t_Node_i address_space_bs__p_node,
                                     constants__t_NodeClass_i* const address_space_bs__p_node_class)
{
    assert(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;

    bool res = util_NodeClass__C_to_B(node->node_class, address_space_bs__p_node_class);
    if (false == res)
    {
        *address_space_bs__p_node_class = constants__c_NodeClass_indet;
    }
}

void address_space_bs__get_DataType(const constants__t_Node_i address_space_bs__p_node,
                                    constants__t_NodeId_i* const address_space_bs__p_data_type)
{
    assert(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_data_type = SOPC_AddressSpace_Get_DataType(address_space_bs__nodes, node);
}

void address_space_bs__get_ValueRank(const constants__t_Node_i address_space_bs__p_node,
                                     t_entier4* const address_space_bs__p_value_rank)
{
    assert(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    *address_space_bs__p_value_rank = *SOPC_AddressSpace_Get_ValueRank(address_space_bs__nodes, node);
}

static bool is_inputArgument(const OpcUa_VariableNode* node)
{
    if (NULL == node || &OpcUa_VariableNode_EncodeableType != node->encodeableType)
    {
        return false;
    }

    /* Type should be Argument */
    if (!(SOPC_IdentifierType_Numeric == node->DataType.IdentifierType &&
          OpcUaId_Argument == node->DataType.Data.Numeric))
    {
        return false;
    }

    return (strcmp(SOPC_String_GetRawCString(&node->BrowseName.Name),
                   sopc_address_space_bs__InputArguments_BrowseName) == 0);
}

void address_space_bs__get_TypeDefinition(const constants__t_Node_i address_space_bs__p_node,
                                          constants__t_ExpandedNodeId_i* const address_space_bs__p_type_def)
{
    util_addspace__get_TypeDefinition(address_space_bs__p_node, address_space_bs__p_type_def);
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
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, node);
    *address_space_bs__p_ref_index = *n_refs;
}

void address_space_bs__get_RefIndex_Reference(const constants__t_Node_i address_space_bs__p_node,
                                              const t_entier4 address_space_bs__p_ref_index,
                                              constants__t_Reference_i* const address_space_bs__p_ref)
{
    assert(NULL != address_space_bs__p_node);
    assert(address_space_bs__p_ref_index > 0);
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, node);
    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, node);
    assert(address_space_bs__p_ref_index <= *n_refs);

    *address_space_bs__p_ref = &(*refs)[address_space_bs__p_ref_index - 1];
}

void address_space_bs__get_InputArguments(const constants__t_Node_i address_space_bs__p_node,
                                          constants__t_Variant_i* const address_space_bs__p_input_arg)
{
    assert(NULL != address_space_bs__p_node);
    assert(NULL != address_space_bs__p_input_arg);

    constants__t_Variant_i result = NULL;
    bool found;
    SOPC_AddressSpace_Node* targetNode;

    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, address_space_bs__p_node);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, address_space_bs__p_node);
    for (int32_t i = 0; i < *n_refs && NULL == result; ++i)
    { /* stop when input argument is found */
        OpcUa_ReferenceNode* ref = &(*refs)[i];
        if (util_addspace__is_property(ref))
        {
            if (ref->TargetId.ServerIndex == 0 && ref->TargetId.NamespaceUri.Length <= 0)
            { // Shall be on same server and shall use only NodeId
                targetNode = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, &ref->TargetId.NodeId, &found);
                if (found && NULL != targetNode && OpcUa_NodeClass_Variable == targetNode->node_class)
                {
                    if (is_inputArgument(&targetNode->data.variable))
                    {
                        result = SOPC_AddressSpace_Get_Value(address_space_bs__nodes, targetNode);
                    }
                }
            }
        }
    }
    *address_space_bs__p_input_arg = result;
}

void address_space_bs__get_conv_Variant_Type(const constants__t_Variant_i address_space_bs__p_variant,
                                             constants__t_NodeId_i* const address_space_bs__p_type)
{
    assert(NULL != address_space_bs__p_variant);
    assert(NULL != address_space_bs__p_type);
    SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
    *address_space_bs__p_type = (SOPC_NodeId*) SOPC_Variant_Get_DataType(address_space_bs__p_variant);
    SOPC_GCC_DIAGNOSTIC_RESTORE
}

void address_space_bs__get_conv_Variant_ValueRank(const constants__t_Variant_i address_space_bs__p_variant,
                                                  t_entier4* const address_space_bs__p_valueRank)
{
    assert(NULL != address_space_bs__p_variant);
    assert(NULL != address_space_bs__p_valueRank);
    *address_space_bs__p_valueRank = SOPC_Variant_Get_ValueRank(address_space_bs__p_variant);
}
