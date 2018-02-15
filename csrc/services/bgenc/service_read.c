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

 File Name            : service_read.c

 Date                 : 15/02/2018 15:01:14

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_read.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_read__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_read__fill_read_response_1(const constants__t_msg_i service_read__p_resp_msg,
                                        const t_bool service_read__p_isvalid,
                                        const constants__t_NodeId_i service_read__p_nid,
                                        const constants__t_AttributeId_i service_read__p_aid,
                                        const constants__t_ReadValue_i service_read__p_rvi)
{
    {
        t_bool service_read__l_is_valid;
        constants__t_Node_i service_read__l_node;
        constants__t_NodeClass_i service_read__l_ncl;
        constants__t_Variant_i service_read__l_value;
        constants__t_StatusCode_i service_read__l_sc;

        if ((service_read__p_isvalid == true) && (service_read__p_aid != constants__c_AttributeId_indet))
        {
            address_space__readall_AddressSpace_Node(service_read__p_nid, &service_read__l_is_valid,
                                                     &service_read__l_node);
            if (service_read__l_is_valid == true)
            {
                address_space__read_NodeClass_Attribute(service_read__l_node, service_read__p_aid, &service_read__l_sc,
                                                        &service_read__l_ncl, &service_read__l_value);
                if (service_read__l_sc == constants__e_sc_ok)
                {
                    if ((service_read__p_aid == constants__e_aid_Value) &&
                        (service_read__l_ncl == constants__e_ncl_Variable))
                    {
                        address_space__get_Value_StatusCode(service_read__l_node, &service_read__l_sc);
                    }
                    else
                    {
                        service_read__l_sc = constants__e_sc_ok;
                    }
                }
                msg_read_response_bs__set_read_response(service_read__p_resp_msg, service_read__p_rvi,
                                                        service_read__l_value, service_read__l_sc, service_read__p_aid);
                address_space__read_AddressSpace_free_value(service_read__l_value);
            }
            else
            {
                msg_read_response_bs__set_read_response(service_read__p_resp_msg, service_read__p_rvi,
                                                        constants__c_Variant_indet, constants__e_sc_bad_node_id_unknown,
                                                        service_read__p_aid);
            }
        }
        else
        {
            if (service_read__p_nid == constants__c_NodeId_indet)
            {
                msg_read_response_bs__set_read_response(service_read__p_resp_msg, service_read__p_rvi,
                                                        constants__c_Variant_indet, constants__e_sc_bad_node_id_invalid,
                                                        service_read__p_aid);
            }
            else
            {
                msg_read_response_bs__set_read_response(service_read__p_resp_msg, service_read__p_rvi,
                                                        constants__c_Variant_indet,
                                                        constants__e_sc_bad_attribute_id_invalid, service_read__p_aid);
            }
        }
    }
}

void service_read__fill_read_response(const constants__t_msg_i service_read__req_msg,
                                      const constants__t_msg_i service_read__resp_msg)
{
    {
        t_entier4 service_read__l_nb_ReadValue;
        t_bool service_read__l_continue;
        t_bool service_read__l_isvalid;
        constants__t_ReadValue_i service_read__l_rvi;
        constants__t_NodeId_i service_read__l_nid;
        constants__t_AttributeId_i service_read__l_aid;

        msg_read_request__get_nb_ReadValue(&service_read__l_nb_ReadValue);
        service_read_it__init_iter_write_request(service_read__l_nb_ReadValue, &service_read__l_continue);
        while (service_read__l_continue == true)
        {
            service_read_it__continue_iter_write_request(&service_read__l_continue, &service_read__l_rvi);
            msg_read_request__getall_ReadValue_NodeId_AttributeId(service_read__req_msg, service_read__l_rvi,
                                                                  &service_read__l_isvalid, &service_read__l_nid,
                                                                  &service_read__l_aid);
            service_read__fill_read_response_1(service_read__resp_msg, service_read__l_isvalid, service_read__l_nid,
                                               service_read__l_aid, service_read__l_rvi);
        }
    }
}
