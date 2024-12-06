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

 File Name            : address_space_namespaces.h

 Date                 : 29/10/2024 09:08:38

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _address_space_namespaces_h
#define _address_space_namespaces_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "address_space_namespaces_metadata.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_bs.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space_namespaces__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define address_space_namespaces__address_space_default_role_permissions_array_bs_UNINITIALISATION address_space_namespaces_metadata__address_space_default_role_permissions_array_bs_UNINITIALISATION
#define address_space_namespaces__delete_rolePermissions address_space_namespaces_metadata__delete_rolePermissions

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_namespaces__get_Namespace_DefaultRolePermissions(
   const constants__t_NamespaceIdx address_space_namespaces__p_idx,
   constants__t_RolePermissionTypes_i * const address_space_namespaces__p_DefaultRolePermissions);
extern void address_space_namespaces__has_Namespace_DefaultRolePermissions(
   const constants__t_NamespaceIdx address_space_namespaces__p_idx,
   t_bool * const address_space_namespaces__bres);

#endif
