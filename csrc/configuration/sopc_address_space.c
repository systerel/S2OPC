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
#include <stdlib.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_address_space.h"
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

void SOPC_AddressSpace_Item_Initialize(SOPC_AddressSpace_Item* item, OpcUa_NodeClass node_class)
{
    switch (node_class)
    {
        FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_INITIALIZE_CASE, NULL)
    default:
        assert(false && "Unknown element type");
    }

    item->node_class = node_class;
    if (node_class == OpcUa_NodeClass_Variable)
    {
        if (SOPC_AddressSpace_Item_Get_NodeId(item)->Namespace != OPCUA_NAMESPACE_INDEX)
        {
            item->value_status = OpcUa_BadDataUnavailable;
        }
        /*Note: set an initial timestamp to could return non null timestamps */
        item->value_source_ts.timestamp = SOPC_Time_GetCurrentTimeUTC();
        item->value_source_ts.picoSeconds = 0;
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

ELEMENT_ATTRIBUTE_GETTER(SOPC_NodeId, NodeId)
ELEMENT_ATTRIBUTE_GETTER(SOPC_QualifiedName, BrowseName)
ELEMENT_ATTRIBUTE_GETTER(SOPC_LocalizedText, DisplayName)
ELEMENT_ATTRIBUTE_GETTER(SOPC_LocalizedText, Description)
ELEMENT_ATTRIBUTE_GETTER(int32_t, NoOfReferences)
ELEMENT_ATTRIBUTE_GETTER(OpcUa_ReferenceNode*, References)

SOPC_Variant* SOPC_AddressSpace_Item_Get_Value(SOPC_AddressSpace_Item* item)
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

int32_t SOPC_AddressSpace_Item_Get_ValueRank(SOPC_AddressSpace_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return item->data.variable.ValueRank;
    case OpcUa_NodeClass_VariableType:
        return item->data.variable_type.ValueRank;
    default:
        assert(false && "Current element has no value rank.");
        return -3;
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
    free(item);
}

SOPC_AddressSpace* SOPC_AddressSpace_Create(bool free_items)
{
    return SOPC_NodeId_Dict_Create(false, free_items ? free_description_item : clear_description_item_value);
}

SOPC_ReturnStatus SOPC_AddressSpace_Append(SOPC_AddressSpace* space, SOPC_AddressSpace_Item* item)
{
    SOPC_NodeId* id = SOPC_AddressSpace_Item_Get_NodeId(item);

    if (item->node_class == OpcUa_NodeClass_Variable)
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

    return SOPC_Dict_Insert(space, id, item) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

void SOPC_AddressSpace_Delete(SOPC_AddressSpace* space)
{
    SOPC_Dict_Delete(space);
}
