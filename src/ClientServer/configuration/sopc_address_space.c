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
        OpcUa_##val##Node_Initialize(&node->data.field);     \
        break;

#define ELEMENT_ATTRIBUTE_CLEAR_CASE(val, field, extra) \
    case OpcUa_NodeClass_##val:                         \
        OpcUa_##val##Node_Clear(&node->data.field);     \
        break;

struct _SOPC_AddressSpace
{
    /* Maps NodeId to SOPC_AddressSpace_Node */
    SOPC_Dict* dict_nodes;
    /* Set to true if the NodeId and SOPC_AddressSpace_Node are const */
    bool readOnlyNodes;
    /* Defined only if readOnlyNodes is true:
     * - dict_nodes unused
     * - const_nodes used instead
     * - array of modifiable Variants,
     *   indexes are defined as UInt32 values in SOPC_AddressSpace_Node variants for all Variable nodes.
     *   Note: Values of VariableType nodes are read-only and hence not represented here. */
    uint32_t nb_nodes;
    SOPC_AddressSpace_Node* const_nodes;
    uint32_t nb_variables;
    SOPC_Variant* variables;
};

void SOPC_AddressSpace_Node_Initialize(SOPC_AddressSpace* space,
                                       SOPC_AddressSpace_Node* node,
                                       OpcUa_NodeClass node_class)
{
    assert(space != NULL);

    switch (node_class)
    {
        FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_INITIALIZE_CASE, NULL)
    default:
        assert(false && "Unknown element type");
    }

    node->node_class = node_class;
    OpcUa_NodeClass* nodeClass = SOPC_AddressSpace_Get_NodeClass(space, node);
    *nodeClass = node_class;

    if (OpcUa_NodeClass_Variable == node_class)
    {
        node->value_status = SOPC_GoodGenericStatus;
        /*Note: set an initial timestamp to return non null timestamps */
        node->value_source_ts.timestamp = SOPC_Time_GetCurrentTimeUTC();
        node->value_source_ts.picoSeconds = 0;
        node->data.variable.ValueRank = -1;
        node->data.variable.AccessLevel = 1;
    }
    else if (OpcUa_NodeClass_VariableType == node_class)
    {
        node->value_status = SOPC_GoodGenericStatus;
        node->value_source_ts.timestamp = 0;
        node->value_source_ts.picoSeconds = 0;
        node->data.variable_type.ValueRank = -1;
    }
    else if (OpcUa_NodeClass_Method == node_class)
    {
        node->data.method.Executable = true;
    }
    else
    {
        node->value_status = SOPC_GoodGenericStatus;
        node->value_source_ts.timestamp = 0;
        node->value_source_ts.picoSeconds = 0;
    }
}

#define ELEMENT_ATTRIBUTE_GETTER_CASE(val, field, extra) \
    case OpcUa_NodeClass_##val:                          \
        return &node->data.field.extra;

#define ELEMENT_ATTRIBUTE_GETTER(ret_type, name)                                                   \
    ret_type* SOPC_AddressSpace_Get_##name(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node) \
    {                                                                                              \
        assert(space != NULL);                                                                     \
        assert(node->node_class > 0);                                                              \
        switch (node->node_class)                                                                  \
        {                                                                                          \
            FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_GETTER_CASE, name)                             \
        default:                                                                                   \
            assert(false && "Unknown element type");                                               \
            return NULL;                                                                           \
        }                                                                                          \
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
static SOPC_Variant* SOPC_AddressSpace_Node_Get_Value(SOPC_AddressSpace_Node* node)
{
    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return &node->data.variable.Value;
    case OpcUa_NodeClass_VariableType:
        return &node->data.variable_type.Value;
    default:
        assert(false && "Current element has no value.");
        return NULL;
    }
}

SOPC_Variant* SOPC_AddressSpace_Get_Value(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        if (space->readOnlyNodes)
        {
            /* When the address space is constant, the only variable part is the variant array space->variables defined
             * outside of the node. In this case the node variable variant content data.variable.Value.Value shall be
             * the index in this array and therefore shall be a single value UInt32 */
            assert(SOPC_VariantArrayType_SingleValue == node->data.variable.Value.ArrayType);
            assert(SOPC_UInt32_Id == node->data.variable.Value.BuiltInTypeId);
            return &space->variables[node->data.variable.Value.Value.Uint32];
        }
        else
        {
            return &node->data.variable.Value;
        }
    case OpcUa_NodeClass_VariableType:
        return &node->data.variable_type.Value;
    default:
        assert(false && "Current element has no value.");
        return NULL;
    }
}

