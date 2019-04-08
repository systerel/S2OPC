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

 File Name            : browse_treatment.h

 Date                 : 08/04/2019 16:46:08

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _browse_treatment_h
#define _browse_treatment_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "browse_treatment_1.h"
#include "browse_treatment_context.h"
#include "browse_treatment_continuation_points.h"
#include "browse_treatment_result_bs.h"
#include "browse_treatment_result_it.h"
#include "browse_treatment_target_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void browse_treatment__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define browse_treatment__clear_browse_result browse_treatment_result_bs__clear_browse_result
#define browse_treatment__getall_and_clear_browse_result browse_treatment_result_bs__getall_and_clear_browse_result
#define browse_treatment__getall_browse_result_reference_at browse_treatment_result_bs__getall_browse_result_reference_at
#define browse_treatment__set_session_closed browse_treatment_continuation_points__set_session_closed

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void browse_treatment__fill_browse_result(
   const t_entier4 browse_treatment__p_startIndex,
   const t_entier4 browse_treatment__p_max_nb_results,
   const constants__t_NodeId_i browse_treatment__p_browseView,
   const constants__t_Node_i browse_treatment__p_src_node,
   const constants__t_BrowseDirection_i browse_treatment__p_browseDirection,
   const t_bool browse_treatment__p_refType_defined,
   const constants__t_NodeId_i browse_treatment__p_referenceType,
   const t_bool browse_treatment__p_includeSubtypes,
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_serviceStatusCode,
   t_bool * const browse_treatment__p_toContinue,
   t_entier4 * const browse_treatment__p_nextIndex);
extern void browse_treatment__fill_browse_result_ref(
   const constants__t_Reference_i browse_treatment__p_ref,
   const constants__t_BrowseDirection_i browse_treatment__p_browseDirection,
   const t_bool browse_treatment__p_refType_defined,
   const constants__t_NodeId_i browse_treatment__p_referenceType,
   const t_bool browse_treatment__p_includeSubtypes,
   t_bool * const browse_treatment__p_continue);
extern void browse_treatment__min_max_nb_result_refs(
   const t_entier4 browse_treatment__p_maxTargetRef,
   const t_entier4 browse_treatment__p_nb_target,
   t_entier4 * const browse_treatment__p_maxResultRefs);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void browse_treatment__compute_browse_result(
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_serviceStatusCode,
   constants__t_ContinuationPoint_i * const browse_treatment__p_continuationPoint,
   t_entier4 * const browse_treatment__p_nbReferences);
extern void browse_treatment__set_browse_value_context(
   const constants__t_session_i browse_treatment__p_session,
   const t_entier4 browse_treatment__p_maxTargetRef,
   const constants__t_NodeId_i browse_treatment__p_browseView,
   const constants__t_NodeId_i browse_treatment__p_nodeId,
   const constants__t_BrowseDirection_i browse_treatment__p_browseDirection,
   const constants__t_NodeId_i browse_treatment__p_referenceType,
   const t_bool browse_treatment__p_includeSubtypes);
extern void browse_treatment__set_browse_value_context_from_continuation_point(
   const constants__t_session_i browse_treatment__p_session,
   const constants__t_ContinuationPoint_i browse_treatment__p_continuationPoint,
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_service_StatusCode);

#endif
