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

 File Name            : service_mgr_bs.h

 Date                 : 29/03/2018 14:46:17

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_mgr_bs_h
#define _service_mgr_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_mgr_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_mgr_bs__UNINITIALISATION(void);
extern void service_mgr_bs__client_async_discovery_request_without_channel(
    const constants__t_channel_config_idx_i service_mgr_bs__channel_config_idx,
    const constants__t_msg_i service_mgr_bs__req_msg,
    const constants__t_application_context_i service_mgr_bs__app_context,
    t_bool* const service_mgr_bs__bres);
extern void service_mgr_bs__client_channel_connected_event_discovery(
    const constants__t_channel_config_idx_i service_mgr_bs__channel_config_idx,
    const constants__t_channel_i service_mgr_bs__channel);
extern void service_mgr_bs__client_discovery_req_failures_on_final_connection_failure(
    const constants__t_channel_config_idx_i service_mgr_bs__channel_config_idx);

#endif
