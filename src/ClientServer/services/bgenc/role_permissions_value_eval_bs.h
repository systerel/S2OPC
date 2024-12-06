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

 File Name            : role_permissions_value_eval_bs.h

 Date                 : 30/10/2024 16:34:41

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _role_permissions_value_eval_bs_h
#define _role_permissions_value_eval_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_bs.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void role_permissions_value_eval_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void role_permissions_value_eval_bs__add_user_role_permissions(
   const constants__t_NodeId_i role_permissions_value_eval_bs__p_role,
   const constants__t_PermissionType_i role_permissions_value_eval_bs__p_permissions);
extern void role_permissions_value_eval_bs__get_merged_user_role_permissions(
   const constants__t_NodeId_i role_permissions_value_eval_bs__p_role,
   constants__t_PermissionType_i * const role_permissions_value_eval_bs__p_permissions);
extern void role_permissions_value_eval_bs__init_user_role_permissions(
   const constants__t_NodeId_i role_permissions_value_eval_bs__p_role);
extern void role_permissions_value_eval_bs__read_rolePermissions_permissions(
   const constants__t_RolePermissionTypes_i role_permissions_value_eval_bs__p_rolePermissions,
   const t_entier4 role_permissions_value_eval_bs__p_idx,
   constants__t_PermissionType_i * const role_permissions_value_eval_bs__p_permissions);
extern void role_permissions_value_eval_bs__read_rolePermissions_roleId(
   const constants__t_RolePermissionTypes_i role_permissions_value_eval_bs__p_rolePermissions,
   const t_entier4 role_permissions_value_eval_bs__p_idx,
   constants__t_NodeId_i * const role_permissions_value_eval_bs__p_roleId);
extern void role_permissions_value_eval_bs__read_variant_rolePermissions(
   const constants__t_RolePermissionTypes_i role_permissions_value_eval_bs__p_rolePermissions,
   t_entier4 * const role_permissions_value_eval_bs__p_nbr_of_rolePermissions);

#endif
