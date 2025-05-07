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

 File Name            : node_management_delete_nodes_items_it.h

 Date                 : 11/02/2025 09:51:28

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _node_management_delete_nodes_items_it_h
#define _node_management_delete_nodes_items_it_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 node_management_delete_nodes_items_it__rreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void node_management_delete_nodes_items_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void node_management_delete_nodes_items_it__continue_iter_delete_nodes_items(
   t_bool * const node_management_delete_nodes_items_it__continue,
   t_entier4 * const node_management_delete_nodes_items_it__index);
extern void node_management_delete_nodes_items_it__init_iter_delete_nodes_items(
   const t_entier4 node_management_delete_nodes_items_it__nb_req,
   t_bool * const node_management_delete_nodes_items_it__continue);

#endif
