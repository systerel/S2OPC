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
    translate_browse_path_1

SEES
    constants

DEFINITIONS
    d_variables ==
    BrowsePathSource,
    BrowsePathResult,
    BrowsePathRemainingNodeId,
    BrowsePathRemainingIndex
    ;

    d_invariant ==
    BrowsePathSource : seq(t_NodeId_i) &
    BrowsePathResult : seq(t_ExpandedNodeId_i) &
    BrowsePathRemainingNodeId : seq(t_ExpandedNodeId_i) &
    BrowsePathRemainingIndex  : seq(NAT1) &

    size(BrowsePathSource) <= k_n_BrowsePathResPerElt_max &
    size(BrowsePathResult) <= k_n_BrowsePathResPerElt_max &

    size(BrowsePathRemainingNodeId) <= k_n_BrowsePathResPerElt_max  &
    size(BrowsePathRemainingIndex)  <= k_n_BrowsePathResPerElt_max  &
    size(BrowsePathRemainingNodeId)  = size(BrowsePathRemainingIndex) &

    !(node).(
        node : ran(BrowsePathResult)
        =>
        node /= c_ExpandedNodeId_indet &
        node : dom(conv_ExpandedNodeId_NodeId))

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_invariant

INITIALISATION
    BrowsePathSource := {} ||
    BrowsePathResult := {} ||
    BrowsePathRemainingNodeId := {} ||
    BrowsePathRemainingIndex  := {}

OPERATIONS

    init_BrowsePathSource =
    BEGIN
        BrowsePathSource := {}
    END
    ;

    res <-- get_BrowsePathSourceSize =
    BEGIN
        res := size(BrowsePathSource)
    END
    ;

    nodeId <-- get_BrowsePathSource (index) =
    PRE
        index : NAT1
    THEN
        nodeId := BrowsePathSource (index)
    END
    ;

    add_BrowsePathSource (nodeId) =
    PRE
        nodeId : t_NodeId_i &
        nodeId : t_NodeId &
        size(BrowsePathSource) < k_n_BrowsePathResPerElt_max
    THEN
        BrowsePathSource
        :(
        BrowsePathSource = BrowsePathSource$0 <- nodeId
        )
    END
    ;

    init_BrowsePathResult =
    BEGIN
        BrowsePathResult := {}
    END
    ;

    res <-- get_BrowsePathResultSize =
    BEGIN
        res := size(BrowsePathResult)
    END
    ;

    res <-- get_BrowsePathResult_IsFull =
    BEGIN
        res
        :(
            res : BOOL &
            (res = TRUE => size(BrowsePathResult) = k_n_BrowsePathResPerElt_max) &
             (res = FALSE => size(BrowsePathResult) < k_n_BrowsePathResPerElt_max)
         )
    END
    ;

    nodeId <-- get_BrowsePathResult (index) =
    PRE
        index : NAT1
    THEN
        nodeId := BrowsePathResult (index)
    END
    ;

    add_BrowsePathResult (nodeId) =
    PRE
        nodeId : t_ExpandedNodeId_i &
        nodeId : t_ExpandedNodeId &
        nodeId : dom(conv_ExpandedNodeId_NodeId) &
        size(BrowsePathResult) < k_n_BrowsePathResPerElt_max
    THEN
        BrowsePathResult
        :(
        BrowsePathResult = BrowsePathResult$0 <- nodeId
        )
    END
    ;

    init_BrowsePathRemaining =
    BEGIN
        BrowsePathRemainingNodeId := {} ||
        BrowsePathRemainingIndex  := {}
    END
    ;

    res <-- get_BrowsePathRemainingSize =
    BEGIN
        res := size(BrowsePathRemainingNodeId)
    END
    ;

    res <-- get_BrowsePathRemaining_IsFull =
    BEGIN
        res
        :(
            res : BOOL &
            (res = TRUE => size(BrowsePathRemainingNodeId) = k_n_BrowsePathResPerElt_max) &
             (res = FALSE => size(BrowsePathRemainingNodeId) < k_n_BrowsePathResPerElt_max)
         )
    END
    ;

    nodeId, remainingIndex <-- get_BrowsePathRemaining (index) =
    PRE
        index : NAT1
    THEN
        nodeId := BrowsePathRemainingNodeId (index) ||
        remainingIndex := BrowsePathRemainingIndex(index)
    END
    ;

    add_BrowsePathResultRemaining (nodeId, index) =
    PRE
        nodeId : t_ExpandedNodeId_i &
        nodeId : t_ExpandedNodeId   &
        index  : NAT1 &
        size(BrowsePathRemainingNodeId) < k_n_BrowsePathResPerElt_max
    THEN
        BrowsePathRemainingNodeId,
        BrowsePathRemainingIndex
        :(
        BrowsePathRemainingNodeId = BrowsePathRemainingNodeId$0 <- nodeId &
        BrowsePathRemainingIndex  = BrowsePathRemainingIndex$0 <- index
        )
    END

END