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
    address_space_namespaces

SEES
    constants

ABSTRACT_VARIABLES
    a_default_role_permissions

INVARIANT
    a_default_role_permissions : t_NamespaceIdx +-> t_RolePermissionTypes

INITIALISATION
    a_default_role_permissions :: t_NamespaceIdx +-> t_RolePermissionTypes

OPERATIONS

    /* Set DRP of all namespaces in cache if it was not done previously.
       Checks if the given NS has a DRP and returns result */
    bres <-- has_Namespace_DefaultRolePermissions(p_idx) =
    PRE
        p_idx : t_NamespaceIdx
    THEN
        bres, a_default_role_permissions
        :(a_default_role_permissions : t_NamespaceIdx +-> t_RolePermissionTypes &
          bres : BOOL &
          bres = bool(p_idx : dom(a_default_role_permissions))
        )
    END
    ;

    /* Returns the DRP of the given NS, has_Namespace_DefaultRolePermissions shall have return TRUE prior to this call.
       Note: returned DRP shall be freed using delete_rolePermissions after use.
    */
    p_DefaultRolePermissions <-- get_Namespace_DefaultRolePermissions(p_idx) =
    PRE
        p_idx : t_NamespaceIdx &
        p_idx : dom(a_default_role_permissions)
    THEN
        p_DefaultRolePermissions
        :(p_DefaultRolePermissions : t_RolePermissionTypes_i &
          p_DefaultRolePermissions = a_default_role_permissions(p_idx)
        )
    END
    ;

    address_space_default_role_permissions_array_bs_UNINITIALISATION =
    BEGIN
        skip
    END
    ;

    /* It shall be called on returned DRP by get_Namespace_DefaultRolePermissions after use */
    delete_rolePermissions(p_rolePermissions) =
    PRE
        p_rolePermissions : t_RolePermissionTypes_i &
        p_rolePermissions : t_RolePermissionTypes
    THEN
        skip
    END

END
