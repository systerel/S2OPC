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

 File Name            : io_dispatch_mgr.h

 Date                 : 29/09/2017 10:51:57

 C Translator Version : tradc Java V1.0 (14/03/2012)

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
#include "channel_mgr_bs.h"
#include "service_mgr.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void io_dispatch_mgr__INITIALISATION(void);

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void io_dispatch_mgr__get_msg_header_type(
   const constants__t_msg_type_i io_dispatch_mgr__msg_typ,
   constants__t_msg_header_type * const io_dispatch_mgr__header_type);
extern void io_dispatch_mgr__get_msg_service_class(
   const constants__t_msg_type_i io_dispatch_mgr__msg_typ,
   constants__t_msg_service_class * const io_dispatch_mgr__service_class);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void io_dispatch_mgr__client_activate_new_session(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_user_i io_dispatch_mgr__user,
   t_bool * const io_dispatch_mgr__bres);
extern void io_dispatch_mgr__client_channel_connected_event(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_channel_i io_dispatch_mgr__channel);
extern void io_dispatch_mgr__client_reactivate_session_new_user(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_user_i io_dispatch_mgr__user,
   constants__t_StatusCode_i * const io_dispatch_mgr__ret);
extern void io_dispatch_mgr__client_secure_channel_timeout(
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx);
extern void io_dispatch_mgr__client_send_close_session_request(
   const constants__t_session_i io_dispatch_mgr__session,
   constants__t_StatusCode_i * const io_dispatch_mgr__ret);
extern void io_dispatch_mgr__client_send_service_request(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_msg_i io_dispatch_mgr__req_msg,
   constants__t_StatusCode_i * const io_dispatch_mgr__ret);
extern void io_dispatch_mgr__close_all_active_connections(
   t_bool * const io_dispatch_mgr__bres);
extern void io_dispatch_mgr__internal_client_activate_orphaned_session(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx);
extern void io_dispatch_mgr__internal_client_create_session(
   const constants__t_session_i io_dispatch_mgr__session,
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx);
extern void io_dispatch_mgr__receive_msg_buffer(
   const constants__t_channel_i io_dispatch_mgr__channel,
   const constants__t_byte_buffer_i io_dispatch_mgr__buffer,
   const constants__t_request_context_i io_dispatch_mgr__request_context);
extern void io_dispatch_mgr__secure_channel_lost(
   const constants__t_channel_i io_dispatch_mgr__channel);
extern void io_dispatch_mgr__server_channel_connected_event(
   const constants__t_endpoint_config_idx_i io_dispatch_mgr__endpoint_config_idx,
   const constants__t_channel_config_idx_i io_dispatch_mgr__channel_config_idx,
   const constants__t_channel_i io_dispatch_mgr__channel);

#endif
