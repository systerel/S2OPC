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

#include "constants.h"

#include "sopc_types.h"
#include "util_address_space.h"

#include "address_space_impl.h"
#include "opcua_identifiers.h"

void util_addspace__get_TypeDefinition(const constants__t_Node_i address_space_bs__p_node,
                                       constants__t_ExpandedNodeId_i* const address_space_bs__p_type_def)
{
    assert(NULL != address_space_bs__p_node);
    SOPC_AddressSpace_Node* node = address_space_bs__p_node;
    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, node);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, node);
    *address_space_bs__p_type_def = constants__c_ExpandedNodeId_indet;

    for (int32_t i = 0; i < *n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &(*refs)[i];

        if (util_addspace__is_type_definition(ref))
        {
            *address_space_bs__p_type_def = &ref->TargetId;
            break;
        }
    }
}

bool util_addspace__is_component(const OpcUa_ReferenceNode* ref)
{
    if (ref->IsInverse)
    {
        return false;
    }

    return SOPC_IdentifierType_Numeric == ref->ReferenceTypeId.IdentifierType &&
           OpcUaId_HasComponent == ref->ReferenceTypeId.Data.Numeric;
}

bool util_addspace__is_type_definition(const OpcUa_ReferenceNode* ref)
{
    if (ref->IsInverse)
    {
        return false;
    }

    return SOPC_IdentifierType_Numeric == ref->ReferenceTypeId.IdentifierType &&
           OpcUaId_HasTypeDefinition == ref->ReferenceTypeId.Data.Numeric;
}

bool util_addspace__is_property(const OpcUa_ReferenceNode* ref)
{
    if (ref->IsInverse)
    {
        return false;
    }

    return SOPC_IdentifierType_Numeric == ref->ReferenceTypeId.IdentifierType &&
           OpcUaId_HasProperty == ref->ReferenceTypeId.Data.Numeric;
}

bool util_addspace__is_reversed_has_child(const OpcUa_ReferenceNode* ref)
{
    if (false == ref->IsInverse)
    {
        return false;
    }

    return ref->ReferenceTypeId.Namespace == OPCUA_NAMESPACE_INDEX &&
           ref->ReferenceTypeId.IdentifierType == SOPC_IdentifierType_Numeric &&
           ref->ReferenceTypeId.Data.Numeric == OpcUaId_HasSubtype;
}
