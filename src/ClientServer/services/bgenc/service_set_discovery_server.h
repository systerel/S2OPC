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

 Date                 : 04/08/2022 14:53:13

 C Translator Version : tradc Java V1.2 (06/02/2022)

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
#include "msg_find_servers_bs.h"
#include "msg_find_servers_on_network_bs.h"
#include "msg_register_server2.h"
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

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_set_discovery_server__local_add_self_server_to_return(
   const constants__t_endpoint_config_idx_i service_set_discovery_server__p_endpoint_config_idx,
   const t_entier4 service_set_discovery_server__p_nbServerUri,
   const constants__t_ServerUris service_set_discovery_server__p_ServerUris,
   const t_entier4 service_set_discovery_server__p_nbServersIn,
   t_bool * const service_set_discovery_server__p_compatSelf,
   constants__t_ApplicationDescription_i * const service_set_discovery_server__p_appDesc,
   t_entier4 * const service_set_discovery_server__p_nbServersOut);
extern void service_set_discovery_server__local_get_nb_servers_on_network_to_return(
   const t_entier4 service_set_discovery_server__p_starting_record_id,
   const t_entier4 service_set_discovery_server__p_max_records_to_return,
   const constants__t_ServerCapabilities service_set_discovery_server__p_serverCapabilities,
   t_entier4 * const service_set_discovery_server__p_nb_servers);
extern void service_set_discovery_server__local_get_nb_servers_to_return(
   const t_entier4 service_set_discovery_server__p_nbServerUri,
   const constants__t_ServerUris service_set_discovery_server__p_ServerUris,
   t_entier4 * const service_set_discovery_server__p_nb_servers);
extern void service_set_discovery_server__local_set_servers_on_network_to_return(
   const constants__t_msg_i service_set_discovery_server__p_resp,
   const t_entier4 service_set_discovery_server__p_starting_record_id,
   const constants__t_ServerCapabilities service_set_discovery_server__p_serverCapabilities,
   const t_entier4 service_set_discovery_server__p_nb_servers);
extern void service_set_discovery_server__local_set_servers_to_return(
   const constants__t_msg_i service_set_discovery_server__p_resp,
   const constants__t_LocaleIds_i service_set_discovery_server__p_localeIds,
   const t_entier4 service_set_discovery_server__p_nbServerUri,
   const constants__t_ServerUris service_set_discovery_server__p_ServerUris,
   const t_entier4 service_set_discovery_server__p_nb_servers,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_discovery_server__ret);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_set_discovery_server__service_set_discovery_server_UNINITIALISATION(void);
extern void service_set_discovery_server__treat_find_servers_on_network_request(
   const constants__t_msg_i service_set_discovery_server__req_msg,
   const constants__t_msg_i service_set_discovery_server__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_discovery_server__ret);
extern void service_set_discovery_server__treat_find_servers_request(
   const constants__t_msg_i service_set_discovery_server__req_msg,
   const constants__t_msg_i service_set_discovery_server__resp_msg,
   const constants__t_endpoint_config_idx_i service_set_discovery_server__endpoint_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_discovery_server__ret);
extern void service_set_discovery_server__treat_register_server2_request(
   const constants__t_msg_i service_set_discovery_server__req_msg,
   const constants__t_msg_i service_set_discovery_server__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_set_discovery_server__ret);

#endif
