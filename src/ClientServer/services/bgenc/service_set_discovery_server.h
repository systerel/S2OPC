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

 File Name            : service_set_discovery_server.h

 Date                 : 02/12/2019 10:19:11

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_set_discovery_server_h
#define _service_set_discovery_server_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_register_server2.h"
#include "service_find_servers_bs.h"
#include "service_register_server2.h"
#include "service_set_discovery_server_data_bs.h"

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
extern void service_set_discovery_server__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_set_discovery_server__treat_find_servers_request service_find_servers_bs__treat_find_servers_request

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_set_discovery_server__service_set_discovery_server_UNINITIALISATION(void);
extern void service_set_discovery_server__treat_find_servers_on_network_request(
   const constants__t_msg_i service_set_discovery_server__req_msg,
   const constants__t_msg_i service_set_discovery_server__resp_msg,
   const constants__t_endpoint_config_idx_i service_set_discovery_server__endpoint_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_discovery_server__ret);
extern void service_set_discovery_server__treat_register_server2_request(
   const constants__t_msg_i service_set_discovery_server__req_msg,
   const constants__t_msg_i service_set_discovery_server__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_discovery_server__ret);

#endif
