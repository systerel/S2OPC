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

 File Name            : subscription_core_1.c

 Date                 : 05/11/2026 10:21:22

 C Translator Version : tradc Java V2.0 (2026-09-14)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "subscription_core_1.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_core_1__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_core_1__init_session_priority_subscriptions_iterator(
   const constants__t_session_i subscription_core_1__p_session,
   const t_entier4 subscription_core_1__p_prio_idx,
   constants__t_prioritySubQueueIterator_i * const subscription_core_1__p_prio_sub_it) {
   {
      constants__t_prioritySubQueue_i subscription_core_1__l_prio_sub_q;
      
      subscription_core_2__get_session_priority_subscriptions_queue(subscription_core_1__p_session,
         subscription_core_1__p_prio_idx,
         &subscription_core_1__l_prio_sub_q);
      subscription_priority_sub_queue_it_bs__init_prio_subscriptions_queue_iterator(subscription_core_1__l_prio_sub_q,
         subscription_core_1__p_prio_sub_it);
   }
}

void subscription_core_1__check_add_subscription(
   const constants__t_session_i subscription_core_1__p_session,
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const t_entier4 subscription_core_1__p_priority,
   const t_bool subscription_core_1__p_isModifySub,
   constants__t_prioritySubQueue_i * const subscription_core_1__p_prio_subs_queue,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core_1__p_StatusCode_service) {
   {
      t_entier4 subscription_core_1__l_nb_sub;
      t_entier4 subscription_core_1__l_prio_idx;
      t_entier4 subscription_core_1__l_nb_prio;
      t_bool subscription_core_1__l_sub_added_to_prio_q;
      
      subscription_core_2__get_card_session_seq_subscription(subscription_core_1__p_session,
         &subscription_core_1__l_nb_sub);
      subscription_core_2__get_session_priority_idx(subscription_core_1__p_session,
         subscription_core_1__p_priority,
         &subscription_core_1__l_prio_idx);
      subscription_core_2__get_card_session_seq_priority(subscription_core_1__p_session,
         &subscription_core_1__l_nb_prio);
      *subscription_core_1__p_prio_subs_queue = constants__c_prioritySubQueue_indet;
      subscription_core_1__l_sub_added_to_prio_q = false;
      if (subscription_core_1__p_isModifySub == true) {
         subscription_core_1__l_nb_sub = subscription_core_1__l_nb_sub -
            1;
      }
      if (subscription_core_1__l_nb_sub == constants__k_n_subscriptionPerSession_max ||
         (subscription_core_1__l_prio_idx == 0 &&
         subscription_core_1__l_nb_prio == constants__k_n_subscriptionPerSession_max)) {
         *subscription_core_1__p_StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_subscriptions;
      }
      else {
         if (subscription_core_1__l_prio_idx == 0) {
            subscription_priority_sub_queue_bs__allocate_new_prio_queue(subscription_core_1__p_subscription,
               &subscription_core_1__l_sub_added_to_prio_q,
               subscription_core_1__p_prio_subs_queue);
         }
         else {
            subscription_core_2__get_session_priority_subscriptions_queue(subscription_core_1__p_session,
               subscription_core_1__l_prio_idx,
               subscription_core_1__p_prio_subs_queue);
            subscription_priority_sub_queue_bs__add_subscription_to_prio_queue(*subscription_core_1__p_prio_subs_queue,
               subscription_core_1__p_subscription,
               &subscription_core_1__l_sub_added_to_prio_q);
         }
         if (subscription_core_1__l_sub_added_to_prio_q == true) {
            *subscription_core_1__p_StatusCode_service = constants_statuscodes_bs__e_sc_ok;
         }
         else {
            *subscription_core_1__p_StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
         }
      }
   }
}

