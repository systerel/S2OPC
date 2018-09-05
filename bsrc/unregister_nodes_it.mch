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
    unregister_nodes_it

SEES
    constants

DEFINITIONS
    d_variables ==
        rreqs_to_iterate,
        rreqs_iterated
    ;

    d_inv ==
    (
        rreqs_to_iterate <: t_unregisterNodesReqIndex &
        rreqs_iterated <: t_unregisterNodesReqIndex &
        rreqs_to_iterate /\ rreqs_iterated = {}
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS
    continue <-- init_iter_unregister_nodes_request(nb_req) =
    PRE
        nb_req : NAT &
        nb_req <= k_n_unregisterNodes_max
    THEN
        rreqs_iterated := {} ||
        rreqs_to_iterate := 1..nb_req ||
        continue := bool(0 < nb_req)
    END
    ;

    continue, index <-- continue_iter_unregister_nodes_request =
    PRE
        rreqs_to_iterate /= {}
    THEN
        ANY l_index WHERE
            l_index : NAT &
            l_index : t_unregisterNodesReqIndex &
            l_index : rreqs_to_iterate
        THEN
            rreqs_iterated := rreqs_iterated \/ {l_index} ||
            rreqs_to_iterate := rreqs_to_iterate - {l_index} ||
            index := l_index ||
            continue := bool(rreqs_to_iterate /= {l_index})
        END
    END

END
