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
    role_permissions_value_eval
SEES
    constants,
    address_space_bs

ABSTRACT_VARIABLES
    a_role_permissions

INVARIANT
    a_role_permissions : t_NodeId * t_RolePermissionTypes --> t_PermissionType_i

INITIALISATION
    a_role_permissions :: t_NodeId * t_RolePermissionTypes --> t_PermissionType_i

OPERATIONS

    p_role_permissions <-- get_permissions_of_role(p_role_nodeId, p_rolesPermissions) =
    PRE
        p_role_nodeId : t_NodeId_i &
        /* Accepts only nodes for which type definition is RoleType */
        conv_ExpandedNodeId_NodeId(Ref_HasTypeDefinition(a_NodeId~(p_role_nodeId))) = c_RoleType_NodeId &
        p_rolesPermissions : t_RolePermissionTypes_i &
        p_rolesPermissions : t_RolePermissionTypes
    THEN
        p_role_permissions := a_role_permissions(p_role_nodeId |-> p_rolesPermissions)
        /* Note: it might be necessary if we want to link the operation treatment and a_role_permissions (see operation invariant)
        a_role_permissions, p_role_permissions
        :(a_role_permissions : t_NodeId * t_RolePermissionTypes --> t_PermissionType_i &
          {p_role_nodeId |-> p_rolesPermissions} <<| a_role_permissions = {p_role_nodeId |-> p_rolesPermissions} <<| a_role_permissions$0 &
          p_role_permissions : t_PermissionType_i &
          p_role_permissions = a_role_permissions(p_role_nodeId |-> p_rolesPermissions))
        */
    END

END
