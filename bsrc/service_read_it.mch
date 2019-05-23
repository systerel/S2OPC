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
    service_read_it

SEES
    constants

DEFINITIONS
    d_variables ==
        rreqs_nb_reqs,
        rreqs_to_iterate,
        rreqs_iterated
    ;

    d_inv ==
    (
        rreqs_nb_reqs : NAT &
        rreqs_nb_reqs <= k_n_read_resp_max &

        rreqs_to_iterate <: t_ReadValue &
        rreqs_iterated <: t_ReadValue &
        rreqs_to_iterate /\ rreqs_iterated = {}
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS
    continue <-- init_iter_read_request(nb_req) =
    PRE
        nb_req : NAT &
        nb_req <= k_n_read_resp_max
    THEN
        rreqs_nb_reqs := nb_req ||
        rreqs_iterated := {} ||
        rreqs_to_iterate := cast_t_ReadValue[1..nb_req] ||
        continue := bool(0 < nb_req)
    END
    ;

    continue, rvi <-- continue_iter_read_request =
    PRE
        rreqs_to_iterate /= {}
    THEN
        ANY l_rvi WHERE
            l_rvi : t_ReadValue_i &
            l_rvi : t_ReadValue &
            l_rvi : rreqs_to_iterate
        THEN
            rreqs_iterated := rreqs_iterated \/ {l_rvi} ||
            rreqs_to_iterate := rreqs_to_iterate - {l_rvi} ||
            rvi := l_rvi ||
            continue := bool(rreqs_to_iterate /= {l_rvi})
        END
    END

END
