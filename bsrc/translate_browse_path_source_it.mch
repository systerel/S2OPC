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
    translate_browse_path_source_it

SEES
    constants

DEFINITIONS
    d_variables ==
    browsePathSourceIdx_to_iterate,
    browsePathSourceIdx_iterated,
    nb_browsePathSourceIdx_max_refs,
    nextBrowsePathSourceIdx
    ;

    d_inv ==
    (
    nb_browsePathSourceIdx_max_refs : NAT &
    browsePathSourceIdx_to_iterate <: NAT1 &
    browsePathSourceIdx_iterated   <: NAT1 &
    browsePathSourceIdx_to_iterate /\ browsePathSourceIdx_iterated = {} &
    nb_browsePathSourceIdx_max_refs = card(browsePathSourceIdx_to_iterate \/ browsePathSourceIdx_iterated) &
    nextBrowsePathSourceIdx : NAT1
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_browsePathSourceIdx(p_nb_browsePathSourceIdx_max_refs) =
    PRE
        p_nb_browsePathSourceIdx_max_refs : NAT
    THEN
        browsePathSourceIdx_iterated    := {}        ||
        browsePathSourceIdx_to_iterate  := 1..p_nb_browsePathSourceIdx_max_refs ||
        nb_browsePathSourceIdx_max_refs := p_nb_browsePathSourceIdx_max_refs ||
        nextBrowsePathSourceIdx         := 1 ||
        p_continue               := bool(p_nb_browsePathSourceIdx_max_refs > 0)
    END
    ;

    p_continue, p_browsePathSourceIdx <-- continue_iter_browsePathSourceIdx =
    PRE
        nb_browsePathSourceIdx_max_refs > 0 &
        nextBrowsePathSourceIdx <= nb_browsePathSourceIdx_max_refs
    THEN
        browsePathSourceIdx_iterated   := 1 .. nextBrowsePathSourceIdx ||
        browsePathSourceIdx_to_iterate := nextBrowsePathSourceIdx + 1 .. nb_browsePathSourceIdx_max_refs ||
        p_browsePathSourceIdx          := nextBrowsePathSourceIdx ||
        nextBrowsePathSourceIdx        := nextBrowsePathSourceIdx + 1 ||
        p_continue                     := bool(nextBrowsePathSourceIdx /= nb_browsePathSourceIdx_max_refs)
    END

END
