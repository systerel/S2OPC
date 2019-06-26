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

#include <assert.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_address_space.h"
#include "sopc_dict.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"
#include "sopc_types.h"

#define ELEMENT_ATTRIBUTE_INITIALIZE_CASE(val, field, extra) \
    case OpcUa_NodeClass_##val:                              \
        OpcUa_##val##Node_Initialize(&item->data.field);     \
        break;

#define ELEMENT_ATTRIBUTE_CLEAR_CASE(val, field, extra) \
    case OpcUa_NodeClass_##val:                         \
        OpcUa_##val##Node_Clear(&item->data.field);     \
        break;

struct _SOPC_AddressSpace
{
    /* Maps NodeId to SOPC_AddressSpace_Item */
    SOPC_Dict* dict_items;
    /* Set to true if the NodeId and SOPC_AddressSpace_Item are const */
    bool readOnlyItems;
    /* Defined only if readOnlyNodes is true:
     * - dict_items unused
     * - const_items used instead
     * - array of modifiable Variants,
     *   indexes are defined as UInt32 values in SOPC_AddressSpace_Item variants for all Variable nodes.
     *   Note: Values of VariableType nodes are read-only and hence not represented here. */
    uint32_t nb_items;
    SOPC_AddressSpace_Item* const_items;
    uint32_t nb_variables;
    SOPC_Variant* variables;
};

void SOPC_AddressSpace_Item_Initialize(SOPC_AddressSpace_Item* item, OpcUa_NodeClass node_class)
{
    switch (node_class)
    {
        FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_INITIALIZE_CASE, NULL)
    default:
        assert(false && "Unknown element type");
    }

    item->node_class = node_class;
    OpcUa_NodeClass* nodeClass = SOPC_AddressSpace_Item_Get_NodeClass(item);
    *nodeClass = node_class;

    if (node_class == OpcUa_NodeClass_Variable)
    {
        item->value_status = SOPC_GoodGenericStatus;
        /*Note: set an initial timestamp to return non null timestamps */
        item->value_source_ts.timestamp = SOPC_Time_GetCurrentTimeUTC();
        item->value_source_ts.picoSeconds = 0;
        item->data.variable.ValueRank = -1;
        item->data.variable.AccessLevel = 1;
    }
    else if (node_class == OpcUa_NodeClass_VariableType)
    {
        item->value_status = SOPC_GoodGenericStatus;
        item->value_source_ts.timestamp = 0;
        item->value_source_ts.picoSeconds = 0;
        item->data.variable_type.ValueRank = -1;
    }
    else
    {
        item->value_status = SOPC_GoodGenericStatus;
        item->value_source_ts.timestamp = 0;
        item->value_source_ts.picoSeconds = 0;
    }
}

#define ELEMENT_ATTRIBUTE_GETTER_CASE(val, field, extra) \
    case OpcUa_NodeClass_##val:                          \
        return &item->data.field.extra;

#define ELEMENT_ATTRIBUTE_GETTER(ret_type, name)                              \
    ret_type* SOPC_AddressSpace_Item_Get_##name(SOPC_AddressSpace_Item* item) \
    {                                                                         \
        assert(item->node_class > 0);                                         \
        switch (item->node_class)                                             \
        {                                                                     \
            FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_GETTER_CASE, name)        \
        default:                                                              \
            assert(false && "Unknown element type");                          \
            return NULL;                                                      \
        }                                                                     \
    }

ELEMENT_ATTRIBUTE_GETTER(OpcUa_NodeClass, NodeClass)
ELEMENT_ATTRIBUTE_GETTER(SOPC_NodeId, NodeId)
ELEMENT_ATTRIBUTE_GETTER(SOPC_QualifiedName, BrowseName)
ELEMENT_ATTRIBUTE_GETTER(SOPC_LocalizedText, DisplayName)
ELEMENT_ATTRIBUTE_GETTER(SOPC_LocalizedText, Description)
ELEMENT_ATTRIBUTE_GETTER(int32_t, NoOfReferences)
ELEMENT_ATTRIBUTE_GETTER(OpcUa_ReferenceNode*, References)
ELEMENT_ATTRIBUTE_GETTER(uint32_t, WriteMask)
ELEMENT_ATTRIBUTE_GETTER(uint32_t, UserWriteMask)

// Important note: only for internal use for non const address space
static SOPC_Variant* SOPC_AddressSpace_Item_Get_Value(SOPC_AddressSpace_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return &item->data.variable.Value;
    case OpcUa_NodeClass_VariableType:
        return &item->data.variable_type.Value;
    default:
        assert(false && "Current element has no value.");
        return NULL;
    }
}

