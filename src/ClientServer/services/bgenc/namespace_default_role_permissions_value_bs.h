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

 File Name            : namespace_default_role_permissions_value_bs.h

 Date                 : 30/09/2024 14:56:32

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _namespace_default_role_permissions_value_bs_h
#define _namespace_default_role_permissions_value_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void namespace_default_role_permissions_value_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void namespace_default_role_permissions_value_bs__delete_rolePermissions(
   const constants__t_RolePermissionTypes_i namespace_default_role_permissions_value_bs__p_rolePermissions);
extern void namespace_default_role_permissions_value_bs__get_conv_Variant_RolePermissionTypes(
   const constants__t_Variant_i namespace_default_role_permissions_value_bs__p_variant,
   constants__t_RolePermissionTypes_i * const namespace_default_role_permissions_value_bs__p_rolePermissions);

#endif
