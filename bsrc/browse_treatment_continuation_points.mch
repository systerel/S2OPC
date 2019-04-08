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
    session_ContinuationPoint

INVARIANT
    s_ContinuationPoints <: t_ContinuationPoint &
    session_ContinuationPoint : t_ContinuationPoint >+> t_session // a continuation point could be associated to a session (=> 1 CP / session)

INITIALISATION
    s_ContinuationPoints :: {} ||
    session_ContinuationPoint :: {}

OPERATIONS

    bres, p_ContinuationPoint <-- create_continuation_point (p_session, p_nextIndex, p_maxTargetRef, p_browseView,
                                                              p_nodeId, p_browseDirection, p_referenceType, p_includeSubtypes) =
    PRE
        p_session : t_session_i                     &
        p_session : t_session                       &
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
        p_includeSubtypes : BOOL
    THEN
        /* No continuation point shall be define for given session to create one */
        bres := bool(p_session /: dom(session_ContinuationPoint~)) ||
        IF p_session /: dom(session_ContinuationPoint~)
        THEN
            ANY l_ContinuationPoint WHERE
                l_ContinuationPoint : t_ContinuationPoint - s_ContinuationPoints
            THEN
                p_ContinuationPoint :(p_ContinuationPoint : t_ContinuationPoint_i & p_ContinuationPoint = l_ContinuationPoint) ||
                s_ContinuationPoints := s_ContinuationPoints \/ {l_ContinuationPoint} ||
                session_ContinuationPoint := session_ContinuationPoint \/ {l_ContinuationPoint |-> p_session}
            END
        ELSE
            p_ContinuationPoint := c_ContinuationPoint_indet
        END
    END
    ;

    bres, p_nextIndex, p_maxTargetRef, p_browseView, p_nodeId,
    p_browseDirection, p_referenceType, p_includeSubtypes
      <-- getall_and_clear_continuation_point (p_session, p_continuationPoint) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_continuationPoint : t_ContinuationPoint_i &
        p_continuationPoint : t_ContinuationPoint
    THEN
        bres, s_ContinuationPoints, session_ContinuationPoint, p_nextIndex, p_maxTargetRef, p_browseView, p_nodeId, p_browseDirection, p_referenceType, p_includeSubtypes
        :(
            bres : BOOL &
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

            bres = bool(session_ContinuationPoint(p_continuationPoint) = p_session) &
            /* Clear the continuation point once used to free session continuation point for BrowseNext treatment */
            (bres = TRUE =>
                s_ContinuationPoints = s_ContinuationPoints$0 - {p_continuationPoint} &
                session_ContinuationPoint = {p_continuationPoint} <<| session_ContinuationPoint$0
            )
        )
    END
    ;

    set_session_closed(p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        /* De-associate and clear the continuation point associated with session */
        s_ContinuationPoints := s_ContinuationPoints - {session_ContinuationPoint~(p_session)} ||
        session_ContinuationPoint := session_ContinuationPoint |>> {p_session}
    END
    ;

    /* TODO: to be called on UNINIT to free all continuations AND on NodeManagement service calls modifying the address space !!! */
    UNINITIALISATION =
    BEGIN
        s_ContinuationPoints :: {} ||
        session_ContinuationPoint :: {}
    END

END
