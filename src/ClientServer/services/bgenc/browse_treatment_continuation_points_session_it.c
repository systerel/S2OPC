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

 File Name            : browse_treatment_continuation_points_session_it.c

 Date                 : 04/08/2022 14:53:01

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "browse_treatment_continuation_points_session_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 browse_treatment_continuation_points_session_it__session_i;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void browse_treatment_continuation_points_session_it__INITIALISATION(void) {
   browse_treatment_continuation_points_session_it__session_i = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void browse_treatment_continuation_points_session_it__init_iter_session(
   t_bool * const browse_treatment_continuation_points_session_it__p_continue) {
   constants__get_card_t_session(&browse_treatment_continuation_points_session_it__session_i);
   *browse_treatment_continuation_points_session_it__p_continue = (1 <= browse_treatment_continuation_points_session_it__session_i);
}

void browse_treatment_continuation_points_session_it__continue_iter_session(
   t_bool * const browse_treatment_continuation_points_session_it__p_continue,
   constants__t_session_i * const browse_treatment_continuation_points_session_it__p_session) {
   constants__get_cast_t_session(browse_treatment_continuation_points_session_it__session_i,
      browse_treatment_continuation_points_session_it__p_session);
   browse_treatment_continuation_points_session_it__session_i = browse_treatment_continuation_points_session_it__session_i -
      1;
   *browse_treatment_continuation_points_session_it__p_continue = (1 <= browse_treatment_continuation_points_session_it__session_i);
}

