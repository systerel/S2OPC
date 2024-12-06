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
    address_space_authorization_session_roles_it_bs

SEES
    constants

DEFINITIONS
    d_variables ==
        a_user_roles,
        roles_to_iterate,
        roles_iterated,
        s_roles;

    d_inv ==
        a_user_roles : t_user <-> t_NodeId &
        s_roles <: t_NodeId &
        roles_to_iterate <: s_roles &
        roles_iterated   <: s_roles &
        roles_to_iterate /\ roles_iterated = {} &
        roles_to_iterate \/ roles_iterated = s_roles;

    d_init ==
        d_variables
        :( a_user_roles = {} &
           s_roles = {} &
           roles_to_iterate = {} &
           roles_iterated = {}
         )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_inv

INITIALISATION
    d_init

OPERATIONS

    p_continue <-- init_iter_roles(p_user, p_roles) =
    PRE
        p_user : t_user_i &
        p_user : t_user &
        p_roles : t_sessionRoles_i &
        p_roles : t_sessionRoles
    THEN
        a_user_roles,
        s_roles :(s_roles <: t_NodeId &
                  a_user_roles : t_user <-> t_NodeId &
                  a_user_roles = {p_user} * s_roles) ||
        roles_iterated := {} ||
        roles_to_iterate := s_roles ||
        p_continue := bool(roles_to_iterate /= {})
    END
    ;

    p_continue, p_role <-- continue_iter_roles =
    PRE
        roles_to_iterate /= {}
    THEN
        ANY l_role WHERE
            l_role : t_NodeId_i &
            l_role : roles_to_iterate
        THEN
            roles_iterated   := roles_iterated   \/ {l_role} ||
            roles_to_iterate := roles_to_iterate -  {l_role} ||
            p_role           := l_role                       ||
            p_continue       := bool(roles_to_iterate /= {l_role})
        END
    END
    ;

    clear_iter_roles =
    BEGIN
        d_init
    END

END
