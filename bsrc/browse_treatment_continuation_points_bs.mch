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

OPERATIONS
    bres, p_ContinuationPoint <-- create_continuation_point_bs (p_nextIndex, p_maxTargetRef, p_browseView,
                                                                p_nodeId, p_browseDirection, p_referenceType, p_includeSubtypes) =
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
        p_includeSubtypes : BOOL
    THEN
        bres :: BOOL ||
        p_ContinuationPoint :: t_ContinuationPoint_i
    END
    ;

    unused_continuationPoint(p_continuationPoint) =
    PRE
        p_continuationPoint : t_ContinuationPoint_i &
        p_continuationPoint : t_ContinuationPoint
    THEN
        skip
    END

END
