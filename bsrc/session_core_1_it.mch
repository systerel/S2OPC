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
    session_core_1_it

SEES
    constants

DEFINITIONS
    d_variables ==
        sessions_to_iterate,
        sessions_iterated
    ;

    d_inv ==
    (
        sessions_to_iterate <: t_session &
        sessions_iterated   <: t_session &
        sessions_to_iterate /\ sessions_iterated = {}
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_variables :( d_inv )

OPERATIONS

    p_continue <-- init_iter_session =
    BEGIN
        sessions_iterated   := {}        ||
        sessions_to_iterate := t_session ||
        p_continue          := bool(t_session /= {})
    END
    ;

    p_continue, p_session <-- continue_iter_session =
    PRE
        sessions_to_iterate /= {}
    THEN
        ANY l_session WHERE
            l_session : t_session_i &
            l_session : t_session   &
            l_session : sessions_to_iterate
        THEN
            sessions_iterated   := sessions_iterated   \/ {l_session} ||
            sessions_to_iterate := sessions_to_iterate -  {l_session} ||
            p_session           := l_session                          ||
            p_continue          := bool(sessions_to_iterate /= {l_session})
        END
    END

END
