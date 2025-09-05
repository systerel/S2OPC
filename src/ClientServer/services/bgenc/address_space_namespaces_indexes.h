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

 File Name            : address_space_namespaces_indexes.h

 Date                 : 05/09/2025 13:22:02

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _address_space_namespaces_indexes_h
#define _address_space_namespaces_indexes_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "address_space_namespaces_indexes_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space_bs.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 address_space_namespaces_indexes__a_nsIdxMax_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space_namespaces_indexes__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_namespaces_indexes__has_NamespaceIndex_index(
   const constants__t_NamespaceIdx address_space_namespaces_indexes__p_idx,
   t_bool * const address_space_namespaces_indexes__bres);
extern void address_space_namespaces_indexes__update_max_namespace_idx(void);

#endif
