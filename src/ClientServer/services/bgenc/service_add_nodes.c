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

 File Name            : service_add_nodes.c

 Date                 : 05/08/2022 09:35:36

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_add_nodes.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_add_nodes__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_add_nodes__treat_add_nodes_items(
   const constants__t_user_i service_add_nodes__p_user,
   const constants__t_msg_i service_add_nodes__p_req_msg,
   const constants__t_msg_i service_add_nodes__p_resp_msg,
   const t_entier4 service_add_nodes__p_nb_nodes_to_add) {
   {
      t_bool service_add_nodes__l_continue;
      t_entier4 service_add_nodes__l_index;
      constants_statuscodes_bs__t_StatusCode_i service_add_nodes__l_sc;
      constants__t_ExpandedNodeId_i service_add_nodes__l_parentExpNid;
      constants__t_NodeId_i service_add_nodes__l_refTypeId;
      constants__t_ExpandedNodeId_i service_add_nodes__l_reqExpNodeId;
      constants__t_QualifiedName_i service_add_nodes__l_browseName;
      constants__t_NodeClass_i service_add_nodes__l_nodeClass;
      constants__t_NodeAttributes_i service_add_nodes__l_nodeAttributes;
      constants__t_ExpandedNodeId_i service_add_nodes__l_typeDefId;
      t_bool service_add_nodes__l_local_treatment;
      t_bool service_add_nodes__l_local_server_exp_node_id;
      constants__t_NodeId_i service_add_nodes__l_node_id;
      t_bool service_add_nodes__l_authorized_add_node;
      constants__t_NodeId_i service_add_nodes__l_new_nid;
      
      node_management_add_nodes_items_it__init_iter_add_nodes_items(service_add_nodes__p_nb_nodes_to_add,
         &service_add_nodes__l_continue);
      while (service_add_nodes__l_continue == true) {
         node_management_add_nodes_items_it__continue_iter_add_nodes_items(&service_add_nodes__l_continue,
            &service_add_nodes__l_index);
         service_add_nodes__l_new_nid = constants__c_NodeId_indet;
         msg_node_management_add_nodes__getall_add_node_item_req_params(service_add_nodes__p_req_msg,
            service_add_nodes__l_index,
            &service_add_nodes__l_sc,
            &service_add_nodes__l_parentExpNid,
            &service_add_nodes__l_refTypeId,
            &service_add_nodes__l_reqExpNodeId,
            &service_add_nodes__l_browseName,
            &service_add_nodes__l_nodeClass,
            &service_add_nodes__l_nodeAttributes,
            &service_add_nodes__l_typeDefId);
         if (service_add_nodes__l_sc == constants_statuscodes_bs__e_sc_ok) {
            service_add_nodes_1__is_local_service_treatment(&service_add_nodes__l_local_treatment);
            if ((service_add_nodes__l_local_treatment == false) &&
               (service_add_nodes__l_reqExpNodeId != constants__c_ExpandedNodeId_indet)) {
               constants__getall_conv_ExpandedNodeId_NodeId(service_add_nodes__l_reqExpNodeId,
                  &service_add_nodes__l_local_server_exp_node_id,
                  &service_add_nodes__l_node_id);
               if (service_add_nodes__l_local_server_exp_node_id == true) {
                  service_add_nodes_1__get_user_authorization(constants__e_operation_type_addnode,
                     service_add_nodes__l_node_id,
                     constants__e_aid_NodeId,
                     service_add_nodes__p_user,
                     &service_add_nodes__l_authorized_add_node);
                  if (service_add_nodes__l_authorized_add_node == false) {
                     service_add_nodes__l_sc = constants_statuscodes_bs__e_sc_bad_user_access_denied;
                  }
               }
            }
         }
         if (service_add_nodes__l_sc == constants_statuscodes_bs__e_sc_ok) {
            service_add_nodes_1__treat_add_nodes_item(service_add_nodes__l_parentExpNid,
               service_add_nodes__l_refTypeId,
               service_add_nodes__l_reqExpNodeId,
               service_add_nodes__l_browseName,
               service_add_nodes__l_nodeClass,
               service_add_nodes__l_nodeAttributes,
               service_add_nodes__l_typeDefId,
               &service_add_nodes__l_sc,
               &service_add_nodes__l_new_nid);
         }
         msg_node_management_add_nodes__setall_msg_add_nodes_item_resp_params(service_add_nodes__p_resp_msg,
            service_add_nodes__l_index,
            service_add_nodes__l_sc,
            service_add_nodes__l_new_nid);
      }
   }
}

void service_add_nodes__treat_add_nodes_request(
   const constants__t_user_i service_add_nodes__p_user,
   const constants__t_msg_i service_add_nodes__p_req_msg,
   const constants__t_msg_i service_add_nodes__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_add_nodes__StatusCode_service) {
   {
      constants_statuscodes_bs__t_StatusCode_i service_add_nodes__l_sc;
      t_entier4 service_add_nodes__l_nb_nodes_to_add;
      t_bool service_add_nodes__l_bres;
      
      *service_add_nodes__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
      service_add_nodes__l_bres = false;
      service_add_nodes_1__is_AddressSpace_constant(&service_add_nodes__l_bres);
      if (service_add_nodes__l_bres == false) {
         msg_node_management_add_nodes__getall_msg_add_nodes_req_params(service_add_nodes__p_req_msg,
            &service_add_nodes__l_sc,
            &service_add_nodes__l_nb_nodes_to_add);
         if (service_add_nodes__l_sc == constants_statuscodes_bs__e_sc_ok) {
            msg_node_management_add_nodes__alloc_msg_add_nodes_resp_results(service_add_nodes__p_resp_msg,
               service_add_nodes__l_nb_nodes_to_add,
               &service_add_nodes__l_bres);
            if (service_add_nodes__l_bres == true) {
               *service_add_nodes__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
               service_add_nodes__treat_add_nodes_items(service_add_nodes__p_user,
                  service_add_nodes__p_req_msg,
                  service_add_nodes__p_resp_msg,
                  service_add_nodes__l_nb_nodes_to_add);
            }
            else {
               *service_add_nodes__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
         }
         else {
            *service_add_nodes__StatusCode_service = service_add_nodes__l_sc;
         }
      }
      else {
         *service_add_nodes__StatusCode_service = constants_statuscodes_bs__e_sc_bad_service_unsupported;
      }
   }
}

