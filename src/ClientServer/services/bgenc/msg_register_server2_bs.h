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

 File Name            : msg_register_server2_bs.h

 Date                 : 04/08/2022 14:53:38

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_register_server2_bs_h
#define _msg_register_server2_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

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
extern void msg_register_server2_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_register_server2_bs__check_mdns_server_capabilities(
   const constants__t_MdnsDiscoveryConfig_i msg_register_server2_bs__p_mdns_discovery_configuration,
   t_bool * const msg_register_server2_bs__p_valid_server_capabilities);
extern void msg_register_server2_bs__check_mdns_server_name(
   const constants__t_MdnsDiscoveryConfig_i msg_register_server2_bs__p_mdns_discovery_configuration,
   t_bool * const msg_register_server2_bs__p_valid_mdns_server_name);
extern void msg_register_server2_bs__check_registered_discovery_url(
   const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
   t_bool * const msg_register_server2_bs__p_valid_discovery_url);
extern void msg_register_server2_bs__check_registered_product_uri(
   const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
   t_bool * const msg_register_server2_bs__p_valid_product_uri);
extern void msg_register_server2_bs__check_registered_semaphore_file(
   const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
   t_bool * const msg_register_server2_bs__p_valid_semaphore_file);
extern void msg_register_server2_bs__check_registered_server_names(
   const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
   t_bool * const msg_register_server2_bs__p_valid_server_names);
extern void msg_register_server2_bs__check_registered_server_type(
   const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
   t_bool * const msg_register_server2_bs__p_valid_server_type);
extern void msg_register_server2_bs__check_registered_server_uri(
   const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
   t_bool * const msg_register_server2_bs__p_valid_server_uri);
extern void msg_register_server2_bs__get_register_server2_req_registered_server(
   const constants__t_msg_i msg_register_server2_bs__p_req,
   constants__t_RegisteredServer_i * const msg_register_server2_bs__p_registered_server);
extern void msg_register_server2_bs__get_registered_server_is_online(
   const constants__t_RegisteredServer_i msg_register_server2_bs__p_registered_server,
   t_bool * const msg_register_server2_bs__p_is_online);
extern void msg_register_server2_bs__getall_register_server2_req_msdn_discovery_config(
   const constants__t_msg_i msg_register_server2_bs__p_req,
   t_bool * const msg_register_server2_bs__p_has_discovery_configuration,
   t_bool * const msg_register_server2_bs__p_has_one_and_only_one_mdns_config,
   constants__t_MdnsDiscoveryConfig_i * const msg_register_server2_bs__p_mdns_discovery_configuration,
   t_entier4 * const msg_register_server2_bs__p_nb_discovery_config,
   t_entier4 * const msg_register_server2_bs__p_mdns_discovery_config_index);
extern void msg_register_server2_bs__set_register_server2_resp_configuration_results(
   const constants__t_msg_i msg_register_server2_bs__p_resp,
   const t_entier4 msg_register_server2_bs__p_nb_discovery_config,
   const t_entier4 msg_register_server2_bs__p_mdns_discovery_config_index,
   constants_statuscodes_bs__t_StatusCode_i * const msg_register_server2_bs__p_sc);

#endif
