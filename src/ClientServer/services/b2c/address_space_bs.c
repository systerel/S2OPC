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

static const SOPC_NodeId DataVariable_Type = {SOPC_IdentifierType_Numeric, 0,
                                              .Data.Numeric = OpcUaId_BaseDataVariableType};
static const SOPC_NodeId Property_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_PropertyType};

static const SOPC_NodeId Organizes_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Organizes};
static const SOPC_NodeId Aggregates_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Aggregates};
static const SOPC_NodeId HasComponent_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasComponent};
static const SOPC_NodeId HasProperty_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasProperty};

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

// Note: to make this function generic for a SOPC_AddressSpace_Node we need to have setters for attributes
static SOPC_ReturnStatus util_AddCommonNodeAttributes(OpcUa_Node* node,
                                                      const OpcUa_NodeAttributes* commonNodeAttributes,
                                                      constants_statuscodes_bs__t_StatusCode_i* sc_addnode)
{
    assert(NULL != sc_addnode);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Note: NodeAttributes masks are the same than AttributeWrite masks
    // DisplayName
    if (0 != (commonNodeAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_DisplayName))
    {
        status = SOPC_LocalizedText_Copy(&node->DisplayName, &commonNodeAttributes->DisplayName);
    }
    else
    {
        // Use browse name
        SOPC_LocalizedText lt;
        SOPC_LocalizedText_Initialize(&lt);
        status = SOPC_String_AttachFrom(&lt.defaultText, &node->BrowseName.Name);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_LocalizedText_Copy(&node->DisplayName, &lt);
        }
    }
    assert(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (commonNodeAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_Description))
        {
            status = SOPC_LocalizedText_Copy(&node->Description, &commonNodeAttributes->Description);
        } // else: no description
    }
    assert(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (commonNodeAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_WriteMask) ||
            commonNodeAttributes->WriteMask != 0)
        {
            // Note: server does not manage write masks, it is not possible to specify some
            *sc_addnode = constants_statuscodes_bs__e_sc_bad_node_attributes_invalid;
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (commonNodeAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_UserWriteMask) ||
            commonNodeAttributes->UserWriteMask != 0)
        {
            // Note: server does not manage write masks, it is not possible to specify some
            *sc_addnode = constants_statuscodes_bs__e_sc_bad_node_attributes_invalid;
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    return status;
}

