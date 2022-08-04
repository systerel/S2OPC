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

 File Name            : msg_find_servers_on_network_bs.h

 Date                 : 04/08/2022 14:53:36

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_find_servers_on_network_bs_h
#define _msg_find_servers_on_network_bs_h

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
extern void msg_find_servers_on_network_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_find_servers_on_network_bs__alloc_find_servers_on_network_servers(
   const constants__t_msg_i msg_find_servers_on_network_bs__p_resp,
   const t_entier4 msg_find_servers_on_network_bs__p_nb_servers,
   t_bool * const msg_find_servers_on_network_bs__p_allocSuccess);
extern void msg_find_servers_on_network_bs__get_find_servers_on_network_req_params(
   const constants__t_msg_i msg_find_servers_on_network_bs__p_req,
   t_entier4 * const msg_find_servers_on_network_bs__p_startingRecordId,
   t_entier4 * const msg_find_servers_on_network_bs__p_maxRecordsToReturn,
   constants__t_ServerCapabilities * const msg_find_servers_on_network_bs__p_serverCapabilities);
extern void msg_find_servers_on_network_bs__set_find_servers_on_network_resp(
   const constants__t_msg_i msg_find_servers_on_network_bs__p_resp,
   const constants__t_Timestamp msg_find_servers_on_network_bs__p_counterResetTime);
extern void msg_find_servers_on_network_bs__set_find_servers_on_network_server(
   const constants__t_msg_i msg_find_servers_on_network_bs__p_resp,
   const t_entier4 msg_find_servers_on_network_bs__p_srv_index,
   const t_entier4 msg_find_servers_on_network_bs__p_recordId,
   const constants__t_RegisteredServer_i msg_find_servers_on_network_bs__p_registered_server,
   const constants__t_MdnsDiscoveryConfig_i msg_find_servers_on_network_bs__p_mdns_config);

#endif
