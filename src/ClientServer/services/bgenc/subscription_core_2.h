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

 File Name            : subscription_core_2.h

 Date                 : 03/11/2026 09:52:27

 C Translator Version : tradc Java V2.0 (2026-09-14)

******************************************************************************/

#ifndef _subscription_core_2_h
#define _subscription_core_2_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "channel_mgr.h"
#include "constants.h"
#include "constants_statuscodes_bs.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "request_handle_bs.h"
#include "subscription_core_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 subscription_core_2__a_KeepAliveCounter_i[constants__t_subscription_i_max+1];
extern t_entier4 subscription_core_2__a_LifetimeCounter_i[constants__t_subscription_i_max+1];
extern t_bool subscription_core_2__a_MessageSent_i[constants__t_subscription_i_max+1];
extern t_bool subscription_core_2__a_MoreNotifications_i[constants__t_subscription_i_max+1];
extern t_bool subscription_core_2__a_PublishingEnabled_i[constants__t_subscription_i_max+1];
extern constants__t_sub_seq_num_i subscription_core_2__a_SeqNum_i[constants__t_subscription_i_max+1];
extern t_entier4 subscription_core_2__a_keepAliveExpCount_i[constants__t_subscription_i_max+1];
extern t_entier4 subscription_core_2__a_lifetimeExpCount_i[constants__t_subscription_i_max+1];
extern t_entier4 subscription_core_2__a_maxNotifsPerPublish_i[constants__t_subscription_i_max+1];
extern constants__t_monitoredItemQueue_i subscription_core_2__a_monitoredItemQueue_i[constants__t_subscription_i_max+1];
extern constants__t_notifRepublishQueue_i subscription_core_2__a_notifRepublishQueue_i[constants__t_subscription_i_max+1];
extern constants__t_opcua_duration_i subscription_core_2__a_publishInterval_i[constants__t_subscription_i_max+1];
extern constants__t_publishReqQueue_i subscription_core_2__a_publishRequestQueue_i[constants__t_session_i_max+1];
extern constants__t_timer_id_i subscription_core_2__a_publishTimer_i[constants__t_subscription_i_max+1];
extern t_entier4 subscription_core_2__a_session_nb_priorities_i[constants__t_session_i_max+1];
extern t_entier4 subscription_core_2__a_session_nb_subscriptions_i[constants__t_session_i_max+1];
extern t_entier4 subscription_core_2__a_session_priority_idx_i[constants__t_session_i_max+1][constants__t_priority_i_max+1];
extern constants__t_prioritySubQueue_i subscription_core_2__a_session_priority_subscriptions_queue_i[constants__t_session_i_max+1][constants__t_prio_idx_i_max+1];
extern t_entier4 subscription_core_2__a_session_seq_priority_i[constants__t_session_i_max+1][constants__t_prio_idx_i_max+1];
extern t_entier4 subscription_core_2__a_subscription_priority_i[constants__t_subscription_i_max+1];
extern constants__t_session_i subscription_core_2__a_subscription_session_i[constants__t_subscription_i_max+1];
extern constants__t_subscriptionState_i subscription_core_2__a_subscription_state_i[constants__t_subscription_i_max+1];
extern t_bool subscription_core_2__s_subscription_i[constants__t_subscription_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void subscription_core_2__INITIALISATION(void);

/*---------------------
   Inlined OPERATIONS
  ---------------------*/
#define subscription_core_2__is_valid_subscription(subscription_core_2__p_subscription, \
   subscription_core_2__is_valid) { \
   *subscription_core_2__is_valid = subscription_core_2__s_subscription_i[subscription_core_2__p_subscription]; \
}

#define subscription_core_2__getall_session(subscription_core_2__p_subscription, \
   subscription_core_2__p_session) { \
   *subscription_core_2__p_session = subscription_core_2__a_subscription_session_i[subscription_core_2__p_subscription]; \
}

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void subscription_core_2__local_is_valid_subscription(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_bool * const subscription_core_2__is_valid);
extern void subscription_core_2__local_log_subscription_transition(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_subscriptionState_i subscription_core_2__p_old_state,
   const constants__t_subscriptionState_i subscription_core_2__p_new_state,
   const t_entier4 subscription_core_2__p_transition);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void subscription_core_2__decrease_card_session_seq_subscription(
   const constants__t_session_i subscription_core_2__p_session);
extern void subscription_core_2__decrement_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription);
extern void subscription_core_2__decrement_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription);
extern void subscription_core_2__del_subscription(
   const constants__t_subscription_i subscription_core_2__p_subscription);
extern void subscription_core_2__get_card_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   t_entier4 * const subscription_core_2__p_priority_idx);
extern void subscription_core_2__get_card_session_seq_subscription(
   const constants__t_session_i subscription_core_2__p_session,
   t_entier4 * const subscription_core_2__p_sub_idx);
extern void subscription_core_2__get_session_priority_idx(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority,
   t_entier4 * const subscription_core_2__p_priority_idx);
extern void subscription_core_2__get_session_priority_subscriptions_queue(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_prio_idx,
   constants__t_prioritySubQueue_i * const subscription_core_2__p_prioritySubQueue);
extern void subscription_core_2__get_session_publishRequestQueue(
   const constants__t_session_i subscription_core_2__p_session,
   t_bool * const subscription_core_2__p_bres,
   constants__t_publishReqQueue_i * const subscription_core_2__p_publishReqQueue);
extern void subscription_core_2__get_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority_idx,
   t_entier4 * const subscription_core_2__p_priority);
