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
 * Implements the address space nodes typing utilities
 */

#include "address_space_typing_bs.h"

#include <assert.h>
#include <inttypes.h>

#include "address_space_impl.h"
#include "util_address_space.h"

#include "sopc_address_space.h"
#include "sopc_embedded_nodeset2.h"
#include "sopc_logger.h"
#include "sopc_toolkit_config_constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_typing_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

static bool is_component_of(const SOPC_NodeId* component, const constants_bs__t_Node_i node);
static bool recursive_check_object_has_method(int recursionLimit, const SOPC_NodeId* object, const SOPC_NodeId* method);

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

static const SOPC_NodeId* get_direct_parent_of_node(const constants_bs__t_Node_i child)
{
    SOPC_NodeId* directParent = NULL;
    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, child);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, child);
    for (int32_t i = 0; i < *n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &(*refs)[i];

        if (util_addspace__is_reversed_has_child(ref))
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

static const SOPC_NodeId* get_direct_parent(const SOPC_NodeId* childNodeId)
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
    else if (SOPC_HAS_SUBTYPE_HYBRID_RESOLUTION)
    {
        // Parent not found in static array of extracted HasSubtype references, start research in address space

        void* node;
        bool node_found = false;

        node = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, childNodeId, &node_found);

        if (node_found)
        {
            // Starting to check if direct parent is researched parent
            result = get_direct_parent_of_node(node);
        }
    }
    return result;
}

#define RECURSION_LIMIT SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL

static bool recursive_is_transitive_subtype(int recursionLimit,
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
    const SOPC_NodeId* directParent = get_direct_parent(currentTypeOrSubtype);
    if (NULL != directParent)
    {
        if (SOPC_NodeId_Equal(directParent, expectedParentType))
        {
            return true;
        }
        else
        {
            return recursive_is_transitive_subtype(recursionLimit, originSubtype, directParent, expectedParentType);
        }
    } // else: transitive research failed

    return false;
}

void address_space_typing_bs__is_transitive_subtype(const constants__t_NodeId_i address_space_typing_bs__p_subtype,
                                                    const constants__t_NodeId_i address_space_typing_bs__p_parent_type,
                                                    t_bool* const address_space_typing_bs__bres)
{
    *address_space_typing_bs__bres =
        recursive_is_transitive_subtype(RECURSION_LIMIT, address_space_typing_bs__p_subtype,
                                        address_space_typing_bs__p_subtype, address_space_typing_bs__p_parent_type);
}

static SOPC_NodeId Enumeration_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 29};

void address_space_typing_bs__is_compatible_simple_type_or_enumeration(
    const constants__t_NodeId_i address_space_typing_bs__p_value_type,
    const constants__t_NodeId_i address_space_typing_bs__p_data_type,
    t_bool* const address_space_typing_bs__bres)
{
    SOPC_NodeId* const valueType = address_space_typing_bs__p_value_type;
    SOPC_NodeId* const dataType = address_space_typing_bs__p_data_type;

    /* Two possibilities for compatibility:
     * 1. DataType is a simple type which means it is encoded as a Built-In type
     * 2. DataType is an enumeration of enumeration subtype, it is encoded as a Int32
     *
     * Pre-requesite: the actual value type shall be a Built-In type (except Structure type (i=22))
     * Evaluation of compatibility:
     * Case 1.: DataType is direct subtype of the value type built-in type => compatible simple type
     * Case 2.: Value built-in type is Int32 and DataType is transitive subtype of Enumeration type (i=29)
     * */
    *address_space_typing_bs__bres = false;
    // Pre-requesite
    if (SOPC_IdentifierType_Numeric != valueType->IdentifierType || OPCUA_NAMESPACE_INDEX != valueType->Namespace ||
        valueType->Data.Numeric > SOPC_BUILTINID_MAX || SOPC_ExtensionObject_Id == valueType->Data.Numeric)
    {
        // Not a built-in type (or Structure built-in type)
        return;
    }

    // Case 1 evaluation
    bool res = recursive_is_transitive_subtype(1, dataType, dataType, valueType);

    // Case 2 evaluation
    if (!res && SOPC_Int32_Id == valueType->Data.Numeric)
    {
        if (SOPC_NodeId_Equal(&Enumeration_Type, dataType))
        {
            // DataType is the EnumarationType
            res = true;
        }
        else
        {
            // Check if DataType is a subtype of EnumerationType
            address_space_typing_bs__is_transitive_subtype(dataType, &Enumeration_Type, &res);
        }
    }

    *address_space_typing_bs__bres = res;
}

