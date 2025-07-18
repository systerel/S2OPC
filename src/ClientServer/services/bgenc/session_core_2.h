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

 Date                 : 18/11/2026 09:48:10

 C Translator Version : tradc Java V2.0 (2026-09-14)

******************************************************************************/

#ifndef _session_core_2_h
#define _session_core_2_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "session_core_3.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core_2__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define session_core_2__get_card_s_session session_core_3__get_card_s_session
#define session_core_2__get_channel_nb_sessions session_core_3__get_channel_nb_sessions
#define session_core_2__get_init_time session_core_3__get_init_time
#define session_core_2__get_server_session_client_app_desc session_core_3__get_server_session_client_app_desc
#define session_core_2__get_server_session_client_cert_tb session_core_3__get_server_session_client_cert_tb
#define session_core_2__get_server_session_name session_core_3__get_server_session_name
#define session_core_2__get_server_session_preferred_locales session_core_3__get_server_session_preferred_locales
#define session_core_2__get_server_session_user_auth_attempts session_core_3__get_server_session_user_auth_attempts
#define session_core_2__get_session_channel session_core_3__get_session_channel
#define session_core_2__get_session_roles session_core_3__get_session_roles
#define session_core_2__get_session_state session_core_3__get_session_state
#define session_core_2__getall_orphaned session_core_3__getall_orphaned
#define session_core_2__getall_session_channel session_core_3__getall_session_channel
#define session_core_2__getall_to_create session_core_3__getall_to_create
#define session_core_2__is_client_session session_core_3__is_client_session
#define session_core_2__is_valid_session session_core_3__is_valid_session
#define session_core_2__reset_server_client_create_session_info session_core_3__reset_server_client_create_session_info
#define session_core_2__reset_server_session_preferred_locales session_core_3__reset_server_session_preferred_locales
#define session_core_2__reset_server_session_roles session_core_3__reset_server_session_roles
#define session_core_2__reset_server_session_user_auth_attempts session_core_3__reset_server_session_user_auth_attempts
#define session_core_2__reset_session_orphaned session_core_3__reset_session_orphaned
#define session_core_2__reset_session_to_create session_core_3__reset_session_to_create
#define session_core_2__set_client_orphaned session_core_3__set_client_orphaned
#define session_core_2__set_server_session_client_app_desc session_core_3__set_server_session_client_app_desc
#define session_core_2__set_server_session_client_cert_tb session_core_3__set_server_session_client_cert_tb
#define session_core_2__set_server_session_name session_core_3__set_server_session_name
#define session_core_2__set_server_session_preferred_locales session_core_3__set_server_session_preferred_locales
#define session_core_2__set_server_session_user_auth_attempts session_core_3__set_server_session_user_auth_attempts
#define session_core_2__set_session_roles_2 session_core_3__set_session_roles_2
#define session_core_2__set_session_state_1 session_core_3__set_session_state_1
#define session_core_2__set_session_to_create session_core_3__set_session_to_create

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_2__add_session(
   const constants__t_session_i session_core_2__p_session,
   const t_bool session_core_2__p_is_client,
   const constants__t_timeref_i session_core_2__p_timeref);
extern void session_core_2__remove_session(
   const constants__t_session_i session_core_2__p_session);
extern void session_core_2__reset_session_channel(
   const constants__t_session_i session_core_2__p_session);
extern void session_core_2__set_session_channel(
   const constants__t_session_i session_core_2__session,
   const constants__t_channel_i session_core_2__p_channel);

#endif
