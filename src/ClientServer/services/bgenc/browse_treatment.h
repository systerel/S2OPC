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

 Date                 : 04/08/2022 14:53:02

 C Translator Version : tradc Java V1.2 (06/02/2022)

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
#include "address_space_itf.h"
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
#define browse_treatment__clear_browse_value_context browse_treatment_context__clear_browse_value_context
#define browse_treatment__continuation_points_UNINITIALISATION browse_treatment_continuation_points__continuation_points_UNINITIALISATION
#define browse_treatment__getall_and_move_browse_result browse_treatment_result_bs__getall_and_move_browse_result
#define browse_treatment__getall_browse_result_reference_at browse_treatment_result_bs__getall_browse_result_reference_at
#define browse_treatment__release_continuation_point browse_treatment_continuation_points__release_continuation_point
#define browse_treatment__set_session_closed browse_treatment_continuation_points__set_session_closed

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void browse_treatment__apply_result_mask_filter(
   const constants__t_BrowseResultMask_i browse_treatment__p_resultMask,
   const constants__t_NodeId_i browse_treatment__p_RefType,
   const t_bool browse_treatment__p_IsForward,
   const constants__t_QualifiedName_i browse_treatment__p_BrowseName,
   const constants__t_LocalizedText_i browse_treatment__p_DisplayName,
   const constants__t_NodeClass_i browse_treatment__p_NodeClass,
   const constants__t_ExpandedNodeId_i browse_treatment__p_TypeDefinition,
   constants__t_NodeId_i * const browse_treatment__out_RefType,
   t_bool * const browse_treatment__out_IsForward,
   constants__t_QualifiedName_i * const browse_treatment__out_BrowseName,
   constants__t_LocalizedText_i * const browse_treatment__out_DisplayName,
   constants__t_NodeClass_i * const browse_treatment__out_NodeClass,
   constants__t_ExpandedNodeId_i * const browse_treatment__out_TypeDefinition);
extern void browse_treatment__fill_browse_result(
   const t_entier4 browse_treatment__p_startIndex,
   const t_entier4 browse_treatment__p_max_nb_results,
   const constants__t_NodeId_i browse_treatment__p_browseView,
   const constants__t_Node_i browse_treatment__p_src_node,
   const constants__t_BrowseDirection_i browse_treatment__p_browseDirection,
   const t_bool browse_treatment__p_refType_defined,
   const constants__t_NodeId_i browse_treatment__p_referenceType,
   const t_bool browse_treatment__p_includeSubtypes,
   const constants__t_BrowseResultMask_i browse_treatment__p_resultMask,
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_serviceStatusCode,
   t_bool * const browse_treatment__p_toContinue,
   t_entier4 * const browse_treatment__p_nextIndex);
extern void browse_treatment__fill_browse_result_ref(
   const constants__t_Reference_i browse_treatment__p_ref,
   const constants__t_NodeId_i browse_treatment__p_browseView,
   const constants__t_BrowseDirection_i browse_treatment__p_browseDirection,
   const t_bool browse_treatment__p_refType_defined,
   const constants__t_NodeId_i browse_treatment__p_referenceType,
   const t_bool browse_treatment__p_includeSubtypes,
   const constants__t_BrowseResultMask_i browse_treatment__p_resultMask,
   t_bool * const browse_treatment__p_continue,
   t_bool * const browse_treatment__p_alloc_failed);
extern void browse_treatment__local_is_valid_ReferenceTypeId(
   const t_bool browse_treatment__p_refType_defined,
   const constants__t_NodeId_i browse_treatment__p_referenceTypeId,
   t_bool * const browse_treatment__bres);
extern void browse_treatment__min_nb_result_refs(
   const t_entier4 browse_treatment__p_maxTargetRef,
   const t_entier4 browse_treatment__p_nb_target,
   t_entier4 * const browse_treatment__p_maxResultRefs);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void browse_treatment__compute_browse_result(
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_serviceStatusCode,
   constants__t_ContinuationPointId_i * const browse_treatment__p_continuationPointId,
   t_entier4 * const browse_treatment__p_nbReferences);
extern void browse_treatment__set_browse_value_context(
   const constants__t_session_i browse_treatment__p_session,
   const t_entier4 browse_treatment__p_maxTargetRef,
   const constants__t_NodeId_i browse_treatment__p_browseView,
   const constants__t_NodeId_i browse_treatment__p_nodeId,
   const constants__t_BrowseDirection_i browse_treatment__p_browseDirection,
   const constants__t_NodeId_i browse_treatment__p_referenceType,
   const t_bool browse_treatment__p_includeSubtypes,
   const constants__t_BrowseNodeClassMask_i browse_treatment__p_nodeClassMask,
   const constants__t_BrowseResultMask_i browse_treatment__p_resultMask,
   const t_bool browse_treatment__p_autoReleaseCP,
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_service_StatusCode);
extern void browse_treatment__set_browse_value_context_from_continuation_point(
   const constants__t_session_i browse_treatment__p_session,
   const constants__t_ContinuationPointId_i browse_treatment__p_continuationPointId,
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_service_StatusCode);

#endif
