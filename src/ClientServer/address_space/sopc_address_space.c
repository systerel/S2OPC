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

#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_address_space.h"
#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_date_time.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
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
    bool free_nodes;
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
    /* Array of uint32_t, used to store max numeric id stored in each namespace (used by the fresh nodeId
       generation function). */
    SOPC_Array* array_maxNsNumId;
};

void SOPC_AddressSpace_Node_Initialize(SOPC_AddressSpace* space,
                                       SOPC_AddressSpace_Node* node,
                                       OpcUa_NodeClass node_class)
{
    SOPC_ASSERT(space != NULL);

    switch (node_class)
    {
        FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_INITIALIZE_CASE, NULL)
    default:
        SOPC_ASSERT(false && "Unknown element type");
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

typedef enum
{
    SOPC_InternalNodeClass_Unspecified = 0,
    SOPC_InternalNodeClass_Object = 1,
    SOPC_InternalNodeClass_Variable = 2,
    SOPC_InternalNodeClass_Method = 3,
    SOPC_InternalNodeClass_ObjectType = 4,
    SOPC_InternalNodeClass_VariableType = 5,
    SOPC_InternalNodeClass_ReferenceType = 6,
    SOPC_InternalNodeClass_DataType = 7,
    SOPC_InternalNodeClass_View = 8,
} SOPC_InternalNodeClass;

static const bool NODE_CLASS_TO_ATTRIBS[SOPC_InternalNodeClass_View + 1]
                                       [SOPC_AttributeId_AccessLevelEx - SOPC_AttributeId_IsAbstract + 1] = {
                                           {//  8     9     10     11     12     13     14
                                            // 15    16     17     18     19     20     21     22
                                            // 23    24     25     26     27
                                            // Unspecified class
                                            false, false, false, false, false, false, false,

                                            false, false, false, false, false, false, false, false,

                                            false, false, false, false, false},

                                           {// ObjectNode
                                            false, false, false, false, true,  false, false,

                                            false, false, false, false, false, false, false, false,

                                            false, true,  true,  true,  false},

                                           {// VariableNode
                                            false, false, false, false, false, true, true,

                                            true,  true,  true,  true,  true,  true, false, false,

                                            false, true,  true,  true,  true},

                                           {// MethodNode
                                            false, false, false, false, false, false, false,

                                            false, false, false, false, false, false, true,  true,

                                            false, true,  true,  true,  false},

                                           {// ObjectTypeNode
                                            true,  false, false, false, false, false, false,

                                            false, false, false, false, false, false, false, false,

                                            false, true,  true,  true,  false},

                                           {// VariableTypeNode
                                            true,  false, false, false, false, true,  true,

                                            true,  true,  false, false, false, false, true, true,

                                            false, true,  true,  true,  false},

                                           {// ReferenceTypeNode
                                            true,  true,  true,  false, false, false, false,

                                            false, false, false, false, false, false, false, false,

                                            false, true,  true,  true,  false},

                                           {// DataTypeNode
                                            true,  false, false, false, false, false, false,

                                            false, false, false, false, false, false, false, false,

                                            true,  true,  true,  true,  false},

                                           {// ViewNode
                                            false, false, false, true,  true,  false, false,

                                            false, false, false, false, false, false, false, false,

                                            false, true,  true,  true,  false},
};

bool SOPC_AddressSpace_Has_Attribute(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node, SOPC_AttributeId attribute)
{
    SOPC_ASSERT(NULL != space);
    SOPC_ASSERT(NULL != node);

    if ((int64_t) attribute <= (int64_t) SOPC_AttributeId_Invalid)
    {
        // Invalid attribute
        return false;
    }
    else if (attribute < SOPC_AttributeId_IsAbstract)
    {
        // Common attributes present in all OpcUa_*Node types
        return true;
    }
    else if (attribute > SOPC_AttributeId_UserExecutable)
    {
        // Unexpected attributes
        return false;
    }

    SOPC_InternalNodeClass inc = SOPC_InternalNodeClass_Unspecified;
    switch (node->node_class)
    {
    case OpcUa_NodeClass_Unspecified:
        inc = SOPC_InternalNodeClass_Unspecified;
        break;
    case OpcUa_NodeClass_Object:
        inc = SOPC_InternalNodeClass_Object;
        break;
    case OpcUa_NodeClass_Variable:
        inc = SOPC_InternalNodeClass_Variable;
        break;
    case OpcUa_NodeClass_Method:
        inc = SOPC_InternalNodeClass_Method;
        break;
    case OpcUa_NodeClass_ObjectType:
        inc = SOPC_InternalNodeClass_ObjectType;
        break;
    case OpcUa_NodeClass_VariableType:
        inc = SOPC_InternalNodeClass_VariableType;
        break;
    case OpcUa_NodeClass_ReferenceType:
        inc = SOPC_InternalNodeClass_ReferenceType;
        break;
    case OpcUa_NodeClass_DataType:
        inc = SOPC_InternalNodeClass_DataType;
        break;
    case OpcUa_NodeClass_View:
        inc = SOPC_InternalNodeClass_View;
        break;
    default:
        return false;
    }
    return NODE_CLASS_TO_ATTRIBS[inc][attribute - SOPC_AttributeId_IsAbstract];
}

#define ELEMENT_ATTRIBUTE_GETTER_CASE(val, field, extra) \
    case OpcUa_NodeClass_##val:                          \
        return &node->data.field.extra;

#define ELEMENT_ATTRIBUTE_GETTER(ret_type, name)                                                   \
    ret_type* SOPC_AddressSpace_Get_##name(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node) \
    {                                                                                              \
        SOPC_ASSERT(space != NULL);                                                                \
        SOPC_ASSERT(node != NULL);                                                                 \
        SOPC_ASSERT(node->node_class > 0);                                                         \
        switch (node->node_class)                                                                  \
        {                                                                                          \
            FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_GETTER_CASE, name)                             \
        default:                                                                                   \
            SOPC_ASSERT(false && "Unknown element type");                                          \
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
ELEMENT_ATTRIBUTE_GETTER(int32_t, NoOfRolePermissions)
ELEMENT_ATTRIBUTE_GETTER(OpcUa_RolePermissionType*, RolePermissions)
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
        SOPC_ASSERT(false && "Current element has no value.");
        return NULL;
    }
}

