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

#include <stdbool.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"

#include "address_space_impl.h"
#include "sopc_node_mgt_helper_internal.h"
#include "util_address_space.h"

static const SOPC_NodeId DataVariable_Type = {SOPC_IdentifierType_Numeric, 0,
                                              .Data.Numeric = OpcUaId_BaseDataVariableType};
static const SOPC_NodeId Property_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_PropertyType};

static const SOPC_NodeId Organizes_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Organizes};
static const SOPC_NodeId Aggregates_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Aggregates};
static const SOPC_NodeId HasComponent_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasComponent};
static const SOPC_NodeId HasProperty_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasProperty};

static const SOPC_NodeId HierarchicalReferences_Type_NodeId = {SOPC_IdentifierType_Numeric, 0,
                                                               .Data.Numeric = OpcUaId_HierarchicalReferences};

static bool is_type_or_subtype(const SOPC_NodeId* actualType, const SOPC_NodeId* expectedType)
{
    return SOPC_NodeId_Equal(actualType, expectedType) ||
           util_addspace__recursive_is_transitive_subtype(RECURSION_LIMIT, actualType, actualType, expectedType);
}

static bool check_variable_organizes_reference(OpcUa_NodeClass parentNodeClass, SOPC_StatusCode* scAddNode)
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
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "check_variable_organizes_reference: cannot add a Variable node with Organizes "
                               "reference from parent NodeClass %" PRIi32,
                               (int32_t) parentNodeClass);
        *scAddNode = OpcUa_BadReferenceNotAllowed;
        return false;
    }
}

static bool check_variable_has_component_reference(SOPC_AddressSpace* addSpace,
                                                   SOPC_AddressSpace_Node* parentNode,
                                                   OpcUa_NodeClass parentNodeClass,
                                                   const SOPC_NodeId* typeDefId,
                                                   SOPC_StatusCode* scAddNode)
{
    char* parentNodeIdStr = NULL;
    char* typeNodeIdStr = NULL;
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
        parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
        typeNodeIdStr = SOPC_NodeId_ToCString(typeDefId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "check_variable_has_component_reference: cannot add a Variable node which has not a "
                               "DataVariable type: %s with HasComponent reference from parent %s",
                               typeNodeIdStr, parentNodeIdStr);
        SOPC_Free(parentNodeIdStr);
        SOPC_Free(typeNodeIdStr);

        *scAddNode = OpcUa_BadTypeDefinitionInvalid;
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
            parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
            typeNodeIdStr = SOPC_NodeId_ToCString(&parentTypeDef->NodeId);
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "check_variable_has_component_reference: cannot add a Variable node with HasComponent reference from "
                "Variable parent node %s which has not a DataVariable type: %s",
                parentNodeIdStr, typeNodeIdStr);
            SOPC_Free(parentNodeIdStr);
            SOPC_Free(typeNodeIdStr);

            *scAddNode = OpcUa_BadReferenceNotAllowed;
            return false;
        }
        break;
    default:
        parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "check_variable_has_component_reference: cannot add a Variable node with HasComponent "
                               "reference from parent %s with NodeClass %" PRIi32,
                               parentNodeIdStr, (int32_t) parentNodeClass);
        SOPC_Free(parentNodeIdStr);

        *scAddNode = OpcUa_BadReferenceNotAllowed;
        return false;
    }
    return true;
}

static bool check_variable_has_property_reference(SOPC_AddressSpace* addSpace,
                                                  SOPC_AddressSpace_Node* parentNode,
                                                  OpcUa_NodeClass parentNodeClass,
                                                  const SOPC_NodeId* typeDefId,
                                                  SOPC_StatusCode* scAddNode)
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
            char* parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "check_variable_has_property_reference: cannot add a Variable node with HasProperty "
                                   "reference from parent %s with NodeClass %" PRIi32,
                                   parentNodeIdStr, (int32_t) parentNodeClass);
            SOPC_Free(parentNodeIdStr);

            *scAddNode = OpcUa_BadReferenceNotAllowed;
            return false;
        }

        // Target Variable is a property
        bool isPropertyVariable = is_type_or_subtype(typeDefId, &Property_Type);
        if (!isPropertyVariable)
        {
            char* typeDefIdStr = SOPC_NodeId_ToCString(typeDefId);
            char* parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "check_variable_has_property_reference: cannot add a Variable node with HasProperty "
                                   "reference which has not TypeDefintion=Property (%s) with parent %s ",
                                   typeDefIdStr, parentNodeIdStr);
            SOPC_Free(typeDefIdStr);
            SOPC_Free(parentNodeIdStr);

            *scAddNode = OpcUa_BadReferenceNotAllowed;
            return false;
        }
    }
    return true;
}

