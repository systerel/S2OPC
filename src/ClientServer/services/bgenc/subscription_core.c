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

 File Name            : subscription_core.c

 Date                 : 24/04/2025 10:11:31

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "subscription_core.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_core__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_core__local_compute_create_monitored_item_revised_params(
   const constants__t_AttributeId_i subscription_core__p_aid,
   const t_entier4 subscription_core__p_reqQueueSize,
   constants__t_opcua_duration_i * const subscription_core__revisedSamplingItv,
   t_entier4 * const subscription_core__revisedQueueSize) {
   *subscription_core__revisedSamplingItv = constants__c_opcua_duration_zero;
   if (subscription_core__p_aid != constants__e_aid_EventNotifier) {
      if (subscription_core__p_reqQueueSize <= 1) {
         *subscription_core__revisedQueueSize = 1;
      }
      else if (subscription_core__p_reqQueueSize > constants__k_n_notifQueueSize_max) {
         *subscription_core__revisedQueueSize = constants__k_n_notifQueueSize_max;
      }
      else {
         *subscription_core__revisedQueueSize = subscription_core__p_reqQueueSize;
      }
   }
   else {
      if (subscription_core__p_reqQueueSize == 0) {
         *subscription_core__revisedQueueSize = constants__k_n_notifEventQueueSize_default;
      }
      else if (subscription_core__p_reqQueueSize == 1) {
         *subscription_core__revisedQueueSize = constants__k_n_notifEventQueueSize_min;
      }
      else if (subscription_core__p_reqQueueSize > constants__k_n_notifQueueSize_max) {
         *subscription_core__revisedQueueSize = constants__k_n_notifQueueSize_max;
      }
      else {
         *subscription_core__revisedQueueSize = subscription_core__p_reqQueueSize;
      }
   }
}

void subscription_core__local_compute_msg_nb_notifs(
   const t_entier4 subscription_core__p_max_notifs,
   const t_entier4 subscription_core__p_avail_data_notifs,
   const t_entier4 subscription_core__p_avail_event_notifs,
   t_entier4 * const subscription_core__nb_data_notifs,
   t_entier4 * const subscription_core__nb_event_notifs,
   t_bool * const subscription_core__moreNotifs) {
   {
      t_entier4 subscription_core__l_max_nb_notifications;
      t_entier4 subscription_core__l_avail_notifs;
      t_entier4 subscription_core__l_overflow_notifs;
      t_entier4 subscription_core__l_abs_diff_notifs;
      
      *subscription_core__moreNotifs = false;
      *subscription_core__nb_data_notifs = subscription_core__p_avail_data_notifs;
      *subscription_core__nb_event_notifs = subscription_core__p_avail_event_notifs;
      if ((subscription_core__p_max_notifs > 0) &&
         (subscription_core__p_max_notifs < constants__k_n_monitoredItemNotif_max)) {
         subscription_core__l_max_nb_notifications = subscription_core__p_max_notifs;
      }
      else {
         subscription_core__l_max_nb_notifications = constants__k_n_monitoredItemNotif_max;
      }
      subscription_core__l_avail_notifs = subscription_core__p_avail_data_notifs +
         subscription_core__p_avail_event_notifs;
      if (subscription_core__l_avail_notifs > subscription_core__l_max_nb_notifications) {
         if (subscription_core__p_avail_event_notifs <= 0) {
            *subscription_core__nb_data_notifs = subscription_core__l_max_nb_notifications;
         }
         else if (subscription_core__p_avail_data_notifs <= 0) {
            *subscription_core__nb_event_notifs = subscription_core__l_max_nb_notifications;
         }
         else {
            subscription_core__l_overflow_notifs = subscription_core__l_avail_notifs -
               subscription_core__l_max_nb_notifications;
            if (subscription_core__p_avail_event_notifs > subscription_core__p_avail_data_notifs) {
               subscription_core__l_abs_diff_notifs = subscription_core__p_avail_event_notifs -
                  subscription_core__p_avail_data_notifs;
            }
            else if (subscription_core__p_avail_event_notifs < subscription_core__p_avail_data_notifs) {
               subscription_core__l_abs_diff_notifs = subscription_core__p_avail_data_notifs -
                  subscription_core__p_avail_event_notifs;
            }
            else {
               subscription_core__l_abs_diff_notifs = 0;
            }
            if (subscription_core__l_abs_diff_notifs > subscription_core__l_overflow_notifs) {
               subscription_core__l_abs_diff_notifs = subscription_core__l_overflow_notifs;
            }
            if (subscription_core__p_avail_event_notifs > subscription_core__p_avail_data_notifs) {
               *subscription_core__nb_event_notifs = *subscription_core__nb_event_notifs -
                  subscription_core__l_abs_diff_notifs;
            }
            else if (subscription_core__p_avail_event_notifs < subscription_core__p_avail_data_notifs) {
               *subscription_core__nb_data_notifs = *subscription_core__nb_data_notifs -
                  subscription_core__l_abs_diff_notifs;
            }
            subscription_core__l_overflow_notifs = subscription_core__l_overflow_notifs -
               subscription_core__l_abs_diff_notifs;
            if (subscription_core__l_overflow_notifs > 0) {
               *subscription_core__nb_data_notifs = *subscription_core__nb_data_notifs -
                  (subscription_core__l_overflow_notifs /
                  2);
               *subscription_core__nb_event_notifs = *subscription_core__nb_event_notifs -
                  (subscription_core__l_overflow_notifs -
                  (subscription_core__l_overflow_notifs /
                  2));
            }
         }
         *subscription_core__moreNotifs = true;
      }
   }
}

void subscription_core__get_fresh_subscription(
   t_bool * const subscription_core__bres,
   constants__t_subscription_i * const subscription_core__p_subscription) {
   {
      constants__t_subscription_i subscription_core__l_subscription;
      t_bool subscription_core__l_is_subscription;
      t_bool subscription_core__l_continue;
      
      *subscription_core__bres = false;
      *subscription_core__p_subscription = constants__c_subscription_indet;
      subscription_core__l_subscription = constants__c_subscription_indet;
      subscription_core__l_is_subscription = true;
      subscription_core_it__init_iter_subscription(&subscription_core__l_continue);
      if (subscription_core__l_continue == true) {
         while ((subscription_core__l_continue == true) &&
            (subscription_core__l_is_subscription == true)) {
            subscription_core_it__continue_iter_subscription(&subscription_core__l_continue,
               &subscription_core__l_subscription);
            subscription_core_1__is_valid_subscription(subscription_core__l_subscription,
               &subscription_core__l_is_subscription);
         }
      }
      if (subscription_core__l_is_subscription == false) {
         *subscription_core__bres = true;
         *subscription_core__p_subscription = subscription_core__l_subscription;
      }
   }
}

void subscription_core__pop_invalid_and_check_valid_publishReqQueued(
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__p_validPubReqQueued) {
   {
      constants__t_publishReqQueue_i subscription_core__l_pubReqQueue;
      t_bool subscription_core__l_continue;
      constants__t_session_i subscription_core__l_session;
      constants__t_timeref_i subscription_core__l_req_exp_time;
      constants__t_server_request_handle_i subscription_core__l_req_handle;
      constants__t_request_context_i subscription_core__l_req_ctx;
      constants__t_msg_i subscription_core__l_resp_msg;
      t_bool subscription_core__l_is_expired;
      
      *subscription_core__p_validPubReqQueued = false;
      subscription_core_1__get_subscription_publishRequestQueue(subscription_core__p_subscription,
         &subscription_core__l_pubReqQueue);
      publish_request_queue_bs__init_iter_publish_request(subscription_core__l_pubReqQueue,
         &subscription_core__l_continue);
      subscription_core__l_is_expired = true;
      while ((subscription_core__l_continue == true) &&
         (subscription_core__l_is_expired == true)) {
         publish_request_queue_bs__continue_pop_head_iter_publish_request(subscription_core__l_pubReqQueue,
            &subscription_core__l_continue,
            &subscription_core__l_session,
            &subscription_core__l_req_exp_time,
            &subscription_core__l_req_handle,
            &subscription_core__l_req_ctx,
            &subscription_core__l_resp_msg);
         publish_request_queue_bs__is_request_expired(subscription_core__l_req_exp_time,
            &subscription_core__l_is_expired);
         if (subscription_core__l_is_expired == true) {
            msg_subscription_publish_bs__generate_internal_send_publish_response_event(subscription_core__l_session,
               subscription_core__l_resp_msg,
               subscription_core__l_req_handle,
               subscription_core__l_req_ctx,
               constants_statuscodes_bs__e_sc_bad_timeout);
         }
         else {
            publish_request_queue_bs__prepend_publish_request_to_queue(subscription_core__l_pubReqQueue,
               subscription_core__l_session,
               subscription_core__l_req_exp_time,
               subscription_core__l_req_handle,
               subscription_core__l_req_ctx,
               subscription_core__l_resp_msg,
               subscription_core__p_validPubReqQueued);
         }
      }
   }
}

void subscription_core__local_fill_notification_message_for_data_monitored_item(
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   const constants__t_notif_msg_i subscription_core__p_notif_msg,
   const t_entier4 subscription_core__p_cur_index,
   const t_entier4 subscription_core__nb_notif_to_dequeue,
   t_entier4 * const subscription_core__p_next_index) {
   {
      constants__t_notificationQueue_i subscription_core__l_notifQueue;
      t_bool subscription_core__l_continue;
      t_bool subscription_core__l_isEvent;
      constants__t_WriteValuePointer_i subscription_core__l_writeValuePointer;
      constants__t_monitoredItemId_i subscription_core__l_monitoredItemId;
      constants__t_subscription_i subscription_core__l_subscription;
      constants__t_NodeId_i subscription_core__l_nid;
      constants__t_AttributeId_i subscription_core__l_aid;
      constants__t_IndexRange_i subscription_core__l_indexRange;
      constants__t_TimestampsToReturn_i subscription_core__l_timestampToReturn;
      constants__t_monitoringMode_i subscription_core__l_monitoringMode;
      constants__t_client_handle_i subscription_core__l_clientHandle;
      
      subscription_core__l_isEvent = true;
      *subscription_core__p_next_index = subscription_core__p_cur_index;
      monitored_item_notification_queue_bs__get_monitored_item_notification_queue(subscription_core__p_monitoredItemPointer,
         &subscription_core__l_continue,
         &subscription_core__l_notifQueue);
      if (subscription_core__l_continue == true) {
         monitored_item_notification_queue_bs__init_iter_monitored_item_notification(subscription_core__p_monitoredItemPointer,
            subscription_core__l_notifQueue,
            &subscription_core__l_continue,
            &subscription_core__l_isEvent);
      }
      while (((subscription_core__l_continue == true) &&
         (subscription_core__l_isEvent == false)) &&
         (*subscription_core__p_next_index <= subscription_core__nb_notif_to_dequeue)) {
         monitored_item_notification_queue_bs__continue_pop_iter_monitor_item_data_notification(subscription_core__p_monitoredItemPointer,
            subscription_core__l_notifQueue,
            &subscription_core__l_continue,
            &subscription_core__l_writeValuePointer);
         monitored_item_pointer_bs__getall_monitoredItemPointer(subscription_core__p_monitoredItemPointer,
            &subscription_core__l_monitoredItemId,
            &subscription_core__l_subscription,
            &subscription_core__l_nid,
            &subscription_core__l_aid,
            &subscription_core__l_indexRange,
            &subscription_core__l_timestampToReturn,
            &subscription_core__l_monitoringMode,
            &subscription_core__l_clientHandle);
         msg_subscription_publish_bs__setall_notification_msg_monitored_item_data_notif(subscription_core__p_notif_msg,
            *subscription_core__p_next_index,
            subscription_core__l_monitoredItemId,
            subscription_core__l_clientHandle,
            subscription_core__l_writeValuePointer);
         *subscription_core__p_next_index = *subscription_core__p_next_index +
            1;
      }
   }
}

