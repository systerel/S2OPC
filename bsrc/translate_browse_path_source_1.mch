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
    translate_browse_path_source_1

SEES
    constants

ABSTRACT_VARIABLES
    BrowsePathSource

INVARIANT
    BrowsePathSource : seq(t_NodeId_i) &
    size(BrowsePathSource) <= k_n_BrowsePathResPerElt_max

INITIALISATION
    BrowsePathSource := {}

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
        index : NAT1 &
        index <= size(BrowsePathSource)
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

END
