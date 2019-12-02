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

 File Name            : service_register_server2.h

 Date                 : 03/12/2019 10:31:10

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_register_server2_h
#define _service_register_server2_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "service_register_server2_set_it_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "service_set_discovery_server_data_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 service_register_server2__RegisteredServer2_Counter_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_register_server2__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_register_server2__clear_iter_registered_server2_set service_register_server2_set_it_bs__clear_iter_registered_server2_set
#define service_register_server2__continue_iter_monitored_item service_register_server2_set_it_bs__continue_iter_monitored_item
#define service_register_server2__get_registered_server2_counter_reset_time service_register_server2_set_it_bs__get_registered_server2_counter_reset_time
#define service_register_server2__get_registered_server2_mdns_config service_register_server2_set_it_bs__get_registered_server2_mdns_config
#define service_register_server2__get_registered_server2_recordId service_register_server2_set_it_bs__get_registered_server2_recordId
#define service_register_server2__get_registered_server2_registered_server service_register_server2_set_it_bs__get_registered_server2_registered_server
#define service_register_server2__init_iter_registered_server2_set service_register_server2_set_it_bs__init_iter_registered_server2_set

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_register_server2__local_need_register_server2_update(
   const constants__t_RegisteredServer_i service_register_server2__p_registered_server,
   t_bool * const service_register_server2__hasServerUri,
   t_entier4 * const service_register_server2__recordId);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_register_server2__register_server2_create_or_update(
   const constants__t_RegisteredServer_i service_register_server2__p_registered_server,
   const constants__t_MdnsDiscoveryConfig_i service_register_server2__p_mdns_config,
   constants_statuscodes_bs__t_StatusCode_i * const service_register_server2__p_sc);
extern void service_register_server2__register_server2_remove(
   const constants__t_RegisteredServer_i service_register_server2__p_registered_server);
extern void service_register_server2__service_register_server2_UNINITIALISATION(void);

#endif
