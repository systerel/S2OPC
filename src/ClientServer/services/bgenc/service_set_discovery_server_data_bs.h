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

 File Name            : service_set_discovery_server_data_bs.h

 Date                 : 04/08/2022 14:53:46

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_set_discovery_server_data_bs_h
#define _service_set_discovery_server_data_bs_h

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
extern void service_set_discovery_server_data_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_set_discovery_server_data_bs__get_ApplicationDescription(
   const constants__t_endpoint_config_idx_i service_set_discovery_server_data_bs__p_endpoint_config_idx,
   constants__t_ApplicationDescription_i * const service_set_discovery_server_data_bs__p_app_desc);
extern void service_set_discovery_server_data_bs__get_ApplicationDescription_ServerUri(
   const constants__t_ApplicationDescription_i service_set_discovery_server_data_bs__p_app_desc,
   constants__t_ServerUri * const service_set_discovery_server_data_bs__p_ServerUri);
extern void service_set_discovery_server_data_bs__get_RegisteredServer_ServerUri(
   const constants__t_RegisteredServer_i service_set_discovery_server_data_bs__p_reg_server,
   constants__t_ServerUri * const service_set_discovery_server_data_bs__p_server_uri);
extern void service_set_discovery_server_data_bs__has_ServerCapabilities(
   const constants__t_MdnsDiscoveryConfig_i service_set_discovery_server_data_bs__p_mdns_config,
   const constants__t_ServerCapabilities service_set_discovery_server_data_bs__p_server_capabilities,
   t_bool * const service_set_discovery_server_data_bs__p_bool);
extern void service_set_discovery_server_data_bs__has_ServerUri(
   const constants__t_ServerUri service_set_discovery_server_data_bs__p_singleServerUri,
   const t_entier4 service_set_discovery_server_data_bs__p_nbServerUri,
   const constants__t_ServerUris service_set_discovery_server_data_bs__p_ServerUris,
   t_bool * const service_set_discovery_server_data_bs__p_bool);
extern void service_set_discovery_server_data_bs__is_empty_ServerUri(
   const constants__t_ServerUri service_set_discovery_server_data_bs__p_server_uri,
   t_bool * const service_set_discovery_server_data_bs__p_bool);
extern void service_set_discovery_server_data_bs__is_equal_ServerUri(
   const constants__t_ServerUri service_set_discovery_server_data_bs__p_left,
   const constants__t_ServerUri service_set_discovery_server_data_bs__p_right,
   t_bool * const service_set_discovery_server_data_bs__p_bool);

#endif
