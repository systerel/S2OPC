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

 File Name            : session_core_3.h

 Date                 : 17/11/2026 15:32:45

 C Translator Version : tradc Java V2.0 (2026-09-14)

******************************************************************************/

#ifndef _session_core_3_h
#define _session_core_3_h

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
extern constants__t_channel_i session_core_3__a_channel_i[constants__t_session_i_max+1];
extern t_entier4 session_core_3__a_channel_nb_sessions_i[constants__t_channel_i_max+1];
extern constants__t_channel_config_idx_i session_core_3__a_client_orphaned_i[constants__t_session_i_max+1];
extern t_bool session_core_3__a_client_session_i[constants__t_session_i_max+1];
extern constants__t_channel_config_idx_i session_core_3__a_client_to_create_i[constants__t_session_i_max+1];
extern constants__t_ApplicationDescription_i session_core_3__a_server_client_app_desc_i[constants__t_session_i_max+1];
extern constants__t_CertThumbprint_i session_core_3__a_server_client_cert_tb_i[constants__t_session_i_max+1];
extern constants__t_LocaleIds_i session_core_3__a_server_client_locales_i[constants__t_session_i_max+1];
extern constants__t_SessionName_i session_core_3__a_server_session_name_i[constants__t_session_i_max+1];
extern t_entier4 session_core_3__a_server_user_auth_attempts_i[constants__t_session_i_max+1];
extern constants__t_timeref_i session_core_3__a_session_init_time_i[constants__t_session_i_max+1];
extern constants__t_sessionRoles_i session_core_3__a_session_roles_i[constants__t_session_i_max+1];
extern constants__t_sessionState_i session_core_3__a_state_i[constants__t_session_i_max+1];
extern t_entier4 session_core_3__card_s_session_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core_3__INITIALISATION(void);

/*---------------------
   Inlined OPERATIONS
  ---------------------*/
#define session_core_3__get_session_roles(session_core_3__p_session, \
   session_core_3__p_session_roles) { \
   *session_core_3__p_session_roles = session_core_3__a_session_roles_i[session_core_3__p_session]; \
}

