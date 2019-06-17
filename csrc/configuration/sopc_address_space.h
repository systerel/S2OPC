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

#include "sopc_array.h"
#include "sopc_dict.h"
#include "sopc_types.h"

#define FOR_EACH_ELEMENT_TYPE(x, extra)                                                                               \
    x(DataType, data_type, extra) x(Method, method, extra) x(Object, object, extra) x(ObjectType, object_type, extra) \
        x(ReferenceType, reference_type, extra) x(Variable, variable, extra) x(VariableType, variable_type, extra)    \
            x(View, view, extra)

#define ELEMENT_ATTRIBUTE_GETTER_DECL(ret_type, lowercase_name, camel_case_name) \
    ret_type* current_element_##lowercase_name(SOPC_AddressSpace_Item* item);

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
        OpcUa_VariableNode variable;
        OpcUa_VariableTypeNode variable_type;
        OpcUa_ViewNode view;
    } data;
} SOPC_AddressSpace_Item;

/* Address space structure */
/* Maps NodeId to SOPC_AddressSpace_Item */
typedef SOPC_Dict SOPC_AddressSpace;

void SOPC_AddressSpace_Item_Initialize(SOPC_AddressSpace_Item* item, OpcUa_NodeClass element_type);

/* Common attributes */
OpcUa_NodeClass* SOPC_AddressSpace_Item_Get_NodeClass(SOPC_AddressSpace_Item* item);
SOPC_NodeId* SOPC_AddressSpace_Item_Get_NodeId(SOPC_AddressSpace_Item* item);
SOPC_QualifiedName* SOPC_AddressSpace_Item_Get_BrowseName(SOPC_AddressSpace_Item* item);
SOPC_LocalizedText* SOPC_AddressSpace_Item_Get_DisplayName(SOPC_AddressSpace_Item* item);
SOPC_LocalizedText* SOPC_AddressSpace_Item_Get_Description(SOPC_AddressSpace_Item* item);
uint32_t* SOPC_AddressSpace_Item_Get_WriteMask(SOPC_AddressSpace_Item* item);
uint32_t* SOPC_AddressSpace_Item_Get_UserWriteMask(SOPC_AddressSpace_Item* item);
int32_t* SOPC_AddressSpace_Item_Get_NoOfReferences(SOPC_AddressSpace_Item* item);
OpcUa_ReferenceNode** SOPC_AddressSpace_Item_Get_References(SOPC_AddressSpace_Item* item);
/* Variable and VariableType common attributes */
SOPC_Variant* SOPC_AddressSpace_Item_Get_Value(SOPC_AddressSpace_Item* item);
SOPC_Byte SOPC_AddressSpace_Item_Get_AccessLevel(SOPC_AddressSpace_Item* item);
SOPC_NodeId* SOPC_AddressSpace_Item_Get_DataType(SOPC_AddressSpace_Item* item);
int32_t* SOPC_AddressSpace_Item_Get_ValueRank(SOPC_AddressSpace_Item* item);
int32_t SOPC_AddressSpace_Item_Get_NoOfArrayDimensions(SOPC_AddressSpace_Item* item);
uint32_t* SOPC_AddressSpace_Item_Get_ArrayDimensions(SOPC_AddressSpace_Item* item);
/* Types common attribute */
SOPC_Boolean* SOPC_AddressSpace_Item_Get_IsAbstract(SOPC_AddressSpace_Item* item);

void SOPC_AddressSpace_Item_Clear(SOPC_AddressSpace_Item* item);

SOPC_AddressSpace* SOPC_AddressSpace_Create(bool free_items);
SOPC_ReturnStatus SOPC_AddressSpace_Append(SOPC_AddressSpace* space, SOPC_AddressSpace_Item* item);
void SOPC_AddressSpace_Delete(SOPC_AddressSpace* space);

#endif /* SOPC_ADDRESS_SPACE_H_ */
