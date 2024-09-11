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
    session_role_identities_it

SEES
    constants

DEFINITIONS
    d_variables ==
    identityIdx_to_iterate,
    identityIdx_iterated,
    nb_identities
    ;

    d_inv ==
    (
    nb_identities : NAT &
    identityIdx_to_iterate <: NAT1 &
    identityIdx_iterated   <: NAT1 &
    identityIdx_to_iterate /\ identityIdx_iterated = {} &
    1..nb_identities = identityIdx_to_iterate \/ identityIdx_iterated
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_identities(p_nb_identities) =
    PRE
        p_nb_identities : NAT
    THEN
        identityIdx_iterated := {}        ||
        identityIdx_to_iterate := 1..p_nb_identities ||
        nb_identities := p_nb_identities ||
        p_continue := bool(p_nb_identities > 0)
    END
    ;

    p_continue, p_identityIdx <-- continue_iter_identities =
    PRE
        identityIdx_to_iterate /= {}
    THEN
        ANY l_identityIdx WHERE
            l_identityIdx : NAT &
            l_identityIdx : identityIdx_to_iterate
        THEN
            identityIdx_iterated   := identityIdx_iterated   \/ {l_identityIdx} ||
            identityIdx_to_iterate := identityIdx_to_iterate -  {l_identityIdx} ||
            p_identityIdx          := l_identityIdx                                            ||
            p_continue             := bool(identityIdx_to_iterate /= {l_identityIdx})
        END
    END

END
