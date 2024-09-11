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
    session_role_identity_eval

SEES
    constants

ABSTRACT_VARIABLES
    a_user_identity_match

INVARIANT
    a_user_identity_match : t_user <-> t_Identity

INITIALISATION
    a_user_identity_match :: t_user <-> t_Identity

OPERATIONS
    p_bres <-- user_and_identity_match(p_user, p_identity) =
    PRE
        p_user : t_user_i &
        p_user : t_user &
        p_identity : t_Identity_i
    THEN
        p_bres := bool(p_user |-> p_identity: a_user_identity_match)
    END
END
