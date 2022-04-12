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

/******************************************************************************

 File Name            : subscription_absent_node_bs.h

 Date                 : 05/08/2022 08:41:21

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _subscription_absent_node_bs_h
#define _subscription_absent_node_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void subscription_absent_node_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void subscription_absent_node_bs__absent_Node_is_known(
   const constants__t_endpoint_config_idx_i subscription_absent_node_bs__p_endpoint_config_idx,
   const constants__t_NodeId_i subscription_absent_node_bs__p_nid,
   t_bool * const subscription_absent_node_bs__bres_knownNode,
   constants__t_NodeClass_i * const subscription_absent_node_bs__knownNodeClass,
   constants__t_RawStatusCode * const subscription_absent_node_bs__valueSc);
extern void subscription_absent_node_bs__nodeId_do_nothing(
   const constants__t_NodeId_i subscription_absent_node_bs__p_nid);

#endif
