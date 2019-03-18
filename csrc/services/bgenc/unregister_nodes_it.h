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

 File Name            : unregister_nodes_it.h

 Date                 : 29/01/2019 09:56:50

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _unregister_nodes_it_h
#define _unregister_nodes_it_h

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
extern t_entier4 unregister_nodes_it__rreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void unregister_nodes_it__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void unregister_nodes_it__continue_iter_unregister_nodes_request(
   t_bool * const unregister_nodes_it__continue,
   t_entier4 * const unregister_nodes_it__index);
extern void unregister_nodes_it__init_iter_unregister_nodes_request(
   const t_entier4 unregister_nodes_it__nb_req,
   t_bool * const unregister_nodes_it__continue);

#endif
