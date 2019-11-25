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
    browse_treatment_result_it

SEES
    constants

DEFINITIONS
    d_variables ==
    browseResult_to_iterate,
    browseResult_iterated,
    nb_browseResult_max_refs,
    nextBrowseResult
    ;

    d_inv ==
    (
    nb_browseResult_max_refs : NAT &
    browseResult_to_iterate <: NAT1 &
    browseResult_iterated   <: NAT1 &
    browseResult_to_iterate /\ browseResult_iterated = {} &
    nb_browseResult_max_refs = card(browseResult_to_iterate \/ browseResult_iterated) &
    nextBrowseResult : NAT1
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_browseResult(p_nb_browseResult_max_refs) =
    PRE
        p_nb_browseResult_max_refs : NAT
    THEN
        browseResult_iterated    := {}        ||
        browseResult_to_iterate  := 1..p_nb_browseResult_max_refs ||
        nb_browseResult_max_refs := p_nb_browseResult_max_refs ||
        nextBrowseResult         := 1 ||
        p_continue               := bool(p_nb_browseResult_max_refs > 0)
    END
    ;

    p_continue, p_browseResultIdx <-- continue_iter_browseResult =
    PRE
        nb_browseResult_max_refs > 0 &
        nextBrowseResult <= nb_browseResult_max_refs
    THEN
        browseResult_iterated   := 1 .. nextBrowseResult ||
        browseResult_to_iterate := nextBrowseResult + 1 .. nb_browseResult_max_refs ||
        p_browseResultIdx       := nextBrowseResult ||
        nextBrowseResult        := nextBrowseResult + 1 ||
        p_continue              := bool(nextBrowseResult /= nb_browseResult_max_refs)
    END

END
