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
    user_authorization_bs

SEES
    constants

ABSTRACT_VARIABLES
    s_AuthorizedOperations

INVARIANT
    s_AuthorizedOperations <: t_operation_type * t_NodeId * t_AttributeId * t_user

INITIALISATION
    s_AuthorizedOperations :: {{}}

OPERATIONS
    p_authorized <-- get_user_authorization (p_operation_type, p_node_id, p_attribute_id, p_user) =
    PRE
        p_operation_type : t_operation_type_i &
        p_operation_type : t_operation_type   &
        p_node_id        : t_NodeId_i         &
        p_node_id        : t_NodeId           &
        p_attribute_id   : t_AttributeId_i    &
        p_attribute_id   : t_AttributeId      &
        p_user           : t_user_i           &
        p_user           : t_user
    THEN
        p_authorized := bool(p_operation_type |-> p_node_id |-> p_attribute_id |-> p_user : s_AuthorizedOperations)
    END

END
