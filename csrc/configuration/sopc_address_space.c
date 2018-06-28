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
#include "sopc_dict.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

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
    item->value_status = OpcUa_BadDataUnavailable;
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

SOPC_Byte* SOPC_AddressSpace_Item_Get_AccessLevel(SOPC_AddressSpace_Item* item)
{
    switch (item->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return &item->data.variable.AccessLevel;
    default:
        assert(false && "Current element has no access level.");
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

static uint64_t nodeid_hash(const void* id)
{
    uint64_t hash;
    SOPC_NodeId_Hash((const SOPC_NodeId*) id, &hash);
    return hash;
}

static bool nodeid_equal(const void* a, const void* b)
{
    int32_t cmp;
    SOPC_NodeId_Compare((const SOPC_NodeId*) a, (const SOPC_NodeId*) b, &cmp);

    return cmp == 0;
}

SOPC_AddressSpace* SOPC_AddressSpace_Create(bool free_items)
{
    return SOPC_Dict_Create(NULL, nodeid_hash, nodeid_equal, NULL,
                            free_items ? free_description_item : clear_description_item_value);
}

SOPC_ReturnStatus SOPC_AddressSpace_Append(SOPC_AddressSpace* space, SOPC_AddressSpace_Item* item)
{
    SOPC_NodeId* id = SOPC_AddressSpace_Item_Get_NodeId(item);
    return SOPC_Dict_Insert(space, id, item) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

void SOPC_AddressSpace_Delete(SOPC_AddressSpace* space)
{
    SOPC_Dict_Delete(space);
}
