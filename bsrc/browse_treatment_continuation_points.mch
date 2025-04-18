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
    browse_treatment_continuation_points
SEES
    constants

ABSTRACT_VARIABLES
    s_ContinuationPoints,
    id_ContinuationPoint,
    ContinuationPoint_session

INVARIANT
    s_ContinuationPoints <: t_ContinuationPoint &
    id_ContinuationPoint : s_ContinuationPoints >-> t_ContinuationPointId & // Each continuation has a unique Id
    ContinuationPoint_session : s_ContinuationPoints >-> t_session // a continuation point is associated to a session (=> 1 CP / session)

INITIALISATION
    s_ContinuationPoints := {} ||
    id_ContinuationPoint := {} ||
    ContinuationPoint_session := {}

OPERATIONS

    bres, p_ContinuationPointId <-- has_continuation_point(p_session) =
    PRE
        p_session : t_session_i
    THEN
        bres, p_ContinuationPointId
        :(bres : BOOL &
          p_ContinuationPointId : t_ContinuationPointId_i &
          bres = bool(p_session : t_session & p_session : ran(ContinuationPoint_session)) &
          (bres = TRUE =>
              p_ContinuationPointId = id_ContinuationPoint(ContinuationPoint_session~(p_session))) &
          (bres = FALSE =>
              p_ContinuationPointId = c_ContinuationPointId_indet)
         )
    END
    ;

    bres, p_ContinuationPointId <-- create_continuation_point (p_session, p_nextIndex, p_maxTargetRef, p_browseView,
                                                               p_nodeId, p_browseDirection, p_referenceType,
                                                               p_includeSubtypes, p_nodeClassMask, p_resultMask) =
    PRE
        p_session : t_session_i                     &
        p_nextIndex    : NAT                        &
        p_nextIndex    : t_RefIndex                 &
        p_maxTargetRef : NAT                        &
        p_browseView : t_NodeId_i                   &
        p_nodeId : t_NodeId_i                       &
        p_nodeId : t_NodeId                         &
        p_browseDirection : t_BrowseDirection_i     &
        p_browseDirection : t_BrowseDirection       &
        p_referenceType : t_NodeId_i                &
        p_includeSubtypes : BOOL                    &
        p_nodeClassMask : t_BrowseNodeClassMask_i   &
        p_resultMask    : t_BrowseResultMask_i
    THEN
        /* No continuation point shall be defined for given session to create a new one */
        IF p_session : t_session &  p_session /: ran(ContinuationPoint_session)
        THEN
            /* Memory check */
            IF s_ContinuationPoints /= t_ContinuationPoint &
               t_ContinuationPointId /= ran(id_ContinuationPoint)
            THEN
                bres := TRUE ||
                ANY l_ContinuationPoint, l_continuationPointId WHERE
                    l_ContinuationPoint : t_ContinuationPoint - s_ContinuationPoints &
                    l_continuationPointId : t_ContinuationPointId - ran(id_ContinuationPoint)
                THEN
                    p_ContinuationPointId :(p_ContinuationPointId : t_ContinuationPointId_i & p_ContinuationPointId = l_continuationPointId) ||
                    s_ContinuationPoints := s_ContinuationPoints \/ {l_ContinuationPoint} ||
                    id_ContinuationPoint(l_ContinuationPoint) := l_continuationPointId ||
                    ContinuationPoint_session := ContinuationPoint_session \/ {l_ContinuationPoint |-> p_session}
                END
            ELSE
                bres := FALSE ||
                p_ContinuationPointId := c_ContinuationPointId_indet
            END
        ELSE
            bres := FALSE ||
            p_ContinuationPointId := c_ContinuationPointId_indet
        END
    END
    ;

    bres, p_nextIndex, p_maxTargetRef, p_browseView, p_nodeId,
    p_browseDirection, p_referenceType, p_includeSubtypes, p_nodeClassMask, p_resultMask
      <-- getall_continuation_point (p_session, p_continuationPointId) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_continuationPointId : t_ContinuationPointId_i
    THEN
        bres, p_nextIndex, p_maxTargetRef, p_browseView, p_nodeId,
        p_browseDirection, p_referenceType, p_includeSubtypes,
        p_nodeClassMask, p_resultMask
        :(
            bres : BOOL &
            p_nextIndex    : NAT                        &
            p_nextIndex    : t_RefIndex                 &
            p_maxTargetRef : NAT                        &
            p_browseView : t_NodeId_i                   &
            p_nodeId : t_NodeId_i                       &
            p_browseDirection : t_BrowseDirection_i     &
            p_referenceType : t_NodeId_i                &
            p_includeSubtypes : BOOL                    &
            p_nodeClassMask : t_BrowseNodeClassMask_i   &
            p_resultMask    : t_BrowseResultMask_i &

            bres = bool(p_session : ran(ContinuationPoint_session) &
                        p_continuationPointId =
                            id_ContinuationPoint(ContinuationPoint_session~(p_session))) &
            /* Clear the continuation point once used to free session continuation point for BrowseNext treatment */
            (bres = TRUE =>
                p_nodeId : t_NodeId &
                p_browseDirection : t_BrowseDirection
            )
        )
    END
    ;

    bres <-- release_continuation_point (p_session, p_continuationPointId) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_continuationPointId : t_ContinuationPointId_i
    THEN
        s_ContinuationPoints, id_ContinuationPoint, ContinuationPoint_session, bres
        :(
            bres = bool(p_session: ran(ContinuationPoint_session$0) &
                        p_continuationPointId =
                            id_ContinuationPoint(ContinuationPoint_session~(p_session))) &
            /* Release the continuation point  */
            (bres = TRUE =>
                s_ContinuationPoints = s_ContinuationPoints$0 - {ContinuationPoint_session$0~(p_session)} &
                id_ContinuationPoint = {ContinuationPoint_session$0~(p_session)} <<| id_ContinuationPoint$0 &
                ContinuationPoint_session = ContinuationPoint_session$0 |>> {p_session}) &
            (bres = FALSE =>
                s_ContinuationPoints = s_ContinuationPoints$0 &
                id_ContinuationPoint = id_ContinuationPoint$0 &
                ContinuationPoint_session = ContinuationPoint_session$0)
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

    set_session_closed(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        /* De-associate and clear the continuation point associated with session */
        ContinuationPoint_session := ContinuationPoint_session |>> {p_session} ||
        IF p_session : ran(ContinuationPoint_session) THEN
            s_ContinuationPoints := s_ContinuationPoints - {ContinuationPoint_session~(p_session)} ||
            id_ContinuationPoint := {ContinuationPoint_session~(p_session)} <<| id_ContinuationPoint
        END
    END
    ;

    continuation_points_UNINITIALISATION =
    BEGIN
        s_ContinuationPoints := {} ||
        id_ContinuationPoint := {} ||
        ContinuationPoint_session := {}
    END

END
