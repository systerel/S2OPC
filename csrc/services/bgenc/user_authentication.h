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

 Date                 : 29/01/2019 09:56:50

 C Translator Version : tradc Java V1.0 (14/03/2012)

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

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void user_authentication__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define user_authentication__deallocate_user user_authentication_bs__deallocate_user
#define user_authentication__get_local_user user_authentication_bs__get_local_user

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void user_authentication__allocate_user_if_authenticated(
   const constants__t_endpoint_config_idx_i user_authentication__p_endpoint_config_idx,
   const constants__t_user_token_i user_authentication__p_user_token,
   const constants__t_StatusCode_i user_authentication__p_sc_valid_user,
   constants__t_StatusCode_i * const user_authentication__p_sc_allocated_valid_user,
   constants__t_user_i * const user_authentication__p_user);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void user_authentication__allocate_valid_and_authenticated_user(
   const constants__t_user_token_i user_authentication__p_user_token,
   const constants__t_channel_config_idx_i user_authentication__p_channel_config_idx,
   const constants__t_endpoint_config_idx_i user_authentication__p_endpoint_config_idx,
   constants__t_StatusCode_i * const user_authentication__p_sc_valid_user,
   constants__t_user_i * const user_authentication__p_user);

#endif
