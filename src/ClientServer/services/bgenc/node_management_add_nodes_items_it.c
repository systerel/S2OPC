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

 File Name            : node_management_add_nodes_items_it.c

 Date                 : 05/08/2022 09:01:22

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "node_management_add_nodes_items_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 node_management_add_nodes_items_it__rreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void node_management_add_nodes_items_it__INITIALISATION(void) {
   node_management_add_nodes_items_it__rreqs_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void node_management_add_nodes_items_it__init_iter_add_nodes_items(
   const t_entier4 node_management_add_nodes_items_it__nb_req,
   t_bool * const node_management_add_nodes_items_it__continue) {
   node_management_add_nodes_items_it__rreqs_i = node_management_add_nodes_items_it__nb_req;
   *node_management_add_nodes_items_it__continue = (0 < node_management_add_nodes_items_it__nb_req);
}

void node_management_add_nodes_items_it__continue_iter_add_nodes_items(
   t_bool * const node_management_add_nodes_items_it__continue,
   t_entier4 * const node_management_add_nodes_items_it__index) {
   *node_management_add_nodes_items_it__index = node_management_add_nodes_items_it__rreqs_i;
   node_management_add_nodes_items_it__rreqs_i = node_management_add_nodes_items_it__rreqs_i -
      1;
   *node_management_add_nodes_items_it__continue = (0 < node_management_add_nodes_items_it__rreqs_i);
}

