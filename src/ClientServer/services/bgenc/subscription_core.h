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

 File Name            : subscription_core.h

 Date                 : 04/08/2022 14:53:21

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _subscription_core_h
#define _subscription_core_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "monitored_item_notification_queue_bs.h"
#include "monitored_item_pointer_bs.h"
#include "monitored_item_queue_bs.h"
#include "msg_subscription_publish_bs.h"
#include "notification_republish_queue_bs.h"
#include "publish_request_queue_bs.h"
#include "subscription_core_1.h"
#include "subscription_core_bs.h"
#include "subscription_core_it.h"

/*-----------------
   EXTENDS Clause
  -----------------*/
#include "monitored_item_queue_it_bs.h"
#include "notification_republish_queue_it_bs.h"

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
extern void subscription_core__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define subscription_core__clear_iter_monitored_item monitored_item_queue_it_bs__clear_iter_monitored_item
#define subscription_core__clear_notif_republish_iterator notification_republish_queue_it_bs__clear_notif_republish_iterator
#define subscription_core__compute_create_subscription_revised_params subscription_core_bs__compute_create_subscription_revised_params
#define subscription_core__continue_iter_monitored_item monitored_item_queue_it_bs__continue_iter_monitored_item
#define subscription_core__continue_iter_notif_republish notification_republish_queue_it_bs__continue_iter_notif_republish
#define subscription_core__generate_internal_send_publish_response_event msg_subscription_publish_bs__generate_internal_send_publish_response_event
#define subscription_core__get_available_republish notification_republish_queue_it_bs__get_available_republish
#define subscription_core__get_nodeToMonitoredItemQueue subscription_core_bs__get_nodeToMonitoredItemQueue
#define subscription_core__get_republish_notif_from_queue notification_republish_queue_bs__get_republish_notif_from_queue
#define subscription_core__get_subscription_notifRepublishQueue subscription_core_1__get_subscription_notifRepublishQueue
#define subscription_core__getall_monitoredItemPointer monitored_item_pointer_bs__getall_monitoredItemPointer
#define subscription_core__getall_session subscription_core_1__getall_session
#define subscription_core__getall_subscription subscription_core_1__getall_subscription
#define subscription_core__init_iter_monitored_item monitored_item_queue_it_bs__init_iter_monitored_item
#define subscription_core__init_iter_notif_republish notification_republish_queue_it_bs__init_iter_notif_republish
#define subscription_core__is_notification_triggered monitored_item_pointer_bs__is_notification_triggered
#define subscription_core__is_valid_subscription subscription_core_1__is_valid_subscription
#define subscription_core__set_msg_publish_resp_notificationMsg msg_subscription_publish_bs__set_msg_publish_resp_notificationMsg
#define subscription_core__set_msg_publish_resp_subscription msg_subscription_publish_bs__set_msg_publish_resp_subscription
#define subscription_core__set_subscription_PublishingEnabled subscription_core_1__set_subscription_PublishingEnabled

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void subscription_core__fill_notification_message(
   const constants__t_notificationQueue_i subscription_core__p_queue,
   const constants__t_notif_msg_i subscription_core__p_notif_msg,
   const t_entier4 subscription_core__nb_notif_to_dequeue);
extern void subscription_core__get_fresh_subscription(
   t_bool * const subscription_core__bres,
   constants__t_subscription_i * const subscription_core__p_subscription);
extern void subscription_core__local_close_subscription(
   const constants__t_subscription_i subscription_core__p_subscription);
extern void subscription_core__pop_invalid_and_check_valid_publishReqQueued(
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__p_validPubReqQueued);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void subscription_core__close_subscription(
   const constants__t_subscription_i subscription_core__p_subscription);
extern void subscription_core__compute_create_monitored_item_revised_params(
   const t_entier4 subscription_core__p_reqQueueSize,
   constants__t_opcua_duration_i * const subscription_core__revisedSamplingItv,
   t_entier4 * const subscription_core__revisedQueueSize);
