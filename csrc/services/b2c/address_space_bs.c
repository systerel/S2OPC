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

/** \file
 *
 * Implements the structures behind the address space.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "address_space_bs.h"
#include "b2c.h"

#include "address_space_impl.h"
#include "sopc_dict.h"
#include "util_b2c.h"
#include "util_variant.h"

bool sopc_addressSpace_configured = false;
SOPC_AddressSpace* address_space_bs__nodes = NULL;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_bs__INITIALISATION(void)
{
    if (sopc_addressSpace_configured)
    {
        assert(NULL != address_space_bs__nodes);
    }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/

void address_space_bs__UNINITIALISATION(void) {}

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

    val = SOPC_Dict_Get(address_space_bs__nodes, pnid_req, &val_found);

    if (val_found)
    {
        *address_space_bs__nid_valid = true;
        *address_space_bs__node = val;
    }
}

/* Reads any attribute and outputs a variant (valid or not)
 * As this function uses the *_2_Variant_i functions, the value must be freed once used
 */
void address_space_bs__read_AddressSpace_Attribute_value(const constants__t_Node_i address_space_bs__node,
                                                         const constants__t_NodeClass_i address_space_bs__ncl,
                                                         const constants__t_AttributeId_i address_space_bs__aid,
                                                         constants__t_StatusCode_i* const address_space_bs__sc,
                                                         constants__t_Variant_i* const address_space_bs__variant)
{
    SOPC_AddressSpace_Item* item = address_space_bs__node;

    /* Note: conv_* variables are abstract, we must be confident */
    *address_space_bs__sc = constants__e_sc_ok;
    switch (address_space_bs__aid)
    {
    case constants__e_aid_NodeId:
        *address_space_bs__variant = util_variant__new_Variant_from_NodeId(SOPC_AddressSpace_Item_Get_NodeId(item));
        break;
    case constants__e_aid_NodeClass:
        *address_space_bs__variant = util_variant__new_Variant_from_NodeClass(item->node_class);
        break;
    case constants__e_aid_BrowseName:
        *address_space_bs__variant =
            util_variant__new_Variant_from_QualifiedName(SOPC_AddressSpace_Item_Get_BrowseName(item));
        break;
    case constants__e_aid_DisplayName:
        *address_space_bs__variant =
            util_variant__new_Variant_from_LocalizedText(SOPC_AddressSpace_Item_Get_DisplayName(item));
        break;
    case constants__e_aid_Value:
        if (constants__e_ncl_Variable == address_space_bs__ncl ||
            constants__e_ncl_VariableType == address_space_bs__ncl)
        {
            *address_space_bs__variant = util_variant__new_Variant_from_Variant(SOPC_AddressSpace_Item_Get_Value(item));
        }
        else
        {
            *address_space_bs__sc = constants__e_sc_bad_attribute_id_invalid;
            *address_space_bs__variant = util_variant__new_Variant_from_Indet();
        }
        break;
    case constants__e_aid_AccessLevel:
        *address_space_bs__variant =
            util_variant__new_Variant_from_Byte(SOPC_AccessLevelMask_CurrentRead | SOPC_AccessLevelMask_CurrentWrite);
        break;
    default:
        /* TODO: maybe return NULL here, to be consistent with msg_read_response_bs__write_read_response_iter and
         * service_read__treat_read_request behavior. */
        *address_space_bs__variant = util_variant__new_Variant_from_Indet();
        break;
    }
}

void address_space_bs__set_Value(const constants__t_Node_i address_space_bs__node,
                                 const constants__t_Variant_i address_space_bs__value)
{
    SOPC_AddressSpace_Item* item = address_space_bs__node;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_Variant* pvar = SOPC_AddressSpace_Item_Get_Value(item);
    // Clear old value
    SOPC_Variant_Clear(pvar);
    /* Deep-copy the new value */
    status = SOPC_Variant_Copy(pvar, address_space_bs__value);
    assert(SOPC_STATUS_OK == status);
}

void address_space_bs__get_Value_StatusCode(const constants__t_Node_i address_space_bs__node,
                                            constants__t_StatusCode_i* const address_space_bs__sc)
{
    SOPC_AddressSpace_Item* item = address_space_bs__node;
    util_status_code__C_to_B(item->value_status, address_space_bs__sc);
}

void address_space_bs__read_AddressSpace_free_value(const constants__t_Variant_i address_space_bs__val)
{
    free(address_space_bs__val);
}

void address_space_bs__get_BrowseName(const constants__t_Node_i address_space_bs__p_node,
                                      constants__t_QualifiedName_i* const address_space_bs__p_browse_name)
{
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    *address_space_bs__p_browse_name = SOPC_AddressSpace_Item_Get_BrowseName(item);
}

void address_space_bs__get_DisplayName(const constants__t_Node_i address_space_bs__p_node,
                                       constants__t_LocalizedText_i* const address_space_bs__p_display_name)
{
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    *address_space_bs__p_display_name = SOPC_AddressSpace_Item_Get_DisplayName(item);
}

void address_space_bs__get_NodeClass(const constants__t_Node_i address_space_bs__p_node,
                                     constants__t_NodeClass_i* const address_space_bs__p_node_class)
{
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;

    bool res = util_NodeClass__C_to_B(item->node_class, address_space_bs__p_node_class);
    if (false == res)
    {
        *address_space_bs__p_node_class = constants__c_NodeClass_indet;
    }
}

void address_space_bs__get_TypeDefinition(const constants__t_Node_i address_space_bs__p_node,
                                          constants__t_ExpandedNodeId_i* const address_space_bs__p_type_def)
{
    // TODO: Not implemented yet
    (void) address_space_bs__p_node;
    *address_space_bs__p_type_def = constants__c_ExpandedNodeId_indet;
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

void address_space_bs__get_Node_RefIndexBegin(const constants__t_Node_i address_space_bs__p_node,
                                              t_entier4* const address_space_bs__p_ref_index)
{
    (void) address_space_bs__p_node;
    *address_space_bs__p_ref_index = 0;
}

void address_space_bs__get_Node_RefIndexEnd(const constants__t_Node_i address_space_bs__p_node,
                                            t_entier4* const address_space_bs__p_ref_index)
{
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    int32_t* n_refs = SOPC_AddressSpace_Item_Get_NoOfReferences(item);
    *address_space_bs__p_ref_index = *n_refs - 1;
}

void address_space_bs__get_RefIndex_Reference(const constants__t_Node_i address_space_bs__p_node,
                                              const t_entier4 address_space_bs__p_ref_index,
                                              constants__t_Reference_i* const address_space_bs__p_ref)
{
    SOPC_AddressSpace_Item* item = address_space_bs__p_node;
    OpcUa_ReferenceNode** refs = SOPC_AddressSpace_Item_Get_References(item);
    int32_t* n_refs = SOPC_AddressSpace_Item_Get_NoOfReferences(item);
    assert(address_space_bs__p_ref_index < *n_refs);
    *address_space_bs__p_ref = &(*refs)[address_space_bs__p_ref_index];
}
