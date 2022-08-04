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

 File Name            : subscription_core_1.h

 Date                 : 04/08/2022 14:53:20

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

#ifndef _subscription_core_1_h
#define _subscription_core_1_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 subscription_core_1__a_KeepAliveCounter_i[constants__t_subscription_i_max+1];
extern t_entier4 subscription_core_1__a_LifetimeCounter_i[constants__t_subscription_i_max+1];
extern t_bool subscription_core_1__a_MessageSent_i[constants__t_subscription_i_max+1];
extern t_bool subscription_core_1__a_MoreNotifications_i[constants__t_subscription_i_max+1];
extern t_bool subscription_core_1__a_PublishingEnabled_i[constants__t_subscription_i_max+1];
extern constants__t_sub_seq_num_i subscription_core_1__a_SeqNum_i[constants__t_subscription_i_max+1];
extern t_entier4 subscription_core_1__a_keepAliveExpCount_i[constants__t_subscription_i_max+1];
extern t_entier4 subscription_core_1__a_lifetimeExpCount_i[constants__t_subscription_i_max+1];
extern t_entier4 subscription_core_1__a_maxNotifsPerPublish_i[constants__t_subscription_i_max+1];
extern constants__t_monitoredItemQueue_i subscription_core_1__a_monitoredItemQueue_i[constants__t_subscription_i_max+1];
extern constants__t_notifRepublishQueue_i subscription_core_1__a_notifRepublishQueue_i[constants__t_subscription_i_max+1];
extern constants__t_notificationQueue_i subscription_core_1__a_pendingNotificationQueue_i[constants__t_subscription_i_max+1];
extern constants__t_opcua_duration_i subscription_core_1__a_publishInterval_i[constants__t_subscription_i_max+1];
extern constants__t_publishReqQueue_i subscription_core_1__a_publishRequestQueue_i[constants__t_subscription_i_max+1];
extern constants__t_timer_id_i subscription_core_1__a_publishTimer_i[constants__t_subscription_i_max+1];
extern constants__t_subscription_i subscription_core_1__a_session_subscription_i[constants__t_session_i_max+1];
extern constants__t_session_i subscription_core_1__a_subscription_session_i[constants__t_subscription_i_max+1];
extern constants__t_subscriptionState_i subscription_core_1__a_subscription_state_i[constants__t_subscription_i_max+1];
extern t_bool subscription_core_1__s_subscription_i[constants__t_subscription_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void subscription_core_1__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void subscription_core_1__add_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const constants__t_session_i subscription_core_1__p_session,
   const constants__t_opcua_duration_i subscription_core_1__p_revPublishInterval,
   const t_entier4 subscription_core_1__p_revLifetimeCount,
   const t_entier4 subscription_core_1__p_revMaxKeepAlive,
   const t_entier4 subscription_core_1__p_maxNotificationsPerPublish,
   const t_bool subscription_core_1__p_publishEnabled,
   const constants__t_notificationQueue_i subscription_core_1__p_notifQueue,
   const constants__t_publishReqQueue_i subscription_core_1__p_publishQueue,
   const constants__t_notifRepublishQueue_i subscription_core_1__p_republishQueue,
   const constants__t_monitoredItemQueue_i subscription_core_1__p_monitoredItemQueue,
   const constants__t_timer_id_i subscription_core_1__p_timerId);
extern void subscription_core_1__decrement_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription);
extern void subscription_core_1__decrement_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription);
extern void subscription_core_1__delete_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription);
extern void subscription_core_1__get_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_entier4 * const subscription_core_1__p_keepAliveCounter);
extern void subscription_core_1__get_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_entier4 * const subscription_core_1__p_lifetimeCounter);
extern void subscription_core_1__get_subscription_MaxNotifsPerPublish(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_entier4 * const subscription_core_1__p_maxNotificationsPerPublish);
extern void subscription_core_1__get_subscription_MessageSent(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_bool * const subscription_core_1__p_firstMsgSent);
extern void subscription_core_1__get_subscription_MoreNotifications(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_bool * const subscription_core_1__p_moreNotifs);
extern void subscription_core_1__get_subscription_PublishingEnabled(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_bool * const subscription_core_1__p_pubEnabled);
extern void subscription_core_1__get_subscription_SeqNum(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_sub_seq_num_i * const subscription_core_1__p_seqNumToSend);
extern void subscription_core_1__get_subscription_monitoredItemQueue(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_monitoredItemQueue_i * const subscription_core_1__p_monitoredItemQueue);
extern void subscription_core_1__get_subscription_notifRepublishQueue(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_notifRepublishQueue_i * const subscription_core_1__p_republishQueue);
extern void subscription_core_1__get_subscription_notificationQueue(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_notificationQueue_i * const subscription_core_1__p_notificationQueue);
extern void subscription_core_1__get_subscription_publishInterval(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_opcua_duration_i * const subscription_core_1__p_publishInterval);
extern void subscription_core_1__get_subscription_publishRequestQueue(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_publishReqQueue_i * const subscription_core_1__p_publishReqQueue);
extern void subscription_core_1__get_subscription_state(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_subscriptionState_i * const subscription_core_1__p_state);
extern void subscription_core_1__get_subscription_timer_id(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_timer_id_i * const subscription_core_1__p_timer_id);
extern void subscription_core_1__getall_session(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_session_i * const subscription_core_1__p_session);
extern void subscription_core_1__getall_subscription(
   const constants__t_session_i subscription_core_1__p_session,
   t_bool * const subscription_core_1__p_dom,
   constants__t_subscription_i * const subscription_core_1__p_subscription);
extern void subscription_core_1__is_valid_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_bool * const subscription_core_1__is_valid);
extern void subscription_core_1__reset_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription);
extern void subscription_core_1__reset_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription);
extern void subscription_core_1__set_subscription_MaxLifetimeAndKeepAliveCount(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const t_entier4 subscription_core_1__p_revLifetimeCount,
   const t_entier4 subscription_core_1__p_revMaxKeepAlive);
extern void subscription_core_1__set_subscription_MaxNotifsPerPublish(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const t_entier4 subscription_core_1__p_maxNotificationsPerPublish);
extern void subscription_core_1__set_subscription_MessageSent(
   const constants__t_subscription_i subscription_core_1__p_subscription);
extern void subscription_core_1__set_subscription_MoreNotifications(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const t_bool subscription_core_1__p_moreNotifs);
extern void subscription_core_1__set_subscription_PublishingEnabled(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const t_bool subscription_core_1__p_pubEnabled);
extern void subscription_core_1__set_subscription_SeqNum(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const constants__t_sub_seq_num_i subscription_core_1__p_nextSeqNum);
extern void subscription_core_1__set_subscription_publishInterval(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const constants__t_opcua_duration_i subscription_core_1__p_revPublishInterval);
extern void subscription_core_1__set_subscription_state(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const constants__t_subscriptionState_i subscription_core_1__p_state);
extern void subscription_core_1__set_subscription_timer_id(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const constants__t_timer_id_i subscription_core_1__p_timer_id);

#endif