void subscription_core__local_fill_data_notification_message(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_notif_msg_i subscription_core__p_notif_msg,
   const t_entier4 subscription_core__p_nb_data_notifs) {
   {
      constants__t_monitoredItemQueue_i subscription_core__l_monitored_item_queue;
      t_bool subscription_core__l_continue_mi;
      constants__t_monitoredItemQueueIterator_i subscription_core__l_iterator;
      constants__t_monitoredItemPointer_i subscription_core__l_monitoredItemPointer;
      t_bool subscription_core__l_isEvent;
      t_entier4 subscription_core__l_index;
      t_entier4 subscription_core__l_new_index;
      constants__t_monitoredItemId_i subscription_core__l_monitoredItemId;
      constants__t_subscription_i subscription_core__l_subscription;
      constants__t_NodeId_i subscription_core__l_nid;
      constants__t_AttributeId_i subscription_core__l_aid;
      constants__t_IndexRange_i subscription_core__l_indexRange;
      constants__t_TimestampsToReturn_i subscription_core__l_timestampToReturn;
      constants__t_monitoringMode_i subscription_core__l_monitoringMode;
      constants__t_client_handle_i subscription_core__l_clientHandle;
      
      subscription_core__l_index = 1;
      subscription_core_1__get_subscription_monitoredItemQueue(subscription_core__p_subscription,
         &subscription_core__l_monitored_item_queue);
      monitored_item_queue_it_bs__init_iter_monitored_item(subscription_core__l_monitored_item_queue,
         &subscription_core__l_continue_mi,
         &subscription_core__l_iterator);
      while ((subscription_core__l_continue_mi == true) &&
         (subscription_core__l_index <= subscription_core__p_nb_data_notifs)) {
         monitored_item_queue_it_bs__continue_iter_monitored_item(subscription_core__l_iterator,
            subscription_core__l_monitored_item_queue,
            &subscription_core__l_continue_mi,
            &subscription_core__l_monitoredItemPointer);
         monitored_item_pointer_bs__getall_monitoredItemPointer(subscription_core__l_monitoredItemPointer,
            &subscription_core__l_monitoredItemId,
            &subscription_core__l_subscription,
            &subscription_core__l_nid,
            &subscription_core__l_aid,
            &subscription_core__l_indexRange,
            &subscription_core__l_timestampToReturn,
            &subscription_core__l_monitoringMode,
            &subscription_core__l_clientHandle);
         monitored_item_pointer_bs__is_event_monitoredItem(subscription_core__l_monitoredItemPointer,
            &subscription_core__l_isEvent);
         if ((subscription_core__l_monitoringMode == constants__e_monitoringMode_reporting) &&
            (subscription_core__l_isEvent == false)) {
            subscription_core__local_fill_notification_message_for_data_monitored_item(subscription_core__l_monitoredItemPointer,
               subscription_core__p_notif_msg,
               subscription_core__l_index,
               subscription_core__p_nb_data_notifs,
               &subscription_core__l_new_index);
            subscription_core__l_index = subscription_core__l_new_index;
         }
      }
      monitored_item_queue_it_bs__clear_iter_monitored_item(subscription_core__l_iterator);
   }
}

void subscription_core__local_fill_notification_message_for_event_monitored_item(
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   const constants__t_notif_msg_i subscription_core__p_notif_msg,
   const t_entier4 subscription_core__p_cur_index,
   const t_entier4 subscription_core__nb_notif_to_dequeue,
   t_entier4 * const subscription_core__p_next_index) {
   {
      constants__t_notificationQueue_i subscription_core__l_notifQueue;
      t_bool subscription_core__l_continue;
      t_bool subscription_core__l_isEvent;
      constants__t_eventFieldList_i subscription_core__l_eventFieldList;
      constants__t_monitoredItemId_i subscription_core__l_monitoredItemId;
      constants__t_subscription_i subscription_core__l_subscription;
      constants__t_NodeId_i subscription_core__l_nid;
      constants__t_AttributeId_i subscription_core__l_aid;
      constants__t_IndexRange_i subscription_core__l_indexRange;
      constants__t_TimestampsToReturn_i subscription_core__l_timestampToReturn;
      constants__t_monitoringMode_i subscription_core__l_monitoringMode;
      constants__t_client_handle_i subscription_core__l_clientHandle;
      
      subscription_core__l_isEvent = false;
      *subscription_core__p_next_index = subscription_core__p_cur_index;
      monitored_item_notification_queue_bs__get_monitored_item_notification_queue(subscription_core__p_monitoredItemPointer,
         &subscription_core__l_continue,
         &subscription_core__l_notifQueue);
      if (subscription_core__l_continue == true) {
         monitored_item_notification_queue_bs__init_iter_monitored_item_notification(subscription_core__p_monitoredItemPointer,
            subscription_core__l_notifQueue,
            &subscription_core__l_continue,
            &subscription_core__l_isEvent);
      }
      while (((subscription_core__l_continue == true) &&
         (subscription_core__l_isEvent == true)) &&
         (*subscription_core__p_next_index <= subscription_core__nb_notif_to_dequeue)) {
         monitored_item_notification_queue_bs__continue_pop_iter_monitor_item_event_notification(subscription_core__p_monitoredItemPointer,
            subscription_core__l_notifQueue,
            &subscription_core__l_continue,
            &subscription_core__l_eventFieldList);
         monitored_item_pointer_bs__getall_monitoredItemPointer(subscription_core__p_monitoredItemPointer,
            &subscription_core__l_monitoredItemId,
            &subscription_core__l_subscription,
            &subscription_core__l_nid,
            &subscription_core__l_aid,
            &subscription_core__l_indexRange,
            &subscription_core__l_timestampToReturn,
            &subscription_core__l_monitoringMode,
            &subscription_core__l_clientHandle);
         msg_subscription_publish_bs__setall_notification_msg_monitored_item_event_notif(subscription_core__p_notif_msg,
            *subscription_core__p_next_index,
            subscription_core__l_monitoredItemId,
            subscription_core__l_eventFieldList);
         *subscription_core__p_next_index = *subscription_core__p_next_index +
            1;
      }
   }
}

void subscription_core__local_fill_event_notification_message(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_notif_msg_i subscription_core__p_notif_msg,
   const t_entier4 subscription_core__p_nb_event_notifs) {
   {
      constants__t_monitoredItemQueue_i subscription_core__l_monitored_item_queue;
      t_bool subscription_core__l_continue_mi;
      constants__t_monitoredItemQueueIterator_i subscription_core__l_iterator;
      constants__t_monitoredItemPointer_i subscription_core__l_monitoredItemPointer;
      t_bool subscription_core__l_isEvent;
      t_entier4 subscription_core__l_index;
      t_entier4 subscription_core__l_new_index;
      constants__t_monitoredItemId_i subscription_core__l_monitoredItemId;
      constants__t_subscription_i subscription_core__l_subscription;
      constants__t_NodeId_i subscription_core__l_nid;
      constants__t_AttributeId_i subscription_core__l_aid;
      constants__t_IndexRange_i subscription_core__l_indexRange;
      constants__t_TimestampsToReturn_i subscription_core__l_timestampToReturn;
      constants__t_monitoringMode_i subscription_core__l_monitoringMode;
      constants__t_client_handle_i subscription_core__l_clientHandle;
      
      subscription_core__l_index = 1;
      subscription_core_1__get_subscription_monitoredItemQueue(subscription_core__p_subscription,
         &subscription_core__l_monitored_item_queue);
      monitored_item_queue_it_bs__init_iter_monitored_item(subscription_core__l_monitored_item_queue,
         &subscription_core__l_continue_mi,
         &subscription_core__l_iterator);
      while ((subscription_core__l_continue_mi == true) &&
         (subscription_core__l_index <= subscription_core__p_nb_event_notifs)) {
         monitored_item_queue_it_bs__continue_iter_monitored_item(subscription_core__l_iterator,
            subscription_core__l_monitored_item_queue,
            &subscription_core__l_continue_mi,
            &subscription_core__l_monitoredItemPointer);
         monitored_item_pointer_bs__getall_monitoredItemPointer(subscription_core__l_monitoredItemPointer,
            &subscription_core__l_monitoredItemId,
            &subscription_core__l_subscription,
            &subscription_core__l_nid,
            &subscription_core__l_aid,
            &subscription_core__l_indexRange,
            &subscription_core__l_timestampToReturn,
            &subscription_core__l_monitoringMode,
            &subscription_core__l_clientHandle);
         monitored_item_pointer_bs__is_event_monitoredItem(subscription_core__l_monitoredItemPointer,
            &subscription_core__l_isEvent);
         if ((subscription_core__l_monitoringMode == constants__e_monitoringMode_reporting) &&
            (subscription_core__l_isEvent == true)) {
            subscription_core__local_fill_notification_message_for_event_monitored_item(subscription_core__l_monitoredItemPointer,
               subscription_core__p_notif_msg,
               subscription_core__l_index,
               subscription_core__p_nb_event_notifs,
               &subscription_core__l_new_index);
            subscription_core__l_index = subscription_core__l_new_index;
         }
      }
      monitored_item_queue_it_bs__clear_iter_monitored_item(subscription_core__l_iterator);
   }
}

void subscription_core__local_close_subscription(
   const constants__t_subscription_i subscription_core__p_subscription) {
   {
      constants__t_timer_id_i subscription_core__l_timer_id;
      constants__t_publishReqQueue_i subscription_core__l_publish_queue;
      constants__t_notifRepublishQueue_i subscription_core__l_republish_queue;
      t_bool subscription_core__l_continue_pub;
      constants__t_session_i subscription_core__l_session;
      constants__t_timeref_i subscription_core__l_req_exp_time;
      constants__t_server_request_handle_i subscription_core__l_req_handle;
      constants__t_request_context_i subscription_core__l_req_ctx;
      constants__t_msg_i subscription_core__l_resp_msg;
      constants__t_monitoredItemQueue_i subscription_core__l_monitored_item_queue;
      constants__t_notificationQueue_i subscription_core__l_notification_queue;
      t_bool subscription_core__l_continue_mi;
      constants__t_monitoredItemQueueIterator_i subscription_core__l_iterator;
      constants__t_monitoredItemPointer_i subscription_core__l_monitoredItemPointer;
      constants__t_monitoredItemId_i subscription_core__l_monitoredItemId;
      constants__t_subscription_i subscription_core__l_subscription;
      constants__t_NodeId_i subscription_core__l_nid;
      constants__t_AttributeId_i subscription_core__l_aid;
      constants__t_IndexRange_i subscription_core__l_indexRange;
      constants__t_TimestampsToReturn_i subscription_core__l_timestampToReturn;
      constants__t_monitoringMode_i subscription_core__l_monitoringMode;
      constants__t_client_handle_i subscription_core__l_clientHandle;
      constants__t_monitoredItemQueue_i subscription_core__l_monitored_item_node_queue;
      t_bool subscription_core__l_bres;
      
      subscription_core_1__get_subscription_timer_id(subscription_core__p_subscription,
         &subscription_core__l_timer_id);
      subscription_core_bs__delete_publish_timer(subscription_core__l_timer_id);
      subscription_core_1__get_subscription_publishRequestQueue(subscription_core__p_subscription,
         &subscription_core__l_publish_queue);
      subscription_core_1__get_subscription_notifRepublishQueue(subscription_core__p_subscription,
         &subscription_core__l_republish_queue);
      publish_request_queue_bs__init_iter_publish_request(subscription_core__l_publish_queue,
         &subscription_core__l_continue_pub);
      while (subscription_core__l_continue_pub == true) {
         publish_request_queue_bs__continue_pop_head_iter_publish_request(subscription_core__l_publish_queue,
            &subscription_core__l_continue_pub,
            &subscription_core__l_session,
            &subscription_core__l_req_exp_time,
            &subscription_core__l_req_handle,
            &subscription_core__l_req_ctx,
            &subscription_core__l_resp_msg);
         msg_subscription_publish_bs__generate_internal_send_publish_response_event(subscription_core__l_session,
            subscription_core__l_resp_msg,
            subscription_core__l_req_handle,
            subscription_core__l_req_ctx,
            constants_statuscodes_bs__e_sc_bad_no_subscription);
      }
      publish_request_queue_bs__clear_and_deallocate_publish_queue(subscription_core__l_publish_queue);
      notification_republish_queue_bs__clear_and_deallocate_republish_queue(subscription_core__l_republish_queue);
      subscription_core_1__get_subscription_monitoredItemQueue(subscription_core__p_subscription,
         &subscription_core__l_monitored_item_queue);
      monitored_item_queue_it_bs__init_iter_monitored_item(subscription_core__l_monitored_item_queue,
         &subscription_core__l_continue_mi,
         &subscription_core__l_iterator);
      while (subscription_core__l_continue_mi == true) {
         monitored_item_queue_it_bs__continue_iter_monitored_item(subscription_core__l_iterator,
            subscription_core__l_monitored_item_queue,
            &subscription_core__l_continue_mi,
            &subscription_core__l_monitoredItemPointer);
         monitored_item_pointer_bs__getall_monitoredItemPointer(subscription_core__l_monitoredItemPointer,
            &subscription_core__l_monitoredItemId,
            &subscription_core__l_subscription,
            &subscription_core__l_nid,
            &subscription_core__l_aid,
            &subscription_core__l_indexRange,
            &subscription_core__l_timestampToReturn,
            &subscription_core__l_monitoringMode,
            &subscription_core__l_clientHandle);
         subscription_core_bs__get_nodeToMonitoredItemQueue(subscription_core__l_nid,
            &subscription_core__l_bres,
            &subscription_core__l_monitored_item_node_queue);
         if (subscription_core__l_bres == true) {
            monitored_item_queue_bs__remove_monitored_item(subscription_core__l_monitored_item_node_queue,
               subscription_core__l_monitoredItemPointer,
               &subscription_core__l_bres);
         }
         monitored_item_notification_queue_bs__get_monitored_item_notification_queue(subscription_core__l_monitoredItemPointer,
            &subscription_core__l_bres,
            &subscription_core__l_notification_queue);
         if (subscription_core__l_bres == true) {
            monitored_item_notification_queue_bs__clear_and_deallocate_monitored_item_notification_queue(subscription_core__l_monitoredItemPointer,
               subscription_core__l_notification_queue);
         }
         monitored_item_pointer_bs__delete_monitored_item_pointer(subscription_core__l_monitoredItemPointer);
      }
      monitored_item_queue_it_bs__clear_iter_monitored_item(subscription_core__l_iterator);
      monitored_item_queue_bs__clear_and_deallocate_monitored_item_queue(subscription_core__l_monitored_item_queue);
      subscription_core_1__delete_subscription(subscription_core__p_subscription);
   }
}