static SOPC_ReturnStatus util_AddVariableNodeAttributes(SOPC_AddressSpace_Node* node,
                                                        OpcUa_VariableNode* varNode,
                                                        const SOPC_ExtensionObject* nodeAttributes,
                                                        constants_statuscodes_bs__t_StatusCode_i* sc_addnode)
{
    assert(NULL != sc_addnode);

    assert(NULL != varNode);
    assert(NULL != nodeAttributes);
    assert(SOPC_ExtObjBodyEncoding_Object == nodeAttributes->Encoding);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    assert(&OpcUa_NodeAttributes_EncodeableType == nodeAttributes->Body.Object.ObjType ||
           &OpcUa_VariableAttributes_EncodeableType == nodeAttributes->Body.Object.ObjType);
    // Common fields have same offsets in OpcUa_Node and OpcUa_VariableNode
    // Common attributes have same offsets in OpcUa_NodeAttributes and OpcUa_VariableAttributes
    status = util_AddCommonNodeAttributes((OpcUa_Node*) varNode,
                                          (OpcUa_NodeAttributes*) nodeAttributes->Body.Object.Value, sc_addnode);
    OpcUa_VariableAttributes* varAttrs = (OpcUa_VariableAttributes*) nodeAttributes->Body.Object.Value;
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttrs->SpecifiedAttributes & OpcUa_NodeAttributesMask_AccessLevel))
        {
            varNode->AccessLevel = varAttrs->AccessLevel;
        }
        else
        {
            // Allow read access
            varNode->AccessLevel = SOPC_AccessLevelMask_CurrentRead;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttrs->SpecifiedAttributes & OpcUa_NodeAttributesMask_UserAccessLevel) ||
            (varAttrs->UserAccessLevel != 0 && varAttrs->UserAccessLevel != varAttrs->AccessLevel))
        {
            // Note: server does not manage to set user access level this way
            *sc_addnode = constants_statuscodes_bs__e_sc_bad_node_attributes_invalid;
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttrs->SpecifiedAttributes & OpcUa_NodeAttributesMask_ArrayDimensions))
        {
            if (varAttrs->NoOfArrayDimensions > 0 && NULL != varAttrs->ArrayDimensions)
            {
                varNode->NoOfArrayDimensions = varAttrs->NoOfArrayDimensions;
                varNode->ArrayDimensions = varAttrs->ArrayDimensions;
            }
            else if (varAttrs->NoOfArrayDimensions > 0)
            {
                // Incoherent parameters
                *sc_addnode = constants_statuscodes_bs__e_sc_bad_node_attributes_invalid;
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        } // else: remains 0
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttrs->SpecifiedAttributes & OpcUa_NodeAttributesMask_DataType))
        {
            status = SOPC_NodeId_Copy(&varNode->DataType, &varAttrs->DataType);
            if (SOPC_STATUS_OK != status)
            {
                *sc_addnode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
        }
        else
        {
            // Base DataType
            varNode->DataType.Namespace = 0;
            varNode->DataType.IdentifierType = SOPC_IdentifierType_Numeric;
            varNode->DataType.Data.Numeric = OpcUaId_BaseDataType;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttrs->SpecifiedAttributes & OpcUa_NodeAttributesMask_Historizing) && varAttrs->Historizing)
        {
            // Note: server does not manage to set historizing
            *sc_addnode = constants_statuscodes_bs__e_sc_bad_node_attributes_invalid;
            status = SOPC_STATUS_INVALID_PARAMETERS;
        } // else: remains false
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttrs->SpecifiedAttributes & OpcUa_NodeAttributesMask_MinimumSamplingInterval))
        {
            varNode->MinimumSamplingInterval = varAttrs->MinimumSamplingInterval;
        } // else: remains 0
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttrs->SpecifiedAttributes & OpcUa_NodeAttributesMask_Value))
        {
            status = SOPC_Variant_Copy(&varNode->Value, &varAttrs->Value);
            if (SOPC_STATUS_OK != status)
            {
                *sc_addnode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
            else
            {
                // No source timestamp available, keep 0 init value
                node->value_status = SOPC_GoodGenericStatus;
            }
        }
        else
        {
            // NULL variant by default, set status code
            node->value_status = OpcUa_UncertainInitialValue;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttrs->SpecifiedAttributes & OpcUa_NodeAttributesMask_ValueRank))
        {
            varNode->ValueRank = varAttrs->ValueRank;
        }
        else
        {
            // Any value rank
            varNode->ValueRank = constants__c_ValueRank_Any;
        }
    }
    return status;
}

static SOPC_ReturnStatus util_AddRefChildToParentNode(const SOPC_NodeId* parentNodeId,
                                                      const SOPC_NodeId* childNodeId,
                                                      const SOPC_NodeId* refTypeId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    bool found = false;
    SOPC_AddressSpace_Node* parentNode = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, parentNodeId, &found);
    assert(found && NULL != parentNode);
    int32_t* nbRefs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, parentNode);
    assert(NULL != nbRefs);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, parentNode);
    assert(NULL != refs);
    if (*nbRefs > 0 && (uint64_t) *nbRefs < SIZE_MAX)
    {
        OpcUa_ReferenceNode* newRefs = SOPC_Calloc(((size_t) *nbRefs) + 1, sizeof(*newRefs));
        if (NULL == newRefs)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            // Copy old references
            void* to = memcpy(newRefs, *refs, ((size_t) *nbRefs) * sizeof(*newRefs));
            assert(to == newRefs);
            SOPC_Free(*refs);
            *refs = newRefs;
        }
        if (SOPC_STATUS_OK == status)
        {
            // Add hierarchical reference to new child
            OpcUa_ReferenceNode* hierarchicalRef = &newRefs[*nbRefs];
            OpcUa_ReferenceNode_Initialize(hierarchicalRef);
            hierarchicalRef->IsInverse = false;
            status = SOPC_NodeId_Copy(&hierarchicalRef->ReferenceTypeId, refTypeId);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_NodeId_Copy(&hierarchicalRef->TargetId.NodeId, childNodeId);
            }
            else
            {
                assert(SOPC_STATUS_OUT_OF_MEMORY == status);
            }
            if (SOPC_STATUS_OK != status)
            {
                assert(SOPC_STATUS_OUT_OF_MEMORY == status);
                SOPC_NodeId_Clear(&hierarchicalRef->ReferenceTypeId);
                SOPC_NodeId_Clear(&hierarchicalRef->TargetId.NodeId);
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            // Update number of references
            *nbRefs += 1;
        }
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }
    return status;
}

