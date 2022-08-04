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

 File Name            : msg_unregister_nodes_bs.h

 Date                 : 04/08/2022 14:53:42

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_unregister_nodes_bs_h
#define _msg_unregister_nodes_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_unregister_nodes_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_unregister_nodes_bs__get_msg_unregister_nodes_req_nb_nodes(
   const constants__t_msg_i msg_unregister_nodes_bs__p_req_msg,
   t_entier4 * const msg_unregister_nodes_bs__p_nb_nodes);
extern void msg_unregister_nodes_bs__get_msg_unregister_nodes_req_node_id(
   const constants__t_msg_i msg_unregister_nodes_bs__p_req_msg,
   const t_entier4 msg_unregister_nodes_bs__p_index,
   constants__t_NodeId_i * const msg_unregister_nodes_bs__p_node_id);

#endif
