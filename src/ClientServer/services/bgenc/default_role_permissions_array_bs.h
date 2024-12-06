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

 File Name            : default_role_permissions_array_bs.h

 Date                 : 02/12/2024 16:45:16

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _default_role_permissions_array_bs_h
#define _default_role_permissions_array_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "namespace_array_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void default_role_permissions_array_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void default_role_permissions_array_bs__add_DefaultRolePermissions_at_idx(
   const constants__t_NamespaceUri default_role_permissions_array_bs__p_namespaceUri,
   const constants__t_NamespaceIdx default_role_permissions_array_bs__p_idx,
   const constants__t_RolePermissionTypes_i default_role_permissions_array_bs__p_DefaultRolePermissions);
extern void default_role_permissions_array_bs__address_space_default_role_permissions_array_bs_UNINITIALISATION(void);
extern void default_role_permissions_array_bs__get_DefaultRolePermissions_at_idx(
   const constants__t_NamespaceIdx default_role_permissions_array_bs__p_idx,
   constants__t_RolePermissionTypes_i * const default_role_permissions_array_bs__p_DefaultRolePermissions);
extern void default_role_permissions_array_bs__has_DefaultRolePermissions_at_idx(
   const constants__t_NamespaceIdx default_role_permissions_array_bs__p_idx,
   t_bool * const default_role_permissions_array_bs__bres);
extern void default_role_permissions_array_bs__init_array_of_DefaultRolePermissions(
   const t_entier4 default_role_permissions_array_bs__p_nb_namespaces);
extern void default_role_permissions_array_bs__is_default_role_permissions_initialized(
   t_bool * const default_role_permissions_array_bs__p_res);

#endif
