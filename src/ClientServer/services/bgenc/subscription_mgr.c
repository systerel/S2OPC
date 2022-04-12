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

 File Name            : subscription_mgr.c

 Date                 : 05/08/2022 08:40:52

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "subscription_mgr.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_mgr__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_mgr__fill_publish_response_msg(
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   const constants__t_subscription_i subscription_mgr__p_subscription,
   const t_bool subscription_mgr__p_moreNotifs) {
   {
      constants__t_notifRepublishQueue_i subscription_mgr__l_republishQueue;
      t_entier4 subscription_mgr__l_nb_seq_nums;
      t_bool subscription_mgr__l_bres;
      
      subscription_core__get_subscription_notifRepublishQueue(subscription_mgr__p_subscription,
         &subscription_mgr__l_republishQueue);
      subscription_core__get_available_republish(subscription_mgr__l_republishQueue,
         &subscription_mgr__l_nb_seq_nums);
      if (subscription_mgr__l_nb_seq_nums > 0) {
         msg_subscription_publish_ack_bs__allocate_subscription_available_seq_nums(subscription_mgr__p_resp_msg,
            subscription_mgr__l_nb_seq_nums,
            &subscription_mgr__l_bres);
         if (subscription_mgr__l_bres == true) {
            subscription_mgr__fill_publish_response_msg_available_seq_nums(subscription_mgr__p_resp_msg,
               subscription_mgr__l_republishQueue,
               subscription_mgr__l_nb_seq_nums);
         }
      }
      subscription_core__set_msg_publish_resp_subscription(subscription_mgr__p_resp_msg,
         subscription_mgr__p_subscription);
      subscription_core__set_msg_publish_resp_notificationMsg(subscription_mgr__p_resp_msg,
         subscription_mgr__p_moreNotifs);
   }
}

void subscription_mgr__fill_publish_response_msg_ack_results(
   const constants__t_session_i subscription_mgr__p_session,
   const constants__t_msg_i subscription_mgr__p_req_msg,
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   const t_entier4 subscription_mgr__p_nb_acks) {
   {
      t_bool subscription_mgr__l_has_sub;
      constants__t_subscription_i subscription_mgr__l_session_sub;
      t_entier4 subscription_mgr__l_index;
      constants__t_subscription_i subscription_mgr__l_sub;
      constants__t_sub_seq_num_i subscription_mgr__l_seq_num;
      t_bool subscription_mgr__l_valid_seq_num;
      
      subscription_core__getall_subscription(subscription_mgr__p_session,
         &subscription_mgr__l_has_sub,
         &subscription_mgr__l_session_sub);
      subscription_mgr__l_index = 1;
      while (subscription_mgr__l_index <= subscription_mgr__p_nb_acks) {
         msg_subscription_publish_ack_bs__getall_msg_publish_request_ack(subscription_mgr__p_req_msg,
            subscription_mgr__l_index,
            &subscription_mgr__l_sub,
            &subscription_mgr__l_seq_num);
         if ((subscription_mgr__l_has_sub == true) &&
            (subscription_mgr__l_sub == subscription_mgr__l_session_sub)) {
            subscription_core__subscription_ack_notif_msg(subscription_mgr__l_session_sub,
               subscription_mgr__l_seq_num,
               &subscription_mgr__l_valid_seq_num);
            if (subscription_mgr__l_valid_seq_num == true) {
               msg_subscription_publish_ack_bs__setall_msg_publish_resp_ack_result(subscription_mgr__p_resp_msg,
                  subscription_mgr__l_index,
                  constants_statuscodes_bs__e_sc_ok);
            }
            else {
               msg_subscription_publish_ack_bs__setall_msg_publish_resp_ack_result(subscription_mgr__p_resp_msg,
                  subscription_mgr__l_index,
                  constants_statuscodes_bs__e_sc_bad_sequence_number_unknown);
            }
         }
         else {
            msg_subscription_publish_ack_bs__setall_msg_publish_resp_ack_result(subscription_mgr__p_resp_msg,
               subscription_mgr__l_index,
               constants_statuscodes_bs__e_sc_bad_subscription_id_invalid);
         }
         subscription_mgr__l_index = subscription_mgr__l_index +
            1;
      }
   }
}

void subscription_mgr__fill_publish_response_msg_available_seq_nums(
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   const constants__t_notifRepublishQueue_i subscription_mgr__republishQueue,
   const t_entier4 subscription_mgr__nb_seq_nums) {
   {
      t_bool subscription_mgr__l_continue;
      constants__t_notifRepublishQueueIterator_i subscription_mgr__l_republishIterator;
      t_entier4 subscription_mgr__l_index;
      constants__t_sub_seq_num_i subscription_mgr__l_seq_num;
      
      subscription_mgr__l_index = 1;
      subscription_core__init_iter_notif_republish(subscription_mgr__republishQueue,
         &subscription_mgr__l_continue,
         &subscription_mgr__l_republishIterator);
      while ((subscription_mgr__l_continue == true) &&
         (subscription_mgr__l_index <= subscription_mgr__nb_seq_nums)) {
         subscription_core__continue_iter_notif_republish(subscription_mgr__republishQueue,
            subscription_mgr__l_republishIterator,
            &subscription_mgr__l_continue,
            &subscription_mgr__l_seq_num);
         msg_subscription_publish_ack_bs__setall_msg_publish_resp_available_seq_num(subscription_mgr__p_resp_msg,
            subscription_mgr__l_index,
            subscription_mgr__l_seq_num);
         subscription_mgr__l_index = subscription_mgr__l_index +
            1;
      }
      subscription_core__clear_notif_republish_iterator(subscription_mgr__republishQueue,
         subscription_mgr__l_republishIterator);
   }
}

