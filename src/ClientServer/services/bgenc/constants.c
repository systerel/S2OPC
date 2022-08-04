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

 File Name            : constants.c

 Date                 : 04/08/2022 14:53:06

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void constants__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void constants__read_cast_t_ReadValue(
   const t_entier4 constants__ii,
   constants__t_ReadValue_i * const constants__rvi) {
   *constants__rvi = constants__ii;
}

void constants__get_cast_t_WriteValue(
   const t_entier4 constants__ii,
   constants__t_WriteValue_i * const constants__wvi) {
   *constants__wvi = constants__ii;
}

void constants__get_cast_t_BrowseValue(
   const t_entier4 constants__p_ind,
   constants__t_BrowseValue_i * const constants__p_bvi) {
   *constants__p_bvi = constants__p_ind;
}

void constants__get_Is_Dir_Forward_Compatible(
   const constants__t_BrowseDirection_i constants__p_dir,
   const t_bool constants__p_IsForward,
   t_bool * const constants__p_dir_compat) {
   switch (constants__p_dir) {
   case constants__e_bd_forward:
      *constants__p_dir_compat = constants__p_IsForward;
      break;
   case constants__e_bd_inverse:
      *constants__p_dir_compat = (constants__p_IsForward == false);
      break;
   default:
      *constants__p_dir_compat = true;
      break;
   }
}