static bool check_variable_reference_type_to_parent(SOPC_AddressSpace* addSpace,
                                                    SOPC_AddressSpace_Node* parentNode,
                                                    OpcUa_NodeClass parentNodeClass,
                                                    const SOPC_NodeId* referenceTypeId,
                                                    const SOPC_NodeId* typeDefId,
                                                    SOPC_StatusCode* scAddNode)
{
    bool validRef = false;
    bool identifiedRefType = false;

    // Evaluate Organizes reference type from parent to this variable node
    bool isOrganizesRef = is_type_or_subtype(referenceTypeId, &Organizes_Type);
    if (isOrganizesRef)
    {
        identifiedRefType = true;
        validRef = check_variable_organizes_reference(parentNodeClass, scAddNode);
    }

    if (!identifiedRefType)
    {
        // Evaluate HasComponent reference type from parent to this variable node
        bool isHasComponentRef = is_type_or_subtype(referenceTypeId, &HasComponent_Type);

        if (isHasComponentRef)
        {
            validRef =
                check_variable_has_component_reference(addSpace, parentNode, parentNodeClass, typeDefId, scAddNode);
        }
    }

    if (!identifiedRefType)
    {
        // Evaluate HasProperty reference type from parent to this variable node
        bool isHasPropertyRef = is_type_or_subtype(referenceTypeId, &HasProperty_Type);
        if (isHasPropertyRef)
        {
            validRef =
                check_variable_has_property_reference(addSpace, parentNode, parentNodeClass, typeDefId, scAddNode);
        }
    }

    if (!identifiedRefType)
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
            char* referenceTypeIdStr = SOPC_NodeId_ToCString(referenceTypeId);
            char* parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "check_variable_reference_type_to_parent: cannot add a Variable node with "
                                   "ReferenceType %s from parent %s with NodeClass=%" PRIi32,
                                   referenceTypeIdStr, parentNodeIdStr, (int32_t) parentNodeClass);
            SOPC_Free(referenceTypeIdStr);
            SOPC_Free(parentNodeIdStr);

            *scAddNode = OpcUa_BadReferenceNotAllowed;
        }
    }

    return validRef;
}

