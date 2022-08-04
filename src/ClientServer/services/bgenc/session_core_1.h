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

 File Name            : session_core_1.h

 Date                 : 04/08/2022 14:53:17

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _session_core_1_h
#define _session_core_1_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "session_core_1_it.h"
#include "session_core_2.h"
#include "session_core_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core_1__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define session_core_1__clear_Signature session_core_bs__clear_Signature
#define session_core_1__client_activate_session_req_do_crypto session_core_bs__client_activate_session_req_do_crypto
#define session_core_1__client_activate_session_resp_check session_core_bs__client_activate_session_resp_check
#define session_core_1__client_close_session_req_msg session_core_bs__client_close_session_req_msg
#define session_core_1__client_close_session_resp_msg session_core_bs__client_close_session_resp_msg
#define session_core_1__client_create_session_check_crypto session_core_bs__client_create_session_check_crypto
#define session_core_1__client_create_session_req_do_crypto session_core_bs__client_create_session_req_do_crypto
#define session_core_1__client_create_session_set_user_token_secu_properties session_core_bs__client_create_session_set_user_token_secu_properties
#define session_core_1__client_gen_activate_orphaned_session_internal_event session_core_bs__client_gen_activate_orphaned_session_internal_event
#define session_core_1__client_gen_activate_user_session_internal_event session_core_bs__client_gen_activate_user_session_internal_event
#define session_core_1__client_gen_create_session_internal_event session_core_bs__client_gen_create_session_internal_event
#define session_core_1__client_get_token_from_session session_core_bs__client_get_token_from_session
#define session_core_1__client_set_NonceServer session_core_bs__client_set_NonceServer
#define session_core_1__client_set_session_token session_core_bs__client_set_session_token
#define session_core_1__drop_NonceClient session_core_bs__drop_NonceClient
#define session_core_1__drop_user_server session_core_bs__drop_user_server
#define session_core_1__get_NonceClient session_core_bs__get_NonceClient
#define session_core_1__get_NonceServer session_core_bs__get_NonceServer
#define session_core_1__get_channel_session session_core_2__get_channel_session
#define session_core_1__get_server_session_preferred_locales session_core_2__get_server_session_preferred_locales
#define session_core_1__get_session_app_context session_core_bs__get_session_app_context
#define session_core_1__get_session_channel session_core_2__get_session_channel
#define session_core_1__get_session_state session_core_2__get_session_state
#define session_core_1__get_session_user_client session_core_bs__get_session_user_client
#define session_core_1__get_session_user_secu_client session_core_bs__get_session_user_secu_client
#define session_core_1__get_session_user_server session_core_bs__get_session_user_server
#define session_core_1__get_session_user_server_certificate session_core_bs__get_session_user_server_certificate
#define session_core_1__getall_orphaned session_core_2__getall_orphaned
#define session_core_1__getall_session_channel session_core_2__getall_session_channel
#define session_core_1__getall_to_create session_core_2__getall_to_create
#define session_core_1__is_same_user_server session_core_bs__is_same_user_server
#define session_core_1__is_valid_session session_core_2__is_valid_session
#define session_core_1__may_validate_server_certificate session_core_bs__may_validate_server_certificate
#define session_core_1__reset_session_to_create session_core_2__reset_session_to_create
#define session_core_1__server_activate_session_check_crypto session_core_bs__server_activate_session_check_crypto
#define session_core_1__server_close_session_check_req session_core_bs__server_close_session_check_req
#define session_core_1__server_create_session_req_do_crypto session_core_bs__server_create_session_req_do_crypto
#define session_core_1__server_get_fresh_session_token session_core_bs__server_get_fresh_session_token
#define session_core_1__server_get_session_from_token session_core_bs__server_get_session_from_token
#define session_core_1__server_is_valid_session_token session_core_bs__server_is_valid_session_token
#define session_core_1__server_may_need_user_token_encryption session_core_bs__server_may_need_user_token_encryption
#define session_core_1__server_session_timeout_evaluation session_core_bs__server_session_timeout_evaluation
#define session_core_1__server_session_timeout_msg_received session_core_bs__server_session_timeout_msg_received
#define session_core_1__server_session_timeout_start_timer session_core_bs__server_session_timeout_start_timer
#define session_core_1__server_set_fresh_nonce session_core_bs__server_set_fresh_nonce
#define session_core_1__session_do_nothing session_core_bs__session_do_nothing
#define session_core_1__set_session_app_context session_core_bs__set_session_app_context
#define session_core_1__set_session_channel session_core_2__set_session_channel
#define session_core_1__set_session_to_create session_core_2__set_session_to_create
#define session_core_1__set_session_user_client session_core_bs__set_session_user_client
#define session_core_1__set_session_user_server session_core_bs__set_session_user_server

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void session_core_1__l_reset_server_session_preferred_locales(
   const constants__t_session_i session_core_1__p_session);
extern void session_core_1__l_set_session_state(
   const constants__t_session_i session_core_1__p_session,
   const constants__t_sessionState session_core_1__p_state,
   const t_bool session_core_1__is_client);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_1__create_session(
   const constants__t_session_i session_core_1__session,
   const constants__t_channel_i session_core_1__channel,
   const constants__t_sessionState session_core_1__state,
   const t_bool session_core_1__is_client);
extern void session_core_1__init_new_session(
   const t_bool session_core_1__is_client,
   constants__t_session_i * const session_core_1__p_session);
extern void session_core_1__set_server_session_preferred_locales_or_indet(
   const constants__t_session_i session_core_1__p_session,
   const constants__t_LocaleIds_i session_core_1__p_localesIds);
extern void session_core_1__set_session_orphaned(
   const constants__t_session_i session_core_1__session,
   const constants__t_channel_config_idx_i session_core_1__channel_config_idx);
extern void session_core_1__set_session_state(
   const constants__t_session_i session_core_1__session,
   const constants__t_sessionState session_core_1__state,
   const t_bool session_core_1__is_client);
extern void session_core_1__set_session_state_closed(
   const constants__t_session_i session_core_1__session,
   const constants_statuscodes_bs__t_StatusCode_i session_core_1__sc_reason,
   const t_bool session_core_1__is_client);

#endif
