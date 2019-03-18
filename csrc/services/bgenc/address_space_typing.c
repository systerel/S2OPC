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

 Date                 : 29/01/2019 09:56:36

 C Translator Version : tradc Java V1.0 (14/03/2012)

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
void address_space_typing__is_included_ValueRank(
   const t_entier4 address_space_typing__p_concValueRank,
   const t_entier4 address_space_typing__p_expValueRank,
   t_bool * const address_space_typing__bres) {
   if (address_space_typing__p_expValueRank == -3) {
      *address_space_typing__bres = ((address_space_typing__p_concValueRank == -1) ||
         (address_space_typing__p_concValueRank == 1));
   }
   else if (address_space_typing__p_expValueRank == -2) {
      *address_space_typing__bres = true;
   }
   else if (address_space_typing__p_expValueRank == -1) {
      *address_space_typing__bres = (address_space_typing__p_concValueRank == -1);
   }
   else if (address_space_typing__p_expValueRank == 0) {
      *address_space_typing__bres = (address_space_typing__p_concValueRank >= 1);
   }
   else {
      *address_space_typing__bres = (address_space_typing__p_expValueRank == address_space_typing__p_concValueRank);
   }
}

