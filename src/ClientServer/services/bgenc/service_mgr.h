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

 File Name            : service_mgr.h

 Date                 : 04/08/2022 14:53:10

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _service_mgr_h
#define _service_mgr_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "address_space_itf.h"
#include "call_method_mgr.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"
#include "service_get_endpoints_bs.h"
#include "service_mgr_bs.h"
#include "service_read.h"
#include "service_register_nodes.h"
#include "service_response_cb_bs.h"
#include "service_set_discovery_server.h"
#include "service_set_view.h"
#include "service_unregister_nodes.h"
#include "session_mgr.h"
#include "subscription_mgr.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_mgr__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_mgr__bless_msg_out message_out_bs__bless_msg_out
#define service_mgr__client_async_activate_new_session_with_channel session_mgr__client_async_activate_new_session_with_channel
#define service_mgr__client_async_activate_new_session_without_channel session_mgr__client_async_activate_new_session_without_channel
#define service_mgr__client_async_discovery_request_without_channel service_mgr_bs__client_async_discovery_request_without_channel
#define service_mgr__client_channel_connected_event_discovery service_mgr_bs__client_channel_connected_event_discovery
#define service_mgr__client_channel_connected_event_session session_mgr__client_channel_connected_event_session
#define service_mgr__client_close_session session_mgr__client_close_session
#define service_mgr__client_close_sessions_on_final_connection_failure session_mgr__client_close_sessions_on_final_connection_failure
#define service_mgr__client_discovery_req_failures_on_final_connection_failure service_mgr_bs__client_discovery_req_failures_on_final_connection_failure
#define service_mgr__client_req_handle_to_request_id request_handle_bs__client_req_handle_to_request_id
#define service_mgr__client_request_id_to_req_handle request_handle_bs__client_request_id_to_req_handle
#define service_mgr__client_secure_channel_lost_session_sm session_mgr__client_secure_channel_lost_session_sm
#define service_mgr__dealloc_msg_out message_out_bs__dealloc_msg_out
#define service_mgr__decode_msg_type message_in_bs__decode_msg_type
#define service_mgr__find_channel_to_close session_mgr__find_channel_to_close
#define service_mgr__get_local_user session_mgr__get_local_user
#define service_mgr__get_session_user_server session_mgr__get_session_user_server
#define service_mgr__is_valid_app_msg_in message_in_bs__is_valid_app_msg_in
#define service_mgr__is_valid_app_msg_out message_out_bs__is_valid_app_msg_out
#define service_mgr__is_valid_buffer_out message_out_bs__is_valid_buffer_out
#define service_mgr__is_valid_msg_in_type message_in_bs__is_valid_msg_in_type
#define service_mgr__is_valid_request_context message_in_bs__is_valid_request_context
#define service_mgr__is_valid_session session_mgr__is_valid_session
#define service_mgr__is_valid_subscription subscription_mgr__is_valid_subscription
#define service_mgr__server_evaluate_session_timeout session_mgr__server_evaluate_session_timeout
#define service_mgr__server_secure_channel_lost_session_sm session_mgr__server_secure_channel_lost_session_sm
#define service_mgr__server_subscription_data_changed subscription_mgr__server_subscription_data_changed
#define service_mgr__server_subscription_publish_timeout subscription_mgr__server_subscription_publish_timeout

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_mgr__dealloc_msg_in_header_if_cond(
   const t_bool service_mgr__p_cond,
   const constants__t_msg_header_i service_mgr__p_req_msg_header);
extern void service_mgr__get_response_type(
   const constants__t_msg_type_i service_mgr__req_msg_typ,
   constants__t_msg_type_i * const service_mgr__resp_msg_typ);
extern void service_mgr__local_client_discovery_service_request(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_application_context_i service_mgr__app_context,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle);
extern void service_mgr__treat_discovery_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_msg_i service_mgr__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__StatusCode_service);
extern void service_mgr__treat_session_local_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_msg_i service_mgr__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__StatusCode_service);
extern void service_mgr__treat_session_nano_extended_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_session_i service_mgr__session,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_server_request_handle_i service_mgr__req_handle,
   const constants__t_request_context_i service_mgr__req_ctx,
   const constants__t_msg_header_i service_mgr__req_header,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_msg_i service_mgr__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__StatusCode_service,
   t_bool * const service_mgr__async_resp_msg);
