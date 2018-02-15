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

/******************************************************************************

 File Name            : address_space.c

 Date                 : 15/02/2018 15:01:10

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_bool address_space__ResponseWrite_allocated;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space__INITIALISATION(void)
{
    address_space__ResponseWrite_allocated = false;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space__read_NodeClass_Attribute(const constants__t_Node_i address_space__node,
                                             const constants__t_AttributeId_i address_space__aid,
                                             constants__t_StatusCode_i* const address_space__sc,
                                             constants__t_NodeClass_i* const address_space__ncl,
                                             constants__t_Variant_i* const address_space__val)
{
    address_space_bs__get_NodeClass(address_space__node, address_space__ncl);
    address_space_bs__read_AddressSpace_Attribute_value(address_space__node, *address_space__ncl, address_space__aid,
                                                        address_space__sc, address_space__val);
}

void address_space__alloc_write_request_responses(const t_entier4 address_space__nb_req,
                                                  t_bool* const address_space__bret)
{
    if (address_space__nb_req <= constants__k_n_WriteResponse_max)
    {
        response_write_bs__alloc_write_request_responses_malloc(address_space__nb_req,
                                                                &address_space__ResponseWrite_allocated);
    }
    else
    {
        address_space__ResponseWrite_allocated = false;
    }
    *address_space__bret = address_space__ResponseWrite_allocated;
}

void address_space__treat_write_request_WriteValues(constants__t_StatusCode_i* const address_space__StatusCode_service)
{
    {
        t_entier4 address_space__l_nb_req;
        t_bool address_space__l_continue;
        constants__t_AttributeId_i address_space__l_aid;
        constants__t_NodeId_i address_space__l_nid;
        constants__t_Variant_i address_space__l_value;
        constants__t_WriteValue_i address_space__l_wvi;
        constants__t_StatusCode_i address_space__l_status1;
        constants__t_StatusCode_i address_space__l_status2;
        t_bool address_space__l_isvalid;

        *address_space__StatusCode_service = constants__e_sc_ok;
        service_write_decode_bs__get_nb_WriteValue(&address_space__l_nb_req);
        address_space_it__init_iter_write_request(address_space__l_nb_req, &address_space__l_continue);
        while (address_space__l_continue == true)
        {
            address_space_it__continue_iter_write_request(&address_space__l_continue, &address_space__l_wvi);
            service_write_decode_bs__getall_WriteValue(address_space__l_wvi, &address_space__l_isvalid,
                                                       &address_space__l_status1, &address_space__l_nid,
                                                       &address_space__l_aid, &address_space__l_value);
            address_space__treat_write_1(address_space__l_isvalid, address_space__l_status1, address_space__l_nid,
                                         address_space__l_aid, address_space__l_value, &address_space__l_status2);
            response_write_bs__set_ResponseWrite_StatusCode(address_space__l_wvi, address_space__l_status2);
        }
    }
}

void address_space__dealloc_write_request_responses(void)
{
    address_space__ResponseWrite_allocated = false;
    response_write_bs__reset_ResponseWrite();
}

void address_space__treat_write_1(const t_bool address_space__isvalid,
                                  const constants__t_StatusCode_i address_space__status,
                                  const constants__t_NodeId_i address_space__nid,
                                  const constants__t_AttributeId_i address_space__aid,
                                  const constants__t_Variant_i address_space__value,
                                  constants__t_StatusCode_i* const address_space__sc)
{
    if (address_space__isvalid == true)
    {
        address_space__treat_write_2(address_space__nid, address_space__aid, address_space__value, address_space__sc);
    }
    else
    {
        *address_space__sc = address_space__status;
    }
}

void address_space__treat_write_2(const constants__t_NodeId_i address_space__nid,
                                  const constants__t_AttributeId_i address_space__aid,
                                  const constants__t_Variant_i address_space__value,
                                  constants__t_StatusCode_i* const address_space__sc)
{
    {
        t_bool address_space__l_isvalid;
        constants__t_Node_i address_space__l_node;
        constants__t_NodeClass_i address_space__l_ncl;

        *address_space__sc = constants__e_sc_bad_node_id_unknown;
        address_space_bs__readall_AddressSpace_Node(address_space__nid, &address_space__l_isvalid,
                                                    &address_space__l_node);
        if (address_space__l_isvalid == true)
        {
            if (address_space__aid == constants__e_aid_Value)
            {
                address_space_bs__get_NodeClass(address_space__l_node, &address_space__l_ncl);
                if (address_space__l_ncl == constants__e_ncl_Variable)
                {
                    address_space_bs__set_Value(address_space__l_node, address_space__value);
                    *address_space__sc = constants__e_sc_ok;
                }
            }
            else
            {
                *address_space__sc = constants__e_sc_bad_write_not_supported;
            }
        }
    }
}
