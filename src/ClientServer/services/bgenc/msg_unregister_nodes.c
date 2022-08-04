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

 File Name            : msg_unregister_nodes.c

 Date                 : 04/08/2022 14:53:09

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "msg_unregister_nodes.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 msg_unregister_nodes__nb_nodes;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_unregister_nodes__INITIALISATION(void) {
   msg_unregister_nodes__nb_nodes = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_unregister_nodes__getall_msg_unregister_nodes_req_nb_nodes(
   const constants__t_msg_i msg_unregister_nodes__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_unregister_nodes__p_sc,
   t_entier4 * const msg_unregister_nodes__p_nb_nodes) {
   msg_unregister_nodes_bs__get_msg_unregister_nodes_req_nb_nodes(msg_unregister_nodes__p_req_msg,
      msg_unregister_nodes__p_nb_nodes);
   if (*msg_unregister_nodes__p_nb_nodes <= 0) {
      *msg_unregister_nodes__p_sc = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
   }
   else if (*msg_unregister_nodes__p_nb_nodes > constants__k_n_unregisterNodes_max) {
      *msg_unregister_nodes__p_sc = constants_statuscodes_bs__e_sc_bad_too_many_ops;
   }
   else {
      msg_unregister_nodes__nb_nodes = *msg_unregister_nodes__p_nb_nodes;
      *msg_unregister_nodes__p_sc = constants_statuscodes_bs__e_sc_ok;
   }
}

