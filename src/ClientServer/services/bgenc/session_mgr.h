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

 File Name            : session_mgr.h

 Date                 : 04/08/2022 14:53:19

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _session_mgr_h
#define _session_mgr_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "session_core.h"
#include "session_mgr_it.h"
#include "session_request_handle_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "app_cb_call_context_bs.h"
#include "channel_mgr.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_mgr__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define session_mgr__client_secure_channel_lost_session_sm session_core__client_secure_channel_lost_session_sm
#define session_mgr__find_channel_to_close session_core__find_channel_to_close
#define session_mgr__get_local_user session_core__get_local_user
#define session_mgr__get_server_session_preferred_locales session_core__get_server_session_preferred_locales
#define session_mgr__get_session_user_server session_core__get_session_user_server
#define session_mgr__getall_valid_session_channel session_core__getall_valid_session_channel
#define session_mgr__is_valid_session session_core__is_valid_session
#define session_mgr__server_secure_channel_lost_session_sm session_core__server_secure_channel_lost_session_sm

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void session_mgr__local_client_activate_sessions_on_SC_connection(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx);
extern void session_mgr__local_client_close_session(
   const constants__t_session_i session_mgr__session,
   const constants_statuscodes_bs__t_StatusCode_i session_mgr__sc_reason);
extern void session_mgr__local_client_close_session_if_needed(
   const t_bool session_mgr__cond,
   const constants__t_session_i session_mgr__session,
   const constants_statuscodes_bs__t_StatusCode_i session_mgr__sc_reason);
extern void session_mgr__local_client_close_sessions_on_SC_final_connection_failure(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_mgr__client_async_activate_new_session_with_channel(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_user_token_i session_mgr__p_user_token,
   const constants__t_session_application_context_i session_mgr__app_context,
   t_bool * const session_mgr__bres);
extern void session_mgr__client_async_activate_new_session_without_channel(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
   const constants__t_user_token_i session_mgr__p_user_token,
   const constants__t_session_application_context_i session_mgr__app_context,
   t_bool * const session_mgr__bres);
extern void session_mgr__client_channel_connected_event_session(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
   const constants__t_channel_i session_mgr__channel);
extern void session_mgr__client_close_session(
   const constants__t_session_i session_mgr__session,
   const constants_statuscodes_bs__t_StatusCode_i session_mgr__sc_reason);
extern void session_mgr__client_close_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__close_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token);
extern void session_mgr__client_close_sessions_on_final_connection_failure(
   const constants__t_channel_config_idx_i session_mgr__channel_config_idx);
extern void session_mgr__client_create_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   const constants__t_msg_i session_mgr__create_req_msg,
   t_bool * const session_mgr__bret);
extern void session_mgr__client_receive_session_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   const constants__t_msg_type_i session_mgr__resp_typ,
   const constants__t_msg_header_i session_mgr__resp_header,
   const constants__t_msg_i session_mgr__resp_msg,
   constants__t_session_i * const session_mgr__session);
extern void session_mgr__client_sc_activate_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   const constants__t_channel_i session_mgr__channel,
   const constants__t_msg_i session_mgr__activate_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__ret,
   constants__t_session_token_i * const session_mgr__session_token);
extern void session_mgr__client_user_activate_session_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   const constants__t_user_token_i session_mgr__p_user_token,
   const constants__t_msg_i session_mgr__activate_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token);
extern void session_mgr__client_validate_session_service_req(
   const constants__t_session_i session_mgr__session,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__ret,
   constants__t_channel_i * const session_mgr__channel,
   constants__t_session_token_i * const session_mgr__session_token);
extern void session_mgr__client_validate_session_service_resp(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_client_request_handle_i session_mgr__req_handle,
   t_bool * const session_mgr__bres,
   constants__t_session_i * const session_mgr__session);
extern void session_mgr__server_close_session(
   const constants__t_session_i session_mgr__session,
   const constants_statuscodes_bs__t_StatusCode_i session_mgr__sc_reason);
extern void session_mgr__server_evaluate_session_timeout(
   const constants__t_session_i session_mgr__session);
extern void session_mgr__server_receive_session_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_session_token_i session_mgr__session_token,
   const constants__t_msg_i session_mgr__req_msg,
   const constants__t_msg_type_i session_mgr__req_typ,
   const constants__t_msg_i session_mgr__resp_msg,
   constants__t_session_i * const session_mgr__session,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__service_ret);
extern void session_mgr__server_validate_session_service_req(
   const constants__t_channel_i session_mgr__channel,
   const constants__t_session_token_i session_mgr__session_token,
   t_bool * const session_mgr__is_valid_res,
   constants__t_session_i * const session_mgr__session,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__status_code_err);
extern void session_mgr__server_validate_session_service_resp(
   const constants__t_session_i session_mgr__session,
   t_bool * const session_mgr__is_valid_res,
   constants_statuscodes_bs__t_StatusCode_i * const session_mgr__status_code_err,
   constants__t_channel_i * const session_mgr__channel);
extern void session_mgr__session_get_endpoint_config(
   const constants__t_session_i session_mgr__p_session,
   constants__t_endpoint_config_idx_i * const session_mgr__endpoint_config_idx);

#endif