void subscription_mgr__fill_response_subscription_create_monitored_items(
   const constants__t_session_i subscription_mgr__p_session,
   const constants__t_subscription_i subscription_mgr__p_subscription,
   const constants__t_TimestampsToReturn_i subscription_mgr__p_tsToReturn,
   const constants__t_user_i subscription_mgr__p_user,
   const constants__t_LocaleIds_i subscription_mgr__p_locales,
   const constants__t_msg_i subscription_mgr__p_req_msg,
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   const t_entier4 subscription_mgr__p_nb_monitored_items) {
   {
      constants__t_endpoint_config_idx_i subscription_mgr__l_endpoint_config_idx;
      t_bool subscription_mgr__l_continue;
      t_entier4 subscription_mgr__l_index;
      t_bool subscription_mgr__l_bres;
      t_bool subscription_mgr__l_bres_presentNode;
      t_bool subscription_mgr__l_bres_absent_knownNode;
      constants__t_NodeClass_i subscription_mgr__l_knownNodeClass;
      constants__t_NodeId_i subscription_mgr__l_nid;
      constants__t_AttributeId_i subscription_mgr__l_aid;
      constants__t_monitoringMode_i subscription_mgr__l_monitMode;
      constants__t_client_handle_i subscription_mgr__l_clientHandle;
      constants__t_opcua_duration_i subscription_mgr__l_samplingItv;
      t_entier4 subscription_mgr__l_queueSize;
      constants__t_Node_i subscription_mgr__l_node;
      constants_statuscodes_bs__t_StatusCode_i subscription_mgr__l_sc;
      constants__t_Variant_i subscription_mgr__l_value;
      constants__t_RawStatusCode subscription_mgr__l_valueSc;
      constants__t_Timestamp subscription_mgr__l_val_ts_src;
      constants__t_Timestamp subscription_mgr__l_val_ts_srv;
      constants__t_monitoredItemPointer_i subscription_mgr__l_monitoredItemPointer;
      constants__t_monitoredItemId_i subscription_mgr__l_monitoredItemId;
      constants__t_opcua_duration_i subscription_mgr__l_revSamplingItv;
      t_entier4 subscription_mgr__l_revQueueSize;
      constants__t_IndexRange_i subscription_mgr__l_indexRange;
      
      session_mgr__session_get_endpoint_config(subscription_mgr__p_session,
         &subscription_mgr__l_endpoint_config_idx);
      subscription_create_monitored_item_it__init_iter_monitored_item_request(subscription_mgr__p_nb_monitored_items,
         &subscription_mgr__l_continue);
      while (subscription_mgr__l_continue == true) {
         subscription_mgr__l_node = constants__c_Node_indet;
         subscription_mgr__l_value = constants__c_Variant_indet;
         subscription_mgr__l_monitoredItemId = constants__c_monitoredItemId_indet;
         subscription_mgr__l_revSamplingItv = constants__c_opcua_duration_indet;
         subscription_mgr__l_revQueueSize = 0;
         subscription_mgr__l_indexRange = constants__c_IndexRange_indet;
         subscription_create_monitored_item_it__continue_iter_monitored_item_request(&subscription_mgr__l_continue,
            &subscription_mgr__l_index);
         msg_subscription_create_monitored_item__getall_monitored_item_req_params(subscription_mgr__p_req_msg,
            subscription_mgr__l_index,
            &subscription_mgr__l_bres,
            &subscription_mgr__l_sc,
            &subscription_mgr__l_nid,
            &subscription_mgr__l_aid,
            &subscription_mgr__l_monitMode,
            &subscription_mgr__l_clientHandle,
            &subscription_mgr__l_samplingItv,
            &subscription_mgr__l_queueSize,
            &subscription_mgr__l_indexRange);
         if (subscription_mgr__l_bres == true) {
            address_space_itf__readall_AddressSpace_Node(subscription_mgr__l_nid,
               &subscription_mgr__l_bres_presentNode,
               &subscription_mgr__l_node);
            subscription_absent_node__if_not_present_is_Node_known(subscription_mgr__l_bres_presentNode,
               subscription_mgr__l_endpoint_config_idx,
               subscription_mgr__l_nid,
               &subscription_mgr__l_bres_absent_knownNode,
               &subscription_mgr__l_knownNodeClass,
               &subscription_mgr__l_valueSc);
            if ((subscription_mgr__l_bres_presentNode == false) &&
               (subscription_mgr__l_bres_absent_knownNode == false)) {
               subscription_mgr__l_sc = constants_statuscodes_bs__e_sc_bad_node_id_unknown;
            }
            else {
               if (subscription_mgr__l_bres_presentNode == true) {
                  address_space_itf__read_Node_Attribute(subscription_mgr__p_user,
                     subscription_mgr__p_locales,
                     subscription_mgr__l_node,
                     subscription_mgr__l_nid,
                     subscription_mgr__l_aid,
                     subscription_mgr__l_indexRange,
                     &subscription_mgr__l_sc,
                     &subscription_mgr__l_value,
                     &subscription_mgr__l_valueSc,
                     &subscription_mgr__l_val_ts_src,
                     &subscription_mgr__l_val_ts_srv);
               }
               else {
                  subscription_absent_node__eval_knownNode_requested_properties(subscription_mgr__l_nid,
                     subscription_mgr__l_knownNodeClass,
                     subscription_mgr__l_aid,
                     subscription_mgr__l_indexRange,
                     &subscription_mgr__l_sc,
                     &subscription_mgr__l_value,
                     &subscription_mgr__l_val_ts_src,
                     &subscription_mgr__l_val_ts_srv);
               }
               if ((subscription_mgr__l_sc != constants_statuscodes_bs__e_sc_ok) &&
                  (((subscription_mgr__l_sc == constants_statuscodes_bs__e_sc_bad_not_readable) ||
                  (subscription_mgr__l_sc == constants_statuscodes_bs__e_sc_bad_user_access_denied)) ||
                  (subscription_mgr__l_sc == constants_statuscodes_bs__e_sc_bad_index_range_no_data))) {
                  constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(subscription_mgr__l_sc,
                     &subscription_mgr__l_valueSc);
                  subscription_mgr__l_sc = constants_statuscodes_bs__e_sc_ok;
               }
               if (subscription_mgr__l_sc == constants_statuscodes_bs__e_sc_ok) {
                  subscription_core__create_monitored_item(subscription_mgr__p_subscription,
                     subscription_mgr__l_nid,
                     subscription_mgr__l_aid,
                     subscription_mgr__l_indexRange,
                     subscription_mgr__l_value,
                     subscription_mgr__l_valueSc,
                     subscription_mgr__l_val_ts_src,
                     subscription_mgr__l_val_ts_srv,
                     subscription_mgr__p_tsToReturn,
                     subscription_mgr__l_monitMode,
                     subscription_mgr__l_clientHandle,
                     &subscription_mgr__l_sc,
                     &subscription_mgr__l_monitoredItemPointer,
                     &subscription_mgr__l_monitoredItemId);
                  subscription_core__compute_create_monitored_item_revised_params(subscription_mgr__l_queueSize,
                     &subscription_mgr__l_revSamplingItv,
                     &subscription_mgr__l_revQueueSize);
                  address_space_itf__read_AddressSpace_free_variant(subscription_mgr__l_value);
               }
            }
         }
         msg_subscription_create_monitored_item__setall_msg_monitored_item_resp_params(subscription_mgr__p_resp_msg,
            subscription_mgr__l_index,
            subscription_mgr__l_sc,
            subscription_mgr__l_monitoredItemId,
            subscription_mgr__l_revSamplingItv,
            subscription_mgr__l_revQueueSize);
      }
   }
}