static void util_RemLastRefInParentNode(const SOPC_NodeId* parentNodeId)
{
    // Rollback reference added in parent
    bool found = false;
    SOPC_AddressSpace_Node* parentNode = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, parentNodeId, &found);
    assert(found && NULL != parentNode);
    int32_t* nbRefs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, parentNode);
    assert(NULL != nbRefs);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, parentNode);
    *nbRefs -= 1;
    OpcUa_ReferenceNode_Clear(&(*refs[*nbRefs]));
    return;
}

void address_space_bs__addNode_AddressSpace_Variable(
    const constants__t_ExpandedNodeId_i address_space_bs__p_parentNid,
    const constants__t_NodeId_i address_space_bs__p_refTypeId,
    const constants__t_NodeId_i address_space_bs__p_newNodeId,
    const constants__t_QualifiedName_i address_space_bs__p_browseName,
    const constants__t_NodeClass_i address_space_bs__p_nodeClass,
    const constants__t_NodeAttributes_i address_space_bs__p_nodeAttributes,
    const constants__t_ExpandedNodeId_i address_space_bs__p_typeDefId,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc_addnode)
{
    assert(SOPC_AddressSpace_AreNodesFreed(address_space_bs__nodes));
    assert(!SOPC_AddressSpace_AreReadOnlyNodes(address_space_bs__nodes));

    assert(constants__e_ncl_Variable == address_space_bs__p_nodeClass);
    assert(SOPC_ExtObjBodyEncoding_Object == address_space_bs__p_nodeAttributes->Encoding);
    assert(&OpcUa_NodeAttributes_EncodeableType == address_space_bs__p_nodeAttributes->Body.Object.ObjType ||
           &OpcUa_VariableAttributes_EncodeableType == address_space_bs__p_nodeAttributes->Body.Object.ObjType);
    assert(0 == address_space_bs__p_typeDefId->ServerIndex);
    *address_space_bs__sc_addnode = constants_statuscodes_bs__e_sc_bad_out_of_memory;
    SOPC_AddressSpace_Node* newNode = SOPC_Calloc(1, sizeof(*newNode));
    if (NULL == newNode)
    {
        return;
    }
    SOPC_AddressSpace_Node_Initialize(address_space_bs__nodes, newNode, OpcUa_NodeClass_Variable);

    // Copy the main parameters not included in NodeAttributes structure
    OpcUa_VariableNode* varNode = &newNode->data.variable;
    // NodeID
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(&varNode->NodeId, address_space_bs__p_newNodeId);
    assert(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);

    // BrowseName
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_QualifiedName_Copy(&varNode->BrowseName, address_space_bs__p_browseName);
        assert(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    }
    // References from new node (backward to parent and forward to type)
    if (SOPC_STATUS_OK == status)
    {
        varNode->References = SOPC_Calloc(2, sizeof(*varNode->References));
        if (NULL == varNode->References)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            varNode->NoOfReferences = 2;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        // Set HasTypeDefinition
        OpcUa_ReferenceNode* hasTypeDef = &varNode->References[0];
        hasTypeDef->IsInverse = false;
        hasTypeDef->ReferenceTypeId.Namespace = 0;
        hasTypeDef->ReferenceTypeId.IdentifierType = SOPC_IdentifierType_Numeric;
        hasTypeDef->ReferenceTypeId.Data.Numeric = OpcUaId_HasTypeDefinition;
        status = SOPC_ExpandedNodeId_Copy(&hasTypeDef->TargetId, address_space_bs__p_typeDefId);
        assert(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    }
    if (SOPC_STATUS_OK == status)
    {
        // Set hierarchical reference to parent
        OpcUa_ReferenceNode* hierarchicalRef = &varNode->References[1];
        hierarchicalRef->IsInverse = true;
        status = SOPC_NodeId_Copy(&hierarchicalRef->ReferenceTypeId, address_space_bs__p_refTypeId);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ExpandedNodeId_Copy(&hierarchicalRef->TargetId, address_space_bs__p_parentNid);
        }
        else
        {
            assert(SOPC_STATUS_OUT_OF_MEMORY == status);
        }
    }
    // Manage NodeAttributes
    if (SOPC_STATUS_OK == status)
    {
        status = util_AddVariableNodeAttributes(newNode, varNode, address_space_bs__p_nodeAttributes,
                                                address_space_bs__sc_addnode);
    }
    // Set reciprocal reference from parent and add node to address space
    if (SOPC_STATUS_OK == status)
    {
        status = util_AddRefChildToParentNode(&address_space_bs__p_parentNid->NodeId, address_space_bs__p_newNodeId,
                                              address_space_bs__p_refTypeId);
        // Add node to address space
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_AddressSpace_Append(address_space_bs__nodes, newNode);
            if (SOPC_STATUS_OK != status)
            {
                assert(SOPC_STATUS_OUT_OF_MEMORY == status);
                // Rollback reference added in parent
                util_RemLastRefInParentNode(&address_space_bs__p_parentNid->NodeId);
            }
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        *address_space_bs__sc_addnode = constants_statuscodes_bs__e_sc_ok;
    }
    else
    {
        // Clear and dealloc node
        SOPC_AddressSpace_Node_Clear(address_space_bs__nodes, newNode);
        SOPC_Free(newNode);
    }
}

