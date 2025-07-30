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
    history_read_it

SEES
    constants

DEFINITIONS
    d_variables ==
        rreqs_to_iterate,
        rreqs_iterated
    ;

    d_inv ==
    (
        rreqs_to_iterate <: t_historyReadReqIndex &
        rreqs_iterated <: t_historyReadReqIndex &
        rreqs_to_iterate /\ rreqs_iterated = {}
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_hist_read_request(p_nb_req) =
    PRE
        p_nb_req : NAT &
        p_nb_req <= k_n_histRead_max
    THEN
        rreqs_iterated := {} ||
        rreqs_to_iterate := 1..p_nb_req ||
        p_continue := bool(0 < p_nb_req)
    END
    ;

    p_continue, p_index <-- continue_iter_hist_read_request =
    PRE
        rreqs_to_iterate /= {}
    THEN
        ANY l_index WHERE
            l_index : NAT &
            l_index : t_historyReadReqIndex &
            l_index : rreqs_to_iterate
        THEN
            rreqs_iterated := rreqs_iterated \/ {l_index} ||
            rreqs_to_iterate := rreqs_to_iterate - {l_index} ||
            p_index := l_index ||
            p_continue := bool(rreqs_to_iterate /= {l_index})
        END
    END

END
