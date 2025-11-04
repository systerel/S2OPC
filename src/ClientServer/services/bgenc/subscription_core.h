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

 Date                 : 04/11/2025 11:03:36

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
#include "monitored_item_filter_treatment.h"
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
#include "address_space_itf.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_bool subscription_core__continue_iter_prio;
extern t_entier4 subscription_core__idx_iter_sub;
extern t_entier4 subscription_core__min_idx_iter_sub;
extern t_entier4 subscription_core__nb_subs_iter_sub;
extern t_entier4 subscription_core__next_idx_iter_prio;
extern t_entier4 subscription_core__next_idx_iter_sub;
extern t_entier4 subscription_core__prio_idx_iter_prio;
extern constants__t_session_i subscription_core__session_iter_prio;

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
#define subscription_core__flush_internal_multi_send_publish_response_events msg_subscription_publish_bs__flush_internal_multi_send_publish_response_events
#define subscription_core__generate_internal_multi_send_publish_response_event msg_subscription_publish_bs__generate_internal_multi_send_publish_response_event
#define subscription_core__generate_internal_send_publish_response_event msg_subscription_publish_bs__generate_internal_send_publish_response_event
#define subscription_core__get_available_republish notification_republish_queue_it_bs__get_available_republish
#define subscription_core__get_card_session_seq_subscription subscription_core_1__get_card_session_seq_subscription
#define subscription_core__get_event_user_authorization monitored_item_filter_treatment__get_event_user_authorization
#define subscription_core__get_nodeToMonitoredItemQueue subscription_core_bs__get_nodeToMonitoredItemQueue
#define subscription_core__get_republish_notif_from_queue notification_republish_queue_bs__get_republish_notif_from_queue
#define subscription_core__get_subscription_notifRepublishQueue subscription_core_1__get_subscription_notifRepublishQueue
#define subscription_core__getall_monitoredItemPointer monitored_item_pointer_bs__getall_monitoredItemPointer
#define subscription_core__getall_session subscription_core_1__getall_session
#define subscription_core__init_iter_monitored_item monitored_item_queue_it_bs__init_iter_monitored_item
#define subscription_core__init_iter_notif_republish notification_republish_queue_it_bs__init_iter_notif_republish
#define subscription_core__is_notification_triggered monitored_item_pointer_bs__is_notification_triggered
#define subscription_core__is_valid_subscription subscription_core_1__is_valid_subscription
#define subscription_core__is_valid_subscription_on_session subscription_core_1__is_valid_subscription_on_session
#define subscription_core__reset_subscription_LifetimeCounter subscription_core_1__reset_subscription_LifetimeCounter
#define subscription_core__set_msg_publish_resp_notificationMsg msg_subscription_publish_bs__set_msg_publish_resp_notificationMsg
#define subscription_core__set_msg_publish_resp_subscription msg_subscription_publish_bs__set_msg_publish_resp_subscription
#define subscription_core__set_subscription_PublishingEnabled subscription_core_1__set_subscription_PublishingEnabled

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void subscription_core__get_fresh_subscription(
   t_bool * const subscription_core__bres,
   constants__t_subscription_i * const subscription_core__p_subscription);
extern void subscription_core__local_close_subscription(
   const constants__t_subscription_i subscription_core__p_subscription);
extern void subscription_core__local_compute_create_monitored_item_revised_params(
   const constants__t_AttributeId_i subscription_core__p_aid,
   const t_entier4 subscription_core__p_reqQueueSize,
   constants__t_opcua_duration_i * const subscription_core__revisedSamplingItv,
   t_entier4 * const subscription_core__revisedQueueSize);
extern void subscription_core__local_compute_msg_nb_notifs(
   const t_entier4 subscription_core__p_max_notifs,
   const t_entier4 subscription_core__p_avail_data_notifs,
   const t_entier4 subscription_core__p_avail_event_notifs,
   t_entier4 * const subscription_core__nb_data_notifs,
   t_entier4 * const subscription_core__nb_event_notifs,
   t_bool * const subscription_core__moreNotifs);
extern void subscription_core__local_continue_iter_session_seq_priority(
   t_bool * const subscription_core__p_continue,
   t_entier4 * const subscription_core__p_prio_idx);
extern void subscription_core__local_continue_iter_subscription_priority(
   t_bool * const subscription_core__p_continue,
   constants__t_subscription_i * const subscription_core__p_subscription);
extern void subscription_core__local_fill_data_notification_message(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_notif_msg_i subscription_core__p_notif_msg,
   const t_entier4 subscription_core__p_nb_data_notifs);
extern void subscription_core__local_fill_event_notification_message(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_notif_msg_i subscription_core__p_notif_msg,
   const t_entier4 subscription_core__p_nb_event_notifs);
