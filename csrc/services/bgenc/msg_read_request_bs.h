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

 File Name            : msg_read_request_bs.h

 Date                 : 15/02/2018 15:01:18

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _msg_read_request_bs_h
#define _msg_read_request_bs_h

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
extern void msg_read_request_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_request_bs__getall_req_ReadValue_AttributeId(
    const constants__t_msg_i msg_read_request_bs__msg,
    const constants__t_ReadValue_i msg_read_request_bs__rvi,
    t_bool* const msg_read_request_bs__isvalid,
    constants__t_AttributeId_i* const msg_read_request_bs__aid);
extern void msg_read_request_bs__getall_req_ReadValue_NodeId(const constants__t_msg_i msg_read_request_bs__msg,
                                                             const constants__t_ReadValue_i msg_read_request_bs__rvi,
                                                             t_bool* const msg_read_request_bs__isvalid,
                                                             constants__t_NodeId_i* const msg_read_request_bs__nid);
extern void msg_read_request_bs__read_req_MaxAge(const constants__t_msg_i msg_read_request_bs__p_msg,
                                                 t_bool* const msg_read_request_bs__p_maxAge_valid);
extern void msg_read_request_bs__read_req_TimestampsToReturn(
    const constants__t_msg_i msg_read_request_bs__p_msg,
    constants__t_TimestampsToReturn_i* const msg_read_request_bs__p_tsToReturn);
extern void msg_read_request_bs__read_req_nb_ReadValue(const constants__t_msg_i msg_read_request_bs__msg,
                                                       t_entier4* const msg_read_request_bs__nb);

#endif
