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

#include "sopc_address_space.h"
#include "sopc_logger.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_typing_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

static void log_error_for_unknown_node(const SOPC_NodeId* nodeId, const char* node_adjective, const char* error)
{
    if (nodeId->IdentifierType == SOPC_IdentifierType_Numeric)
    {
        SOPC_Logger_TraceError("address_space_typing_bs__is_transitive_child: %s node, error %s: ns=%" PRIu16
                               ";i=%" PRIu32,
                               node_adjective, error, nodeId->Namespace, nodeId->Data.Numeric);
    }
    else if (nodeId->IdentifierType == SOPC_IdentifierType_String)
    {
        SOPC_Logger_TraceError("address_space_typing_bs__is_transitive_child: %s node, error %s: ns=%" PRIu16 ";s=%s",
                               node_adjective, error, nodeId->Namespace,
                               SOPC_String_GetRawCString(&nodeId->Data.String));
    }
    else
    {
        SOPC_Logger_TraceError("address_space_typing_bs__is_transitive_child: %s node, error: %s", node_adjective,
                               error);
    }
}

static void log_info_for_unknown_node(const SOPC_NodeId* nodeId, const char* node_adjective, const char* info)
{
    if (nodeId->IdentifierType == SOPC_IdentifierType_Numeric)
    {
        SOPC_Logger_TraceInfo("address_space_typing_bs__is_transitive_child: %s node, info %s: ns=%" PRIu16
                              ";i=%" PRIu32,
                              node_adjective, info, nodeId->Namespace, nodeId->Data.Numeric);
    }
    else if (nodeId->IdentifierType == SOPC_IdentifierType_String)
    {
        SOPC_Logger_TraceInfo("address_space_typing_bs__is_transitive_child: %s node, info %s: ns=%" PRIu16 ";s=%s",
                              node_adjective, info, nodeId->Namespace, SOPC_String_GetRawCString(&nodeId->Data.String));
    }
    else
    {
        SOPC_Logger_TraceInfo("address_space_typing_bs__is_transitive_child: %s node, info: %s", node_adjective, info);
    }
}

static bool is_reversed_has_child(const OpcUa_ReferenceNode* ref)
{
    if (false == ref->IsInverse)
    {
        return false;
    }

    return ref->ReferenceTypeId.Namespace == OPCUA_NAMESPACE_INDEX &&
           ref->ReferenceTypeId.IdentifierType == SOPC_IdentifierType_Numeric &&
           ref->ReferenceTypeId.Data.Numeric == OpcUaId_HasSubtype;
}

static const SOPC_NodeId* get_direct_parent(const constants__t_Node_i child)
{
    SOPC_NodeId* directParent = NULL;
    int32_t* n_refs = SOPC_AddressSpace_Item_Get_NoOfReferences(child);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Item_Get_References(child);
    for (int32_t i = 0; i < *n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &(*refs)[i];

        if (is_reversed_has_child(ref))
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

#define RECURSION_LIMIT SOPC_MAX_VARIANT_NESTED_LEVEL

static bool recursive_is_transitive_subtype(int recursionLimit,
                                            const SOPC_NodeId* originSubtype,
                                            const SOPC_NodeId* currentTypeOrSubtype,
                                            const SOPC_NodeId* expectedParentType)
{
    recursionLimit--;
    if (recursionLimit <= 0)
    {
        return false;
    }

    void* node;
    bool node_found = false;

    node = SOPC_Dict_Get(address_space_bs__nodes, currentTypeOrSubtype, &node_found);

    if (node_found)
    {
        // Starting to check if direct parent is researched parent
        const SOPC_NodeId* directParent = get_direct_parent(node);
        if (NULL == directParent)
        {
            log_info_for_unknown_node(originSubtype, "root subtype", "while searching for parent");
            log_info_for_unknown_node(currentTypeOrSubtype, "transitive parent type", "has no more parent");
        }
        else
        {
            if (NULL != directParent)
            {
                if (SOPC_NodeId_Equal(directParent, expectedParentType))
                {
                    return true;
                }
                else
                {
                    return recursive_is_transitive_subtype(recursionLimit, originSubtype, directParent,
                                                           expectedParentType);
                }
            }
        }
    }
    else
    {
        log_error_for_unknown_node(originSubtype, "root subtype", "while searching for parent");
        log_error_for_unknown_node(currentTypeOrSubtype, "transitive parent type", "is not present in address space");
    }
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
