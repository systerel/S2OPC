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
    user_bs

SEES
    constants

ABSTRACT_VARIABLES
    s_user_anonymous,
    s_user_username,
    a_username

INVARIANT
    s_user_anonymous <: t_user &
    s_user_username <: t_user &
    s_user_anonymous /\ s_user_username = {} &
    a_username : s_user_username --> t_Criteria

INITIALISATION
    s_user_anonymous,
    s_user_username,
    a_username :( s_user_anonymous <: t_user &
    s_user_username <: t_user &
    s_user_anonymous /\ s_user_username = {} &
    a_username : s_user_username --> t_Criteria)

OPERATIONS
    b_res <-- is_anonymous(p_user) =
    PRE
        p_user : t_user_i &
        p_user : t_user
    THEN
        b_res := bool(p_user : s_user_anonymous)
    END
    ;

    b_res <-- is_username(p_user) =
    PRE
        p_user : t_user_i &
        p_user : t_user
    THEN
        b_res := bool(p_user : s_user_username)
    END
    ;

    b_res <-- are_username_equal(p_user, p_username) =
    PRE
        p_user : t_user_i &
        p_user : t_user &
        p_user : s_user_username &
        p_username : t_Criteria_i
    THEN
        b_res := bool(a_username(p_user) = p_username)
    END

END
