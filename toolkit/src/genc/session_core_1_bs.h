/*
 *  Copyright (C) 2017 Systerel and others.
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

 File Name            : session_core_1_bs.h

 Date                 : 29/09/2017 10:52:04

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_core_1_bs_h
#define _session_core_1_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr_bs.h"
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core_1_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_1_bs__client_activate_session_req_do_crypto(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_config_idx_i session_core_1_bs__channel_config_idx,
   const constants__t_Nonce_i session_core_1_bs__server_nonce,
   t_bool * const session_core_1_bs__valid,
   constants__t_SignatureData_i * const session_core_1_bs__signature);
extern void session_core_1_bs__client_create_session_check_crypto(
   const constants__t_session_i session_core_1_bs__p_session,
   const constants__t_channel_config_idx_i session_core_1_bs__p_channel_config_idx,
   const constants__t_msg_i session_core_1_bs__p_resp_msg,
   t_bool * const session_core_1_bs__valid);
extern void session_core_1_bs__client_create_session_req_do_crypto(
   const constants__t_session_i session_core_1_bs__p_session,
   const constants__t_channel_i session_core_1_bs__p_channel,
   const constants__t_channel_config_idx_i session_core_1_bs__p_channel_config_idx,
   t_bool * const session_core_1_bs__valid,
   t_bool * const session_core_1_bs__nonce_needed);
extern void session_core_1_bs__client_get_token_from_session(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_session_token_i * const session_core_1_bs__session_token);
extern void session_core_1_bs__client_set_session_token(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_session_token_i session_core_1_bs__token);
extern void session_core_1_bs__create_session(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__channel,
   const constants__t_sessionState session_core_1_bs__state);
extern void session_core_1_bs__create_session_failure(
   const constants__t_session_i session_core_1_bs__session);
extern void session_core_1_bs__delete_session(
   const constants__t_session_i session_core_1_bs__session);
extern void session_core_1_bs__drop_NonceClient(
   const constants__t_session_i session_core_1_bs__p_session);
extern void session_core_1_bs__get_NonceClient(
   const constants__t_session_i session_core_1_bs__p_session,
   constants__t_Nonce_i * const session_core_1_bs__nonce);
extern void session_core_1_bs__get_NonceServer(
   const constants__t_session_i session_core_1_bs__p_session,
   constants__t_Nonce_i * const session_core_1_bs__nonce);
extern void session_core_1_bs__get_session_channel(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_channel_i * const session_core_1_bs__channel);
extern void session_core_1_bs__get_session_state(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_sessionState * const session_core_1_bs__state);
extern void session_core_1_bs__get_session_user(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_user_i * const session_core_1_bs__user);
extern void session_core_1_bs__init_new_session(
   const t_bool session_core_1_bs__is_client,
   constants__t_session_i * const session_core_1_bs__session);
extern void session_core_1_bs__is_valid_session(
   const constants__t_session_i session_core_1_bs__session,
   t_bool * const session_core_1_bs__ret);
extern void session_core_1_bs__is_valid_user(
   const constants__t_user_i session_core_1_bs__user,
   t_bool * const session_core_1_bs__ret);
extern void session_core_1_bs__server_activate_session_check_crypto(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__channel,
   const constants__t_channel_config_idx_i session_core_1_bs__channel_config_idx,
   const constants__t_msg_i session_core_1_bs__activate_req_msg,
   t_bool * const session_core_1_bs__valid);
extern void session_core_1_bs__server_create_session_req_do_crypto(
   const constants__t_session_i session_core_1_bs__p_session,
   const constants__t_msg_i session_core_1_bs__p_req_msg,
   const constants__t_endpoint_config_idx_i session_core_1_bs__p_endpoint_config_idx,
   const constants__t_channel_config_idx_i session_core_1_bs__p_channel_config_idx,
   t_bool * const session_core_1_bs__valid,
   constants__t_SignatureData_i * const session_core_1_bs__signature);
extern void session_core_1_bs__server_get_fresh_session_token(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_session_token_i * const session_core_1_bs__token);
extern void session_core_1_bs__server_get_session_from_token(
   const constants__t_session_token_i session_core_1_bs__session_token,
   constants__t_session_i * const session_core_1_bs__session);
extern void session_core_1_bs__server_is_valid_session_token(
   const constants__t_session_token_i session_core_1_bs__token,
   t_bool * const session_core_1_bs__ret);
extern void session_core_1_bs__set_session_channel(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__channel);
extern void session_core_1_bs__set_session_orphaned(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__lost_channel,
   const constants__t_channel_config_idx_i session_core_1_bs__channel_config_idx);
extern void session_core_1_bs__set_session_state(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_sessionState session_core_1_bs__state);
extern void session_core_1_bs__set_session_state_closed(
   const constants__t_session_i session_core_1_bs__session,
   const t_bool session_core_1_bs__is_client);
extern void session_core_1_bs__set_session_user(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_user_i session_core_1_bs__user);

#endif
