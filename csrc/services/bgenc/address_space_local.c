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

 File Name            : address_space_local.c

 Date                 : 19/04/2019 16:19:30

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space_local.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 address_space_local__local_service_treatment_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space_local__INITIALISATION(void) {
   address_space_local__local_service_treatment_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space_local__set_local_service_treatment(void) {
   address_space_local__local_service_treatment_i = address_space_local__local_service_treatment_i +
      1;
}

void address_space_local__unset_local_service_treatment(void) {
   if (address_space_local__local_service_treatment_i != 0) {
      address_space_local__local_service_treatment_i = address_space_local__local_service_treatment_i -
         1;
   }
}

void address_space_local__is_local_service_treatment(
   t_bool * const address_space_local__bres) {
   *address_space_local__bres = (address_space_local__local_service_treatment_i != 0);
}

