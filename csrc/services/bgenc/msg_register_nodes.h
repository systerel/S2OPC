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

 File Name            : msg_register_nodes.h

 Date                 : 29/01/2019 12:57:49

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _msg_register_nodes_h
#define _msg_register_nodes_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_register_nodes_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 msg_register_nodes__nb_nodes;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_register_nodes__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define msg_register_nodes__alloc_msg_register_nodes_resp_results msg_register_nodes_bs__alloc_msg_register_nodes_resp_results
#define msg_register_nodes__get_msg_register_nodes_req_node_id msg_register_nodes_bs__get_msg_register_nodes_req_node_id
#define msg_register_nodes__setall_msg_register_nodes_resp_node_id msg_register_nodes_bs__setall_msg_register_nodes_resp_node_id

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_register_nodes__getall_msg_register_nodes_req_nb_nodes(
   const constants__t_msg_i msg_register_nodes__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_register_nodes__p_sc,
   t_entier4 * const msg_register_nodes__p_nb_nodes);

#endif
