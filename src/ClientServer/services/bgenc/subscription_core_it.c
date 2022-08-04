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

 File Name            : subscription_core_it.c

 Date                 : 04/08/2022 14:53:21

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "subscription_core_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 subscription_core_it__subscription_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_core_it__INITIALISATION(void) {
   subscription_core_it__subscription_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_core_it__init_iter_subscription(
   t_bool * const subscription_core_it__p_continue) {
   constants__get_card_t_subscription(&subscription_core_it__subscription_i);
   *subscription_core_it__p_continue = (1 <= subscription_core_it__subscription_i);
}

void subscription_core_it__continue_iter_subscription(
   t_bool * const subscription_core_it__p_continue,
   constants__t_subscription_i * const subscription_core_it__p_subscription) {
   constants__get_cast_t_subscription(subscription_core_it__subscription_i,
      subscription_core_it__p_subscription);
   subscription_core_it__subscription_i = subscription_core_it__subscription_i -
      1;
   *subscription_core_it__p_continue = (1 <= subscription_core_it__subscription_i);
}