void subscription_core_1__add_subscription(
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
   const t_bool subscription_core_1__p_moreNotifs) {
   {
      t_entier4 subscription_core_1__l_prio_idx;
      t_entier4 subscription_core_1__l_prio_idx1;
      t_bool subscription_core_1__l_continue;
      t_entier4 subscription_core_1__l_prio;
      constants__t_prioritySubQueue_i subscription_core_1__l_sub_queue;
      t_entier4 subscription_core_1__l_nb_sub;
      t_entier4 subscription_core_1__l_nb_prio;
      
      subscription_core_2__set_subscription(subscription_core_1__p_subscription);
      subscription_core_2__set_subscription_state(subscription_core_1__p_subscription,
         subscription_core_1__p_state,
         3);
      subscription_core_2__set_subscription_session(subscription_core_1__p_subscription,
         subscription_core_1__p_session);
      subscription_core_2__set_subscription_priority(subscription_core_1__p_subscription,
         subscription_core_1__p_priority);
      subscription_core_2__set_subscription_publishInterval(subscription_core_1__p_subscription,
         subscription_core_1__p_revPublishInterval);
      subscription_core_2__set_subscription_MaxLifetimeAndKeepAliveCount(subscription_core_1__p_subscription,
         subscription_core_1__p_revLifetimeCount,
         subscription_core_1__p_revMaxKeepAlive);
      subscription_core_2__set_subscription_LifetimeAndKeepAliveCounter(subscription_core_1__p_subscription,
         subscription_core_1__p_revLifetimeCount,
         subscription_core_1__p_revKeepAliveCount);
      subscription_core_2__set_subscription_MaxNotifsPerPublish(subscription_core_1__p_subscription,
         subscription_core_1__p_maxNotificationsPerPublish);
      subscription_core_2__setreset_subscription_MessageSent(subscription_core_1__p_subscription,
         subscription_core_1__p_firstMsgSent);
      subscription_core_2__set_subscription_PublishingEnabled(subscription_core_1__p_subscription,
         subscription_core_1__p_publishEnabled);
      subscription_core_2__set_subscription_MoreNotifications(subscription_core_1__p_subscription,
         subscription_core_1__p_moreNotifs);
      subscription_core_2__set_subscription_SeqNum(subscription_core_1__p_subscription,
         subscription_core_1__p_seqNumInit);
      subscription_core_2__set_subscription_notifRepublishQueue(subscription_core_1__p_subscription,
         subscription_core_1__p_republishQueue);
      subscription_core_2__set_subscription_monitoredItemQueue(subscription_core_1__p_subscription,
         subscription_core_1__p_monitoredItemQueue);
      subscription_core_2__set_subscription_timer_id(subscription_core_1__p_subscription,
         subscription_core_1__p_timerId);
      subscription_core_2__log_subscription_transition(subscription_core_1__p_subscription,
         constants__c_subscriptionState_indet,
         subscription_core_1__p_state,
         3);
      subscription_core_2__get_card_session_seq_subscription(subscription_core_1__p_session,
         &subscription_core_1__l_nb_sub);
      subscription_core_2__get_card_session_seq_priority(subscription_core_1__p_session,
         &subscription_core_1__l_nb_prio);
      subscription_core_2__get_session_priority_idx(subscription_core_1__p_session,
         subscription_core_1__p_priority,
         &subscription_core_1__l_prio_idx);
      if (subscription_core_1__l_prio_idx == 0) {
         subscription_core_1__l_prio_idx = subscription_core_1__l_nb_prio +
            1;
         subscription_core_2__getall_session_seq_priority(subscription_core_1__p_session,
            subscription_core_1__l_nb_prio,
            &subscription_core_1__l_prio);
         subscription_core_1__l_continue = 1 < subscription_core_1__l_prio_idx &&
            subscription_core_1__l_prio != constants__c_priority_indet &&
            subscription_core_1__p_priority < subscription_core_1__l_prio;
         if (subscription_core_1__l_continue == true) {
            while (subscription_core_1__l_continue == true) {
               subscription_core_1__l_prio_idx1 = subscription_core_1__l_prio_idx -
                  1;
               subscription_core_2__swap_session_seq_priority(subscription_core_1__p_session,
                  subscription_core_1__l_prio_idx,
                  subscription_core_1__l_prio_idx1);
               subscription_core_2__get_session_priority_subscriptions_queue(subscription_core_1__p_session,
                  subscription_core_1__l_prio_idx1,
                  &subscription_core_1__l_sub_queue);
               subscription_core_2__set_session_priority_subscriptions_queue(subscription_core_1__p_session,
                  subscription_core_1__l_prio_idx,
                  subscription_core_1__l_sub_queue);
               subscription_core_1__l_prio_idx = subscription_core_1__l_prio_idx1;
               subscription_core_2__getall_session_seq_priority(subscription_core_1__p_session,
                  subscription_core_1__l_prio_idx1,
                  &subscription_core_1__l_prio);
               subscription_core_1__l_continue = 1 < subscription_core_1__l_prio_idx &&
                  subscription_core_1__l_prio != constants__c_priority_indet &&
                  subscription_core_1__p_priority < subscription_core_1__l_prio;
            }
         }
         subscription_core_2__insert_session_seq_priority(subscription_core_1__p_session,
            subscription_core_1__l_prio_idx,
            subscription_core_1__p_priority);
         subscription_core_2__set_session_priority_subscriptions_queue(subscription_core_1__p_session,
            subscription_core_1__l_prio_idx,
            subscription_core_1__p_prio_subs_queue);
      }
      subscription_core_2__set_card_session_seq_subscription(subscription_core_1__p_session,
         subscription_core_1__l_nb_sub +
         1);
   }
}

