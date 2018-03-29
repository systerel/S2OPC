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

 File Name            : channel_mgr_bs.h

 Date                 : 29/03/2018 14:46:14

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _channel_mgr_bs_h
#define _channel_mgr_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr_1.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void channel_mgr_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void channel_mgr_bs__channel_do_nothing(const constants__t_channel_i channel_mgr_bs__channel);
extern void channel_mgr_bs__define_SecurityPolicy(const constants__t_channel_i channel_mgr_bs__p_channel);
extern void channel_mgr_bs__finalize_close_secure_channel(const constants__t_channel_i channel_mgr_bs__p_channel);
extern void channel_mgr_bs__get_SecurityPolicy(const constants__t_channel_i channel_mgr_bs__channel,
                                               constants__t_SecurityPolicy* const channel_mgr_bs__secpol);
extern void channel_mgr_bs__is_valid_channel_config_idx(
    const constants__t_channel_config_idx_i channel_mgr_bs__p_config_idx,
    t_bool* const channel_mgr_bs__bres);
extern void channel_mgr_bs__is_valid_endpoint_config_idx(
    const constants__t_endpoint_config_idx_i channel_mgr_bs__p_config_idx,
    t_bool* const channel_mgr_bs__bres);
extern void channel_mgr_bs__last_connected_channel_lost(void);
extern void channel_mgr_bs__prepare_cli_open_secure_channel(
    const constants__t_channel_config_idx_i channel_mgr_bs__p_config_idx);
extern void channel_mgr_bs__reset_SecurityPolicy(const constants__t_channel_i channel_mgr_bs__channel);
extern void channel_mgr_bs__send_channel_msg_buffer(
    const constants__t_channel_i channel_mgr_bs__channel,
    const constants__t_byte_buffer_i channel_mgr_bs__buffer,
    const constants__t_request_context_i channel_mgr_bs__request_context);

#endif