SOPC_NodeId* SOPC_AddressSpace_Get_DataType(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return &node->data.variable.DataType;
    case OpcUa_NodeClass_VariableType:
        return &node->data.variable_type.DataType;
    default:
        assert(false && "Current element has no data type.");
        return NULL;
    }
}

int32_t* SOPC_AddressSpace_Get_ValueRank(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return &node->data.variable.ValueRank;
    case OpcUa_NodeClass_VariableType:
        return &node->data.variable_type.ValueRank;
    default:
        assert(false && "Current element has no value rank.");
        return NULL;
    }
}

int32_t SOPC_AddressSpace_Get_NoOfArrayDimensions(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return node->data.variable.NoOfArrayDimensions;
    case OpcUa_NodeClass_VariableType:
        return node->data.variable_type.NoOfArrayDimensions;
    default:
        assert(false && "Current element has no NoOfDimensions.");
        return -1;
    }
}

uint32_t* SOPC_AddressSpace_Get_ArrayDimensions(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return node->data.variable.ArrayDimensions;
    case OpcUa_NodeClass_VariableType:
        return node->data.variable_type.ArrayDimensions;
    default:
        assert(false && "Current element has no ArrayDimensions.");
        return NULL;
    }
}

SOPC_Byte SOPC_AddressSpace_Get_AccessLevel(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return node->data.variable.AccessLevel;
    default:
        assert(false && "Current element has no access level.");
        return 0;
    }
}

SOPC_Boolean SOPC_AddressSpace_Get_Executable(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Method:
        return node->data.method.Executable;
    default:
        assert(false && "Current element has no Executable attribute.");
        return false;
    }
}

SOPC_Boolean* SOPC_AddressSpace_Get_IsAbstract(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_VariableType:
        return &node->data.variable_type.IsAbstract;
    case OpcUa_NodeClass_ObjectType:
        return &node->data.object_type.IsAbstract;
    case OpcUa_NodeClass_ReferenceType:
        return &node->data.reference_type.IsAbstract;
    case OpcUa_NodeClass_DataType:
        return &node->data.data_type.IsAbstract;
    default:
        assert(false && "Current element has no IsAbstract attribute.");
        return NULL;
    }
}

SOPC_StatusCode SOPC_AddressSpace_Get_StatusCode(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    if (!space->readOnlyNodes)
    {
        return node->value_status;
    }
    else
    {
        return SOPC_GoodGenericStatus;
    }
}

bool SOPC_AddressSpace_Set_StatusCode(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node, SOPC_StatusCode status)
{
    assert(space != NULL);

    if (!space->readOnlyNodes)
    {
        node->value_status = status;
        return true;
    }
    return false;
}

SOPC_Value_Timestamp SOPC_AddressSpace_Get_SourceTs(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    if (!space->readOnlyNodes)
    {
        return node->value_source_ts;
    }
    else
    {
        SOPC_Value_Timestamp ts = {SOPC_Time_GetCurrentTimeUTC(), 0};
        return ts;
    }
}

bool SOPC_AddressSpace_Set_SourceTs(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node, SOPC_Value_Timestamp ts)
{
    assert(space != NULL);

    if (!space->readOnlyNodes)
    {
        node->value_source_ts = ts;
        return true;
    }
    return false;
}

static void SOPC_AddressSpace_Node_Clear_Local(SOPC_AddressSpace_Node* node)
{
    assert(node->node_class > 0);

    switch (node->node_class)
    {
        FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_CLEAR_CASE, NULL)
    default:
        assert(false && "Unknown element type");
    }
}

void SOPC_AddressSpace_Node_Clear(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    SOPC_AddressSpace_Node_Clear_Local(node);
}

static void clear_description_node_value(void* data)
{
    SOPC_AddressSpace_Node* node = data;

    if (node->node_class == OpcUa_NodeClass_Variable || node->node_class == OpcUa_NodeClass_VariableType)
    {
        SOPC_Variant_Clear(SOPC_AddressSpace_Node_Get_Value(node));
    }
}

