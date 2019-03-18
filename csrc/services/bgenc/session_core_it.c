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

 File Name            : session_core_it.c

 Date                 : 29/01/2019 09:56:46

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_core_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 session_core_it__session_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_it__INITIALISATION(void) {
   session_core_it__session_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_it__init_iter_session(
   t_bool * const session_core_it__p_continue) {
   constants__get_card_t_session(&session_core_it__session_i);
   *session_core_it__p_continue = (1 <= session_core_it__session_i);
}

void session_core_it__continue_iter_session(
   t_bool * const session_core_it__p_continue,
   constants__t_session_i * const session_core_it__p_session) {
   constants__get_cast_t_session(session_core_it__session_i,
      session_core_it__p_session);
   session_core_it__session_i = session_core_it__session_i -
      1;
   *session_core_it__p_continue = (1 <= session_core_it__session_i);
}

