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

 File Name            : service_browse_seq.c

 Date                 : 05/02/2018 16:15:19

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_browse_seq.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_browse_seq__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_browse_seq__fill_browse_response_ref(const constants__t_BrowseValue_i service_browse_seq__p_bvi,
                                                  const constants__t_Reference_i service_browse_seq__p_ref,
                                                  const constants__t_BrowseDirection_i service_browse_seq__p_dir,
                                                  const t_bool service_browse_seq__p_isreftype,
                                                  const constants__t_NodeId_i service_browse_seq__p_ref_type,
                                                  const t_bool service_browse_seq__p_inc_subtype,
                                                  t_bool* const service_browse_seq__p_continue_bri)
{
    {
        constants__t_NodeId_i service_browse_seq__l_RefType;
        constants__t_ExpandedNodeId_i service_browse_seq__l_TargetNode;
        t_bool service_browse_seq__l_IsForward;
        t_bool service_browse_seq__l_res;
        constants__t_BrowseResult_i service_browse_seq__l_bri;

        *service_browse_seq__p_continue_bri = true;
        address_space__get_Reference_ReferenceType(service_browse_seq__p_ref, &service_browse_seq__l_RefType);
        address_space__get_Reference_TargetNode(service_browse_seq__p_ref, &service_browse_seq__l_TargetNode);
        address_space__get_Reference_IsForward(service_browse_seq__p_ref, &service_browse_seq__l_IsForward);
        constants__get_Is_Dir_Forward_Compatible(service_browse_seq__p_dir, service_browse_seq__l_IsForward,
                                                 &service_browse_seq__l_res);
        if (service_browse_seq__l_res == true)
        {
            service_browse__Is_RefTypes_Compatible(service_browse_seq__p_isreftype, service_browse_seq__p_ref_type,
                                                   service_browse_seq__p_inc_subtype, service_browse_seq__l_RefType,
                                                   &service_browse_seq__l_res);
            if (service_browse_seq__l_res == true)
            {
                service_browse_seq_it__continue_iter_browse_result(service_browse_seq__p_continue_bri,
                                                                   &service_browse_seq__l_bri);
                service_browse__copy_target_node_browse_result(
                    service_browse_seq__p_bvi, service_browse_seq__l_bri, service_browse_seq__l_RefType,
                    service_browse_seq__l_TargetNode, service_browse_seq__l_IsForward, &service_browse_seq__l_res);
                if (service_browse_seq__l_res == false)
                {
                    ;
                }
            }
        }
    }
}

void service_browse_seq__fill_browse_response(const constants__t_BrowseValue_i service_browse_seq__p_bvi,
                                              const t_entier4 service_browse_seq__p_nb_bri,
                                              const constants__t_Node_i service_browse_seq__p_src_node,
                                              const constants__t_BrowseDirection_i service_browse_seq__p_dir,
                                              const t_bool service_browse_seq__p_isreftype,
                                              const constants__t_NodeId_i service_browse_seq__p_reftype,
                                              const t_bool service_browse_seq__p_inc_subtype)
{
    {
        t_bool service_browse_seq__l_continue_bri;
        t_bool service_browse_seq__l_continue_ref;
        constants__t_Reference_i service_browse_seq__l_ref;

        service_browse_seq__l_ref = constants__c_Reference_indet;
        service_browse_seq_it__init_iter_browse_result(service_browse_seq__p_nb_bri,
                                                       &service_browse_seq__l_continue_bri);
        if (service_browse_seq__l_continue_bri == true)
        {
            service_browse_seq__l_continue_bri = true;
            service_browse_seq_it__init_iter_reference(service_browse_seq__p_src_node,
                                                       &service_browse_seq__l_continue_ref);
            while ((service_browse_seq__l_continue_ref == true) && (service_browse_seq__l_continue_bri == true))
            {
                service_browse_seq_it__continue_iter_reference(&service_browse_seq__l_continue_ref,
                                                               &service_browse_seq__l_ref);
                service_browse_seq__fill_browse_response_ref(
                    service_browse_seq__p_bvi, service_browse_seq__l_ref, service_browse_seq__p_dir,
                    service_browse_seq__p_isreftype, service_browse_seq__p_reftype, service_browse_seq__p_inc_subtype,
                    &service_browse_seq__l_continue_bri);
            }
            if (service_browse_seq__l_ref != constants__c_Reference_indet)
            {
                service_browse__fill_continuation_point(service_browse_seq__p_bvi, service_browse_seq__l_continue_ref,
                                                        service_browse_seq__l_ref);
            }
        }
    }
}

