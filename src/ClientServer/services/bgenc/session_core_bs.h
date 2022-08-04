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

 File Name            : session_core_bs.h

 Date                 : 04/08/2022 14:53:47

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _session_core_bs_h
#define _session_core_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

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
extern void session_core_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_bs__clear_Signature(
   const constants__t_session_i session_core_bs__p_session,
   const t_bool session_core_bs__p_is_client,
   const constants__t_SignatureData_i session_core_bs__p_signature);
extern void session_core_bs__client_activate_session_req_do_crypto(
   const constants__t_session_i session_core_bs__session,
   const constants__t_channel_config_idx_i session_core_bs__channel_config_idx,
   const constants__t_Nonce_i session_core_bs__server_nonce,
   t_bool * const session_core_bs__valid,
   constants__t_SignatureData_i * const session_core_bs__signature);
extern void session_core_bs__client_activate_session_resp_check(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   const constants__t_msg_i session_core_bs__p_resp_msg,
   t_bool * const session_core_bs__valid);
extern void session_core_bs__client_close_session_req_msg(
   const constants__t_msg_i session_core_bs__req_msg);
extern void session_core_bs__client_close_session_resp_msg(
   const constants__t_msg_i session_core_bs__resp_msg);
extern void session_core_bs__client_create_session_check_crypto(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   const constants__t_msg_i session_core_bs__p_resp_msg,
   t_bool * const session_core_bs__p_valid);
extern void session_core_bs__client_create_session_req_do_crypto(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_channel_i session_core_bs__p_channel,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   t_bool * const session_core_bs__valid,
   t_bool * const session_core_bs__nonce_needed);
extern void session_core_bs__client_create_session_set_user_token_secu_properties(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   const constants__t_msg_i session_core_bs__p_resp_msg,
   t_bool * const session_core_bs__p_valid);
extern void session_core_bs__client_gen_activate_orphaned_session_internal_event(
   const constants__t_session_i session_core_bs__session,
   const constants__t_channel_config_idx_i session_core_bs__channel_config_idx);
extern void session_core_bs__client_gen_activate_user_session_internal_event(
   const constants__t_session_i session_core_bs__session,
   const constants__t_user_token_i session_core_bs__p_user_token);
extern void session_core_bs__client_gen_create_session_internal_event(
   const constants__t_session_i session_core_bs__session,
   const constants__t_channel_config_idx_i session_core_bs__channel_config_idx);
extern void session_core_bs__client_get_token_from_session(
   const constants__t_session_i session_core_bs__session,
   constants__t_session_token_i * const session_core_bs__session_token);
extern void session_core_bs__client_set_NonceServer(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_msg_i session_core_bs__p_resp_msg);
extern void session_core_bs__client_set_session_token(
   const constants__t_session_i session_core_bs__session,
   const constants__t_session_token_i session_core_bs__token);
extern void session_core_bs__delete_session_application_context(
   const constants__t_session_i session_core_bs__p_session);
extern void session_core_bs__delete_session_token(
   const constants__t_session_i session_core_bs__p_session,
   const t_bool session_core_bs__p_is_client);
extern void session_core_bs__drop_NonceClient(
   const constants__t_session_i session_core_bs__p_session);
extern void session_core_bs__drop_user_server(
   const constants__t_session_i session_core_bs__p_session);
extern void session_core_bs__get_NonceClient(
   const constants__t_session_i session_core_bs__p_session,
   constants__t_Nonce_i * const session_core_bs__nonce);
extern void session_core_bs__get_NonceServer(
   const constants__t_session_i session_core_bs__p_session,
   const t_bool session_core_bs__p_is_client,
   constants__t_Nonce_i * const session_core_bs__nonce);
extern void session_core_bs__get_session_app_context(
   const constants__t_session_i session_core_bs__p_session,
   constants__t_session_application_context_i * const session_core_bs__p_app_context);
extern void session_core_bs__get_session_user_client(
   const constants__t_session_i session_core_bs__session,
   constants__t_user_token_i * const session_core_bs__p_user_token);
extern void session_core_bs__get_session_user_secu_client(
   const constants__t_session_i session_core_bs__session,
   constants__t_SecurityPolicy * const session_core_bs__p_user_secu);
