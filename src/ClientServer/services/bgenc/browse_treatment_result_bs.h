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

 File Name            : browse_treatment_result_bs.h

 Date                 : 04/08/2022 14:53:29

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _browse_treatment_result_bs_h
#define _browse_treatment_result_bs_h

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
extern void browse_treatment_result_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void browse_treatment_result_bs__alloc_browse_result(
   const t_entier4 browse_treatment_result_bs__p_maxResultRefs,
   t_bool * const browse_treatment_result_bs__p_alloc_bres);
extern void browse_treatment_result_bs__clear_browse_result(void);
extern void browse_treatment_result_bs__get_browse_result_nb_references(
   t_entier4 * const browse_treatment_result_bs__p_nb_references);
extern void browse_treatment_result_bs__getall_and_move_browse_result(
   t_entier4 * const browse_treatment_result_bs__p_nb_references,
   constants__t_BrowseResultReferences_i * const browse_treatment_result_bs__p_browseResult);
extern void browse_treatment_result_bs__getall_browse_result_reference_at(
   const t_entier4 browse_treatment_result_bs__p_refIndex,
   constants__t_NodeId_i * const browse_treatment_result_bs__p_refTypeId,
   t_bool * const browse_treatment_result_bs__p_isForward,
   constants__t_ExpandedNodeId_i * const browse_treatment_result_bs__p_NodeId,
   constants__t_QualifiedName_i * const browse_treatment_result_bs__p_BrowseName,
   constants__t_LocalizedText_i * const browse_treatment_result_bs__p_DisplayName,
   constants__t_NodeClass_i * const browse_treatment_result_bs__p_NodeClass,
   constants__t_ExpandedNodeId_i * const browse_treatment_result_bs__p_TypeDefinition);
extern void browse_treatment_result_bs__is_BrowseName_in_mask(
   const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
   t_bool * const browse_treatment_result_bs__bres);
extern void browse_treatment_result_bs__is_DisplayName_in_mask(
   const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
   t_bool * const browse_treatment_result_bs__bres);
extern void browse_treatment_result_bs__is_IsForward_in_mask(
   const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
   t_bool * const browse_treatment_result_bs__bres);
extern void browse_treatment_result_bs__is_NodeClass_in_mask(
   const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
   t_bool * const browse_treatment_result_bs__bres);
extern void browse_treatment_result_bs__is_ReferenceType_in_mask(
   const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
   t_bool * const browse_treatment_result_bs__bres);
extern void browse_treatment_result_bs__is_TypeDefinition_in_mask(
   const constants__t_BrowseResultMask_i browse_treatment_result_bs__p_resultMask,
   t_bool * const browse_treatment_result_bs__bres);
extern void browse_treatment_result_bs__setall_browse_result_reference_at(
   const t_entier4 browse_treatment_result_bs__p_refIndex,
   const constants__t_NodeId_i browse_treatment_result_bs__p_refTypeId,
   const t_bool browse_treatment_result_bs__p_isForward,
   const constants__t_ExpandedNodeId_i browse_treatment_result_bs__p_NodeId,
   const constants__t_QualifiedName_i browse_treatment_result_bs__p_BrowseName,
   const constants__t_LocalizedText_i browse_treatment_result_bs__p_DisplayName,
   const constants__t_NodeClass_i browse_treatment_result_bs__p_NodeClass,
   const constants__t_ExpandedNodeId_i browse_treatment_result_bs__p_TypeDefinition,
   t_bool * const browse_treatment_result_bs__p_alloc_failed);
extern void browse_treatment_result_bs__unused_browse_view(
   const constants__t_NodeId_i browse_treatment_result_bs__p_browseView);

#endif
