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

/******************************************************************************

 File Name            : address_space_user_permissions.h

 Date                 : 30/10/2024 16:23:51

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _address_space_user_permissions_h
#define _address_space_user_permissions_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "address_space_authorization_session_roles_it_bs.h"
#include "address_space_namespaces.h"
#include "address_space_user_permissions_bs.h"
#include "role_permissions_value_eval.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_bs.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space_user_permissions__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define address_space_user_permissions__address_space_default_role_permissions_array_bs_UNINITIALISATION address_space_namespaces__address_space_default_role_permissions_array_bs_UNINITIALISATION
#define address_space_user_permissions__is_operation_authorized address_space_user_permissions_bs__is_operation_authorized

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void address_space_user_permissions__l_read_AddressSpace_RolePermissionsOrDefault(
   const constants__t_NodeId_i address_space_user_permissions__p_nodeId,
   constants__t_RolePermissionTypes_i * const address_space_user_permissions__p_rolePermissions);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_user_permissions__has_AddressSpace_RolePermissions(
   const constants__t_NodeId_i address_space_user_permissions__p_nodeId,
   t_bool * const address_space_user_permissions__bres);
extern void address_space_user_permissions__read_AddressSpace_UserRolePermissions(
   const constants__t_NodeId_i address_space_user_permissions__p_nodeId,
   const constants__t_user_i address_space_user_permissions__p_user,
   const constants__t_sessionRoles_i address_space_user_permissions__p_roles,
   constants__t_PermissionType_i * const address_space_user_permissions__p_permissions);

#endif
