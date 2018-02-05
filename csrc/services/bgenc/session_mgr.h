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

 File Name            : session_mgr.h

 Date                 : 05/02/2018 16:15:22

 C Translator Version : tradc Java V1.0 (14/03/2012)

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
#include "channel_mgr.h"
#include "constants.h"
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
#define session_mgr__get_session_user_or_indet session_core__get_session_user_or_indet
#define session_mgr__server_close_session_sm session_core__server_close_session_sm
#define session_mgr__server_secure_channel_lost_session_sm session_core__server_secure_channel_lost_session_sm

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void session_mgr__local_sc_activate_sessions_on_SC_connection(
    const constants__t_channel_config_idx_i session_mgr__channel_config_idx);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_mgr__client_async_activate_new_session_with_channel(
    const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
    const constants__t_channel_i session_mgr__channel,
    const constants__t_user_i session_mgr__user,
    t_bool* const session_mgr__bres);
extern void session_mgr__client_async_activate_new_session_without_channel(
    const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
    const constants__t_user_i session_mgr__user,
    t_bool* const session_mgr__bres);
extern void session_mgr__client_channel_connected_event_session(
    const constants__t_channel_config_idx_i session_mgr__channel_config_idx,
    const constants__t_channel_i session_mgr__channel);
extern void session_mgr__client_close_session(const constants__t_session_i session_mgr__session);
extern void session_mgr__client_close_session_req(const constants__t_session_i session_mgr__session,
                                                  const constants__t_request_handle_i session_mgr__req_handle,
                                                  const constants__t_msg_i session_mgr__close_req_msg,
                                                  constants__t_StatusCode_i* const session_mgr__ret,
                                                  constants__t_channel_i* const session_mgr__channel,
                                                  constants__t_session_token_i* const session_mgr__session_token);
extern void session_mgr__client_create_session_req(const constants__t_session_i session_mgr__session,
                                                   const constants__t_channel_i session_mgr__channel,
                                                   const constants__t_request_handle_i session_mgr__req_handle,
                                                   const constants__t_msg_i session_mgr__create_req_msg,
                                                   t_bool* const session_mgr__bret);
extern void session_mgr__client_receive_session_resp(const constants__t_channel_i session_mgr__channel,
                                                     const constants__t_request_handle_i session_mgr__req_handle,
                                                     const constants__t_msg_type_i session_mgr__resp_typ,
                                                     const constants__t_msg_header_i session_mgr__resp_header,
                                                     const constants__t_msg_i session_mgr__resp_msg,
                                                     constants__t_session_i* const session_mgr__session);
extern void session_mgr__client_sc_activate_session_req(const constants__t_session_i session_mgr__session,
                                                        const constants__t_request_handle_i session_mgr__req_handle,
                                                        const constants__t_channel_i session_mgr__channel,
                                                        const constants__t_msg_i session_mgr__activate_req_msg,
                                                        constants__t_StatusCode_i* const session_mgr__ret,
                                                        constants__t_session_token_i* const session_mgr__session_token);
extern void session_mgr__client_user_activate_session_req(
    const constants__t_session_i session_mgr__session,
    const constants__t_request_handle_i session_mgr__req_handle,
    const constants__t_user_i session_mgr__user,
    const constants__t_msg_i session_mgr__activate_req_msg,
    constants__t_StatusCode_i* const session_mgr__ret,
    constants__t_channel_i* const session_mgr__channel,
    constants__t_session_token_i* const session_mgr__session_token);
extern void session_mgr__client_validate_session_service_req(
    const constants__t_session_i session_mgr__session,
    const constants__t_request_handle_i session_mgr__req_handle,
    constants__t_StatusCode_i* const session_mgr__ret,
    constants__t_channel_i* const session_mgr__channel,
    constants__t_session_token_i* const session_mgr__session_token);
extern void session_mgr__client_validate_session_service_resp(
    const constants__t_channel_i session_mgr__channel,
    const constants__t_request_handle_i session_mgr__req_handle,
    t_bool* const session_mgr__bres,
    constants__t_session_i* const session_mgr__session);
extern void session_mgr__server_receive_session_req(const constants__t_channel_i session_mgr__channel,
                                                    const constants__t_session_token_i session_mgr__session_token,
                                                    const constants__t_msg_i session_mgr__req_msg,
                                                    const constants__t_msg_type_i session_mgr__req_typ,
                                                    const constants__t_msg_i session_mgr__resp_msg,
                                                    constants__t_session_i* const session_mgr__session,
                                                    constants__t_StatusCode_i* const session_mgr__service_ret);
extern void session_mgr__server_validate_session_service_req(
    const constants__t_channel_i session_mgr__channel,
    const constants__t_request_handle_i session_mgr__req_handle,
    const constants__t_session_token_i session_mgr__session_token,
    t_bool* const session_mgr__is_valid_res,
    constants__t_session_i* const session_mgr__session,
    t_bool* const session_mgr__snd_err);
extern void session_mgr__server_validate_session_service_resp(
    const constants__t_channel_i session_mgr__channel,
    const constants__t_session_i session_mgr__session,
    const constants__t_request_handle_i session_mgr__req_handle,
    t_bool* const session_mgr__is_valid_res,
    t_bool* const session_mgr__snd_err);

#endif
