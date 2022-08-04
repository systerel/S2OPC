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

 File Name            : app_cb_call_context_bs.h

 Date                 : 04/08/2022 14:53:28

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _app_cb_call_context_bs_h
#define _app_cb_call_context_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void app_cb_call_context_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void app_cb_call_context_bs__clear_app_call_context(void);
extern void app_cb_call_context_bs__set_app_call_context_channel_config(
   const constants__t_channel_config_idx_i app_cb_call_context_bs__p_channel_config,
   const constants__t_endpoint_config_idx_i app_cb_call_context_bs__p_endpoint_config);
extern void app_cb_call_context_bs__set_app_call_context_session(
   const constants__t_user_i app_cb_call_context_bs__p_user);

#endif
