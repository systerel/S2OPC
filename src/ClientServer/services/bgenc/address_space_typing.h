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

 File Name            : address_space_typing.h

 Date                 : 04/08/2022 14:52:59

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _address_space_typing_h
#define _address_space_typing_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "address_space_typing_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space_typing__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define address_space_typing__check_object_has_method address_space_typing_bs__check_object_has_method
#define address_space_typing__is_transitive_subtype address_space_typing_bs__is_transitive_subtype
#define address_space_typing__is_valid_ReferenceTypeId address_space_typing_bs__is_valid_ReferenceTypeId

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_typing__is_included_ValueRank(
   const t_entier4 address_space_typing__p_concValueRank,
   const t_entier4 address_space_typing__p_expValueRank,
   t_bool * const address_space_typing__bres);
extern void address_space_typing__is_transitive_subtype_or_compatible_simple_type_or_enumeration(
   const t_bool address_space_typing__p_res_is_transitive_type,
   const constants__t_NodeId_i address_space_typing__p_actual_value_type,
   const constants__t_NodeId_i address_space_typing__p_exp_data_type,
   t_bool * const address_space_typing__bres);

#endif