extern void subscription_core__local_fill_notification_message_for_data_monitored_item(
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   const constants__t_notif_msg_i subscription_core__p_notif_msg,
   const t_entier4 subscription_core__p_cur_index,
   const t_entier4 subscription_core__nb_notif_to_dequeue,
   t_entier4 * const subscription_core__p_next_index);
extern void subscription_core__local_fill_notification_message_for_event_monitored_item(
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   const constants__t_notif_msg_i subscription_core__p_notif_msg,
   const t_entier4 subscription_core__p_cur_index,
   const t_entier4 subscription_core__nb_notif_to_dequeue,
   t_entier4 * const subscription_core__p_next_index);
extern void subscription_core__local_init_iter_session_seq_priority(
   const constants__t_session_i subscription_core__p_session,
   t_bool * const subscription_core__p_continue);
extern void subscription_core__local_init_iter_subscription_priority(
   const t_entier4 subscription_core__p_prio_idx);
extern void subscription_core__local_monitored_item_nb_available_notifications(
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   t_entier4 * const subscription_core__p_nb_available_notifs,
   t_bool * const subscription_core__p_isEvent);
extern void subscription_core__local_subscription_nb_available_notifications(
   const constants__t_subscription_i subscription_core__p_subscription,
   t_entier4 * const subscription_core__p_nb_available_data_notifs,
   t_entier4 * const subscription_core__p_nb_available_event_notifs);
extern void subscription_core__pop_invalid_and_check_valid_publishReqQueued(
   const constants__t_session_i subscription_core__p_session,
   t_bool * const subscription_core__p_validPubReqQueued);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void subscription_core__allocate_new_session_publish_queue(
   const constants__t_session_i subscription_core__p_session,
   t_bool * const subscription_core__bres);
extern void subscription_core__clear_monitored_item_notifications(
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer);
extern void subscription_core__close_subscription(
   const constants__t_subscription_i subscription_core__p_subscription);
extern void subscription_core__compute_create_monitored_item_revised_params(
   const constants__t_AttributeId_i subscription_core__p_aid,
   const t_entier4 subscription_core__p_reqQueueSize,
   constants__t_opcua_duration_i * const subscription_core__revisedSamplingItv,
   t_entier4 * const subscription_core__revisedQueueSize);
extern void subscription_core__continue_iter_subscription_session(
   t_bool * const subscription_core__p_continue,
   constants__t_subscription_i * const subscription_core__p_subscription);
extern void subscription_core__create_monitored_item(
   const constants__t_endpoint_config_idx_i subscription_core__p_endpoint_idx,
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
   const constants__t_monitoringFilter_i subscription_core__p_filter,
   const t_bool subscription_core__p_discardOldest,
   const t_entier4 subscription_core__p_queueSize,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__StatusCode_service,
   constants__t_monitoredItemPointer_i * const subscription_core__monitoredItemPointer,
   constants__t_monitoredItemId_i * const subscription_core__monitoredItemId,
   constants__t_filterResult_i * const subscription_core__filterResult);
extern void subscription_core__create_subscription(
   const constants__t_session_i subscription_core__p_session,
   const constants__t_opcua_duration_i subscription_core__p_revPublishInterval,
   const t_entier4 subscription_core__p_revLifetimeCount,
   const t_entier4 subscription_core__p_revMaxKeepAlive,
   const t_entier4 subscription_core__p_maxNotificationsPerPublish,
   const t_bool subscription_core__p_publishEnabled,
   const t_entier4 subscription_core__p_priority,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__StatusCode_service,
   constants__t_subscription_i * const subscription_core__subscription);
extern void subscription_core__deallocate_publish_queue_and_gen_no_sub_responses(
   const constants__t_session_i subscription_core__p_session,
   const t_bool subscription_core__p_send_no_sub_resp);
extern void subscription_core__delete_monitored_item(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_monitoredItemId_i subscription_core__p_mi_id,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_sc);
extern void subscription_core__enqueue_publish_request(
   const constants__t_session_i subscription_core__p_session,
   const constants__t_timeref_i subscription_core__p_req_exp_time,
   const constants__t_server_request_handle_i subscription_core__p_req_handle,
   const constants__t_request_context_i subscription_core__p_req_ctx,
   const constants__t_msg_i subscription_core__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__StatusCode_service,
   t_bool * const subscription_core__async_resp_msg);
extern void subscription_core__init_iter_subscription_session(
   const constants__t_session_i subscription_core__p_session,
   t_bool * const subscription_core__p_continue);