void address_space_bs__addNode_check_valid_node_attributes_type(
    const constants__t_NodeClass_i address_space_bs__p_nodeClass,
    const constants__t_NodeAttributes_i address_space_bs__p_nodeAttributes,
    t_bool* const address_space_bs__bres)
{
    // Check NodeAttributes is well decoded as an OPC UA object: verified in msg_node_management_add_nodes_bs
    assert(SOPC_ExtObjBodyEncoding_Object == address_space_bs__p_nodeAttributes->Encoding);

    // Check NodeAttributes type depending on NodeClass
    SOPC_EncodeableType* expectedNodeAttrsType = NULL;
    switch (address_space_bs__p_nodeClass)
    {
    case constants__e_ncl_Object:
        expectedNodeAttrsType = &OpcUa_ObjectAttributes_EncodeableType;
        break;
    case constants__e_ncl_Variable:
        expectedNodeAttrsType = &OpcUa_VariableAttributes_EncodeableType;
        break;
    case constants__e_ncl_Method:
        expectedNodeAttrsType = &OpcUa_MethodAttributes_EncodeableType;
        break;
    case constants__e_ncl_ObjectType:
        expectedNodeAttrsType = &OpcUa_ObjectTypeAttributes_EncodeableType;
        break;
    case constants__e_ncl_VariableType:
        expectedNodeAttrsType = &OpcUa_VariableTypeAttributes_EncodeableType;
        break;
    case constants__e_ncl_ReferenceType:
        expectedNodeAttrsType = &OpcUa_ReferenceTypeAttributes_EncodeableType;
        break;
    case constants__e_ncl_DataType:
        expectedNodeAttrsType = &OpcUa_DataTypeAttributes_EncodeableType;
        break;
    case constants__e_ncl_View:
        expectedNodeAttrsType = &OpcUa_ViewAttributes_EncodeableType;
        break;
    default:
        assert(false &&
               "NodeClass must have been already checked by "
               "msg_node_management_add_nodes_bs__getall_add_node_item_req_params");
    }

    SOPC_EncodeableType* actualNodeAttrsType = address_space_bs__p_nodeAttributes->Body.Object.ObjType;
    if (&OpcUa_NodeAttributes_EncodeableType == actualNodeAttrsType || expectedNodeAttrsType == actualNodeAttrsType)
    {
        *address_space_bs__bres = true;
    }
    else
    {
        *address_space_bs__bres = false;
    }
}

static bool is_type_or_subtype(const SOPC_NodeId* actualType, const SOPC_NodeId* expectedType)
{
    return SOPC_NodeId_Equal(actualType, expectedType) ||
           util_addspace__recursive_is_transitive_subtype(RECURSION_LIMIT, actualType, actualType, expectedType);
}

