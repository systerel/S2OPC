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

 File Name            : session_core_2.h

 Date                 : 05/02/2018 16:15:21

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_core_2_h
#define _session_core_2_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern constants__t_channel_i session_core_2__a_channel_i[constants__t_session_i_max + 1];
extern constants__t_channel_config_idx_i session_core_2__a_client_orphaned_i[constants__t_session_i_max + 1];
extern constants__t_channel_config_idx_i session_core_2__a_client_to_create_i[constants__t_session_i_max + 1];
extern constants__t_sessionState session_core_2__a_state_i[constants__t_session_i_max + 1];
extern t_bool session_core_2__s_session_i[constants__t_session_i_max + 1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_core_2__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_core_2__add_session(const constants__t_session_i session_core_2__p_session);
extern void session_core_2__get_session_channel(const constants__t_session_i session_core_2__session,
                                                constants__t_channel_i* const session_core_2__channel);
extern void session_core_2__get_session_state(const constants__t_session_i session_core_2__session,
                                              constants__t_sessionState* const session_core_2__state);
extern void session_core_2__getall_orphaned(
    const constants__t_session_i session_core_2__p_session,
    t_bool* const session_core_2__p_dom,
    constants__t_channel_config_idx_i* const session_core_2__p_channel_config_idx);
extern void session_core_2__getall_session_channel(const constants__t_session_i session_core_2__p_session,
                                                   t_bool* const session_core_2__p_dom,
                                                   constants__t_channel_i* const session_core_2__p_channel);
extern void session_core_2__getall_to_create(
    const constants__t_session_i session_core_2__p_session,
    t_bool* const session_core_2__p_dom,
    constants__t_channel_config_idx_i* const session_core_2__p_channel_config_idx);
extern void session_core_2__is_valid_session(const constants__t_session_i session_core_2__session,
                                             t_bool* const session_core_2__ret);
extern void session_core_2__remove_session(const constants__t_session_i session_core_2__p_session);
extern void session_core_2__reset_session_channel(const constants__t_session_i session_core_2__p_session);
extern void session_core_2__reset_session_orphaned(const constants__t_session_i session_core_2__p_session);
extern void session_core_2__reset_session_to_create(const constants__t_session_i session_core_2__p_session);
extern void session_core_2__set_session_channel(const constants__t_session_i session_core_2__session,
                                                const constants__t_channel_i session_core_2__channel);
extern void session_core_2__set_session_orphaned_1(
    const constants__t_session_i session_core_2__p_session,
    const constants__t_channel_config_idx_i session_core_2__p_channel_config_idx);
extern void session_core_2__set_session_state_1(const constants__t_session_i session_core_2__p_session,
                                                const constants__t_sessionState session_core_2__p_state);
extern void session_core_2__set_session_to_create(
    const constants__t_session_i session_core_2__p_session,
    const constants__t_channel_config_idx_i session_core_2__p_channel_config_idx);

#endif
