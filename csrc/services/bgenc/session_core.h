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

 File Name            : session_core.h

 Date                 : 05/02/2018 16:15:22

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_core_h
#define _session_core_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "session_core_1.h"
#include "session_core_it.h"

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
extern void session_core__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define session_core__client_gen_activate_orphaned_session_internal_event \
    session_core_1__client_gen_activate_orphaned_session_internal_event
#define session_core__client_gen_activate_user_session_internal_event \
    session_core_1__client_gen_activate_user_session_internal_event
#define session_core__client_gen_create_session_internal_event session_core_1__client_gen_create_session_internal_event
#define session_core__client_get_token_from_session session_core_1__client_get_token_from_session
#define session_core__delete_session session_core_1__delete_session
#define session_core__get_session_channel session_core_1__get_session_channel
#define session_core__getall_orphaned session_core_1__getall_orphaned
#define session_core__getall_session_channel session_core_1__getall_session_channel
#define session_core__getall_to_activate session_core_1__getall_to_activate
#define session_core__getall_to_create session_core_1__getall_to_create
#define session_core__is_valid_session session_core_1__is_valid_session
#define session_core__is_valid_user session_core_1__is_valid_user
#define session_core__reset_session_to_activate session_core_1__reset_session_to_activate
#define session_core__reset_session_to_create session_core_1__reset_session_to_create
#define session_core__server_get_session_from_token session_core_1__server_get_session_from_token
#define session_core__set_session_to_activate session_core_1__set_session_to_activate
#define session_core__set_session_to_create session_core_1__set_session_to_create

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void session_core__server_internal_activate_req_and_resp(
    const constants__t_channel_i session_core__channel,
    const constants__t_session_i session_core__session,
    const constants__t_user_i session_core__user,
    const constants__t_msg_i session_core__activate_req_msg,
    const constants__t_msg_i session_core__activate_resp_msg,
    t_bool* const session_core__res_activated);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core__client_activate_session_resp_sm(const constants__t_channel_i session_core__channel,
                                                          const constants__t_session_i session_core__session,
                                                          const constants__t_msg_i session_core__activate_resp_msg,
                                                          t_bool* const session_core__bret);
extern void session_core__client_close_session_req_sm(const constants__t_session_i session_core__session,
                                                      const constants__t_msg_i session_core__close_req_msg,
                                                      constants__t_StatusCode_i* const session_core__ret,
                                                      constants__t_channel_i* const session_core__channel,
                                                      constants__t_session_token_i* const session_core__session_token);
extern void session_core__client_close_session_resp_sm(const constants__t_channel_i session_core__channel,
                                                       const constants__t_session_i session_core__session,
                                                       const constants__t_msg_i session_core__close_resp_msg);
extern void session_core__client_close_session_sm(const constants__t_session_i session_core__session);
extern void session_core__client_create_session_req_sm(const constants__t_session_i session_core__session,
                                                       const constants__t_channel_i session_core__channel,
                                                       const constants__t_msg_i session_core__create_req_msg,
                                                       t_bool* const session_core__valid);
extern void session_core__client_create_session_resp_sm(const constants__t_channel_i session_core__channel,
                                                        const constants__t_session_i session_core__session,
                                                        const constants__t_session_token_i session_core__session_token,
                                                        const constants__t_msg_i session_core__create_resp_msg,
                                                        t_bool* const session_core__bret);
extern void session_core__client_init_session_sm(constants__t_session_i* const session_core__nsession);
extern void session_core__client_sc_activate_session_req_sm(
    const constants__t_session_i session_core__session,
    const constants__t_channel_i session_core__channel,
    const constants__t_msg_i session_core__activate_req_msg,
    constants__t_session_token_i* const session_core__session_token);
extern void session_core__client_secure_channel_lost_session_sm(
    const constants__t_channel_i session_core__lost_channel,
    const constants__t_channel_config_idx_i session_core__channel_config_idx);
extern void session_core__client_user_activate_session_req_sm(
    const constants__t_session_i session_core__session,
    const constants__t_user_i session_core__user,
    const constants__t_msg_i session_core__activate_req_msg,
    constants__t_StatusCode_i* const session_core__ret,
    constants__t_channel_i* const session_core__channel,
    constants__t_session_token_i* const session_core__session_token);
extern void session_core__get_session_state_or_closed(const constants__t_session_i session_core__session,
                                                      constants__t_sessionState* const session_core__state);
extern void session_core__get_session_user_or_indet(const constants__t_session_i session_core__session,
                                                    constants__t_user_i* const session_core__user);
extern void session_core__is_session_valid_for_service(const constants__t_channel_i session_core__channel,
                                                       const constants__t_session_i session_core__session,
                                                       t_bool* const session_core__ret);
extern void session_core__server_activate_session_req_and_resp_sm(
    const constants__t_channel_i session_core__channel,
    const constants__t_session_i session_core__session,
    const constants__t_user_i session_core__user,
    const constants__t_msg_i session_core__activate_req_msg,
    const constants__t_msg_i session_core__activate_resp_msg,
    constants__t_StatusCode_i* const session_core__ret);
extern void session_core__server_close_session_req_and_resp_sm(const constants__t_channel_i session_core__channel,
                                                               const constants__t_session_i session_core__session,
                                                               const constants__t_msg_i session_core__close_req_msg,
                                                               const constants__t_msg_i session_core__close_resp_msg,
                                                               constants__t_StatusCode_i* const session_core__ret);
extern void session_core__server_close_session_sm(const constants__t_session_i session_core__session);
extern void session_core__server_create_session_req_and_resp_sm(
    const constants__t_channel_i session_core__channel,
    const constants__t_msg_i session_core__create_req_msg,
    const constants__t_msg_i session_core__create_resp_msg,
    constants__t_session_i* const session_core__nsession,
    constants__t_StatusCode_i* const session_core__service_ret);
extern void session_core__server_secure_channel_lost_session_sm(const constants__t_channel_i session_core__channel);

#endif