static bool check_variable_organizes_reference(OpcUa_NodeClass parentNodeClass,
                                               constants_statuscodes_bs__t_StatusCode_i* sc_addnode)
{
    /* §7.11 Part 3 (1.05): The SourceNode of References of this type shall be an Object, ObjectType or a View.
     *                      [...] The TargetNode of this ReferenceType can be of any NodeClass.
     */
    switch (parentNodeClass)
    {
    case OpcUa_NodeClass_Object:
    case OpcUa_NodeClass_ObjectType:
    case OpcUa_NodeClass_View:
        return true;
    default:
        *sc_addnode = constants_statuscodes_bs__e_sc_bad_reference_not_allowed;
        return false;
    }
}

static bool check_variable_has_component_reference(SOPC_AddressSpace_Node* parentNode,
                                                   OpcUa_NodeClass parentNodeClass,
                                                   const SOPC_NodeId* typeDefId,
                                                   constants_statuscodes_bs__t_StatusCode_i* sc_addnode)
{
    SOPC_ExpandedNodeId* parentTypeDef = NULL;
    bool isParentNodeDataVariable = false;

    /* §7.7 Part 3 (1.05): If the TargetNode is a Variable, the SourceNode shall be an Object, an ObjectType, a
     * DataVariable or a VariableType. By using the HasComponent Reference, the Variable is defined
     * as DataVariable.
     */
    // Target Variable is a DataVariable
    bool isDataVariable = is_type_or_subtype(typeDefId, &DataVariable_Type);
    if (!isDataVariable)
    {
        *sc_addnode = constants_statuscodes_bs__e_sc_bad_type_definition_invalid;
        return false;
    }
    switch (parentNodeClass)
    {
    case OpcUa_NodeClass_Object:
    case OpcUa_NodeClass_ObjectType:
    case OpcUa_NodeClass_VariableType:
        return true;
    case OpcUa_NodeClass_Variable:
        // Source Variable is a DataVariable
        util_addspace__get_TypeDefinition(parentNode, &parentTypeDef);
        if (NULL != parentTypeDef)
        {
            isParentNodeDataVariable = is_type_or_subtype(&parentTypeDef->NodeId, &DataVariable_Type);
        }
        if (!isParentNodeDataVariable)
        {
            *sc_addnode = constants_statuscodes_bs__e_sc_bad_reference_not_allowed;
            return false;
        }
        break;
    default:
        *sc_addnode = constants_statuscodes_bs__e_sc_bad_reference_not_allowed;
        return false;
    }
    return true;
}

static bool check_variable_has_property_reference(SOPC_AddressSpace_Node* parentNode,
                                                  OpcUa_NodeClass parentNodeClass,
                                                  const SOPC_NodeId* typeDefId,
                                                  constants_statuscodes_bs__t_StatusCode_i* sc_addnode)
{
    SOPC_ExpandedNodeId* parentTypeDef = NULL;

    /* §7.8 Part 3 (1.05): The SourceNode of this ReferenceType can be of any NodeClass. The TargetNode shall be a
     * Variable. By using the HasProperty Reference, the Variable is defined as Property.
     * Properties shall not have Properties, a Property shall never be the SourceNode of a HasProperty Reference.
     */
    if (OpcUa_NodeClass_Variable == parentNodeClass)
    {
        // Source Variable is not a Property itself
        util_addspace__get_TypeDefinition(parentNode, &parentTypeDef);
        bool isParentNodeProperty = true;
        if (NULL != parentTypeDef)
        {
            isParentNodeProperty = is_type_or_subtype(&parentTypeDef->NodeId, &Property_Type);
        }
        if (isParentNodeProperty)
        {
            *sc_addnode = constants_statuscodes_bs__e_sc_bad_reference_not_allowed;
            return false;
        }

        // Target Variable is a property
        bool isPropertyVariable = is_type_or_subtype(typeDefId, &Property_Type);
        if (!isPropertyVariable)
        {
            *sc_addnode = constants_statuscodes_bs__e_sc_bad_reference_not_allowed;
            return false;
        }
    }
    return true;
}

