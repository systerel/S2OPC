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

#include "role_permissions_value_eval_bs.h"

#include "sopc_assert.h"
#include "sopc_macros.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void role_permissions_value_eval_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
static OpcUa_PermissionType mergedPermissions = OpcUa_PermissionType_None;

void role_permissions_value_eval_bs__add_user_role_permissions(
    const constants__t_NodeId_i role_permissions_value_eval_bs__p_role,
    const constants__t_PermissionType_i role_permissions_value_eval_bs__p_permissions)
{
    SOPC_UNUSED_ARG(role_permissions_value_eval_bs__p_role);
    mergedPermissions |= role_permissions_value_eval_bs__p_permissions;
}

void role_permissions_value_eval_bs__get_merged_user_role_permissions(
    const constants__t_NodeId_i role_permissions_value_eval_bs__p_role,
    constants__t_PermissionType_i* const role_permissions_value_eval_bs__p_permissions)
{
    SOPC_UNUSED_ARG(role_permissions_value_eval_bs__p_role);
    *role_permissions_value_eval_bs__p_permissions = mergedPermissions;
    mergedPermissions = OpcUa_PermissionType_None;
}

void role_permissions_value_eval_bs__init_user_role_permissions(
    const constants__t_NodeId_i role_permissions_value_eval_bs__p_role)
{
    SOPC_UNUSED_ARG(role_permissions_value_eval_bs__p_role);
    mergedPermissions = OpcUa_PermissionType_None;
}

static OpcUa_RolePermissionType* get_role_permission_at_idx(
    const constants__t_RolePermissionTypes_i role_permissions_value_eval_bs__p_rolePermissions,
    const t_entier4 role_permissions_value_eval_bs__p_idx)
{
    SOPC_ASSERT(NULL != role_permissions_value_eval_bs__p_rolePermissions);
    SOPC_Variant* variant = role_permissions_value_eval_bs__p_rolePermissions;

    SOPC_ASSERT(SOPC_VariantArrayType_Array == variant->ArrayType);
    SOPC_ASSERT(role_permissions_value_eval_bs__p_idx - 1 < variant->Value.Array.Length);
    SOPC_ASSERT(SOPC_ExtensionObject_Id == variant->BuiltInTypeId);

    OpcUa_RolePermissionType* rolePermission =
        (OpcUa_RolePermissionType*) variant->Value.Array.Content.ExtObjectArr[role_permissions_value_eval_bs__p_idx - 1]
            .Body.Object.Value;
    return rolePermission;
}

void role_permissions_value_eval_bs__read_rolePermissions_permissions(
    const constants__t_RolePermissionTypes_i role_permissions_value_eval_bs__p_rolePermissions,
    const t_entier4 role_permissions_value_eval_bs__p_idx,
    constants__t_PermissionType_i* const role_permissions_value_eval_bs__p_permissions)
{
    OpcUa_RolePermissionType* rolePermission = get_role_permission_at_idx(
        role_permissions_value_eval_bs__p_rolePermissions, role_permissions_value_eval_bs__p_idx);
    *role_permissions_value_eval_bs__p_permissions = (OpcUa_PermissionType) rolePermission->Permissions;
}

void role_permissions_value_eval_bs__read_rolePermissions_roleId(
    const constants__t_RolePermissionTypes_i role_permissions_value_eval_bs__p_rolePermissions,
    const t_entier4 role_permissions_value_eval_bs__p_idx,
    constants__t_NodeId_i* const role_permissions_value_eval_bs__p_roleId)
{
    OpcUa_RolePermissionType* rolePermission = get_role_permission_at_idx(
        role_permissions_value_eval_bs__p_rolePermissions, role_permissions_value_eval_bs__p_idx);
    *role_permissions_value_eval_bs__p_roleId = &rolePermission->RoleId;
}

void role_permissions_value_eval_bs__read_variant_rolePermissions(
    const constants__t_RolePermissionTypes_i role_permissions_value_eval_bs__p_rolePermissions,
    t_entier4* const role_permissions_value_eval_bs__p_nbr_of_rolePermissions)
{
    SOPC_ASSERT(NULL != role_permissions_value_eval_bs__p_rolePermissions);
    SOPC_ASSERT(SOPC_VariantArrayType_Array == role_permissions_value_eval_bs__p_rolePermissions->ArrayType);
    *role_permissions_value_eval_bs__p_nbr_of_rolePermissions =
        role_permissions_value_eval_bs__p_rolePermissions->Value.Array.Length;
}
