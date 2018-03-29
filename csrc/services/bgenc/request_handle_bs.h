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

 File Name            : request_handle_bs.h

 Date                 : 29/03/2018 14:46:16

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _request_handle_bs_h
#define _request_handle_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void request_handle_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void request_handle_bs__client_fresh_req_handle(
    const constants__t_msg_type_i request_handle_bs__req_typ,
    const constants__t_msg_type_i request_handle_bs__resp_typ,
    const t_bool request_handle_bs__is_applicative,
    const constants__t_application_context_i request_handle_bs__app_context,
    constants__t_request_handle_i* const request_handle_bs__request_handle);
extern void request_handle_bs__client_remove_req_handle(
    const constants__t_request_handle_i request_handle_bs__req_handle);
extern void request_handle_bs__client_req_handle_to_request_id(
    const constants__t_request_handle_i request_handle_bs__req_handle,
    constants__t_request_context_i* const request_handle_bs__request_id);
extern void request_handle_bs__client_request_id_to_req_handle(
    const constants__t_request_context_i request_handle_bs__request_id,
    constants__t_request_handle_i* const request_handle_bs__request_handle);
extern void request_handle_bs__client_validate_response_request_handle(
    const constants__t_channel_i request_handle_bs__channel,
    const constants__t_request_handle_i request_handle_bs__req_handle,
    const constants__t_msg_type_i request_handle_bs__resp_typ,
    t_bool* const request_handle_bs__ret);
extern void request_handle_bs__get_req_handle_app_context(
    const constants__t_request_handle_i request_handle_bs__req_handle,
    t_bool* const request_handle_bs__is_applicative,
    constants__t_application_context_i* const request_handle_bs__app_context);
extern void request_handle_bs__get_req_handle_channel(const constants__t_request_handle_i request_handle_bs__req_handle,
                                                      constants__t_channel_i* const request_handle_bs__channel);
extern void request_handle_bs__get_req_handle_req_typ(const constants__t_request_handle_i request_handle_bs__req_handle,
                                                      constants__t_msg_type_i* const request_handle_bs__req_typ);
extern void request_handle_bs__get_req_handle_resp_typ(
    const constants__t_request_handle_i request_handle_bs__req_handle,
    constants__t_msg_type_i* const request_handle_bs__resp_typ);
extern void request_handle_bs__is_valid_req_handle(const constants__t_request_handle_i request_handle_bs__req_handle,
                                                   t_bool* const request_handle_bs__ret);
extern void request_handle_bs__req_handle_do_nothing(const constants__t_request_handle_i request_handle_bs__req_handle);
extern void request_handle_bs__set_req_handle_channel(const constants__t_request_handle_i request_handle_bs__req_handle,
                                                      const constants__t_channel_i request_handle_bs__channel);

#endif
