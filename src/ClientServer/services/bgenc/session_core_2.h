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

 File Name            : session_core_2.h

 Date                 : 04/08/2022 14:53:17

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _session_core_2_h
#define _session_core_2_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern constants__t_channel_i session_core_2__a_channel_i[constants__t_session_i_max+1];
extern constants__t_channel_config_idx_i session_core_2__a_client_orphaned_i[constants__t_session_i_max+1];
extern constants__t_channel_config_idx_i session_core_2__a_client_to_create_i[constants__t_session_i_max+1];
extern constants__t_session_i session_core_2__a_reverse_channel_i[constants__t_channel_i_max+1];
extern constants__t_LocaleIds_i session_core_2__a_server_client_locales_i[constants__t_session_i_max+1];
extern constants__t_sessionState session_core_2__a_state_i[constants__t_session_i_max+1];
extern t_bool session_core_2__s_session_i[constants__t_session_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core_2__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_2__add_session(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_sessionState session_core_2__p_state);
extern void session_core_2__get_channel_session(
   const constants__t_channel_i session_core_2__p_channel,
   constants__t_session_i * const session_core_2__p_session);
extern void session_core_2__get_server_session_preferred_locales(
   const constants__t_session_i session_core_2__p_session,
   constants__t_LocaleIds_i * const session_core_2__p_localeIds);
extern void session_core_2__get_session_channel(
   const constants__t_session_i session_core_2__session,
   constants__t_channel_i * const session_core_2__channel);
extern void session_core_2__get_session_state(
   const constants__t_session_i session_core_2__session,
   constants__t_sessionState * const session_core_2__state);
extern void session_core_2__getall_orphaned(
   const constants__t_session_i session_core_2__p_session,
   t_bool * const session_core_2__p_dom,
   constants__t_channel_config_idx_i * const session_core_2__p_channel_config_idx);
extern void session_core_2__getall_session_channel(
   const constants__t_session_i session_core_2__p_session,
   t_bool * const session_core_2__p_dom,
   constants__t_channel_i * const session_core_2__p_channel);
extern void session_core_2__getall_to_create(
   const constants__t_session_i session_core_2__p_session,
   t_bool * const session_core_2__p_dom,
   constants__t_channel_config_idx_i * const session_core_2__p_channel_config_idx);
extern void session_core_2__is_valid_session(
   const constants__t_session_i session_core_2__session,
   t_bool * const session_core_2__ret);
extern void session_core_2__remove_session(
   const constants__t_session_i session_core_2__p_session);
extern void session_core_2__reset_server_session_preferred_locales(
   const constants__t_session_i session_core_2__p_session,
   constants__t_LocaleIds_i * const session_core_2__p_localeIds);
extern void session_core_2__reset_session_channel(
   const constants__t_session_i session_core_2__p_session);
extern void session_core_2__reset_session_orphaned(
   const constants__t_session_i session_core_2__p_session);
extern void session_core_2__reset_session_to_create(
   const constants__t_session_i session_core_2__p_session);
extern void session_core_2__set_server_session_preferred_locales(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_LocaleIds_i session_core_2__p_localesIds);
extern void session_core_2__set_session_channel(
   const constants__t_session_i session_core_2__session,
   const constants__t_channel_i session_core_2__channel);
extern void session_core_2__set_session_orphaned_1(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_channel_config_idx_i session_core_2__p_channel_config_idx);
extern void session_core_2__set_session_state_1(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_sessionState session_core_2__p_state);
extern void session_core_2__set_session_to_create(
   const constants__t_session_i session_core_2__p_session,
   const constants__t_channel_config_idx_i session_core_2__p_channel_config_idx);

#endif
