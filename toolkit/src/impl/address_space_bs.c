/*
 *  Copyright (C) 2017 Systerel and others.
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


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "b2c.h"
#include "address_space_bs.h"

#include "address_space_impl.h"
#include "gen_addspace.h"
#include "util_variant.h"


/*
 * TODO: Change the following structs for the newer model
 * Kept the same data-structures, even if these concrete variables became abstract.
 * Changed the name.
 * Their index is 0-based.
 */
constants__t_NodeId_i a_NodeId[NB_NODES];
constants__t_NodeClass_i a_NodeClass[NB_NODES];
constants__t_Variant_i a_Value[NB_NODES];
constants__t_StatusCode_i a_Value_StatusCode[NB_NODES];

/*
 * The following are the pointers to arrays containing the nodes the AddressSpace.
 * There are also variables storing required lengths and offsets.
 * These pointers and variables must be initialized before address_space_bs__INITIALISATION is called.
 * Index of the arrays should start at 1, so array[0] is never accessed.
 *
 * Attributes are grouped by families.
 * It is possible to group the 22 attributes in 8 families,
 * and to order the nodes by their node class,
 * so that the arrays of the attributes are contiguous.
 *
 * NodeClass order: Variable, VariableType, ObjectType, ReferenceType, DataType, Method, Object, View
 *
 * Families:
 *  - All: NodeId, NodeClass, BrowseName, Description, DisplayName, UserWriteMask, WriteMask
 *  - Vars: AccessLevel, UserAccessLevel, Historizing, MinimumSamplingInterval
 *  - Vars+Types: ArrayDimensions, DataType, Value, ValueRank
 *  - Method: Executable, UserExecutable
 *  - RefType: InverseName, Symmetric
 *  - View: ContainsNoLoops
 *  - View+Obj: EventNotifier
 *  - Types: IsAbstract
 *
 * Offsets:
 *  - All: 0
 *  - Vars: 0
 *  - Vars+Types: 0
 *  - Method: nVariables + nVariableType + nObjectType + nReferenceType + nDataType
 *  - RefType: nVariables + nVariableType + nObjectType
 *  - View: nVariables + nVariableType + nObjectType + nReferenceType + nDataType + nMethod + nObject
 *  - View+Obj: nVariables + nVariableType + nObjectType + nReferenceType + nDataType + nMethod
 *  - Types: nVariables
 */
size_t address_space_bs__nNodeId = 0; /* Required by the hashmap */
/* Family All */
constants__t_QualifiedName_i address_space_bs__a_BrowseName = NULL;
constants__t_LocalizedText_i address_space_bs__a_DisplayName = NULL;
/* Family Vars */

/* TODO: HasTypeReference is considered "All", but it should not */
constants__t_ExpandedNodeId_i address_space_bs__HasTypeDefinition = NULL;


/*
 * The following pointers store the references.
 * Three elements of the references (ReferenceType, TargetNode, IsForward) out of the 4 (SourceNode)
 * are stored in 3 arrays indexed by a t_ReferenceIndex (starting at 1).
 * Two more arrays (Node_RefIndexBegin, Node_RefIndexEnd) associates a t_Node to the slice of t_ReferenceIndex.
 *
 * These pointers to arrays must be initialized before address_space_bs__INITIALISATION is called.
 */
constants__t_NodeId_i address_space_bs__refs_ReferenceType = NULL;
constants__t_ExpandedNodeId_i address_space_bs__refs_TargetNode = NULL;
t_bool *address_space_bs__refs_IsForward = NULL;
size_t *address_space_bs__RefIndexBegin = NULL;
size_t *address_space_bs__RefIndexEnd = NULL;


/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_bs__INITIALISATION(void)
{
    if(STATUS_OK != gen_addspace(a_NodeId, a_NodeClass, a_Value, a_Value_StatusCode))
        exit(1);

    /* TODO: handle more correctly the assert, provide log, and adequate exit */
    assert(0 != address_space_bs__nNodeId);
    assert(NULL != a_NodeId);
    assert(NULL != a_NodeClass);
    assert(NULL != address_space_bs__a_BrowseName);
    assert(NULL != address_space_bs__a_DisplayName);
    assert(NULL != a_Value);
    assert(NULL != a_Value_StatusCode);
    assert(NULL != address_space_bs__HasTypeDefinition);
    assert(NULL != address_space_bs__refs_ReferenceType);
    assert(NULL != address_space_bs__refs_TargetNode);
    assert(NULL != address_space_bs__refs_IsForward);
    assert(NULL != address_space_bs__RefIndexBegin);
    assert(NULL != address_space_bs__RefIndexEnd);
}


/*--------------------
   OPERATIONS Clause
  --------------------*/

/* As INITIALISATION may use mallocs, needs an UNINIT */
void address_space_bs__UNINITIALISATION(void)
{
    free_addspace(a_NodeId, a_NodeClass, a_Value, a_Value_StatusCode);
}


/* This is a_NodeId~ */
void address_space_bs__readall_AddressSpace_Node(
   const constants__t_NodeId_i address_space_bs__nid,
   t_bool * const address_space_bs__nid_valid,
   constants__t_Node_i * const address_space_bs__node)
{
    constants__t_Node_i i;
    SOPC_NodeId *pnid_req, *pnid;
    int nid_cmp;

    *address_space_bs__nid_valid = false;

    pnid_req = (SOPC_NodeId *)address_space_bs__nid;
    if(NULL == pnid_req)
        return;

    /* Very impressive hashmap with a single entry, and time to compute hash is 0! */
    for(i=0; i<NB_NODES; ++i)
    {
        pnid = (SOPC_NodeId *)a_NodeId[i];
        if(NULL == pnid)
            continue;

        if(STATUS_OK != SOPC_NodeId_Compare(pnid_req, pnid, &nid_cmp))
            continue; /* That should be an error */

        if(nid_cmp == 0) {
            *address_space_bs__nid_valid = true;
            *address_space_bs__node = i+1;
            return;
        }
    }
}


