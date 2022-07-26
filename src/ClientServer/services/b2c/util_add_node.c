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
#include <stdbool.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_builtintypes.h"
#include "sopc_mem_alloc.h"

#include "address_space_impl.h"
#include "util_add_node.h"
#include "util_address_space.h"

static const SOPC_NodeId DataVariable_Type = {SOPC_IdentifierType_Numeric, 0,
                                              .Data.Numeric = OpcUaId_BaseDataVariableType};
static const SOPC_NodeId Property_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_PropertyType};

static const SOPC_NodeId Organizes_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Organizes};
static const SOPC_NodeId Aggregates_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Aggregates};
static const SOPC_NodeId HasComponent_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasComponent};
static const SOPC_NodeId HasProperty_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasProperty};

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

void util_add_node__check_constraints_addNode_AddressSpace_Variable(
    const SOPC_NodeId* parentNid,
    const SOPC_NodeId* refTypeId,
    const SOPC_QualifiedName* browseName,
    const SOPC_NodeId* typeDefId,
    constants_statuscodes_bs__t_StatusCode_i* sc_addnode)
{
    *sc_addnode = constants_statuscodes_bs__e_sc_bad_unexpected_error;
    bool found = false;
    SOPC_AddressSpace_Node* parentNode = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, parentNid, &found);
    assert(found && NULL != parentNode);
    OpcUa_NodeClass* parentNodeClass = SOPC_AddressSpace_Get_NodeClass(address_space_bs__nodes, parentNode);
    assert(NULL != parentNodeClass);

    bool refTypeOk =
        check_variable_reference_type_to_parent(parentNode, *parentNodeClass, refTypeId, typeDefId, sc_addnode);
    if (!refTypeOk)
    {
        return;
    }

    // Check type definition is VariableType node class
    SOPC_AddressSpace_Node* typeDefNode = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, typeDefId, &found);
    assert(found && NULL != typeDefNode);
    OpcUa_NodeClass* typeDefNodeClass = SOPC_AddressSpace_Get_NodeClass(address_space_bs__nodes, typeDefNode);
    assert(NULL != typeDefNodeClass);
    if (OpcUa_NodeClass_VariableType != *typeDefNodeClass)
    {
        *sc_addnode = constants_statuscodes_bs__e_sc_bad_type_definition_invalid;
        return;
    }

    bool browseNameOk = check_browse_name_unique_from_parent(parentNode, browseName, sc_addnode);
    if (browseNameOk)
    {
        *sc_addnode = constants_statuscodes_bs__e_sc_ok;
    }
}

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

SOPC_ReturnStatus util_add_node__AddVariableNodeAttributes(SOPC_AddressSpace_Node* node,
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

SOPC_ReturnStatus util_add_node__AddRefChildToParentNode(const SOPC_NodeId* parentNodeId,
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

void util_add_node__RemLastRefInParentNode(const SOPC_NodeId* parentNodeId)
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
