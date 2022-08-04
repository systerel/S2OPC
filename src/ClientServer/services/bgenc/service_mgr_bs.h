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

 File Name            : service_mgr_bs.h

 Date                 : 04/08/2022 14:53:45

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_mgr_bs_h
#define _service_mgr_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_mgr_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_mgr_bs__client_async_discovery_request_without_channel(
   const constants__t_channel_config_idx_i service_mgr_bs__channel_config_idx,
   const constants__t_msg_i service_mgr_bs__req_msg,
   const constants__t_application_context_i service_mgr_bs__app_context,
   t_bool * const service_mgr_bs__bres);
extern void service_mgr_bs__client_channel_connected_event_discovery(
   const constants__t_channel_config_idx_i service_mgr_bs__channel_config_idx,
   const constants__t_channel_i service_mgr_bs__channel);
extern void service_mgr_bs__client_discovery_req_failures_on_final_connection_failure(
   const constants__t_channel_config_idx_i service_mgr_bs__channel_config_idx);
extern void service_mgr_bs__service_mgr_bs_UNINITIALISATION(void);

#endif