SOPC_Variant* SOPC_AddressSpace_Get_Value(SOPC_AddressSpace* space, SOPC_AddressSpace_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_Variable:
        if (space->readOnlyItems)
        {
            assert(SOPC_VariantArrayType_SingleValue == item->data.variable.Value.ArrayType);
            assert(SOPC_UInt32_Id == item->data.variable.Value.BuiltInTypeId);
            return &space->variables[item->data.variable.Value.Value.Uint32];
        }
        else
        {
            return &item->data.variable.Value;
        }
    case OpcUa_NodeClass_VariableType:
        return &item->data.variable_type.Value;
    default:
        assert(false && "Current element has no value.");
        return NULL;
    }
}

SOPC_NodeId* SOPC_AddressSpace_Item_Get_DataType(SOPC_AddressSpace_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return &item->data.variable.DataType;
    case OpcUa_NodeClass_VariableType:
        return &item->data.variable_type.DataType;
    default:
        assert(false && "Current element has no data type.");
        return NULL;
    }
}

int32_t* SOPC_AddressSpace_Item_Get_ValueRank(SOPC_AddressSpace_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return &item->data.variable.ValueRank;
    case OpcUa_NodeClass_VariableType:
        return &item->data.variable_type.ValueRank;
    default:
        assert(false && "Current element has no value rank.");
        return NULL;
    }
}

int32_t SOPC_AddressSpace_Item_Get_NoOfArrayDimensions(SOPC_AddressSpace_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return item->data.variable.NoOfArrayDimensions;
    case OpcUa_NodeClass_VariableType:
        return item->data.variable_type.NoOfArrayDimensions;
    default:
        assert(false && "Current element has no NoOfDimensions.");
        return -1;
    }
}

uint32_t* SOPC_AddressSpace_Item_Get_ArrayDimensions(SOPC_AddressSpace_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return item->data.variable.ArrayDimensions;
    case OpcUa_NodeClass_VariableType:
        return item->data.variable_type.ArrayDimensions;
    default:
        assert(false && "Current element has no ArrayDimensions.");
        return NULL;
    }
}

SOPC_Byte SOPC_AddressSpace_Item_Get_AccessLevel(SOPC_AddressSpace_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return item->data.variable.AccessLevel;
    default:
        assert(false && "Current element has no access level.");
        return 0;
    }
}

SOPC_Boolean* SOPC_AddressSpace_Item_Get_IsAbstract(SOPC_AddressSpace_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_VariableType:
        return &item->data.variable_type.IsAbstract;
    case OpcUa_NodeClass_ObjectType:
        return &item->data.object_type.IsAbstract;
    case OpcUa_NodeClass_ReferenceType:
        return &item->data.reference_type.IsAbstract;
    case OpcUa_NodeClass_DataType:
        return &item->data.data_type.IsAbstract;
    default:
        assert(false && "Current element has no IsAbstract attribute.");
        return NULL;
    }
}

SOPC_StatusCode SOPC_AddressSpace_Get_StatusCode(SOPC_AddressSpace* space, SOPC_AddressSpace_Item* item)
{
    if (!space->readOnlyItems)
    {
        return item->value_status;
    }
    else
    {
        return SOPC_GoodGenericStatus;
    }
}

void SOPC_AddressSpace_Set_StatusCode(SOPC_AddressSpace* space, SOPC_AddressSpace_Item* item, SOPC_StatusCode status)
{
    if (!space->readOnlyItems)
    {
        item->value_status = status;
    }
}

SOPC_Value_Timestamp SOPC_AddressSpace_Get_SourceTs(SOPC_AddressSpace* space, SOPC_AddressSpace_Item* item)
{
    if (!space->readOnlyItems)
    {
        return item->value_source_ts;
    }
    else
    {
        SOPC_Value_Timestamp ts = {SOPC_Time_GetCurrentTimeUTC(), 0};
        return ts;
    }
}

void SOPC_AddressSpace_Set_SourceTs(SOPC_AddressSpace* space, SOPC_AddressSpace_Item* item, SOPC_Value_Timestamp ts)
{
    if (!space->readOnlyItems)
    {
        item->value_source_ts = ts;
    }
}

void SOPC_AddressSpace_Item_Clear(SOPC_AddressSpace_Item* item)
{
    assert(item->node_class > 0);

    switch (item->node_class)
    {
        FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_CLEAR_CASE, NULL)
    default:
        assert(false && "Unknown element type");
    }
}

