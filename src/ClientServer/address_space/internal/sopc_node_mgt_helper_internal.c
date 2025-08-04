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

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#include "sopc_node_mgt_helper_internal.h"

#include "sopc_address_space_utils_internal.h"

static const SOPC_NodeId DataVariable_Type = SOPC_NODEID_NS0_NUMERIC(OpcUaId_BaseDataVariableType);
static const SOPC_NodeId Property_Type = SOPC_NODEID_NS0_NUMERIC(OpcUaId_PropertyType);

static const SOPC_NodeId Organizes_Type = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Organizes);
static const SOPC_NodeId Aggregates_Type = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Aggregates);
static const SOPC_NodeId HasComponent_Type = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasComponent);
static const SOPC_NodeId HasProperty_Type = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasProperty);

static const SOPC_NodeId HierarchicalReferences_Type_NodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HierarchicalReferences);

static bool is_type_or_subtype(SOPC_AddressSpace* addSpace,
                               const SOPC_NodeId* actualType,
                               const SOPC_NodeId* expectedType)
{
    return SOPC_NodeId_Equal(actualType, expectedType) ||
           SOPC_AddressSpaceUtil_RecursiveIsTransitiveSubtype(addSpace, RECURSION_LIMIT, actualType, actualType,
                                                              expectedType);
}

static bool check_organizes_reference(OpcUa_NodeClass parentNodeClass, SOPC_StatusCode* scAddNode)
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
                               "check_organizes_reference: cannot add a Variable or Object node with Organizes "
                               "reference from parent NodeClass %" PRIi32,
                               (int32_t) parentNodeClass);
        *scAddNode = OpcUa_BadReferenceNotAllowed;
        return false;
    }
}

static bool check_node_has_component_reference(OpcUa_NodeClass targetNodeclass,
                                               SOPC_AddressSpace* addSpace,
                                               SOPC_AddressSpace_Node* parentNode,
                                               OpcUa_NodeClass parentNodeClass,
                                               const SOPC_NodeId* typeDefId,
                                               SOPC_StatusCode* scAddNode)
{
    char* typeNodeIdStr = NULL;
    SOPC_ExpandedNodeId* parentTypeDef = NULL;
    bool isParentNodeDataVariable = false;
    char* parentNodeIdStr = NULL;
    bool isDataVariable = false;

    switch (targetNodeclass)
    {
    case OpcUa_NodeClass_Object:
    case OpcUa_NodeClass_Method:
        /* §7.7 Part 3 (1.05): If the TargetNode is an Object or a Method, the SourceNode shall be an Object
         * or ObjectType.
         */
        switch (parentNodeClass)
        {
        case OpcUa_NodeClass_Object:
        case OpcUa_NodeClass_ObjectType:
            return true;
        default:
            parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "check_object_has_component_reference: cannot add an Object node with HasComponent "
                                   "reference from parent %s with NodeClass %" PRIi32,
                                   parentNodeIdStr, (int32_t) parentNodeClass);
            SOPC_Free(parentNodeIdStr);

            *scAddNode = OpcUa_BadReferenceNotAllowed;
            return false;
        }
        break;

    case OpcUa_NodeClass_Variable:
        /* §7.7 Part 3 (1.05): If the TargetNode is a Variable, the SourceNode shall be an Object, an ObjectType, a
         * DataVariable or a VariableType. By using the HasComponent Reference, the Variable is defined
         * as DataVariable.
         */
        // Target Variable is a DataVariable
        isDataVariable = is_type_or_subtype(addSpace, typeDefId, &DataVariable_Type);
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
            parentTypeDef = SOPC_AddressSpaceUtil_GetTypeDefinition(addSpace, parentNode);
            if (NULL != parentTypeDef)
            {
                isParentNodeDataVariable = is_type_or_subtype(addSpace, &parentTypeDef->NodeId, &DataVariable_Type);
            }
            if (!isParentNodeDataVariable)
            {
                parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
                typeNodeIdStr = SOPC_NodeId_ToCString(&parentTypeDef->NodeId);
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "check_variable_has_component_reference: cannot add a Variable node with "
                                       "HasComponent reference from "
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
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "check_variable_has_component_reference: cannot add a Variable node with HasComponent "
                "reference from parent %s with NodeClass %" PRIi32,
                parentNodeIdStr, (int32_t) parentNodeClass);
            SOPC_Free(parentNodeIdStr);

            *scAddNode = OpcUa_BadReferenceNotAllowed;
            return false;
        }
        break;
    default:
        break;
    }

    return true;
}