void service_browse_seq__treat_browse_request_BrowseValue_1(const constants__t_BrowseValue_i service_browse_seq__p_bvi,
                                                            const t_entier4 service_browse_seq__p_nb_target_max)
{
    {
        t_bool service_browse_seq__l_isvalid;
        constants__t_NodeId_i service_browse_seq__l_SrcNodeId;
        constants__t_BrowseDirection_i service_browse_seq__l_dir;
        t_bool service_browse_seq__l_isreftype;
        constants__t_NodeId_i service_browse_seq__l_reftype;
        t_bool service_browse_seq__l_incsubtyp;
        t_entier4 service_browse_seq__l_nb_target;
        constants__t_Node_i service_browse_seq__l_src_node;
        t_entier4 service_browse_seq__l_nb_bri;

        service_browse_decode_bs__getall_BrowseValue(service_browse_seq__p_bvi, &service_browse_seq__l_isvalid,
                                                     &service_browse_seq__l_SrcNodeId, &service_browse_seq__l_dir,
                                                     &service_browse_seq__l_isreftype, &service_browse_seq__l_reftype,
                                                     &service_browse_seq__l_incsubtyp);
        if (service_browse_seq__l_isvalid == true)
        {
            service_browse__get_SourceNode_NbRef(service_browse_seq__l_SrcNodeId, &service_browse_seq__l_isvalid,
                                                 &service_browse_seq__l_nb_target, &service_browse_seq__l_src_node);
            if (service_browse_seq__l_isvalid == true)
            {
                service_browse__alloc_browse_result(service_browse_seq__p_bvi, service_browse_seq__p_nb_target_max,
                                                    service_browse_seq__l_nb_target, &service_browse_seq__l_isvalid,
                                                    &service_browse_seq__l_nb_bri);
                if (service_browse_seq__l_isvalid == true)
                {
                    service_browse_seq__fill_browse_response(
                        service_browse_seq__p_bvi, service_browse_seq__l_nb_bri, service_browse_seq__l_src_node,
                        service_browse_seq__l_dir, service_browse_seq__l_isreftype, service_browse_seq__l_reftype,
                        service_browse_seq__l_incsubtyp);
                }
            }
        }
    }
}

void service_browse_seq__treat_browse_request_BrowseValues(
    constants__t_StatusCode_i* const service_browse_seq__StatusCode_service)
{
    {
        t_entier4 service_browse_seq__l_nb_target_max;
        t_entier4 service_browse_seq__l_nb_bvi;
        t_bool service_browse_seq__l_continue;
        constants__t_BrowseValue_i service_browse_seq__l_bvi;
        t_bool service_browse_seq__l_isallocated;

        service_browse_decode_bs__get_nb_BrowseTargetMax(&service_browse_seq__l_nb_target_max);
        service_browse_decode_bs__get_nb_BrowseValue(&service_browse_seq__l_nb_bvi);
        service_browse__alloc_browse_response(service_browse_seq__l_nb_bvi, &service_browse_seq__l_isallocated);
        if (service_browse_seq__l_isallocated == true)
        {
            *service_browse_seq__StatusCode_service = constants__e_sc_ok;
            service_browse_seq_it__init_iter_browse_request(service_browse_seq__l_nb_bvi,
                                                            &service_browse_seq__l_continue);
            while (service_browse_seq__l_continue == true)
            {
                service_browse_seq_it__continue_iter_browse_request(&service_browse_seq__l_continue,
                                                                    &service_browse_seq__l_bvi);
                service_browse_seq__treat_browse_request_BrowseValue_1(service_browse_seq__l_bvi,
                                                                       service_browse_seq__l_nb_target_max);
            }
        }
        else
        {
            *service_browse_seq__StatusCode_service = constants__e_sc_bad_out_of_memory;
        }
    }
}
