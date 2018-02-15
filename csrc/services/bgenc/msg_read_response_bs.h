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

 File Name            : msg_read_response_bs.h

 Date                 : 15/02/2018 15:01:19

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _msg_read_response_bs_h
#define _msg_read_response_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_read_response_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_response_bs__alloc_read_response(
    const t_entier4 msg_read_response_bs__p_nb_resps,
    const constants__t_TimestampsToReturn_i msg_read_response_bs__p_TimestampsToReturn,
    const constants__t_msg_i msg_read_response_bs__p_resp_msg,
    t_bool* const msg_read_response_bs__p_isvalid);
extern void msg_read_response_bs__set_read_response(const constants__t_msg_i msg_read_response_bs__resp_msg,
                                                    const constants__t_ReadValue_i msg_read_response_bs__rvi,
                                                    const constants__t_Variant_i msg_read_response_bs__val,
                                                    const constants__t_StatusCode_i msg_read_response_bs__sc,
                                                    const constants__t_AttributeId_i msg_read_response_bs__aid);

#endif
