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

 File Name            : address_space_namespaces_metadata.h

 Date                 : 29/10/2024 09:08:38

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _address_space_namespaces_metadata_h
#define _address_space_namespaces_metadata_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "default_role_permissions_array_bs.h"
#include "namespace_array_bs.h"
#include "namespace_array_it.h"
#include "namespaces_default_role_permissions.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_bs.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space_namespaces_metadata__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define address_space_namespaces_metadata__address_space_default_role_permissions_array_bs_UNINITIALISATION default_role_permissions_array_bs__address_space_default_role_permissions_array_bs_UNINITIALISATION
#define address_space_namespaces_metadata__delete_rolePermissions namespaces_default_role_permissions__delete_rolePermissions
#define address_space_namespaces_metadata__get_DefaultRolePermissions_at_idx default_role_permissions_array_bs__get_DefaultRolePermissions_at_idx
#define address_space_namespaces_metadata__has_DefaultRolePermissions_at_idx default_role_permissions_array_bs__has_DefaultRolePermissions_at_idx

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void address_space_namespaces_metadata__l_fill_default_role_permissions(
   const constants__t_Variant_i address_space_namespaces_metadata__p_val);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_namespaces_metadata__may_initialize_default_role_permissions(void);

#endif
