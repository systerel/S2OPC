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

/**
 * \brief Create an empty AddressSpace to fill with nodes with ::SOPC_AddressSpace_Append
 *
 * \warning The NodeManagement services are incompatible with an AddressSpace created with parameter free_nodes = false
 *
 * \param free_nodes indicates if the nodes shall be freed on call to ::SOPC_AddressSpace_Delete.
 *                   In order to have NodeManagement services enabled, this parameter shall be set to true.
 *
 * \return the created AddressSpace to fill with nodes
 */
SOPC_AddressSpace* SOPC_AddressSpace_Create(bool free_nodes);

/**
 * \brief Returns true if the AddressSpace will release nodes on ::SOPC_AddressSpace_Delete.
 *        It meands it has been created using ::SOPC_AddressSpace_Create with (free_nodes=true)
 *
 * \param space  the AddressSpace to be evaluated
 *
 * \return       true if the AddressSpace nodes will be freed on call to ::SOPC_AddressSpace_Delete, false otherwise
 */
bool SOPC_AddressSpace_AreNodesReleasable(const SOPC_AddressSpace* space);

/**
 * \brief Create an AddressSpace filled with given nodes and variants for variables values.
 *        The nodes are read only and cannot be modified and only the given variables values
 *        can be changed after AddresSpace creation.
 *        The Variable nodes values shall contain the index in the variants array as an UInt32 single value.
 *
 * \note  This functionality allows to have a constant nodes array that can be stored in ROM
 *        on device with limited memory resources.
 *
 * \warning The NodeManagement services are incompatible with an AddressSpace created with this function
 *
 * \param nb_nodes      the number of nodes in the nodes array
 * \param nodes         the array of nodes contained by this constant AddressSpace,
 *                      the Variable(/VariableType) nodes Value field shall contain
 *                      the index in the variables array containing the actual value as a single UInt32 value.
 *                      And the nodes array shall be constant to be stored in ROM.
 * \param nb_variables  the number of variants in the variables array
 * \param variables     the array containing the actual Variable nodes Value field.
 *
 * \return the created read only nodes AddressSpace
 */
SOPC_AddressSpace* SOPC_AddressSpace_CreateReadOnlyNodes(uint32_t nb_nodes,
                                                         SOPC_AddressSpace_Node* nodes,
                                                         uint32_t nb_variables,
                                                         SOPC_Variant* variables);

/**
 * \brief Returns true if the AddressSpace has been created using ::SOPC_AddressSpace_CreateReadOnlyNodes
 *
 * \param space  the AddressSpace to be evaluated
 *
 * \return       true if the AddressSpace nodes are read only, false otherwise
 */
bool SOPC_AddressSpace_AreReadOnlyNodes(const SOPC_AddressSpace* space);

/**
 * \brief Appends a new node to the AddressSpace
 *
 * \param space  the AddressSpace to which the node will be appended
 * \param node   the node to append
 *
 * \return       SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_STATE if the AddressSpace nodes are read only
 *               and SOPC_STATUS_NOK in other cases of failure.
 */
SOPC_ReturnStatus SOPC_AddressSpace_Append(SOPC_AddressSpace* space, SOPC_AddressSpace_Node* node);

/**
 * \brief Deletes the AddressSpace content.
 *        It clears the Variable / VariableType nodes values and clear/free each node when
 *        AddressSpace was created using ::SOPC_AddressSpace_Create with (free_nodes=true) configuration.
 *
 * \param space  the AddressSpace to delete
 */
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
