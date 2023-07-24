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
    translate_browse_path_result_1_it

SEES
    constants

DEFINITIONS
    d_variables ==
    browsePathIdx_to_iterate,
    browsePathIdx_iterated,
    nb_browsePathIdx_max_refs,
    nextBrowsePathIdx
    ;

    d_inv ==
    (
    nb_browsePathIdx_max_refs : NAT &
    browsePathIdx_to_iterate <: NAT1 &
    browsePathIdx_iterated   <: NAT1 &
    browsePathIdx_to_iterate /\ browsePathIdx_iterated = {} &
    nb_browsePathIdx_max_refs = card(browsePathIdx_to_iterate \/ browsePathIdx_iterated) &
    nextBrowsePathIdx : NATURAL1
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_browsePathIdx(p_nb_browsePathIdx_max_refs) =
    PRE
        p_nb_browsePathIdx_max_refs : NAT
    THEN
        browsePathIdx_iterated    := {}        ||
        browsePathIdx_to_iterate  := 1..p_nb_browsePathIdx_max_refs ||
        nb_browsePathIdx_max_refs := p_nb_browsePathIdx_max_refs ||
        nextBrowsePathIdx         := 1 ||
        p_continue               := bool(p_nb_browsePathIdx_max_refs > 0)
    END
    ;

    p_continue, p_browsePathIdx <-- continue_iter_browsePathIdx =
    PRE
        nextBrowsePathIdx <= nb_browsePathIdx_max_refs
    THEN
        browsePathIdx_iterated   := 1 .. nextBrowsePathIdx ||
        browsePathIdx_to_iterate := nextBrowsePathIdx + 1 .. nb_browsePathIdx_max_refs ||
        p_browsePathIdx          :(  p_browsePathIdx : NAT1 &
                                           p_browsePathIdx = nextBrowsePathIdx) ||
        nextBrowsePathIdx        := nextBrowsePathIdx + 1 ||
        p_continue                     := bool(nextBrowsePathIdx < nb_browsePathIdx_max_refs)
    END

END
