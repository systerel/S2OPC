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

 File Name            : msg_subscription_create_monitored_item_bs.h

 Date                 : 04/08/2022 14:53:39

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_subscription_create_monitored_item_bs_h
#define _msg_subscription_create_monitored_item_bs_h

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
extern void msg_subscription_create_monitored_item_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_subscription_create_monitored_item_bs__alloc_msg_create_monitored_items_resp_results(
   const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_resp_msg,
   const t_entier4 msg_subscription_create_monitored_item_bs__p_nb_results,
   t_bool * const msg_subscription_create_monitored_item_bs__bres);
extern void msg_subscription_create_monitored_item_bs__check_msg_create_monitored_items_req_not_null(
   const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_req_msg,
   t_bool * const msg_subscription_create_monitored_item_bs__l_monitored_item_not_null);
extern void msg_subscription_create_monitored_item_bs__get_msg_create_monitored_items_req_nb_monitored_items(
   const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_req_msg,
   t_entier4 * const msg_subscription_create_monitored_item_bs__p_nb_monitored_items);
extern void msg_subscription_create_monitored_item_bs__get_msg_create_monitored_items_req_subscription(
   const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_req_msg,
   constants__t_subscription_i * const msg_subscription_create_monitored_item_bs__p_subscription);
extern void msg_subscription_create_monitored_item_bs__get_msg_create_monitored_items_req_timestamp_to_ret(
   const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_req_msg,
   constants__t_TimestampsToReturn_i * const msg_subscription_create_monitored_item_bs__p_timestampToRet);
extern void msg_subscription_create_monitored_item_bs__getall_monitored_item_req_params(
   const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_req_msg,
   const t_entier4 msg_subscription_create_monitored_item_bs__p_index,
   t_bool * const msg_subscription_create_monitored_item_bs__p_bres,
   constants_statuscodes_bs__t_StatusCode_i * const msg_subscription_create_monitored_item_bs__p_sc,
   constants__t_NodeId_i * const msg_subscription_create_monitored_item_bs__p_nid,
   constants__t_AttributeId_i * const msg_subscription_create_monitored_item_bs__p_aid,
   constants__t_monitoringMode_i * const msg_subscription_create_monitored_item_bs__p_monitMode,
   constants__t_client_handle_i * const msg_subscription_create_monitored_item_bs__p_clientHandle,
   constants__t_opcua_duration_i * const msg_subscription_create_monitored_item_bs__p_samplingItv,
   t_entier4 * const msg_subscription_create_monitored_item_bs__p_queueSize,
   constants__t_IndexRange_i * const msg_subscription_create_monitored_item_bs__p_indexRange);
extern void msg_subscription_create_monitored_item_bs__setall_msg_monitored_item_resp_params(
   const constants__t_msg_i msg_subscription_create_monitored_item_bs__p_resp_msg,
   const t_entier4 msg_subscription_create_monitored_item_bs__p_index,
   const constants_statuscodes_bs__t_StatusCode_i msg_subscription_create_monitored_item_bs__p_sc,
   const constants__t_monitoredItemId_i msg_subscription_create_monitored_item_bs__p_monitored_item_id,
   const constants__t_opcua_duration_i msg_subscription_create_monitored_item_bs__p_revSamplingItv,
   const t_entier4 msg_subscription_create_monitored_item_bs__p_revQueueSize);

#endif