void subscription_core__local_subscription_nb_available_notifications(
   const constants__t_subscription_i subscription_core__p_subscription,
   t_entier4 * const subscription_core__p_nb_available_data_notifs,
   t_entier4 * const subscription_core__p_nb_available_event_notifs) {
   {
      constants__t_monitoredItemQueue_i subscription_core__l_monitored_item_queue;
      t_bool subscription_core__l_continue_mi;
      constants__t_monitoredItemQueueIterator_i subscription_core__l_iterator;
      constants__t_monitoredItemPointer_i subscription_core__l_monitoredItemPointer;
      constants__t_monitoredItemId_i subscription_core__l_monitoredItemId;
      constants__t_subscription_i subscription_core__l_subscription;
      constants__t_NodeId_i subscription_core__l_nid;
      constants__t_AttributeId_i subscription_core__l_aid;
      constants__t_IndexRange_i subscription_core__l_indexRange;
      constants__t_TimestampsToReturn_i subscription_core__l_timestampToReturn;
      constants__t_monitoringMode_i subscription_core__l_monitoringMode;
      constants__t_client_handle_i subscription_core__l_clientHandle;
      t_entier4 subscription_core__l_mi_available_notifs;
      t_bool subscription_core__l_isEvent;
      t_entier4 subscription_core__l_offset_from_maxint;
      
      *subscription_core__p_nb_available_data_notifs = 0;
      *subscription_core__p_nb_available_event_notifs = 0;
      subscription_core_1__get_subscription_monitoredItemQueue(subscription_core__p_subscription,
         &subscription_core__l_monitored_item_queue);
      monitored_item_queue_it_bs__init_iter_monitored_item(subscription_core__l_monitored_item_queue,
         &subscription_core__l_continue_mi,
         &subscription_core__l_iterator);
      while (subscription_core__l_continue_mi == true) {
         monitored_item_queue_it_bs__continue_iter_monitored_item(subscription_core__l_iterator,
            subscription_core__l_monitored_item_queue,
            &subscription_core__l_continue_mi,
            &subscription_core__l_monitoredItemPointer);
         monitored_item_pointer_bs__getall_monitoredItemPointer(subscription_core__l_monitoredItemPointer,
            &subscription_core__l_monitoredItemId,
            &subscription_core__l_subscription,
            &subscription_core__l_nid,
            &subscription_core__l_aid,
            &subscription_core__l_indexRange,
            &subscription_core__l_timestampToReturn,
            &subscription_core__l_monitoringMode,
            &subscription_core__l_clientHandle);
         if (subscription_core__l_monitoringMode == constants__e_monitoringMode_reporting) {
            subscription_core__local_monitored_item_nb_available_notifications(subscription_core__l_monitoredItemPointer,
               &subscription_core__l_mi_available_notifs,
               &subscription_core__l_isEvent);
            if (subscription_core__l_isEvent == true) {
               subscription_core__l_offset_from_maxint = MAXINT -
                  *subscription_core__p_nb_available_event_notifs;
               if (subscription_core__l_mi_available_notifs <= subscription_core__l_offset_from_maxint) {
                  *subscription_core__p_nb_available_event_notifs = *subscription_core__p_nb_available_event_notifs +
                     subscription_core__l_mi_available_notifs;
               }
               else {
                  *subscription_core__p_nb_available_event_notifs = MAXINT;
               }
            }
            else {
               subscription_core__l_offset_from_maxint = MAXINT -
                  *subscription_core__p_nb_available_data_notifs;
               if (subscription_core__l_mi_available_notifs <= subscription_core__l_offset_from_maxint) {
                  *subscription_core__p_nb_available_data_notifs = *subscription_core__p_nb_available_data_notifs +
                     subscription_core__l_mi_available_notifs;
               }
               else {
                  *subscription_core__p_nb_available_data_notifs = MAXINT;
               }
            }
         }
      }
      monitored_item_queue_it_bs__clear_iter_monitored_item(subscription_core__l_iterator);
   }
}

void subscription_core__local_monitored_item_nb_available_notifications(
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   t_entier4 * const subscription_core__p_nb_available_notifs,
   t_bool * const subscription_core__p_isEvent) {
   {
      t_bool subscription_core__l_bres;
      constants__t_notificationQueue_i subscription_core__l_mi_notif_queue;
      
      *subscription_core__p_isEvent = false;
      *subscription_core__p_nb_available_notifs = 0;
      monitored_item_notification_queue_bs__get_monitored_item_notification_queue(subscription_core__p_monitoredItemPointer,
         &subscription_core__l_bres,
         &subscription_core__l_mi_notif_queue);
      if (subscription_core__l_bres == true) {
         monitored_item_notification_queue_bs__get_length_monitored_item_notification_queue(subscription_core__l_mi_notif_queue,
            subscription_core__p_nb_available_notifs);
         monitored_item_notification_queue_bs__is_event_monitored_item_notification_queue(subscription_core__p_monitoredItemPointer,
            subscription_core__l_mi_notif_queue,
            subscription_core__p_isEvent);
      }
   }
}

void subscription_core__is_valid_subscription_on_session(
   const constants__t_session_i subscription_core__p_session,
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__is_valid) {
   {
      t_bool subscription_core__l_dom;
      constants__t_subscription_i subscription_core__l_sub;
      
      subscription_core_1__getall_subscription(subscription_core__p_session,
         &subscription_core__l_dom,
         &subscription_core__l_sub);
      *subscription_core__is_valid = ((subscription_core__l_dom == true) &&
         (subscription_core__p_subscription == subscription_core__l_sub));
   }
}

void subscription_core__empty_session_publish_requests(
   const constants__t_subscription_i subscription_core__p_subscription) {
   {
      constants__t_publishReqQueue_i subscription_core__l_PublishRequestQueue;
      constants__t_notifRepublishQueue_i subscription_core__l_RepublishQueue;
      
      subscription_core_1__get_subscription_publishRequestQueue(subscription_core__p_subscription,
         &subscription_core__l_PublishRequestQueue);
      publish_request_queue_bs__clear_publish_queue(subscription_core__l_PublishRequestQueue);
      subscription_core_1__get_subscription_notifRepublishQueue(subscription_core__p_subscription,
         &subscription_core__l_RepublishQueue);
      notification_republish_queue_bs__clear_republish_queue(subscription_core__l_RepublishQueue);
   }
}

void subscription_core__create_subscription(
   const constants__t_session_i subscription_core__p_session,
   const constants__t_opcua_duration_i subscription_core__p_revPublishInterval,
   const t_entier4 subscription_core__p_revLifetimeCount,
   const t_entier4 subscription_core__p_revMaxKeepAlive,
   const t_entier4 subscription_core__p_maxNotificationsPerPublish,
   const t_bool subscription_core__p_publishEnabled,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__StatusCode_service,
   constants__t_subscription_i * const subscription_core__subscription) {
   {
      t_bool subscription_core__l_bres;
      constants__t_subscription_i subscription_core__l_subscription;
      t_bool subscription_core__l_bres_pub;
      constants__t_publishReqQueue_i subscription_core__l_newPublishQueue;
      t_bool subscription_core__l_bres_repub;
      constants__t_notifRepublishQueue_i subscription_core__l_newRepublishQueue;
      t_bool subscription_core__l_bres_monitored;
      constants__t_monitoredItemQueue_i subscription_core__l_newMonitoredItemQueue;
      t_bool subscription_core__l_bres_timer;
      constants__t_timer_id_i subscription_core__l_timerId;
      
      *subscription_core__StatusCode_service = constants_statuscodes_bs__c_StatusCode_indet;
      *subscription_core__subscription = constants__c_subscription_indet;
      subscription_core__get_fresh_subscription(&subscription_core__l_bres,
         &subscription_core__l_subscription);
      if (subscription_core__l_bres == true) {
         publish_request_queue_bs__allocate_new_publish_queue(&subscription_core__l_bres_pub,
            &subscription_core__l_newPublishQueue);
         notification_republish_queue_bs__allocate_new_republish_queue(&subscription_core__l_bres_repub,
            &subscription_core__l_newRepublishQueue);
         monitored_item_queue_bs__allocate_new_monitored_item_queue(&subscription_core__l_bres_monitored,
            &subscription_core__l_newMonitoredItemQueue);
         subscription_core_bs__create_periodic_publish_timer(subscription_core__l_subscription,
            subscription_core__p_revPublishInterval,
            &subscription_core__l_bres_timer,
            &subscription_core__l_timerId);
         if ((((subscription_core__l_bres_pub == true) &&
            (subscription_core__l_bres_repub == true)) &&
            (subscription_core__l_bres_monitored == true)) &&
            (subscription_core__l_bres_timer == true)) {
            *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
            subscription_core_1__add_subscription(subscription_core__l_subscription,
               subscription_core__p_session,
               subscription_core__p_revPublishInterval,
               subscription_core__p_revLifetimeCount,
               subscription_core__p_revMaxKeepAlive,
               subscription_core__p_maxNotificationsPerPublish,
               subscription_core__p_publishEnabled,
               subscription_core__l_newPublishQueue,
               subscription_core__l_newRepublishQueue,
               subscription_core__l_newMonitoredItemQueue,
               subscription_core__l_timerId);
            *subscription_core__subscription = subscription_core__l_subscription;
         }
         else {
            *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            if (subscription_core__l_bres_pub == true) {
               publish_request_queue_bs__clear_and_deallocate_publish_queue(subscription_core__l_newPublishQueue);
            }
            if (subscription_core__l_bres_repub == true) {
               notification_republish_queue_bs__clear_and_deallocate_republish_queue(subscription_core__l_newRepublishQueue);
            }
            if (subscription_core__l_bres_monitored == true) {
               monitored_item_queue_bs__clear_and_deallocate_monitored_item_queue(subscription_core__l_newMonitoredItemQueue);
            }
            if (subscription_core__l_bres_timer == true) {
               subscription_core_bs__delete_publish_timer(subscription_core__l_timerId);
            }
         }
      }
      else {
         *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_subscriptions;
      }
   }
}

