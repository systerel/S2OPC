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
    session_roles_granted_bs

SEES
    constants

ABSTRACT_VARIABLES
    s_sessionRoles

INVARIANT
    s_sessionRoles <: t_NodeId

INITIALISATION
    s_sessionRoles :(s_sessionRoles <: t_NodeId)

OPERATIONS
    initialize_session_roles =
    BEGIN
        s_sessionRoles := {}
    END
    ;

    add_role_to_session(p_role_nodeId) =
    PRE
        p_role_nodeId : t_NodeId_i &
        p_role_nodeId : t_NodeId
    THEN
        s_sessionRoles := s_sessionRoles \/ {p_role_nodeId}
    END
    ;

    b_roles <-- pop_session_roles =
    BEGIN
        b_roles :: t_sessionRoles_i ||
        s_sessionRoles := {}
    END

END
