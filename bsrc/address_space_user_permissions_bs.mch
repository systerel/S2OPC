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
    address_space_user_permissions_bs

SEES
    constants

ABSTRACT_CONSTANTS
    conv_merge_user_permission

PROPERTIES
    conv_merge_user_permission : POW(t_PermissionType_i) --> t_PermissionType_i

ABSTRACT_VARIABLES
    a_permissions_authorizes_operation,

    a_user,
    s_user_permissions

INVARIANT
    a_permissions_authorizes_operation : t_PermissionType_i <-> t_operation_type &

    a_user : t_user_i &
    s_user_permissions <: t_PermissionType_i


INITIALISATION
    a_permissions_authorizes_operation :: t_PermissionType_i <-> t_operation_type ||

    a_user := c_user_indet ||
    s_user_permissions :: {}

OPERATIONS
    p_bres <-- is_operation_authorized(p_permissions, p_operation_type) =
    PRE
        p_permissions : t_PermissionType_i &
        p_operation_type : t_operation_type_i &
        p_operation_type : t_operation_type
    THEN
        p_bres := bool(p_permissions |-> p_operation_type : a_permissions_authorizes_operation)
    END
    ;

    init_user_permissions(p_user) =
    PRE
        p_user : t_user_i &
        p_user : t_user
    THEN
        a_user := p_user
    END
    ;

    add_user_permissions(p_user, p_permissions) =
    PRE
        p_user : t_user_i &
        a_user = p_user &
        p_permissions : t_PermissionType_i
    THEN
        s_user_permissions := s_user_permissions \/ {p_permissions}
    END
    ;

    p_permissions <-- get_merged_user_permissions(p_user) =
    PRE
        p_user : t_user_i &
        a_user = p_user
    THEN
        a_user := c_user_indet ||
        s_user_permissions := {} ||
        p_permissions := conv_merge_user_permission(s_user_permissions)
    END

END