void subscription_core_1__delete_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription) {
   {
      constants__t_subscriptionState_i subscription_core_1__l_old_state;
      t_entier4 subscription_core_1__l_prio;
      constants__t_session_i subscription_core_1__l_session;
      t_entier4 subscription_core_1__l_priority;
      t_entier4 subscription_core_1__l_sub_prio_idx;
      t_entier4 subscription_core_1__l_prio_idx_it;
      t_entier4 subscription_core_1__l_prio_idx_it1;
      t_entier4 subscription_core_1__l_nb_prio;
      t_entier4 subscription_core_1__l_nb_sub_prio;
      constants__t_prioritySubQueue_i subscription_core_1__l_prio_subs_queue;
      constants__t_prioritySubQueue_i subscription_core_1__l_queue;
      
      subscription_core_2__get_subscription_state(subscription_core_1__p_subscription,
         &subscription_core_1__l_old_state);
      subscription_core_2__log_subscription_transition(subscription_core_1__p_subscription,
         subscription_core_1__l_old_state,
         constants__c_subscriptionState_indet,
         25);
      subscription_core_2__get_subscription_session(subscription_core_1__p_subscription,
         &subscription_core_1__l_session);
      subscription_core_2__get_subscription_priority(subscription_core_1__p_subscription,
         &subscription_core_1__l_priority);
      subscription_core_2__get_session_priority_idx(subscription_core_1__l_session,
         subscription_core_1__l_priority,
         &subscription_core_1__l_sub_prio_idx);
      if (subscription_core_1__l_sub_prio_idx != 0) {
         subscription_core_2__get_card_session_seq_priority(subscription_core_1__l_session,
            &subscription_core_1__l_nb_prio);
         subscription_core_2__get_session_priority_subscriptions_queue(subscription_core_1__l_session,
            subscription_core_1__l_sub_prio_idx,
            &subscription_core_1__l_prio_subs_queue);
         subscription_priority_sub_queue_bs__get_queue_nb_subs(subscription_core_1__l_prio_subs_queue,
            &subscription_core_1__l_nb_sub_prio);
         if (subscription_core_1__l_nb_sub_prio == 1) {
            subscription_core_2__remove_session_seq_priority(subscription_core_1__l_session,
               subscription_core_1__l_sub_prio_idx);
            subscription_core_1__l_prio_idx_it = subscription_core_1__l_sub_prio_idx;
            if (subscription_core_1__l_prio_idx_it < subscription_core_1__l_nb_prio) {
               while (subscription_core_1__l_prio_idx_it < subscription_core_1__l_nb_prio) {
                  subscription_core_1__l_prio_idx_it1 = subscription_core_1__l_prio_idx_it +
                     1;
                  subscription_core_2__get_session_seq_priority(subscription_core_1__l_session,
                     subscription_core_1__l_prio_idx_it1,
                     &subscription_core_1__l_prio);
                  subscription_core_2__swap_session_seq_priority(subscription_core_1__l_session,
                     subscription_core_1__l_prio_idx_it,
                     subscription_core_1__l_prio_idx_it1);
                  subscription_core_2__get_session_priority_subscriptions_queue(subscription_core_1__l_session,
                     subscription_core_1__l_prio_idx_it1,
                     &subscription_core_1__l_queue);
                  subscription_core_2__set_session_priority_subscriptions_queue(subscription_core_1__l_session,
                     subscription_core_1__l_prio_idx_it,
                     subscription_core_1__l_queue);
                  subscription_core_1__l_prio_idx_it = subscription_core_1__l_prio_idx_it1;
               }
            }
            subscription_core_2__remove_session_priority_subscriptions_queue(subscription_core_1__l_session,
               subscription_core_1__l_nb_prio);
            subscription_priority_sub_queue_bs__clear_and_deallocate_prio_queue(subscription_core_1__l_prio_subs_queue);
         }
         else if (subscription_core_1__l_nb_sub_prio > 1) {
            subscription_priority_sub_queue_bs__remove_subscription_from_queue(subscription_core_1__l_prio_subs_queue,
               subscription_core_1__p_subscription);
         }
      }
      subscription_core_2__del_subscription(subscription_core_1__p_subscription);
      subscription_core_2__decrease_card_session_seq_subscription(subscription_core_1__l_session);
   }
}

void subscription_core_1__reconfigure_session_priority_subscriptions_queue_last_sub(
   const constants__t_session_i subscription_core_1__p_session,
   const constants__t_subscription_i subscription_core_1__p_sub) {
   {
      t_entier4 subscription_core_1__l_prio;
      t_entier4 subscription_core_1__l_prio_idx;
      constants__t_prioritySubQueue_i subscription_core_1__l_prio_sub_q;
      
      subscription_core_2__get_subscription_priority(subscription_core_1__p_sub,
         &subscription_core_1__l_prio);
      subscription_core_2__get_session_priority_idx(subscription_core_1__p_session,
         subscription_core_1__l_prio,
         &subscription_core_1__l_prio_idx);
      if (subscription_core_1__l_prio_idx != 0) {
         subscription_core_2__get_session_priority_subscriptions_queue(subscription_core_1__p_session,
            subscription_core_1__l_prio_idx,
            &subscription_core_1__l_prio_sub_q);
         subscription_priority_sub_queue_bs__reconfigure_priority_subscriptions_queue_last_sub(subscription_core_1__l_prio_sub_q,
            subscription_core_1__p_sub);
      }
   }
}

