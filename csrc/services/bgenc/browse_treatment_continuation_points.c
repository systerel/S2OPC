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

 File Name            : browse_treatment_continuation_points.c

 Date                 : 05/04/2019 14:46:17

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "browse_treatment_continuation_points.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void browse_treatment_continuation_points__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment_continuation_points__create_continuation_point(
   const constants__t_session_i browse_treatment_continuation_points__p_session,
   const t_entier4 browse_treatment_continuation_points__p_nextIndex,
   const t_entier4 browse_treatment_continuation_points__p_maxTargetRef,
   const constants__t_NodeId_i browse_treatment_continuation_points__p_browseView,
   const constants__t_NodeId_i browse_treatment_continuation_points__p_nodeId,
   const constants__t_BrowseDirection_i browse_treatment_continuation_points__p_browseDirection,
   const constants__t_NodeId_i browse_treatment_continuation_points__p_referenceType,
   const t_bool browse_treatment_continuation_points__p_includeSubtypes,
   t_bool * const browse_treatment_continuation_points__bres,
   constants__t_ContinuationPoint_i * const browse_treatment_continuation_points__p_ContinuationPoint) {
   *browse_treatment_continuation_points__bres = false;
   *browse_treatment_continuation_points__p_ContinuationPoint = constants__c_ContinuationPoint_indet;
}

void browse_treatment_continuation_points__getall_and_clear_continuation_point(
   const constants__t_session_i browse_treatment_continuation_points__p_session,
   const constants__t_ContinuationPoint_i browse_treatment_continuation_points__p_continuationPoint,
   t_bool * const browse_treatment_continuation_points__bres,
   t_entier4 * const browse_treatment_continuation_points__p_nextIndex,
   t_entier4 * const browse_treatment_continuation_points__p_maxTargetRef,
   constants__t_NodeId_i * const browse_treatment_continuation_points__p_browseView,
   constants__t_NodeId_i * const browse_treatment_continuation_points__p_nodeId,
   constants__t_BrowseDirection_i * const browse_treatment_continuation_points__p_browseDirection,
   constants__t_NodeId_i * const browse_treatment_continuation_points__p_referenceType,
   t_bool * const browse_treatment_continuation_points__p_includeSubtypes) {
   *browse_treatment_continuation_points__bres = false;
   *browse_treatment_continuation_points__p_nextIndex = 0;
   *browse_treatment_continuation_points__p_maxTargetRef = 0;
   *browse_treatment_continuation_points__p_browseView = constants__c_NodeId_indet;
   *browse_treatment_continuation_points__p_nodeId = constants__c_NodeId_indet;
   *browse_treatment_continuation_points__p_browseDirection = constants__e_bd_indet;
   *browse_treatment_continuation_points__p_referenceType = constants__c_NodeId_indet;
   *browse_treatment_continuation_points__p_includeSubtypes = false;
}

void browse_treatment_continuation_points__set_session_closed(
   const constants__t_session_i browse_treatment_continuation_points__p_session) {
   ;
}

