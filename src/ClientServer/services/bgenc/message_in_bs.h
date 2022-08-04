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

 File Name            : message_in_bs.h

 Date                 : 04/08/2022 14:53:32

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _message_in_bs_h
#define _message_in_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void message_in_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void message_in_bs__bless_msg_in(
   const constants__t_msg_i message_in_bs__msg);
extern void message_in_bs__client_read_msg_header_req_handle(
   const constants__t_msg_header_i message_in_bs__msg_header,
   constants__t_client_request_handle_i * const message_in_bs__handle);
extern void message_in_bs__copy_msg_resp_header_into_msg(
   const constants__t_msg_header_i message_in_bs__msg_header,
   const constants__t_msg_i message_in_bs__msg);
extern void message_in_bs__dealloc_msg_in(
   const constants__t_msg_i message_in_bs__msg);
extern void message_in_bs__dealloc_msg_in_buffer(
   const constants__t_byte_buffer_i message_in_bs__msg_buffer);
extern void message_in_bs__dealloc_msg_in_header(
   const constants__t_msg_header_i message_in_bs__msg_header);
extern void message_in_bs__decode_msg(
   const constants__t_msg_type_i message_in_bs__msg_type,
   const constants__t_byte_buffer_i message_in_bs__msg_buffer,
   constants__t_msg_i * const message_in_bs__msg);
extern void message_in_bs__decode_msg_header(
   const t_bool message_in_bs__is_request,
   const constants__t_byte_buffer_i message_in_bs__msg_buffer,
   constants__t_msg_header_i * const message_in_bs__msg_header);
extern void message_in_bs__decode_msg_type(
   const constants__t_byte_buffer_i message_in_bs__msg_buffer,
   constants__t_msg_type_i * const message_in_bs__msg_typ);
extern void message_in_bs__decode_service_fault_msg_req_handle(
   const constants__t_byte_buffer_i message_in_bs__msg_buffer,
   constants__t_client_request_handle_i * const message_in_bs__req_handle);
extern void message_in_bs__forget_resp_msg_in(
   const constants__t_msg_header_i message_in_bs__msg_header,
   const constants__t_msg_i message_in_bs__msg);
extern void message_in_bs__get_msg_in_type(
   const constants__t_msg_i message_in_bs__req_msg,
   constants__t_msg_type_i * const message_in_bs__msgtype);
extern void message_in_bs__is_valid_app_msg_in(
   const constants__t_msg_i message_in_bs__msg,
   t_bool * const message_in_bs__bres,
   constants__t_msg_type_i * const message_in_bs__msg_typ);
extern void message_in_bs__is_valid_msg_in(
   const constants__t_msg_i message_in_bs__msg,
   t_bool * const message_in_bs__bres);
extern void message_in_bs__is_valid_msg_in_header(
   const constants__t_msg_header_i message_in_bs__msg_header,
   t_bool * const message_in_bs__bres);
extern void message_in_bs__is_valid_msg_in_type(
   const constants__t_msg_type_i message_in_bs__msg_typ,
   t_bool * const message_in_bs__bres);
extern void message_in_bs__is_valid_request_context(
   const constants__t_request_context_i message_in_bs__req_context,
   t_bool * const message_in_bs__bres);
extern void message_in_bs__read_activate_req_msg_identity_token(
   const constants__t_msg_i message_in_bs__p_msg,
   t_bool * const message_in_bs__p_valid_user_token,
   constants__t_user_token_i * const message_in_bs__p_user_token);
extern void message_in_bs__read_activate_req_msg_locales(
   const constants__t_msg_i message_in_bs__p_msg,
   constants__t_LocaleIds_i * const message_in_bs__p_localeIds);
extern void message_in_bs__read_create_session_msg_session_token(
   const constants__t_msg_i message_in_bs__msg,
   constants__t_session_token_i * const message_in_bs__session_token);
extern void message_in_bs__read_msg_req_header_session_token(
   const constants__t_msg_header_i message_in_bs__msg_header,
   constants__t_session_token_i * const message_in_bs__session_token);
extern void message_in_bs__read_msg_resp_header_service_status(
   const constants__t_msg_header_i message_in_bs__msg_header,
   constants_statuscodes_bs__t_StatusCode_i * const message_in_bs__status);
extern void message_in_bs__server_read_msg_header_req_handle(
   const constants__t_msg_header_i message_in_bs__msg_header,
   constants__t_server_request_handle_i * const message_in_bs__handle);

#endif
