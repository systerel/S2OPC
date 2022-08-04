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

 File Name            : msg_subscription_publish_bs.h

 Date                 : 04/08/2022 14:53:41

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _msg_subscription_publish_bs_h
#define _msg_subscription_publish_bs_h

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
#include "request_handle_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_subscription_publish_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_subscription_publish_bs__alloc_notification_message_items(
   const constants__t_msg_i msg_subscription_publish_bs__p_publish_resp_msg,
   const t_entier4 msg_subscription_publish_bs__p_nb_monitored_item_notifications,
   t_bool * const msg_subscription_publish_bs__bres,
   constants__t_notif_msg_i * const msg_subscription_publish_bs__p_notifMsg);
extern void msg_subscription_publish_bs__generate_internal_send_publish_response_event(
   const constants__t_session_i msg_subscription_publish_bs__p_session,
   const constants__t_msg_i msg_subscription_publish_bs__p_publish_resp_msg,
   const constants__t_server_request_handle_i msg_subscription_publish_bs__p_req_handle,
   const constants__t_request_context_i msg_subscription_publish_bs__p_req_context,
   const constants_statuscodes_bs__t_StatusCode_i msg_subscription_publish_bs__p_statusCode);
extern void msg_subscription_publish_bs__get_notification_message_no_items(
   const constants__t_msg_i msg_subscription_publish_bs__p_publish_resp_msg,
   constants__t_notif_msg_i * const msg_subscription_publish_bs__p_notifMsg);
extern void msg_subscription_publish_bs__set_msg_publish_resp_notificationMsg(
   const constants__t_msg_i msg_subscription_publish_bs__p_resp_msg,
   const t_bool msg_subscription_publish_bs__p_moreNotifs);
extern void msg_subscription_publish_bs__set_msg_publish_resp_subscription(
   const constants__t_msg_i msg_subscription_publish_bs__p_resp_msg,
   const constants__t_subscription_i msg_subscription_publish_bs__p_subscription);
extern void msg_subscription_publish_bs__set_notification_message_sequence_number(
   const constants__t_notif_msg_i msg_subscription_publish_bs__p_notifMsg,
   const constants__t_sub_seq_num_i msg_subscription_publish_bs__p_seq_num);
extern void msg_subscription_publish_bs__set_publish_response_msg(
   const constants__t_msg_i msg_subscription_publish_bs__p_publish_resp_msg);
extern void msg_subscription_publish_bs__setall_notification_msg_monitored_item_notif(
   const constants__t_notif_msg_i msg_subscription_publish_bs__p_notifMsg,
   const t_entier4 msg_subscription_publish_bs__p_index,
   const constants__t_monitoredItemId_i msg_subscription_publish_bs__p_monitored_item_id,
   const constants__t_client_handle_i msg_subscription_publish_bs__p_clientHandle,
   const constants__t_WriteValuePointer_i msg_subscription_publish_bs__p_wv_pointer);

#endif