void address_space_typing_bs__is_valid_ReferenceTypeId(const constants__t_NodeId_i address_space_typing_bs__p_nodeId,
                                                       t_bool* const address_space_typing_bs__bres)
{
    const SOPC_NodeId* nodeId = address_space_typing_bs__p_nodeId;
    *address_space_typing_bs__bres = false;
    if (SOPC_IdentifierType_Numeric == nodeId->IdentifierType && OPCUA_NAMESPACE_INDEX == nodeId->Namespace &&
        nodeId->Data.Numeric <= SOPC_MAX_TYPE_INFO_NODE_ID)
    {
        // NodeId is in statically extracted type nodes info
        const SOPC_AddressSpaceTypeInfo* typeInfo = &SOPC_Embedded_HasSubTypeBackward[nodeId->Data.Numeric];
        *address_space_typing_bs__bres = OpcUa_NodeClass_ReferenceType == typeInfo->nodeClass;
    }
    else if (SOPC_HAS_SUBTYPE_HYBRID_RESOLUTION)
    {
        // NodeId not in static array of type nodes info, start research in address space
        bool node_found = false;
        SOPC_AddressSpace_Node* node = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, nodeId, &node_found);

        if (node_found)
        {
            // Check node has expected class
            *address_space_typing_bs__bres = OpcUa_NodeClass_ReferenceType == node->node_class;
        }
    }
}

static bool recursive_check_object_has_method(int recursionLimit,
                                              const SOPC_NodeId* objectId,
                                              const SOPC_NodeId* methodId)
{
    recursionLimit--;
    if (recursionLimit < 0)
    {
        return false;
    }

    if (NULL == objectId || NULL == methodId)
    {
        return false;
    }

    bool res;
    constants__t_ExpandedNodeId_i type;

    SOPC_AddressSpace_Node* object = SOPC_AddressSpace_Get_Node(address_space_bs__nodes, objectId, &res);

    if (!res)
    {
        return false;
    }

    /* Try direct reference */
    res = is_component_of(methodId, object); /* Sub type of hasComponent is not managed */
    if (!res)
    {
        /* Object has not a direct reference with the method.
           Try with its type or supertype */
        switch (object->node_class)
        {
        case OpcUa_NodeClass_Object:
            util_addspace__get_TypeDefinition(object, &type);
            if (NULL != type && type->ServerIndex == 0 && type->NamespaceUri.Length <= 0)
            { // Shall be on same server and shall use only NodeId
                res = recursive_check_object_has_method(recursionLimit, &type->NodeId, methodId);
            }
            break;

        case OpcUa_NodeClass_ObjectType:
            res = recursive_check_object_has_method(recursionLimit, get_direct_parent(objectId), methodId);
            break;

        default:
            return res = false;
        }
    }
    return res;
}

void address_space_typing_bs__check_object_has_method(const constants__t_NodeId_i address_space_typing_bs__p_object,
                                                      const constants__t_NodeId_i address_space_typing_bs__p_method,
                                                      t_bool* const address_space_typing_bs__p_bool)
{
    assert(NULL != address_space_typing_bs__p_object);
    assert(NULL != address_space_typing_bs__p_method);
    const SOPC_NodeId* object = address_space_typing_bs__p_object;
    const SOPC_NodeId* method = address_space_typing_bs__p_method;

    *address_space_typing_bs__p_bool = recursive_check_object_has_method(RECURSION_LIMIT, object, method);
}

static bool is_component_of(const SOPC_NodeId* component, const constants_bs__t_Node_i node)
{
    assert(NULL != node);
    assert(NULL != component);

    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, node);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, node);
    bool res = false;

    for (int32_t i = 0; i < *n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &(*refs)[i];

        if (util_addspace__is_component(ref))
        {
            if (ref->TargetId.ServerIndex == 0 && ref->TargetId.NamespaceUri.Length <= 0)
            { // Shall be on same server and shall use only NodeId
                if (SOPC_NodeId_Equal(&ref->TargetId.NodeId, component))
                {
                    res = true;
                    break;
                }
            }
        }
    }
    return res;
}