extern void subscription_core__modify_monitored_item(
   const constants__t_endpoint_config_idx_i subscription_core__p_endpoint_idx,
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_monitoredItemId_i subscription_core__p_mi_id,
   const constants__t_TimestampsToReturn_i subscription_core__p_timestampToReturn,
   const constants__t_client_handle_i subscription_core__p_clientHandle,
   const constants__t_monitoringFilter_i subscription_core__p_filter,
   const t_bool subscription_core__p_discardOldest,
   const t_entier4 subscription_core__p_queueSize,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_sc,
   constants__t_filterResult_i * const subscription_core__p_filterResult,
   constants__t_opcua_duration_i * const subscription_core__p_revSamplingItv,
   t_entier4 * const subscription_core__p_revQueueSize);
extern void subscription_core__modify_subscription(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_opcua_duration_i subscription_core__p_revPublishInterval,
   const t_entier4 subscription_core__p_revLifetimeCount,
   const t_entier4 subscription_core__p_revMaxKeepAlive,
   const t_entier4 subscription_core__p_revMaxNotifPerPublish,
   const t_entier4 subscription_core__p_priority);
extern void subscription_core__receive_publish_request(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_msg_i subscription_core__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__StatusCode_service,
   t_bool * const subscription_core__async_resp_msg,
   t_bool * const subscription_core__moreNotifs);
extern void subscription_core__server_subscription_add_notification_on_event_if_triggered(
   const t_bool subscription_core__p_userAccessGranted,
   const constants__t_LocaleIds_i subscription_core__p_localeIds,
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   const constants__t_client_handle_i subscription_core__p_clientHandle,
   const constants__t_TimestampsToReturn_i subscription_core__p_timestampToReturn,
   const constants__t_Event_i subscription_core__p_event);
extern void subscription_core__server_subscription_add_notification_on_node_or_monitMode_change(
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   const constants__t_NodeId_i subscription_core__p_nid,
   const constants__t_AttributeId_i subscription_core__p_aid,
   const constants__t_Variant_i subscription_core__p_VariantValuePointer,
   const constants__t_RawStatusCode subscription_core__p_ValueSc,
   const constants__t_Timestamp subscription_core__p_val_ts_src,
   const constants__t_Timestamp subscription_core__p_val_ts_srv,
   t_bool * const subscription_core__bres);
extern void subscription_core__server_subscription_add_notification_on_value_change(
   const constants__t_LocaleIds_i subscription_core__p_localeIds,
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   const constants__t_TimestampsToReturn_i subscription_core__p_timestampToReturn,
   const constants__t_WriteValuePointer_i subscription_core__p_writeValuePointer);
extern void subscription_core__server_subscription_core_check_valid_publish_req_queue(
   const constants__t_session_i subscription_core__p_session,
   t_bool * const subscription_core__p_validPublishingReqQueued);
extern void subscription_core__server_subscription_core_publish_timeout(
   const constants__t_session_i subscription_core__p_session,
   const constants__t_subscription_i subscription_core__p_subscription,
   const t_bool subscription_core__p_validPublishReqQueued,
   t_bool * const subscription_core__p_msg_to_send,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_msg_sc,
   constants__t_msg_i * const subscription_core__p_publish_resp_msg,
   constants__t_server_request_handle_i * const subscription_core__p_req_handle,
   constants__t_request_context_i * const subscription_core__p_req_context,
   t_bool * const subscription_core__p_moreNotifs);
extern void subscription_core__server_subscription_core_publish_timeout_check_lifetime(
   const constants__t_session_i subscription_core__p_session,
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__p_close_sub,
   t_bool * const subscription_core__p_msg_to_send,
   constants__t_msg_i * const subscription_core__p_publish_resp_msg,
   constants__t_server_request_handle_i * const subscription_core__p_req_handle,
   constants__t_request_context_i * const subscription_core__p_req_context,
   t_bool * const subscription_core__p_validPubReqQueued);
extern void subscription_core__server_subscription_core_publish_timeout_return_moreNotifs(
   const constants__t_session_i subscription_core__p_session,
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__p_msg_to_send,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_msg_sc,
   constants__t_msg_i * const subscription_core__p_publish_resp_msg,
   constants__t_server_request_handle_i * const subscription_core__p_req_handle,
   constants__t_request_context_i * const subscription_core__p_req_context,
   t_bool * const subscription_core__p_moreNotifs);
extern void subscription_core__set_monit_mode_monitored_item(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_monitoredItemId_i subscription_core__p_mi_id,
   const constants__t_monitoringMode_i subscription_core__p_monitoring_mode,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_sc,
   constants__t_monitoredItemPointer_i * const subscription_core__p_mi_pointer,
   constants__t_monitoringMode_i * const subscription_core__p_prevMonitMode);
extern void subscription_core__subscription_ack_notif_msg(
   const constants__t_subscription_i subscription_core__p_sub,
   const constants__t_sub_seq_num_i subscription_core__p_seq_num,
   t_bool * const subscription_core__is_valid_seq_num);
extern void subscription_core__subscription_core_UNINITIALISATION(void);

#endif