void subscription_core__modify_subscription(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_opcua_duration_i subscription_core__p_revPublishInterval,
   const t_entier4 subscription_core__p_revLifetimeCount,
   const t_entier4 subscription_core__p_revMaxKeepAlive,
   const t_entier4 subscription_core__p_revMaxNotifPerPublish) {
   {
      constants__t_timer_id_i subscription_core__l_timerId;
      
      subscription_core_1__set_subscription_publishInterval(subscription_core__p_subscription,
         subscription_core__p_revPublishInterval);
      subscription_core_1__set_subscription_MaxLifetimeAndKeepAliveCount(subscription_core__p_subscription,
         subscription_core__p_revLifetimeCount,
         subscription_core__p_revMaxKeepAlive);
      subscription_core_1__reset_subscription_LifetimeCounter(subscription_core__p_subscription);
      subscription_core_1__set_subscription_MaxNotifsPerPublish(subscription_core__p_subscription,
         subscription_core__p_revMaxNotifPerPublish);
      subscription_core_1__get_subscription_timer_id(subscription_core__p_subscription,
         &subscription_core__l_timerId);
      subscription_core_bs__modify_publish_timer_period(subscription_core__l_timerId,
         subscription_core__p_revPublishInterval);
   }
}

void subscription_core__close_subscription(
   const constants__t_subscription_i subscription_core__p_subscription) {
   subscription_core__local_close_subscription(subscription_core__p_subscription);
}

void subscription_core__subscription_ack_notif_msg(
   const constants__t_subscription_i subscription_core__p_sub,
   const constants__t_sub_seq_num_i subscription_core__p_seq_num,
   t_bool * const subscription_core__is_valid_seq_num) {
   {
      constants__t_notifRepublishQueue_i subscription_core__l_republishQueue;
      
      subscription_core_1__get_subscription_notifRepublishQueue(subscription_core__p_sub,
         &subscription_core__l_republishQueue);
      notification_republish_queue_bs__remove_republish_notif_from_queue(subscription_core__l_republishQueue,
         subscription_core__p_seq_num,
         subscription_core__is_valid_seq_num);
   }
}

