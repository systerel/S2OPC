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

 File Name            : msg_register_server2.h

 Date                 : 04/08/2022 14:53:08

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_register_server2_h
#define _msg_register_server2_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_register_server2_bs.h"

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
extern void msg_register_server2__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define msg_register_server2__set_register_server2_resp_configuration_results msg_register_server2_bs__set_register_server2_resp_configuration_results

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_register_server2__check_register_server2_req(
   const constants__t_msg_i msg_register_server2__p_req,
   constants_statuscodes_bs__t_StatusCode_i * const msg_register_server2__p_sc,
   t_bool * const msg_register_server2__p_is_online,
   t_entier4 * const msg_register_server2__p_nb_discovery_config,
   t_entier4 * const msg_register_server2__p_mdns_discovery_config_index,
   constants__t_RegisteredServer_i * const msg_register_server2__p_registered_server,
   constants__t_MdnsDiscoveryConfig_i * const msg_register_server2__p_mdns_config);

#endif