extern void session_core_bs__get_session_user_server(
   const constants__t_session_i session_core_bs__session,
   constants__t_user_i * const session_core_bs__p_user);
extern void session_core_bs__get_session_user_server_certificate(
   const constants__t_session_i session_core_bs__session,
   constants__t_byte_buffer_i * const session_core_bs__p_user_server_cert);
extern void session_core_bs__is_same_user_server(
   const constants__t_user_i session_core_bs__p_user_left,
   const constants__t_user_i session_core_bs__p_user_right,
   t_bool * const session_core_bs__p_bres);
extern void session_core_bs__may_validate_server_certificate(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   const constants__t_byte_buffer_i session_core_bs__p_user_server_cert,
   const constants__t_SecurityPolicy session_core_bs__p_user_secu_policy,
   t_bool * const session_core_bs__valid_cert);
extern void session_core_bs__notify_set_session_state(
   const constants__t_session_i session_core_bs__session,
   const constants__t_sessionState session_core_bs__prec_state,
   const constants__t_sessionState session_core_bs__state,
   const constants_statuscodes_bs__t_StatusCode_i session_core_bs__sc_reason,
   const t_bool session_core_bs__is_client);
extern void session_core_bs__remove_NonceServer(
   const constants__t_session_i session_core_bs__p_session,
   const t_bool session_core_bs__p_is_client);
extern void session_core_bs__server_activate_session_check_crypto(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_channel_i session_core_bs__p_channel,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   const constants__t_msg_i session_core_bs__p_activate_req_msg,
   t_bool * const session_core_bs__p_valid);
extern void session_core_bs__server_close_session_check_req(
   const constants__t_msg_i session_core_bs__req_msg,
   const constants__t_msg_i session_core_bs__resp_msg);
extern void session_core_bs__server_create_session_req_do_crypto(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_msg_i session_core_bs__p_req_msg,
   const constants__t_endpoint_config_idx_i session_core_bs__p_endpoint_config_idx,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const session_core_bs__status,
   constants__t_SignatureData_i * const session_core_bs__signature);
extern void session_core_bs__server_get_fresh_session_token(
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   const constants__t_session_i session_core_bs__session,
   constants__t_session_token_i * const session_core_bs__token);
extern void session_core_bs__server_get_session_from_token(
   const constants__t_session_token_i session_core_bs__session_token,
   constants__t_session_i * const session_core_bs__session);
extern void session_core_bs__server_is_valid_session_token(
   const constants__t_session_token_i session_core_bs__token,
   t_bool * const session_core_bs__ret);
extern void session_core_bs__server_may_need_user_token_encryption(
   const constants__t_endpoint_config_idx_i session_core_bs__p_endpoint_config_idx,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   t_bool * const session_core_bs__p_bres);
extern void session_core_bs__server_session_timeout_evaluation(
   const constants__t_session_i session_core_bs__session,
   t_bool * const session_core_bs__expired);
extern void session_core_bs__server_session_timeout_msg_received(
   const constants__t_session_i session_core_bs__session);
extern void session_core_bs__server_session_timeout_start_timer(
   const constants__t_session_i session_core_bs__session,
   const constants__t_msg_i session_core_bs__resp_msg,
   t_bool * const session_core_bs__timer_created);
extern void session_core_bs__server_session_timeout_stop_timer(
   const constants__t_session_i session_core_bs__session);
extern void session_core_bs__server_set_fresh_nonce(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_channel_config_idx_i session_core_bs__p_channel_config_idx,
   t_bool * const session_core_bs__p_bres,
   constants__t_Nonce_i * const session_core_bs__p_nonce);
extern void session_core_bs__session_do_nothing(
   const constants__t_session_i session_core_bs__session);
extern void session_core_bs__set_session_app_context(
   const constants__t_session_i session_core_bs__p_session,
   const constants__t_session_application_context_i session_core_bs__p_app_context);
extern void session_core_bs__set_session_user_client(
   const constants__t_session_i session_core_bs__session,
   const constants__t_user_token_i session_core_bs__p_user_token);
extern void session_core_bs__set_session_user_server(
   const constants__t_session_i session_core_bs__session,
   const constants__t_user_i session_core_bs__p_user);

#endif
