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

 File Name            : channel_mgr.h

 Date                 : 04/08/2022 14:53:05

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _channel_mgr_h
#define _channel_mgr_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "channel_mgr_1.h"
#include "channel_mgr_bs.h"
#include "channel_mgr_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "time_reference_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_bool channel_mgr__all_channel_closing;
extern t_bool channel_mgr__all_client_channel_closing;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void channel_mgr__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define channel_mgr__channel_do_nothing channel_mgr_bs__channel_do_nothing
#define channel_mgr__get_SecurityPolicy channel_mgr_bs__get_SecurityPolicy
#define channel_mgr__get_channel_info channel_mgr_1__get_channel_info
#define channel_mgr__get_connected_channel channel_mgr_1__get_connected_channel
#define channel_mgr__get_connection_time channel_mgr_1__get_connection_time
#define channel_mgr__is_client_channel channel_mgr_1__is_client_channel
#define channel_mgr__is_connected_channel channel_mgr_1__is_connected_channel
#define channel_mgr__is_disconnecting_channel channel_mgr_1__is_disconnecting_channel
#define channel_mgr__is_valid_channel_config_idx channel_mgr_bs__is_valid_channel_config_idx
#define channel_mgr__is_valid_endpoint_config_idx channel_mgr_bs__is_valid_endpoint_config_idx
#define channel_mgr__send_channel_error_msg channel_mgr_bs__send_channel_error_msg
#define channel_mgr__send_channel_msg_buffer channel_mgr_bs__send_channel_msg_buffer
#define channel_mgr__server_get_endpoint_config channel_mgr_1__server_get_endpoint_config

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void channel_mgr__l_check_all_channel_lost(void);
extern void channel_mgr__l_close_secure_channel(
   const constants__t_channel_i channel_mgr__p_channel);
extern void channel_mgr__l_is_new_sc_connection_allowed(
   const t_bool channel_mgr__is_one_sc_auto_closing,
   t_bool * const channel_mgr__l_allowed_new_sc);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void channel_mgr__channel_lost(
   const constants__t_channel_i channel_mgr__channel);
extern void channel_mgr__cli_open_secure_channel(
   const constants__t_channel_config_idx_i channel_mgr__config_idx,
   const t_bool channel_mgr__is_one_sc_auto_closing,
   t_bool * const channel_mgr__bres);
extern void channel_mgr__cli_set_connected_channel(
   const constants__t_channel_config_idx_i channel_mgr__config_idx,
   const constants__t_channel_i channel_mgr__channel,
   t_bool * const channel_mgr__bres);
extern void channel_mgr__cli_set_connection_timeout_channel(
   const constants__t_channel_config_idx_i channel_mgr__config_idx,
   t_bool * const channel_mgr__bres);
extern void channel_mgr__close_all_channel(
   const t_bool channel_mgr__p_clientOnly,
   t_bool * const channel_mgr__bres);
extern void channel_mgr__close_secure_channel(
   const constants__t_channel_i channel_mgr__channel);
extern void channel_mgr__is_auto_close_channel_active(
   t_bool * const channel_mgr__p_auto_closed_active);
extern void channel_mgr__srv_new_secure_channel(
   const constants__t_endpoint_config_idx_i channel_mgr__endpoint_config_idx,
   const constants__t_channel_config_idx_i channel_mgr__channel_config_idx,
   const constants__t_channel_i channel_mgr__channel,
   const t_bool channel_mgr__is_one_sc_auto_closing,
   t_bool * const channel_mgr__bres);

#endif
