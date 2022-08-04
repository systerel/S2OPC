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

 File Name            : msg_find_servers_bs.h

 Date                 : 04/08/2022 14:53:36

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_find_servers_bs_h
#define _msg_find_servers_bs_h

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
extern void msg_find_servers_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_find_servers_bs__alloc_find_servers(
   const constants__t_msg_i msg_find_servers_bs__p_resp,
   const t_entier4 msg_find_servers_bs__p_nb_servers,
   t_bool * const msg_find_servers_bs__p_allocSuccess);
extern void msg_find_servers_bs__get_find_servers_req_params(
   const constants__t_msg_i msg_find_servers_bs__p_req,
   constants__t_LocaleIds_i * const msg_find_servers_bs__p_LocaleIds,
   t_entier4 * const msg_find_servers_bs__p_nbServerUri,
   constants__t_ServerUris * const msg_find_servers_bs__p_ServerUris);
extern void msg_find_servers_bs__set_find_servers_server(
   const constants__t_msg_i msg_find_servers_bs__p_resp,
   const t_entier4 msg_find_servers_bs__p_srv_index,
   const constants__t_LocaleIds_i msg_find_servers_bs__p_localeIds,
   const constants__t_RegisteredServer_i msg_find_servers_bs__p_registered_server,
   constants_statuscodes_bs__t_StatusCode_i * const msg_find_servers_bs__ret);
extern void msg_find_servers_bs__set_find_servers_server_ApplicationDescription(
   const constants__t_msg_i msg_find_servers_bs__p_resp,
   const t_entier4 msg_find_servers_bs__p_srv_index,
   const constants__t_LocaleIds_i msg_find_servers_bs__p_localeIds,
   const constants__t_endpoint_config_idx_i msg_find_servers_bs__p_endpoint_config_idx,
   const constants__t_ApplicationDescription_i msg_find_servers_bs__p_app_desc,
   constants_statuscodes_bs__t_StatusCode_i * const msg_find_servers_bs__ret);

#endif
