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

 File Name            : message_out_bs.h

 Date                 : 04/08/2022 14:53:32

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _message_out_bs_h
#define _message_out_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void message_out_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void message_out_bs__alloc_msg_header(
   const t_bool message_out_bs__p_is_request,
   constants__t_msg_header_i * const message_out_bs__nmsg_header);
extern void message_out_bs__alloc_req_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
   constants__t_msg_header_i * const message_out_bs__nmsg_header,
   constants__t_msg_i * const message_out_bs__nmsg);
extern void message_out_bs__alloc_resp_msg(
   const constants__t_msg_type_i message_out_bs__msg_type,
   constants__t_msg_header_i * const message_out_bs__nmsg_header,
   constants__t_msg_i * const message_out_bs__nmsg);
extern void message_out_bs__bless_msg_out(
   const constants__t_msg_i message_out_bs__msg);
extern void message_out_bs__client_write_msg_out_header_req_handle(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_client_request_handle_i message_out_bs__req_handle);
extern void message_out_bs__copy_msg_resp_header_into_msg_out(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_msg_i message_out_bs__msg);
extern void message_out_bs__dealloc_msg_header_out(
   const constants__t_msg_header_i message_out_bs__msg_header);
extern void message_out_bs__dealloc_msg_out(
   const constants__t_msg_i message_out_bs__msg);
extern void message_out_bs__encode_msg(
   const constants__t_channel_config_idx_i message_out_bs__channel_cfg,
   const constants__t_msg_header_type_i message_out_bs__header_type,
   const constants__t_msg_type_i message_out_bs__msg_type,
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_msg_i message_out_bs__msg,
   constants_statuscodes_bs__t_StatusCode_i * const message_out_bs__sc,
   constants__t_byte_buffer_i * const message_out_bs__buffer);
extern void message_out_bs__forget_resp_msg_out(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_msg_i message_out_bs__msg);
extern void message_out_bs__get_msg_out_type(
   const constants__t_msg_i message_out_bs__msg,
   constants__t_msg_type_i * const message_out_bs__msgtype);
extern void message_out_bs__is_valid_app_msg_out(
   const constants__t_msg_i message_out_bs__msg,
   t_bool * const message_out_bs__bres,
   constants__t_msg_type_i * const message_out_bs__msg_typ);
extern void message_out_bs__is_valid_buffer_out(
   const constants__t_byte_buffer_i message_out_bs__buffer,
   t_bool * const message_out_bs__bres);
extern void message_out_bs__is_valid_msg_out(
   const constants__t_msg_i message_out_bs__msg,
   t_bool * const message_out_bs__bres);
extern void message_out_bs__is_valid_msg_out_header(
   const constants__t_msg_header_i message_out_bs__msg_header,
   t_bool * const message_out_bs__bres);
extern void message_out_bs__server_write_msg_out_header_req_handle(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_server_request_handle_i message_out_bs__req_handle);
extern void message_out_bs__write_msg_out_header_session_token(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants__t_session_token_i message_out_bs__session_token);
extern void message_out_bs__write_msg_resp_header_service_status(
   const constants__t_msg_header_i message_out_bs__msg_header,
   const constants_statuscodes_bs__t_StatusCode_i message_out_bs__status_code);

#endif
