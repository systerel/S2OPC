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
    address_space_user_permissions

SEES
    constants,
    address_space_bs

ABSTRACT_VARIABLES
    a_node_has_permissions,
    a_user_node_permissions,
    a_permissions_authorizes_operation

INVARIANT
    a_node_has_permissions <: t_NodeId &
    a_user_node_permissions : t_user * t_NodeId --> t_PermissionType_i &
    a_permissions_authorizes_operation : t_PermissionType_i <-> t_operation_type

INITIALISATION
    a_node_has_permissions : (a_node_has_permissions <: t_NodeId) ||
    a_user_node_permissions :: t_user * t_NodeId --> t_PermissionType_i ||
    a_permissions_authorizes_operation : (a_permissions_authorizes_operation : t_PermissionType_i <-> t_operation_type)

OPERATIONS

    /* Operation promoted from default_role_permissions_array_bs */
    address_space_default_role_permissions_array_bs_UNINITIALISATION =
    BEGIN
        skip
    END
    ;

    bres <-- has_AddressSpace_RolePermissions(p_nodeId) =
    PRE
        p_nodeId : t_NodeId_i &
        p_nodeId : t_NodeId
    THEN
        bres := bool(p_nodeId : a_node_has_permissions)
    END
    ;

    /* Note: association user -> role is responsibility of caller, we cannot express it since defined in session (no SEE possible) */
    p_permissions <-- read_AddressSpace_UserRolePermissions(p_nodeId, p_user, p_roles) =
    PRE
        p_nodeId : t_NodeId_i &
        p_nodeId : t_NodeId &
        p_user   : t_user_i &
        p_user   : t_user &
        p_roles  : t_sessionRoles_i &
        p_roles  : t_sessionRoles &
        p_nodeId : a_node_has_permissions
    THEN
        p_permissions := a_user_node_permissions(p_user |-> p_nodeId)
    END
    ;

    p_bres <-- is_operation_authorized(p_permissions, p_operation_type) =
    PRE
        p_permissions : t_PermissionType_i &
        p_operation_type : t_operation_type_i &
        p_operation_type : t_operation_type
    THEN
        p_bres := bool(p_permissions |-> p_operation_type : a_permissions_authorizes_operation)
    END

END
