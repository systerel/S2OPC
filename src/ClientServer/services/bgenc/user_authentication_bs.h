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

 File Name            : user_authentication_bs.h

 Date                 : 04/08/2022 14:53:48

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _user_authentication_bs_h
#define _user_authentication_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void user_authentication_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void user_authentication_bs__allocate_authenticated_user(
   const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
   const constants__t_user_token_i user_authentication_bs__p_user_token,
   t_bool * const user_authentication_bs__p_is_allocated_user,
   constants__t_user_i * const user_authentication_bs__p_user);
extern void user_authentication_bs__deallocate_user(
   const constants__t_user_i user_authentication_bs__p_user);
extern void user_authentication_bs__decrypt_user_token(
   const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
   const constants__t_Nonce_i user_authentication_bs__p_server_nonce,
   const constants__t_SecurityPolicy user_authentication_bs__p_user_secu_policy,
   const constants__t_user_token_type_i user_authentication_bs__p_token_type,
   const constants__t_user_token_i user_authentication_bs__p_user_token,
   t_bool * const user_authentication_bs__p_sc_valid_user_token,
   constants__t_user_token_i * const user_authentication_bs__p_user_token_decrypted);
extern void user_authentication_bs__encrypt_user_token(
   const constants__t_channel_config_idx_i user_authentication_bs__p_channel_config_idx,
   const constants__t_byte_buffer_i user_authentication_bs__p_server_cert,
   const constants__t_Nonce_i user_authentication_bs__p_server_nonce,
   const constants__t_SecurityPolicy user_authentication_bs__p_user_secu_policy,
   const constants__t_user_token_type_i user_authentication_bs__p_token_type,
   const constants__t_user_token_i user_authentication_bs__p_user_token,
   t_bool * const user_authentication_bs__p_valid,
   constants__t_user_token_i * const user_authentication_bs__p_user_token_encrypted);
extern void user_authentication_bs__get_local_user(
   const constants__t_endpoint_config_idx_i user_authentication_bs__endpoint_config_idx,
   constants__t_user_i * const user_authentication_bs__p_user);
extern void user_authentication_bs__get_user_token_type_from_token(
   const constants__t_user_token_i user_authentication_bs__p_user_token,
   constants__t_user_token_type_i * const user_authentication_bs__p_user_token_type);
extern void user_authentication_bs__has_user_token_policy_available(
   const constants__t_channel_config_idx_i user_authentication_bs__p_channel_config_idx,
   const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
   t_bool * const user_authentication_bs__p_user_token_policy_available);
extern void user_authentication_bs__is_user_token_supported(
   const constants__t_user_token_type_i user_authentication_bs__p_user_token_type,
   const constants__t_user_token_i user_authentication_bs__p_user_token,
   const constants__t_channel_config_idx_i user_authentication_bs__p_channel_config_idx,
   const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
   t_bool * const user_authentication_bs__p_supported_user_token_type,
   constants__t_SecurityPolicy * const user_authentication_bs__p_user_security_policy);
extern void user_authentication_bs__is_valid_user_authentication(
   const constants__t_endpoint_config_idx_i user_authentication_bs__p_endpoint_config_idx,
   const constants__t_user_token_type_i user_authentication_bs__p_token_type,
   const constants__t_user_token_i user_authentication_bs__p_user_token,
   constants_statuscodes_bs__t_StatusCode_i * const user_authentication_bs__p_sc_valid_user);
extern void user_authentication_bs__shallow_copy_user_token(
   const constants__t_user_token_type_i user_authentication_bs__p_token_type,
   const constants__t_user_token_i user_authentication_bs__p_user_token,
   t_bool * const user_authentication_bs__p_valid,
   constants__t_user_token_i * const user_authentication_bs__p_user_token_copy);

#endif
