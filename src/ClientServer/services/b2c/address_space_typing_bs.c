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

#include <inttypes.h>

#include "address_space_impl.h"
#include "sopc_address_space.h"
#include "sopc_address_space_utils_internal.h"
#include "sopc_assert.h"
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

void address_space_typing_bs__is_transitive_subtype(const constants__t_NodeId_i address_space_typing_bs__p_subtype,
                                                    const constants__t_NodeId_i address_space_typing_bs__p_parent_type,
                                                    t_bool* const address_space_typing_bs__bres)
{
    *address_space_typing_bs__bres = SOPC_AddressSpaceUtil_RecursiveIsTransitiveSubtype(
        address_space_bs__nodes, RECURSION_LIMIT, address_space_typing_bs__p_subtype,
        address_space_typing_bs__p_subtype, address_space_typing_bs__p_parent_type);
}

static SOPC_NodeId Enumeration_Type = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Enumeration);

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
    bool res =
        SOPC_AddressSpaceUtil_RecursiveIsTransitiveSubtype(address_space_bs__nodes, 1, dataType, dataType, valueType);

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
    *address_space_typing_bs__bres =
        SOPC_AddressSpaceUtil_IsValidReferenceTypeId(address_space_bs__nodes, address_space_typing_bs__p_nodeId);
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
            type = SOPC_AddressSpaceUtil_GetTypeDefinition(address_space_bs__nodes, object);
            if (NULL != type && type->ServerIndex == 0 && type->NamespaceUri.Length <= 0)
            { // Shall be on same server and shall use only NodeId
                res = recursive_check_object_has_method(recursionLimit, &type->NodeId, methodId);
            }
            break;

        case OpcUa_NodeClass_ObjectType:
            res = recursive_check_object_has_method(
                recursionLimit, SOPC_AddressSpaceUtil_GetDirectParentType(address_space_bs__nodes, objectId), methodId);
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
    SOPC_ASSERT(NULL != address_space_typing_bs__p_object);
    SOPC_ASSERT(NULL != address_space_typing_bs__p_method);
    const SOPC_NodeId* object = address_space_typing_bs__p_object;
    const SOPC_NodeId* method = address_space_typing_bs__p_method;

    *address_space_typing_bs__p_bool = recursive_check_object_has_method(RECURSION_LIMIT, object, method);
}

static bool is_component_of(const SOPC_NodeId* component, const constants_bs__t_Node_i node)
{
    SOPC_ASSERT(NULL != node);
    SOPC_ASSERT(NULL != component);

    int32_t* n_refs = SOPC_AddressSpace_Get_NoOfReferences(address_space_bs__nodes, node);
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Get_References(address_space_bs__nodes, node);
    bool res = false;

    for (int32_t i = 0; i < *n_refs; ++i)
    {
        OpcUa_ReferenceNode* ref = &(*refs)[i];

        if (SOPC_AddressSpaceUtil_IsComponent(ref))
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
