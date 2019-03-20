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

 File Name            : user_authentication.c

 Date                 : 14/03/2019 10:48:21

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "user_authentication.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void user_authentication__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void user_authentication__allocate_user_if_authenticated(
   const constants__t_endpoint_config_idx_i user_authentication__p_endpoint_config_idx,
   const constants__t_user_token_i user_authentication__p_user_token,
   const constants_statuscodes_bs__t_StatusCode_i user_authentication__p_sc_valid_user,
   constants_statuscodes_bs__t_StatusCode_i * const user_authentication__p_sc_allocated_valid_user,
   constants__t_user_i * const user_authentication__p_user) {
   {
      t_bool user_authentication__l_is_user_allocated;
      
      if (user_authentication__p_sc_valid_user == constants_statuscodes_bs__e_sc_ok) {
         user_authentication_bs__allocate_authenticated_user(user_authentication__p_endpoint_config_idx,
            user_authentication__p_user_token,
            &user_authentication__l_is_user_allocated,
            user_authentication__p_user);
         if (user_authentication__l_is_user_allocated == true) {
            *user_authentication__p_sc_allocated_valid_user = user_authentication__p_sc_valid_user;
         }
         else {
            *user_authentication__p_user = constants__c_user_indet;
            *user_authentication__p_sc_allocated_valid_user = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else {
         *user_authentication__p_sc_allocated_valid_user = user_authentication__p_sc_valid_user;
         *user_authentication__p_user = constants__c_user_indet;
      }
   }
}

void user_authentication__allocate_valid_and_authenticated_user(
   const constants__t_user_token_i user_authentication__p_user_token,
   const constants__t_channel_config_idx_i user_authentication__p_channel_config_idx,
   const constants__t_endpoint_config_idx_i user_authentication__p_endpoint_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const user_authentication__p_sc_valid_user,
   constants__t_user_i * const user_authentication__p_user) {
   {
      constants__t_user_token_type_i user_authentication__l_user_token_type;
      t_bool user_authentication__l_is_supported_user_token_type;
      constants_statuscodes_bs__t_StatusCode_i user_authentication__l_sc_user_authentication;
      
      user_authentication_bs__get_user_token_type_from_token(user_authentication__p_user_token,
         &user_authentication__l_user_token_type);
      user_authentication_bs__is_user_token_supported(user_authentication__l_user_token_type,
         user_authentication__p_user_token,
         user_authentication__p_channel_config_idx,
         user_authentication__p_endpoint_config_idx,
         &user_authentication__l_is_supported_user_token_type);
      if (user_authentication__l_is_supported_user_token_type == true) {
         if (user_authentication__l_user_token_type != constants__e_userTokenType_anonymous) {
            user_authentication_bs__is_valid_user_authentication(user_authentication__p_endpoint_config_idx,
               user_authentication__l_user_token_type,
               user_authentication__p_user_token,
               &user_authentication__l_sc_user_authentication);
         }
         else {
            user_authentication__l_sc_user_authentication = constants_statuscodes_bs__e_sc_ok;
         }
         user_authentication__allocate_user_if_authenticated(user_authentication__p_endpoint_config_idx,
            user_authentication__p_user_token,
            user_authentication__l_sc_user_authentication,
            user_authentication__p_sc_valid_user,
            user_authentication__p_user);
      }
      else {
         *user_authentication__p_user = constants__c_user_indet;
         *user_authentication__p_sc_valid_user = constants_statuscodes_bs__e_sc_bad_identity_token_invalid;
      }
   }
}

