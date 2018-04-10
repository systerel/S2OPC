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

/** \file
 *
 * Implements the base machine for the constants
 */

#include "constants_bs.h"
#include "b2c.h"

#include "sopc_types.h"

void constants_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void constants_bs__get_Is_SubType(const constants_bs__t_NodeId_i constants_bs__p_type1,
                                  const constants_bs__t_NodeId_i constants_bs__p_type2,
                                  t_bool* const constants_bs__p_res)
{
    (void) constants_bs__p_type1;
    (void) constants_bs__p_type2;
    /* TODO: implement a functional subtype query */
    *constants_bs__p_res = true;
}

void constants_bs__getall_conv_ExpandedNodeId_NodeId(const constants_bs__t_ExpandedNodeId_i constants_bs__p_expnid,
                                                     t_bool* const constants_bs__p_isvalid,
                                                     constants_bs__t_NodeId_i* const constants_bs__p_nid)
{
    /* Reminder: This is a borrow */
    *constants_bs__p_nid = constants_bs__c_NodeId_indet;
    if (constants_bs__p_expnid != constants_bs__c_ExpandedNodeId_indet)
    {
        *constants_bs__p_nid = (constants_bs__t_NodeId_i) & ((SOPC_ExpandedNodeId*) constants_bs__p_expnid)->NodeId;
    }
    if (NULL == constants_bs__p_nid)
        *constants_bs__p_isvalid = false;
    else
        *constants_bs__p_isvalid = true;
}

void constants_bs__get_card_t_channel(t_entier4* const constants_bs__p_card_channel)
{
    *constants_bs__p_card_channel = constants_bs__t_channel_i_max;
}

void constants_bs__get_card_t_session(t_entier4* const constants_bs__p_card_session)
{
    *constants_bs__p_card_session = constants_bs__t_session_i_max;
}

void constants_bs__get_cast_t_channel(const t_entier4 constants_bs__p_ind,
                                      constants_bs__t_channel_i* const constants_bs__p_channel)
{
    *constants_bs__p_channel = constants_bs__p_ind;
}

void constants_bs__get_cast_t_session(const t_entier4 constants_bs__p_ind,
                                      constants_bs__t_session_i* const constants_bs__p_session)
{
    *constants_bs__p_session = constants_bs__p_ind;
}

void constants_bs__is_t_channel(const constants_bs__t_channel_i constants_bs__p_channel,
                                t_bool* const constants_bs__p_res)
{
    *constants_bs__p_res = (constants_bs__c_channel_indet != constants_bs__p_channel && constants_bs__p_channel > 0 &&
                            constants_bs__p_channel <= SOPC_MAX_SECURE_CONNECTIONS);
}

void constants_bs__is_t_channel_config_idx(const constants_bs__t_channel_config_idx_i constants_bs__p_config_idx,
                                           t_bool* const constants_bs__p_res)
{
    *constants_bs__p_res =
        (constants_bs__c_channel_config_idx_indet != constants_bs__p_config_idx && constants_bs__p_config_idx > 0 &&
         constants_bs__p_config_idx <= SOPC_MAX_SECURE_CONNECTIONS * 2); // (1 for client side + 1 for server side)
}

void constants_bs__is_t_endpoint_config_idx(
    const constants_bs__t_endpoint_config_idx_i constants_bs__p_endpoint_config_idx,
    t_bool* const constants_bs__p_res)
{
    *constants_bs__p_res = (constants_bs__p_endpoint_config_idx != constants_bs__c_endpoint_config_idx_indet &&
                            constants_bs__p_endpoint_config_idx > 0 &&
                            constants_bs__p_endpoint_config_idx <= SOPC_MAX_ENDPOINT_DESCRIPTION_CONFIGURATIONS);
}
