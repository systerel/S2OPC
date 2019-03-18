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

 File Name            : service_browse.h

 Date                 : 29/01/2019 12:57:50

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_browse_h
#define _service_browse_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_browse_response_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_browse__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_browse__free_browse_result msg_browse_response_bs__free_browse_result
#define service_browse__set_ResponseBrowse_BrowseStatus msg_browse_response_bs__set_ResponseBrowse_BrowseStatus
#define service_browse__write_BrowseResponse_msg_out msg_browse_response_bs__write_BrowseResponse_msg_out

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_browse__Is_RefTypes_Compatible(
   const t_bool service_browse__p_is_ref_type1,
   const constants__t_NodeId_i service_browse__p_ref_type1,
   const t_bool service_browse__p_inc_subtypes,
   const constants__t_NodeId_i service_browse__p_ref_type2,
   t_bool * const service_browse__p_ref_types_compat);
extern void service_browse__alloc_browse_response(
   const t_entier4 service_browse__p_nb_bvi,
   t_bool * const service_browse__p_isallocated);
extern void service_browse__alloc_browse_result(
   const constants__t_BrowseValue_i service_browse__p_bvi,
   const t_entier4 service_browse__p_nb_target_max,
   const t_entier4 service_browse__p_nb_target,
   t_bool * const service_browse__p_isallocated,
   t_entier4 * const service_browse__p_nb_bri);
extern void service_browse__copy_target_node_browse_result(
   const constants__t_BrowseValue_i service_browse__p_bvi,
   const constants__t_BrowseResult_i service_browse__p_bri,
   const constants__t_NodeId_i service_browse__p_RefType,
   const constants__t_ExpandedNodeId_i service_browse__p_NodeId,
   const t_bool service_browse__p_IsForward,
   t_bool * const service_browse__p_res);
extern void service_browse__fill_continuation_point(
   const constants__t_BrowseValue_i service_browse__p_bvi,
   const t_bool service_browse__p_continue_ref,
   const constants__t_Reference_i service_browse__p_ref);
extern void service_browse__getall_SourceNode_NbRef(
   const constants__t_NodeId_i service_browse__p_src_nodeid,
   t_bool * const service_browse__p_isvalid,
   t_entier4 * const service_browse__p_nb_ref,
   constants__t_Node_i * const service_browse__p_src_node);

#endif
