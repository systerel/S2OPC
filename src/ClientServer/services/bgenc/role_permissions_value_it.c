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

 File Name            : role_permissions_value_it.c

 Date                 : 30/10/2024 16:34:23

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "role_permissions_value_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 role_permissions_value_it__currentRolePermissionIdx_i;
t_entier4 role_permissions_value_it__nb_rolePermissions_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void role_permissions_value_it__INITIALISATION(void) {
   role_permissions_value_it__currentRolePermissionIdx_i = 0;
   role_permissions_value_it__nb_rolePermissions_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void role_permissions_value_it__init_iter_rolePermissions(
   const t_entier4 role_permissions_value_it__p_nb_rolePermissions,
   t_bool * const role_permissions_value_it__p_continue) {
   role_permissions_value_it__nb_rolePermissions_i = role_permissions_value_it__p_nb_rolePermissions;
   role_permissions_value_it__currentRolePermissionIdx_i = 0;
   *role_permissions_value_it__p_continue = (0 < role_permissions_value_it__p_nb_rolePermissions);
}

void role_permissions_value_it__continue_iter_rolePermissions(
   t_bool * const role_permissions_value_it__p_continue,
   t_entier4 * const role_permissions_value_it__p_rolePermissionIdx) {
   role_permissions_value_it__currentRolePermissionIdx_i = role_permissions_value_it__currentRolePermissionIdx_i +
      1;
   *role_permissions_value_it__p_rolePermissionIdx = role_permissions_value_it__currentRolePermissionIdx_i;
   *role_permissions_value_it__p_continue = (role_permissions_value_it__currentRolePermissionIdx_i < role_permissions_value_it__nb_rolePermissions_i);
}

