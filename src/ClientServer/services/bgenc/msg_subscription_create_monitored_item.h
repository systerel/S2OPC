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

 File Name            : msg_subscription_create_monitored_item.h

 Date                 : 04/08/2022 14:53:08

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_subscription_create_monitored_item_h
#define _msg_subscription_create_monitored_item_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_subscription_create_monitored_item_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 msg_subscription_create_monitored_item__nb_monitored_items;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_subscription_create_monitored_item__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define msg_subscription_create_monitored_item__alloc_msg_create_monitored_items_resp_results msg_subscription_create_monitored_item_bs__alloc_msg_create_monitored_items_resp_results
#define msg_subscription_create_monitored_item__getall_monitored_item_req_params msg_subscription_create_monitored_item_bs__getall_monitored_item_req_params
#define msg_subscription_create_monitored_item__setall_msg_monitored_item_resp_params msg_subscription_create_monitored_item_bs__setall_msg_monitored_item_resp_params

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_subscription_create_monitored_item__getall_msg_create_monitored_items_req_params(
   const constants__t_msg_i msg_subscription_create_monitored_item__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_subscription_create_monitored_item__p_sc,
   constants__t_subscription_i * const msg_subscription_create_monitored_item__p_subscription,
   constants__t_TimestampsToReturn_i * const msg_subscription_create_monitored_item__p_timestampToRet,
   t_entier4 * const msg_subscription_create_monitored_item__p_nb_monitored_items);

#endif
