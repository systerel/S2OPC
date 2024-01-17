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
#include <string.h>

#include "sopc_address_space_utils_internal.h"

#include "opcua_identifiers.h"
#include "sopc_assert.h"
#include "sopc_embedded_nodeset2.h"
#include "sopc_logger.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_types.h"

typedef bool SOPC_AddressSpaceUtil_IsExpectedRefCb(const OpcUa_ReferenceNode* ref);
typedef bool SOPC_AddressSpaceUtil_IsExpectedRefNodeCb(const SOPC_AddressSpace_Node* refNode);

static SOPC_ExpandedNodeId* SOPC_Internal_AddressSpaceUtil_GetReferencedNode(
    SOPC_AddressSpaceUtil_IsExpectedRefCb* refEvalCb,
    SOPC_AddressSpaceUtil_IsExpectedRefNodeCb* refNodeEvalCb, /* might be NULL if criteria is only on ref */
    SOPC_AddressSpace* addSpace,
    SOPC_AddressSpace_Node* node)
{
    SOPC_ASSERT(NULL != node);
    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(addSpace, node);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(addSpace, node);

    SOPC_AddressSpace_Node* refNode = NULL;
    bool found = false;

    for (int32_t i = 0; i < *n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &(*refs)[i];

        if ((*refEvalCb)(ref))
        {
            found = true;
            if (NULL != *refNodeEvalCb)
            {
                if (ref->TargetId.NamespaceUri.Length <= 0 && 0 == ref->TargetId.ServerIndex)
                {
                    refNode = SOPC_AddressSpace_Get_Node(addSpace, &ref->TargetId.NodeId, &found);
                    if (found)
                    {
                        found = (*refNodeEvalCb)(refNode);
                    }
                }
            }
            if (found)
            {
                return &ref->TargetId;
            }
        }
    }
    return NULL;
}

SOPC_ExpandedNodeId* SOPC_AddressSpaceUtil_GetTypeDefinition(SOPC_AddressSpace* addSpace, SOPC_AddressSpace_Node* node)
{
    return SOPC_Internal_AddressSpaceUtil_GetReferencedNode(&SOPC_AddressSpaceUtil_IsTypeDefinition, NULL, addSpace,
                                                            node);
}

bool SOPC_AddressSpaceUtil_IsComponent(const OpcUa_ReferenceNode* ref)
{
    if (ref->IsInverse)
    {
        return false;
    }

    return SOPC_IdentifierType_Numeric == ref->ReferenceTypeId.IdentifierType &&
           OpcUaId_HasComponent == ref->ReferenceTypeId.Data.Numeric;
}

bool SOPC_AddressSpaceUtil_IsTypeDefinition(const OpcUa_ReferenceNode* ref)
{
    if (ref->IsInverse)
    {
        return false;
    }

    return SOPC_IdentifierType_Numeric == ref->ReferenceTypeId.IdentifierType &&
           OpcUaId_HasTypeDefinition == ref->ReferenceTypeId.Data.Numeric;
}

bool SOPC_AddressSpaceUtil_IsProperty(const OpcUa_ReferenceNode* ref)
{
    if (ref->IsInverse)
    {
        return false;
    }

    return SOPC_IdentifierType_Numeric == ref->ReferenceTypeId.IdentifierType &&
           OpcUaId_HasProperty == ref->ReferenceTypeId.Data.Numeric;
}

bool SOPC_AddressSpaceUtil_IsReversedHasChild(const OpcUa_ReferenceNode* ref)
{
    if (false == ref->IsInverse)
    {
        return false;
    }

    return ref->ReferenceTypeId.Namespace == OPCUA_NAMESPACE_INDEX &&
           ref->ReferenceTypeId.IdentifierType == SOPC_IdentifierType_Numeric &&
           ref->ReferenceTypeId.Data.Numeric == OpcUaId_HasSubtype;
}

static bool SOPC_AddressSpaceUtil_IsEncodingOf(const OpcUa_ReferenceNode* ref)
{
    if (!ref->IsInverse)
    {
        return false;
    }

    return SOPC_IdentifierType_Numeric == ref->ReferenceTypeId.IdentifierType &&
           OpcUaId_HasEncoding == ref->ReferenceTypeId.Data.Numeric;
}

