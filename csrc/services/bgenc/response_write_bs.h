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

 File Name            : response_write_bs.h

 Date                 : 05/02/2018 16:15:27

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _response_write_bs_h
#define _response_write_bs_h

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
extern void response_write_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void response_write_bs__alloc_write_request_responses_malloc(
    const t_entier4 response_write_bs__nb_req,
    t_bool* const response_write_bs__ResponseWrite_allocated);
extern void response_write_bs__getall_ResponseWrite_StatusCode(const constants__t_WriteValue_i response_write_bs__wvi,
                                                               t_bool* const response_write_bs__isvalid,
                                                               constants__t_StatusCode_i* const response_write_bs__sc);
extern void response_write_bs__reset_ResponseWrite(void);
extern void response_write_bs__set_ResponseWrite_StatusCode(const constants__t_WriteValue_i response_write_bs__wvi,
                                                            const constants__t_StatusCode_i response_write_bs__sc);
extern void response_write_bs__write_WriteResponse_msg_out(const constants__t_msg_i response_write_bs__msg_out);

#endif
