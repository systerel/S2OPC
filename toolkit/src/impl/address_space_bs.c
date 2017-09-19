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
 * The following are the pointers to arrays containing the nodes the AddressSpace.
 * There are also variables storing required lengths and offsets.
 * These pointers and variables must be initialized before address_space_bs__INITIALISATION is called.
 * Index of the arrays should start at 1, so array[0] is never accessed.
 *
 * Attributes are grouped by families.
 * It is possible to group the 22 attributes + 1 pseudo-attribute in 9 families,
 * and to order the nodes by their node class,
 * so that the arrays of the attributes are contiguous.
 * The pseudo-attribute is HasTypeDefinition,
 * which has the same properties as an attribute of Object and Variable:
 * a unique and mandatory field.
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
 *  - HasTypeDef
 *
 * Offsets:
 *  - All: 0
 *  - Vars: nViews + nObjects
 *  - Vars+Types: nViews + nObjects
 *  - Method: nViews + nObjects + nVariables + nVariableTypes + nObjectTypes + nReferenceTypes + nDataTypes
 *  - RefType: nViews + nObjects + nVariables + nVariableTypes + nObjectTypes
 *  - View: 0
 *  - View+Obj: 0
 *  - Types: nViews + nObjects + nVariables
 *  - HasTypeDef: nViews
 *
 * Supplementary notes on C types: some of the types are constants__t_type_i *address_space_bs__array;
 * other types are constants__t_type_i address_space_bs__array.
 * The choice of which form is used depends on:
 *  - how the data are generated
 *  - the used B-type, whether it typedefs to a void * or not
 */
/* Sizes */
int32_t address_space_bs__nNodeIds = 0; /* Required by the hashmap */
int32_t address_space_bs__nVariables = 0;
int32_t address_space_bs__nVariableTypes = 0;
int32_t address_space_bs__nObjectTypes = 0;
int32_t address_space_bs__nReferenceTypes = 0;
int32_t address_space_bs__nDataTypes = 0;
int32_t address_space_bs__nMethods = 0; /* May be useless */
int32_t address_space_bs__nObjects = 0;
int32_t address_space_bs__nViews = 0;
/* Offsets. These are filled in INITIALISATION(), and are not public */
int32_t offVars = 0;
int32_t offVarsTypes = 0;
int32_t offMethods = 0;
int32_t offRefTypes = 0;
int32_t offTypes = 0;
int32_t offHasTypeDefs = 0;
/* Family All */
constants__t_NodeId_i           *address_space_bs__a_NodeId = NULL;
constants__t_NodeClass_i        *address_space_bs__a_NodeClass = NULL;
constants__t_QualifiedName_i    address_space_bs__a_BrowseName = NULL;
constants__t_LocalizedText_i    address_space_bs__a_DisplayName = NULL;
int32_t                         *address_space_bs__a_DisplayName_begin = NULL;
int32_t                         *address_space_bs__a_DisplayName_end = NULL;
/* Family Vars */
constants__t_Variant_i          *address_space_bs__a_Value = NULL;
constants__t_StatusCode_i       *address_space_bs__a_Value_StatusCode = NULL;

/* Family HasTypeDefinition */
constants__t_ExpandedNodeId_i   *address_space_bs__HasTypeDefinition = NULL;


/*
 * The following pointers store the references.
 * Three elements of the references (ReferenceType, TargetNode, IsForward) out of the 4 (SourceNode)
 * are stored in 3 arrays indexed by a t_ReferenceIndex (starting at 1).
 * Two more arrays (Node_RefIndexBegin, Node_RefIndexEnd) associates a t_Node to the slice of t_ReferenceIndex.
 *
 * These pointers to arrays must be initialized before address_space_bs__INITIALISATION is called.
 */
constants__t_NodeId_i           *address_space_bs__refs_ReferenceType = NULL;
constants__t_ExpandedNodeId_i   *address_space_bs__refs_TargetNode = NULL;
bool                            *address_space_bs__refs_IsForward = NULL;
int32_t                         *address_space_bs__RefIndexBegin = NULL;
int32_t                         *address_space_bs__RefIndexEnd = NULL;


