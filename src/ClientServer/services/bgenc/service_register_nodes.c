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

 File Name            : service_register_nodes.c

 Date                 : 04/08/2022 14:53:12

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_register_nodes.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_register_nodes__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_register_nodes__fill_response_register_nodes(
   const constants__t_msg_i service_register_nodes__p_req_msg,
   const constants__t_msg_i service_register_nodes__p_resp_msg,
   const t_entier4 service_register_nodes__p_nb_nodes,
   t_bool * const service_register_nodes__p_bres) {
   {
      t_bool service_register_nodes__l_continue;
      t_entier4 service_register_nodes__l_index;
      constants__t_NodeId_i service_register_nodes__l_node_id;
      t_bool service_register_nodes__l_ok;
      
      register_nodes_it__init_iter_register_nodes_request(service_register_nodes__p_nb_nodes,
         &service_register_nodes__l_continue);
      service_register_nodes__l_ok = true;
      service_register_nodes__l_index = 0;
      service_register_nodes__l_node_id = constants__c_NodeId_indet;
      while ((service_register_nodes__l_continue == true) &&
         (service_register_nodes__l_ok == true)) {
         register_nodes_it__continue_iter_register_nodes_request(&service_register_nodes__l_continue,
            &service_register_nodes__l_index);
         msg_register_nodes__get_msg_register_nodes_req_node_id(service_register_nodes__p_req_msg,
            service_register_nodes__l_index,
            &service_register_nodes__l_node_id);
         msg_register_nodes__setall_msg_register_nodes_resp_node_id(service_register_nodes__p_resp_msg,
            service_register_nodes__l_index,
            service_register_nodes__l_node_id,
            &service_register_nodes__l_ok);
      }
      *service_register_nodes__p_bres = service_register_nodes__l_ok;
   }
}

void service_register_nodes__treat_register_nodes_request(
   const constants__t_msg_i service_register_nodes__p_req_msg,
   const constants__t_msg_i service_register_nodes__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_register_nodes__ret) {
   {
      constants_statuscodes_bs__t_StatusCode_i service_register_nodes__l_sc;
      t_entier4 service_register_nodes__l_nb_nodes;
      t_bool service_register_nodes__l_bres;
      
      service_register_nodes__l_bres = false;
      msg_register_nodes__getall_msg_register_nodes_req_nb_nodes(service_register_nodes__p_req_msg,
         &service_register_nodes__l_sc,
         &service_register_nodes__l_nb_nodes);
      if (service_register_nodes__l_sc == constants_statuscodes_bs__e_sc_ok) {
         msg_register_nodes__alloc_msg_register_nodes_resp_results(service_register_nodes__p_resp_msg,
            service_register_nodes__l_nb_nodes,
            &service_register_nodes__l_bres);
         if (service_register_nodes__l_bres == true) {
            service_register_nodes__fill_response_register_nodes(service_register_nodes__p_req_msg,
               service_register_nodes__p_resp_msg,
               service_register_nodes__l_nb_nodes,
               &service_register_nodes__l_bres);
            if (service_register_nodes__l_bres == true) {
               *service_register_nodes__ret = constants_statuscodes_bs__e_sc_ok;
            }
            else {
               *service_register_nodes__ret = constants_statuscodes_bs__e_sc_bad_unexpected_error;
            }
         }
         else {
            *service_register_nodes__ret = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else {
         *service_register_nodes__ret = service_register_nodes__l_sc;
      }
   }
}

