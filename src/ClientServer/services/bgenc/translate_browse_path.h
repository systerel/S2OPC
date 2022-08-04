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

 File Name            : translate_browse_path.h

 Date                 : 04/08/2022 14:53:24

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _translate_browse_path_h
#define _translate_browse_path_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "browse_treatment.h"
#include "msg_translate_browse_path_bs.h"
#include "translate_browse_path_1.h"
#include "translate_browse_path_element_it.h"
#include "translate_browse_path_result_it.h"
#include "translate_browse_path_source_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_itf.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "node_id_pointer_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void translate_browse_path__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define translate_browse_path__alloc_browse_path_result msg_translate_browse_path_bs__alloc_browse_path_result
#define translate_browse_path__clear_browse_value_context browse_treatment__clear_browse_value_context
#define translate_browse_path__compute_browse_result browse_treatment__compute_browse_result
#define translate_browse_path__continuation_points_UNINITIALISATION browse_treatment__continuation_points_UNINITIALISATION
#define translate_browse_path__free_translate_browse_paths_response msg_translate_browse_path_bs__free_translate_browse_paths_response
#define translate_browse_path__getall_and_move_browse_result browse_treatment__getall_and_move_browse_result
#define translate_browse_path__read_nb_BrowsePaths msg_translate_browse_path_bs__read_nb_BrowsePaths
#define translate_browse_path__release_continuation_point browse_treatment__release_continuation_point
#define translate_browse_path__set_browse_value_context browse_treatment__set_browse_value_context
#define translate_browse_path__set_browse_value_context_from_continuation_point browse_treatment__set_browse_value_context_from_continuation_point
#define translate_browse_path__set_session_closed browse_treatment__set_session_closed
#define translate_browse_path__write_translate_browse_paths_response msg_translate_browse_path_bs__write_translate_browse_paths_response

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void translate_browse_path__checkAndAdd_BrowsePathRemaining(
   const constants__t_ExpandedNodeId_i translate_browse_path__expandedNodeId,
   const t_entier4 translate_browse_path__index,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation);
extern void translate_browse_path__checkAndAdd_BrowsePathResult(
   const constants__t_ExpandedNodeId_i translate_browse_path__expandedNodeId,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation);
extern void translate_browse_path__check_startingNode(
   const constants__t_NodeId_i translate_browse_path__nodeid,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__StatusCode);
extern void translate_browse_path__compute_browse_result_from_source(
   const constants__t_NodeId_i translate_browse_path__source,
   const constants__t_BrowseDirection_i translate_browse_path__browseDirection,
   const constants__t_NodeId_i translate_browse_path__referenceTypeId,
   const t_bool translate_browse_path__includedSubtypes,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation,
   t_entier4 * const translate_browse_path__nbReferences);
extern void translate_browse_path__copy_browsePathResult_to_msg(
   const constants__t_BrowsePath_i translate_browse_path__browsePath,
   const constants_statuscodes_bs__t_StatusCode_i translate_browse_path__in_statusCode_operation,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__out_statusCode_operation);
extern void translate_browse_path__copy_browsePathResult_to_source(
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation);
extern void translate_browse_path__free_BrowsePathRemaining(void);
extern void translate_browse_path__free_BrowsePathResult(void);
extern void translate_browse_path__free_BrowsePathSource(void);
extern void translate_browse_path__get_browseDirection_from_isInverse(
   const t_bool translate_browse_path__isInverse,
   constants__t_BrowseDirection_i * const translate_browse_path__browseDirection);
extern void translate_browse_path__get_translateStatus_from_browseStatus(
   const constants_statuscodes_bs__t_StatusCode_i translate_browse_path__browse_statusCode,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__translate_statusCode);
extern void translate_browse_path__treat_browse_result_one_source(
   const t_entier4 translate_browse_path__index,
   const constants__t_QualifiedName_i translate_browse_path__targetName,
   const t_entier4 translate_browse_path__nbReferences,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation);
extern void translate_browse_path__treat_one_relative_path_element(
   const constants__t_RelativePathElt_i translate_browse_path__relativePathElt,
   const t_entier4 translate_browse_path__index,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__statusCode_operation);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void translate_browse_path__init_translate_browse_paths_request(
   const constants__t_msg_i translate_browse_path__req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const translate_browse_path__StatusCode_service);
extern void translate_browse_path__treat_one_translate_browse_path(
   const constants__t_BrowsePath_i translate_browse_path__browsePath);

#endif
