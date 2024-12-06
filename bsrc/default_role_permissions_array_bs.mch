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
    default_role_permissions_array_bs

SEES
    constants,
    namespace_array_bs

ABSTRACT_VARIABLES
    init_default_role_permissions_bs,
    a_default_role_permissions_bs,
    nbr_default_role_permissions_bs

INVARIANT
    init_default_role_permissions_bs : BOOL &
    a_default_role_permissions_bs : t_NamespaceIdx +-> t_RolePermissionTypes_i &
    nbr_default_role_permissions_bs : NAT

INITIALISATION
    init_default_role_permissions_bs := FALSE ||
    a_default_role_permissions_bs :: {{}} ||
    nbr_default_role_permissions_bs := 0

OPERATIONS
    init_array_of_DefaultRolePermissions(p_nb_namespaces) =
    PRE
        p_nb_namespaces : NAT
    THEN
        init_default_role_permissions_bs := TRUE ||
        nbr_default_role_permissions_bs := p_nb_namespaces
    END
    ;

    p_res <-- is_default_role_permissions_initialized =
    BEGIN
        p_res := bool(init_default_role_permissions_bs = TRUE)
    END
    ;

    add_DefaultRolePermissions_at_idx(p_namespaceUri, p_idx, p_DefaultRolePermissions) =
    PRE
        p_namespaceUri : t_NamespaceUri &
        p_idx : t_NamespaceIdx &
        // Use for log only
        p_namespaceUri = a_namespaceUris(cast_t_NamespaceIdx~(p_idx)) &
        // Make sure the array is initialized
        init_default_role_permissions_bs = TRUE &
        // Make sure we try to add the DRP at a valid array index
        cast_t_NamespaceIdx~(p_idx) <= nbr_default_role_permissions_bs &
        p_DefaultRolePermissions : t_RolePermissionTypes_i
    THEN
        a_default_role_permissions_bs :(
            a_default_role_permissions_bs : t_NamespaceIdx +-> t_RolePermissionTypes_i &
            a_default_role_permissions_bs = a_default_role_permissions_bs$0 \/ {p_idx |-> p_DefaultRolePermissions}
        )
    END
    ;

    bres <-- has_DefaultRolePermissions_at_idx(p_idx) =
    PRE
        p_idx : t_NamespaceIdx &
        // Make sure the array is initialized
        init_default_role_permissions_bs = TRUE
    THEN
        bres := bool(p_idx : dom(a_default_role_permissions_bs) &
                     a_default_role_permissions_bs(p_idx) : t_RolePermissionTypes)
    END
    ;

    p_DefaultRolePermissions <-- get_DefaultRolePermissions_at_idx(p_idx) =
    PRE
        p_idx : t_NamespaceIdx &
        // Make sure the array is initialized
        init_default_role_permissions_bs = TRUE &
        p_idx : dom(a_default_role_permissions_bs) &
        a_default_role_permissions_bs(p_idx) : t_RolePermissionTypes
    THEN
        p_DefaultRolePermissions := a_default_role_permissions_bs(p_idx)
    END
    ;

    address_space_default_role_permissions_array_bs_UNINITIALISATION =
    BEGIN
        init_default_role_permissions_bs := FALSE ||
        a_default_role_permissions_bs :: {{}} ||
        nbr_default_role_permissions_bs := 0
    END

END