extern void service_mgr__treat_session_nano_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_session_i service_mgr__session,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_msg_i service_mgr__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__StatusCode_service);
extern void service_mgr__treat_session_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_session_i service_mgr__session,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_server_request_handle_i service_mgr__req_handle,
   const constants__t_request_context_i service_mgr__req_ctx,
   const constants__t_msg_header_i service_mgr__req_header,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_msg_i service_mgr__resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__StatusCode_service,
   t_bool * const service_mgr__async_resp_msg);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_mgr__client_discovery_service_request(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_application_context_i service_mgr__app_context,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle);
extern void service_mgr__client_receive_discovery_service_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer);
extern void service_mgr__client_receive_session_service_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer);
extern void service_mgr__client_receive_session_treatment_resp(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer);
extern void service_mgr__client_service_activate_orphaned_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_channel_i service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle);
extern void service_mgr__client_service_activate_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_user_token_i service_mgr__p_user_token,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret,
   constants__t_channel_i * const service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle);
extern void service_mgr__client_service_close_session(
   const constants__t_session_i service_mgr__session,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret,
   constants__t_channel_i * const service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle);
extern void service_mgr__client_service_create_session(
   const constants__t_session_i service_mgr__session,
   const constants__t_channel_i service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle);
extern void service_mgr__client_service_fault_to_resp_type(
   const constants__t_byte_buffer_i service_mgr__msg_buffer,
   t_bool * const service_mgr__valid,
   constants__t_msg_type_i * const service_mgr__resp_typ);
extern void service_mgr__client_service_request(
   const constants__t_session_i service_mgr__session,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_application_context_i service_mgr__app_context,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret,
   constants__t_channel_i * const service_mgr__channel,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_client_request_handle_i * const service_mgr__req_handle);
extern void service_mgr__client_snd_msg_failure(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_client_request_handle_i service_mgr__request_handle,
   const constants_statuscodes_bs__t_StatusCode_i service_mgr__error_status);
extern void service_mgr__internal_server_inactive_session_prio_event(
   const constants__t_session_i service_mgr__p_session,
   const constants__t_sessionState service_mgr__p_newSessionState,
   t_bool * const service_mgr__bres);
extern void service_mgr__server_receive_discovery_service_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer,
   t_bool * const service_mgr__valid_req,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__sc,
   constants__t_byte_buffer_i * const service_mgr__buffer_out);
extern void service_mgr__server_receive_local_service_req(
   const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
   const constants__t_msg_service_class_i service_mgr__req_class,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_msg_i service_mgr__req_msg,
   const constants__t_application_context_i service_mgr__app_context,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__ret);
extern void service_mgr__server_receive_session_service_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_request_context_i service_mgr__req_context,
   const constants__t_byte_buffer_i service_mgr__msg_buffer,
   t_bool * const service_mgr__valid_req,
   t_bool * const service_mgr__async_resp,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__sc,
   constants__t_byte_buffer_i * const service_mgr__buffer_out);
extern void service_mgr__server_receive_session_treatment_req(
   const constants__t_channel_i service_mgr__channel,
   const constants__t_msg_type_i service_mgr__req_typ,
   const constants__t_byte_buffer_i service_mgr__msg_buffer,
   t_bool * const service_mgr__valid_req,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__sc,
   constants__t_byte_buffer_i * const service_mgr__buffer_out);
extern void service_mgr__server_send_publish_response(
   const constants__t_session_i service_mgr__session,
   const constants__t_server_request_handle_i service_mgr__req_handle,
   const constants_statuscodes_bs__t_StatusCode_i service_mgr__statusCode,
   const constants__t_msg_type_i service_mgr__resp_typ,
   const constants__t_msg_i service_mgr__publish_resp_msg,
   t_bool * const service_mgr__bres,
   constants_statuscodes_bs__t_StatusCode_i * const service_mgr__sc,
   constants__t_byte_buffer_i * const service_mgr__buffer_out,
   constants__t_channel_i * const service_mgr__channel);
extern void service_mgr__service_mgr_UNINITIALISATION(void);

#endif
