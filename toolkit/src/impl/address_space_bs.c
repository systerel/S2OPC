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

#include "b2c.h"
#include "address_space_bs.h"

#include "address_space_impl.h"
#include "gen_addspace.h"
#include "util_variant.h"


/*
 * Kept the same data-structures, even if these concrete variables became abstract.
 * Changed the name.
 * There index is 0-based.
 */
constants__t_NodeId_i a_NodeId[NB_NODES];
constants__t_NodeClass_i a_NodeClass[NB_NODES];
constants__t_Variant_i a_Value[NB_NODES];
constants__t_StatusCode_i a_Value_StatusCode[NB_NODES];


/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_bs__INITIALISATION(void)
{
    if(STATUS_OK != gen_addspace(a_NodeId, a_NodeClass, a_Value, a_Value_StatusCode))
        exit(1);
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

    *address_space_bs__nid_valid = FALSE;

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
            *address_space_bs__nid_valid = (!FALSE);
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


void address_space_bs__read_Value_StatusCode(
   const constants__t_Node_i address_space_bs__node,
   constants__t_StatusCode_i * const address_space_bs__sc)
{
    *address_space_bs__sc = a_Value_StatusCode[address_space_bs__node-1];
}


void address_space_bs__read_NodeClass(
   const constants__t_Node_i address_space_bs__node,
   constants__t_NodeClass_i * const address_space_bs__sc)
{
    *address_space_bs__sc = a_NodeClass[address_space_bs__node-1];
}


void address_space_bs__read_AddressSpace_free_value(
   const constants__t_Variant_i address_space_bs__val)
{
    free(address_space_bs__val);
}
