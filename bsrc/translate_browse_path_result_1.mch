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
    translate_browse_path_result_1

SEES
    constants

DEFINITIONS
    d_variables ==
        BrowsePathResult,
        BrowsePathRemainingNodeId,
        BrowsePathRemainingIndex
        ;

    d_invariant ==
        BrowsePathResult : seq(t_ExpandedNodeId_i) &
        BrowsePathRemainingNodeId : seq(t_ExpandedNodeId_i) &
        BrowsePathRemainingIndex  : seq(NAT1) &

        size(BrowsePathResult) <= k_n_BrowsePathResPerElt_max &

        size(BrowsePathRemainingNodeId) <= k_n_BrowsePathResPerElt_max  &
        size(BrowsePathRemainingNodeId)  = size(BrowsePathRemainingIndex) &

        ran(BrowsePathResult) <: dom(conv_ExpandedNodeId_NodeId) &
        ran(BrowsePathRemainingNodeId) <: t_ExpandedNodeId

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_invariant

ASSERTIONS
    ran(BrowsePathResult) <: t_ExpandedNodeId

INITIALISATION
    BrowsePathResult := {} ||
    BrowsePathRemainingNodeId := {} ||
    BrowsePathRemainingIndex  := {}

OPERATIONS

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
        :(  res : BOOL &
            (res = TRUE =>
                size(BrowsePathResult) = k_n_BrowsePathResPerElt_max) &
            (res = FALSE =>
                size(BrowsePathResult) < k_n_BrowsePathResPerElt_max)
        )
    END
    ;

    nodeId <-- get_BrowsePathResult (index) =
    PRE
        index : NAT1 &
        index <= size(BrowsePathResult)
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
        :( BrowsePathResult = BrowsePathResult$0 <- nodeId )
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
        :(  res : BOOL &
            (res = TRUE =>
                size(BrowsePathRemainingNodeId) = k_n_BrowsePathResPerElt_max) &
            (res = FALSE =>
                size(BrowsePathRemainingNodeId) < k_n_BrowsePathResPerElt_max)
        )
    END
    ;

    nodeId, remainingIndex <-- get_BrowsePathRemaining (index) =
    PRE
        index : NAT1 &
        index <= size(BrowsePathRemainingNodeId)
    THEN
        nodeId := BrowsePathRemainingNodeId (index) ||
        remainingIndex := BrowsePathRemainingIndex(index)
    END
    ;

    add_BrowsePathResultRemaining (nodeId, path_index) =
    PRE
        nodeId : t_ExpandedNodeId_i &
        nodeId : t_ExpandedNodeId   &
        path_index  : NAT1 &
        size(BrowsePathRemainingNodeId) < k_n_BrowsePathResPerElt_max
    THEN
        BrowsePathRemainingNodeId,
        BrowsePathRemainingIndex
        :(  BrowsePathRemainingNodeId = BrowsePathRemainingNodeId$0 <- nodeId &
            BrowsePathRemainingIndex  = BrowsePathRemainingIndex$0 <- path_index
        )
    END

END
