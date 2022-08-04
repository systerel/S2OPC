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

 File Name            : service_register_nodes.h

 Date                 : 04/08/2022 14:53:12

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_register_nodes_h
#define _service_register_nodes_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_register_nodes.h"
#include "register_nodes_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_register_nodes__INITIALISATION(void);

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_register_nodes__fill_response_register_nodes(
   const constants__t_msg_i service_register_nodes__p_req_msg,
   const constants__t_msg_i service_register_nodes__p_resp_msg,
   const t_entier4 service_register_nodes__p_nb_nodes,
   t_bool * const service_register_nodes__p_bres);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_register_nodes__treat_register_nodes_request(
   const constants__t_msg_i service_register_nodes__p_req_msg,
   const constants__t_msg_i service_register_nodes__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_register_nodes__ret);

#endif
