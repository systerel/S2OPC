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

 File Name            : browse_treatment_continuation_points.h

 Date                 : 10/04/2019 12:55:44

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _browse_treatment_continuation_points_h
#define _browse_treatment_continuation_points_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "browse_treatment_continuation_points_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "session_mgr.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void browse_treatment_continuation_points__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void browse_treatment_continuation_points__continuation_points_UNINITIALISATION(void);
extern void browse_treatment_continuation_points__create_continuation_point(
   const constants__t_session_i browse_treatment_continuation_points__p_session,
   const t_entier4 browse_treatment_continuation_points__p_nextIndex,
   const t_entier4 browse_treatment_continuation_points__p_maxTargetRef,
   const constants__t_NodeId_i browse_treatment_continuation_points__p_browseView,
   const constants__t_NodeId_i browse_treatment_continuation_points__p_nodeId,
   const constants__t_BrowseDirection_i browse_treatment_continuation_points__p_browseDirection,
   const constants__t_NodeId_i browse_treatment_continuation_points__p_referenceType,
   const t_bool browse_treatment_continuation_points__p_includeSubtypes,
   const constants__t_BrowseNodeClassMask_i browse_treatment_continuation_points__p_nodeClassMask,
   t_bool * const browse_treatment_continuation_points__bres,
   constants__t_ContinuationPoint_i * const browse_treatment_continuation_points__p_ContinuationPoint);
extern void browse_treatment_continuation_points__getall_and_clear_continuation_point(
   const constants__t_session_i browse_treatment_continuation_points__p_session,
   const constants__t_ContinuationPoint_i browse_treatment_continuation_points__p_continuationPoint,
   t_bool * const browse_treatment_continuation_points__bres,
   t_entier4 * const browse_treatment_continuation_points__p_nextIndex,
   t_entier4 * const browse_treatment_continuation_points__p_maxTargetRef,
   constants__t_NodeId_i * const browse_treatment_continuation_points__p_browseView,
   constants__t_NodeId_i * const browse_treatment_continuation_points__p_nodeId,
   constants__t_BrowseDirection_i * const browse_treatment_continuation_points__p_browseDirection,
   constants__t_NodeId_i * const browse_treatment_continuation_points__p_referenceType,
   t_bool * const browse_treatment_continuation_points__p_includeSubtypes,
   constants__t_BrowseNodeClassMask_i * const browse_treatment_continuation_points__p_nodeClassMask);
extern void browse_treatment_continuation_points__set_session_closed(
   const constants__t_session_i browse_treatment_continuation_points__p_session);

#endif
