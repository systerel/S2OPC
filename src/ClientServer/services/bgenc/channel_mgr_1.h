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

 File Name            : channel_mgr_1.h

 Date                 : 04/08/2022 14:53:05

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _channel_mgr_1_h
#define _channel_mgr_1_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern constants__t_timeref_i channel_mgr_1__a_channel_connected_time_i[constants__t_channel_i_max+1];
extern constants__t_channel_config_idx_i channel_mgr_1__a_config_i[constants__t_channel_i_max+1];
extern constants__t_channel_i channel_mgr_1__a_config_inv_i[constants__t_channel_config_idx_i_max+1];
extern constants__t_endpoint_config_idx_i channel_mgr_1__a_endpoint_i[constants__t_channel_i_max+1];
extern t_entier4 channel_mgr_1__card_channel_connected_i;
extern t_entier4 channel_mgr_1__card_cli_channel_connecting_i;
extern t_bool channel_mgr_1__s_channel_connected_i[constants__t_channel_i_max+1];
extern t_bool channel_mgr_1__s_cli_channel_connecting_i[constants__t_channel_config_idx_i_max+1];
extern t_bool channel_mgr_1__s_cli_channel_disconnecting_i[constants__t_channel_config_idx_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void channel_mgr_1__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void channel_mgr_1__add_channel_connected(
   const constants__t_channel_i channel_mgr_1__p_channel,
   const constants__t_timeref_i channel_mgr_1__p_timeref);
extern void channel_mgr_1__add_cli_channel_connecting(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx);
extern void channel_mgr_1__add_cli_channel_disconnecting(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx);
extern void channel_mgr_1__get_card_channel_connected(
   t_entier4 * const channel_mgr_1__p_card_connected);
extern void channel_mgr_1__get_card_channel_used(
   t_entier4 * const channel_mgr_1__p_card_used);
extern void channel_mgr_1__get_card_cli_channel_connecting(
   t_entier4 * const channel_mgr_1__p_card_connecting);
extern void channel_mgr_1__get_channel_info(
   const constants__t_channel_i channel_mgr_1__channel,
   constants__t_channel_config_idx_i * const channel_mgr_1__config_idx);
extern void channel_mgr_1__get_connected_channel(
   const constants__t_channel_config_idx_i channel_mgr_1__config_idx,
   constants__t_channel_i * const channel_mgr_1__channel);
extern void channel_mgr_1__get_connection_time(
   const constants__t_channel_i channel_mgr_1__p_channel,
   constants__t_timeref_i * const channel_mgr_1__p_timeref);
extern void channel_mgr_1__getall_channel_connected(
   const constants__t_channel_i channel_mgr_1__p_channel,
   t_bool * const channel_mgr_1__p_dom,
   constants__t_channel_config_idx_i * const channel_mgr_1__p_config_idx);
extern void channel_mgr_1__getall_config_inv(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx,
   t_bool * const channel_mgr_1__p_dom,
   constants__t_channel_i * const channel_mgr_1__p_channel);
extern void channel_mgr_1__is_channel_connected(
   const constants__t_channel_i channel_mgr_1__p_channel,
   t_bool * const channel_mgr_1__p_con);
extern void channel_mgr_1__is_cli_channel_connecting(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx,
   t_bool * const channel_mgr_1__p_is_channel_connecting);
extern void channel_mgr_1__is_client_channel(
   const constants__t_channel_i channel_mgr_1__channel,
   t_bool * const channel_mgr_1__bres);
extern void channel_mgr_1__is_connected_channel(
   const constants__t_channel_i channel_mgr_1__channel,
   t_bool * const channel_mgr_1__bres);
extern void channel_mgr_1__is_disconnecting_channel(
   const constants__t_channel_config_idx_i channel_mgr_1__config_idx,
   t_bool * const channel_mgr_1__bres);
extern void channel_mgr_1__remove_channel_connected(
   const constants__t_channel_i channel_mgr_1__p_channel);
extern void channel_mgr_1__remove_cli_channel_connecting(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx);
extern void channel_mgr_1__remove_cli_channel_disconnecting(
   const constants__t_channel_config_idx_i channel_mgr_1__p_config_idx);
extern void channel_mgr_1__reset_config(
   const constants__t_channel_i channel_mgr_1__p_channel);
extern void channel_mgr_1__reset_endpoint(
   const constants__t_channel_i channel_mgr_1__p_channel);
extern void channel_mgr_1__server_get_endpoint_config(
   const constants__t_channel_i channel_mgr_1__channel,
   constants__t_endpoint_config_idx_i * const channel_mgr_1__endpoint_config_idx);
extern void channel_mgr_1__set_config(
   const constants__t_channel_i channel_mgr_1__p_channel,
   const constants__t_channel_config_idx_i channel_mgr_1__p_channel_config_idx);
extern void channel_mgr_1__set_endpoint(
   const constants__t_channel_i channel_mgr_1__p_channel,
   const constants__t_endpoint_config_idx_i channel_mgr_1__p_endpoint_config_idx);

#endif
