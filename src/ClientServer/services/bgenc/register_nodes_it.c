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

 File Name            : register_nodes_it.c

 Date                 : 04/08/2022 14:53:09

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "register_nodes_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 register_nodes_it__rreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void register_nodes_it__INITIALISATION(void) {
   register_nodes_it__rreqs_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void register_nodes_it__init_iter_register_nodes_request(
   const t_entier4 register_nodes_it__nb_req,
   t_bool * const register_nodes_it__continue) {
   register_nodes_it__rreqs_i = register_nodes_it__nb_req;
   *register_nodes_it__continue = (0 < register_nodes_it__nb_req);
}

void register_nodes_it__continue_iter_register_nodes_request(
   t_bool * const register_nodes_it__continue,
   t_entier4 * const register_nodes_it__index) {
   *register_nodes_it__index = register_nodes_it__rreqs_i;
   register_nodes_it__rreqs_i = register_nodes_it__rreqs_i -
      1;
   *register_nodes_it__continue = (0 < register_nodes_it__rreqs_i);
}