/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_bs__INITIALISATION(void)
{
    /* TODO: handle more correctly the assert, provide log, and adequate exit */
    /* TODO: cleints must be able to not fail here */
    assert(0 != address_space_bs__nNodeIds);
    assert(NULL != address_space_bs__a_NodeId);
    assert(NULL != address_space_bs__a_NodeClass);
    assert(NULL != address_space_bs__a_BrowseName);
    assert(NULL != address_space_bs__a_DisplayName);
    assert(NULL != address_space_bs__a_DisplayName_begin);
    assert(NULL != address_space_bs__a_DisplayName_end);
    assert(NULL != address_space_bs__a_Value);
    assert(NULL != address_space_bs__a_Value_StatusCode);
    /*assert(NULL != address_space_bs__HasTypeDefinition);*/
    assert(NULL != address_space_bs__refs_ReferenceType);
    assert(NULL != address_space_bs__refs_TargetNode);
    assert(NULL != address_space_bs__refs_IsForward);
    assert(NULL != address_space_bs__RefIndexBegin);
    assert(NULL != address_space_bs__RefIndexEnd);

    /* Compute offsets */
    offHasTypeDefs = address_space_bs__nViews;
    offVars = offHasTypeDefs + address_space_bs__nObjects;
    offVarsTypes = offVars;
    offTypes = offVars + address_space_bs__nVariables;
    offRefTypes = offTypes + address_space_bs__nVariableTypes + address_space_bs__nObjectTypes;
    offMethods = offRefTypes + address_space_bs__nReferenceTypes + address_space_bs__nDataTypes;
}


/*--------------------
   OPERATIONS Clause
  --------------------*/

/* As INITIALISATION may use mallocs, needs an UNINIT */
void address_space_bs__UNINITIALISATION(void)
{
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
        pnid = (SOPC_NodeId *)address_space_bs__a_NodeId[i];
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
        *address_space_bs__variant = util_variant__new_Variant_from_NodeId(address_space_bs__a_NodeId[address_space_bs__node]);
        break;
    case constants__e_aid_NodeClass:
        *address_space_bs__variant = util_variant__new_Variant_from_NodeClass(address_space_bs__a_NodeClass[address_space_bs__node]);
        break;
    case constants__e_aid_Value:
        *address_space_bs__variant = util_variant__new_Variant_from_Variant(address_space_bs__a_Value[address_space_bs__node-offVarsTypes]);
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
    /* TODO: this may fail, but the operation is not specified to be able to fail */
    SOPC_Variant *pvar = malloc(sizeof(SOPC_Variant));
    constants__t_Variant_i poldvar = NULL;

    /* Deep-copy the value */
    if(NULL != pvar)
    {
        SOPC_Variant_Initialize(pvar);
        if(STATUS_OK == SOPC_Variant_Copy(pvar, (SOPC_Variant *)address_space_bs__value))
        {
            poldvar = address_space_bs__a_Value[address_space_bs__node-offVarsTypes];
            SOPC_Variant_Clear((SOPC_Variant *)poldvar);
            free((void *)poldvar);
            poldvar = (constants__t_Variant_i)pvar;
        }
    }
}


void address_space_bs__get_Value_StatusCode(
   const constants__t_Node_i address_space_bs__node,
   constants__t_StatusCode_i * const address_space_bs__sc)
{
    *address_space_bs__sc = address_space_bs__a_Value_StatusCode[address_space_bs__node-offVarsTypes];
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
    int32_t i = address_space_bs__a_DisplayName_begin[address_space_bs__p_node];

    /* TODO: this constraint should be pushed to dataprep */
    assert(address_space_bs__a_DisplayName_end[address_space_bs__p_node] >= i);
    *address_space_bs__p_display_name = &((SOPC_LocalizedText *)address_space_bs__a_DisplayName)[i];
}


void address_space_bs__get_NodeClass(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_NodeClass_i * const address_space_bs__p_node_class)
{
    *address_space_bs__p_node_class = address_space_bs__a_NodeClass[address_space_bs__p_node];
}


void address_space_bs__get_TypeDefinition(
   const constants__t_Node_i address_space_bs__p_node,
   constants__t_ExpandedNodeId_i * const address_space_bs__p_type_def)
{
    /* TODO: Temporary check. HasTypeDefinition is defined by PRE but current implentation does not populate it.
     *  Also uncomment the assert in address_space_bs__INITIALISATION */
    if(NULL == address_space_bs__HasTypeDefinition)
        *address_space_bs__p_type_def = constants__c_ExpandedNodeId_indet;
    else
        *address_space_bs__p_type_def = &((SOPC_ExpandedNodeId *)address_space_bs__HasTypeDefinition)[address_space_bs__p_node - offHasTypeDefs];
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
    *address_space_bs__p_ref = address_space_bs__p_ref_index;
}


