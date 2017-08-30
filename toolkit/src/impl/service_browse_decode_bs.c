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

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_browse_decode_bs.h"


/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_browse_decode_bs__INITIALISATION(void)
{
}


/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_browse_decode_bs__decode_browse_request(
   const constants__t_byte_buffer_i service_browse_decode_bs__req_payload,
   constants__t_StatusCode_i * const service_browse_decode_bs__StatusCode_service)
{
}


void service_browse_decode_bs__free_browse_request(void)
{
}


void service_browse_decode_bs__get_nb_BrowseValue(
   t_entier4 * const service_browse_decode_bs__nb_req)
{
}


void service_browse_decode_bs__get_nb_BrowseTargetMax(
   t_entier4 * const service_browse_decode_bs__p_nb_BrowseTargetMax)
{
}


void service_browse_decode_bs__getall_BrowseValue(
   const constants__t_BrowseValue_i service_browse_decode_bs__p_bvi,
   t_bool * const service_browse_decode_bs__p_isvalid,
   constants__t_NodeId_i * const service_browse_decode_bs__p_NodeId,
   constants__t_BrowseDirection_i * const service_browse_decode_bs__p_dir,
   t_bool * const service_browse_decode_bs__p_isreftype,
   constants__t_NodeId_i * const service_browse_decode_bs__p_reftype,
   t_bool * const service_browse_decode_bs__p_inc_subtype)
{
}


