/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOPC_ADDRESS_SPACE_H_
#define SOPC_ADDRESS_SPACE_H_

#include <stdint.h>

#include "sopc_array.h"
#include "sopc_types.h"

#define FOR_EACH_ELEMENT_TYPE(x, extra)                                                                               \
    x(DataType, data_type, extra) x(Method, method, extra) x(Object, object, extra) x(ObjectType, object_type, extra) \
        x(ReferenceType, reference_type, extra) x(Variable, variable, extra) x(VariableType, variable_type, extra)    \
            x(View, view, extra)

#define ELEMENT_ATTRIBUTE_GETTER_DECL(ret_type, lowercase_name, camel_case_name) \
    ret_type* current_element_##lowercase_name(SOPC_AddressSpace_Description_Item* item);

typedef struct
{
    OpcUa_NodeClass node_class;
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
} SOPC_AddressSpace_Description_Item;

/* Address space structure */
typedef struct _SOPC_AddressSpace
{
    uint32_t nbVariables;
    uint32_t nbVariableTypes;
    uint32_t nbObjectTypes;
    uint32_t nbReferenceTypes;
    uint32_t nbDataTypes;
    uint32_t nbMethods;
    uint32_t nbObjects;
    uint32_t nbViews;
    uint32_t nbNodesTotal; /* Sum of precedent numbers */

    /* Note: node index is valid for [1, nbNodesTotal], index 0 is used for invalid node */
    /* Note 2: nodes shall be provided by node class and with a predefined order on their node class:
       View, Object, Variable, VariableType, ObjectType, ReferenceType, DataType, Method */

    SOPC_QualifiedName* browseNameArray; /* Browse name by node index */

    int* descriptionIdxArray_begin;       /* Given node index, provides the start index in descriptionArray*/
    int* descriptionIdxArray_end;         /* Given node index, provides the end index in descriptionArray*/
    SOPC_LocalizedText* descriptionArray; /* Given description index, provides the localized text */
    size_t nbDescriptionsTotal; /* Number of elements in descriptionArray, including the 0th invalid element */

    int* displayNameIdxArray_begin;       /* Given node index, provides the start index in displayNameArray*/
    int* displayNameIdxArray_end;         /* Given node index, provides the end index in displayNameArray*/
    SOPC_LocalizedText* displayNameArray; /* Given displayName index, provides the localized text */
    size_t nbDisplayNamesTotal; /* Number of elements in displayNameArray, including the 0th invalid element */

    OpcUa_NodeClass* nodeClassArray; /* All nodes classes by node index */

    SOPC_NodeId** nodeIdArray; /* All nodes Ids by node index */

    int* referenceIdxArray_begin;     /* Given node index, provides the start reference index*/
    int* referenceIdxArray_end;       /* Given node index, provides the end in reference index */
    SOPC_NodeId** referenceTypeArray; /* Given reference index, provides the reference type node Id */
    SOPC_ExpandedNodeId**
        referenceTargetArray;      /* Given reference index, provides the reference target expended node Id */
    bool* referenceIsForwardArray; /* Given reference index, provides the reference isForward flag value */
    size_t nbReferencesTotal; /* Number of elements in referenceTyepeArray and referenceTargetArray, including the 0th
                                 invalid element */

    SOPC_Variant* valueArray; /* Given (node index - nbViews - nbObjects), provides the variable/variableType value */
    SOPC_StatusCode*
        valueStatusArray; /* Given (node index - nbViews - nbObjects), provides the variable/variableType value */

    SOPC_Byte* accessLevelArray; /* Given (node index - nbViews - nbObjects), provides the variable access level */

    SOPC_NodeId** externalNodeIds; /* Node IDs that are referenced from but don't belong to the address space */
    size_t nbExternalNodeIds;
} SOPC_AddressSpace;

typedef SOPC_Array SOPC_AddressSpace_Description;

void SOPC_AddressSpace_Description_Item_Initialize(SOPC_AddressSpace_Description_Item* item, uint32_t element_type);

SOPC_NodeId* SOPC_AddressSpace_Description_Item_Get_NodeId(SOPC_AddressSpace_Description_Item* item);
SOPC_QualifiedName* SOPC_AddressSpace_Description_Item_Get_BrowseName(SOPC_AddressSpace_Description_Item* item);
SOPC_LocalizedText* SOPC_AddressSpace_Description_Item_Get_DisplayName(SOPC_AddressSpace_Description_Item* item);
SOPC_LocalizedText* SOPC_AddressSpace_Description_Item_Get_Description(SOPC_AddressSpace_Description_Item* item);
int32_t* SOPC_AddressSpace_Description_Item_Get_NoOfReferences(SOPC_AddressSpace_Description_Item* item);
OpcUa_ReferenceNode** SOPC_AddressSpace_Description_Item_Get_References(SOPC_AddressSpace_Description_Item* item);
SOPC_Variant* SOPC_AddressSpace_Description_Item_Get_Value(SOPC_AddressSpace_Description_Item* item);

void SOPC_AddressSpace_Description_Item_Clear(SOPC_AddressSpace_Description_Item* item);

SOPC_AddressSpace_Description* SOPC_AddressSpace_Description_Create(void);

SOPC_ReturnStatus SOPC_AddressSpace_Description_Append(SOPC_AddressSpace_Description* desc,
                                                       SOPC_AddressSpace_Description_Item* item);
SOPC_ReturnStatus SOPC_AddressSpace_Generate(const SOPC_AddressSpace_Description* desc, SOPC_AddressSpace* space);

void SOPC_AddressSpace_Description_Delete(SOPC_AddressSpace_Description* desc);

void SOPC_AddressSpace_Clear(SOPC_AddressSpace* space);

#endif /* SOPC_ADDRESS_SPACE_H_ */
