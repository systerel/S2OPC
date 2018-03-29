/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/******************************************************************************

 File Name            : service_mgr.h

 Date                 : 29/03/2018 14:46:10

 C Translator Version : tradc Java V1.0 (14/03/2012)

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
#include "address_space.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"
#include "service_browse_seq.h"
#include "service_get_endpoints_bs.h"
#include "service_mgr_1.h"
#include "service_mgr_bs.h"
#include "service_read.h"
#include "service_response_cb_bs.h"
#include "service_write_decode_bs.h"
#include "session_mgr.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_mgr__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_mgr__bless_msg_out message_out_bs__bless_msg_out
#define service_mgr__client_async_activate_new_session_with_channel \
    session_mgr__client_async_activate_new_session_with_channel
#define service_mgr__client_async_activate_new_session_without_channel \
    session_mgr__client_async_activate_new_session_without_channel
#define service_mgr__client_async_discovery_request_without_channel \
    service_mgr_bs__client_async_discovery_request_without_channel
#define service_mgr__client_channel_connected_event_discovery service_mgr_bs__client_channel_connected_event_discovery
#define service_mgr__client_channel_connected_event_session session_mgr__client_channel_connected_event_session
#define service_mgr__client_close_session session_mgr__client_close_session
#define service_mgr__client_close_sessions_on_final_connection_failure \
    session_mgr__client_close_sessions_on_final_connection_failure
#define service_mgr__client_discovery_req_failures_on_final_connection_failure \
    service_mgr_bs__client_discovery_req_failures_on_final_connection_failure
#define service_mgr__client_req_handle_to_request_id request_handle_bs__client_req_handle_to_request_id
#define service_mgr__client_request_id_to_req_handle request_handle_bs__client_request_id_to_req_handle
#define service_mgr__client_secure_channel_lost_session_sm session_mgr__client_secure_channel_lost_session_sm
#define service_mgr__dealloc_msg_out message_out_bs__dealloc_msg_out
#define service_mgr__decode_msg_type message_in_bs__decode_msg_type
#define service_mgr__is_valid_app_msg_in message_in_bs__is_valid_app_msg_in
#define service_mgr__is_valid_app_msg_out message_out_bs__is_valid_app_msg_out
#define service_mgr__is_valid_buffer_out message_out_bs__is_valid_buffer_out
#define service_mgr__is_valid_msg_in_type message_in_bs__is_valid_msg_in_type
#define service_mgr__is_valid_request_context message_in_bs__is_valid_request_context
#define service_mgr__is_valid_session session_mgr__is_valid_session
#define service_mgr__server_evaluate_session_timeout session_mgr__server_evaluate_session_timeout
#define service_mgr__server_secure_channel_lost_session_sm session_mgr__server_secure_channel_lost_session_sm

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_mgr__get_response_type(const constants__t_msg_type_i service_mgr__req_msg_typ,
                                           constants__t_msg_type_i* const service_mgr__resp_msg_typ);
extern void service_mgr__local_client_discovery_service_request(
    const constants__t_channel_i service_mgr__channel,
    const constants__t_msg_i service_mgr__req_msg,
    const constants__t_application_context_i service_mgr__app_context,
    constants__t_StatusCode_i* const service_mgr__ret,
    constants__t_byte_buffer_i* const service_mgr__buffer_out,
    constants__t_request_handle_i* const service_mgr__req_handle);
extern void service_mgr__treat_discovery_service_req(
    const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
    const constants__t_msg_type_i service_mgr__req_typ,
    const constants__t_msg_i service_mgr__req_msg,
    const constants__t_msg_i service_mgr__resp_msg,
    constants__t_StatusCode_i* const service_mgr__StatusCode_service);
extern void service_mgr__treat_read_request(const constants__t_msg_i service_mgr__p_request_msg,
                                            const constants__t_msg_i service_mgr__p_response_msg,
                                            constants__t_StatusCode_i* const service_mgr__StatusCode_service);
extern void service_mgr__treat_session_service_req(const constants__t_msg_type_i service_mgr__req_typ,
                                                   const constants__t_msg_i service_mgr__req_msg,
                                                   const constants__t_msg_i service_mgr__resp_msg,
                                                   constants__t_StatusCode_i* const service_mgr__StatusCode_service);
extern void service_mgr__treat_write_request(const constants__t_msg_i service_mgr__write_msg,
                                             constants__t_StatusCode_i* const service_mgr__StatusCode_service);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_mgr__client_discovery_service_request(
    const constants__t_channel_i service_mgr__channel,
    const constants__t_msg_i service_mgr__req_msg,
    const constants__t_application_context_i service_mgr__app_context,
    constants__t_StatusCode_i* const service_mgr__ret,
    constants__t_byte_buffer_i* const service_mgr__buffer_out,
    constants__t_request_handle_i* const service_mgr__req_handle);
extern void service_mgr__client_receive_discovery_service_resp(
    const constants__t_channel_i service_mgr__channel,
    const constants__t_msg_type_i service_mgr__resp_typ,
    const constants__t_byte_buffer_i service_mgr__msg_buffer);
extern void service_mgr__client_receive_session_service_resp(const constants__t_channel_i service_mgr__channel,
                                                             const constants__t_msg_type_i service_mgr__resp_typ,
                                                             const constants__t_byte_buffer_i service_mgr__msg_buffer);
extern void service_mgr__client_receive_session_treatment_resp(
    const constants__t_channel_i service_mgr__channel,
    const constants__t_msg_type_i service_mgr__resp_typ,
    const constants__t_byte_buffer_i service_mgr__msg_buffer);
extern void service_mgr__client_service_activate_orphaned_session(
    const constants__t_session_i service_mgr__session,
    const constants__t_channel_i service_mgr__channel,
    constants__t_byte_buffer_i* const service_mgr__buffer_out,
    constants__t_request_handle_i* const service_mgr__req_handle);
extern void service_mgr__client_service_activate_session(const constants__t_session_i service_mgr__session,
                                                         const constants__t_user_i service_mgr__user,
                                                         constants__t_StatusCode_i* const service_mgr__ret,
                                                         constants__t_channel_i* const service_mgr__channel,
                                                         constants__t_byte_buffer_i* const service_mgr__buffer_out,
                                                         constants__t_request_handle_i* const service_mgr__req_handle);
extern void service_mgr__client_service_close_session(const constants__t_session_i service_mgr__session,
                                                      constants__t_StatusCode_i* const service_mgr__ret,
                                                      constants__t_channel_i* const service_mgr__channel,
                                                      constants__t_byte_buffer_i* const service_mgr__buffer_out,
                                                      constants__t_request_handle_i* const service_mgr__req_handle);
extern void service_mgr__client_service_create_session(const constants__t_session_i service_mgr__session,
                                                       const constants__t_channel_i service_mgr__channel,
                                                       constants__t_byte_buffer_i* const service_mgr__buffer_out,
                                                       constants__t_request_handle_i* const service_mgr__req_handle);
extern void service_mgr__client_service_request(const constants__t_session_i service_mgr__session,
                                                const constants__t_msg_i service_mgr__req_msg,
                                                const constants__t_application_context_i service_mgr__app_context,
                                                constants__t_StatusCode_i* const service_mgr__ret,
                                                constants__t_channel_i* const service_mgr__channel,
                                                constants__t_byte_buffer_i* const service_mgr__buffer_out,
                                                constants__t_request_handle_i* const service_mgr__req_handle);
extern void service_mgr__client_snd_msg_failure(const constants__t_channel_i service_mgr__channel,
                                                const constants__t_request_handle_i service_mgr__request_handle,
                                                const constants__t_StatusCode_i service_mgr__error_status);
extern void service_mgr__internal_client_request_timeout(
    const constants__t_channel_i service_mgr__channel,
    const constants__t_request_handle_i service_mgr__request_handle);
extern void service_mgr__server_receive_discovery_service_req(
    const constants__t_channel_i service_mgr__channel,
    const constants__t_msg_type_i service_mgr__req_typ,
    const constants__t_byte_buffer_i service_mgr__msg_buffer,
    constants__t_byte_buffer_i* const service_mgr__buffer_out);
extern void service_mgr__server_receive_local_service_req(
    const constants__t_endpoint_config_idx_i service_mgr__endpoint_config_idx,
    const constants__t_msg_service_class_i service_mgr__req_class,
    const constants__t_msg_type_i service_mgr__req_typ,
    const constants__t_msg_i service_mgr__req_msg,
    const constants__t_application_context_i service_mgr__app_context,
    constants__t_StatusCode_i* const service_mgr__ret);
extern void service_mgr__server_receive_session_service_req(const constants__t_channel_i service_mgr__channel,
                                                            const constants__t_msg_type_i service_mgr__req_typ,
                                                            const constants__t_byte_buffer_i service_mgr__msg_buffer,
                                                            constants__t_byte_buffer_i* const service_mgr__buffer_out);
extern void service_mgr__server_receive_session_treatment_req(
    const constants__t_channel_i service_mgr__channel,
    const constants__t_msg_type_i service_mgr__req_typ,
    const constants__t_byte_buffer_i service_mgr__msg_buffer,
    constants__t_byte_buffer_i* const service_mgr__buffer_out);

#endif
