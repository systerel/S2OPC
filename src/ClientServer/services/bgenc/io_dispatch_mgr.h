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

 File Name            : io_dispatch_mgr.h

 Date                 : 04/08/2022 14:53:06

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _io_dispatch_mgr_h
#define _io_dispatch_mgr_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "app_cb_call_context_bs.h"
#include "channel_mgr.h"
#include "data_value_pointer_bs.h"
#include "node_id_pointer_bs.h"
#include "service_mgr.h"
#include "time_reference_bs.h"
#include "write_value_pointer_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void io_dispatch_mgr__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define io_dispatch_mgr__internal_server_inactive_session_prio_event service_mgr__internal_server_inactive_session_prio_event

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void io_dispatch_mgr__get_msg_header_type(
   const constants__t_msg_type_i io_dispatch_mgr__msg_typ,
   constants__t_msg_header_type_i * const io_dispatch_mgr__header_type);
extern void io_dispatch_mgr__get_msg_service_class(
   const constants__t_msg_type_i io_dispatch_mgr__msg_typ,
   constants__t_msg_service_class_i * const io_dispatch_mgr__service_class);
extern void io_dispatch_mgr__l_may_close_secure_channel_without_session(
   t_bool * const io_dispatch_mgr__l_is_one_sc_closing);
extern void io_dispatch_mgr__l_set_app_call_context_channel_config(
   const constants__t_channel_i io_dispatch_mgr__p_channel);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void io_dispatch_mgr__UNINITIALISATION(void);
extern void io_dispatch_mgr__client_activate_new_session(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_user_token_i io_dispatch_mgr__p_user_token,
   const constants__t_session_application_context_i io_dispatch_mgr__app_context,
   t_bool * const io_dispatch_mgr__bres);
extern void io_dispatch_mgr__client_channel_connected_event(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_channel_i io_dispatch_mgr__channel);
extern void io_dispatch_mgr__client_reactivate_session_new_user(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_user_token_i io_dispatch_mgr__p_user_token);
extern void io_dispatch_mgr__client_request_timeout(
   const constants__t_channel_i io_dispatch_mgr__channel,
   const constants__t_client_request_handle_i io_dispatch_mgr__request_handle);
extern void io_dispatch_mgr__client_secure_channel_timeout(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx);
extern void io_dispatch_mgr__client_send_close_session_request(
   const constants__t_session_i io_dispatch_mgr__session,
   constants_statuscodes_bs__t_StatusCode_i * const io_dispatch_mgr__ret);
extern void io_dispatch_mgr__client_send_discovery_request(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_msg_i io_dispatch_mgr__req_msg,
   const constants__t_application_context_i io_dispatch_mgr__app_context,
   constants_statuscodes_bs__t_StatusCode_i * const io_dispatch_mgr__ret);
extern void io_dispatch_mgr__client_send_service_request(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_msg_i io_dispatch_mgr__req_msg,
   const constants__t_application_context_i io_dispatch_mgr__app_context,
   constants_statuscodes_bs__t_StatusCode_i * const io_dispatch_mgr__ret);
extern void io_dispatch_mgr__close_all_active_connections(
   const t_bool io_dispatch_mgr__p_clientOnly,
   t_bool * const io_dispatch_mgr__bres);
extern void io_dispatch_mgr__internal_client_activate_orphaned_session(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx);
extern void io_dispatch_mgr__internal_client_create_session(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx);
extern void io_dispatch_mgr__internal_server_data_changed(
   const constants__t_WriteValuePointer_i io_dispatch_mgr__p_old_write_value_pointer,
   const constants__t_WriteValuePointer_i io_dispatch_mgr__p_new_write_value_pointer,
   t_bool * const io_dispatch_mgr__bres);
extern void io_dispatch_mgr__internal_server_evaluate_session_timeout(
   const constants__t_session_i io_dispatch_mgr__session);
extern void io_dispatch_mgr__internal_server_send_publish_response_prio_event(
   const constants__t_session_i io_dispatch_mgr__p_session,
   const constants__t_server_request_handle_i io_dispatch_mgr__p_req_handle,
   const constants__t_request_context_i io_dispatch_mgr__p_req_context,
   const constants__t_msg_i io_dispatch_mgr__p_publish_resp_msg,
   const constants_statuscodes_bs__t_StatusCode_i io_dispatch_mgr__p_statusCode,
   t_bool * const io_dispatch_mgr__bres);
extern void io_dispatch_mgr__internal_server_subscription_publish_timeout(
   const constants__t_subscription_i io_dispatch_mgr__p_subscription,
   t_bool * const io_dispatch_mgr__bres);
extern void io_dispatch_mgr__receive_msg_buffer(
   const constants__t_channel_i io_dispatch_mgr__channel,
   const constants__t_byte_buffer_i io_dispatch_mgr__buffer,
   const constants__t_request_context_i io_dispatch_mgr__request_context,
   t_bool * const io_dispatch_mgr__valid_msg);
extern void io_dispatch_mgr__secure_channel_lost(
   const constants__t_channel_i io_dispatch_mgr__channel);
extern void io_dispatch_mgr__server_channel_connected_event(
   const constants__t_endpoint_config_idx_i io_dispatch_mgr__endpoint_config_idx,
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_channel_i io_dispatch_mgr__channel,
   t_bool * const io_dispatch_mgr__bres);
extern void io_dispatch_mgr__server_treat_local_service_request(
   const constants__t_endpoint_config_idx_i io_dispatch_mgr__endpoint_config_idx,
   const constants__t_msg_i io_dispatch_mgr__req_msg,
   const constants__t_application_context_i io_dispatch_mgr__app_context,
   constants_statuscodes_bs__t_StatusCode_i * const io_dispatch_mgr__ret);
extern void io_dispatch_mgr__snd_msg_failure(
   const constants__t_channel_i io_dispatch_mgr__channel,
   const constants__t_request_context_i io_dispatch_mgr__request_id,
   const constants_statuscodes_bs__t_StatusCode_i io_dispatch_mgr__error_status);

#endif
