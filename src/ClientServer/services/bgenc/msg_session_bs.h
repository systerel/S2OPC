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

 File Name            : msg_session_bs.h

 Date                 : 04/08/2022 14:53:38

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_session_bs_h
#define _msg_session_bs_h

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
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_session_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_session_bs__create_session_req_check_client_certificate(
   const constants__t_msg_i msg_session_bs__p_req_msg,
   const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
   t_bool * const msg_session_bs__valid);
extern void msg_session_bs__create_session_req_export_maxResponseMessageSize(
   const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
   const constants__t_msg_i msg_session_bs__p_req_msg);
extern void msg_session_bs__create_session_resp_check_server_certificate(
   const constants__t_msg_i msg_session_bs__p_resp_msg,
   const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
   t_bool * const msg_session_bs__valid);
extern void msg_session_bs__create_session_resp_check_server_endpoints(
   const constants__t_msg_i msg_session_bs__p_resp_msg,
   const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
   t_bool * const msg_session_bs__valid);
extern void msg_session_bs__create_session_resp_export_maxRequestMessageSize(
   const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
   const constants__t_msg_i msg_session_bs__p_resp_msg);
extern void msg_session_bs__write_activate_msg_user(
   const constants__t_msg_i msg_session_bs__msg,
   const constants__t_user_token_i msg_session_bs__p_user_token);
extern void msg_session_bs__write_activate_req_msg_locales(
   const constants__t_msg_i msg_session_bs__p_req_msg,
   const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx);
extern void msg_session_bs__write_activate_session_req_msg_crypto(
   const constants__t_msg_i msg_session_bs__activate_req_msg,
   const constants__t_SignatureData_i msg_session_bs__signature,
   t_bool * const msg_session_bs__bret);
extern void msg_session_bs__write_activate_session_resp_nonce(
   const constants__t_msg_i msg_session_bs__activate_resp_msg,
   const constants__t_Nonce_i msg_session_bs__nonce);
extern void msg_session_bs__write_create_session_msg_server_endpoints(
   const constants__t_msg_i msg_session_bs__req_msg,
   const constants__t_msg_i msg_session_bs__resp_msg,
   const constants__t_endpoint_config_idx_i msg_session_bs__endpoint_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const msg_session_bs__ret);
extern void msg_session_bs__write_create_session_msg_session_revised_timeout(
   const constants__t_msg_i msg_session_bs__req_msg,
   const constants__t_msg_i msg_session_bs__resp_msg);
extern void msg_session_bs__write_create_session_msg_session_token(
   const constants__t_msg_i msg_session_bs__msg,
   const constants__t_session_i msg_session_bs__session,
   const constants__t_session_token_i msg_session_bs__session_token);
extern void msg_session_bs__write_create_session_req_msg_clientDescription(
   const constants__t_msg_i msg_session_bs__p_req_msg,
   const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx);
extern void msg_session_bs__write_create_session_req_msg_crypto(
   const constants__t_msg_i msg_session_bs__p_req_msg,
   const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
   const constants__t_Nonce_i msg_session_bs__p_nonce,
   t_bool * const msg_session_bs__bret);
extern void msg_session_bs__write_create_session_req_msg_endpointUrl(
   const constants__t_msg_i msg_session_bs__msg,
   const constants__t_channel_config_idx_i msg_session_bs__channel_config_idx);
extern void msg_session_bs__write_create_session_req_msg_maxResponseMessageSize(
   const constants__t_msg_i msg_session_bs__p_req_msg);
extern void msg_session_bs__write_create_session_req_msg_serverUri(
   const constants__t_msg_i msg_session_bs__msg,
   const constants__t_channel_config_idx_i msg_session_bs__channel_config_idx);
extern void msg_session_bs__write_create_session_req_msg_sessionName(
   const constants__t_msg_i msg_session_bs__p_req_msg,
   const constants__t_session_application_context_i msg_session_bs__p_app_context);
extern void msg_session_bs__write_create_session_req_msg_sessionTimeout(
   const constants__t_msg_i msg_session_bs__create_req_msg);
extern void msg_session_bs__write_create_session_resp_cert(
   const constants__t_msg_i msg_session_bs__p_msg,
   const constants__t_channel_config_idx_i msg_session_bs__p_channel_config_idx,
   t_bool * const msg_session_bs__bret);
extern void msg_session_bs__write_create_session_resp_msg_maxRequestMessageSize(
   const constants__t_msg_i msg_session_bs__p_resp_msg);
extern void msg_session_bs__write_create_session_resp_nonce(
   const constants__t_msg_i msg_session_bs__p_msg,
   const constants__t_Nonce_i msg_session_bs__p_nonce);
extern void msg_session_bs__write_create_session_resp_signature(
   const constants__t_msg_i msg_session_bs__p_msg,
   const constants__t_SignatureData_i msg_session_bs__p_signature,
   t_bool * const msg_session_bs__bret);

#endif