extern void subscription_core_2__get_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_entier4 * const subscription_core_2__p_keepAliveCounter);
extern void subscription_core_2__get_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_entier4 * const subscription_core_2__p_lifetimeCounter);
extern void subscription_core_2__get_subscription_MaxNotifsPerPublish(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_entier4 * const subscription_core_2__p_maxNotificationsPerPublish);
extern void subscription_core_2__get_subscription_MessageSent(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_bool * const subscription_core_2__p_firstMsgSent);
extern void subscription_core_2__get_subscription_MoreNotifications(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_bool * const subscription_core_2__p_moreNotifs);
extern void subscription_core_2__get_subscription_PublishingEnabled(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_bool * const subscription_core_2__p_pubEnabled);
extern void subscription_core_2__get_subscription_SeqNum(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_sub_seq_num_i * const subscription_core_2__p_seqNumToSend);
extern void subscription_core_2__get_subscription_monitoredItemQueue(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_monitoredItemQueue_i * const subscription_core_2__p_monitoredItemQueue);
extern void subscription_core_2__get_subscription_notifRepublishQueue(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_notifRepublishQueue_i * const subscription_core_2__p_republishQueue);
extern void subscription_core_2__get_subscription_priority(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_entier4 * const subscription_core_2__p_priority);
extern void subscription_core_2__get_subscription_publishInterval(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_opcua_duration_i * const subscription_core_2__p_publishInterval);
extern void subscription_core_2__get_subscription_session(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_session_i * const subscription_core_2__p_session);
extern void subscription_core_2__get_subscription_state(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_subscriptionState_i * const subscription_core_2__p_state);
extern void subscription_core_2__get_subscription_timer_id(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_timer_id_i * const subscription_core_2__p_timer_id);
extern void subscription_core_2__getall_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority_idx,
   t_entier4 * const subscription_core_2__p_priority);
extern void subscription_core_2__insert_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority_idx,
   const t_entier4 subscription_core_2__p_priority);
extern void subscription_core_2__is_valid_subscription_on_session(
   const constants__t_session_i subscription_core_2__p_session,
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_bool * const subscription_core_2__is_valid);
extern void subscription_core_2__log_subscription_request_received(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_entier4 subscription_core_2__p_transition);
extern void subscription_core_2__log_subscription_request_received_invalid_on_session(
   const constants__t_subscription_i subscription_core_2__p_subscription);
extern void subscription_core_2__log_subscription_transition(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_subscriptionState_i subscription_core_2__p_old_state,
   const constants__t_subscriptionState_i subscription_core_2__p_new_state,
   const t_entier4 subscription_core_2__p_transition);
extern void subscription_core_2__remove_session_priority_subscriptions_queue(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_prio_idx);
extern void subscription_core_2__remove_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority_idx);
extern void subscription_core_2__reset_session_publishRequestQueue(
   const constants__t_session_i subscription_core_2__p_session);
extern void subscription_core_2__reset_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription);
extern void subscription_core_2__reset_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription);
extern void subscription_core_2__set_card_session_seq_subscription(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_sub_idx);
extern void subscription_core_2__set_session_priority_subscriptions_queue(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_prio_idx,
   const constants__t_prioritySubQueue_i subscription_core_2__p_prioritySubQueue);
extern void subscription_core_2__set_session_publishRequestQueue(
   const constants__t_session_i subscription_core_2__p_session,
   const constants__t_publishReqQueue_i subscription_core_2__p_publishReqQueue);
extern void subscription_core_2__set_subscription(
   const constants__t_subscription_i subscription_core_2__p_subscription);
extern void subscription_core_2__set_subscription_LifetimeAndKeepAliveCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_entier4 subscription_core_2__p_LifetimeCounter,
   const t_entier4 subscription_core_2__p_KeepAliveCounter);
extern void subscription_core_2__set_subscription_MaxLifetimeAndKeepAliveCount(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_entier4 subscription_core_2__p_revLifetimeCount,
   const t_entier4 subscription_core_2__p_revMaxKeepAlive);
extern void subscription_core_2__set_subscription_MaxNotifsPerPublish(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_entier4 subscription_core_2__p_maxNotificationsPerPublish);
extern void subscription_core_2__set_subscription_MessageSent(
   const constants__t_subscription_i subscription_core_2__p_subscription);
extern void subscription_core_2__set_subscription_MoreNotifications(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_bool subscription_core_2__p_moreNotifs);
extern void subscription_core_2__set_subscription_PublishingEnabled(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_bool subscription_core_2__p_pubEnabled);
extern void subscription_core_2__set_subscription_SeqNum(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_sub_seq_num_i subscription_core_2__p_nextSeqNum);
extern void subscription_core_2__set_subscription_monitoredItemQueue(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_monitoredItemQueue_i subscription_core_2__p_monitoredItemQueue);
extern void subscription_core_2__set_subscription_notifRepublishQueue(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_notifRepublishQueue_i subscription_core_2__p_republishQueue);
extern void subscription_core_2__set_subscription_priority(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_entier4 subscription_core_2__p_priority);
extern void subscription_core_2__set_subscription_publishInterval(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_opcua_duration_i subscription_core_2__p_revPublishInterval);
extern void subscription_core_2__set_subscription_session(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_session_i subscription_core_2__p_session);
extern void subscription_core_2__set_subscription_state(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_subscriptionState_i subscription_core_2__p_state,
   const t_entier4 subscription_core_2__p_transition);
extern void subscription_core_2__set_subscription_timer_id(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_timer_id_i subscription_core_2__p_timer_id);
extern void subscription_core_2__setreset_subscription_MessageSent(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_bool subscription_core_2__p_bool);
extern void subscription_core_2__swap_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority_idx0,
   const t_entier4 subscription_core_2__p_priority_idx1);

#endif