static bool check_browse_name_unique_from_parent(SOPC_AddressSpace* addSpace,
                                                 SOPC_AddressSpace_Node* parentNode,
                                                 const SOPC_QualifiedName* browseName,
                                                 SOPC_StatusCode* scAddNode)
{
    /* Simplified version of the BrowseName uniqueness check:
     * - apply for all whereas it is only indicated for Properties and TypeDefinition / InstanceDeclaration.
     *   (but AddNodes service status code indicates: The browse name is not unique among nodes that share
     *    the same relationship with the parent.)
     * - check for any hierarchical reference type whereas it might be acceptable
     *   (but part 3 (1.05) §4.5.4 indicates: A TypeDefinitionNode or an InstanceDeclaration
     *    shall never reference two Nodes having the same BrowseName using forward hierarchical References.)
     */

    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(addSpace, parentNode);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(addSpace, parentNode);
    int32_t comparison = -1;
    bool found = false;

    for (int32_t i = 0; i < *n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &(*refs)[i];

        if (!ref->IsInverse &&
            util_addspace__recursive_is_transitive_subtype(RECURSION_LIMIT, &ref->ReferenceTypeId,
                                                           &ref->ReferenceTypeId, &HierarchicalReferences_Type_NodeId))
        {
            // Note: NamespaceUri should not be interpreted as external server but is not managed for now
            if (0 == ref->TargetId.ServerIndex && ref->TargetId.NamespaceUri.Length <= 0)
            {
                SOPC_AddressSpace_Node* node = SOPC_AddressSpace_Get_Node(addSpace, &ref->TargetId.NodeId, &found);
                if (found)
                {
                    SOPC_QualifiedName* otherBrowseName = SOPC_AddressSpace_Get_BrowseName(addSpace, node);
                    SOPC_ReturnStatus status = SOPC_QualifiedName_Compare(browseName, otherBrowseName, &comparison);
                    SOPC_ASSERT(SOPC_STATUS_OK == status);
                    if (0 == comparison)
                    {
                        char* parentNodeIdStr =
                            SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                               "check_browse_name_unique_from_parent: cannot add a Variable node with "
                                               "duplicated BrowseName %s from parent %s",
                                               SOPC_String_GetRawCString(&browseName->Name), parentNodeIdStr);
                        SOPC_Free(parentNodeIdStr);

                        *scAddNode = OpcUa_BadBrowseNameDuplicated;
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

SOPC_StatusCode SOPC_NodeMgtHelperInternal_CheckConstraints_AddVariable(SOPC_AddressSpace* addSpace,
                                                                        const SOPC_ExpandedNodeId* parentNid,
                                                                        const SOPC_NodeId* refTypeId,
                                                                        const SOPC_QualifiedName* browseName,
                                                                        const SOPC_ExpandedNodeId* typeDefId)
{
    SOPC_StatusCode retCode = OpcUa_BadUnexpectedError;

    if (parentNid->ServerIndex != 0)
    {
        // We do not manage out of server parent
        return OpcUa_BadParentNodeIdInvalid;
    }
    bool found = false;
    SOPC_AddressSpace_Node* parentNode = SOPC_AddressSpace_Get_Node(addSpace, &parentNid->NodeId, &found);
    if (!found)
    {
        // We do not manage out of server parent
        return OpcUa_BadParentNodeIdInvalid;
    }
    SOPC_ASSERT(NULL != parentNode);
    OpcUa_NodeClass* parentNodeClass = SOPC_AddressSpace_Get_NodeClass(addSpace, parentNode);
    SOPC_ASSERT(NULL != parentNodeClass);

    if (typeDefId->ServerIndex != 0)
    {
        // We do not manage out of server type
        return OpcUa_BadTypeDefinitionInvalid;
    }
    SOPC_AddressSpace_Node* typeDefNode = SOPC_AddressSpace_Get_Node(addSpace, &typeDefId->NodeId, &found);
    if (!found)
    {
        // We do not manage out of server type
        return OpcUa_BadTypeDefinitionInvalid;
    }
    SOPC_ASSERT(NULL != typeDefNode);

    bool refTypeOk = check_variable_reference_type_to_parent(addSpace, parentNode, *parentNodeClass, refTypeId,
                                                             &typeDefId->NodeId, &retCode);
    if (!refTypeOk)
    {
        return retCode;
    }

    // Check type definition is VariableType node class
    OpcUa_NodeClass* typeDefNodeClass = SOPC_AddressSpace_Get_NodeClass(addSpace, typeDefNode);
    SOPC_ASSERT(NULL != typeDefNodeClass);
    if (OpcUa_NodeClass_VariableType != *typeDefNodeClass)
    {
        char* typeDefIdStr = SOPC_NodeId_ToCString(&typeDefId->NodeId);
        char* parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "SOPC_NodeMgtHelperInternal_CheckConstraints_AddVariable: cannot add a Variable node "
                               "with a TypeDefinition %s which is not of VariableType NodeClass (%" PRIi32
                               ") from parent %s",
                               typeDefIdStr, (int32_t) *typeDefNodeClass, parentNodeIdStr);
        SOPC_Free(typeDefIdStr);
        SOPC_Free(parentNodeIdStr);

        retCode = OpcUa_BadTypeDefinitionInvalid;
        return retCode;
    }

    bool browseNameOk = check_browse_name_unique_from_parent(addSpace, parentNode, browseName, &retCode);
    if (browseNameOk)
    {
        retCode = SOPC_GoodGenericStatus;
    }
    return retCode;
}

// Note: to make this function generic for a SOPC_AddressSpace_Node we need to have setters for attributes
static SOPC_ReturnStatus util_AddCommonNodeAttributes(OpcUa_Node* node,
                                                      const OpcUa_NodeAttributes* commonNodeAttributes,
                                                      SOPC_StatusCode* scAddNode)
{
    SOPC_ASSERT(NULL != scAddNode);
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
    SOPC_ASSERT(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (commonNodeAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_Description))
        {
            status = SOPC_LocalizedText_Copy(&node->Description, &commonNodeAttributes->Description);
        } // else: no description
    }
    SOPC_ASSERT(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (commonNodeAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_WriteMask) ||
            commonNodeAttributes->WriteMask != 0)
        {
            char* nodeIdStr = SOPC_NodeId_ToCString(&node->NodeId);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "util_AddCommonNodeAttributes: cannot add Variable node %s with WriteMask attribute "
                                   "value since it is not supported",
                                   nodeIdStr);
            SOPC_Free(nodeIdStr);

            // Note: server does not manage write masks, it is not possible to specify some
            *scAddNode = OpcUa_BadNodeAttributesInvalid;
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (commonNodeAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_UserWriteMask) ||
            commonNodeAttributes->UserWriteMask != 0)
        {
            char* nodeIdStr = SOPC_NodeId_ToCString(&node->NodeId);
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "util_AddCommonNodeAttributes: cannot add Variable node %s with UserWriteMask attribute value "
                "since it is not supported",
                nodeIdStr);
            SOPC_Free(nodeIdStr);

            // Note: server does not manage write masks, it is not possible to specify some
            *scAddNode = OpcUa_BadNodeAttributesInvalid;
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_NodeMgtHelperInternal_AddVariableNodeAttributes(SOPC_AddressSpace* addSpace,
                                                                       SOPC_AddressSpace_Node* node,
                                                                       OpcUa_VariableNode* varNode,
                                                                       const OpcUa_VariableAttributes* varAttributes,
                                                                       SOPC_StatusCode* scAddNode)
{
    SOPC_ASSERT(NULL != scAddNode);

    SOPC_ASSERT(NULL != varNode);
    SOPC_ASSERT(NULL != varAttributes);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Common fields have same offsets in OpcUa_Node and OpcUa_VariableNode
    // Common attributes have same offsets in OpcUa_NodeAttributes and OpcUa_VariableAttributes
    status =
        util_AddCommonNodeAttributes((OpcUa_Node*) varNode, (const OpcUa_NodeAttributes*) varAttributes, scAddNode);
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_AccessLevel))
        {
            varNode->AccessLevel = varAttributes->AccessLevel;
        }
        else
        {
            // Allow read access
            varNode->AccessLevel = SOPC_AccessLevelMask_CurrentRead;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_UserAccessLevel) ||
            (varAttributes->UserAccessLevel != 0 && varAttributes->UserAccessLevel != varAttributes->AccessLevel))
        {
            char* nodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, node));
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "SOPC_NodeMgtHelperInternal_AddVariableNodeAttributes: cannot add Variable node %s "
                "with UserAccessLevel attribute value since it is specific to each user (managed by application)",
                nodeIdStr);
            SOPC_Free(nodeIdStr);

            // Note: server does not manage to set user access level this way
            *scAddNode = OpcUa_BadNodeAttributesInvalid;
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_ArrayDimensions))
        {
            if (varAttributes->NoOfArrayDimensions > 0 && NULL != varAttributes->ArrayDimensions)
            {
                varNode->NoOfArrayDimensions = varAttributes->NoOfArrayDimensions;
                varNode->ArrayDimensions = varAttributes->ArrayDimensions;
            }
            else if (varAttributes->NoOfArrayDimensions > 0)
            {
                char* nodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, node));
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "SOPC_NodeMgtHelperInternal_AddVariableNodeAttributes: cannot add Variable node "
                    "%s since NoOfarraysDimensions and ArrayDimensions attributes values are not coherent",
                    nodeIdStr);
                SOPC_Free(nodeIdStr);

                // Incoherent parameters
                *scAddNode = OpcUa_BadNodeAttributesInvalid;
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        } // else: remains 0
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_DataType))
        {
            status = SOPC_NodeId_Copy(&varNode->DataType, &varAttributes->DataType);
            if (SOPC_STATUS_OK != status)
            {
                *scAddNode = OpcUa_BadOutOfMemory;
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
        if (0 != (varAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_Historizing) &&
            varAttributes->Historizing)
        {
            char* nodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, node));
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "SOPC_NodeMgtHelperInternal_AddVariableNodeAttributes: cannot add Variable node %s "
                                   "with Historizing attribute value since it is not supported",
                                   nodeIdStr);
            SOPC_Free(nodeIdStr);

            // Note: server does not manage to set historizing
            *scAddNode = OpcUa_BadNodeAttributesInvalid;
            status = SOPC_STATUS_INVALID_PARAMETERS;
        } // else: remains false
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_MinimumSamplingInterval))
        {
            char* nodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, node));
            SOPC_Logger_TraceWarning(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "SOPC_NodeMgtHelperInternal_AddVariableNodeAttributes: add Variable node %s but "
                "ignoring MinimumSamplingInterval attribute value %lf since server only supports 0",
                nodeIdStr, varAttributes->MinimumSamplingInterval);
            SOPC_Free(nodeIdStr);

            // remains 0
        } // else: remains 0
    }
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (varAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_Value))
        {
            status = SOPC_Variant_Copy(&varNode->Value, &varAttributes->Value);
            if (SOPC_STATUS_OK != status)
            {
                *scAddNode = OpcUa_BadOutOfMemory;
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
        if (0 != (varAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_ValueRank))
        {
            varNode->ValueRank = varAttributes->ValueRank;
        }
        else
        {
            // Any value rank
            varNode->ValueRank = -2;
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_NodeMgtHelperInternal_AddRefChildToParentNode(SOPC_AddressSpace* addSpace,
                                                                     const SOPC_NodeId* parentNodeId,
                                                                     const SOPC_NodeId* childNodeId,
                                                                     const SOPC_NodeId* refTypeId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    bool found = false;
    SOPC_AddressSpace_Node* parentNode = SOPC_AddressSpace_Get_Node(addSpace, parentNodeId, &found);
    SOPC_ASSERT(found && NULL != parentNode);
    int32_t* nbRefs = SOPC_AddressSpace_Get_NoOfReferences(addSpace, parentNode);
    SOPC_ASSERT(NULL != nbRefs);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(addSpace, parentNode);
    SOPC_ASSERT(NULL != refs);
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
            SOPC_ASSERT(to == newRefs);
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
                SOPC_ASSERT(SOPC_STATUS_OUT_OF_MEMORY == status);
            }
            if (SOPC_STATUS_OK != status)
            {
                SOPC_ASSERT(SOPC_STATUS_OUT_OF_MEMORY == status);
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

bool SOPC_NodeMgtHelperInternal_RemoveLastRefInParentNode(SOPC_AddressSpace* addSpace, const SOPC_NodeId* parentNodeId)
{
    // Rollback reference added in parent
    bool found = false;
    SOPC_AddressSpace_Node* parentNode = SOPC_AddressSpace_Get_Node(addSpace, parentNodeId, &found);
    SOPC_ASSERT(found && NULL != parentNode);
    int32_t* nbRefs = SOPC_AddressSpace_Get_NoOfReferences(addSpace, parentNode);
    SOPC_ASSERT(NULL != nbRefs);
    if (*nbRefs < 1)
    {
        return false;
    }
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(addSpace, parentNode);
    *nbRefs -= 1;
    OpcUa_ReferenceNode_Clear(&(*refs[*nbRefs]));
    return true;
}