#define session_core_3__get_server_session_name(session_core_3__p_session, \
   session_core_3__p_sessionName) { \
   *session_core_3__p_sessionName = session_core_3__a_server_session_name_i[session_core_3__p_session]; \
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_3__decrease_card_s_session(void);
extern void session_core_3__decrease_channel_nb_sessions(
   const constants__t_channel_i session_core_3__p_channel);
extern void session_core_3__get_card_s_session(
   t_entier4 * const session_core_3__p_nb_sessions);
extern void session_core_3__get_channel_nb_sessions(
   const constants__t_channel_i session_core_3__p_channel,
   t_entier4 * const session_core_3__p_nb_sessions);
extern void session_core_3__get_init_time(
   const constants__t_session_i session_core_3__p_session,
   constants__t_timeref_i * const session_core_3__p_timeref);
extern void session_core_3__get_server_session_client_app_desc(
   const constants__t_session_i session_core_3__p_session,
   constants__t_ApplicationDescription_i * const session_core_3__p_cliAppDesc);
extern void session_core_3__get_server_session_client_cert_tb(
   const constants__t_session_i session_core_3__p_session,
   constants__t_CertThumbprint_i * const session_core_3__p_cliCertTb);
extern void session_core_3__get_server_session_preferred_locales(
   const constants__t_session_i session_core_3__p_session,
   constants__t_LocaleIds_i * const session_core_3__p_localeIds);
extern void session_core_3__get_server_session_user_auth_attempts(
   const constants__t_session_i session_core_3__p_session,
   t_entier4 * const session_core_3__p_attempts);
extern void session_core_3__get_session_channel(
   const constants__t_session_i session_core_3__session,
   constants__t_channel_i * const session_core_3__channel);
extern void session_core_3__get_session_state(
   const constants__t_session_i session_core_3__session,
   constants__t_sessionState_i * const session_core_3__state);
extern void session_core_3__getall_orphaned(
   const constants__t_session_i session_core_3__p_session,
   t_bool * const session_core_3__p_dom,
   constants__t_channel_config_idx_i * const session_core_3__p_channel_config_idx);
extern void session_core_3__getall_session_channel(
   const constants__t_session_i session_core_3__p_session,
   t_bool * const session_core_3__p_dom,
   constants__t_channel_i * const session_core_3__p_channel);
extern void session_core_3__getall_to_create(
   const constants__t_session_i session_core_3__p_session,
   t_bool * const session_core_3__p_dom,
   constants__t_channel_config_idx_i * const session_core_3__p_channel_config_idx);
extern void session_core_3__increase_card_s_session(void);
extern void session_core_3__increase_channel_nb_sessions(
   const constants__t_channel_i session_core_3__p_channel);
extern void session_core_3__is_client_session(
   const constants__t_session_i session_core_3__p_session,
   t_bool * const session_core_3__ret);
extern void session_core_3__is_valid_session(
   const constants__t_session_i session_core_3__session,
   t_bool * const session_core_3__ret);
extern void session_core_3__reset_channel(
   const constants__t_session_i session_core_3__p_session);
extern void session_core_3__reset_init_time(
   const constants__t_session_i session_core_3__p_session);
extern void session_core_3__reset_server_client_create_session_info(
   const constants__t_session_i session_core_3__p_session,
   constants__t_ApplicationDescription_i * const session_core_3__p_cliAppDesc,
   constants__t_CertThumbprint_i * const session_core_3__p_certTb,
   constants__t_SessionName_i * const session_core_3__p_sessionName);
extern void session_core_3__reset_server_session_preferred_locales(
   const constants__t_session_i session_core_3__p_session,
   constants__t_LocaleIds_i * const session_core_3__p_localeIds);
extern void session_core_3__reset_server_session_roles(
   const constants__t_session_i session_core_3__p_session,
   constants__t_sessionRoles_i * const session_core_3__p_roles);
extern void session_core_3__reset_server_session_user_auth_attempts(
   const constants__t_session_i session_core_3__p_session);
extern void session_core_3__reset_session_orphaned(
   const constants__t_session_i session_core_3__p_session);
extern void session_core_3__reset_session_state(
   const constants__t_session_i session_core_3__p_session);
extern void session_core_3__reset_session_to_create(
   const constants__t_session_i session_core_3__p_session);
extern void session_core_3__set_channel(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_channel_i session_core_3__p_channel);
extern void session_core_3__set_client_orphaned(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_channel_config_idx_i session_core_3__p_channel_config_idx);
extern void session_core_3__set_client_session(
   const constants__t_session_i session_core_3__p_session,
   const t_bool session_core_3__p_is_client);
extern void session_core_3__set_init_time(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_timeref_i session_core_3__p_init_time);
extern void session_core_3__set_server_session_client_app_desc(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_ApplicationDescription_i session_core_3__p_cliAppDesc);
extern void session_core_3__set_server_session_client_cert_tb(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_CertThumbprint_i session_core_3__p_cliCertTb);
extern void session_core_3__set_server_session_name(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_SessionName_i session_core_3__p_sessionName);
extern void session_core_3__set_server_session_preferred_locales(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_LocaleIds_i session_core_3__p_localesIds);
extern void session_core_3__set_server_session_user_auth_attempts(
   const constants__t_session_i session_core_3__p_session,
   const t_entier4 session_core_3__p_attempts);
extern void session_core_3__set_session_roles_2(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_sessionRoles_i session_core_3__p_roles);
extern void session_core_3__set_session_state_1(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_sessionState_i session_core_3__p_state);
extern void session_core_3__set_session_to_create(
   const constants__t_session_i session_core_3__p_session,
   const constants__t_channel_config_idx_i session_core_3__p_channel_config_idx);

#endif
