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

 File Name            : namespaces_default_role_permissions.h

 Date                 : 31/10/2024 15:35:57

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _namespaces_default_role_permissions_h
#define _namespaces_default_role_permissions_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "namespace_default_role_permissions.h"
#include "namespaces_refs_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_bs.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void namespaces_default_role_permissions__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define namespaces_default_role_permissions__delete_rolePermissions namespace_default_role_permissions__delete_rolePermissions

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void namespaces_default_role_permissions__l_node_check_NodeClass_and_TypeDef(
   const constants__t_Node_i namespaces_default_role_permissions__p_node,
   t_bool * const namespaces_default_role_permissions__p_bres);
extern void namespaces_default_role_permissions__l_ref_get_node(
   const constants__t_Reference_i namespaces_default_role_permissions__p_ref,
   constants__t_Node_i * const namespaces_default_role_permissions__p_node);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void namespaces_default_role_permissions__get_DefaultRolePermissions(
   const constants__t_NamespaceUri namespaces_default_role_permissions__p_namespaceUri,
   constants__t_RolePermissionTypes_i * const namespaces_default_role_permissions__p_val_DefaultRolePermissions);

#endif
