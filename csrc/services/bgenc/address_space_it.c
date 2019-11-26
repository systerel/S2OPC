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

 File Name            : address_space_it.c

 Date                 : 26/11/2019 10:29:15

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 address_space_it__wreqs_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_it__INITIALISATION(void) {
   address_space_it__wreqs_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_it__init_iter_write_request(
   const t_entier4 address_space_it__nb_req,
   t_bool * const address_space_it__continue) {
   address_space_it__wreqs_i = address_space_it__nb_req;
   *address_space_it__continue = (0 < address_space_it__nb_req);
}

void address_space_it__continue_iter_write_request(
   t_bool * const address_space_it__continue,
   constants__t_WriteValue_i * const address_space_it__wvi) {
   constants__get_cast_t_WriteValue(address_space_it__wreqs_i,
      address_space_it__wvi);
   address_space_it__wreqs_i = address_space_it__wreqs_i -
      1;
   *address_space_it__continue = (0 < address_space_it__wreqs_i);
}

