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

#ifndef SOPC_ADDRESS_SPACE_H_
#define SOPC_ADDRESS_SPACE_H_

#include <stdint.h>

#include "sopc_types.h"

#define FOR_EACH_ELEMENT_TYPE(x, extra)                                                                               \
    x(DataType, data_type, extra) x(Method, method, extra) x(Object, object, extra) x(ObjectType, object_type, extra) \
        x(ReferenceType, reference_type, extra) x(Variable, variable, extra) x(VariableType, variable_type, extra)    \
            x(View, view, extra)

#define ELEMENT_ATTRIBUTE_GETTER_DECL(ret_type, lowercase_name, camel_case_name) \
    ret_type* current_element_##lowercase_name(SOPC_AddressSpace_Node* node);

typedef struct SOPC_Value_Timestamp
{
    SOPC_DateTime timestamp;
    uint16_t picoSeconds;
} SOPC_Value_Timestamp;

typedef struct
{
    OpcUa_NodeClass node_class;
    SOPC_StatusCode value_status;
    SOPC_Value_Timestamp value_source_ts;
    union {
        OpcUa_DataTypeNode data_type;
        OpcUa_MethodNode method;
        OpcUa_ObjectNode object;
        OpcUa_ObjectTypeNode object_type;
        OpcUa_ReferenceTypeNode reference_type;
        OpcUa_VariableNode
            variable; // Note: variable.Value shall never be accessed directly (it is an index in const @space)
        OpcUa_VariableTypeNode variable_type;
        OpcUa_ViewNode view;
    } data;
} SOPC_AddressSpace_Node;

/* Address space structure */
typedef struct _SOPC_AddressSpace SOPC_AddressSpace;

SOPC_AddressSpace* SOPC_AddressSpace_Create(bool free_nodes);
SOPC_AddressSpace* SOPC_AddressSpace_CreateReadOnlyNodes(uint32_t nb_nodes,
                                                         SOPC_AddressSpace_Node* nodes,
                                                         uint32_t nb_variables,
                                                         SOPC_Variant* variables);
bool SOPC_AddressSpace_AreReadOnlyNodes(const SOPC_AddressSpace* space);
SOPC_ReturnStatus SOPC_AddressSpace_Append(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
void SOPC_AddressSpace_Delete(SOPC_AddressSpace* space);

SOPC_AddressSpace_Node* SOPC_AddressSpace_Get_Node(SOPC_AddressSpace* space, const SOPC_NodeId* key, bool* found);

/**
 * \brief Type of callback functions for \ref SOPC_AddressSpace_ForEach. Both the key and
 * value belong to the address space and shall not be modified. The value of
 * \p user_data is set when calling \ref SOPC_AddressSpace_ForEach.
 * Note: \p key actual type is const SOPC_NodeId* but shall remain void in declaration for generic data structure usage
 * Note: \p value actual type is const SOPC_AddressSpace_Node* but shall remain void in declaration for generic data
 * structure usage
 */
typedef void SOPC_AddressSpace_ForEach_Fct(const void* key, const void* value, void* user_data);

void SOPC_AddressSpace_ForEach(SOPC_AddressSpace* space, SOPC_AddressSpace_ForEach_Fct* pFunc, void* user_data);

/* Common attributes */
OpcUa_NodeClass* SOPC_AddressSpace_Get_NodeClass(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
SOPC_NodeId* SOPC_AddressSpace_Get_NodeId(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
SOPC_QualifiedName* SOPC_AddressSpace_Get_BrowseName(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
SOPC_LocalizedText* SOPC_AddressSpace_Get_DisplayName(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
SOPC_LocalizedText* SOPC_AddressSpace_Get_Description(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
uint32_t* SOPC_AddressSpace_Get_WriteMask(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
uint32_t* SOPC_AddressSpace_Get_UserWriteMask(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
int32_t* SOPC_AddressSpace_Get_NoOfReferences(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
OpcUa_ReferenceNode** SOPC_AddressSpace_Get_References(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
/* Variable and VariableType common attributes */
SOPC_Variant* SOPC_AddressSpace_Get_Value(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);

SOPC_Byte SOPC_AddressSpace_Get_AccessLevel(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
SOPC_Boolean SOPC_AddressSpace_Get_Executable(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
SOPC_NodeId* SOPC_AddressSpace_Get_DataType(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
int32_t* SOPC_AddressSpace_Get_ValueRank(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
int32_t SOPC_AddressSpace_Get_NoOfArrayDimensions(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
uint32_t* SOPC_AddressSpace_Get_ArrayDimensions(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
/* Types common attribute */
SOPC_Boolean* SOPC_AddressSpace_Get_IsAbstract(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);

SOPC_StatusCode SOPC_AddressSpace_Get_StatusCode(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
bool SOPC_AddressSpace_Set_StatusCode(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node, SOPC_StatusCode status);

SOPC_Value_Timestamp SOPC_AddressSpace_Get_SourceTs(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);
bool SOPC_AddressSpace_Set_SourceTs(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node, SOPC_Value_Timestamp ts);

/* Address space node structure */
void SOPC_AddressSpace_Node_Initialize(SOPC_AddressSpace* space,
                                       SOPC_AddressSpace_Node* node,
                                       OpcUa_NodeClass element_type);
void SOPC_AddressSpace_Node_Clear(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);

#endif /* SOPC_ADDRESS_SPACE_H_ */
