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

 File Name            : msg_subscription_monitored_item.c

 Date                 : 08/09/2022 13:07:04

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "msg_subscription_monitored_item.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 msg_subscription_monitored_item__nb_monitored_items;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void msg_subscription_monitored_item__INITIALISATION(void) {
   msg_subscription_monitored_item__nb_monitored_items = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void msg_subscription_monitored_item__getall_msg_create_monitored_items_req_params(
   const constants__t_msg_i msg_subscription_monitored_item__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_subscription_monitored_item__p_sc,
   constants__t_subscription_i * const msg_subscription_monitored_item__p_subscription,
   constants__t_TimestampsToReturn_i * const msg_subscription_monitored_item__p_timestampToRet,
   t_entier4 * const msg_subscription_monitored_item__p_nb_monitored_items) {
   msg_subscription_monitored_item_bs__get_msg_create_monitored_items_req_subscription(msg_subscription_monitored_item__p_req_msg,
      msg_subscription_monitored_item__p_subscription);
   msg_subscription_monitored_item_bs__get_msg_create_monitored_items_req_timestamp_to_ret(msg_subscription_monitored_item__p_req_msg,
      msg_subscription_monitored_item__p_timestampToRet);
   msg_subscription_monitored_item_bs__get_msg_create_monitored_items_req_nb_monitored_items(msg_subscription_monitored_item__p_req_msg,
      msg_subscription_monitored_item__p_nb_monitored_items);
   if (*msg_subscription_monitored_item__p_nb_monitored_items <= 0) {
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
   }
   else if (*msg_subscription_monitored_item__p_nb_monitored_items > constants__k_n_monitoredItem_max) {
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_bad_too_many_ops;
   }
   else if (*msg_subscription_monitored_item__p_timestampToRet == constants__c_TimestampsToReturn_indet) {
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_bad_timestamps_to_return_invalid;
   }
   else {
      msg_subscription_monitored_item__nb_monitored_items = *msg_subscription_monitored_item__p_nb_monitored_items;
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_ok;
   }
}

void msg_subscription_monitored_item__getall_msg_modify_monitored_items_req_params(
   const constants__t_msg_i msg_subscription_monitored_item__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_subscription_monitored_item__p_sc,
   constants__t_subscription_i * const msg_subscription_monitored_item__p_subscription,
   constants__t_TimestampsToReturn_i * const msg_subscription_monitored_item__p_timestampToRet,
   t_entier4 * const msg_subscription_monitored_item__p_nb_monitored_items) {
   msg_subscription_monitored_item_bs__get_msg_modify_monitored_items_req_subscription(msg_subscription_monitored_item__p_req_msg,
      msg_subscription_monitored_item__p_subscription);
   msg_subscription_monitored_item_bs__get_msg_modify_monitored_items_req_timestamp_to_ret(msg_subscription_monitored_item__p_req_msg,
      msg_subscription_monitored_item__p_timestampToRet);
   msg_subscription_monitored_item_bs__get_msg_modify_monitored_items_req_nb_monitored_items(msg_subscription_monitored_item__p_req_msg,
      msg_subscription_monitored_item__p_nb_monitored_items);
   if (*msg_subscription_monitored_item__p_nb_monitored_items <= 0) {
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
   }
   else if (*msg_subscription_monitored_item__p_nb_monitored_items > constants__k_n_monitoredItem_max) {
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_bad_too_many_ops;
   }
   else if (*msg_subscription_monitored_item__p_timestampToRet == constants__c_TimestampsToReturn_indet) {
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_bad_timestamps_to_return_invalid;
   }
   else {
      msg_subscription_monitored_item__nb_monitored_items = *msg_subscription_monitored_item__p_nb_monitored_items;
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_ok;
   }
}

void msg_subscription_monitored_item__getall_msg_delete_monitored_items_req_params(
   const constants__t_msg_i msg_subscription_monitored_item__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_subscription_monitored_item__p_sc,
   constants__t_subscription_i * const msg_subscription_monitored_item__p_subscription,
   t_entier4 * const msg_subscription_monitored_item__p_nb_monitored_items) {
   msg_subscription_monitored_item_bs__get_msg_delete_monitored_items_req_params(msg_subscription_monitored_item__p_req_msg,
      msg_subscription_monitored_item__p_subscription,
      msg_subscription_monitored_item__p_nb_monitored_items);
   if (*msg_subscription_monitored_item__p_nb_monitored_items <= 0) {
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
   }
   else if (*msg_subscription_monitored_item__p_nb_monitored_items > constants__k_n_monitoredItem_max) {
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_bad_too_many_ops;
   }
   else {
      msg_subscription_monitored_item__nb_monitored_items = *msg_subscription_monitored_item__p_nb_monitored_items;
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_ok;
   }
}

void msg_subscription_monitored_item__getall_msg_set_monit_mode_monitored_items_req_params(
   const constants__t_msg_i msg_subscription_monitored_item__p_req_msg,
   constants_statuscodes_bs__t_StatusCode_i * const msg_subscription_monitored_item__p_sc,
   constants__t_subscription_i * const msg_subscription_monitored_item__p_subscription,
   constants__t_monitoringMode_i * const msg_subscription_monitored_item__p_monitoring_mode,
   t_entier4 * const msg_subscription_monitored_item__p_nb_monitored_items) {
   msg_subscription_monitored_item_bs__get_msg_set_monit_mode_monitored_items_req_params(msg_subscription_monitored_item__p_req_msg,
      msg_subscription_monitored_item__p_subscription,
      msg_subscription_monitored_item__p_monitoring_mode,
      msg_subscription_monitored_item__p_nb_monitored_items);
   if (((*msg_subscription_monitored_item__p_monitoring_mode != constants__e_monitoringMode_disabled) &&
      (*msg_subscription_monitored_item__p_monitoring_mode != constants__e_monitoringMode_sampling)) &&
      (*msg_subscription_monitored_item__p_monitoring_mode != constants__e_monitoringMode_reporting)) {
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_bad_monitoring_mode_invalid;
   }
   else if (*msg_subscription_monitored_item__p_nb_monitored_items <= 0) {
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
   }
   else if (*msg_subscription_monitored_item__p_nb_monitored_items > constants__k_n_monitoredItem_max) {
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_bad_too_many_ops;
   }
   else {
      msg_subscription_monitored_item__nb_monitored_items = *msg_subscription_monitored_item__p_nb_monitored_items;
      *msg_subscription_monitored_item__p_sc = constants_statuscodes_bs__e_sc_ok;
   }
}

