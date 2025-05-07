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
    node_management_delete_nodes_items_it
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
        rreqs_nb_reqs <= k_n_nodesToDelete_max &

        rreqs_to_iterate <: t_nodeToDeleteReqIndex &
        rreqs_iterated <: t_nodeToDeleteReqIndex &
        rreqs_to_iterate /\ rreqs_iterated = {} &
        rreqs_to_iterate \/ rreqs_iterated = 1..rreqs_nb_reqs
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS
    continue <-- init_iter_delete_nodes_items(nb_req) =
    PRE
        nb_req : NAT &
        nb_req : t_nodeToDeleteReqIndex &
        nb_req <= k_n_nodesToDelete_max
    THEN
        rreqs_nb_reqs := nb_req ||
        rreqs_iterated := {} ||
        rreqs_to_iterate := 1..nb_req ||
        continue := bool(0 < nb_req)
    END
    ;

    continue, index <-- continue_iter_delete_nodes_items =
    PRE
        rreqs_to_iterate /= {} &
        rreqs_to_iterate <: 1..rreqs_nb_reqs
    THEN
        ANY l_index WHERE
            l_index : NAT &
            l_index : t_nodeToDeleteReqIndex &
            l_index : rreqs_to_iterate
        THEN
            rreqs_iterated := rreqs_iterated \/ {l_index} ||
            rreqs_to_iterate := rreqs_to_iterate - {l_index} ||
            index := l_index ||
            continue := bool(rreqs_to_iterate /= {l_index})
        END
    END

END
