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

 File Name            : channel_mgr_bs.h

 Date                 : 04/08/2022 14:53:30

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _channel_mgr_bs_h
#define _channel_mgr_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr_1.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void channel_mgr_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void channel_mgr_bs__channel_do_nothing(
   const constants__t_channel_i channel_mgr_bs__channel);
extern void channel_mgr_bs__define_SecurityPolicy(
   const constants__t_channel_i channel_mgr_bs__p_channel);
extern void channel_mgr_bs__finalize_close_secure_channel(
   const constants__t_channel_i channel_mgr_bs__p_channel);
extern void channel_mgr_bs__get_SecurityPolicy(
   const constants__t_channel_i channel_mgr_bs__channel,
   constants__t_SecurityPolicy * const channel_mgr_bs__secpol);
extern void channel_mgr_bs__is_valid_channel_config_idx(
   const constants__t_channel_config_idx_i channel_mgr_bs__p_config_idx,
   t_bool * const channel_mgr_bs__bres);
extern void channel_mgr_bs__is_valid_endpoint_config_idx(
   const constants__t_endpoint_config_idx_i channel_mgr_bs__p_config_idx,
   t_bool * const channel_mgr_bs__bres);
extern void channel_mgr_bs__last_connected_channel_lost(
   const t_bool channel_mgr_bs__p_clientOnly);
extern void channel_mgr_bs__prepare_cli_open_secure_channel(
   const constants__t_channel_config_idx_i channel_mgr_bs__p_config_idx);
extern void channel_mgr_bs__reset_SecurityPolicy(
   const constants__t_channel_i channel_mgr_bs__channel);
extern void channel_mgr_bs__send_channel_error_msg(
   const constants__t_channel_i channel_mgr_bs__channel,
   const constants_statuscodes_bs__t_StatusCode_i channel_mgr_bs__status_code,
   const constants__t_request_context_i channel_mgr_bs__request_context);
extern void channel_mgr_bs__send_channel_msg_buffer(
   const constants__t_channel_i channel_mgr_bs__channel,
   const constants__t_byte_buffer_i channel_mgr_bs__buffer,
   const constants__t_request_context_i channel_mgr_bs__request_context);

#endif
