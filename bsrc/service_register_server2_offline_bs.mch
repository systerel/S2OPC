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
    service_register_server2_offline_bs

SEES
    constants

DEFINITIONS
    d_variables_set_it ==
    s_registered_server2_offline_to_iterate_it,
    s_registered_server2_offline_iterated_it
    ;

    d_inv_set_it ==
    s_registered_server2_offline_to_iterate_it <: s_RegisteredServer2_offline &
    s_registered_server2_offline_iterated_it <: s_RegisteredServer2_offline &
    s_registered_server2_offline_to_iterate_it /\ s_registered_server2_offline_iterated_it = {} &
    s_registered_server2_offline_to_iterate_it \/ s_registered_server2_offline_iterated_it = s_RegisteredServer2_offline
    ;

    d_init_set_it ==
    s_registered_server2_offline_to_iterate_it :(s_registered_server2_offline_to_iterate_it = {}) ||
    s_registered_server2_offline_iterated_it :(s_registered_server2_offline_iterated_it = {})

ABSTRACT_VARIABLES
    s_RegisteredServer2_offline,
    d_variables_set_it

INVARIANT
    s_RegisteredServer2_offline <: NAT &
    d_inv_set_it

INITIALISATION
    s_RegisteredServer2_offline :( s_RegisteredServer2_offline <: NAT & s_RegisteredServer2_offline = {}) ||
    d_init_set_it

OPERATIONS

    /* Iterators on set */

    continue <-- init_iter_registered_server2_offline_set =
    BEGIN
        s_registered_server2_offline_to_iterate_it,
        s_registered_server2_offline_iterated_it,
        continue
        :(s_registered_server2_offline_to_iterate_it <: s_RegisteredServer2_offline &
          s_registered_server2_offline_iterated_it <: s_RegisteredServer2_offline &
          continue : BOOL &
          s_registered_server2_offline_to_iterate_it = s_RegisteredServer2_offline &
          s_registered_server2_offline_iterated_it = {} &
          continue = bool(s_registered_server2_offline_to_iterate_it /= {})
        )
    END
    ;

    continue, p_recordId <-- continue_iter_registered_server2_offline_set =
    PRE
        s_registered_server2_offline_to_iterate_it /= {}
    THEN
        ANY l_recordId WHERE
            l_recordId : NAT &
            l_recordId : s_registered_server2_offline_to_iterate_it
        THEN
            continue := bool(s_registered_server2_offline_to_iterate_it /= {l_recordId}) ||
            p_recordId := l_recordId ||
            s_registered_server2_offline_to_iterate_it := s_registered_server2_offline_to_iterate_it - {l_recordId} ||
            s_registered_server2_offline_iterated_it := s_registered_server2_offline_iterated_it \/ {l_recordId}
        END
    END
    ;

    /* Operations on set */

    reset_registered_server2_offline_set =
    BEGIN
        s_RegisteredServer2_offline :( s_RegisteredServer2_offline <: NAT & s_RegisteredServer2_offline = {})
    END
    ;

    add_to_registered_server2_offline_set(p_recordId) =
    PRE
        p_recordId : NAT
    THEN
        s_RegisteredServer2_offline :( s_RegisteredServer2_offline <: NAT & s_RegisteredServer2_offline = s_RegisteredServer2_offline$0 \/ {p_recordId})
    END
    ;

    service_register_server2_offline_bs_UNINITIALISATION =
    BEGIN
        skip
    END

END
