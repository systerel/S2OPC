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

 File Name            : msg_node_management_delete_nodes.c

 Date                 : 16/06/2025 14:29:58

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "msg_node_management_delete_nodes.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 msg_node_management_delete_nodes__nb_nodes_to_delete;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_node_management_delete_nodes__INITIALISATION(void) {
   msg_node_management_delete_nodes__nb_nodes_to_delete = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_node_management_delete_nodes__getall_msg_delete_nodes_req_params(
   const constants__t_msg_i msg_node_management_delete_nodes__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_node_management_delete_nodes__p_sc,
   t_entier4 * const msg_node_management_delete_nodes__p_nb_nodes_to_delete) {
   msg_node_management_delete_nodes__nb_nodes_to_delete = 0;
   msg_node_management_delete_nodes_bs__get_msg_delete_nodes_req_nb_delete_nodes(msg_node_management_delete_nodes__p_req_msg,
      msg_node_management_delete_nodes__p_nb_nodes_to_delete);
   if (*msg_node_management_delete_nodes__p_nb_nodes_to_delete <= 0) {
      *msg_node_management_delete_nodes__p_nb_nodes_to_delete = 0;
      *msg_node_management_delete_nodes__p_sc = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
   }
   else if (*msg_node_management_delete_nodes__p_nb_nodes_to_delete > constants__k_n_nodesToDelete_max) {
      *msg_node_management_delete_nodes__p_sc = constants_statuscodes_bs__e_sc_bad_too_many_ops;
   }
   else {
      *msg_node_management_delete_nodes__p_sc = constants_statuscodes_bs__e_sc_ok;
      msg_node_management_delete_nodes__nb_nodes_to_delete = *msg_node_management_delete_nodes__p_nb_nodes_to_delete;
   }
}

