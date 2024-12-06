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

 File Name            : namespace_default_role_permissions_value.h

 Date                 : 01/10/2024 10:37:49

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _namespace_default_role_permissions_value_h
#define _namespace_default_role_permissions_value_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   EXTENDS Clause
  -----------------*/
#include "namespace_default_role_permissions_value_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_bs.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void namespace_default_role_permissions_value__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define namespace_default_role_permissions_value__delete_rolePermissions namespace_default_role_permissions_value_bs__delete_rolePermissions
#define namespace_default_role_permissions_value__get_conv_Variant_RolePermissionTypes namespace_default_role_permissions_value_bs__get_conv_Variant_RolePermissionTypes

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void namespace_default_role_permissions_value__l_ref_get_node(
   const constants__t_Reference_i namespace_default_role_permissions_value__p_ref,
   constants__t_Node_i * const namespace_default_role_permissions_value__p_node,
   constants__t_NodeId_i * const namespace_default_role_permissions_value__p_nodeId);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void namespace_default_role_permissions_value__ref_maybe_get_DefaultRolePermissions(
   const constants__t_Reference_i namespace_default_role_permissions_value__p_ref,
   constants__t_RolePermissionTypes_i * const namespace_default_role_permissions_value__p_maybe_val_DefaultRolePermissions);

#endif