static void free_description_node(void* data)
{
    SOPC_AddressSpace_Node* node = data;
    SOPC_AddressSpace_Node_Clear_Local(node);
    SOPC_Free(node);
}

SOPC_AddressSpace* SOPC_AddressSpace_Create(bool free_nodes)
{
    SOPC_AddressSpace* result = SOPC_Calloc(1, sizeof(SOPC_AddressSpace));
    if (NULL == result)
    {
        return NULL;
    }
    result->readOnlyNodes = false;
    result->dict_nodes =
        SOPC_NodeId_Dict_Create(false, free_nodes ? free_description_node : clear_description_node_value);
    if (NULL == result->dict_nodes)
    {
        SOPC_Free(result);
        return NULL;
    }
    return result;
}

SOPC_AddressSpace* SOPC_AddressSpace_CreateReadOnlyNodes(uint32_t nb_nodes,
                                                         SOPC_AddressSpace_Node* nodes,
                                                         uint32_t nb_variables,
                                                         SOPC_Variant* variables)
{
    SOPC_AddressSpace* result = SOPC_Calloc(1, sizeof(SOPC_AddressSpace));
    if (NULL == result)
    {
        return NULL;
    }
    result->readOnlyNodes = true;

    result->nb_nodes = nb_nodes;
    result->const_nodes = nodes;
    result->nb_variables = nb_variables;
    result->variables = variables;

    return result;
}

bool SOPC_AddressSpace_AreReadOnlyNodes(const SOPC_AddressSpace* space)
{
    assert(space != NULL);

    return space->readOnlyNodes;
}

SOPC_ReturnStatus SOPC_AddressSpace_Append(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    assert(space != NULL);

    if (space->readOnlyNodes)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_NodeId* id = SOPC_AddressSpace_Get_NodeId(space, node);

    if (OpcUa_NodeClass_Variable == node->node_class)
    {
        /*Note: set an initial timestamp to could return non null timestamps */
        if ((node->value_source_ts.timestamp == 0 && node->value_source_ts.picoSeconds == 0))
        {
            if (node->value_source_ts.timestamp == 0 && node->value_source_ts.picoSeconds == 0)
            {
                node->value_source_ts.timestamp = SOPC_Time_GetCurrentTimeUTC();
            }
        }
    }

    return SOPC_Dict_Insert(space->dict_nodes, id, node) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
}

void SOPC_AddressSpace_Delete(SOPC_AddressSpace* space)
{
    if (NULL != space)
    {
        SOPC_Dict_Delete(space->dict_nodes);
        space->dict_nodes = NULL;
        for (uint32_t i = 0; i < space->nb_variables; i++)
        {
            SOPC_Variant_Clear(&space->variables[i]);
        }
        // Do not free variables array which was provided as input
        space->nb_nodes = 0;
        space->const_nodes = NULL;
        space->nb_variables = 0;
        space->variables = NULL;
        SOPC_Free(space);
    }
}

SOPC_AddressSpace_Node* SOPC_AddressSpace_Get_Node(SOPC_AddressSpace* space, const SOPC_NodeId* key, bool* found)
{
    assert(space != NULL);

    if (!space->readOnlyNodes)
    {
        return SOPC_Dict_Get(space->dict_nodes, key, found);
    }
    else
    {
        SOPC_AddressSpace_Node* result = NULL;
        bool lfound = false;
        for (uint32_t i = 0; i < space->nb_nodes && !lfound; i++)
        {
            lfound = SOPC_NodeId_Equal(key, SOPC_AddressSpace_Get_NodeId(space, &space->const_nodes[i]));
            if (lfound)
            {
                result = &space->const_nodes[i];
            }
        }
        *found = lfound;
        return result;
    }
}

void SOPC_AddressSpace_ForEach(SOPC_AddressSpace* space, SOPC_AddressSpace_ForEach_Fct* pFunc, void* user_data)
{
    assert(space != NULL);

    if (!space->readOnlyNodes)
    {
        SOPC_Dict_ForEach(space->dict_nodes, pFunc, user_data);
    }
    else
    {
        for (uint32_t i = 0; i < space->nb_nodes; i++)
        {
            (*pFunc)(SOPC_AddressSpace_Get_NodeId(space, &space->const_nodes[i]), &space->const_nodes[i], user_data);
        }
    }
}
