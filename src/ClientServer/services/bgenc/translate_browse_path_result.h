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

 File Name            : translate_browse_path_result.h

 Date                 : 27/03/2024 10:00:26

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _translate_browse_path_result_h
#define _translate_browse_path_result_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "browse_treatment.h"
#include "translate_browse_path_result_1.h"
#include "translate_browse_path_result_1_it.h"
#include "translate_browse_path_result_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_itf.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "msg_translate_browse_path_bs.h"
#include "node_id_pointer_bs.h"
#include "translate_browse_path_source.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void translate_browse_path_result__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define translate_browse_path_result__clear_browse_value_context browse_treatment__clear_browse_value_context
#define translate_browse_path_result__compute_browse_result browse_treatment__compute_browse_result
#define translate_browse_path_result__continuation_points_UNINITIALISATION browse_treatment__continuation_points_UNINITIALISATION
#define translate_browse_path_result__get_BrowsePathRemaining translate_browse_path_result_1__get_BrowsePathRemaining
#define translate_browse_path_result__get_BrowsePathRemainingSize translate_browse_path_result_1__get_BrowsePathRemainingSize
#define translate_browse_path_result__get_BrowsePathResult translate_browse_path_result_1__get_BrowsePathResult
#define translate_browse_path_result__get_BrowsePathResultSize translate_browse_path_result_1__get_BrowsePathResultSize
#define translate_browse_path_result__getall_and_move_browse_result browse_treatment__getall_and_move_browse_result
#define translate_browse_path_result__release_continuation_point browse_treatment__release_continuation_point
#define translate_browse_path_result__set_browse_value_context browse_treatment__set_browse_value_context
#define translate_browse_path_result__set_browse_value_context_from_continuation_point browse_treatment__set_browse_value_context_from_continuation_point
#define translate_browse_path_result__set_session_closed browse_treatment__set_session_closed

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void translate_browse_path_result__checkAndAdd_BrowsePathRemaining(
   const constants__t_ExpandedNodeId_i translate_browse_path_result__expandedNodeId,
   const t_entier4 translate_browse_path_result__path_index,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation);
extern void translate_browse_path_result__checkAndAdd_BrowsePathResult(
   const constants__t_ExpandedNodeId_i translate_browse_path_result__expandedNodeId,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation);
extern void translate_browse_path_result__compute_browse_result_from_source(
   const constants__t_NodeId_i translate_browse_path_result__source,
   const constants__t_BrowseDirection_i translate_browse_path_result__browseDirection,
   const constants__t_NodeId_i translate_browse_path_result__referenceTypeId,
   const t_bool translate_browse_path_result__includedSubtypes,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation,
   t_entier4 * const translate_browse_path_result__nbReferences);
extern void translate_browse_path_result__get_browseDirection_from_isInverse(
   const t_bool translate_browse_path_result__isInverse,
   constants__t_BrowseDirection_i * const translate_browse_path_result__browseDirection);
extern void translate_browse_path_result__get_translateStatus_from_browseStatus(
   const constants_statuscodes_bs__t_StatusCode_i translate_browse_path_result__browse_statusCode,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__translate_statusCode);
extern void translate_browse_path_result__treat_browse_result_one_source(
   const t_entier4 translate_browse_path_result__path_index,
   const constants__t_QualifiedName_i translate_browse_path_result__targetName,
   const t_entier4 translate_browse_path_result__nbReferences,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation);
extern void translate_browse_path_result__treat_browse_result_one_source_1(
   const t_entier4 translate_browse_path_result__path_index,
   const constants__t_QualifiedName_i translate_browse_path_result__targetName,
   const t_entier4 translate_browse_path_result__browseResult_index,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation);
extern void translate_browse_path_result__treat_one_relative_path_element_1(
   const constants__t_BrowseDirection_i translate_browse_path_result__browseDirection,
   const constants__t_NodeId_i translate_browse_path_result__referenceTypeId,
   const t_bool translate_browse_path_result__includedSubtypes,
   const t_entier4 translate_browse_path_result__path_index,
   const constants__t_QualifiedName_i translate_browse_path_result__targetName,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation);
extern void translate_browse_path_result__treat_one_relative_path_element_2(
   const t_entier4 translate_browse_path_result__loop_index,
   const constants__t_BrowseDirection_i translate_browse_path_result__browseDirection,
   const constants__t_NodeId_i translate_browse_path_result__referenceTypeId,
   const t_bool translate_browse_path_result__includedSubtypes,
   const t_entier4 translate_browse_path_result__path_index,
   const constants__t_QualifiedName_i translate_browse_path_result__targetName,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void translate_browse_path_result__free_BrowsePathRemaining(void);
extern void translate_browse_path_result__free_BrowsePathResult(void);
extern void translate_browse_path_result__treat_one_relative_path_element(
   const constants__t_RelativePathElt_i translate_browse_path_result__relativePathElt,
   const t_entier4 translate_browse_path_result__path_index,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path_result__statusCode_operation);

#endif
