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

#include "address_space_user_permissions_bs.h"
#include "sopc_logger.h"
#include "sopc_macros.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space_user_permissions_bs__INITIALISATION(void)
{ /*Translated from B but an intialisation is not needed from this module.*/
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
static OpcUa_PermissionType mergedPermissions = OpcUa_PermissionType_None;

void address_space_user_permissions_bs__add_user_permissions(
    const constants__t_user_i address_space_user_permissions_bs__p_user,
    const constants__t_PermissionType_i address_space_user_permissions_bs__p_permissions)
{
    SOPC_UNUSED_ARG(address_space_user_permissions_bs__p_user);
    mergedPermissions |= address_space_user_permissions_bs__p_permissions;
}

void address_space_user_permissions_bs__get_merged_user_permissions(
    const constants__t_user_i address_space_user_permissions_bs__p_user,
    constants__t_PermissionType_i* const address_space_user_permissions_bs__p_permissions)
{
    SOPC_UNUSED_ARG(address_space_user_permissions_bs__p_user);
    *address_space_user_permissions_bs__p_permissions = mergedPermissions;
    mergedPermissions = OpcUa_PermissionType_None;
}

void address_space_user_permissions_bs__init_user_permissions(
    const constants__t_user_i address_space_user_permissions_bs__p_user)
{
    SOPC_UNUSED_ARG(address_space_user_permissions_bs__p_user);
    mergedPermissions = OpcUa_PermissionType_None;
}

extern void address_space_user_permissions_bs__is_operation_authorized(
    const constants__t_PermissionType_i address_space_user_permissions_bs__p_permissions,
    const constants__t_operation_type_i address_space_user_permissions_bs__p_operation_type,
    t_bool* const address_space_user_permissions_bs__p_bres)
{
    switch (address_space_user_permissions_bs__p_operation_type)
    {
    case constants__e_operation_type_read:
        *address_space_user_permissions_bs__p_bres =
            (OpcUa_PermissionType_Read ==
             (address_space_user_permissions_bs__p_permissions & OpcUa_PermissionType_Read));
        break;
    case constants__e_operation_type_write:
        *address_space_user_permissions_bs__p_bres =
            (OpcUa_PermissionType_Write ==
             (address_space_user_permissions_bs__p_permissions & OpcUa_PermissionType_Write));
        break;
    case constants__e_operation_type_executable:
        *address_space_user_permissions_bs__p_bres =
            (OpcUa_PermissionType_Call ==
             (address_space_user_permissions_bs__p_permissions & OpcUa_PermissionType_Call));
        break;
    case constants__e_operation_type_addnode:
        *address_space_user_permissions_bs__p_bres =
            (OpcUa_PermissionType_AddNode ==
             (address_space_user_permissions_bs__p_permissions & OpcUa_PermissionType_AddNode));
        break;
    case constants__e_operation_type_receive_events:
        *address_space_user_permissions_bs__p_bres =
            (OpcUa_PermissionType_ReceiveEvents ==
             (address_space_user_permissions_bs__p_permissions & OpcUa_PermissionType_ReceiveEvents));
        break;
    default:
        *address_space_user_permissions_bs__p_bres = false;
        break;
    }
}
