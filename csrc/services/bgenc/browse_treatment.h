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

 Date                 : 05/04/2019 08:04:44

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
#include "browse_treatment_context.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void browse_treatment__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void browse_treatment__clear_browse_results(void);
extern void browse_treatment__compute_browse_result(
   constants_statuscodes_bs__t_StatusCode_i * const browse_treatment__p_serviceStatusCode,
   constants__t_ContinuationPoint_i * const browse_treatment__p_continuationPoint,
   t_entier4 * const browse_treatment__p_nbReferences);
extern void browse_treatment__getall_and_clear_browse_results(
   constants__t_BrowseResult_i * const browse_treatment__p_browseResult);
extern void browse_treatment__getall_browse_result(
   const t_entier4 browse_treatment__p_refIndex,
   constants__t_NodeId_i * const browse_treatment__p_refTypeId,
   t_bool * const browse_treatment__p_isForward,
   constants__t_ExpandedNodeId_i * const browse_treatment__p_NodeId,
   constants__t_QualifiedName_i * const browse_treatment__p_BrowseName,
   constants__t_LocalizedText_i * const browse_treatment__p_DisplayName,
   constants__t_NodeClass_i * const browse_treatment__p_NodeClass,
   constants__t_ExpandedNodeId_i * const browse_treatment__p_TypeDefinition);
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
extern void browse_treatment__set_session_closed(
   const constants__t_session_i browse_treatment__p_session);

#endif
