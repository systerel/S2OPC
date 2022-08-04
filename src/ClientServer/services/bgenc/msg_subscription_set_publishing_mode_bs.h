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

 File Name            : msg_subscription_set_publishing_mode_bs.h

 Date                 : 04/08/2022 14:53:41

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_subscription_set_publishing_mode_bs_h
#define _msg_subscription_set_publishing_mode_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_subscription_set_publishing_mode_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_subscription_set_publishing_mode_bs__allocate_msg_subscription_set_publishing_mode_resp_results_array(
   const constants__t_msg_i msg_subscription_set_publishing_mode_bs__p_resp_msg,
   const t_entier4 msg_subscription_set_publishing_mode_bs__l_nb_reqs,
   t_bool * const msg_subscription_set_publishing_mode_bs__bres);
extern void msg_subscription_set_publishing_mode_bs__getall_msg_set_publishing_mode_at_index(
   const constants__t_msg_i msg_subscription_set_publishing_mode_bs__p_req_msg,
   const t_entier4 msg_subscription_set_publishing_mode_bs__p_index,
   constants__t_subscription_i * const msg_subscription_set_publishing_mode_bs__p_sub_id);
extern void msg_subscription_set_publishing_mode_bs__getall_msg_subscription_set_publishing_mode_params(
   const constants__t_msg_i msg_subscription_set_publishing_mode_bs__p_req_msg,
   t_entier4 * const msg_subscription_set_publishing_mode_bs__p_nb_reqs,
   t_bool * const msg_subscription_set_publishing_mode_bs__p_pub_enabled);
extern void msg_subscription_set_publishing_mode_bs__setall_msg_subscription_set_publishing_mode_resp_at_index(
   const constants__t_msg_i msg_subscription_set_publishing_mode_bs__p_resp_msg,
   const t_entier4 msg_subscription_set_publishing_mode_bs__p_index,
   const t_bool msg_subscription_set_publishing_mode_bs__p_valid_sub);

#endif
