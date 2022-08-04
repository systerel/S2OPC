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

 File Name            : browse_treatment_continuation_points_bs.h

 Date                 : 04/08/2022 14:53:29

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _browse_treatment_continuation_points_bs_h
#define _browse_treatment_continuation_points_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void browse_treatment_continuation_points_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void browse_treatment_continuation_points_bs__clear_continuation_point(
   const constants__t_ContinuationPoint_i browse_treatment_continuation_points_bs__p_continuationPoint);
extern void browse_treatment_continuation_points_bs__create_continuation_point_bs(
   const t_entier4 browse_treatment_continuation_points_bs__p_nextIndex,
   const t_entier4 browse_treatment_continuation_points_bs__p_maxTargetRef,
   const constants__t_NodeId_i browse_treatment_continuation_points_bs__p_browseView,
   const constants__t_NodeId_i browse_treatment_continuation_points_bs__p_nodeId,
   const constants__t_BrowseDirection_i browse_treatment_continuation_points_bs__p_browseDirection,
   const constants__t_NodeId_i browse_treatment_continuation_points_bs__p_referenceType,
   const t_bool browse_treatment_continuation_points_bs__p_includeSubtypes,
   const constants__t_BrowseNodeClassMask_i browse_treatment_continuation_points_bs__p_nodeClassMask,
   const constants__t_BrowseResultMask_i browse_treatment_continuation_points_bs__p_resultMask,
   t_bool * const browse_treatment_continuation_points_bs__bres,
   constants__t_ContinuationPoint_i * const browse_treatment_continuation_points_bs__p_ContinuationPoint);
extern void browse_treatment_continuation_points_bs__get_continuation_point_id(
   const constants__t_ContinuationPoint_i browse_treatment_continuation_points_bs__p_continuationPoint,
   constants__t_ContinuationPointId_i * const browse_treatment_continuation_points_bs__p_continuationPointId);
extern void browse_treatment_continuation_points_bs__getall_continuation_point_bs(
   const constants__t_ContinuationPoint_i browse_treatment_continuation_points_bs__p_ContinuationPoint,
   t_entier4 * const browse_treatment_continuation_points_bs__p_nextIndex,
   t_entier4 * const browse_treatment_continuation_points_bs__p_maxTargetRef,
   constants__t_NodeId_i * const browse_treatment_continuation_points_bs__p_browseView,
   constants__t_NodeId_i * const browse_treatment_continuation_points_bs__p_nodeId,
   constants__t_BrowseDirection_i * const browse_treatment_continuation_points_bs__p_browseDirection,
   constants__t_NodeId_i * const browse_treatment_continuation_points_bs__p_referenceType,
   t_bool * const browse_treatment_continuation_points_bs__p_includeSubtypes,
   constants__t_BrowseNodeClassMask_i * const browse_treatment_continuation_points_bs__p_nodeClassMask,
   constants__t_BrowseResultMask_i * const browse_treatment_continuation_points_bs__p_resultMask);

#endif