void subscription_mgr__create_notification_on_monitored_items_if_data_changed(
   const constants__t_monitoredItemQueue_i subscription_mgr__p_monitoredItemQueue,
   const constants__t_WriteValuePointer_i subscription_mgr__p_old_wv_pointer,
   const constants__t_WriteValuePointer_i subscription_mgr__p_new_wv_pointer) {
   {
      t_bool subscription_mgr__l_continue;
      constants__t_monitoredItemQueueIterator_i subscription_mgr__l_iterator;
      constants__t_monitoredItemPointer_i subscription_mgr__l_monitoredItemPointer;
      t_bool subscription_mgr__l_notification_triggered;
      constants__t_monitoredItemId_i subscription_mgr__l_monitoredItemId;
      constants__t_subscription_i subscription_mgr__l_subscription;
      t_bool subscription_mgr__l_valid_subscription;
      constants__t_session_i subscription_mgr__l_session;
      t_bool subscription_mgr__l_session_valid;
      constants__t_user_i subscription_mgr__l_user;
      t_bool subscription_mgr__l_valid_user_access;
      constants__t_NodeId_i subscription_mgr__l_nid;
      constants__t_AttributeId_i subscription_mgr__l_aid;
      constants__t_TimestampsToReturn_i subscription_mgr__l_timestampToReturn;
      constants__t_monitoringMode_i subscription_mgr__l_monitoringMode;
      constants__t_client_handle_i subscription_mgr__l_clientHandle;
      
      subscription_core__init_iter_monitored_item(subscription_mgr__p_monitoredItemQueue,
         &subscription_mgr__l_continue,
         &subscription_mgr__l_iterator);
      while (subscription_mgr__l_continue == true) {
         subscription_core__continue_iter_monitored_item(subscription_mgr__l_iterator,
            subscription_mgr__p_monitoredItemQueue,
            &subscription_mgr__l_continue,
            &subscription_mgr__l_monitoredItemPointer);
         subscription_core__is_notification_triggered(subscription_mgr__l_monitoredItemPointer,
            subscription_mgr__p_old_wv_pointer,
            subscription_mgr__p_new_wv_pointer,
            &subscription_mgr__l_notification_triggered);
         subscription_core__getall_monitoredItemPointer(subscription_mgr__l_monitoredItemPointer,
            &subscription_mgr__l_monitoredItemId,
            &subscription_mgr__l_subscription,
            &subscription_mgr__l_nid,
            &subscription_mgr__l_aid,
            &subscription_mgr__l_timestampToReturn,
            &subscription_mgr__l_monitoringMode,
            &subscription_mgr__l_clientHandle);
         subscription_core__is_valid_subscription(subscription_mgr__l_subscription,
            &subscription_mgr__l_valid_subscription);
         subscription_mgr__l_valid_user_access = false;
         if (subscription_mgr__l_valid_subscription == true) {
            subscription_core__getall_session(subscription_mgr__l_subscription,
               &subscription_mgr__l_session);
            session_mgr__is_valid_session(subscription_mgr__l_session,
               &subscription_mgr__l_session_valid);
            if (subscription_mgr__l_session_valid == true) {
               session_mgr__get_session_user_server(subscription_mgr__l_session,
                  &subscription_mgr__l_user);
               address_space_itf__get_user_authorization(constants__e_operation_type_read,
                  subscription_mgr__l_nid,
                  subscription_mgr__l_aid,
                  subscription_mgr__l_user,
                  &subscription_mgr__l_valid_user_access);
            }
         }
         else {
            subscription_mgr__l_valid_user_access = false;
         }
         if (((subscription_mgr__l_valid_user_access == true) &&
            (subscription_mgr__l_notification_triggered == true)) &&
            (subscription_mgr__l_monitoringMode == constants__e_monitoringMode_reporting)) {
            subscription_core__server_subscription_add_notification(subscription_mgr__l_subscription,
               subscription_mgr__l_monitoredItemPointer,
               subscription_mgr__l_timestampToReturn,
               subscription_mgr__p_new_wv_pointer);
         }
      }
      subscription_core__clear_iter_monitored_item(subscription_mgr__l_iterator);
   }
}

