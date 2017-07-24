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
 * Hand-modified _bs.h to specify my own #defines
 */


#ifndef _constants_bs_h
#define _constants_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------------------
   SETS Clause: deferred sets
  -----------------------------*/

typedef void * constants_bs__t_ByteString_i;
typedef void * constants_bs__t_NodeId_i;
#define constants_bs__t_Node_i t_entier4
#define constants_bs__t_UserId_i t_entier4
typedef void * constants_bs__t_Variant_i;
typedef void* constants_bs__t_msg_i;
typedef void* constants_bs__t_session_token_i;
typedef void* constants_bs__t_session_i;
#define constants_bs__t_channel_i t_entier4
#define constants_bs__t_channel_config_idx_i t_entier4
#define constants_bs__t_endpoint_config_idx_i t_entier4
#define constants_bs__t_request_handle_i t_entier4
#define constants_bs__t_user_i t_entier4


/*--------------------------
   Added by the Translator
  --------------------------*/
#define constants_bs__t_ByteString_i_max (-1)
#define constants_bs__t_NodeId_i_max (-1)
#define constants_bs__t_Node_i_max (-1)
#define constants_bs__t_UserId_i_max (-1)
#define constants_bs__t_Variant_i_max (-1)
#define constants_bs__t_channel_config_idx_i_max (-1)
#define constants_bs__t_channel_i_max (-1)
#define constants_bs__t_endpoint_config_idx_i_max (-1)
#define constants_bs__t_msg_i_max (-1)
#define constants_bs__t_request_handle_i_max (-1)
#define constants_bs__t_session_i_max (-1)
#define constants_bs__t_session_token_i_max (-1)
#define constants_bs__t_user_i_max (-1)

/*------------------------------------------------
   CONCRETE_CONSTANTS Clause: scalars and arrays
  ------------------------------------------------*/
#define constants_bs__c_ByteString_indet 0
#define constants_bs__c_NodeId_indet 0
#define constants_bs__c_Node_indet 0
#define constants_bs__c_UserId_indet 0
#define constants_bs__c_Variant_indet 0
#define constants_bs__c_channel_config_idx_indet 0
#define constants_bs__c_channel_indet 0
#define constants_bs__c_endpoint_config_idx_indet 0
#define constants_bs__c_msg_indet 0
#define constants_bs__c_request_handle_indet 0
#define constants_bs__c_session_indet 0
#define constants_bs__c_session_token_indet 0
#define constants_bs__c_user_indet 0

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void constants_bs__INITIALISATION(void);

#endif