static bool check_variable_reference_type_to_parent(SOPC_AddressSpace_Node* parentNode,
                                                    OpcUa_NodeClass parentNodeClass,
                                                    const SOPC_NodeId* referenceTypeId,
                                                    const SOPC_NodeId* typeDefId,
                                                    constants_statuscodes_bs__t_StatusCode_i* sc_addnode)
{
    bool validRef = false;

    // Evaluate Organizes reference type from parent to this variable node
    bool isOrganizesRef = is_type_or_subtype(referenceTypeId, &Organizes_Type);
    if (isOrganizesRef)
    {
        validRef = check_variable_organizes_reference(parentNodeClass, sc_addnode);
        if (!validRef)
        {
            return false;
        }
    }

    if (!validRef)
    {
        // Evaluate HasComponent reference type from parent to this variable node
        bool isHasComponentRef = is_type_or_subtype(referenceTypeId, &HasComponent_Type);

        if (isHasComponentRef)
        {
            validRef = check_variable_has_component_reference(parentNode, parentNodeClass, typeDefId, sc_addnode);
            if (!validRef)
            {
                return false;
            }
        }
    }

    if (!validRef)
    {
        // Evaluate HasProperty reference type from parent to this variable node
        bool isHasPropertyRef = is_type_or_subtype(referenceTypeId, &HasProperty_Type);
        if (isHasPropertyRef)
        {
            validRef = check_variable_has_property_reference(parentNode, parentNodeClass, typeDefId, sc_addnode);
        }
    }

    if (!validRef)
    {
        // Evaluate (unknown) Aggregates reference type from parent to this variable node
        bool isAggregatesRef = util_addspace__recursive_is_transitive_subtype(RECURSION_LIMIT, referenceTypeId,
                                                                              referenceTypeId, &Aggregates_Type);
        /* §7.6 Part 3 (1.05): The Aggregates ReferenceType is an abstract ReferenceType; only subtypes of it can be
         * used. It is a subtype of HasChild. [..] There are no constraints defined for this abstract ReferenceType.
         */
        if (isAggregatesRef)
        {
            validRef = true;
        }
        else
        {
            // We did not identify a valid reference type
            *sc_addnode = constants_statuscodes_bs__e_sc_bad_reference_not_allowed;
        }
    }

    return validRef;
}

