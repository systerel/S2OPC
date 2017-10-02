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

 File Name            : service_get_endpoints_bs.h

 Date                 : 29/09/2017 10:52:03

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_get_endpoints_bs_h
#define _service_get_endpoints_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_get_endpoints_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_get_endpoints_bs__treat_get_endpoints_request(
   const constants__t_msg_i service_get_endpoints_bs__req_msg,
   const constants__t_msg_i service_get_endpoints_bs__resp_msg,
   const constants__t_endpoint_config_idx_i service_get_endpoints_bs__endpoint_config_idx,
   constants__t_StatusCode_i * const service_get_endpoints_bs__ret);

#endif