void subscription_core__receive_publish_request(
   const constants__t_session_i subscription_core__p_session,
   const constants__t_timeref_i subscription_core__p_req_exp_time,
   const constants__t_server_request_handle_i subscription_core__p_req_handle,
   const constants__t_request_context_i subscription_core__p_req_ctx,
   const constants__t_msg_i subscription_core__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__StatusCode_service,
   t_bool * const subscription_core__async_resp_msg,
   constants__t_subscription_i * const subscription_core__subscription,
   t_bool * const subscription_core__moreNotifs) {
   {
      t_bool subscription_core__l_dom;
      constants__t_subscription_i subscription_core__l_subscription;
      constants__t_subscriptionState_i subscription_core__l_subscriptionState;
      constants__t_publishReqQueue_i subscription_core__l_PublishingReqQueue;
      t_bool subscription_core__l_PublishingEnabled;
      t_entier4 subscription_core__l_nb_avail_data_notifs;
      t_entier4 subscription_core__l_nb_avail_event_notifs;
      t_entier4 subscription_core__l_nb_data_notifs;
      t_entier4 subscription_core__l_nb_event_notifs;
      t_entier4 subscription_core__l_max_configured_notifications;
      t_bool subscription_core__l_NotificationAvailable;
      t_bool subscription_core__l_MoreNotifications;
      constants__t_notifRepublishQueue_i subscription_core__l_notifRepublishQueue;
      t_bool subscription_core__l_bres;
      t_entier4 subscription_core__l_nb_pub_reqs;
      t_entier4 subscription_core__l_nb_repub_notifs;
      constants__t_session_i subscription_core__l_old_session;
      constants__t_server_request_handle_i subscription_core__l_old_req_handle;
      constants__t_request_context_i subscription_core__l_old_req_ctx;
      constants__t_msg_i subscription_core__l_old_resp_msg;
      constants__t_notif_msg_i subscription_core__l_notifMsg;
      constants__t_sub_seq_num_i subscription_core__l_seq_num;
      constants__t_sub_seq_num_i subscription_core__l_next_seq_num;
      
      subscription_core_1__getall_subscription(subscription_core__p_session,
         &subscription_core__l_dom,
         &subscription_core__l_subscription);
      subscription_core_1__get_subscription_state(subscription_core__l_subscription,
         &subscription_core__l_subscriptionState);
      subscription_core_1__get_subscription_publishRequestQueue(subscription_core__l_subscription,
         &subscription_core__l_PublishingReqQueue);
      subscription_core_1__get_subscription_PublishingEnabled(subscription_core__l_subscription,
         &subscription_core__l_PublishingEnabled);
      subscription_core_1__get_subscription_MoreNotifications(subscription_core__l_subscription,
         &subscription_core__l_MoreNotifications);
      subscription_core__local_subscription_nb_available_notifications(subscription_core__l_subscription,
         &subscription_core__l_nb_avail_data_notifs,
         &subscription_core__l_nb_avail_event_notifs);
      subscription_core_1__get_subscription_MaxNotifsPerPublish(subscription_core__l_subscription,
         &subscription_core__l_max_configured_notifications);
      subscription_core__l_NotificationAvailable = ((subscription_core__l_nb_avail_data_notifs > 0) ||
         (subscription_core__l_nb_avail_event_notifs > 0));
      subscription_core_1__get_subscription_notifRepublishQueue(subscription_core__l_subscription,
         &subscription_core__l_notifRepublishQueue);
      *subscription_core__moreNotifs = false;
      *subscription_core__subscription = constants__c_subscription_indet;
      if (((subscription_core__l_subscriptionState == constants__e_subscriptionState_normal) &&
         ((subscription_core__l_PublishingEnabled == false) ||
         ((subscription_core__l_PublishingEnabled == true) &&
         (subscription_core__l_MoreNotifications == false)))) ||
         (subscription_core__l_subscriptionState == constants__e_subscriptionState_keepAlive)) {
         publish_request_queue_bs__get_nb_publish_requests(subscription_core__l_PublishingReqQueue,
            &subscription_core__l_nb_pub_reqs);
         if (subscription_core__l_nb_pub_reqs == constants__k_n_publishRequestPerSub_max) {
            publish_request_queue_bs__discard_oldest_publish_request(subscription_core__l_PublishingReqQueue,
               &subscription_core__l_old_session,
               &subscription_core__l_old_resp_msg,
               &subscription_core__l_old_req_handle,
               &subscription_core__l_old_req_ctx);
            msg_subscription_publish_bs__generate_internal_send_publish_response_event(subscription_core__l_old_session,
               subscription_core__l_old_resp_msg,
               subscription_core__l_old_req_handle,
               subscription_core__l_old_req_ctx,
               constants_statuscodes_bs__e_sc_bad_too_many_publish_requests);
         }
         publish_request_queue_bs__append_publish_request_to_queue(subscription_core__l_PublishingReqQueue,
            subscription_core__p_session,
            subscription_core__p_req_exp_time,
            subscription_core__p_req_handle,
            subscription_core__p_req_ctx,
            subscription_core__p_resp_msg,
            &subscription_core__l_bres);
         if (subscription_core__l_bres == true) {
            *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
            *subscription_core__async_resp_msg = true;
         }
         else {
            *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_publish_requests;
            *subscription_core__async_resp_msg = false;
         }
      }
      else if (((subscription_core__l_subscriptionState == constants__e_subscriptionState_normal) &&
         (subscription_core__l_PublishingEnabled == true)) &&
         (subscription_core__l_MoreNotifications == true)) {
         *subscription_core__subscription = subscription_core__l_subscription;
         *subscription_core__async_resp_msg = false;
         subscription_core_1__reset_subscription_LifetimeCounter(subscription_core__l_subscription);
         subscription_core__local_compute_msg_nb_notifs(subscription_core__l_max_configured_notifications,
            subscription_core__l_nb_avail_data_notifs,
            subscription_core__l_nb_avail_event_notifs,
            &subscription_core__l_nb_data_notifs,
            &subscription_core__l_nb_event_notifs,
            subscription_core__moreNotifs);
         msg_subscription_publish_bs__alloc_notification_message_items(subscription_core__p_resp_msg,
            subscription_core__l_nb_data_notifs,
            subscription_core__l_nb_event_notifs,
            &subscription_core__l_bres,
            &subscription_core__l_notifMsg);
         if (subscription_core__l_bres == true) {
            subscription_core_1__get_subscription_SeqNum(subscription_core__l_subscription,
               &subscription_core__l_seq_num);
            msg_subscription_publish_bs__set_notification_message_sequence_number(subscription_core__l_notifMsg,
               subscription_core__l_seq_num);
            subscription_core_bs__get_next_subscription_sequence_number(subscription_core__l_seq_num,
               &subscription_core__l_next_seq_num);
            subscription_core_1__set_subscription_SeqNum(subscription_core__l_subscription,
               subscription_core__l_next_seq_num);
            subscription_core__local_fill_data_notification_message(subscription_core__l_subscription,
               subscription_core__l_notifMsg,
               subscription_core__l_nb_data_notifs);
            subscription_core__local_fill_event_notification_message(subscription_core__l_subscription,
               subscription_core__l_notifMsg,
               subscription_core__l_nb_event_notifs);
            notification_republish_queue_bs__get_nb_republish_notifs(subscription_core__l_notifRepublishQueue,
               &subscription_core__l_nb_repub_notifs);
            if (subscription_core__l_nb_repub_notifs == constants__k_n_republishNotifPerSub_max) {
               notification_republish_queue_bs__discard_oldest_republish_notif(subscription_core__l_notifRepublishQueue);
            }
            notification_republish_queue_bs__add_republish_notif_to_queue(subscription_core__l_notifRepublishQueue,
               subscription_core__l_seq_num,
               subscription_core__l_notifMsg,
               &subscription_core__l_bres);
            *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
         }
         else {
            *subscription_core__moreNotifs = true;
            *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
         subscription_core_1__set_subscription_MoreNotifications(subscription_core__l_subscription,
            *subscription_core__moreNotifs);
         subscription_core_1__set_subscription_MessageSent(subscription_core__l_subscription);
      }
      else if (((subscription_core__l_subscriptionState == constants__e_subscriptionState_late) &&
         (subscription_core__l_PublishingEnabled == true)) &&
         ((subscription_core__l_MoreNotifications == true) ||
         (subscription_core__l_NotificationAvailable == true))) {
         subscription_core_1__set_subscription_state(subscription_core__l_subscription,
            constants__e_subscriptionState_normal);
         *subscription_core__subscription = subscription_core__l_subscription;
         *subscription_core__async_resp_msg = false;
         subscription_core_1__reset_subscription_LifetimeCounter(subscription_core__l_subscription);
         subscription_core__local_compute_msg_nb_notifs(subscription_core__l_max_configured_notifications,
            subscription_core__l_nb_avail_data_notifs,
            subscription_core__l_nb_avail_event_notifs,
            &subscription_core__l_nb_data_notifs,
            &subscription_core__l_nb_event_notifs,
            subscription_core__moreNotifs);
         msg_subscription_publish_bs__alloc_notification_message_items(subscription_core__p_resp_msg,
            subscription_core__l_nb_data_notifs,
            subscription_core__l_nb_event_notifs,
            &subscription_core__l_bres,
            &subscription_core__l_notifMsg);
         if (subscription_core__l_bres == true) {
            subscription_core_1__get_subscription_SeqNum(subscription_core__l_subscription,
               &subscription_core__l_seq_num);
            msg_subscription_publish_bs__set_notification_message_sequence_number(subscription_core__l_notifMsg,
               subscription_core__l_seq_num);
            subscription_core_bs__get_next_subscription_sequence_number(subscription_core__l_seq_num,
               &subscription_core__l_next_seq_num);
            subscription_core_1__set_subscription_SeqNum(subscription_core__l_subscription,
               subscription_core__l_next_seq_num);
            subscription_core__local_fill_data_notification_message(subscription_core__l_subscription,
               subscription_core__l_notifMsg,
               subscription_core__l_nb_data_notifs);
            subscription_core__local_fill_event_notification_message(subscription_core__l_subscription,
               subscription_core__l_notifMsg,
               subscription_core__l_nb_event_notifs);
            notification_republish_queue_bs__get_nb_republish_notifs(subscription_core__l_notifRepublishQueue,
               &subscription_core__l_nb_repub_notifs);
            if (subscription_core__l_nb_repub_notifs == constants__k_n_republishNotifPerSub_max) {
               notification_republish_queue_bs__discard_oldest_republish_notif(subscription_core__l_notifRepublishQueue);
            }
            notification_republish_queue_bs__add_republish_notif_to_queue(subscription_core__l_notifRepublishQueue,
               subscription_core__l_seq_num,
               subscription_core__l_notifMsg,
               &subscription_core__l_bres);
            *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
         }
         else {
            *subscription_core__moreNotifs = true;
            *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
         subscription_core_1__set_subscription_MoreNotifications(subscription_core__l_subscription,
            *subscription_core__moreNotifs);
         subscription_core_1__set_subscription_MessageSent(subscription_core__l_subscription);
      }
      else if ((subscription_core__l_subscriptionState == constants__e_subscriptionState_late) &&
         ((subscription_core__l_PublishingEnabled == false) ||
         (((subscription_core__l_PublishingEnabled == true) &&
         (subscription_core__l_NotificationAvailable == false)) &&
         (subscription_core__l_MoreNotifications == false)))) {
         subscription_core_1__set_subscription_state(subscription_core__l_subscription,
            constants__e_subscriptionState_keepAlive);
         *subscription_core__subscription = subscription_core__l_subscription;
         *subscription_core__async_resp_msg = false;
         subscription_core_1__reset_subscription_LifetimeCounter(subscription_core__l_subscription);
         *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
         msg_subscription_publish_bs__get_notification_message_no_items(subscription_core__p_resp_msg,
            &subscription_core__l_notifMsg);
         subscription_core_1__get_subscription_SeqNum(subscription_core__l_subscription,
            &subscription_core__l_seq_num);
         msg_subscription_publish_bs__set_notification_message_sequence_number(subscription_core__l_notifMsg,
            subscription_core__l_seq_num);
         subscription_core_1__set_subscription_MessageSent(subscription_core__l_subscription);
      }
      else {
         *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_bad_invalid_state;
         *subscription_core__async_resp_msg = false;
      }
   }
}

void subscription_core__compute_create_monitored_item_revised_params(
   const constants__t_AttributeId_i subscription_core__p_aid,
   const t_entier4 subscription_core__p_reqQueueSize,
   constants__t_opcua_duration_i * const subscription_core__revisedSamplingItv,
   t_entier4 * const subscription_core__revisedQueueSize) {
   subscription_core__local_compute_create_monitored_item_revised_params(subscription_core__p_aid,
      subscription_core__p_reqQueueSize,
      subscription_core__revisedSamplingItv,
      subscription_core__revisedQueueSize);
}

void subscription_core__create_monitored_item(
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
   constants__t_filterResult_i * const subscription_core__filterResult) {
   {
      constants__t_monitoringFilterCtx_i subscription_core__l_filterCtx;
      t_bool subscription_core__l_isEvent;
      t_bool subscription_core__l_bres;
      constants__t_monitoredItemQueue_i subscription_core__l_sub_monitIt_queue;
      constants__t_monitoredItemQueue_i subscription_core__l_node_monitIt_queue;
      constants__t_notificationQueue_i subscription_core__l_sub_notif_queue;
      constants__t_Timestamp subscription_core__l_ts_src;
      constants__t_Timestamp subscription_core__l_ts_srv;
      
      *subscription_core__monitoredItemPointer = constants__c_monitoredItemPointer_indet;
      *subscription_core__monitoredItemId = constants__c_monitoredItemId_indet;
      monitored_item_filter_treatment__check_monitored_item_filter_valid_and_fill_result(subscription_core__p_endpoint_idx,
         subscription_core__p_nid,
         subscription_core__p_aid,
         subscription_core__p_filter,
         subscription_core__p_value,
         subscription_core__StatusCode_service,
         &subscription_core__l_filterCtx,
         subscription_core__filterResult,
         &subscription_core__l_isEvent);
      if (*subscription_core__StatusCode_service == constants_statuscodes_bs__e_sc_ok) {
         monitored_item_pointer_bs__create_monitored_item_pointer(subscription_core__p_subscription,
            subscription_core__p_nid,
            subscription_core__p_aid,
            subscription_core__p_indexRange,
            subscription_core__p_timestampToReturn,
            subscription_core__p_monitoringMode,
            subscription_core__p_clientHandle,
            subscription_core__l_filterCtx,
            subscription_core__p_discardOldest,
            subscription_core__p_queueSize,
            subscription_core__StatusCode_service,
            subscription_core__monitoredItemPointer,
            subscription_core__monitoredItemId);
      }
      if (*subscription_core__StatusCode_service == constants_statuscodes_bs__e_sc_ok) {
         subscription_core_bs__get_nodeToMonitoredItemQueue(subscription_core__p_nid,
            &subscription_core__l_bres,
            &subscription_core__l_node_monitIt_queue);
         if (subscription_core__l_bres == true) {
            subscription_core_1__get_subscription_monitoredItemQueue(subscription_core__p_subscription,
               &subscription_core__l_sub_monitIt_queue);
            monitored_item_queue_bs__add_monitored_item_to_queue(subscription_core__l_node_monitIt_queue,
               *subscription_core__monitoredItemPointer,
               &subscription_core__l_bres);
            if (subscription_core__l_bres == true) {
               monitored_item_queue_bs__add_monitored_item_to_queue(subscription_core__l_sub_monitIt_queue,
                  *subscription_core__monitoredItemPointer,
                  &subscription_core__l_bres);
               if (subscription_core__l_bres == false) {
                  monitored_item_queue_bs__remove_monitored_item(subscription_core__l_node_monitIt_queue,
                     *subscription_core__monitoredItemPointer,
                     &subscription_core__l_bres);
                  subscription_core__l_bres = false;
               }
            }
            if (subscription_core__l_bres == true) {
               monitored_item_notification_queue_bs__allocate_new_monitored_item_notification_queue(*subscription_core__monitoredItemPointer,
                  subscription_core__l_isEvent,
                  &subscription_core__l_bres,
                  &subscription_core__l_sub_notif_queue);
               if (subscription_core__l_bres == true) {
                  if ((subscription_core__p_monitoringMode != constants__e_monitoringMode_disabled) &&
                     (subscription_core__p_aid != constants__e_aid_EventNotifier)) {
                     switch (subscription_core__p_timestampToReturn) {
                     case constants__e_ttr_source:
                        subscription_core__l_ts_src = subscription_core__p_val_ts_src;
                        subscription_core__l_ts_srv = constants__c_Timestamp_null;
                        break;
                     case constants__e_ttr_server:
                        subscription_core__l_ts_src = constants__c_Timestamp_null;
                        subscription_core__l_ts_srv = subscription_core__p_val_ts_srv;
                        break;
                     case constants__e_ttr_neither:
                        subscription_core__l_ts_src = constants__c_Timestamp_null;
                        subscription_core__l_ts_srv = constants__c_Timestamp_null;
                        break;
                     default:
                        subscription_core__l_ts_src = subscription_core__p_val_ts_src;
                        subscription_core__l_ts_srv = subscription_core__p_val_ts_srv;
                        break;
                     }
                     monitored_item_notification_queue_bs__add_first_monitored_item_notification_to_queue(*subscription_core__monitoredItemPointer,
                        subscription_core__l_sub_notif_queue,
                        subscription_core__p_nid,
                        subscription_core__p_aid,
                        subscription_core__p_value,
                        subscription_core__p_valueSc,
                        subscription_core__l_ts_src,
                        subscription_core__l_ts_srv,
                        &subscription_core__l_bres);
                  }
               }
               if (subscription_core__l_bres == false) {
                  monitored_item_queue_bs__remove_monitored_item(subscription_core__l_node_monitIt_queue,
                     *subscription_core__monitoredItemPointer,
                     &subscription_core__l_bres);
                  monitored_item_queue_bs__remove_monitored_item(subscription_core__l_sub_monitIt_queue,
                     *subscription_core__monitoredItemPointer,
                     &subscription_core__l_bres);
                  subscription_core__l_bres = false;
               }
            }
         }
         if (subscription_core__l_bres == false) {
            *subscription_core__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            monitored_item_pointer_bs__delete_monitored_item_pointer(*subscription_core__monitoredItemPointer);
         }
      }
      else {
         monitored_item_filter_treatment__delete_event_filter_context(subscription_core__l_filterCtx);
      }
   }
}

void subscription_core__modify_monitored_item(
   const constants__t_endpoint_config_idx_i subscription_core__p_endpoint_idx,
   const constants__t_monitoredItemId_i subscription_core__p_mi_id,
   const constants__t_TimestampsToReturn_i subscription_core__p_timestampToReturn,
   const constants__t_client_handle_i subscription_core__p_clientHandle,
   const constants__t_monitoringFilter_i subscription_core__p_filter,
   const t_bool subscription_core__p_discardOldest,
   const t_entier4 subscription_core__p_queueSize,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_sc,
   constants__t_filterResult_i * const subscription_core__p_filterResult,
   constants__t_opcua_duration_i * const subscription_core__p_revSamplingItv,
   t_entier4 * const subscription_core__p_revQueueSize) {
   {
      t_bool subscription_core__bres;
      constants__t_monitoredItemPointer_i subscription_core__l_monitoredItemPointer;
      constants__t_monitoredItemId_i subscription_core__l_monitoredItemId;
      constants__t_subscription_i subscription_core__l_subscription;
      constants__t_NodeId_i subscription_core__l_nid;
      constants__t_AttributeId_i subscription_core__l_aid;
      constants__t_IndexRange_i subscription_core__l_indexRange;
      constants__t_TimestampsToReturn_i subscription_core__l_timestampToReturn;
      constants__t_monitoringMode_i subscription_core__l_monitoringMode;
      constants__t_client_handle_i subscription_core__l_clientHandle;
      constants__t_monitoringFilterCtx_i subscription_core__l_filterCtx;
      t_bool subscription_core__l_isEvent;
      
      monitored_item_pointer_bs__getall_monitoredItemId(subscription_core__p_mi_id,
         &subscription_core__bres,
         &subscription_core__l_monitoredItemPointer);
      *subscription_core__p_filterResult = constants__c_filterResult_indet;
      *subscription_core__p_revSamplingItv = constants__c_opcua_duration_zero;
      *subscription_core__p_revQueueSize = 0;
      if (subscription_core__bres == true) {
         monitored_item_pointer_bs__getall_monitoredItemPointer(subscription_core__l_monitoredItemPointer,
            &subscription_core__l_monitoredItemId,
            &subscription_core__l_subscription,
            &subscription_core__l_nid,
            &subscription_core__l_aid,
            &subscription_core__l_indexRange,
            &subscription_core__l_timestampToReturn,
            &subscription_core__l_monitoringMode,
            &subscription_core__l_clientHandle);
         subscription_core__local_compute_create_monitored_item_revised_params(subscription_core__l_aid,
            subscription_core__p_queueSize,
            subscription_core__p_revSamplingItv,
            subscription_core__p_revQueueSize);
         monitored_item_filter_treatment__check_monitored_item_filter_valid_and_fill_result(subscription_core__p_endpoint_idx,
            subscription_core__l_nid,
            subscription_core__l_aid,
            subscription_core__p_filter,
            constants__c_Variant_indet,
            subscription_core__p_sc,
            &subscription_core__l_filterCtx,
            subscription_core__p_filterResult,
            &subscription_core__l_isEvent);
         if (*subscription_core__p_sc == constants_statuscodes_bs__e_sc_ok) {
            monitored_item_pointer_bs__modify_monitored_item_pointer(subscription_core__l_monitoredItemPointer,
               subscription_core__p_timestampToReturn,
               subscription_core__p_clientHandle,
               subscription_core__l_filterCtx,
               subscription_core__p_discardOldest,
               *subscription_core__p_revQueueSize,
               subscription_core__p_sc);
            if (*subscription_core__p_sc == constants_statuscodes_bs__e_sc_ok) {
               monitored_item_notification_queue_bs__resize_monitored_item_notification_queue(subscription_core__l_monitoredItemPointer);
            }
         }
      }
      else {
         *subscription_core__p_sc = constants_statuscodes_bs__e_sc_bad_monitored_item_id_invalid;
      }
   }
}

void subscription_core__delete_monitored_item(
   const constants__t_subscription_i subscription_core__p_subscription,
   const constants__t_monitoredItemId_i subscription_core__p_mi_id,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_sc) {
   {
      constants__t_monitoredItemQueue_i subscription_core__l_subscription_queue;
      constants__t_monitoredItemPointer_i subscription_core__l_monitoredItemPointer;
      constants__t_monitoredItemId_i subscription_core__l_monitoredItemId;
      constants__t_subscription_i subscription_core__l_subscription;
      constants__t_NodeId_i subscription_core__l_nid;
      constants__t_AttributeId_i subscription_core__l_aid;
      constants__t_IndexRange_i subscription_core__l_indexRange;
      constants__t_TimestampsToReturn_i subscription_core__l_timestampToReturn;
      constants__t_monitoringMode_i subscription_core__l_monitoringMode;
      constants__t_client_handle_i subscription_core__l_clientHandle;
      t_bool subscription_core__l_isMonitoredItemFound;
      constants__t_monitoredItemQueue_i subscription_core__l_monitored_item_queue;
      t_bool subscription_core__l_bres;
      constants__t_notificationQueue_i subscription_core__l_notificationQueue;
      
      subscription_core__l_isMonitoredItemFound = false;
      subscription_core__l_monitoredItemPointer = constants__c_monitoredItemPointer_indet;
      subscription_core__l_monitored_item_queue = constants__c_monitoredItemQueue_indet;
      subscription_core__l_nid = constants__c_NodeId_indet;
      monitored_item_pointer_bs__getall_monitoredItemId(subscription_core__p_mi_id,
         &subscription_core__l_isMonitoredItemFound,
         &subscription_core__l_monitoredItemPointer);
      if (subscription_core__l_isMonitoredItemFound == true) {
         *subscription_core__p_sc = constants_statuscodes_bs__e_sc_ok;
         monitored_item_pointer_bs__getall_monitoredItemPointer(subscription_core__l_monitoredItemPointer,
            &subscription_core__l_monitoredItemId,
            &subscription_core__l_subscription,
            &subscription_core__l_nid,
            &subscription_core__l_aid,
            &subscription_core__l_indexRange,
            &subscription_core__l_timestampToReturn,
            &subscription_core__l_monitoringMode,
            &subscription_core__l_clientHandle);
         subscription_core_1__get_subscription_monitoredItemQueue(subscription_core__p_subscription,
            &subscription_core__l_subscription_queue);
         monitored_item_queue_bs__remove_monitored_item(subscription_core__l_subscription_queue,
            subscription_core__l_monitoredItemPointer,
            &subscription_core__l_bres);
         subscription_core_bs__get_nodeToMonitoredItemQueue(subscription_core__l_nid,
            &subscription_core__l_bres,
            &subscription_core__l_monitored_item_queue);
         if (subscription_core__l_bres == true) {
            monitored_item_queue_bs__remove_monitored_item(subscription_core__l_monitored_item_queue,
               subscription_core__l_monitoredItemPointer,
               &subscription_core__l_bres);
         }
         monitored_item_notification_queue_bs__get_monitored_item_notification_queue(subscription_core__l_monitoredItemPointer,
            &subscription_core__l_bres,
            &subscription_core__l_notificationQueue);
         if (subscription_core__l_bres == true) {
            monitored_item_notification_queue_bs__clear_and_deallocate_monitored_item_notification_queue(subscription_core__l_monitoredItemPointer,
               subscription_core__l_notificationQueue);
         }
         monitored_item_pointer_bs__delete_monitored_item_pointer(subscription_core__l_monitoredItemPointer);
      }
      else {
         *subscription_core__p_sc = constants_statuscodes_bs__e_sc_bad_monitored_item_id_invalid;
      }
   }
}

void subscription_core__set_monit_mode_monitored_item(
   const constants__t_monitoredItemId_i subscription_core__p_mi_id,
   const constants__t_monitoringMode_i subscription_core__p_monitoring_mode,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_sc,
   constants__t_monitoredItemPointer_i * const subscription_core__p_mi_pointer,
   constants__t_monitoringMode_i * const subscription_core__p_prevMonitMode) {
   {
      t_bool subscription_core__l_isMonitoredItemFound;
      constants__t_monitoredItemPointer_i subscription_core__l_monitoredItemPointer;
      constants__t_monitoredItemId_i subscription_core__l_monitoredItemId;
      constants__t_subscription_i subscription_core__l_subscription;
      constants__t_NodeId_i subscription_core__l_nid;
      constants__t_AttributeId_i subscription_core__l_aid;
      constants__t_IndexRange_i subscription_core__l_indexRange;
      constants__t_TimestampsToReturn_i subscription_core__l_timestampToReturn;
      constants__t_client_handle_i subscription_core__l_clientHandle;
      
      *subscription_core__p_mi_pointer = constants__c_monitoredItemPointer_indet;
      *subscription_core__p_prevMonitMode = constants__c_monitoringMode_indet;
      subscription_core__l_monitoredItemPointer = constants__c_monitoredItemPointer_indet;
      monitored_item_pointer_bs__getall_monitoredItemId(subscription_core__p_mi_id,
         &subscription_core__l_isMonitoredItemFound,
         &subscription_core__l_monitoredItemPointer);
      if (subscription_core__l_isMonitoredItemFound == true) {
         *subscription_core__p_mi_pointer = subscription_core__l_monitoredItemPointer;
         monitored_item_pointer_bs__getall_monitoredItemPointer(subscription_core__l_monitoredItemPointer,
            &subscription_core__l_monitoredItemId,
            &subscription_core__l_subscription,
            &subscription_core__l_nid,
            &subscription_core__l_aid,
            &subscription_core__l_indexRange,
            &subscription_core__l_timestampToReturn,
            subscription_core__p_prevMonitMode,
            &subscription_core__l_clientHandle);
         monitored_item_pointer_bs__set_monit_mode_monitored_item_pointer(subscription_core__l_monitoredItemPointer,
            subscription_core__p_monitoring_mode);
         *subscription_core__p_sc = constants_statuscodes_bs__e_sc_ok;
      }
      else {
         *subscription_core__p_sc = constants_statuscodes_bs__e_sc_bad_monitored_item_id_invalid;
      }
   }
}

void subscription_core__clear_monitored_item_notifications(
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer) {
   {
      t_bool subscription_core__l_bres;
      constants__t_notificationQueue_i subscription_core__l_notifQueue;
      
      monitored_item_notification_queue_bs__get_monitored_item_notification_queue(subscription_core__p_monitoredItemPointer,
         &subscription_core__l_bres,
         &subscription_core__l_notifQueue);
      if (subscription_core__l_bres == true) {
         monitored_item_notification_queue_bs__clear_monitored_item_notification_queue(subscription_core__p_monitoredItemPointer,
            subscription_core__l_notifQueue);
      }
   }
}

void subscription_core__server_subscription_core_check_valid_publish_req_queue(
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__p_validPublishingReqQueued) {
   subscription_core__pop_invalid_and_check_valid_publishReqQueued(subscription_core__p_subscription,
      subscription_core__p_validPublishingReqQueued);
}

void subscription_core__server_subscription_core_publish_timeout_check_lifetime(
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__p_close_sub,
   t_bool * const subscription_core__p_msg_to_send,
   constants__t_session_i * const subscription_core__p_session,
   constants__t_msg_i * const subscription_core__p_publish_resp_msg,
   constants__t_server_request_handle_i * const subscription_core__p_req_handle,
   constants__t_request_context_i * const subscription_core__p_req_context,
   t_bool * const subscription_core__p_validPubReqQueued) {
   {
      t_entier4 subscription_core__l_lifetimeCounter;
      
      *subscription_core__p_close_sub = false;
      *subscription_core__p_msg_to_send = false;
      *subscription_core__p_session = constants__c_session_indet;
      *subscription_core__p_publish_resp_msg = constants__c_msg_indet;
      *subscription_core__p_req_handle = constants__c_server_request_handle_any;
      *subscription_core__p_req_context = constants__c_request_context_indet;
      subscription_core__pop_invalid_and_check_valid_publishReqQueued(subscription_core__p_subscription,
         subscription_core__p_validPubReqQueued);
      if (*subscription_core__p_validPubReqQueued == false) {
         subscription_core_1__get_subscription_LifetimeCounter(subscription_core__p_subscription,
            &subscription_core__l_lifetimeCounter);
         if (subscription_core__l_lifetimeCounter <= 1) {
            *subscription_core__p_close_sub = true;
         }
         else {
            subscription_core_1__decrement_subscription_LifetimeCounter(subscription_core__p_subscription);
         }
      }
   }
}

void subscription_core__server_subscription_core_publish_timeout(
   const constants__t_subscription_i subscription_core__p_subscription,
   const t_bool subscription_core__p_validPublishReqQueued,
   t_bool * const subscription_core__p_msg_to_send,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_msg_sc,
   constants__t_session_i * const subscription_core__p_session,
   constants__t_msg_i * const subscription_core__p_publish_resp_msg,
   constants__t_server_request_handle_i * const subscription_core__p_req_handle,
   constants__t_request_context_i * const subscription_core__p_req_context,
   t_bool * const subscription_core__p_moreNotifs) {
   {
      t_bool subscription_core__l_bres;
      constants__t_subscriptionState_i subscription_core__l_State;
      constants__t_publishReqQueue_i subscription_core__l_PublishingReqQueue;
      t_bool subscription_core__l_PublishingEnabled;
      t_entier4 subscription_core__l_nb_avail_data_notifs;
      t_entier4 subscription_core__l_nb_avail_event_notifs;
      t_entier4 subscription_core__l_nb_data_notifs;
      t_entier4 subscription_core__l_nb_event_notifs;
      t_entier4 subscription_core__l_max_configured_notifications;
      t_bool subscription_core__l_NotificationAvailable;
      t_bool subscription_core__l_MessageSent;
      t_entier4 subscription_core__l_KeepAliveCounter;
      constants__t_notifRepublishQueue_i subscription_core__l_notifRepublishQueue;
      t_entier4 subscription_core__l_nb_repub_notifs;
      constants__t_notif_msg_i subscription_core__l_notifMsg;
      constants__t_timeref_i subscription_core__l_req_exp_time;
      constants__t_sub_seq_num_i subscription_core__l_seq_num;
      constants__t_sub_seq_num_i subscription_core__l_next_seq_num;
      
      *subscription_core__p_msg_to_send = false;
      *subscription_core__p_session = constants__c_session_indet;
      *subscription_core__p_publish_resp_msg = constants__c_msg_indet;
      *subscription_core__p_req_handle = constants__c_server_request_handle_any;
      *subscription_core__p_req_context = constants__c_request_context_indet;
      *subscription_core__p_moreNotifs = false;
      *subscription_core__p_msg_sc = constants_statuscodes_bs__c_StatusCode_indet;
      subscription_core_1__get_subscription_state(subscription_core__p_subscription,
         &subscription_core__l_State);
      subscription_core_1__get_subscription_publishRequestQueue(subscription_core__p_subscription,
         &subscription_core__l_PublishingReqQueue);
      subscription_core_1__get_subscription_PublishingEnabled(subscription_core__p_subscription,
         &subscription_core__l_PublishingEnabled);
      subscription_core__local_subscription_nb_available_notifications(subscription_core__p_subscription,
         &subscription_core__l_nb_avail_data_notifs,
         &subscription_core__l_nb_avail_event_notifs);
      subscription_core_1__get_subscription_MaxNotifsPerPublish(subscription_core__p_subscription,
         &subscription_core__l_max_configured_notifications);
      subscription_core__l_NotificationAvailable = ((subscription_core__l_nb_avail_data_notifs > 0) ||
         (subscription_core__l_nb_avail_event_notifs > 0));
      subscription_core_1__get_subscription_MessageSent(subscription_core__p_subscription,
         &subscription_core__l_MessageSent);
      subscription_core_1__get_subscription_KeepAliveCounter(subscription_core__p_subscription,
         &subscription_core__l_KeepAliveCounter);
      subscription_core_1__get_subscription_notifRepublishQueue(subscription_core__p_subscription,
         &subscription_core__l_notifRepublishQueue);
      if ((((subscription_core__l_State == constants__e_subscriptionState_normal) &&
         (subscription_core__p_validPublishReqQueued == true)) &&
         (subscription_core__l_PublishingEnabled == true)) &&
         (subscription_core__l_NotificationAvailable == true)) {
         subscription_core_1__reset_subscription_LifetimeCounter(subscription_core__p_subscription);
         publish_request_queue_bs__pop_valid_publish_request_queue(subscription_core__l_PublishingReqQueue,
            subscription_core__p_session,
            &subscription_core__l_req_exp_time,
            subscription_core__p_req_handle,
            subscription_core__p_req_context,
            subscription_core__p_publish_resp_msg);
         *subscription_core__p_msg_to_send = true;
         subscription_core__local_compute_msg_nb_notifs(subscription_core__l_max_configured_notifications,
            subscription_core__l_nb_avail_data_notifs,
            subscription_core__l_nb_avail_event_notifs,
            &subscription_core__l_nb_data_notifs,
            &subscription_core__l_nb_event_notifs,
            subscription_core__p_moreNotifs);
         msg_subscription_publish_bs__alloc_notification_message_items(*subscription_core__p_publish_resp_msg,
            subscription_core__l_nb_data_notifs,
            subscription_core__l_nb_event_notifs,
            &subscription_core__l_bres,
            &subscription_core__l_notifMsg);
         if (subscription_core__l_bres == true) {
            subscription_core_1__get_subscription_SeqNum(subscription_core__p_subscription,
               &subscription_core__l_seq_num);
            msg_subscription_publish_bs__set_notification_message_sequence_number(subscription_core__l_notifMsg,
               subscription_core__l_seq_num);
            subscription_core_bs__get_next_subscription_sequence_number(subscription_core__l_seq_num,
               &subscription_core__l_next_seq_num);
            subscription_core_1__set_subscription_SeqNum(subscription_core__p_subscription,
               subscription_core__l_next_seq_num);
            subscription_core__local_fill_data_notification_message(subscription_core__p_subscription,
               subscription_core__l_notifMsg,
               subscription_core__l_nb_data_notifs);
            subscription_core__local_fill_event_notification_message(subscription_core__p_subscription,
               subscription_core__l_notifMsg,
               subscription_core__l_nb_event_notifs);
            notification_republish_queue_bs__get_nb_republish_notifs(subscription_core__l_notifRepublishQueue,
               &subscription_core__l_nb_repub_notifs);
            if (subscription_core__l_nb_repub_notifs == constants__k_n_republishNotifPerSub_max) {
               notification_republish_queue_bs__discard_oldest_republish_notif(subscription_core__l_notifRepublishQueue);
            }
            notification_republish_queue_bs__add_republish_notif_to_queue(subscription_core__l_notifRepublishQueue,
               subscription_core__l_seq_num,
               subscription_core__l_notifMsg,
               &subscription_core__l_bres);
            *subscription_core__p_msg_sc = constants_statuscodes_bs__e_sc_ok;
         }
         else {
            *subscription_core__p_moreNotifs = true;
            *subscription_core__p_msg_sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
         subscription_core_1__set_subscription_MoreNotifications(subscription_core__p_subscription,
            *subscription_core__p_moreNotifs);
         subscription_core_1__set_subscription_MessageSent(subscription_core__p_subscription);
      }
      else if ((((subscription_core__l_State == constants__e_subscriptionState_normal) &&
         (subscription_core__p_validPublishReqQueued == true)) &&
         (subscription_core__l_MessageSent == false)) &&
         ((subscription_core__l_PublishingEnabled == false) ||
         ((subscription_core__l_PublishingEnabled == true) &&
         (subscription_core__l_NotificationAvailable == false)))) {
         subscription_core_1__reset_subscription_LifetimeCounter(subscription_core__p_subscription);
         publish_request_queue_bs__pop_valid_publish_request_queue(subscription_core__l_PublishingReqQueue,
            subscription_core__p_session,
            &subscription_core__l_req_exp_time,
            subscription_core__p_req_handle,
            subscription_core__p_req_context,
            subscription_core__p_publish_resp_msg);
         msg_subscription_publish_bs__set_publish_response_msg(*subscription_core__p_publish_resp_msg);
         *subscription_core__p_msg_to_send = true;
         *subscription_core__p_msg_sc = constants_statuscodes_bs__e_sc_ok;
         msg_subscription_publish_bs__get_notification_message_no_items(*subscription_core__p_publish_resp_msg,
            &subscription_core__l_notifMsg);
         subscription_core_1__get_subscription_SeqNum(subscription_core__p_subscription,
            &subscription_core__l_seq_num);
         msg_subscription_publish_bs__set_notification_message_sequence_number(subscription_core__l_notifMsg,
            subscription_core__l_seq_num);
         subscription_core_1__set_subscription_MessageSent(subscription_core__p_subscription);
      }
      else if (((subscription_core__l_State == constants__e_subscriptionState_normal) &&
         (subscription_core__p_validPublishReqQueued == false)) &&
         ((subscription_core__l_MessageSent == false) ||
         ((subscription_core__l_PublishingEnabled == true) &&
         (subscription_core__l_NotificationAvailable == true)))) {
         subscription_core_1__set_subscription_state(subscription_core__p_subscription,
            constants__e_subscriptionState_late);
      }
      else if (((subscription_core__l_State == constants__e_subscriptionState_normal) &&
         (subscription_core__l_MessageSent == true)) &&
         ((subscription_core__l_PublishingEnabled == false) ||
         ((subscription_core__l_PublishingEnabled == true) &&
         (subscription_core__l_NotificationAvailable == false)))) {
         subscription_core_1__set_subscription_state(subscription_core__p_subscription,
            constants__e_subscriptionState_keepAlive);
         subscription_core_1__reset_subscription_KeepAliveCounter(subscription_core__p_subscription);
         subscription_core_1__decrement_subscription_KeepAliveCounter(subscription_core__p_subscription);
      }
      else if (subscription_core__l_State == constants__e_subscriptionState_late) {
         ;
      }
      else if ((((subscription_core__l_State == constants__e_subscriptionState_keepAlive) &&
         (subscription_core__l_PublishingEnabled == true)) &&
         (subscription_core__l_NotificationAvailable == true)) &&
         (subscription_core__p_validPublishReqQueued == true)) {
         subscription_core_1__set_subscription_state(subscription_core__p_subscription,
            constants__e_subscriptionState_normal);
         subscription_core_1__reset_subscription_LifetimeCounter(subscription_core__p_subscription);
         publish_request_queue_bs__pop_valid_publish_request_queue(subscription_core__l_PublishingReqQueue,
            subscription_core__p_session,
            &subscription_core__l_req_exp_time,
            subscription_core__p_req_handle,
            subscription_core__p_req_context,
            subscription_core__p_publish_resp_msg);
         *subscription_core__p_msg_to_send = true;
         subscription_core__local_compute_msg_nb_notifs(subscription_core__l_max_configured_notifications,
            subscription_core__l_nb_avail_data_notifs,
            subscription_core__l_nb_avail_event_notifs,
            &subscription_core__l_nb_data_notifs,
            &subscription_core__l_nb_event_notifs,
            subscription_core__p_moreNotifs);
         msg_subscription_publish_bs__alloc_notification_message_items(*subscription_core__p_publish_resp_msg,
            subscription_core__l_nb_data_notifs,
            subscription_core__l_nb_event_notifs,
            &subscription_core__l_bres,
            &subscription_core__l_notifMsg);
         if (subscription_core__l_bres == true) {
            subscription_core_1__get_subscription_SeqNum(subscription_core__p_subscription,
               &subscription_core__l_seq_num);
            msg_subscription_publish_bs__set_notification_message_sequence_number(subscription_core__l_notifMsg,
               subscription_core__l_seq_num);
            subscription_core_bs__get_next_subscription_sequence_number(subscription_core__l_seq_num,
               &subscription_core__l_next_seq_num);
            subscription_core_1__set_subscription_SeqNum(subscription_core__p_subscription,
               subscription_core__l_next_seq_num);
            subscription_core__local_fill_data_notification_message(subscription_core__p_subscription,
               subscription_core__l_notifMsg,
               subscription_core__l_nb_data_notifs);
            subscription_core__local_fill_event_notification_message(subscription_core__p_subscription,
               subscription_core__l_notifMsg,
               subscription_core__l_nb_event_notifs);
            notification_republish_queue_bs__get_nb_republish_notifs(subscription_core__l_notifRepublishQueue,
               &subscription_core__l_nb_repub_notifs);
            if (subscription_core__l_nb_repub_notifs == constants__k_n_republishNotifPerSub_max) {
               notification_republish_queue_bs__discard_oldest_republish_notif(subscription_core__l_notifRepublishQueue);
            }
            notification_republish_queue_bs__add_republish_notif_to_queue(subscription_core__l_notifRepublishQueue,
               subscription_core__l_seq_num,
               subscription_core__l_notifMsg,
               &subscription_core__l_bres);
            *subscription_core__p_msg_sc = constants_statuscodes_bs__e_sc_ok;
         }
         else {
            *subscription_core__p_moreNotifs = true;
            *subscription_core__p_msg_sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
         subscription_core_1__set_subscription_MoreNotifications(subscription_core__p_subscription,
            *subscription_core__p_moreNotifs);
         subscription_core_1__set_subscription_MessageSent(subscription_core__p_subscription);
      }
      else if ((((subscription_core__l_State == constants__e_subscriptionState_keepAlive) &&
         (subscription_core__p_validPublishReqQueued == true)) &&
         (subscription_core__l_KeepAliveCounter <= 1)) &&
         ((subscription_core__l_PublishingEnabled == false) ||
         ((subscription_core__l_PublishingEnabled == true) &&
         (subscription_core__l_NotificationAvailable == false)))) {
         subscription_core_1__reset_subscription_KeepAliveCounter(subscription_core__p_subscription);
         publish_request_queue_bs__pop_valid_publish_request_queue(subscription_core__l_PublishingReqQueue,
            subscription_core__p_session,
            &subscription_core__l_req_exp_time,
            subscription_core__p_req_handle,
            subscription_core__p_req_context,
            subscription_core__p_publish_resp_msg);
         msg_subscription_publish_bs__set_publish_response_msg(*subscription_core__p_publish_resp_msg);
         *subscription_core__p_msg_to_send = true;
         *subscription_core__p_msg_sc = constants_statuscodes_bs__e_sc_ok;
         msg_subscription_publish_bs__get_notification_message_no_items(*subscription_core__p_publish_resp_msg,
            &subscription_core__l_notifMsg);
         subscription_core_1__get_subscription_SeqNum(subscription_core__p_subscription,
            &subscription_core__l_seq_num);
         msg_subscription_publish_bs__set_notification_message_sequence_number(subscription_core__l_notifMsg,
            subscription_core__l_seq_num);
         subscription_core_1__set_subscription_MessageSent(subscription_core__p_subscription);
      }
      else if (((subscription_core__l_State == constants__e_subscriptionState_keepAlive) &&
         (subscription_core__l_KeepAliveCounter > 1)) &&
         ((subscription_core__l_PublishingEnabled == false) ||
         ((subscription_core__l_PublishingEnabled == true) &&
         (subscription_core__l_NotificationAvailable == false)))) {
         subscription_core_1__decrement_subscription_KeepAliveCounter(subscription_core__p_subscription);
      }
      else if (((subscription_core__l_State == constants__e_subscriptionState_keepAlive) &&
         (subscription_core__p_validPublishReqQueued == false)) &&
         ((subscription_core__l_KeepAliveCounter <= 1) ||
         (((subscription_core__l_KeepAliveCounter > 1) &&
         (subscription_core__l_PublishingEnabled == true)) &&
         (subscription_core__l_NotificationAvailable == true)))) {
         subscription_core_1__set_subscription_state(subscription_core__p_subscription,
            constants__e_subscriptionState_late);
      }
      else {
         ;
      }
   }
}

void subscription_core__server_subscription_core_publish_timeout_return_moreNotifs(
   const constants__t_subscription_i subscription_core__p_subscription,
   t_bool * const subscription_core__p_msg_to_send,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core__p_msg_sc,
   constants__t_session_i * const subscription_core__p_session,
   constants__t_msg_i * const subscription_core__p_publish_resp_msg,
   constants__t_server_request_handle_i * const subscription_core__p_req_handle,
   constants__t_request_context_i * const subscription_core__p_req_context,
   t_bool * const subscription_core__p_moreNotifs) {
   {
      constants__t_timeref_i subscription_core__l_req_exp_time;
      constants__t_publishReqQueue_i subscription_core__l_PublishingReqQueue;
      t_entier4 subscription_core__l_nb_avail_data_notifs;
      t_entier4 subscription_core__l_nb_avail_event_notifs;
      t_entier4 subscription_core__l_nb_data_notifs;
      t_entier4 subscription_core__l_nb_event_notifs;
      t_entier4 subscription_core__l_max_configured_notifications;
      t_entier4 subscription_core__l_nb_repub_notifs;
      t_bool subscription_core__l_bres;
      constants__t_notif_msg_i subscription_core__l_notifMsg;
      constants__t_sub_seq_num_i subscription_core__l_seq_num;
      constants__t_sub_seq_num_i subscription_core__l_next_seq_num;
      constants__t_notifRepublishQueue_i subscription_core__l_notifRepublishQueue;
      
      *subscription_core__p_moreNotifs = false;
      *subscription_core__p_msg_to_send = true;
      subscription_core_1__get_subscription_publishRequestQueue(subscription_core__p_subscription,
         &subscription_core__l_PublishingReqQueue);
      subscription_core__local_subscription_nb_available_notifications(subscription_core__p_subscription,
         &subscription_core__l_nb_avail_data_notifs,
         &subscription_core__l_nb_avail_event_notifs);
      subscription_core_1__get_subscription_MaxNotifsPerPublish(subscription_core__p_subscription,
         &subscription_core__l_max_configured_notifications);
      subscription_core_1__get_subscription_notifRepublishQueue(subscription_core__p_subscription,
         &subscription_core__l_notifRepublishQueue);
      publish_request_queue_bs__pop_valid_publish_request_queue(subscription_core__l_PublishingReqQueue,
         subscription_core__p_session,
         &subscription_core__l_req_exp_time,
         subscription_core__p_req_handle,
         subscription_core__p_req_context,
         subscription_core__p_publish_resp_msg);
      subscription_core__local_compute_msg_nb_notifs(subscription_core__l_max_configured_notifications,
         subscription_core__l_nb_avail_data_notifs,
         subscription_core__l_nb_avail_event_notifs,
         &subscription_core__l_nb_data_notifs,
         &subscription_core__l_nb_event_notifs,
         subscription_core__p_moreNotifs);
      msg_subscription_publish_bs__alloc_notification_message_items(*subscription_core__p_publish_resp_msg,
         subscription_core__l_nb_data_notifs,
         subscription_core__l_nb_event_notifs,
         &subscription_core__l_bres,
         &subscription_core__l_notifMsg);
      if (subscription_core__l_bres == true) {
         subscription_core_1__get_subscription_SeqNum(subscription_core__p_subscription,
            &subscription_core__l_seq_num);
         msg_subscription_publish_bs__set_notification_message_sequence_number(subscription_core__l_notifMsg,
            subscription_core__l_seq_num);
         subscription_core_bs__get_next_subscription_sequence_number(subscription_core__l_seq_num,
            &subscription_core__l_next_seq_num);
         subscription_core_1__set_subscription_SeqNum(subscription_core__p_subscription,
            subscription_core__l_next_seq_num);
         subscription_core__local_fill_data_notification_message(subscription_core__p_subscription,
            subscription_core__l_notifMsg,
            subscription_core__l_nb_data_notifs);
         subscription_core__local_fill_event_notification_message(subscription_core__p_subscription,
            subscription_core__l_notifMsg,
            subscription_core__l_nb_event_notifs);
         notification_republish_queue_bs__get_nb_republish_notifs(subscription_core__l_notifRepublishQueue,
            &subscription_core__l_nb_repub_notifs);
         if (subscription_core__l_nb_repub_notifs == constants__k_n_republishNotifPerSub_max) {
            notification_republish_queue_bs__discard_oldest_republish_notif(subscription_core__l_notifRepublishQueue);
         }
         notification_republish_queue_bs__add_republish_notif_to_queue(subscription_core__l_notifRepublishQueue,
            subscription_core__l_seq_num,
            subscription_core__l_notifMsg,
            &subscription_core__l_bres);
         *subscription_core__p_msg_sc = constants_statuscodes_bs__e_sc_ok;
      }
      else {
         *subscription_core__p_moreNotifs = true;
         *subscription_core__p_msg_sc = constants_statuscodes_bs__e_sc_bad_out_of_memory;
      }
      subscription_core_1__set_subscription_MoreNotifications(subscription_core__p_subscription,
         *subscription_core__p_moreNotifs);
      subscription_core_1__set_subscription_MessageSent(subscription_core__p_subscription);
   }
}

void subscription_core__server_subscription_add_notification_on_event_if_triggered(
   const t_bool subscription_core__p_userAccessGranted,
   const constants__t_LocaleIds_i subscription_core__p_localeIds,
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   const constants__t_client_handle_i subscription_core__p_clientHandle,
   const constants__t_TimestampsToReturn_i subscription_core__p_timestampToReturn,
   const constants__t_Event_i subscription_core__p_event) {
   {
      constants__t_monitoringFilterCtx_i subscription_core__l_filterCtx;
      t_bool subscription_core__l_triggered;
      constants__t_eventFieldList_i subscription_core__l_event_notif;
      t_bool subscription_core__l_bres;
      constants__t_notificationQueue_i subscription_core__l_notif_queue;
      
      monitored_item_pointer_bs__get_monitoredItemFilter(subscription_core__p_monitoredItemPointer,
         &subscription_core__l_filterCtx);
      monitored_item_filter_treatment__server_subscription_get_notification_on_event(subscription_core__p_clientHandle,
         subscription_core__p_localeIds,
         subscription_core__p_timestampToReturn,
         subscription_core__p_userAccessGranted,
         subscription_core__l_filterCtx,
         subscription_core__p_event,
         &subscription_core__l_triggered,
         &subscription_core__l_event_notif);
      monitored_item_notification_queue_bs__get_monitored_item_notification_queue(subscription_core__p_monitoredItemPointer,
         &subscription_core__l_bres,
         &subscription_core__l_notif_queue);
      if ((subscription_core__l_triggered == true) &&
         (subscription_core__l_bres == true)) {
         monitored_item_notification_queue_bs__add_monitored_item_event_notification_to_queue(subscription_core__p_monitoredItemPointer,
            subscription_core__l_notif_queue,
            subscription_core__l_event_notif,
            &subscription_core__l_bres);
      }
   }
}

void subscription_core__server_subscription_add_notification_on_value_change(
   const constants__t_LocaleIds_i subscription_core__p_localeIds,
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   const constants__t_TimestampsToReturn_i subscription_core__p_timestampToReturn,
   const constants__t_WriteValuePointer_i subscription_core__p_writeValuePointer) {
   {
      t_bool subscription_core__l_bres;
      constants__t_notificationQueue_i subscription_core__l_notif_queue;
      t_bool subscription_core__l_is_event;
      
      monitored_item_notification_queue_bs__get_monitored_item_notification_queue(subscription_core__p_monitoredItemPointer,
         &subscription_core__l_bres,
         &subscription_core__l_notif_queue);
      if (subscription_core__l_bres == true) {
         monitored_item_notification_queue_bs__is_event_monitored_item_notification_queue(subscription_core__p_monitoredItemPointer,
            subscription_core__l_notif_queue,
            &subscription_core__l_is_event);
         if (subscription_core__l_is_event == false) {
            monitored_item_notification_queue_bs__add_monitored_item_data_notification_to_queue(subscription_core__p_localeIds,
               subscription_core__p_monitoredItemPointer,
               subscription_core__l_notif_queue,
               subscription_core__p_timestampToReturn,
               subscription_core__p_writeValuePointer,
               &subscription_core__l_bres);
         }
      }
   }
}

void subscription_core__server_subscription_add_notification_on_node_or_monitMode_change(
   const constants__t_monitoredItemPointer_i subscription_core__p_monitoredItemPointer,
   const constants__t_NodeId_i subscription_core__p_nid,
   const constants__t_AttributeId_i subscription_core__p_aid,
   const constants__t_Variant_i subscription_core__p_VariantValuePointer,
   const constants__t_RawStatusCode subscription_core__p_ValueSc,
   const constants__t_Timestamp subscription_core__p_val_ts_src,
   const constants__t_Timestamp subscription_core__p_val_ts_srv,
   t_bool * const subscription_core__bres) {
   {
      constants__t_notificationQueue_i subscription_core__l_notif_queue;
      
      monitored_item_notification_queue_bs__get_monitored_item_notification_queue(subscription_core__p_monitoredItemPointer,
         subscription_core__bres,
         &subscription_core__l_notif_queue);
      if (*subscription_core__bres == true) {
         monitored_item_notification_queue_bs__add_first_monitored_item_notification_to_queue(subscription_core__p_monitoredItemPointer,
            subscription_core__l_notif_queue,
            subscription_core__p_nid,
            subscription_core__p_aid,
            subscription_core__p_VariantValuePointer,
            subscription_core__p_ValueSc,
            subscription_core__p_val_ts_src,
            subscription_core__p_val_ts_srv,
            subscription_core__bres);
      }
   }
}

void subscription_core__subscription_core_UNINITIALISATION(void) {
   {
      constants__t_subscription_i subscription_core__l_subscription;
      t_bool subscription_core__l_is_subscription;
      t_bool subscription_core__l_continue;
      
      subscription_core_it__init_iter_subscription(&subscription_core__l_continue);
      while (subscription_core__l_continue == true) {
         subscription_core_it__continue_iter_subscription(&subscription_core__l_continue,
            &subscription_core__l_subscription);
         subscription_core_1__is_valid_subscription(subscription_core__l_subscription,
            &subscription_core__l_is_subscription);
         if (subscription_core__l_is_subscription == true) {
            subscription_core__local_close_subscription(subscription_core__l_subscription);
         }
      }
      subscription_core_bs__subscription_core_bs_UNINITIALISATION();
      monitored_item_pointer_bs__monitored_item_pointer_bs_UNINITIALISATION();
   }
}