void subscription_mgr__fill_delete_subscriptions_response(
   const constants__t_msg_i subscription_mgr__p_req_msg,
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   const t_bool subscription_mgr__p_has_sub,
   const constants__t_subscription_i subscription_mgr__p_session_sub,
   const t_entier4 subscription_mgr__p_nb_reqs) {
   {
      t_entier4 subscription_mgr__l_index;
      constants__t_subscription_i subscription_mgr__l_sub;
      t_bool subscription_mgr__l_sub_still_valid;
      
      subscription_mgr__l_index = 1;
      while (subscription_mgr__l_index <= subscription_mgr__p_nb_reqs) {
         msg_subscription_delete_bs__getall_msg_delete_subscriptions_at_index(subscription_mgr__p_req_msg,
            subscription_mgr__l_index,
            &subscription_mgr__l_sub);
         if ((subscription_mgr__p_has_sub == true) &&
            (subscription_mgr__l_sub == subscription_mgr__p_session_sub)) {
            subscription_core__is_valid_subscription(subscription_mgr__l_sub,
               &subscription_mgr__l_sub_still_valid);
            if (subscription_mgr__l_sub_still_valid == true) {
               subscription_core__close_subscription(subscription_mgr__l_sub);
            }
            msg_subscription_delete_bs__setall_msg_subscription_delete_subscriptions_resp_at_index(subscription_mgr__p_resp_msg,
               subscription_mgr__l_index,
               true);
         }
         else {
            msg_subscription_delete_bs__setall_msg_subscription_delete_subscriptions_resp_at_index(subscription_mgr__p_resp_msg,
               subscription_mgr__l_index,
               false);
         }
         subscription_mgr__l_index = subscription_mgr__l_index +
            1;
      }
   }
}

void subscription_mgr__fill_set_publishing_mode_response(
   const constants__t_msg_i subscription_mgr__p_req_msg,
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   const t_bool subscription_mgr__p_has_sub,
   const constants__t_subscription_i subscription_mgr__p_session_sub,
   const t_bool subscription_mgr__p_pub_enabled,
   const t_entier4 subscription_mgr__p_nb_reqs) {
   {
      t_entier4 subscription_mgr__l_index;
      constants__t_subscription_i subscription_mgr__l_sub;
      
      subscription_mgr__l_index = 1;
      while (subscription_mgr__l_index <= subscription_mgr__p_nb_reqs) {
         msg_subscription_set_publishing_mode_bs__getall_msg_set_publishing_mode_at_index(subscription_mgr__p_req_msg,
            subscription_mgr__l_index,
            &subscription_mgr__l_sub);
         if ((subscription_mgr__p_has_sub == true) &&
            (subscription_mgr__l_sub == subscription_mgr__p_session_sub)) {
            subscription_core__set_subscription_PublishingEnabled(subscription_mgr__l_sub,
               subscription_mgr__p_pub_enabled);
            msg_subscription_set_publishing_mode_bs__setall_msg_subscription_set_publishing_mode_resp_at_index(subscription_mgr__p_resp_msg,
               subscription_mgr__l_index,
               true);
         }
         else {
            msg_subscription_set_publishing_mode_bs__setall_msg_subscription_set_publishing_mode_resp_at_index(subscription_mgr__p_resp_msg,
               subscription_mgr__l_index,
               false);
         }
         subscription_mgr__l_index = subscription_mgr__l_index +
            1;
      }
   }
}

void subscription_mgr__treat_create_subscription_request(
   const constants__t_session_i subscription_mgr__p_session,
   const constants__t_msg_i subscription_mgr__p_req_msg,
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_mgr__StatusCode_service) {
   {
      t_bool subscription_mgr__l_session_has_subscription;
      constants__t_subscription_i subscription_mgr__l_sub;
      constants__t_opcua_duration_i subscription_mgr__l_reqPublishInterval;
      t_entier4 subscription_mgr__l_reqLifetimeCount;
      t_entier4 subscription_mgr__l_reqMaxKeepAlive;
      t_entier4 subscription_mgr__l_maxNotificationsPerPublish;
      t_bool subscription_mgr__l_publishEnabled;
      constants__t_subscription_i subscription_mgr__l_subscription;
      constants__t_opcua_duration_i subscription_mgr__l_revisedPublishInterval;
      t_entier4 subscription_mgr__l_revisedLifetimeCount;
      t_entier4 subscription_mgr__l_revisedMaxKeepAlive;
      t_entier4 subscription_mgr__l_revisedMaxNotificationsPerPublish;
      
      subscription_core__getall_subscription(subscription_mgr__p_session,
         &subscription_mgr__l_session_has_subscription,
         &subscription_mgr__l_sub);
      if (subscription_mgr__l_session_has_subscription == false) {
         msg_subscription_create_bs__get_msg_create_subscription_req_params(subscription_mgr__p_req_msg,
            &subscription_mgr__l_reqPublishInterval,
            &subscription_mgr__l_reqLifetimeCount,
            &subscription_mgr__l_reqMaxKeepAlive,
            &subscription_mgr__l_maxNotificationsPerPublish,
            &subscription_mgr__l_publishEnabled);
         subscription_core__compute_create_subscription_revised_params(subscription_mgr__l_reqPublishInterval,
            subscription_mgr__l_reqLifetimeCount,
            subscription_mgr__l_reqMaxKeepAlive,
            subscription_mgr__l_maxNotificationsPerPublish,
            &subscription_mgr__l_revisedPublishInterval,
            &subscription_mgr__l_revisedLifetimeCount,
            &subscription_mgr__l_revisedMaxKeepAlive,
            &subscription_mgr__l_revisedMaxNotificationsPerPublish);
         subscription_core__create_subscription(subscription_mgr__p_session,
            subscription_mgr__l_revisedPublishInterval,
            subscription_mgr__l_revisedLifetimeCount,
            subscription_mgr__l_revisedMaxKeepAlive,
            subscription_mgr__l_revisedMaxNotificationsPerPublish,
            subscription_mgr__l_publishEnabled,
            subscription_mgr__StatusCode_service,
            &subscription_mgr__l_subscription);
         if (*subscription_mgr__StatusCode_service == constants_statuscodes_bs__e_sc_ok) {
            msg_subscription_create_bs__set_msg_create_subscription_resp_params(subscription_mgr__p_resp_msg,
               subscription_mgr__l_subscription,
               subscription_mgr__l_revisedPublishInterval,
               subscription_mgr__l_revisedLifetimeCount,
               subscription_mgr__l_revisedMaxKeepAlive);
         }
      }
      else {
         *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_subscriptions;
      }
   }
}

