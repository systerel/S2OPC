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

 File Name            : msg_node_management_add_nodes_bs.h

 Date                 : 16/06/2025 14:30:23

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_node_management_add_nodes_bs_h
#define _msg_node_management_add_nodes_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_node_management_add_nodes_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_node_management_add_nodes_bs__alloc_msg_add_nodes_resp_results(
   const constants__t_msg_i msg_node_management_add_nodes_bs__p_resp_msg,
   const t_entier4 msg_node_management_add_nodes_bs__p_nb_results,
   t_bool * const msg_node_management_add_nodes_bs__bres);
extern void msg_node_management_add_nodes_bs__get_msg_add_nodes_req_nb_add_nodes(
   const constants__t_msg_i msg_node_management_add_nodes_bs__p_req_msg,
   t_entier4 * const msg_node_management_add_nodes_bs__p_nb_add_nodes);
extern void msg_node_management_add_nodes_bs__getall_add_node_item_req_params(
   const constants__t_msg_i msg_node_management_add_nodes_bs__p_req_msg,
   const t_entier4 msg_node_management_add_nodes_bs__p_index,
   constants_statuscodes_bs__t_StatusCode_i * const msg_node_management_add_nodes_bs__p_sc,
   constants__t_ExpandedNodeId_i * const msg_node_management_add_nodes_bs__p_parentExpNid,
   constants__t_NodeId_i * const msg_node_management_add_nodes_bs__p_refTypeId,
   constants__t_ExpandedNodeId_i * const msg_node_management_add_nodes_bs__p_reqExpNodeId,
   constants__t_QualifiedName_i * const msg_node_management_add_nodes_bs__p_browseName,
   constants__t_NodeClass_i * const msg_node_management_add_nodes_bs__p_nodeClass,
   constants__t_NodeAttributes_i * const msg_node_management_add_nodes_bs__p_nodeAttributes,
   constants__t_ExpandedNodeId_i * const msg_node_management_add_nodes_bs__p_typeDefId);
extern void msg_node_management_add_nodes_bs__setall_msg_add_nodes_item_resp_params(
   const constants__t_msg_i msg_node_management_add_nodes_bs__p_resp_msg,
   const t_entier4 msg_node_management_add_nodes_bs__p_index,
   const constants_statuscodes_bs__t_StatusCode_i msg_node_management_add_nodes_bs__p_sc,
   const constants__t_NodeId_i msg_node_management_add_nodes_bs__p_nid);

#endif
