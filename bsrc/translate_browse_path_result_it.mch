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
    translate_browse_path_result_it

SEES
    constants

DEFINITIONS
    d_variables ==
        translate_browseResult_to_iterate,
        translate_browseResult_iterated,
        nb_translate_browseResult_max_refs
    ;

    d_inv ==
    (
    nb_translate_browseResult_max_refs : NAT &
    translate_browseResult_to_iterate <: NAT1 &
    translate_browseResult_iterated   <: NAT1 &
    translate_browseResult_to_iterate /\ translate_browseResult_iterated = {} &
    nb_translate_browseResult_max_refs = card(translate_browseResult_to_iterate \/ translate_browseResult_iterated)
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_translate_browseResult(p_nb_browseResult_max_refs) =
    PRE
        p_nb_browseResult_max_refs : NAT
    THEN
        translate_browseResult_iterated    := {}        ||
        translate_browseResult_to_iterate  := 1..p_nb_browseResult_max_refs ||
        nb_translate_browseResult_max_refs := p_nb_browseResult_max_refs ||
        p_continue               := bool(p_nb_browseResult_max_refs > 0)
    END
    ;

    p_continue, p_browseResultIdx <-- continue_iter_translate_browseResult =
    PRE
        translate_browseResult_to_iterate /= {}
    THEN
        ANY l_browseResult WHERE
            l_browseResult : NAT &
            l_browseResult : translate_browseResult_to_iterate
        THEN
            translate_browseResult_iterated   := translate_browseResult_iterated   \/ {l_browseResult} ||
            translate_browseResult_to_iterate := translate_browseResult_to_iterate -  {l_browseResult} ||
            p_browseResultIdx        := l_browseResult                               ||
            p_continue               := bool(translate_browseResult_to_iterate /= {l_browseResult})
        END
    END

END
