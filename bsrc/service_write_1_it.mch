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
    service_write_1_it

SEES
    constants

DEFINITIONS
    d_variables ==
        wreqs_nb_reqs,
        wreqs_to_iterate,
        wreqs_iterated
    ;

    d_inv ==
    (
        wreqs_nb_reqs : NAT &
        wreqs_nb_reqs <= k_n_WriteResponse_max &

        wreqs_to_iterate <: t_WriteValue &
        wreqs_iterated <: t_WriteValue &
        wreqs_to_iterate /\ wreqs_iterated = {}
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS
    continue <-- init_iter_write_request(nb_req) =
    PRE
        nb_req : NAT &
        nb_req <= k_n_WriteResponse_max
    THEN
        wreqs_nb_reqs := nb_req ||
        wreqs_iterated := {} ||
        wreqs_to_iterate := cast_t_WriteValue[1..nb_req] ||
        continue := bool(0 < nb_req)
    END
    ;

    continue, wvi <-- continue_iter_write_request =
    PRE
        wreqs_to_iterate /= {}
    THEN
        ANY l_wvi WHERE
            l_wvi : t_WriteValue_i &
            l_wvi : t_WriteValue &
            l_wvi : wreqs_to_iterate
        THEN
            wreqs_iterated := wreqs_iterated \/ {l_wvi} ||
            wreqs_to_iterate := wreqs_to_iterate - {l_wvi} ||
            wvi := l_wvi ||
            continue := bool(wreqs_to_iterate /= {l_wvi})
        END
    END

END
