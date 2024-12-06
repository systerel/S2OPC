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
    address_space_namespaces_metadata

SEES
    constants

ABSTRACT_VARIABLES
    a_default_role_permissions

INVARIANT
    a_default_role_permissions : t_NamespaceIdx +-> t_RolePermissionTypes

INITIALISATION
    a_default_role_permissions := {}

OPERATIONS
    may_initialize_default_role_permissions =
    BEGIN
        a_default_role_permissions :(a_default_role_permissions : t_NamespaceIdx +-> t_RolePermissionTypes)
    END
    ;

    bres <-- has_DefaultRolePermissions_at_idx(p_idx) =
    PRE
        p_idx : t_NamespaceIdx
    THEN
        bres := bool(p_idx : dom(a_default_role_permissions))
    END
    ;

    /* Returns the DRP of the given NS, has_DefaultRolePermissions_at_idx shall have return TRUE prior to this call.
       Note: we need to create a variant to be consistant with read_AddressSpace_RolePermissions returned value when DRP is not used.
             The returned DRP shall be freed using delete_rolePermissions after use.
    */
    p_DefaultRolePermissions <-- get_DefaultRolePermissions_at_idx(p_idx) =
    PRE
        p_idx : t_NamespaceIdx &
        // Make sure a DefaultRP has well been added at this index
        p_idx : dom(a_default_role_permissions)
    THEN
        p_DefaultRolePermissions :(
            p_DefaultRolePermissions : t_RolePermissionTypes_i &
            p_DefaultRolePermissions = a_default_role_permissions(p_idx))
    END
    ;

    address_space_default_role_permissions_array_bs_UNINITIALISATION =
    BEGIN
        a_default_role_permissions :: {{}}
    END
    ;

    /* It shall be called on returned DRP by get_DefaultRolePermissions_at_idx after use */
    delete_rolePermissions(p_rolePermissions) =
    PRE
        p_rolePermissions : t_RolePermissionTypes_i &
        p_rolePermissions : t_RolePermissionTypes
    THEN
        skip
    END

END
