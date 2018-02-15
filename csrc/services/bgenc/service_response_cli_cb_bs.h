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

 File Name            : service_response_cli_cb_bs.h

 Date                 : 15/02/2018 15:01:20

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_response_cli_cb_bs_h
#define _service_response_cli_cb_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_response_cli_cb_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_response_cli_cb_bs__cli_service_response(
    const constants__t_session_i service_response_cli_cb_bs__session,
    const constants__t_msg_i service_response_cli_cb_bs__resp_msg,
    const constants__t_application_context_i service_response_cli_cb_bs__app_context);
extern void service_response_cli_cb_bs__cli_snd_failure(
    const constants__t_application_context_i service_response_cli_cb_bs__app_context,
    const constants__t_StatusCode_i service_response_cli_cb_bs__error_status);

#endif
