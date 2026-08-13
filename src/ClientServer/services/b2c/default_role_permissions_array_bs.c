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

#include "default_role_permissions_array_bs.h"
#include "sopc_assert.h"
#include "sopc_builtintypes.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"

#include <string.h>

/*------------------------
   INITIALISATION Clause
  ------------------------*/
static bool isInit = false;
static SOPC_Variant* a_defaultRolePermissions = NULL; // SOPC_Variant[]
static t_entier4 nbr_of_namespaces = 0;
void default_role_permissions_array_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void default_role_permissions_array_bs__add_DefaultRolePermissions_at_idx(
    const constants__t_NamespaceUri default_role_permissions_array_bs__p_namespaceUri,
    const constants__t_NamespaceIdx default_role_permissions_array_bs__p_idx,
    const constants__t_RolePermissionTypes_i default_role_permissions_array_bs__p_DefaultRolePermissions)
{
    SOPC_ASSERT(default_role_permissions_array_bs__p_idx - 1 < nbr_of_namespaces);

    // Variant has already been checked in get_conv_Variant_RolePermissionTypes: it is a RolePermissions
    if (constants__c_RolePermissionTypes_indet == default_role_permissions_array_bs__p_DefaultRolePermissions)
    {
        // Add null variant
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "No DefaultRolePermissions defined for NS=%" PRIu16
                                 ":%s. "
                                 "Therefore this namespace does not handle permissions. RolePermissions "
                                 "attribute of nodes in this namespace will be ignored.",
                                 default_role_permissions_array_bs__p_idx - 1,
                                 SOPC_String_GetRawCString(default_role_permissions_array_bs__p_namespaceUri));
    }
    else
    {
        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "DefaultRolePermissions added for NS=%" PRIu16 ".",
                               default_role_permissions_array_bs__p_idx - 1);
        // Important Note: since this is computed only once and DRP shall be static in server, we keep a shallow copy of
        // content
        a_defaultRolePermissions[default_role_permissions_array_bs__p_idx - 1] =
            *default_role_permissions_array_bs__p_DefaultRolePermissions;
    }
}

void default_role_permissions_array_bs__address_space_default_role_permissions_array_bs_UNINITIALISATION(void)
{
    SOPC_Free(a_defaultRolePermissions);
    a_defaultRolePermissions = NULL;
    nbr_of_namespaces = 0;
    isInit = false;
}

void default_role_permissions_array_bs__get_DefaultRolePermissions_at_idx(
    const constants__t_NamespaceIdx default_role_permissions_array_bs__p_idx,
    constants__t_RolePermissionTypes_i* const default_role_permissions_array_bs__p_DefaultRolePermissions)
{
    SOPC_ASSERT(NULL != a_defaultRolePermissions);
    *default_role_permissions_array_bs__p_DefaultRolePermissions =
        &a_defaultRolePermissions[default_role_permissions_array_bs__p_idx];
}

void default_role_permissions_array_bs__has_DefaultRolePermissions_at_idx(
    const constants__t_NamespaceIdx default_role_permissions_array_bs__p_idx,
    t_bool* const default_role_permissions_array_bs__bres)
{
    SOPC_ASSERT(NULL != a_defaultRolePermissions);
    isInit = true;
    *default_role_permissions_array_bs__bres = false;
    if (default_role_permissions_array_bs__p_idx < nbr_of_namespaces)
    {
        *default_role_permissions_array_bs__bres =
            (SOPC_Null_Id != a_defaultRolePermissions[default_role_permissions_array_bs__p_idx].BuiltInTypeId);
    }
}

void default_role_permissions_array_bs__init_array_of_DefaultRolePermissions(
    const t_entier4 default_role_permissions_array_bs__p_nb_namespaces)
{
    if (default_role_permissions_array_bs__p_nb_namespaces > 0)
    {
        nbr_of_namespaces = default_role_permissions_array_bs__p_nb_namespaces;
        a_defaultRolePermissions = SOPC_Calloc((size_t) nbr_of_namespaces, sizeof(SOPC_Variant));
        SOPC_ASSERT(NULL != a_defaultRolePermissions);
    }
}

void default_role_permissions_array_bs__is_default_role_permissions_initialized(
    t_bool* const default_role_permissions_array_bs__p_res)
{
    *default_role_permissions_array_bs__p_res = isInit;
}
