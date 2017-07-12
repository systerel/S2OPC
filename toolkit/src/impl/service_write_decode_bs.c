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

/** \file
 *
 * Implements the structures behind the address space.
 */


#include "b2c.h"
#include "service_write_decode_bs.h"


/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_write_decode_bs__INITIALISATION(void)
{
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_write_decode_bs__decode_write_request(
   const constants__t_ByteString_i service_write_decode_bs__req_payload,
   constants__t_StatusCode_i * const service_write_decode_bs__StatusCode_service)
{
}


extern void service_write_decode_bs__free_write_request(void)
{
}


extern void service_write_decode_bs__get_nb_WriteValue(
   t_entier4 * const service_write_decode_bs__nb_req)
{
}


extern void service_write_decode_bs__getall_WriteValue(
   const constants__t_WriteValue_i service_write_decode_bs__wvi,
   t_bool * const service_write_decode_bs__isvalid,
   constants__t_NodeId_i * const service_write_decode_bs__nid,
   constants__t_AttributeId_i * const service_write_decode_bs__aid,
   constants__t_Variant_i * const service_write_decode_bs__value)
{
}

