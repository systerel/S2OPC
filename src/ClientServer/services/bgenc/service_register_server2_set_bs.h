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

 File Name            : service_register_server2_set_bs.h

 Date                 : 04/08/2022 14:53:45

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_register_server2_set_bs_h
#define _service_register_server2_set_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_register_server2_set_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_register_server2_set_bs__add_to_registered_server2_set(
   const constants__t_RegisteredServer_i service_register_server2_set_bs__p_registered_server,
   const constants__t_MdnsDiscoveryConfig_i service_register_server2_set_bs__p_mdns_config,
   const t_entier4 service_register_server2_set_bs__p_recordId,
   t_bool * const service_register_server2_set_bs__alloc_success);
extern void service_register_server2_set_bs__clear_iter_registered_server2_set(void);
extern void service_register_server2_set_bs__continue_iter_registered_server2_set(
   t_bool * const service_register_server2_set_bs__continue,
   constants__t_RegisteredServer2Info_i * const service_register_server2_set_bs__p_registeredServerInfo);
extern void service_register_server2_set_bs__get_card_register2_set(
   t_entier4 * const service_register_server2_set_bs__card_set);
extern void service_register_server2_set_bs__get_registered_server2_counter_reset_time(
   constants__t_Timestamp * const service_register_server2_set_bs__p_timestamp);
extern void service_register_server2_set_bs__get_registered_server2_mdns_config(
   const constants__t_RegisteredServer2Info_i service_register_server2_set_bs__p_registeredServerInfo,
   constants__t_MdnsDiscoveryConfig_i * const service_register_server2_set_bs__p_mdns_config);
extern void service_register_server2_set_bs__get_registered_server2_recordId(
   const constants__t_RegisteredServer2Info_i service_register_server2_set_bs__p_registeredServerInfo,
   t_entier4 * const service_register_server2_set_bs__p_recordId);
extern void service_register_server2_set_bs__get_registered_server2_registered_server(
   const constants__t_RegisteredServer2Info_i service_register_server2_set_bs__p_registeredServerInfo,
   constants__t_RegisteredServer_i * const service_register_server2_set_bs__p_registered_server);
extern void service_register_server2_set_bs__init_iter_registered_server2_set(
   t_bool * const service_register_server2_set_bs__continue);
extern void service_register_server2_set_bs__remove_from_registered_server2_set(
   const t_entier4 service_register_server2_set_bs__p_recordId);
extern void service_register_server2_set_bs__reset_registered_server2_set(void);
extern void service_register_server2_set_bs__service_register_server2_set_bs_UNINITIALISATION(void);

#endif
