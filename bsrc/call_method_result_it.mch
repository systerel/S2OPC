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
    call_method_result_it

SEES
    constants

DEFINITIONS
    d_variables ==
    callMethodResultIdx_to_iterate,
    callMethodResultIdx_iterated,
    nb_callMethodResultIdx_max_refs,
    nextCallMethodResultIdx
    ;

    d_inv ==
    (
    nb_callMethodResultIdx_max_refs : NAT &
    callMethodResultIdx_to_iterate <: NAT1 &
    callMethodResultIdx_iterated   <: NAT1 &
    callMethodResultIdx_to_iterate /\ callMethodResultIdx_iterated = {} &
    nb_callMethodResultIdx_max_refs = card(callMethodResultIdx_to_iterate \/ callMethodResultIdx_iterated) &
    nextCallMethodResultIdx : NAT1
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_callMethodResultIdx(p_nb_callMethodResultIdx_max_refs) =
    PRE
        p_nb_callMethodResultIdx_max_refs : NAT
    THEN
        callMethodResultIdx_iterated    := {}        ||
        callMethodResultIdx_to_iterate  := 1..p_nb_callMethodResultIdx_max_refs ||
        nb_callMethodResultIdx_max_refs := p_nb_callMethodResultIdx_max_refs ||
        nextCallMethodResultIdx         := 1 ||
        p_continue               := bool(p_nb_callMethodResultIdx_max_refs > 0)
    END
    ;

    p_continue, p_callMethodResultIdx <-- continue_iter_callMethodResultIdx =
    PRE
        nb_callMethodResultIdx_max_refs > 0 &
        nextCallMethodResultIdx <= nb_callMethodResultIdx_max_refs
    THEN
        callMethodResultIdx_iterated   := 1 .. nextCallMethodResultIdx ||
        callMethodResultIdx_to_iterate := nextCallMethodResultIdx + 1 .. nb_callMethodResultIdx_max_refs ||
        p_callMethodResultIdx          := nextCallMethodResultIdx ||
        nextCallMethodResultIdx        := nextCallMethodResultIdx + 1 ||
        p_continue                     := bool(nextCallMethodResultIdx /= nb_callMethodResultIdx_max_refs)
    END

END