static bool check_browse_name_unique_from_parent(SOPC_AddressSpace_Node* parentNode,
                                                 const SOPC_QualifiedName* browseName,
                                                 constants_statuscodes_bs__t_StatusCode_i* sc_addnode)
{
    /* Simplified version of the BrowseName uniqueness check:
     * - apply for all whereas it is only indicated for Properties and TypeDefinition / InstanceDeclaration.
     *   (but AddNodes service status code indicates: The browse name is not unique among nodes that share
     *    the same relationship with the parent.)
     * - check for any hierarchical reference type whereas it might be acceptable
     *   (but part 3 (1.05) §4.5.4 indicates: A TypeDefinitionNode or an InstanceDeclaration
     *    shall never reference two Nodes having the same BrowseName using forward hierarchical References.)
     */

    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, parentNode);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, parentNode);
    int32_t comparison = -1;
    bool found = false;

    for (int32_t i = 0; i < *n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &(*refs)[i];

        if (!ref->IsInverse && util_addspace__recursive_is_transitive_subtype(
                                   RECURSION_LIMIT, &ref->ReferenceTypeId, &ref->ReferenceTypeId,
                                   constants_bs__c_HierarchicalReferences_Type_NodeId))
        {
            // Note: NamespaceUri should not be interpreted as external server but is not managed for now
            if (0 == ref->TargetId.ServerIndex && ref->TargetId.NamespaceUri.Length <= 0)
            {
                SOPC_AddressSpace_Node* node =
                    SOPC_AddressSpace_Get_Node(address_space_bs__nodes, &ref->TargetId.NodeId, &found);
                if (found)
                {
                    SOPC_QualifiedName* otherBrowseName =
                        SOPC_AddressSpace_Get_BrowseName(address_space_bs__nodes, node);
                    SOPC_ReturnStatus status = SOPC_QualifiedName_Compare(browseName, otherBrowseName, &comparison);
                    assert(SOPC_STATUS_OK == status);
                    if (0 == comparison)
                    {
                        *sc_addnode = constants_statuscodes_bs__e_sc_bad_browse_name_duplicated;
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

void address_space_bs__check_constraints_addNode_AddressSpace_Variable(
    const constants__t_ExpandedNodeId_i address_space_bs__p_parentNid,
    const constants__t_NodeId_i address_space_bs__p_refTypeId,
    const constants__t_NodeId_i address_space_bs__p_newNodeId,
    const constants__t_QualifiedName_i address_space_bs__p_browseName,
    const constants__t_NodeClass_i address_space_bs__p_nodeClass,
    const constants__t_ExpandedNodeId_i address_space_bs__p_typeDefId,
    constants_statuscodes_bs__t_StatusCode_i* const address_space_bs__sc_addnode)
{
    SOPC_UNUSED_ARG(address_space_bs__p_newNodeId);
    SOPC_UNUSED_ARG(address_space_bs__p_nodeClass);
    *address_space_bs__sc_addnode = constants_statuscodes_bs__e_sc_bad_unexpected_error;
    bool found = false;
    SOPC_AddressSpace_Node* parentNode =
        SOPC_AddressSpace_Get_Node(address_space_bs__nodes, &address_space_bs__p_parentNid->NodeId, &found);
    assert(found && NULL != parentNode);
    OpcUa_NodeClass* parentNodeClass = SOPC_AddressSpace_Get_NodeClass(address_space_bs__nodes, parentNode);
    assert(NULL != parentNodeClass);

    bool refTypeOk =
        check_variable_reference_type_to_parent(parentNode, *parentNodeClass, address_space_bs__p_refTypeId,
                                                &address_space_bs__p_typeDefId->NodeId, address_space_bs__sc_addnode);
    if (!refTypeOk)
    {
        return;
    }

    // Check type definition is VariableType node class
    SOPC_AddressSpace_Node* typeDefNode =
        SOPC_AddressSpace_Get_Node(address_space_bs__nodes, &address_space_bs__p_typeDefId->NodeId, &found);
    assert(found && NULL != typeDefNode);
    OpcUa_NodeClass* typeDefNodeClass = SOPC_AddressSpace_Get_NodeClass(address_space_bs__nodes, typeDefNode);
    assert(NULL != typeDefNodeClass);
    if (OpcUa_NodeClass_VariableType != *typeDefNodeClass)
    {
        *address_space_bs__sc_addnode = constants_statuscodes_bs__e_sc_bad_type_definition_invalid;
        return;
    }

    bool browseNameOk =
        check_browse_name_unique_from_parent(parentNode, address_space_bs__p_browseName, address_space_bs__sc_addnode);
    if (browseNameOk)
    {
        *address_space_bs__sc_addnode = constants_statuscodes_bs__e_sc_ok;
    }
}

void address_space_bs__gen_addNode_event(const constants__t_NodeId_i address_space_bs__p_newNodeId)
{
    assert(NULL != address_space_bs__p_newNodeId);
    SOPC_NodeId* nodeIdCopy = SOPC_Calloc(1, sizeof(*nodeIdCopy));
    if (NULL != nodeIdCopy)
    {
        SOPC_NodeId_Initialize(nodeIdCopy);
        SOPC_ReturnStatus status = SOPC_NodeId_Copy(nodeIdCopy, address_space_bs__p_newNodeId);
        if (SOPC_STATUS_OK == status)
        {
            SOPC_EventHandler_Post(SOPC_Services_GetEventHandler(), SE_TO_SE_SERVER_NODE_CHANGED, 0, (uintptr_t) true,
                                   (uintptr_t) nodeIdCopy);
        }
        else
        {
            SOPC_Free(nodeIdCopy);
            nodeIdCopy = NULL;
        }
    }
    if (NULL == nodeIdCopy)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "address_space_bs__gen_addNode_event: NodeId allocation or copy issue");
    }
}

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
     * (DoNotClear flag set to true if constant declaration). Since we will modify variant content it will lead to
     * an hybrid content (constant and not constant) if we do not make a copy before modification.
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

        return constants_statuscodes_bs__e_sc_bad_index_range_invalid; // In case we do not support  the range
                                                                       // either (matrix)
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

void address_space_bs__is_AddressSpace_constant(t_bool* const address_space_bs__bres)
{
    *address_space_bs__bres = (!SOPC_AddressSpace_AreNodesFreed(address_space_bs__nodes) ||
                               SOPC_AddressSpace_AreReadOnlyNodes(address_space_bs__nodes));
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