SOPC_Variant* SOPC_AddressSpace_Get_Value(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        if (space->readOnlyNodes)
        {
            /* When the address space is constant, the only variable part is the variant array space->variables defined
             * outside of the node. In this case the node variable variant content data.variable.Value.Value shall be
             * the index in this array and therefore shall be a single value UInt32 */
            SOPC_ASSERT(SOPC_VariantArrayType_SingleValue == node->data.variable.Value.ArrayType);
            SOPC_ASSERT(SOPC_UInt32_Id == node->data.variable.Value.BuiltInTypeId);
            return &space->variables[node->data.variable.Value.Value.Uint32];
        }
        else
        {
            return &node->data.variable.Value;
        }
    case OpcUa_NodeClass_VariableType:
        return &node->data.variable_type.Value;
    default:
        SOPC_ASSERT(false && "Current element has no value.");
        return NULL;
    }
}

SOPC_NodeId* SOPC_AddressSpace_Get_DataType(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return &node->data.variable.DataType;
    case OpcUa_NodeClass_VariableType:
        return &node->data.variable_type.DataType;
    default:
        SOPC_ASSERT(false && "Current element has no data type.");
        return NULL;
    }
}

int32_t* SOPC_AddressSpace_Get_ValueRank(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return &node->data.variable.ValueRank;
    case OpcUa_NodeClass_VariableType:
        return &node->data.variable_type.ValueRank;
    default:
        SOPC_ASSERT(false && "Current element has no value rank.");
        return NULL;
    }
}

int32_t SOPC_AddressSpace_Get_NoOfArrayDimensions(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return node->data.variable.NoOfArrayDimensions;
    case OpcUa_NodeClass_VariableType:
        return node->data.variable_type.NoOfArrayDimensions;
    default:
        SOPC_ASSERT(false && "Current element has no NoOfDimensions.");
        return -1;
    }
}

uint32_t* SOPC_AddressSpace_Get_ArrayDimensions(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return node->data.variable.ArrayDimensions;
    case OpcUa_NodeClass_VariableType:
        return node->data.variable_type.ArrayDimensions;
    default:
        SOPC_ASSERT(false && "Current element has no ArrayDimensions.");
        return NULL;
    }
}

SOPC_ExtensionObject* SOPC_AddressSpace_Get_DataTypeDefinition(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

    if (OpcUa_NodeClass_DataType == node->node_class)
    {
        return &node->data.data_type.DataTypeDefinition;
    }
    else
    {
        SOPC_ASSERT(false && "Current element has no DataTypeDefinition.");
        return NULL;
    }
}

