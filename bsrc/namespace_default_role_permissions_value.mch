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
    namespace_default_role_permissions_value

SEES
    constants,
    address_space_bs

ABSTRACT_VARIABLES
    a_ref_default_role_permissions,
    a_conv_variant_RolePermissionTypes

INVARIANT
    a_ref_default_role_permissions : t_Reference +-> t_RolePermissionTypes_i &
    a_conv_variant_RolePermissionTypes : t_Variant_i +-> t_RolePermissionTypes_i

INITIALISATION
    a_ref_default_role_permissions := {} ||
    a_conv_variant_RolePermissionTypes := {}

OPERATIONS
    p_rolePermissions <-- get_conv_Variant_RolePermissionTypes(p_variant) =
    PRE
        p_variant : t_Variant_i &
        p_variant : t_Variant
    THEN
        p_rolePermissions := a_conv_variant_RolePermissionTypes(p_variant)
    END
    ;

    p_maybe_val_DefaultRolePermissions <-- ref_maybe_get_DefaultRolePermissions(p_ref) =
    PRE
        p_ref : t_Reference_i &
        Reference_IsForward(p_ref) = TRUE &
        Reference_ReferenceType(p_ref) = c_HasPropertyType_NodeId
    THEN
        p_maybe_val_DefaultRolePermissions :(
            p_maybe_val_DefaultRolePermissions : t_RolePermissionTypes_i &
            (p_ref : dom(a_ref_default_role_permissions) <=>
                (a_NodeClass(a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)))) = e_ncl_Variable &
                a_DataType(a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)))) = c_RolePermissionType_NodeId &
                a_BrowseName(a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)))) = c_DefaultRolePermissions_QualifiedName)) &
            (p_ref : dom(a_ref_default_role_permissions) =>
                a_ref_default_role_permissions(p_ref) = p_maybe_val_DefaultRolePermissions &
            p_maybe_val_DefaultRolePermissions = a_conv_variant_RolePermissionTypes(a_Value(a_NodeId~(conv_ExpandedNodeId_NodeId(Reference_TargetNode(p_ref)))))) &
            (p_ref /: dom(a_ref_default_role_permissions) => p_maybe_val_DefaultRolePermissions = c_RolePermissionTypes_indet))
    END
    ;

    delete_rolePermissions(p_rolePermissions) =
    PRE
        p_rolePermissions : t_RolePermissionTypes_i &
        p_rolePermissions : t_RolePermissionTypes
    THEN
        skip
    END

END
