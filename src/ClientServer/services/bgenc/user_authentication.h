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

 File Name            : user_authentication.h

 Date                 : 04/08/2022 14:53:26

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _user_authentication_h
#define _user_authentication_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "user_authentication_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void user_authentication__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define user_authentication__deallocate_user user_authentication_bs__deallocate_user
#define user_authentication__get_local_user user_authentication_bs__get_local_user
#define user_authentication__has_user_token_policy_available user_authentication_bs__has_user_token_policy_available

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void user_authentication__allocate_user_if_authenticated(
   const constants__t_endpoint_config_idx_i user_authentication__p_endpoint_config_idx,
   const constants__t_user_token_i user_authentication__p_user_token,
   const constants_statuscodes_bs__t_StatusCode_i user_authentication__p_sc_valid_user,
   constants_statuscodes_bs__t_StatusCode_i * const user_authentication__p_sc_allocated_valid_user,
   constants__t_user_i * const user_authentication__p_user);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void user_authentication__allocate_valid_and_authenticated_user(
   const constants__t_user_token_i user_authentication__p_user_token,
   const constants__t_Nonce_i user_authentication__p_server_nonce,
   const constants__t_channel_config_idx_i user_authentication__p_channel_config_idx,
   const constants__t_endpoint_config_idx_i user_authentication__p_endpoint_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const user_authentication__p_sc_valid_user,
   constants__t_user_i * const user_authentication__p_user);
extern void user_authentication__may_encrypt_user_token(
   const constants__t_channel_config_idx_i user_authentication__p_channel_config_idx,
   const constants__t_byte_buffer_i user_authentication__p_user_server_cert,
   const constants__t_Nonce_i user_authentication__p_server_nonce,
   const constants__t_SecurityPolicy user_authentication__p_user_secu_policy,
   const constants__t_user_token_i user_authentication__p_user_token,
   t_bool * const user_authentication__p_valid,
   constants__t_user_token_i * const user_authentication__p_user_token_encrypted);

#endif
