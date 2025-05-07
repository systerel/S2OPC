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

/******************************************************************************

 File Name            : service_delete_nodes.c

 Date                 : 17/06/2025 15:37:43

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_delete_nodes.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_delete_nodes__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_delete_nodes__local_treat_delete_nodes_index(
   const constants__t_user_i service_delete_nodes__p_user,
   const constants__t_msg_i service_delete_nodes__p_req_msg,
   const constants__t_msg_i service_delete_nodes__p_resp_msg,
   const t_entier4 service_delete_nodes__p_index) {
   {
      constants_statuscodes_bs__t_StatusCode_i service_delete_nodes__l_sc;
      constants__t_NodeId_i service_delete_nodes__l_nodeId;
      t_bool service_delete_nodes__l_b_deleteTargetReferences;
      t_bool service_delete_nodes__l_local_treatment;
      t_bool service_delete_nodes__l_authorized_delete_node;
      constants__t_sessionRoles_i service_delete_nodes__l_roles;
      
      msg_node_management_delete_nodes__getall_delete_node_item_req_params(service_delete_nodes__p_req_msg,
         service_delete_nodes__p_index,
         &service_delete_nodes__l_nodeId,
         &service_delete_nodes__l_b_deleteTargetReferences);
      service_delete_nodes__l_sc = constants_statuscodes_bs__e_sc_ok;
      service_add_nodes__is_local_service_treatment(&service_delete_nodes__l_local_treatment);
      if (service_delete_nodes__l_local_treatment == false) {
         service_add_nodes__get_user_roles(service_delete_nodes__p_user,
            &service_delete_nodes__l_roles);
         service_add_nodes__get_user_authorization(constants__e_operation_type_deletenode,
            service_delete_nodes__l_nodeId,
            constants__e_aid_NodeId,
            service_delete_nodes__p_user,
            service_delete_nodes__l_roles,
            &service_delete_nodes__l_authorized_delete_node);
         if (service_delete_nodes__l_authorized_delete_node == false) {
            service_delete_nodes__l_sc = constants_statuscodes_bs__e_sc_bad_user_access_denied;
         }
      }
      if (service_delete_nodes__l_sc == constants_statuscodes_bs__e_sc_ok) {
         service_add_nodes__deleteNode_AddressSpace(service_delete_nodes__l_nodeId,
            service_delete_nodes__l_b_deleteTargetReferences,
            &service_delete_nodes__l_sc);
      }
      msg_node_management_delete_nodes__setall_msg_delete_nodes_item_resp_params(service_delete_nodes__p_resp_msg,
         service_delete_nodes__p_index,
         service_delete_nodes__l_sc);
   }
}

void service_delete_nodes__local_treat_delete_nodes_items(
   const constants__t_user_i service_delete_nodes__p_user,
   const constants__t_msg_i service_delete_nodes__p_req_msg,
   const constants__t_msg_i service_delete_nodes__p_resp_msg,
   const t_entier4 service_delete_nodes__p_nb_nodes_to_add) {
   {
      t_bool service_delete_nodes__l_continue;
      t_entier4 service_delete_nodes__l_index;
      
      node_management_delete_nodes_items_it__init_iter_delete_nodes_items(service_delete_nodes__p_nb_nodes_to_add,
         &service_delete_nodes__l_continue);
      while (service_delete_nodes__l_continue == true) {
         node_management_delete_nodes_items_it__continue_iter_delete_nodes_items(&service_delete_nodes__l_continue,
            &service_delete_nodes__l_index);
         service_delete_nodes__local_treat_delete_nodes_index(service_delete_nodes__p_user,
            service_delete_nodes__p_req_msg,
            service_delete_nodes__p_resp_msg,
            service_delete_nodes__l_index);
      }
   }
}

void service_delete_nodes__treat_delete_nodes_request(
   const constants__t_user_i service_delete_nodes__p_user,
   const constants__t_msg_i service_delete_nodes__p_req_msg,
   const constants__t_msg_i service_delete_nodes__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_delete_nodes__StatusCode_service) {
   {
      constants_statuscodes_bs__t_StatusCode_i service_delete_nodes__l_sc;
      t_entier4 service_delete_nodes__l_nb_nodes_to_delete;
      t_bool service_delete_nodes__l_bres;
      
      *service_delete_nodes__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
      service_add_nodes__is_AddressSpace_constant(&service_delete_nodes__l_bres);
      if (service_delete_nodes__l_bres == false) {
         msg_node_management_delete_nodes__getall_msg_delete_nodes_req_params(service_delete_nodes__p_req_msg,
            &service_delete_nodes__l_sc,
            &service_delete_nodes__l_nb_nodes_to_delete);
         if (service_delete_nodes__l_sc == constants_statuscodes_bs__e_sc_ok) {
            msg_node_management_delete_nodes__alloc_msg_delete_nodes_resp_results(service_delete_nodes__p_resp_msg,
               service_delete_nodes__l_nb_nodes_to_delete,
               &service_delete_nodes__l_bres);
            if (service_delete_nodes__l_bres == true) {
               service_delete_nodes__local_treat_delete_nodes_items(service_delete_nodes__p_user,
                  service_delete_nodes__p_req_msg,
                  service_delete_nodes__p_resp_msg,
                  service_delete_nodes__l_nb_nodes_to_delete);
            }
            else {
               *service_delete_nodes__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
         }
         else {
            *service_delete_nodes__StatusCode_service = service_delete_nodes__l_sc;
         }
      }
      else {
         *service_delete_nodes__StatusCode_service = constants_statuscodes_bs__e_sc_bad_service_unsupported;
      }
   }
}

