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

 Date                 : 05/11/2026 10:21:22

 C Translator Version : tradc Java V2.0 (2026-09-14)

******************************************************************************/

#ifndef _subscription_core_1_h
#define _subscription_core_1_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "subscription_core_2.h"
#include "subscription_priority_sub_queue_bs.h"
#include "subscription_priority_sub_queue_it_bs.h"

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

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void subscription_core_1__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define subscription_core_1__continue_iter_subscription_priority_queue subscription_priority_sub_queue_it_bs__continue_iter_subscription_priority_queue
#define subscription_core_1__decrement_subscription_KeepAliveCounter subscription_core_2__decrement_subscription_KeepAliveCounter
#define subscription_core_1__decrement_subscription_LifetimeCounter subscription_core_2__decrement_subscription_LifetimeCounter
#define subscription_core_1__get_card_session_seq_priority subscription_core_2__get_card_session_seq_priority
#define subscription_core_1__get_card_session_seq_subscription subscription_core_2__get_card_session_seq_subscription
#define subscription_core_1__get_session_publishRequestQueue subscription_core_2__get_session_publishRequestQueue
#define subscription_core_1__get_subscription_KeepAliveCounter subscription_core_2__get_subscription_KeepAliveCounter
#define subscription_core_1__get_subscription_LifetimeCounter subscription_core_2__get_subscription_LifetimeCounter
#define subscription_core_1__get_subscription_MaxNotifsPerPublish subscription_core_2__get_subscription_MaxNotifsPerPublish
#define subscription_core_1__get_subscription_MessageSent subscription_core_2__get_subscription_MessageSent
#define subscription_core_1__get_subscription_MoreNotifications subscription_core_2__get_subscription_MoreNotifications
#define subscription_core_1__get_subscription_PublishingEnabled subscription_core_2__get_subscription_PublishingEnabled
#define subscription_core_1__get_subscription_SeqNum subscription_core_2__get_subscription_SeqNum
#define subscription_core_1__get_subscription_monitoredItemQueue subscription_core_2__get_subscription_monitoredItemQueue
#define subscription_core_1__get_subscription_notifRepublishQueue subscription_core_2__get_subscription_notifRepublishQueue
#define subscription_core_1__get_subscription_priority subscription_core_2__get_subscription_priority
#define subscription_core_1__get_subscription_publishInterval subscription_core_2__get_subscription_publishInterval
#define subscription_core_1__get_subscription_state subscription_core_2__get_subscription_state
#define subscription_core_1__get_subscription_timer_id subscription_core_2__get_subscription_timer_id
#define subscription_core_1__getall_session subscription_core_2__getall_session
#define subscription_core_1__is_valid_subscription subscription_core_2__is_valid_subscription
#define subscription_core_1__is_valid_subscription_on_session subscription_core_2__is_valid_subscription_on_session
#define subscription_core_1__log_subscription_request_received subscription_core_2__log_subscription_request_received
#define subscription_core_1__log_subscription_request_received_invalid_on_session subscription_core_2__log_subscription_request_received_invalid_on_session
#define subscription_core_1__log_subscription_transition subscription_core_2__log_subscription_transition
#define subscription_core_1__reset_session_publishRequestQueue subscription_core_2__reset_session_publishRequestQueue
#define subscription_core_1__reset_subscription_KeepAliveCounter subscription_core_2__reset_subscription_KeepAliveCounter
#define subscription_core_1__reset_subscription_LifetimeCounter subscription_core_2__reset_subscription_LifetimeCounter
#define subscription_core_1__set_session_publishRequestQueue subscription_core_2__set_session_publishRequestQueue
#define subscription_core_1__set_subscription_MaxLifetimeAndKeepAliveCount subscription_core_2__set_subscription_MaxLifetimeAndKeepAliveCount
#define subscription_core_1__set_subscription_MaxNotifsPerPublish subscription_core_2__set_subscription_MaxNotifsPerPublish
#define subscription_core_1__set_subscription_MessageSent subscription_core_2__set_subscription_MessageSent
#define subscription_core_1__set_subscription_MoreNotifications subscription_core_2__set_subscription_MoreNotifications
#define subscription_core_1__set_subscription_PublishingEnabled subscription_core_2__set_subscription_PublishingEnabled
#define subscription_core_1__set_subscription_SeqNum subscription_core_2__set_subscription_SeqNum
#define subscription_core_1__set_subscription_publishInterval subscription_core_2__set_subscription_publishInterval
#define subscription_core_1__set_subscription_state subscription_core_2__set_subscription_state
#define subscription_core_1__set_subscription_timer_id subscription_core_2__set_subscription_timer_id

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void subscription_core_1__add_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const constants__t_session_i subscription_core_1__p_session,
   const constants__t_subscriptionState_i subscription_core_1__p_state,
   const t_bool subscription_core_1__p_firstMsgSent,
   const t_entier4 subscription_core_1__p_priority,
   const constants__t_opcua_duration_i subscription_core_1__p_revPublishInterval,
   const t_entier4 subscription_core_1__p_revLifetimeCount,
   const t_entier4 subscription_core_1__p_revKeepAliveCount,
   const t_entier4 subscription_core_1__p_revMaxKeepAlive,
   const t_entier4 subscription_core_1__p_maxNotificationsPerPublish,
   const t_bool subscription_core_1__p_publishEnabled,
   const constants__t_sub_seq_num_i subscription_core_1__p_seqNumInit,
   const constants__t_notifRepublishQueue_i subscription_core_1__p_republishQueue,
   const constants__t_monitoredItemQueue_i subscription_core_1__p_monitoredItemQueue,
   const constants__t_timer_id_i subscription_core_1__p_timerId,
   const constants__t_prioritySubQueue_i subscription_core_1__p_prio_subs_queue,
   const t_bool subscription_core_1__p_moreNotifs);
extern void subscription_core_1__check_add_subscription(
   const constants__t_session_i subscription_core_1__p_session,
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const t_entier4 subscription_core_1__p_priority,
   const t_bool subscription_core_1__p_isModifySub,
   constants__t_prioritySubQueue_i * const subscription_core_1__p_prio_subs_queue,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core_1__p_StatusCode_service);
extern void subscription_core_1__delete_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription);
extern void subscription_core_1__init_session_priority_subscriptions_iterator(
   const constants__t_session_i subscription_core_1__p_session,
   const t_entier4 subscription_core_1__p_prio_idx,
   constants__t_prioritySubQueueIterator_i * const subscription_core_1__p_prio_sub_it);
extern void subscription_core_1__reconfigure_session_priority_subscriptions_queue_last_sub(
   const constants__t_session_i subscription_core_1__p_session,
   const constants__t_subscription_i subscription_core_1__p_sub);

#endif
