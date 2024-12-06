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

#include "namespace_default_role_permissions_value_bs.h"
#include "sopc_macros.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void namespace_default_role_permissions_value_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/

static SOPC_Variant* namespace_default_role_permissions_value_bs__check_and_get_variant(
    const constants__t_Variant_i namespace_array_bs__p_variant)
{
    SOPC_Variant* variant = namespace_array_bs__p_variant;
    if (NULL == variant)
    {
        return NULL;
    }

    // Variant shall be a list of ExtObject RolePermissionType
    if (variant->BuiltInTypeId != SOPC_ExtensionObject_Id || variant->ArrayType != SOPC_VariantArrayType_Array)
    {
        return NULL;
    }

    for (int32_t i = 0; i < variant->Value.Array.Length; i++)
    {
        if (&OpcUa_RolePermissionType_EncodeableType !=
            variant->Value.Array.Content.ExtObjectArr[i].Body.Object.ObjType)
        {
            return NULL;
        }
    }

    return variant;
}

void namespace_default_role_permissions_value_bs__get_conv_Variant_RolePermissionTypes(
    const constants__t_Variant_i namespace_default_role_permissions_value_bs__p_variant,
    constants__t_RolePermissionTypes_i* const namespace_default_role_permissions_value_bs__p_rolePermissions)
{
    SOPC_Variant* variant = namespace_default_role_permissions_value_bs__check_and_get_variant(
        namespace_default_role_permissions_value_bs__p_variant);
    *namespace_default_role_permissions_value_bs__p_rolePermissions = variant;
}

void namespace_default_role_permissions_value_bs__delete_rolePermissions(
    const constants__t_RolePermissionTypes_i namespace_default_role_permissions_value_bs__p_rolePermissions)
{
    SOPC_Variant_Delete(namespace_default_role_permissions_value_bs__p_rolePermissions);
}
