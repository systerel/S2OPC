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
    monitored_item_event_filter_where_it

SEES
    constants

DEFINITIONS
    d_variables ==
    eventFilterWhereEltIdx_to_iterate,
    eventFilterWhereEltIdx_iterated,
    nb_eventFilterWhereElts
    ;

    d_inv ==
    (
    nb_eventFilterWhereElts : NAT &
    eventFilterWhereEltIdx_to_iterate <: NAT1 &
    eventFilterWhereEltIdx_iterated   <: NAT1 &
    eventFilterWhereEltIdx_to_iterate /\ eventFilterWhereEltIdx_iterated = {} &
    1..nb_eventFilterWhereElts = eventFilterWhereEltIdx_to_iterate \/ eventFilterWhereEltIdx_iterated
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_eventFilterWhereElt(p_nb_eventFilterWhereElts) =
    PRE
        p_nb_eventFilterWhereElts : NAT
    THEN
        eventFilterWhereEltIdx_iterated := {}        ||
        eventFilterWhereEltIdx_to_iterate := 1..p_nb_eventFilterWhereElts ||
        nb_eventFilterWhereElts := p_nb_eventFilterWhereElts ||
        p_continue := bool(p_nb_eventFilterWhereElts > 0)
    END
    ;

    p_continue, p_eventFilterWhereEltIdx <-- continue_iter_eventFilterWhereEltIdx =
    PRE
        eventFilterWhereEltIdx_to_iterate /= {}
    THEN
        ANY l_whereEltIdx WHERE
            l_whereEltIdx : NAT &
            l_whereEltIdx : eventFilterWhereEltIdx_to_iterate
        THEN
            eventFilterWhereEltIdx_iterated   := eventFilterWhereEltIdx_iterated   \/ {l_whereEltIdx} ||
            eventFilterWhereEltIdx_to_iterate := eventFilterWhereEltIdx_to_iterate -  {l_whereEltIdx} ||
            p_eventFilterWhereEltIdx          := l_whereEltIdx                                        ||
            p_continue                        := bool(eventFilterWhereEltIdx_to_iterate /= {l_whereEltIdx})
        END
    END

END