static bool check_node_has_property_reference(OpcUa_NodeClass targetNodeclass,
                                              SOPC_AddressSpace* addSpace,
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
    if (OpcUa_NodeClass_Variable != targetNodeclass)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "check_node_has_property_reference: cannot add anything other than Variable as "
                               "TargetNode with HasProperty reference");
        *scAddNode = OpcUa_BadReferenceNotAllowed;
        return false;
    }
    if (OpcUa_NodeClass_Variable == parentNodeClass)
    {
        // Source Variable is not a Property itself
        parentTypeDef = SOPC_AddressSpaceUtil_GetTypeDefinition(addSpace, parentNode);
        bool isParentNodeProperty = true;
        if (NULL != parentTypeDef)
        {
            isParentNodeProperty = is_type_or_subtype(addSpace, &parentTypeDef->NodeId, &Property_Type);
        }
        if (isParentNodeProperty)
        {
            char* parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "check_node_has_property_reference: cannot add a Variable node with HasProperty "
                                   "reference from parent %s with NodeClass %" PRIi32,
                                   parentNodeIdStr, (int32_t) parentNodeClass);
            SOPC_Free(parentNodeIdStr);

            *scAddNode = OpcUa_BadReferenceNotAllowed;
            return false;
        }

        // Target Variable is a property
        bool isPropertyVariable = is_type_or_subtype(addSpace, typeDefId, &Property_Type);
        if (!isPropertyVariable)
        {
            char* typeDefIdStr = SOPC_NodeId_ToCString(typeDefId);
            char* parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "check_node_has_property_reference: cannot add a Variable node with HasProperty "
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

static bool check_node_reference_type_to_parent(OpcUa_NodeClass targetNodeclass,
                                                SOPC_AddressSpace* addSpace,
                                                SOPC_AddressSpace_Node* parentNode,
                                                OpcUa_NodeClass parentNodeClass,
                                                const SOPC_NodeId* referenceTypeId,
                                                const SOPC_NodeId* typeDefId,
                                                SOPC_StatusCode* scAddNode)
{
    bool validRef = false;
    bool identifiedRefType = false;

    // Evaluate Organizes reference type from parent to this node
    bool isOrganizesRef = is_type_or_subtype(addSpace, referenceTypeId, &Organizes_Type);
    if (isOrganizesRef)
    {
        identifiedRefType = true;
        validRef = check_organizes_reference(parentNodeClass, scAddNode);
    }

    if (!identifiedRefType)
    {
        // Evaluate HasComponent reference type from parent to this node
        bool isHasComponentRef = is_type_or_subtype(addSpace, referenceTypeId, &HasComponent_Type);

        if (isHasComponentRef)
        {
            identifiedRefType = true;
            validRef = check_node_has_component_reference(targetNodeclass, addSpace, parentNode, parentNodeClass,
                                                          typeDefId, scAddNode);
        }
    }

    if (!identifiedRefType)
    {
        // Evaluate HasProperty reference type from parent to this node
        bool isHasPropertyRef = is_type_or_subtype(addSpace, referenceTypeId, &HasProperty_Type);
        if (isHasPropertyRef)
        {
            identifiedRefType = true;
            validRef = check_node_has_property_reference(targetNodeclass, addSpace, parentNode, parentNodeClass,
                                                         typeDefId, scAddNode);
        }
    }

    if (!identifiedRefType)
    {
        // Evaluate (unknown) Aggregates reference type from parent to this node
        bool isAggregatesRef = SOPC_AddressSpaceUtil_RecursiveIsTransitiveSubtype(
            addSpace, RECURSION_LIMIT, referenceTypeId, referenceTypeId, &Aggregates_Type);
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
                                   "check_node_reference_type_to_parent: cannot add a Variable or Object node with "
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

        if (!ref->IsInverse && SOPC_AddressSpaceUtil_RecursiveIsTransitiveSubtype(
                                   addSpace, RECURSION_LIMIT, &ref->ReferenceTypeId, &ref->ReferenceTypeId,
                                   &HierarchicalReferences_Type_NodeId))
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

SOPC_StatusCode SOPC_NodeMgtHelperInternal_CheckConstraints_AddNode(OpcUa_NodeClass targetNodeclass,
                                                                    SOPC_AddressSpace* addSpace,
                                                                    const SOPC_ExpandedNodeId* parentNid,
                                                                    const SOPC_NodeId* refToParentTypeId,
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

    // Check reference type to parent
    const SOPC_NodeId* typeDefNodeId = (OpcUa_NodeClass_Method == targetNodeclass) ? NULL : &typeDefId->NodeId;
    bool refTypeOk = check_node_reference_type_to_parent(targetNodeclass, addSpace, parentNode, *parentNodeClass,
                                                         refToParentTypeId, typeDefNodeId, &retCode);
    if (!refTypeOk)
    {
        return retCode;
    }

    // Check typeDefintion references for Object and Variable node classes.
    if (OpcUa_NodeClass_Object == targetNodeclass || OpcUa_NodeClass_Variable == targetNodeclass)
    {
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

        // Check type definition is VariableType/ObjectType node class
        OpcUa_NodeClass* typeDefNodeClass = SOPC_AddressSpace_Get_NodeClass(addSpace, typeDefNode);
        SOPC_ASSERT(NULL != typeDefNodeClass);
        if ((OpcUa_NodeClass_Variable == targetNodeclass && OpcUa_NodeClass_VariableType != *typeDefNodeClass) ||
            (OpcUa_NodeClass_Object == targetNodeclass && OpcUa_NodeClass_ObjectType != *typeDefNodeClass))
        {
            char* typeDefIdStr = SOPC_NodeId_ToCString(&typeDefId->NodeId);
            char* parentNodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, parentNode));
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "SOPC_NodeMgtHelperInternal_CheckConstraints_AddVariable: cannot add a node "
                "with a TypeDefinition %s which is not of %" PRIi32 " NodeClass (%" PRIi32 ") from parent %s",
                typeDefIdStr, (int32_t) targetNodeclass, (int32_t) *typeDefNodeClass, parentNodeIdStr);
            SOPC_Free(typeDefIdStr);
            SOPC_Free(parentNodeIdStr);

            retCode = OpcUa_BadTypeDefinitionInvalid;
            return retCode;
        }
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
            varNode->AccessLevel = 1; // bit 0 set
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
                varNode->ArrayDimensions = SOPC_Calloc((size_t) varAttributes->NoOfArrayDimensions, sizeof(uint32_t));
                status = SOPC_Copy_Array(varAttributes->NoOfArrayDimensions, (void*) varNode->ArrayDimensions,
                                         (const void*) varAttributes->ArrayDimensions, sizeof(uint32_t),
                                         &SOPC_UInt32_CopyAux);
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_Free(varNode->ArrayDimensions);
                    varNode->ArrayDimensions = NULL;
                    varNode->NoOfArrayDimensions = 0;
                }
            }
            else
            {
                status = NULL == varAttributes->ArrayDimensions && varAttributes->NoOfArrayDimensions > 0
                             ? SOPC_STATUS_INVALID_PARAMETERS
                             : SOPC_STATUS_OK;
            }
            if (SOPC_STATUS_OK != status)
            {
                char* nodeIdStr = SOPC_NodeId_ToCString(SOPC_AddressSpace_Get_NodeId(addSpace, node));
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "SOPC_NodeMgtHelperInternal_AddVariableNodeAttributes: cannot add Variable node "
                                       "%s: either NoOfarraysDimensions (%" PRIi32
                                       ") and ArrayDimensions (%s) attributes values are not "
                                       "coherent or OOM occurred",
                                       nodeIdStr, varAttributes->NoOfArrayDimensions,
                                       varAttributes->ArrayDimensions == NULL ? "NULL" : "not NULL");
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

