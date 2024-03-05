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
    monitored_item_event_filter_select_it

SEES
    constants

DEFINITIONS
    d_variables ==
    eventFilterSelectClauseIdx_to_iterate,
    eventFilterSelectClauseIdx_iterated,
    nb_eventFilterSelectClauses
    ;

    d_inv ==
    (
    nb_eventFilterSelectClauses : NAT &
    eventFilterSelectClauseIdx_to_iterate <: NAT1 &
    eventFilterSelectClauseIdx_iterated   <: NAT1 &
    eventFilterSelectClauseIdx_to_iterate /\ eventFilterSelectClauseIdx_iterated = {} &
    1..nb_eventFilterSelectClauses = eventFilterSelectClauseIdx_to_iterate \/ eventFilterSelectClauseIdx_iterated
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_eventFilterSelectClause(p_nb_eventFilterSelectClauses) =
    PRE
        p_nb_eventFilterSelectClauses : NAT
    THEN
        eventFilterSelectClauseIdx_iterated := {}        ||
        eventFilterSelectClauseIdx_to_iterate := 1..p_nb_eventFilterSelectClauses ||
        nb_eventFilterSelectClauses := p_nb_eventFilterSelectClauses ||
        p_continue := bool(p_nb_eventFilterSelectClauses > 0)
    END
    ;

    p_continue, p_eventFilterSelectClauseIdx <-- continue_iter_eventFilterSelectClauseIdx =
    PRE
        eventFilterSelectClauseIdx_to_iterate /= {}
    THEN
        ANY l_selectClauseIdx WHERE
            l_selectClauseIdx : NAT &
            l_selectClauseIdx : eventFilterSelectClauseIdx_to_iterate
        THEN
            eventFilterSelectClauseIdx_iterated   := eventFilterSelectClauseIdx_iterated   \/ {l_selectClauseIdx} ||
            eventFilterSelectClauseIdx_to_iterate := eventFilterSelectClauseIdx_to_iterate -  {l_selectClauseIdx} ||
            p_eventFilterSelectClauseIdx          := l_selectClauseIdx                                            ||
            p_continue                            := bool(eventFilterSelectClauseIdx_to_iterate /= {l_selectClauseIdx})
        END
    END

END
