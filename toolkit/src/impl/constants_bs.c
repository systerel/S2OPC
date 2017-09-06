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
 * Implements the base machine for the constants
 */


#include "b2c.h"
#include "constants_bs.h"

void constants_bs__INITIALISATION(void)
{
}


/*--------------------
   OPERATIONS Clause
  --------------------*/
void constants_bs__get_Is_SubType(
   const constants_bs__t_NodeId_i constants_bs__p_type1,
   const constants_bs__t_NodeId_i constants_bs__p_type2,
   t_bool * const constants_bs__p_res)
{
    *constants_bs__p_res = true;
}


void constants_bs__getall_conv_ExpandedNodeId_NodeId(
   const constants_bs__t_ExpandedNodeId_i constants_bs__p_expnid,
   t_bool * const constants_bs__p_isvalid,
   constants_bs__t_NodeId_i * const constants_bs__p_nid)
{
}