SOPC_Byte SOPC_AddressSpace_Get_AccessLevel(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return node->data.variable.AccessLevel;
    default:
        SOPC_ASSERT(false && "Current element has no access level.");
        return 0;
    }
}

SOPC_Boolean SOPC_AddressSpace_Get_Executable(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Method:
        return node->data.method.Executable;
    default:
        SOPC_ASSERT(false && "Current element has no Executable attribute.");
        return false;
    }
}

SOPC_Boolean SOPC_AddressSpace_Get_Historizing(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Variable:
        return node->data.variable.Historizing;
    default:
        SOPC_ASSERT(false && "Current element has no Historizing attribute.");
        return false;
    }
}

SOPC_Boolean* SOPC_AddressSpace_Get_IsAbstract(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

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
        SOPC_ASSERT(false && "Current element has no IsAbstract attribute.");
        return NULL;
    }
}

SOPC_Byte SOPC_AddressSpace_Get_EventNotifier(const SOPC_AddressSpace* space, const SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

    switch (node->node_class)
    {
    case OpcUa_NodeClass_Object:
        return node->data.object.EventNotifier;
    default:
        SOPC_ASSERT(false && "Current element has no EventNotifier attribute.");
        return 0;
    }
}

SOPC_StatusCode SOPC_AddressSpace_Get_StatusCode(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

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
    SOPC_ASSERT(space != NULL);

    if (!space->readOnlyNodes)
    {
        node->value_status = status;
        return true;
    }
    return false;
}

SOPC_Value_Timestamp SOPC_AddressSpace_Get_SourceTs(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

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
    SOPC_ASSERT(space != NULL);

    if (!space->readOnlyNodes)
    {
        node->value_source_ts = ts;
        return true;
    }
    return false;
}

static void SOPC_AddressSpace_Node_Clear_Local(SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(NULL != node);
    SOPC_ASSERT(node->node_class > 0);

    switch (node->node_class)
    {
        FOR_EACH_ELEMENT_TYPE(ELEMENT_ATTRIBUTE_CLEAR_CASE, NULL)
    default:
        SOPC_ASSERT(false && "Unknown element type");
    }
}

void SOPC_AddressSpace_Node_Clear(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);
    SOPC_AddressSpace_Node_Clear_Local(node);
}

void SOPC_AddressSpace_Node_Delete(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);
    SOPC_NodeId* nid = SOPC_AddressSpace_Get_NodeId(space, node);
    SOPC_Dict_Remove(space->dict_nodes, (uintptr_t) nid);
}

SOPC_AddressSpace_Node* SOPC_AddressSpace_Node_Copy(const SOPC_AddressSpace_Node* src)
{
    SOPC_AddressSpace_Node* result = SOPC_Calloc(1, sizeof(*result));
    if (NULL == result)
    {
        return NULL;
    }
    result->node_class = src->node_class;
    result->value_status = src->value_status;
    result->value_source_ts = src->value_source_ts;
    SOPC_EncodeableType* nodeEncType = (*(SOPC_EncodeableType* const*) &src->data);
    nodeEncType->Initialize(&result->data);
    SOPC_ReturnStatus status = SOPC_EncodeableObject_Copy(nodeEncType, &result->data, &src->data);
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Free(result);
        result = NULL;
    }
    return result;
}

static void clear_description_node_value(uintptr_t data)
{
    SOPC_AddressSpace_Node* node = (SOPC_AddressSpace_Node*) data;

    if (node->node_class == OpcUa_NodeClass_Variable || node->node_class == OpcUa_NodeClass_VariableType)
    {
        SOPC_Variant_Clear(SOPC_AddressSpace_Node_Get_Value(node));
    }
}

static void free_description_node(uintptr_t data)
{
    SOPC_AddressSpace_Node* node = (SOPC_AddressSpace_Node*) data;
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
    result->free_nodes = free_nodes;
    result->dict_nodes =
        SOPC_NodeId_Dict_Create(false, free_nodes ? &free_description_node : &clear_description_node_value);
    if (NULL == result->dict_nodes)
    {
        SOPC_Free(result);
        return NULL;
    }
    SOPC_Dict_SetTombstoneKey(result->dict_nodes, UINTPTR_MAX);
    return result;
}