static bool SOPC_AddressSpaceUtil_IsHasEncoding(const OpcUa_ReferenceNode* ref)
{
    if (ref->IsInverse)
    {
        return false;
    }

    return SOPC_IdentifierType_Numeric == ref->ReferenceTypeId.IdentifierType &&
           OpcUaId_HasEncoding == ref->ReferenceTypeId.Data.Numeric;
}

static bool SOPC_AddressSpaceUtil_IsDefaultBinaryNode(const SOPC_AddressSpace_Node* node)
{
    if (OpcUa_NodeClass_Object == node->node_class)
    {
        const SOPC_QualifiedName* bn = &node->data.object.BrowseName;
        if (OPCUA_NAMESPACE_INDEX == bn->NamespaceIndex && bn->Name.Length > 0 &&
            0 == strcmp("Default Binary", SOPC_String_GetRawCString(&bn->Name)))
        {
            return true;
        }
    }
    return false;
}

static void log_error_for_unknown_node(const SOPC_NodeId* nodeId, const char* node_adjective, const char* error)
{
    if (nodeId->IdentifierType == SOPC_IdentifierType_Numeric)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "address_space_typing_bs__is_transitive_child: %s, %s node: ns=%" PRIu16 ";i=%" PRIu32,
                               error, node_adjective, nodeId->Namespace, nodeId->Data.Numeric);
    }
    else if (nodeId->IdentifierType == SOPC_IdentifierType_String)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "address_space_typing_bs__is_transitive_child: %s, %s node: ns=%" PRIu16 ";s=%s", error,
                               node_adjective, nodeId->Namespace, SOPC_String_GetRawCString(&nodeId->Data.String));
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "address_space_typing_bs__is_transitive_child: %s node: %s", node_adjective, error);
    }
}

const SOPC_NodeId* SOPC_AddressSpaceUtil_GetDirectParentTypeOfNode(SOPC_AddressSpace* addSpace,
                                                                   SOPC_AddressSpace_Node* child)
{
    SOPC_NodeId* directParent = NULL;
    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(addSpace, child);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(addSpace, child);
    for (int32_t i = 0; i < *n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &(*refs)[i];

        if (SOPC_AddressSpaceUtil_IsReversedHasChild(ref))
        {
            if (ref->TargetId.ServerIndex == 0 && ref->TargetId.NamespaceUri.Length <= 0)
            { // Shall be on same server and shall use only NodeId
                directParent = &ref->TargetId.NodeId;
                break;
            }
            else
            {
                log_error_for_unknown_node(&ref->TargetId.NodeId, "direct parent", "is out of server");
            }
        }
    }
    return directParent;
}

const SOPC_NodeId* SOPC_AddressSpaceUtil_GetDirectParentType(SOPC_AddressSpace* addSpace,
                                                             const SOPC_NodeId* childNodeId)
{
    const SOPC_NodeId* result = NULL;

    if (SOPC_IdentifierType_Numeric == childNodeId->IdentifierType && OPCUA_NAMESPACE_INDEX == childNodeId->Namespace &&
        childNodeId->Data.Numeric <= SOPC_MAX_TYPE_INFO_NODE_ID)
    {
        const SOPC_AddressSpaceTypeInfo* typeInfo = &SOPC_Embedded_HasSubTypeBackward[childNodeId->Data.Numeric];
        if (typeInfo->hasSubtype)
        {
            result = &typeInfo->subtypeNodeId;
        }
    }
    else if (S2OPC_DYNAMIC_TYPE_RESOLUTION)
    {
        // Parent not found in static array of extracted HasSubtype references, start research in address space

        void* node;
        bool node_found = false;

        node = SOPC_AddressSpace_Get_Node(addSpace, childNodeId, &node_found);

        if (node_found)
        {
            // Starting to check if direct parent is researched parent
            result = SOPC_AddressSpaceUtil_GetDirectParentTypeOfNode(addSpace, node);
        }
    }
    return result;
}