void subscription_mgr__treat_modify_subscription_request(
   const constants__t_session_i subscription_mgr__p_session,
   const constants__t_msg_i subscription_mgr__p_req_msg,
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_mgr__StatusCode_service) {
   {
      t_bool subscription_mgr__l_session_has_subscription;
      constants__t_subscription_i subscription_mgr__l_session_sub;
      constants__t_subscription_i subscription_mgr__l_sub;
      constants__t_opcua_duration_i subscription_mgr__l_reqPublishInterval;
      t_entier4 subscription_mgr__l_reqLifetimeCount;
      t_entier4 subscription_mgr__l_reqMaxKeepAlive;
      t_entier4 subscription_mgr__l_maxNotifPerPublish;
      constants__t_opcua_duration_i subscription_mgr__l_revisedPublishInterval;
      t_entier4 subscription_mgr__l_revisedLifetimeCount;
      t_entier4 subscription_mgr__l_revisedMaxKeepAlive;
      t_entier4 subscription_mgr__l_revisedMaxNotificationsPerPublish;
      
      subscription_core__getall_subscription(subscription_mgr__p_session,
         &subscription_mgr__l_session_has_subscription,
         &subscription_mgr__l_session_sub);
      msg_subscription_create_bs__get_msg_modify_subscription_req_params(subscription_mgr__p_req_msg,
         &subscription_mgr__l_sub,
         &subscription_mgr__l_reqPublishInterval,
         &subscription_mgr__l_reqLifetimeCount,
         &subscription_mgr__l_reqMaxKeepAlive,
         &subscription_mgr__l_maxNotifPerPublish);
      if ((subscription_mgr__l_session_has_subscription == true) &&
         (subscription_mgr__l_sub == subscription_mgr__l_session_sub)) {
         subscription_core__compute_create_subscription_revised_params(subscription_mgr__l_reqPublishInterval,
            subscription_mgr__l_reqLifetimeCount,
            subscription_mgr__l_reqMaxKeepAlive,
            subscription_mgr__l_maxNotifPerPublish,
            &subscription_mgr__l_revisedPublishInterval,
            &subscription_mgr__l_revisedLifetimeCount,
            &subscription_mgr__l_revisedMaxKeepAlive,
            &subscription_mgr__l_revisedMaxNotificationsPerPublish);
         subscription_core__modify_subscription(subscription_mgr__l_session_sub,
            subscription_mgr__l_revisedPublishInterval,
            subscription_mgr__l_revisedLifetimeCount,
            subscription_mgr__l_revisedMaxKeepAlive,
            subscription_mgr__l_revisedMaxNotificationsPerPublish);
         msg_subscription_create_bs__set_msg_modify_subscription_resp_params(subscription_mgr__p_resp_msg,
            subscription_mgr__l_revisedPublishInterval,
            subscription_mgr__l_revisedLifetimeCount,
            subscription_mgr__l_revisedMaxKeepAlive);
         *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
      }
      else {
         *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_subscription_id_invalid;
      }
   }
}

