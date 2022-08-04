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

 File Name            : service_unregister_nodes.c

 Date                 : 04/08/2022 14:53:14

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_unregister_nodes.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_unregister_nodes__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_unregister_nodes__treat_unregister_nodes_request(
   const constants__t_msg_i service_unregister_nodes__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_unregister_nodes__ret) {
   {
      t_entier4 service_unregister_nodes__l_nb_nodes;
      
      msg_unregister_nodes__getall_msg_unregister_nodes_req_nb_nodes(service_unregister_nodes__p_req_msg,
         service_unregister_nodes__ret,
         &service_unregister_nodes__l_nb_nodes);
   }
}