bool SOPC_AddressSpaceUtil_RecursiveIsTransitiveSubtype(SOPC_AddressSpace* addSpace,
                                                        int recursionLimit,
                                                        const SOPC_NodeId* originSubtype,
                                                        const SOPC_NodeId* currentTypeOrSubtype,
                                                        const SOPC_NodeId* expectedParentType)
{
    recursionLimit--;
    if (recursionLimit < 0)
    {
        return false;
    }

    // Starting to check if direct parent is researched parent
    const SOPC_NodeId* directParent = SOPC_AddressSpaceUtil_GetDirectParentType(addSpace, currentTypeOrSubtype);
    if (NULL != directParent)
    {
        if (SOPC_NodeId_Equal(directParent, expectedParentType))
        {
            return true;
        }
        else
        {
            return SOPC_AddressSpaceUtil_RecursiveIsTransitiveSubtype(addSpace, recursionLimit, originSubtype,
                                                                      directParent, expectedParentType);
        }
    } // else: transitive research failed

    return false;
}

bool SOPC_AddressSpaceUtil_IsValidReferenceTypeId(SOPC_AddressSpace* addSpace, const SOPC_NodeId* nodeId)
{
    bool result = false;
    if (SOPC_IdentifierType_Numeric == nodeId->IdentifierType && OPCUA_NAMESPACE_INDEX == nodeId->Namespace &&
        nodeId->Data.Numeric <= SOPC_MAX_TYPE_INFO_NODE_ID)
    {
        // NodeId is in statically extracted type nodes info
        const SOPC_AddressSpaceTypeInfo* typeInfo = &SOPC_Embedded_HasSubTypeBackward[nodeId->Data.Numeric];
        result = OpcUa_NodeClass_ReferenceType == typeInfo->nodeClass;
    }
    else
    {
        // NodeId not in static array of type nodes info, start research in address space
        bool node_found = false;
        SOPC_AddressSpace_Node* node = SOPC_AddressSpace_Get_Node(addSpace, nodeId, &node_found);

        if (node_found)
        {
            // Check node has expected class
            result = OpcUa_NodeClass_ReferenceType == node->node_class;
        }
    }
    return result;
}

const SOPC_NodeId* SOPC_AddressSpaceUtil_GetEncodingDataType(SOPC_AddressSpace* addSpace,
                                                             const SOPC_NodeId* encodingNodeId)
{
    const SOPC_NodeId* result = NULL;

    SOPC_AddressSpace_Node* node;
    bool node_found = false;

    node = SOPC_AddressSpace_Get_Node(addSpace, encodingNodeId, &node_found);

    if (node_found)
    {
        if (OpcUa_NodeClass_DataType == node->node_class)
        {
            result = encodingNodeId;
        }
        else if (OpcUa_NodeClass_Object == node->node_class)
        {
            SOPC_ExpandedNodeId* expNodeId = SOPC_Internal_AddressSpaceUtil_GetReferencedNode(
                &SOPC_AddressSpaceUtil_IsEncodingOf, NULL, addSpace, node);
            if (NULL != expNodeId)
            {
                if (expNodeId->NamespaceUri.Length <= 0)
                {
                    // We do not need to check if this is a DataType since it is mandatory for this reference
                    result = &expNodeId->NodeId;
                }
            }
        }
    }

    return result;
}

const SOPC_NodeId* SOPC_AddressSpaceUtil_GetDataTypeDefaultBinaryEncoding(SOPC_AddressSpace* addSpace,
                                                                          const SOPC_NodeId* dataTypeId)
{
    const SOPC_NodeId* result = NULL;

    SOPC_AddressSpace_Node* node;
    bool node_found = false;

    node = SOPC_AddressSpace_Get_Node(addSpace, dataTypeId, &node_found);

    if (node_found)
    {
        if (OpcUa_NodeClass_DataType == node->node_class)
        {
            SOPC_ExpandedNodeId* expNodeId = SOPC_Internal_AddressSpaceUtil_GetReferencedNode(
                &SOPC_AddressSpaceUtil_IsHasEncoding, &SOPC_AddressSpaceUtil_IsDefaultBinaryNode, addSpace, node);
            if (NULL != expNodeId)
            {
                if (expNodeId->NamespaceUri.Length <= 0)
                {
                    // We do not need to check if this is a DataType since it is mandatory for this reference
                    result = &expNodeId->NodeId;
                }
            }
        }
    }

    return result;
}