static void clear_description_item_value(void* data)
{
    SOPC_AddressSpace_Item* item = data;

    if (item->node_class == OpcUa_NodeClass_Variable || item->node_class == OpcUa_NodeClass_VariableType)
    {
        SOPC_Variant_Clear(SOPC_AddressSpace_Item_Get_Value(item));
    }
}

static void free_description_item(void* data)
{
    SOPC_AddressSpace_Item* item = data;
    SOPC_AddressSpace_Item_Clear(item);
    SOPC_Free(item);
}

SOPC_AddressSpace* SOPC_AddressSpace_Create(bool free_items)
{
    SOPC_AddressSpace* result = SOPC_Calloc(1, sizeof(SOPC_AddressSpace));
    if (NULL == result)
    {
        return NULL;
    }
    result->readOnlyItems = false;
    result->dict_items =
        SOPC_NodeId_Dict_Create(false, free_items ? free_description_item : clear_description_item_value);
    if (NULL == result->dict_items)
    {
        free(result);
        return NULL;
    }
    return result;
}

SOPC_AddressSpace* SOPC_AddressSpace_CreateReadOnlyItems(uint32_t nb_items,
                                                         SOPC_AddressSpace_Item* items,
                                                         uint32_t nb_variables,
                                                         SOPC_Variant* variables)
{
    SOPC_AddressSpace* result = SOPC_Calloc(1, sizeof(SOPC_AddressSpace));
    if (NULL == result)
    {
        return NULL;
    }
    result->readOnlyItems = true;

    result->nb_items = nb_items;
    result->const_items = items;
    result->nb_variables = nb_variables;
    result->variables = variables;

    return result;
}

bool SOPC_AddressSpace_AreReadOnlyItems(const SOPC_AddressSpace* space)
{
    return space->readOnlyItems;
}

SOPC_ReturnStatus SOPC_AddressSpace_Append(SOPC_AddressSpace* space, SOPC_AddressSpace_Item* item)
{
    if (space->readOnlyItems)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_NodeId* id = SOPC_AddressSpace_Item_Get_NodeId(item);

    if (OpcUa_NodeClass_Variable == item->node_class)
    {
        /*Note: set an initial timestamp to could return non null timestamps */
        if ((item->value_source_ts.timestamp == 0 && item->value_source_ts.picoSeconds == 0))
        {
            if (item->value_source_ts.timestamp == 0 && item->value_source_ts.picoSeconds == 0)
            {
                item->value_source_ts.timestamp = SOPC_Time_GetCurrentTimeUTC();
            }
        }
    }

    return SOPC_Dict_Insert(space->dict_items, id, item) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

void SOPC_AddressSpace_Delete(SOPC_AddressSpace* space)
{
    SOPC_Dict_Delete(space->dict_items);
    space->dict_items = NULL;
    for (uint32_t i = 0; i < space->nb_variables; i++)
    {
        SOPC_Variant_Clear(&space->variables[i]);
    }
    // Do not free variables array which was provided as input
    space->nb_items = 0;
    space->const_items = NULL;
    space->nb_variables = 0;
    space->variables = NULL;
    free(space);
}

SOPC_AddressSpace_Item* SOPC_AddressSpace_Get_Item(const SOPC_AddressSpace* space, const SOPC_NodeId* key, bool* found)
{
    if (!space->readOnlyItems)
    {
        return SOPC_Dict_Get(space->dict_items, key, found);
    }
    else
    {
        SOPC_AddressSpace_Item* result = NULL;
        bool lfound = false;
        for (uint32_t i = 0; i < space->nb_items && !lfound; i++)
        {
            lfound = SOPC_NodeId_Equal(key, SOPC_AddressSpace_Item_Get_NodeId(&space->const_items[i]));
            if (lfound)
            {
                result = &space->const_items[i];
            }
        }
        *found = lfound;
        return result;
    }
}

void SOPC_AddressSpace_ForEach(SOPC_AddressSpace* space, SOPC_AddressSpace_ForEach_Fct func, void* user_data)
{
    if (!space->readOnlyItems)
    {
        SOPC_Dict_ForEach(space->dict_items, func, user_data);
    }
    else
    {
        for (uint32_t i = 0; i < space->nb_items; i++)
        {
            func(SOPC_AddressSpace_Item_Get_NodeId(&space->const_items[i]), &space->const_items[i], user_data);
        }
    }
}