void subscription_mgr__treat_delete_subscriptions_request(
   const constants__t_session_i subscription_mgr__p_session,
   const constants__t_msg_i subscription_mgr__p_req_msg,
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_mgr__StatusCode_service) {
   {
      t_bool subscription_mgr__l_session_has_subscription;
      constants__t_subscription_i subscription_mgr__l_session_sub;
      t_entier4 subscription_mgr__l_nb_reqs;
      t_bool subscription_mgr__l_bres;
      
      msg_subscription_delete_bs__getall_msg_delete_subscriptions_req_params(subscription_mgr__p_req_msg,
         &subscription_mgr__l_nb_reqs);
      if ((subscription_mgr__l_nb_reqs > 0) &&
         (subscription_mgr__l_nb_reqs <= constants__k_n_genericOperationPerReq_max)) {
         subscription_core__getall_subscription(subscription_mgr__p_session,
            &subscription_mgr__l_session_has_subscription,
            &subscription_mgr__l_session_sub);
         msg_subscription_delete_bs__allocate_msg_delete_subscriptions_resp_results_array(subscription_mgr__p_resp_msg,
            subscription_mgr__l_nb_reqs,
            &subscription_mgr__l_bres);
         if (subscription_mgr__l_bres == true) {
            subscription_mgr__fill_delete_subscriptions_response(subscription_mgr__p_req_msg,
               subscription_mgr__p_resp_msg,
               subscription_mgr__l_session_has_subscription,
               subscription_mgr__l_session_sub,
               subscription_mgr__l_nb_reqs);
            *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
         }
         else {
            *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else if (subscription_mgr__l_nb_reqs <= 0) {
         *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
      }
      else {
         *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_ops;
      }
   }
}

void subscription_mgr__treat_publishing_mode_request(
   const constants__t_session_i subscription_mgr__p_session,
   const constants__t_msg_i subscription_mgr__p_req_msg,
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_mgr__StatusCode_service) {
   {
      t_bool subscription_mgr__l_session_has_subscription;
      constants__t_subscription_i subscription_mgr__l_session_sub;
      t_entier4 subscription_mgr__l_nb_reqs;
      t_bool subscription_mgr__l_pub_enabled;
      t_bool subscription_mgr__l_bres;
      
      msg_subscription_set_publishing_mode_bs__getall_msg_subscription_set_publishing_mode_params(subscription_mgr__p_req_msg,
         &subscription_mgr__l_nb_reqs,
         &subscription_mgr__l_pub_enabled);
      if ((subscription_mgr__l_nb_reqs > 0) &&
         (subscription_mgr__l_nb_reqs <= constants__k_n_genericOperationPerReq_max)) {
         subscription_core__getall_subscription(subscription_mgr__p_session,
            &subscription_mgr__l_session_has_subscription,
            &subscription_mgr__l_session_sub);
         msg_subscription_set_publishing_mode_bs__allocate_msg_subscription_set_publishing_mode_resp_results_array(subscription_mgr__p_resp_msg,
            subscription_mgr__l_nb_reqs,
            &subscription_mgr__l_bres);
         if (subscription_mgr__l_bres == true) {
            subscription_mgr__fill_set_publishing_mode_response(subscription_mgr__p_req_msg,
               subscription_mgr__p_resp_msg,
               subscription_mgr__l_session_has_subscription,
               subscription_mgr__l_session_sub,
               subscription_mgr__l_pub_enabled,
               subscription_mgr__l_nb_reqs);
            *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
         }
         else {
            *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
      else if (subscription_mgr__l_nb_reqs <= 0) {
         *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
      }
      else {
         *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_ops;
      }
   }
}

void subscription_mgr__treat_subscription_publish_request(
   const constants__t_session_i subscription_mgr__p_session,
   const constants__t_msg_header_i subscription_mgr__p_req_header,
   const constants__t_msg_i subscription_mgr__p_req_msg,
   const constants__t_server_request_handle_i subscription_mgr__p_req_handle,
   const constants__t_request_context_i subscription_mgr__p_req_ctx,
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_mgr__StatusCode_service,
   t_bool * const subscription_mgr__async_resp_msg) {
   {
      t_bool subscription_mgr__l_session_has_subscription;
      constants__t_subscription_i subscription_mgr__l_sub;
      constants__t_timeref_i subscription_mgr__l_req_exp_time;
      t_entier4 subscription_mgr__l_nb_subscriptionAcks;
      constants__t_subscription_i subscription_mgr__l_subscription;
      t_bool subscription_mgr__l_moreNotifs;
      t_bool subscription_mgr__l_bres;
      
      subscription_mgr__l_subscription = constants__c_subscription_indet;
      subscription_mgr__l_moreNotifs = false;
      *subscription_mgr__async_resp_msg = false;
      subscription_core__getall_subscription(subscription_mgr__p_session,
         &subscription_mgr__l_session_has_subscription,
         &subscription_mgr__l_sub);
      if (subscription_mgr__l_session_has_subscription == true) {
         msg_subscription_publish_ack_bs__get_msg_header_expiration_time(subscription_mgr__p_req_header,
            &subscription_mgr__l_req_exp_time);
         msg_subscription_publish_ack_bs__get_msg_publish_request_ack_params(subscription_mgr__p_req_msg,
            &subscription_mgr__l_nb_subscriptionAcks);
         if ((subscription_mgr__l_nb_subscriptionAcks > 0) &&
            (subscription_mgr__l_nb_subscriptionAcks <= constants__k_n_genericOperationPerReq_max)) {
            msg_subscription_publish_ack_bs__allocate_subscription_ack_results(subscription_mgr__p_resp_msg,
               subscription_mgr__l_nb_subscriptionAcks,
               &subscription_mgr__l_bres);
            if (subscription_mgr__l_bres == true) {
               subscription_mgr__fill_publish_response_msg_ack_results(subscription_mgr__p_session,
                  subscription_mgr__p_req_msg,
                  subscription_mgr__p_resp_msg,
                  subscription_mgr__l_nb_subscriptionAcks);
               *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
            }
            else {
               *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
         }
         else if (subscription_mgr__l_nb_subscriptionAcks > constants__k_n_genericOperationPerReq_max) {
            *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_ops;
         }
         else {
            *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
         }
         if (*subscription_mgr__StatusCode_service == constants_statuscodes_bs__e_sc_ok) {
            subscription_core__receive_publish_request(subscription_mgr__p_session,
               subscription_mgr__l_req_exp_time,
               subscription_mgr__p_req_handle,
               subscription_mgr__p_req_ctx,
               subscription_mgr__p_resp_msg,
               subscription_mgr__StatusCode_service,
               subscription_mgr__async_resp_msg,
               &subscription_mgr__l_subscription,
               &subscription_mgr__l_moreNotifs);
         }
         if ((*subscription_mgr__StatusCode_service == constants_statuscodes_bs__e_sc_ok) &&
            (*subscription_mgr__async_resp_msg == false)) {
            subscription_mgr__fill_publish_response_msg(subscription_mgr__p_resp_msg,
               subscription_mgr__l_subscription,
               subscription_mgr__l_moreNotifs);
         }
      }
      else {
         *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_no_subscription;
      }
   }
}

void subscription_mgr__treat_subscription_republish_request(
   const constants__t_session_i subscription_mgr__p_session,
   const constants__t_msg_i subscription_mgr__p_req_msg,
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_mgr__StatusCode_service) {
   {
      t_bool subscription_mgr__l_session_has_subscription;
      constants__t_subscription_i subscription_mgr__l_session_sub;
      constants__t_subscription_i subscription_mgr__l_sub;
      constants__t_sub_seq_num_i subscription_mgr__l_seq_num;
      constants__t_notifRepublishQueue_i subscription_mgr__l_republishQueue;
      t_bool subscription_mgr__l_bres;
      constants__t_notif_msg_i subscription_mgr__l_notifMsg;
      
      msg_subscription_publish_ack_bs__getall_msg_republish_request(subscription_mgr__p_req_msg,
         &subscription_mgr__l_sub,
         &subscription_mgr__l_seq_num);
      subscription_core__getall_subscription(subscription_mgr__p_session,
         &subscription_mgr__l_session_has_subscription,
         &subscription_mgr__l_session_sub);
      if ((subscription_mgr__l_session_has_subscription == true) &&
         (subscription_mgr__l_session_sub == subscription_mgr__l_sub)) {
         subscription_core__get_subscription_notifRepublishQueue(subscription_mgr__l_session_sub,
            &subscription_mgr__l_republishQueue);
         subscription_core__get_republish_notif_from_queue(subscription_mgr__l_republishQueue,
            subscription_mgr__l_seq_num,
            &subscription_mgr__l_bres,
            &subscription_mgr__l_notifMsg);
         if (subscription_mgr__l_bres == true) {
            msg_subscription_publish_ack_bs__setall_msg_republish_response(subscription_mgr__p_resp_msg,
               subscription_mgr__l_notifMsg,
               subscription_mgr__StatusCode_service);
         }
         else {
            *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_message_not_available;
         }
      }
      else {
         *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_subscription_id_invalid;
      }
   }
}

void subscription_mgr__treat_subscription_create_monitored_items_req(
   const constants__t_session_i subscription_mgr__p_session,
   const constants__t_user_i subscription_mgr__p_user,
   const constants__t_msg_i subscription_mgr__p_req_msg,
   const constants__t_msg_i subscription_mgr__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_mgr__StatusCode_service) {
   {
      constants_statuscodes_bs__t_StatusCode_i subscription_mgr__l_sc;
      constants__t_subscription_i subscription_mgr__l_subscription;
      t_bool subscription_mgr__l_valid_subscription;
      constants__t_TimestampsToReturn_i subscription_mgr__l_timestampToRet;
      t_entier4 subscription_mgr__l_nb_monitored_items;
      constants__t_LocaleIds_i subscription_mgr__l_locales;
      t_bool subscription_mgr__l_bres;
      
      msg_subscription_create_monitored_item__getall_msg_create_monitored_items_req_params(subscription_mgr__p_req_msg,
         &subscription_mgr__l_sc,
         &subscription_mgr__l_subscription,
         &subscription_mgr__l_timestampToRet,
         &subscription_mgr__l_nb_monitored_items);
      if (subscription_mgr__l_sc == constants_statuscodes_bs__e_sc_ok) {
         subscription_core__is_valid_subscription_on_session(subscription_mgr__p_session,
            subscription_mgr__l_subscription,
            &subscription_mgr__l_valid_subscription);
         if (subscription_mgr__l_valid_subscription == true) {
            msg_subscription_create_monitored_item__alloc_msg_create_monitored_items_resp_results(subscription_mgr__p_resp_msg,
               subscription_mgr__l_nb_monitored_items,
               &subscription_mgr__l_bres);
            if (subscription_mgr__l_bres == true) {
               session_mgr__get_server_session_preferred_locales(subscription_mgr__p_session,
                  &subscription_mgr__l_locales);
               subscription_mgr__fill_response_subscription_create_monitored_items(subscription_mgr__p_session,
                  subscription_mgr__l_subscription,
                  subscription_mgr__l_timestampToRet,
                  subscription_mgr__p_user,
                  subscription_mgr__l_locales,
                  subscription_mgr__p_req_msg,
                  subscription_mgr__p_resp_msg,
                  subscription_mgr__l_nb_monitored_items);
               *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
            }
            else {
               *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
            }
         }
         else {
            *subscription_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_subscription_id_invalid;
         }
      }
      else {
         *subscription_mgr__StatusCode_service = subscription_mgr__l_sc;
      }
   }
}

void subscription_mgr__server_subscription_data_changed(
   const constants__t_WriteValuePointer_i subscription_mgr__p_old_write_value_pointer,
   const constants__t_WriteValuePointer_i subscription_mgr__p_new_write_value_pointer) {
   {
      constants__t_NodeId_i subscription_mgr__l_nid;
      constants__t_AttributeId_i subscription_mgr__l_aid;
      t_bool subscription_mgr__l_nid_valid;
      constants__t_Node_i subscription_mgr__l_node;
      constants__t_monitoredItemQueue_i subscription_mgr__l_monitoredItemQueue;
      t_bool subscription_mgr__l_bres;
      
      write_value_pointer_bs__get_write_value_pointer_NodeId_AttributeId(subscription_mgr__p_new_write_value_pointer,
         &subscription_mgr__l_nid,
         &subscription_mgr__l_aid);
      address_space_itf__readall_AddressSpace_Node(subscription_mgr__l_nid,
         &subscription_mgr__l_nid_valid,
         &subscription_mgr__l_node);
      if (subscription_mgr__l_nid_valid == true) {
         subscription_core__get_nodeToMonitoredItemQueue(subscription_mgr__l_nid,
            &subscription_mgr__l_bres,
            &subscription_mgr__l_monitoredItemQueue);
         if (subscription_mgr__l_bres == true) {
            subscription_mgr__create_notification_on_monitored_items_if_data_changed(subscription_mgr__l_monitoredItemQueue,
               subscription_mgr__p_old_write_value_pointer,
               subscription_mgr__p_new_write_value_pointer);
         }
      }
      write_value_pointer_bs__free_write_value_pointer(subscription_mgr__p_old_write_value_pointer);
      write_value_pointer_bs__free_write_value_pointer(subscription_mgr__p_new_write_value_pointer);
   }
}

void subscription_mgr__server_subscription_publish_timeout(
   const constants__t_subscription_i subscription_mgr__p_subscription) {
   {
      t_bool subscription_mgr__l_closeSubscription;
      t_bool subscription_mgr__l_msg_to_send;
      constants_statuscodes_bs__t_StatusCode_i subscription_mgr__l_msg_sc;
      constants__t_session_i subscription_mgr__l_session;
      constants__t_msg_i subscription_mgr__l_publish_resp_msg;
      constants__t_server_request_handle_i subscription_mgr__l_req_handle;
      constants__t_request_context_i subscription_mgr__l_req_context;
      t_bool subscription_mgr__l_moreNotifs;
      t_bool subscription_mgr__l_validPublishingReqQueued;
      
      subscription_core__server_subscription_core_publish_timeout_check_lifetime(subscription_mgr__p_subscription,
         &subscription_mgr__l_closeSubscription,
         &subscription_mgr__l_msg_to_send,
         &subscription_mgr__l_session,
         &subscription_mgr__l_publish_resp_msg,
         &subscription_mgr__l_req_handle,
         &subscription_mgr__l_req_context,
         &subscription_mgr__l_validPublishingReqQueued);
      if (subscription_mgr__l_closeSubscription == false) {
         subscription_core__server_subscription_core_publish_timeout(subscription_mgr__p_subscription,
            subscription_mgr__l_validPublishingReqQueued,
            &subscription_mgr__l_msg_to_send,
            &subscription_mgr__l_msg_sc,
            &subscription_mgr__l_session,
            &subscription_mgr__l_publish_resp_msg,
            &subscription_mgr__l_req_handle,
            &subscription_mgr__l_req_context,
            &subscription_mgr__l_moreNotifs);
         if (subscription_mgr__l_msg_to_send == true) {
            subscription_mgr__fill_publish_response_msg(subscription_mgr__l_publish_resp_msg,
               subscription_mgr__p_subscription,
               subscription_mgr__l_moreNotifs);
            subscription_core__generate_internal_send_publish_response_event(subscription_mgr__l_session,
               subscription_mgr__l_publish_resp_msg,
               subscription_mgr__l_req_handle,
               subscription_mgr__l_req_context,
               subscription_mgr__l_msg_sc);
            if (subscription_mgr__l_moreNotifs == true) {
               subscription_core__server_subscription_core_check_valid_publish_req_queue(subscription_mgr__p_subscription,
                  &subscription_mgr__l_validPublishingReqQueued);
               while ((subscription_mgr__l_moreNotifs == true) &&
                  (subscription_mgr__l_validPublishingReqQueued == true)) {
                  subscription_core__server_subscription_core_publish_timeout_return_moreNotifs(subscription_mgr__p_subscription,
                     &subscription_mgr__l_msg_to_send,
                     &subscription_mgr__l_msg_sc,
                     &subscription_mgr__l_session,
                     &subscription_mgr__l_publish_resp_msg,
                     &subscription_mgr__l_req_handle,
                     &subscription_mgr__l_req_context,
                     &subscription_mgr__l_moreNotifs);
                  subscription_mgr__fill_publish_response_msg(subscription_mgr__l_publish_resp_msg,
                     subscription_mgr__p_subscription,
                     subscription_mgr__l_moreNotifs);
                  subscription_core__generate_internal_send_publish_response_event(subscription_mgr__l_session,
                     subscription_mgr__l_publish_resp_msg,
                     subscription_mgr__l_req_handle,
                     subscription_mgr__l_req_context,
                     subscription_mgr__l_msg_sc);
                  if (subscription_mgr__l_moreNotifs == true) {
                     subscription_core__server_subscription_core_check_valid_publish_req_queue(subscription_mgr__p_subscription,
                        &subscription_mgr__l_validPublishingReqQueued);
                  }
               }
            }
         }
      }
      else {
         subscription_core__close_subscription(subscription_mgr__p_subscription);
         if (subscription_mgr__l_msg_to_send == true) {
            subscription_mgr__fill_publish_response_msg(subscription_mgr__l_publish_resp_msg,
               subscription_mgr__p_subscription,
               false);
            subscription_core__generate_internal_send_publish_response_event(subscription_mgr__l_session,
               subscription_mgr__l_publish_resp_msg,
               subscription_mgr__l_req_handle,
               subscription_mgr__l_req_context,
               constants_statuscodes_bs__e_sc_ok);
         }
      }
   }
}

void subscription_mgr__server_subscription_session_inactive(
   const constants__t_session_i subscription_mgr__p_session,
   const constants__t_sessionState subscription_mgr__p_newSessionState) {
   {
      t_bool subscription_mgr__l_has_sub;
      constants__t_subscription_i subscription_mgr__l_sub;
      
      subscription_core__getall_subscription(subscription_mgr__p_session,
         &subscription_mgr__l_has_sub,
         &subscription_mgr__l_sub);
      if (subscription_mgr__l_has_sub == true) {
         if (((subscription_mgr__p_newSessionState == constants__e_session_scOrphaned) ||
            (subscription_mgr__p_newSessionState == constants__e_session_scActivating)) ||
            (subscription_mgr__p_newSessionState == constants__e_session_closed)) {
            subscription_core__empty_session_publish_requests(subscription_mgr__l_sub);
            if (subscription_mgr__p_newSessionState == constants__e_session_closed) {
               subscription_core__close_subscription(subscription_mgr__l_sub);
            }
         }
      }
   }
}

void subscription_mgr__subscription_mgr_UNINITIALISATION(void) {
   subscription_core__subscription_core_UNINITIALISATION();
}

