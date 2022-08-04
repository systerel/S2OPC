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

 File Name            : session_core.h

 Date                 : 04/08/2022 14:53:18

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _session_core_h
#define _session_core_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_session_bs.h"
#include "session_channel_it.h"
#include "session_core_1.h"
#include "session_core_it.h"
#include "user_authentication.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"
#include "time_reference_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define session_core__client_gen_activate_orphaned_session_internal_event session_core_1__client_gen_activate_orphaned_session_internal_event
#define session_core__client_gen_activate_user_session_internal_event session_core_1__client_gen_activate_user_session_internal_event
#define session_core__client_gen_create_session_internal_event session_core_1__client_gen_create_session_internal_event
#define session_core__client_get_token_from_session session_core_1__client_get_token_from_session
#define session_core__deallocate_user user_authentication__deallocate_user
#define session_core__drop_user_server session_core_1__drop_user_server
#define session_core__get_local_user user_authentication__get_local_user
#define session_core__get_server_session_preferred_locales session_core_1__get_server_session_preferred_locales
#define session_core__get_session_channel session_core_1__get_session_channel
#define session_core__get_session_user_client session_core_1__get_session_user_client
#define session_core__get_session_user_server session_core_1__get_session_user_server
#define session_core__getall_orphaned session_core_1__getall_orphaned
#define session_core__getall_session_channel session_core_1__getall_session_channel
#define session_core__getall_to_create session_core_1__getall_to_create
#define session_core__has_user_token_policy_available user_authentication__has_user_token_policy_available
#define session_core__is_valid_session session_core_1__is_valid_session
#define session_core__reset_session_to_create session_core_1__reset_session_to_create
#define session_core__server_get_session_from_token session_core_1__server_get_session_from_token
#define session_core__server_session_timeout_evaluation session_core_1__server_session_timeout_evaluation
#define session_core__server_session_timeout_msg_received session_core_1__server_session_timeout_msg_received
#define session_core__server_session_timeout_start_timer session_core_1__server_session_timeout_start_timer
#define session_core__set_session_app_context session_core_1__set_session_app_context
#define session_core__set_session_to_create session_core_1__set_session_to_create
#define session_core__set_session_user_client session_core_1__set_session_user_client

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void session_core__l_client_secure_channel_lost_session_sm(
   const t_bool session_core__p_dom,
   const constants__t_channel_i session_core__p_channel,
   const constants__t_channel_i session_core__p_lost_channel,
   const constants__t_session_i session_core__p_session,
   const constants__t_channel_config_idx_i session_core__p_channel_config_idx);
extern void session_core__l_server_secure_channel_lost_session_sm(
   const t_bool session_core__p_dom,
   const constants__t_channel_i session_core__p_channel,
   const constants__t_channel_i session_core__p_lost_channel,
   const constants__t_session_i session_core__p_session);
extern void session_core__server_internal_activate_req_and_resp(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_sessionState session_core__transitoryState,
   const constants__t_user_i session_core__p_user,
   const constants__t_msg_i session_core__activate_req_msg,
   const constants__t_msg_i session_core__activate_resp_msg,
   t_bool * const session_core__res_activated);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core__allocate_authenticated_user(
   const constants__t_channel_i session_core__p_channel,
   const constants__t_session_i session_core__p_session,
   const constants__t_user_token_i session_core__p_user_token,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__p_sc_valid_user,
   constants__t_user_i * const session_core__p_user);
extern void session_core__client_activate_session_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__activate_resp_msg,
   t_bool * const session_core__bret);
extern void session_core__client_close_session_req_sm(
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__close_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__ret,
   constants__t_channel_i * const session_core__channel,
   constants__t_session_token_i * const session_core__session_token);
extern void session_core__client_close_session_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__close_resp_msg);
extern void session_core__client_close_session_sm(
   const constants__t_session_i session_core__session,
   const constants_statuscodes_bs__t_StatusCode_i session_core__sc_reason);
extern void session_core__client_create_session_req_sm(
   const constants__t_session_i session_core__session,
   const constants__t_channel_i session_core__channel,
   const constants__t_msg_i session_core__create_req_msg,
   t_bool * const session_core__valid);
extern void session_core__client_create_session_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_session_token_i session_core__session_token,
   const constants__t_msg_i session_core__create_resp_msg,
   t_bool * const session_core__bret);
extern void session_core__client_init_session_sm(
   constants__t_session_i * const session_core__nsession);
extern void session_core__client_sc_activate_session_req_sm(
   const constants__t_session_i session_core__session,
   const constants__t_channel_i session_core__channel,
   const constants__t_msg_i session_core__activate_req_msg,
   constants__t_session_token_i * const session_core__session_token);
extern void session_core__client_secure_channel_lost_session_sm(
   const constants__t_channel_i session_core__p_lost_channel,
   const constants__t_channel_config_idx_i session_core__p_channel_config_idx);
extern void session_core__client_user_activate_session_req_sm(
   const constants__t_session_i session_core__session,
   const constants__t_user_token_i session_core__p_user_token,
   const constants__t_msg_i session_core__activate_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__ret,
   constants__t_channel_i * const session_core__channel,
   constants__t_session_token_i * const session_core__session_token);
extern void session_core__find_channel_to_close(
   t_bool * const session_core__p_has_channel_to_close,
   constants__t_channel_i * const session_core__p_channel_to_close);
extern void session_core__get_session_state_or_closed(
   const constants__t_session_i session_core__session,
   constants__t_sessionState * const session_core__state);
extern void session_core__getall_valid_session_channel(
   const constants__t_session_i session_core__session,
   t_bool * const session_core__bres,
   constants__t_channel_i * const session_core__channel);
extern void session_core__is_session_valid_for_service(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   t_bool * const session_core__ret);
extern void session_core__server_activate_session_req_and_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_user_i session_core__user,
   const constants__t_msg_i session_core__activate_req_msg,
   const constants__t_msg_i session_core__activate_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__ret);
extern void session_core__server_close_session_req_and_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_session_i session_core__session,
   const constants__t_msg_i session_core__close_req_msg,
   const constants__t_msg_i session_core__close_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__ret);
extern void session_core__server_close_session_sm(
   const constants__t_session_i session_core__session,
   const constants_statuscodes_bs__t_StatusCode_i session_core__sc_reason);
extern void session_core__server_create_session_req_and_resp_sm(
   const constants__t_channel_i session_core__channel,
   const constants__t_msg_i session_core__create_req_msg,
   const constants__t_msg_i session_core__create_resp_msg,
   constants__t_session_i * const session_core__nsession,
   constants_statuscodes_bs__t_StatusCode_i * const session_core__service_ret);
extern void session_core__server_secure_channel_lost_session_sm(
   const constants__t_channel_i session_core__p_lost_channel);

#endif