/* Reads any attribute and outputs a variant (valid or not)
 * As this function uses the *_2_Variant_i functions, the value must be freed once used
 */
void address_space_bs__read_AddressSpace_Attribute_value(
   const constants__t_Node_i address_space_bs__node,
   const constants__t_AttributeId_i address_space_bs__aid,
   constants__t_Variant_i * const address_space_bs__variant)
{
    /* Note: conv_* variables are abstract, we must be confident */
    switch(address_space_bs__aid)
    {
    case constants__e_aid_NodeId:
        *address_space_bs__variant = util_variant__new_Variant_from_NodeId(a_NodeId[address_space_bs__node-1]);
        break;
    case constants__e_aid_NodeClass:
        *address_space_bs__variant = util_variant__new_Variant_from_NodeClass(a_NodeClass[address_space_bs__node-1]);
        break;
    case constants__e_aid_Value:
        *address_space_bs__variant = util_variant__new_Variant_from_Variant(a_Value[address_space_bs__node-1]);
        break;
    default:
        /* TODO: maybe return NULL here, to be consistent with msg_read_response_bs__write_read_response_iter and service_read__treat_read_request behavior. */
        *address_space_bs__variant = util_variant__new_Variant_from_Indet();
        break;
    }
}


void address_space_bs__set_Value(
   const constants__t_Node_i address_space_bs__node,
   const constants__t_Variant_i address_space_bs__value)
{
    /* TODO: the value is not encoded yet. */
    /* TODO: this may FAIL! But the operation is not specified to be able to fail */
    SOPC_Variant *pvar = malloc(sizeof(SOPC_Variant));

    /* Deep-copy the value */
    if(NULL != pvar)
    {
        SOPC_Variant_Initialize(pvar);
        if(STATUS_OK == SOPC_Variant_Copy(pvar, (SOPC_Variant *)address_space_bs__value))
        {
            SOPC_Variant_Clear((SOPC_Variant *)a_Value[address_space_bs__node-1]);
            free((void *)a_Value[address_space_bs__node-1]);
            a_Value[address_space_bs__node-1] = (constants__t_Variant_i)pvar;
        }
    }
}


void address_space_bs__get_Value_StatusCode(
   const constants__t_Node_i address_space_bs__node,
   constants__t_StatusCode_i * const address_space_bs__sc)
{
    *address_space_bs__sc = a_Value_StatusCode[address_space_bs__node-1];
}


void address_space_bs__read_AddressSpace_free_value(
   const constants__t_Variant_i address_space_bs__val)
{
    free(address_space_bs__val);
}


void address_space_bs__get_BrowseName(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_QualifiedName_i * const address_space_bs__p_browse_name)
{
    *address_space_bs__p_browse_name = &((SOPC_QualifiedName *)address_space_bs__a_BrowseName)[address_space_bs__p_node];
}


void address_space_bs__get_DisplayName(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_LocalizedText_i * const address_space_bs__p_display_name)
{
    *address_space_bs__p_display_name = &((SOPC_LocalizedText *)address_space_bs__a_DisplayName)[address_space_bs__p_node];
}


void address_space_bs__get_NodeClass(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_NodeClass_i * const address_space_bs__p_node_class)
{
    *address_space_bs__p_node_class = a_NodeClass[address_space_bs__p_node-1];
}


void address_space_bs__get_TypeDefinition(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_ExpandedNodeId_i * const address_space_bs__p_type_def)
{
    *address_space_bs__p_type_def = &((SOPC_ExpandedNodeId *)address_space_bs__HasTypeDefinition)[address_space_bs__p_node];
}


void address_space_bs__get_Reference_ReferenceType(
   const constants__t_Reference_i address_space_bs__p_ref,
   constants__t_NodeId_i * const address_space_bs__p_RefType)
{
    *address_space_bs__p_RefType = &((SOPC_ExpandedNodeId *)address_space_bs__refs_ReferenceType)[address_space_bs__p_ref];
}


void address_space_bs__get_Reference_TargetNode(
   const constants__t_Reference_i address_space_bs__p_ref,
   constants__t_ExpandedNodeId_i * const address_space_bs__p_TargetNode)
{
    *address_space_bs__p_TargetNode = &((SOPC_NodeId *)address_space_bs__refs_TargetNode)[address_space_bs__p_ref];
}


void address_space_bs__get_Reference_IsForward(
   const constants__t_Reference_i address_space_bs__p_ref,
   t_bool * const address_space_bs__p_IsForward)
{
    *address_space_bs__p_IsForward = &((t_bool *)address_space_bs__refs_IsForward)[address_space_bs__p_ref];
}


void address_space_bs__get_Node_RefIndexBegin(
   const constants__t_Node_i address_space_bs__p_node,
   t_entier4 * const address_space_bs__p_ref_index)
{
    *address_space_bs__p_ref_index = address_space_bs__RefIndexBegin[address_space_bs__p_node];
}


void address_space_bs__get_Node_RefIndexEnd(
   const constants__t_Node_i address_space_bs__p_node,
   t_entier4 * const address_space_bs__p_ref_index)
{
    *address_space_bs__p_ref_index = address_space_bs__RefIndexEnd[address_space_bs__p_node];
}


void address_space_bs__get_RefIndex_Reference(
   const t_entier4 address_space_bs__p_ref_index,
   constants__t_Reference_i * const address_space_bs__p_ref)
{
    printf("Not implemented\n"); assert(0);
}