SOPC_ReturnStatus SOPC_NodeMgtHelperInternal_AddObjectNodeAttributes(OpcUa_ObjectNode* objNode,
                                                                     const OpcUa_ObjectAttributes* objAttributes,
                                                                     SOPC_StatusCode* scAddNode)
{
    SOPC_ASSERT(NULL != scAddNode);
    SOPC_ASSERT(NULL != objNode);
    SOPC_ASSERT(NULL != objAttributes);

    // Common fields have same offsets in OpcUa_Node and OpcUa_VariableNode
    // Common attributes have same offsets in OpcUa_NodeAttributes and OpcUa_VariableAttributes
    SOPC_ReturnStatus status =
        util_AddCommonNodeAttributes((OpcUa_Node*) objNode, (const OpcUa_NodeAttributes*) objAttributes, scAddNode);

    // Mandatory attribute for Object: EventNotifier
    if (SOPC_STATUS_OK == status)
    {
        if (0 != (objAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_EventNotifier))
        {
            objNode->EventNotifier = objAttributes->EventNotifier;
        }
        else
        {
            objNode->EventNotifier = 0; // do not allow Events on the node by default
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_NodeMgtHelperInternal_AddMethodNodeAttributes(OpcUa_MethodNode* metNode,
                                                                     const OpcUa_MethodAttributes* metAttributes,
                                                                     SOPC_StatusCode* scAddNode)
{
    SOPC_ASSERT(NULL != metNode);
    SOPC_ASSERT(NULL != metAttributes);
    SOPC_ASSERT(NULL != scAddNode);

    // Common fields have same offsets in OpcUa_Node and OpcUa_VariableNode
    // Common attributes have same offsets in OpcUa_NodeAttributes and OpcUa_VariableAttributes
    SOPC_ReturnStatus status =
        util_AddCommonNodeAttributes((OpcUa_Node*) metNode, (const OpcUa_NodeAttributes*) metAttributes, scAddNode);

    // Mandatory attributes for Method: Executable, UserExecutable
    if (SOPC_STATUS_OK == status)
    {
        // Executable
        if (0 != (metAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_Executable))
        {
            metNode->Executable = metAttributes->Executable;
        }
        else
        {
            metNode->Executable = false; // do not allow Executable on the node by default
        }
        // UserExecutable
        if (0 != (metAttributes->SpecifiedAttributes & OpcUa_NodeAttributesMask_UserExecutable) ||
            (metAttributes->UserExecutable != false && metAttributes->UserExecutable != metAttributes->Executable))
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "SOPC_NodeMgtHelperInternal_AddVariableNodeAttributes: cannot add Method node"
                "with UserExecutable attribute value since it is specific to each user (managed by application)");

            // Note: server does not manage to set user access level this way
            *scAddNode = OpcUa_BadNodeAttributesInvalid;
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    return status;
}

SOPC_StatusCode SOPC_NodeMgtHelperInternal_CopyDataInNode(OpcUa_Node* node,
                                                          const SOPC_ExpandedNodeId* parentNodeId,
                                                          const SOPC_NodeId* newNodeId,
                                                          const SOPC_NodeId* refToParentTypeId,
                                                          const SOPC_QualifiedName* browseName,
                                                          const SOPC_ExpandedNodeId* typeDefId)
{
    // NodeID
    SOPC_ReturnStatus status = SOPC_NodeId_Copy(&node->NodeId, newNodeId);
    SOPC_ASSERT(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    // BrowseName
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_QualifiedName_Copy(&node->BrowseName, browseName);
        SOPC_ASSERT(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    }

    // References from new node (backward to parent and forward to type)
    if (SOPC_STATUS_OK == status)
    {
        int32_t nbOfRef = 1; // if OpcUa_NodeClass_Method, only 1 reference (backward to parent).
        if (OpcUa_NodeClass_Object == node->NodeClass || OpcUa_NodeClass_Variable == node->NodeClass)
        {
            nbOfRef = 2;
        }
        node->References = SOPC_Calloc((size_t) nbOfRef, sizeof(*node->References));
        if (NULL == node->References)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            node->NoOfReferences = nbOfRef;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        // Set hierarchical reference to parent
        OpcUa_ReferenceNode* hierarchicalRef = &node->References[0];
        hierarchicalRef->IsInverse = true;
        status = SOPC_NodeId_Copy(&hierarchicalRef->ReferenceTypeId, refToParentTypeId);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ExpandedNodeId_Copy(&hierarchicalRef->TargetId, parentNodeId);
        }
        SOPC_ASSERT(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    }
    if (SOPC_STATUS_OK == status && node->NoOfReferences == 2)
    {
        // Set HasTypeDefinition reference
        OpcUa_ReferenceNode* hasTypeDef = &node->References[1];
        hasTypeDef->IsInverse = false;
        hasTypeDef->ReferenceTypeId.Namespace = 0;
        hasTypeDef->ReferenceTypeId.IdentifierType = SOPC_IdentifierType_Numeric;
        hasTypeDef->ReferenceTypeId.Data.Numeric = OpcUaId_HasTypeDefinition;
        status = SOPC_ExpandedNodeId_Copy(&hasTypeDef->TargetId, typeDefId);
        SOPC_ASSERT(SOPC_STATUS_OK == status || SOPC_STATUS_OUT_OF_MEMORY == status);
    }

    return status;
}

SOPC_ReturnStatus SOPC_NodeMgtHelperInternal_AddRefToNode(SOPC_AddressSpace* addSpace,
                                                          const SOPC_NodeId* nodeId,
                                                          const SOPC_NodeId* refTargetNodeId,
                                                          const SOPC_NodeId* refTypeId,
                                                          const bool refIsInverse)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    bool found = false;
    SOPC_AddressSpace_Node* parentNode = SOPC_AddressSpace_Get_Node(addSpace, nodeId, &found);
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
            hierarchicalRef->IsInverse = refIsInverse;
            status = SOPC_NodeId_Copy(&hierarchicalRef->ReferenceTypeId, refTypeId);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_NodeId_Copy(&hierarchicalRef->TargetId.NodeId, refTargetNodeId);
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

bool SOPC_NodeMgtHelperInternal_RemoveLastRefInTargetNode(SOPC_AddressSpace* addSpace, const SOPC_NodeId* targetNodeId)
{
    // Rollback reference added in parent
    bool found = false;
    SOPC_AddressSpace_Node* parentNode = SOPC_AddressSpace_Get_Node(addSpace, targetNodeId, &found);
    SOPC_ASSERT(found && NULL != parentNode);
    int32_t* nbRefs = SOPC_AddressSpace_Get_NoOfReferences(addSpace, parentNode);
    SOPC_ASSERT(NULL != nbRefs);
    if (*nbRefs < 1)
    {
        return false;
    }
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(addSpace, parentNode);
    *nbRefs -= 1;
    OpcUa_ReferenceNode_Clear(&((*refs)[*nbRefs]));
    return true;
}

bool SOPC_NodeMgtHelperInternal_RemoveRefAtIndex(SOPC_AddressSpace* addSpace,
                                                 SOPC_AddressSpace_Node* node,
                                                 int32_t indexReference)
{
    SOPC_ASSERT(NULL != addSpace && NULL != node);
    int32_t* nbRefs = SOPC_AddressSpace_Get_NoOfReferences(addSpace, node);
    SOPC_ASSERT(NULL != nbRefs);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(addSpace, node);
    SOPC_ASSERT(NULL != refs);
    if (*nbRefs > 0 && (uint64_t) *nbRefs < SIZE_MAX)
    {
        SOPC_ASSERT(indexReference < *nbRefs);
        OpcUa_ReferenceNode* newRefs = SOPC_Calloc(((size_t) *nbRefs) - 1, sizeof(OpcUa_ReferenceNode));
        memcpy(newRefs, *refs, ((size_t) indexReference) * sizeof(OpcUa_ReferenceNode));
        memcpy(newRefs + (size_t) indexReference, *refs + (size_t) indexReference + 1,
               ((size_t)(*nbRefs - indexReference - 1)) * sizeof(OpcUa_ReferenceNode));
        OpcUa_ReferenceNode_Clear(&((*refs)[indexReference]));
        SOPC_Free(*refs);
        *refs = newRefs;
        (*nbRefs)--;
        return true;
    }
    return false;
}
