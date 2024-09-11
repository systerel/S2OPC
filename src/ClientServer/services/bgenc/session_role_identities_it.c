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

 File Name            : session_role_identities_it.c

 Date                 : 05/08/2024 13:22:12

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_role_identities_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 session_role_identities_it__currentIdentityIdx_i;
t_entier4 session_role_identities_it__nb_identities_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_role_identities_it__INITIALISATION(void) {
   session_role_identities_it__currentIdentityIdx_i = 0;
   session_role_identities_it__nb_identities_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_role_identities_it__init_iter_identities(
   const t_entier4 session_role_identities_it__p_nb_identities,
   t_bool * const session_role_identities_it__p_continue) {
   session_role_identities_it__nb_identities_i = session_role_identities_it__p_nb_identities;
   session_role_identities_it__currentIdentityIdx_i = 0;
   *session_role_identities_it__p_continue = (0 < session_role_identities_it__p_nb_identities);
}

void session_role_identities_it__continue_iter_identities(
   t_bool * const session_role_identities_it__p_continue,
   t_entier4 * const session_role_identities_it__p_identityIdx) {
   session_role_identities_it__currentIdentityIdx_i = session_role_identities_it__currentIdentityIdx_i +
      1;
   *session_role_identities_it__p_identityIdx = session_role_identities_it__currentIdentityIdx_i;
   *session_role_identities_it__p_continue = (session_role_identities_it__currentIdentityIdx_i < session_role_identities_it__nb_identities_i);
}

