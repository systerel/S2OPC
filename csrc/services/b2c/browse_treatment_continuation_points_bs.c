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

#include "browse_treatment_continuation_points_bs.h"

void browse_treatment_continuation_points_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment_continuation_points_bs__create_continuation_point_bs(
    const t_entier4 browse_treatment_continuation_points_bs__p_nextIndex,
    const t_entier4 browse_treatment_continuation_points_bs__p_maxTargetRef,
    const constants__t_NodeId_i browse_treatment_continuation_points_bs__p_browseView,
    const constants__t_NodeId_i browse_treatment_continuation_points_bs__p_nodeId,
    const constants__t_BrowseDirection_i browse_treatment_continuation_points_bs__p_browseDirection,
    const constants__t_NodeId_i browse_treatment_continuation_points_bs__p_referenceType,
    const t_bool browse_treatment_continuation_points_bs__p_includeSubtypes,
    const constants__t_BrowseNodeClassMask_i browse_treatment_continuation_points_bs__p_nodeClassMask,
    const constants__t_BrowseResultMask_i browse_treatment_continuation_points_bs__p_resultMask,
    t_bool* const browse_treatment_continuation_points_bs__bres,
    constants__t_ContinuationPoint_i* const browse_treatment_continuation_points_bs__p_ContinuationPoint)
{
    (void) browse_treatment_continuation_points_bs__p_nextIndex;
    (void) browse_treatment_continuation_points_bs__p_maxTargetRef;
    (void) browse_treatment_continuation_points_bs__p_browseView;
    (void) browse_treatment_continuation_points_bs__p_nodeId;
    (void) browse_treatment_continuation_points_bs__p_browseDirection;
    (void) browse_treatment_continuation_points_bs__p_referenceType;
    (void) browse_treatment_continuation_points_bs__p_includeSubtypes;
    (void) browse_treatment_continuation_points_bs__p_nodeClassMask;
    (void) browse_treatment_continuation_points_bs__p_resultMask;
    *browse_treatment_continuation_points_bs__bres = false;
    *browse_treatment_continuation_points_bs__p_ContinuationPoint = NULL;
}
void browse_treatment_continuation_points_bs__unused_continuationPoint(
    const constants__t_ContinuationPoint_i browse_treatment_continuation_points_bs__p_continuationPoint)
{
    (void) browse_treatment_continuation_points_bs__p_continuationPoint;
}
