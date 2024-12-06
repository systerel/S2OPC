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

 File Name            : namespace_default_role_permissions.h

 Date                 : 31/10/2024 15:35:56

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _namespace_default_role_permissions_h
#define _namespace_default_role_permissions_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "namespace_default_role_permissions_value.h"
#include "namespace_metadata_refs_it.h"
#include "namespace_uri.h"
#include "namespaces_uri_eval_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_bs.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void namespace_default_role_permissions__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define namespace_default_role_permissions__delete_rolePermissions namespace_default_role_permissions_value__delete_rolePermissions

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void namespace_default_role_permissions__l_ref_check_isForward_and_RefType(
   const constants__t_Reference_i namespace_default_role_permissions__p_ref,
   const constants__t_NodeId_i namespace_default_role_permissions__p_RefType_NodeId,
   t_bool * const namespace_default_role_permissions__p_bres);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void namespace_default_role_permissions__check_reference_isForward_and_RefType(
   const constants__t_Reference_i namespace_default_role_permissions__p_ref,
   const constants__t_NodeId_i namespace_default_role_permissions__p_RefType_NodeId,
   t_bool * const namespace_default_role_permissions__p_bres);
extern void namespace_default_role_permissions__namespacemetadata_and_uri_match(
   const constants__t_NamespaceUri namespace_default_role_permissions__p_namespaceUri,
   const constants__t_Node_i namespace_default_role_permissions__p_namespacemetadata_Node,
   t_bool * const namespace_default_role_permissions__p_bres,
   constants__t_RolePermissionTypes_i * const namespace_default_role_permissions__p_maybe_val_DefaultRolePermissions);

#endif
