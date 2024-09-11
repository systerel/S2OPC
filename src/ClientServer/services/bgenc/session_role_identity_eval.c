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

 File Name            : session_role_identity_eval.c

 Date                 : 09/08/2024 09:19:53

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_role_identity_eval.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_role_identity_eval__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_role_identity_eval__user_and_identity_match(
   const constants__t_user_i session_role_identity_eval__p_user,
   const constants__t_Identity_i session_role_identity_eval__p_identity,
   t_bool * const session_role_identity_eval__p_bres) {
   {
      constants__t_CriteriaType_i session_role_identity_eval__l_criteria_type;
      constants__t_Criteria_i session_role_identity_eval__l_username;
      t_bool session_role_identity_eval__l_is_anonymous;
      t_bool session_role_identity_eval__l_b_is_user_username;
      
      *session_role_identity_eval__p_bres = false;
      session_role_identity_bs__read_identity_criteriaType(session_role_identity_eval__p_identity,
         &session_role_identity_eval__l_criteria_type);
      if (session_role_identity_eval__l_criteria_type == constants__e_CriteriaType_anonymous) {
         *session_role_identity_eval__p_bres = true;
      }
      else {
         user_bs__is_anonymous(session_role_identity_eval__p_user,
            &session_role_identity_eval__l_is_anonymous);
         if (session_role_identity_eval__l_is_anonymous == false) {
            if (session_role_identity_eval__l_criteria_type == constants__e_CriteriaType_username) {
               user_bs__is_username(session_role_identity_eval__p_user,
                  &session_role_identity_eval__l_b_is_user_username);
               if (session_role_identity_eval__l_b_is_user_username == true) {
                  session_role_identity_bs__read_identity_criteria(session_role_identity_eval__p_identity,
                     &session_role_identity_eval__l_username);
                  user_bs__are_username_equal(session_role_identity_eval__p_user,
                     session_role_identity_eval__l_username,
                     session_role_identity_eval__p_bres);
               }
            }
            else if (session_role_identity_eval__l_criteria_type == constants__e_CriteriaType_authenticatedUser) {
               *session_role_identity_eval__p_bres = true;
            }
         }
      }
   }
}

