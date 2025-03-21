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

 File Name            : address_space_typing.c

 Date                 : 18/03/2025 08:06:54

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space_typing.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_typing__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_typing__is_transitive_subtype_or_compatible_simple_type_or_enumeration(
   const t_bool address_space_typing__p_res_is_transitive_type,
   const constants__t_NodeId_i address_space_typing__p_actual_value_type,
   const constants__t_NodeId_i address_space_typing__p_exp_data_type,
   t_bool * const address_space_typing__bres) {
   *address_space_typing__bres = address_space_typing__p_res_is_transitive_type;
   if (*address_space_typing__bres == false) {
      address_space_typing_bs__is_compatible_simple_type_or_enumeration(address_space_typing__p_actual_value_type,
         address_space_typing__p_exp_data_type,
         address_space_typing__bres);
   }
}

