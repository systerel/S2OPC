/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/** \file
 *
 * Implements the base machine for the constants
 */

#include "constants_bs.h"
#include "b2c.h"

#include "sopc_builtintypes.h"
#include "sopc_types.h"

static SOPC_NodeId ByteString_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_ByteString};
static SOPC_NodeId Byte_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Byte};
static SOPC_NodeId Null_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = 0};

const constants_bs__t_NodeId_i constants_bs__c_ByteString_Type_NodeId = &ByteString_Type;
const constants_bs__t_NodeId_i constants_bs__c_Byte_Type_NodeId = &Byte_Type;
const constants_bs__t_NodeId_i constants_bs__c_Null_Type_NodeId = &Null_Type;

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
                                                     constants_bs__t_NodeId_i* const constants_bs__p_nid)
{
    /* Reminder: This is a borrow */
    *constants_bs__p_nid = &constants_bs__p_expnid->NodeId;
}

void constants_bs__is_t_access_level_currentRead(const constants_bs__t_access_level constants_bs__p_access_lvl,
                                                 t_bool* const constants_bs__bres)
{
    *constants_bs__bres = (constants_bs__p_access_lvl & SOPC_AccessLevelMask_CurrentRead) != 0;
}

void constants_bs__is_t_access_level_currentWrite(const constants_bs__t_access_level constants_bs__p_access_lvl,
                                                  t_bool* const constants_bs__bres)
{
    *constants_bs__bres = (constants_bs__p_access_lvl & SOPC_AccessLevelMask_CurrentWrite) != 0;
}

void constants_bs__is_t_access_level_statusWrite(const constants_bs__t_access_level constants_bs__p_access_lvl,
                                                 t_bool* const constants_bs__bres)
{
    *constants_bs__bres = (constants_bs__p_access_lvl & SOPC_AccessLevelMask_StatusWrite) != 0;
}

void constants_bs__is_t_access_level_timestampWrite(const constants_bs__t_access_level constants_bs__p_access_lvl,
                                                    t_bool* const constants_bs__bres)
{
    *constants_bs__bres = (constants_bs__p_access_lvl & SOPC_AccessLevelMask_TimestampWrite) != 0;
}

void constants_bs__get_card_t_channel(t_entier4* const constants_bs__p_card_channel)
{
    *constants_bs__p_card_channel = constants_bs__t_channel_i_max;
}

void constants_bs__get_card_t_session(t_entier4* const constants_bs__p_card_session)
{
    *constants_bs__p_card_session = constants_bs__t_session_i_max;
}

void constants_bs__get_card_t_subscription(t_entier4* const constants_bs__p_card_subscription)
{
    *constants_bs__p_card_subscription = constants_bs__t_subscription_i_max;
}

void constants_bs__get_cast_t_channel(const t_entier4 constants_bs__p_ind,
                                      constants_bs__t_channel_i* const constants_bs__p_channel)
{
    *constants_bs__p_channel = (uint32_t) constants_bs__p_ind; // TODO: add precondition in B model
}

void constants_bs__get_cast_t_session(const t_entier4 constants_bs__p_ind,
                                      constants_bs__t_session_i* const constants_bs__p_session)
{
    *constants_bs__p_session = (uint32_t) constants_bs__p_ind; // TODO: add precondition in B model
}

void constants_bs__get_cast_t_subscription(const t_entier4 constants_bs__p_ind,
                                           constants_bs__t_subscription_i* const constants_bs__p_subscription)
{
    *constants_bs__p_subscription = (uint32_t) constants_bs__p_ind; // TODO: add precondition in B model
}

void constants_bs__is_t_channel(const constants_bs__t_channel_i constants_bs__p_channel,
                                t_bool* const constants_bs__p_res)
{
    *constants_bs__p_res = (constants_bs__c_channel_indet != constants_bs__p_channel && constants_bs__p_channel > 0 &&
                            constants_bs__p_channel <= constants_bs__t_channel_i_max);
}

void constants_bs__is_t_channel_config_idx(const constants_bs__t_channel_config_idx_i constants_bs__p_config_idx,
                                           t_bool* const constants_bs__p_res)
{
    *constants_bs__p_res =
        (constants_bs__c_channel_config_idx_indet != constants_bs__p_config_idx && constants_bs__p_config_idx > 0 &&
         constants_bs__p_config_idx <= constants_bs__t_channel_config_idx_i_max);
}

void constants_bs__is_t_endpoint_config_idx(
    const constants_bs__t_endpoint_config_idx_i constants_bs__p_endpoint_config_idx,
    t_bool* const constants_bs__p_res)
{
    *constants_bs__p_res = (constants_bs__p_endpoint_config_idx != constants_bs__c_endpoint_config_idx_indet &&
                            constants_bs__p_endpoint_config_idx > 0 &&
                            constants_bs__p_endpoint_config_idx <= constants_bs__t_endpoint_config_idx_i_max);
}
