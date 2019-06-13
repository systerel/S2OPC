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

MACHINE
    browse_treatment_continuation_points_bs

SEES
    constants

ABSTRACT_VARIABLES
    s_ContinuationPoints,
    id_ContinuationPoint

INVARIANT
    s_ContinuationPoints <: t_ContinuationPoint &
    id_ContinuationPoint : s_ContinuationPoints >-> t_ContinuationPointId

INITIALISATION
    s_ContinuationPoints :: {} ||
    id_ContinuationPoint :: {}

OPERATIONS
    bres, p_ContinuationPoint <-- create_continuation_point_bs (p_nextIndex, p_maxTargetRef, p_browseView,
                                                                p_nodeId, p_browseDirection, p_referenceType, p_includeSubtypes,
                                                                p_nodeClassMask, p_resultMask) =
    PRE
        p_nextIndex    : NAT                        &
        p_nextIndex    : t_RefIndex                 &
        p_maxTargetRef : NAT                        &
        p_maxTargetRef <= k_n_BrowseTarget_max      &
        p_browseView : t_NodeId_i                   &
        p_nodeId : t_NodeId_i                       &
        p_nodeId : t_NodeId                         &
        p_browseDirection : t_BrowseDirection_i     &
        p_browseDirection : t_BrowseDirection       &
        p_referenceType : t_NodeId_i                &
        p_referenceType : t_NodeId                  &
        p_includeSubtypes : BOOL                    &
        p_nodeClassMask : t_BrowseNodeClassMask_i   &
        p_resultMask : t_BrowseResultMask_i
    THEN
        /* Memory should allow to create a new continuation point */
        bres := bool(s_ContinuationPoints /= t_ContinuationPoint & t_ContinuationPointId /= ran(id_ContinuationPoint)) ||
        IF s_ContinuationPoints /= t_ContinuationPoint & t_ContinuationPointId /= ran(id_ContinuationPoint)
        THEN
            ANY l_ContinuationPoint, l_continuationPointId WHERE
                l_ContinuationPoint : t_ContinuationPoint - s_ContinuationPoints &
                l_continuationPointId : t_ContinuationPointId - ran(id_ContinuationPoint)
            THEN
                p_ContinuationPoint :(p_ContinuationPoint : t_ContinuationPoint_i & p_ContinuationPoint = l_ContinuationPoint) ||
                s_ContinuationPoints := s_ContinuationPoints \/ {l_ContinuationPoint} ||
                id_ContinuationPoint(l_ContinuationPoint) := l_continuationPointId
            END
        ELSE
            p_ContinuationPoint := c_ContinuationPoint_indet
        END
    END
    ;

    p_nextIndex, p_maxTargetRef, p_browseView, p_nodeId,
    p_browseDirection, p_referenceType, p_includeSubtypes, p_nodeClassMask, p_resultMask
      <-- getall_continuation_point_bs (p_ContinuationPoint) =
    PRE
        p_ContinuationPoint : t_ContinuationPoint_i &
        p_ContinuationPoint : s_ContinuationPoints
    THEN
        p_nextIndex, p_maxTargetRef, p_browseView, p_nodeId,
        p_browseDirection, p_referenceType, p_includeSubtypes,
        p_nodeClassMask, p_resultMask
        :(
            p_nextIndex    : NAT                        &
            p_nextIndex    : t_RefIndex                 &
            p_maxTargetRef : NAT                        &
            p_maxTargetRef <= k_n_BrowseTarget_max      &
            p_browseView : t_NodeId_i                   &
            p_nodeId : t_NodeId_i                       &
            p_browseDirection : t_BrowseDirection_i     &
            p_browseDirection : t_BrowseDirection       &
            p_referenceType : t_NodeId_i                &
            p_includeSubtypes : BOOL                    &
            p_nodeClassMask : t_BrowseNodeClassMask_i   &
            p_resultMask    : t_BrowseResultMask_i
         )
    END
    ;

    p_continuationPointId <-- get_continuation_point_id(p_continuationPoint) =
    PRE
        p_continuationPoint : t_ContinuationPoint_i &
        p_continuationPoint : s_ContinuationPoints
    THEN
        p_continuationPointId
        :(p_continuationPointId : t_ContinuationPointId_i &
          p_continuationPointId = id_ContinuationPoint(p_continuationPoint))
    END
    ;

    clear_continuation_point(p_continuationPoint) =
    PRE
        p_continuationPoint : t_ContinuationPoint_i
    THEN
        IF p_continuationPoint : s_ContinuationPoints THEN
            s_ContinuationPoints, id_ContinuationPoint
            :(
                s_ContinuationPoints = s_ContinuationPoints$0 - {p_continuationPoint} &
                id_ContinuationPoint = id_ContinuationPoint$0 - {p_continuationPoint |-> id_ContinuationPoint$0(p_continuationPoint)}
            )
        END
    END

END