bool SOPC_AddressSpace_AreNodesReleasable(const SOPC_AddressSpace* space)
{
    SOPC_ASSERT(space != NULL);

    return space->free_nodes;
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
    result->free_nodes = false;

    result->nb_nodes = nb_nodes;
    result->const_nodes = nodes;
    result->nb_variables = nb_variables;
    result->variables = variables;

    return result;
}

bool SOPC_AddressSpace_AreReadOnlyNodes(const SOPC_AddressSpace* space)
{
    SOPC_ASSERT(space != NULL);
    return space->readOnlyNodes;
}

SOPC_ReturnStatus SOPC_AddressSpace_Append(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(space != NULL);

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

    return SOPC_Dict_Insert(space->dict_nodes, (uintptr_t) id, (uintptr_t) node) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
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
        SOPC_Array_Delete(space->array_maxNsNumId);
        space->array_maxNsNumId = NULL;
        SOPC_Free(space);
    }
}

SOPC_AddressSpace_Node* SOPC_AddressSpace_Get_Node(SOPC_AddressSpace* space, const SOPC_NodeId* key, bool* found)
{
    SOPC_ASSERT(space != NULL);

    if (!space->readOnlyNodes)
    {
        return (SOPC_AddressSpace_Node*) SOPC_Dict_Get(space->dict_nodes, (uintptr_t) key, found);
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

typedef struct
{
    SOPC_AddressSpace_ForEach_Fct* pFunc;
    uintptr_t user_data;
} AddressSpace_Dict_Context;

/**
 * \brief This function is used to convert Dict_ForEach calls which have void* value to
 *        AddressSpace_ForEach calls which have const void* value.
 * \param key       The dictionary key
 * \param value     The dictionary value
 * \param user_data The AddressSpace_Dict_Context context needed to apply on this \p key / \p value pair.
 */
static void addressSpace_ForEach_Convert(const uintptr_t key, uintptr_t value, uintptr_t user_data)
{
    AddressSpace_Dict_Context* context = (AddressSpace_Dict_Context*) user_data;
    SOPC_ASSERT(NULL != (void*) user_data && NULL != context->pFunc);
    context->pFunc(key, value, context->user_data);
}

void SOPC_AddressSpace_ForEach(SOPC_AddressSpace* space, SOPC_AddressSpace_ForEach_Fct* pFunc, uintptr_t user_data)
{
    SOPC_ASSERT(NULL != space && NULL != pFunc);

    if (!space->readOnlyNodes)
    {
        AddressSpace_Dict_Context context = {.pFunc = pFunc, .user_data = user_data};
        SOPC_Dict_ForEach(space->dict_nodes, addressSpace_ForEach_Convert, (uintptr_t) &context);
    }
    else
    {
        for (uint32_t i = 0; i < space->nb_nodes; i++)
        {
            (*pFunc)((uintptr_t) SOPC_AddressSpace_Get_NodeId(space, &space->const_nodes[i]),
                     (uintptr_t) &space->const_nodes[i], user_data);
        }
    }
}

typedef struct
{
    // Array of uint32_t
    SOPC_Array* array_nsMaxIdArray;
    SOPC_ReturnStatus status;
} AddressSpace_NsInitialize_Context;

/**
 * \brief Compare the numeric ID (if any) of incoming node: \p key with the maxId: \p user_data
 *        for the same NS. If greater, then update maxId.
 *        If the NS of a nodeId in AddSpace is greater than the number of NS (length of the array),
 *        nsInitCtx->status is set to SOPC_STATUS_INVALID_STATE.
 *
 * \param key       The incoming node Id ::SOPC_NodeId*
 * \param value     Unused
 * \param user_data ctx with dict_nsMaxIdArray and a status
 */
static void CompareNodeNumIdWithMaxId_ForEachNS(const uintptr_t key, const uintptr_t value, uintptr_t user_data)
{
    SOPC_UNUSED_ARG(value);
    AddressSpace_NsInitialize_Context* nsInitCtx = (AddressSpace_NsInitialize_Context*) user_data;
    const SOPC_NodeId* nodeId = (SOPC_NodeId*) key;
    if (nodeId->Namespace < (uint16_t) SOPC_Array_Size(nsInitCtx->array_nsMaxIdArray))
    {
        uint32_t* pCurrMax = (uint32_t*) SOPC_Array_Get_Ptr(nsInitCtx->array_nsMaxIdArray, nodeId->Namespace);
        if (SOPC_IdentifierType_Numeric == nodeId->IdentifierType && NULL != pCurrMax &&
            nodeId->Data.Numeric > *pCurrMax)
        {
            *pCurrMax = nodeId->Data.Numeric;
        }
    }
    else
    {
        // Inconsistency in the number of NS
        nsInitCtx->status = SOPC_STATUS_INVALID_STATE;
    }
}

/**
 * \brief Run though all AddSpace to find the max numeric ID
 *        of each namespace.
 *
 * \param addSpace          The addressSpace.
 * \param dict_nsMaxIdArray Dict of max numeric id for each NS [0-nbNs].
 *
 * \return SOPC_STATUS_OK in case of success,
 *         SOPC_STATUS_INVALID_STATE if NS index of a nodeId in AddSpace is greater than the number of NS (nbNs).

 */
static SOPC_ReturnStatus FindInternalMaxIdForEachNS(SOPC_AddressSpace* addSpace, SOPC_Array* array_nsMaxNsId)
{
    AddressSpace_NsInitialize_Context nsInitCtx = {.array_nsMaxIdArray = array_nsMaxNsId, .status = SOPC_STATUS_OK};
    SOPC_AddressSpace_ForEach(addSpace, CompareNodeNumIdWithMaxId_ForEachNS, (uintptr_t) &nsInitCtx);
    return nsInitCtx.status;
}

SOPC_ReturnStatus SOPC_AddressSpace_MaxNsNumId_Initialize(SOPC_AddressSpace* addSpace, uint16_t nbNs)
{
    SOPC_Array* array_nsMaxNsId = SOPC_Array_Create(sizeof(uint32_t), nbNs, NULL);
    SOPC_ReturnStatus status = (NULL != array_nsMaxNsId) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Array_Append_Values(array_nsMaxNsId, NULL, nbNs) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = FindInternalMaxIdForEachNS(addSpace, array_nsMaxNsId);
    }
    if (SOPC_STATUS_OK == status)
    {
        addSpace->array_maxNsNumId = array_nsMaxNsId;
    }
    else
    {
        SOPC_Array_Delete(array_nsMaxNsId);
        array_nsMaxNsId = NULL;
    }
    return status;
}

static uint32_t* Get_MaxNumIdForNs(const SOPC_AddressSpace* addSpace, uint16_t ns)
{
    SOPC_ASSERT(NULL != addSpace && NULL != addSpace->array_maxNsNumId);
    return (uint32_t*) SOPC_Array_Get_Ptr(addSpace->array_maxNsNumId, ns);
}

SOPC_NodeId* SOPC_AddressSpace_GetFreshNodeId(SOPC_AddressSpace* addSpace, uint16_t ns)
{
    SOPC_ASSERT(addSpace != NULL);
    // Check that ns exist in the array of ns index
    size_t nbNs = SOPC_Array_Size(addSpace->array_maxNsNumId);
    if (ns > nbNs)
    {
        // Grow array size to NS required index, initialized to 0 max numeric id
        bool res = SOPC_Array_Append_Values(addSpace->array_maxNsNumId, NULL, ns);
        if (!res)
        {
            return NULL;
        }
    }
    uint32_t* maxNsNumId = Get_MaxNumIdForNs(addSpace, ns);
    SOPC_ASSERT(NULL != maxNsNumId); // maxNsNumId is not supposed to be NULL here since we checked that ns <= nbNs
    if (*maxNsNumId == 0)            // Avoid 0 id which might be considered as invalid
    {
        // First fresh id start from 1
        *maxNsNumId = 1;
    }
    uint32_t finalMaxId = *maxNsNumId + SOPC_FRESH_NODEID_MAX_RETRIES;
    bool nodeIdAlreadyExists = true;
    SOPC_NodeId* newNodeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
    SOPC_NodeId_Initialize(newNodeId);
    newNodeId->Namespace = ns;
    newNodeId->IdentifierType = SOPC_IdentifierType_Numeric;

    while ((*maxNsNumId < finalMaxId) && nodeIdAlreadyExists)
    {
        newNodeId->Data.Numeric = *maxNsNumId;
        const SOPC_AddressSpace_Node* maybeNode = SOPC_AddressSpace_Get_Node(addSpace, newNodeId, &nodeIdAlreadyExists);
        SOPC_UNUSED_RESULT(maybeNode);
        (*maxNsNumId)++;
    }
    // If no new fresh nodeId found
    if (nodeIdAlreadyExists)
    {
        SOPC_NodeId_Clear(newNodeId);
        SOPC_Free(newNodeId);
        newNodeId = NULL;
    }
    return newNodeId;
}
