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
    role_permissions_value_eval_bs

SEES
    address_space_bs,
    constants

ABSTRACT_CONSTANTS
    conv_merge_user_role_permission

PROPERTIES
    conv_merge_user_role_permission : POW(t_PermissionType_i) --> t_PermissionType_i

ABSTRACT_VARIABLES
    a_rolesPermissions,
    a_roles,
    a_permissions,

    a_user_role,
    s_user_role_permissions

INVARIANT
    a_rolesPermissions : t_RolePermissionTypes_i &
    a_roles : seq(t_NodeId) &
    a_permissions : seq(t_PermissionType_i) &
    size(a_roles) = size(a_permissions) &

    a_user_role : t_NodeId &
    s_user_role_permissions <: t_PermissionType_i

INITIALISATION
    a_rolesPermissions := c_RolePermissionTypes_indet ||
    a_roles :: {} ||
    a_permissions :: {} ||

    a_user_role := c_NodeId_indet ||
    s_user_role_permissions :: {}

OPERATIONS
    /* Operations related to iteration on RolePermissions variant value */

    p_nbr_of_rolePermissions <-- read_variant_rolePermissions(p_rolePermissions) =
    PRE
        p_rolePermissions : t_RolePermissionTypes_i &
        p_rolePermissions : t_RolePermissionTypes
    THEN
        a_rolesPermissions,
        a_roles,
        a_permissions,
        p_nbr_of_rolePermissions
        :(a_rolesPermissions : t_RolePermissionTypes_i &
          a_rolesPermissions = p_rolePermissions &
          a_roles : seq(t_NodeId) &
          a_permissions : seq(t_PermissionType_i) &
          size(a_roles) = size(a_permissions) &
          p_nbr_of_rolePermissions = size(a_roles)
        )
    END
    ;

    p_roleId  <-- read_rolePermissions_roleId(p_rolePermissions, p_idx) =
    PRE
        p_rolePermissions : t_RolePermissionTypes_i &
        p_rolePermissions : t_RolePermissionTypes &
        a_rolesPermissions = p_rolePermissions &
        p_idx : NAT &
        p_idx <= size(a_roles)
    THEN
        p_roleId :(p_roleId : t_NodeId_i & p_roleId = a_roles(p_idx))
    END
    ;

    p_permissions  <-- read_rolePermissions_permissions(p_rolePermissions, p_idx) =
    PRE
        p_rolePermissions : t_RolePermissionTypes_i &
        p_rolePermissions : t_RolePermissionTypes &
        a_rolesPermissions = p_rolePermissions &
        p_idx : NAT &
        p_idx <= size(a_roles)
    THEN
        p_permissions :(p_permissions : t_PermissionType_i & p_permissions = a_permissions(p_idx))
    END
    ;

    /* Operation to store user role permissions */

    init_user_role_permissions(p_role) =
    PRE
        p_role : t_NodeId_i &
        conv_ExpandedNodeId_NodeId(Ref_HasTypeDefinition(a_NodeId~(p_role))) = c_RoleType_NodeId
    THEN
        a_user_role := p_role
    END
    ;

    add_user_role_permissions(p_role, p_permissions) =
    PRE
        p_role : t_NodeId_i &
        a_user_role = p_role &
        p_permissions : t_PermissionType_i
    THEN
        s_user_role_permissions :(s_user_role_permissions = s_user_role_permissions$0 \/ {p_permissions})
    END
    ;

    p_permissions <-- get_merged_user_role_permissions(p_role) =
    PRE
        p_role : t_NodeId_i &
        a_user_role = p_role
    THEN
        a_user_role := c_NodeId_indet ||
        s_user_role_permissions :: {} ||
        p_permissions := conv_merge_user_role_permission(s_user_role_permissions)
    END

END
