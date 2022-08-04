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

 File Name            : msg_subscription_create_bs.h

 Date                 : 04/08/2022 14:53:39

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_subscription_create_bs_h
#define _msg_subscription_create_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_subscription_create_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_subscription_create_bs__get_msg_create_subscription_req_params(
   const constants__t_msg_i msg_subscription_create_bs__p_req_msg,
   constants__t_opcua_duration_i * const msg_subscription_create_bs__reqPublishInterval,
   t_entier4 * const msg_subscription_create_bs__reqLifetimeCount,
   t_entier4 * const msg_subscription_create_bs__reqMaxKeepAlive,
   t_entier4 * const msg_subscription_create_bs__maxNotificationsPerPublish,
   t_bool * const msg_subscription_create_bs__publishEnabled);
extern void msg_subscription_create_bs__get_msg_modify_subscription_req_params(
   const constants__t_msg_i msg_subscription_create_bs__p_req_msg,
   constants__t_subscription_i * const msg_subscription_create_bs__subscription,
   constants__t_opcua_duration_i * const msg_subscription_create_bs__reqPublishInterval,
   t_entier4 * const msg_subscription_create_bs__reqLifetimeCount,
   t_entier4 * const msg_subscription_create_bs__reqMaxKeepAlive,
   t_entier4 * const msg_subscription_create_bs__maxNotificationsPerPublish);
extern void msg_subscription_create_bs__set_msg_create_subscription_resp_params(
   const constants__t_msg_i msg_subscription_create_bs__p_resp_msg,
   const constants__t_subscription_i msg_subscription_create_bs__p_subscription,
   const constants__t_opcua_duration_i msg_subscription_create_bs__p_revisedPublishInterval,
   const t_entier4 msg_subscription_create_bs__p_revisedLifetimeCount,
   const t_entier4 msg_subscription_create_bs__p_revisedMaxKeepAlive);
extern void msg_subscription_create_bs__set_msg_modify_subscription_resp_params(
   const constants__t_msg_i msg_subscription_create_bs__p_resp_msg,
   const constants__t_opcua_duration_i msg_subscription_create_bs__p_revisedPublishInterval,
   const t_entier4 msg_subscription_create_bs__p_revisedLifetimeCount,
   const t_entier4 msg_subscription_create_bs__p_revisedMaxKeepAlive);

#endif
