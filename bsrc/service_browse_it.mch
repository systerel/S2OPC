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
    service_browse_it

SEES
    constants

DEFINITIONS
    d_variables ==
        breqs_nb_reqs,
        breqs_to_iterate,
        breqs_iterated
    ;

    d_inv ==
    (
        breqs_nb_reqs : NAT &
        breqs_nb_reqs <= k_n_BrowseResponse_max &

        breqs_to_iterate <: t_BrowseValue &
        breqs_iterated <: t_BrowseValue &
        breqs_to_iterate /\ breqs_iterated = {} &
        breqs_nb_reqs = card(breqs_to_iterate \/ breqs_iterated)
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_browse_request(p_nb_req) =
    PRE
        p_nb_req : NAT &
        p_nb_req <= k_n_BrowseResponse_max
    THEN
        breqs_nb_reqs    := p_nb_req                        ||
        breqs_iterated   := {}                              ||
        breqs_to_iterate := cast_t_BrowseValue[1..p_nb_req] ||
        p_continue       := bool(0 < p_nb_req)
    END
    ;

    p_continue, p_bvi <-- continue_iter_browse_request =
    PRE
        breqs_to_iterate /= {}
    THEN
        ANY l_bvi WHERE
            l_bvi : t_BrowseValue_i &
            l_bvi : t_BrowseValue &
            l_bvi : breqs_to_iterate
        THEN
            breqs_iterated   := breqs_iterated \/ {l_bvi} ||
            breqs_to_iterate := breqs_to_iterate - {l_bvi} ||
            p_bvi            := l_bvi ||
            p_continue       := bool(breqs_to_iterate /= {l_bvi})
        END
    END

END
