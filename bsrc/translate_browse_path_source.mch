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
    translate_browse_path_source

SEES
    constants,
    constants_statuscodes_bs

ABSTRACT_VARIABLES
    BrowsePathSource

INVARIANT
    BrowsePathSource : seq(t_NodeId_i) &
    size(BrowsePathSource) <= k_n_BrowsePathResPerElt_max

INITIALISATION
    BrowsePathSource := {}

OPERATIONS

    statusCode_operation <-- update_one_browse_path_source(source) =
    PRE
        source : t_NodeId_i &
        source : t_NodeId
    THEN
        statusCode_operation,
        BrowsePathSource
        :(  statusCode_operation : t_StatusCode_i &
            statusCode_operation : t_StatusCode   &
            statusCode_operation : {e_sc_ok, e_sc_bad_out_of_memory} &

            BrowsePathSource : seq(t_NodeId_i) &
            (statusCode_operation = e_sc_ok =>
                BrowsePathSource = BrowsePathSource$0 <- source
            ) &
            (statusCode_operation /= e_sc_ok =>
                BrowsePathSource = BrowsePathSource$0
            )
        )
    END
    ;

    free_BrowsePathSource =
    BEGIN
        BrowsePathSource := {}
    END
    ;

    /* Promoted from translate_browse_path_source_1 */
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

END