extern void subscription_core__create_monitored_item(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_NodeId_i subscription_core__p_nid,
   const constants__t_AttributeId_i subscription_core__p_aid,
   const constants__t_IndexRange_i subscription_core__p_indexRange,
   const constants__t_Variant_i subscription_core__p_value,
   const constants__t_RawStatusCode subscription_core__p_valueSc,
   const constants__t_Timestamp subscription_core__p_val_ts_src,
   const constants__t_Timestamp subscription_core__p_val_ts_srv,
   const constants__t_TimestampsToReturn_i subscription_core__p_timestampToReturn,
   const constants__t_monitoringMode_i subscription_core__p_monitoringMode,
   const constants__t_client_handle_i subscription_core__p_clientHandle,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__StatusCode_service,
   constants__t_monitoredItemPointer_i * const subscription_core__monitoredItemPointer,
   constants__t_monitoredItemId_i * const subscription_core__monitoredItemId);
extern void subscription_core__create_subscription(
   const constants__t_session_i subscription_core__p_session,
   const constants__t_opcua_duration_i subscription_core__p_revPublishInterval,
   const t_entier4 subscription_core__p_revLifetimeCount,
   const t_entier4 subscription_core__p_revMaxKeepAlive,
   const t_entier4 subscription_core__p_maxNotificationsPerPublish,
   const t_bool subscription_core__p_publishEnabled,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__StatusCode_service,
   constants__t_subscription_i * const subscription_core__subscription);
extern void subscription_core__empty_session_publish_requests(
   const constants__t_subscription_i subscription_core__p_subscription);
extern void subscription_core__is_valid_subscription_on_session(
   const constants__t_session_i subscription_core__p_session,
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__is_valid);
extern void subscription_core__modify_subscription(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_opcua_duration_i subscription_core__p_revPublishInterval,
   const t_entier4 subscription_core__p_revLifetimeCount,
   const t_entier4 subscription_core__p_revMaxKeepAlive,
   const t_entier4 subscription_core__p_revMaxNotifPerPublish);
extern void subscription_core__receive_publish_request(
   const constants__t_session_i subscription_core__p_session,
   const constants__t_timeref_i subscription_core__p_req_exp_time,
   const constants__t_server_request_handle_i subscription_core__p_req_handle,
   const constants__t_request_context_i subscription_core__p_req_ctx,
   const constants__t_msg_i subscription_core__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__StatusCode_service,
   t_bool * const subscription_core__async_resp_msg,
   constants__t_subscription_i * const subscription_core__subscription,
   t_bool * const subscription_core__moreNotifs);
extern void subscription_core__server_subscription_add_notification(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   const constants__t_TimestampsToReturn_i subscription_core__p_timestampToReturn,
   const constants__t_WriteValuePointer_i subscription_core__p_writeValuePointer);
extern void subscription_core__server_subscription_core_check_valid_publish_req_queue(
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__p_validPublishingReqQueued);
extern void subscription_core__server_subscription_core_publish_timeout(
   const constants__t_subscription_i subscription_core__p_subscription,
   const t_bool subscription_core__p_validPublishReqQueued,
   t_bool * const subscription_core__p_msg_to_send,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_msg_sc,
   constants__t_session_i * const subscription_core__p_session,
   constants__t_msg_i * const subscription_core__p_publish_resp_msg,
   constants__t_server_request_handle_i * const subscription_core__p_req_handle,
   constants__t_request_context_i * const subscription_core__p_req_context,
   t_bool * const subscription_core__p_moreNotifs);
extern void subscription_core__server_subscription_core_publish_timeout_check_lifetime(
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__p_close_sub,
   t_bool * const subscription_core__p_msg_to_send,
   constants__t_session_i * const subscription_core__p_session,
   constants__t_msg_i * const subscription_core__p_publish_resp_msg,
   constants__t_server_request_handle_i * const subscription_core__p_req_handle,
   constants__t_request_context_i * const subscription_core__p_req_context,
   t_bool * const subscription_core__p_validPubReqQueued);
extern void subscription_core__server_subscription_core_publish_timeout_return_moreNotifs(
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__p_msg_to_send,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_msg_sc,
   constants__t_session_i * const subscription_core__p_session,
   constants__t_msg_i * const subscription_core__p_publish_resp_msg,
   constants__t_server_request_handle_i * const subscription_core__p_req_handle,
   constants__t_request_context_i * const subscription_core__p_req_context,
   t_bool * const subscription_core__p_moreNotifs);
extern void subscription_core__subscription_ack_notif_msg(
   const constants__t_subscription_i subscription_core__p_sub,
   const constants__t_sub_seq_num_i subscription_core__p_seq_num,
   t_bool * const subscription_core__is_valid_seq_num);
extern void subscription_core__subscription_core_UNINITIALISATION(void);

#endif
