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

 File Name            : address_space_namespaces.c

 Date                 : 29/10/2024 09:08:38

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space_namespaces.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_namespaces__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_namespaces__has_Namespace_DefaultRolePermissions(
   const constants__t_NamespaceIdx address_space_namespaces__p_idx,
   t_bool * const address_space_namespaces__bres) {
   address_space_namespaces_metadata__may_initialize_default_role_permissions();
   address_space_namespaces_metadata__has_DefaultRolePermissions_at_idx(address_space_namespaces__p_idx,
      address_space_namespaces__bres);
}

void address_space_namespaces__get_Namespace_DefaultRolePermissions(
   const constants__t_NamespaceIdx address_space_namespaces__p_idx,
   constants__t_RolePermissionTypes_i * const address_space_namespaces__p_DefaultRolePermissions) {
   address_space_namespaces_metadata__get_DefaultRolePermissions_at_idx(address_space_namespaces__p_idx,
      address_space_namespaces__p_DefaultRolePermissions);
}

