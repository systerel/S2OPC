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

 File Name            : subscription_core_2.c

 Date                 : 03/11/2026 09:52:27

 C Translator Version : tradc Java V2.0 (2026-09-14)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "subscription_core_2.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 subscription_core_2__a_KeepAliveCounter_i[constants__t_subscription_i_max+1];
t_entier4 subscription_core_2__a_LifetimeCounter_i[constants__t_subscription_i_max+1];
t_bool subscription_core_2__a_MessageSent_i[constants__t_subscription_i_max+1];
t_bool subscription_core_2__a_MoreNotifications_i[constants__t_subscription_i_max+1];
t_bool subscription_core_2__a_PublishingEnabled_i[constants__t_subscription_i_max+1];
constants__t_sub_seq_num_i subscription_core_2__a_SeqNum_i[constants__t_subscription_i_max+1];
t_entier4 subscription_core_2__a_keepAliveExpCount_i[constants__t_subscription_i_max+1];
t_entier4 subscription_core_2__a_lifetimeExpCount_i[constants__t_subscription_i_max+1];
t_entier4 subscription_core_2__a_maxNotifsPerPublish_i[constants__t_subscription_i_max+1];
constants__t_monitoredItemQueue_i subscription_core_2__a_monitoredItemQueue_i[constants__t_subscription_i_max+1];
constants__t_notifRepublishQueue_i subscription_core_2__a_notifRepublishQueue_i[constants__t_subscription_i_max+1];
constants__t_opcua_duration_i subscription_core_2__a_publishInterval_i[constants__t_subscription_i_max+1];
constants__t_publishReqQueue_i subscription_core_2__a_publishRequestQueue_i[constants__t_session_i_max+1];
constants__t_timer_id_i subscription_core_2__a_publishTimer_i[constants__t_subscription_i_max+1];
t_entier4 subscription_core_2__a_session_nb_priorities_i[constants__t_session_i_max+1];
t_entier4 subscription_core_2__a_session_nb_subscriptions_i[constants__t_session_i_max+1];
t_entier4 subscription_core_2__a_session_priority_idx_i[constants__t_session_i_max+1][constants__t_priority_i_max+1];
constants__t_prioritySubQueue_i subscription_core_2__a_session_priority_subscriptions_queue_i[constants__t_session_i_max+1][constants__t_prio_idx_i_max+1];
t_entier4 subscription_core_2__a_session_seq_priority_i[constants__t_session_i_max+1][constants__t_prio_idx_i_max+1];
t_entier4 subscription_core_2__a_subscription_priority_i[constants__t_subscription_i_max+1];
constants__t_session_i subscription_core_2__a_subscription_session_i[constants__t_subscription_i_max+1];
constants__t_subscriptionState_i subscription_core_2__a_subscription_state_i[constants__t_subscription_i_max+1];
t_bool subscription_core_2__s_subscription_i[constants__t_subscription_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_core_2__INITIALISATION(void) {
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__s_subscription_i[i] = false;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_subscription_state_i[i] = constants__c_subscriptionState_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_subscription_session_i[i] = constants__c_session_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_publishInterval_i[i] = constants__c_opcua_duration_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_lifetimeExpCount_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_keepAliveExpCount_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_maxNotifsPerPublish_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_subscription_priority_i[i] = constants__c_priority_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_session_nb_priorities_i[i] = 0;
      }
   }
   {
      t_entier4 i, j;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         for (j = constants__t_prio_idx_i_max; 0 <= j; j = j - 1) {
            subscription_core_2__a_session_seq_priority_i[i][j] = constants__c_priority_indet;
         }
      }
   }
   {
      t_entier4 i, j;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         for (j = constants__t_priority_i_max; 0 <= j; j = j - 1) {
            subscription_core_2__a_session_priority_idx_i[i][j] = 0;
         }
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_session_nb_subscriptions_i[i] = 0;
      }
   }
   {
      t_entier4 i, j;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         for (j = constants__t_prio_idx_i_max; 0 <= j; j = j - 1) {
            subscription_core_2__a_session_priority_subscriptions_queue_i[i][j] = constants__c_prioritySubQueue_indet;
         }
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_PublishingEnabled_i[i] = false;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_MoreNotifications_i[i] = false;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_LifetimeCounter_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_KeepAliveCounter_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_MessageSent_i[i] = false;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_SeqNum_i[i] = constants__c_sub_seq_num_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_publishRequestQueue_i[i] = constants__c_publishReqQueue_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_notifRepublishQueue_i[i] = constants__c_notifRepublishQueue_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_monitoredItemQueue_i[i] = constants__c_monitoredItemQueue_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_2__a_publishTimer_i[i] = constants__c_timer_id_indet;
      }
   }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_core_2__local_log_subscription_transition(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_subscriptionState_i subscription_core_2__p_old_state,
   const constants__t_subscriptionState_i subscription_core_2__p_new_state,
   const t_entier4 subscription_core_2__p_transition) {
   {
      t_bool subscription_core_2__l_moreNotifs;
      t_entier4 subscription_core_2__l_lifetimeCounter;
      t_entier4 subscription_core_2__l_keepAliveCounter;
      t_bool subscription_core_2__l_messageSent;
      t_bool subscription_core_2__l_publishingEnabled;
      
      subscription_core_2__l_moreNotifs = subscription_core_2__a_MoreNotifications_i[subscription_core_2__p_subscription];
      subscription_core_2__l_lifetimeCounter = subscription_core_2__a_LifetimeCounter_i[subscription_core_2__p_subscription];
      subscription_core_2__l_keepAliveCounter = subscription_core_2__a_KeepAliveCounter_i[subscription_core_2__p_subscription];
      subscription_core_2__l_messageSent = subscription_core_2__a_MessageSent_i[subscription_core_2__p_subscription];
      subscription_core_2__l_publishingEnabled = subscription_core_2__a_PublishingEnabled_i[subscription_core_2__p_subscription];
      subscription_core_bs__trace_subscription_transition(subscription_core_2__p_subscription,
         subscription_core_2__p_old_state,
         subscription_core_2__p_new_state,
         subscription_core_2__p_transition,
         subscription_core_2__l_moreNotifs,
         subscription_core_2__l_lifetimeCounter,
         subscription_core_2__l_keepAliveCounter,
         subscription_core_2__l_messageSent,
         subscription_core_2__l_publishingEnabled);
   }
}

void subscription_core_2__local_is_valid_subscription(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_bool * const subscription_core_2__is_valid) {
   constants__is_t_subscription(subscription_core_2__p_subscription,
      subscription_core_2__is_valid);
   if (*subscription_core_2__is_valid == true) {
      *subscription_core_2__is_valid = subscription_core_2__s_subscription_i[subscription_core_2__p_subscription];
   }
}

void subscription_core_2__is_valid_subscription_on_session(
   const constants__t_session_i subscription_core_2__p_session,
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_bool * const subscription_core_2__is_valid) {
   {
      t_bool subscription_core_2__l_is_valid;
      constants__t_session_i subscription_core_2__l_session;
      
      subscription_core_2__l_session = constants__c_session_indet;
      subscription_core_2__local_is_valid_subscription(subscription_core_2__p_subscription,
         &subscription_core_2__l_is_valid);
      if (subscription_core_2__l_is_valid == true) {
         subscription_core_2__l_session = subscription_core_2__a_subscription_session_i[subscription_core_2__p_subscription];
      }
      *subscription_core_2__is_valid = subscription_core_2__l_is_valid == true &&
         subscription_core_2__l_session == subscription_core_2__p_session;
   }
}

void subscription_core_2__set_subscription(
   const constants__t_subscription_i subscription_core_2__p_subscription) {
   subscription_core_2__s_subscription_i[subscription_core_2__p_subscription] = true;
}

void subscription_core_2__get_card_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   t_entier4 * const subscription_core_2__p_priority_idx) {
   *subscription_core_2__p_priority_idx = subscription_core_2__a_session_nb_priorities_i[subscription_core_2__p_session];
}

void subscription_core_2__get_card_session_seq_subscription(
   const constants__t_session_i subscription_core_2__p_session,
   t_entier4 * const subscription_core_2__p_sub_idx) {
   *subscription_core_2__p_sub_idx = subscription_core_2__a_session_nb_subscriptions_i[subscription_core_2__p_session];
}

void subscription_core_2__get_subscription_session(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_session_i * const subscription_core_2__p_session) {
   *subscription_core_2__p_session = subscription_core_2__a_subscription_session_i[subscription_core_2__p_subscription];
}

void subscription_core_2__set_subscription_session(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_session_i subscription_core_2__p_session) {
   subscription_core_2__a_subscription_session_i[subscription_core_2__p_subscription] = subscription_core_2__p_session;
}

void subscription_core_2__remove_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority_idx) {
   {
      t_entier4 subscription_core_2__l_prio;
      
      subscription_core_2__l_prio = subscription_core_2__a_session_seq_priority_i[subscription_core_2__p_session][subscription_core_2__p_priority_idx];
      subscription_core_2__a_session_seq_priority_i[subscription_core_2__p_session][subscription_core_2__p_priority_idx] = constants__c_priority_indet;
      subscription_core_2__a_session_priority_idx_i[subscription_core_2__p_session][subscription_core_2__l_prio] = 0;
      subscription_core_2__a_session_nb_priorities_i[subscription_core_2__p_session] = subscription_core_2__a_session_nb_priorities_i[subscription_core_2__p_session] -
         1;
   }
}

void subscription_core_2__insert_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority_idx,
   const t_entier4 subscription_core_2__p_priority) {
   subscription_core_2__a_session_seq_priority_i[subscription_core_2__p_session][subscription_core_2__p_priority_idx] = subscription_core_2__p_priority;
   subscription_core_2__a_session_priority_idx_i[subscription_core_2__p_session][subscription_core_2__p_priority] = subscription_core_2__p_priority_idx;
   subscription_core_2__a_session_nb_priorities_i[subscription_core_2__p_session] = subscription_core_2__a_session_nb_priorities_i[subscription_core_2__p_session] +
      1;
}

void subscription_core_2__swap_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority_idx0,
   const t_entier4 subscription_core_2__p_priority_idx1) {
   {
      t_entier4 subscription_core_2__l_prio;
      
      subscription_core_2__l_prio = subscription_core_2__a_session_seq_priority_i[subscription_core_2__p_session][subscription_core_2__p_priority_idx1];
      subscription_core_2__a_session_seq_priority_i[subscription_core_2__p_session][subscription_core_2__p_priority_idx0] = subscription_core_2__l_prio;
      subscription_core_2__a_session_seq_priority_i[subscription_core_2__p_session][subscription_core_2__p_priority_idx1] = constants__c_priority_indet;
      subscription_core_2__a_session_priority_idx_i[subscription_core_2__p_session][subscription_core_2__l_prio] = subscription_core_2__p_priority_idx0;
   }
}

void subscription_core_2__set_card_session_seq_subscription(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_sub_idx) {
   subscription_core_2__a_session_nb_subscriptions_i[subscription_core_2__p_session] = subscription_core_2__p_sub_idx;
}

void subscription_core_2__decrease_card_session_seq_subscription(
   const constants__t_session_i subscription_core_2__p_session) {
   {
      t_entier4 subscription_core_2__l_card;
      
      subscription_core_2__l_card = subscription_core_2__a_session_nb_subscriptions_i[subscription_core_2__p_session];
      if (subscription_core_2__l_card >= 1) {
         subscription_core_2__a_session_nb_subscriptions_i[subscription_core_2__p_session] = subscription_core_2__l_card -
            1;
      }
   }
}

void subscription_core_2__getall_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority_idx,
   t_entier4 * const subscription_core_2__p_priority) {
   *subscription_core_2__p_priority = subscription_core_2__a_session_seq_priority_i[subscription_core_2__p_session][subscription_core_2__p_priority_idx];
}

void subscription_core_2__get_session_seq_priority(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority_idx,
   t_entier4 * const subscription_core_2__p_priority) {
   *subscription_core_2__p_priority = subscription_core_2__a_session_seq_priority_i[subscription_core_2__p_session][subscription_core_2__p_priority_idx];
}

void subscription_core_2__get_session_priority_idx(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_priority,
   t_entier4 * const subscription_core_2__p_priority_idx) {
   *subscription_core_2__p_priority_idx = subscription_core_2__a_session_priority_idx_i[subscription_core_2__p_session][subscription_core_2__p_priority];
}

void subscription_core_2__del_subscription(
   const constants__t_subscription_i subscription_core_2__p_subscription) {
   subscription_core_2__s_subscription_i[subscription_core_2__p_subscription] = false;
   subscription_core_2__a_subscription_state_i[subscription_core_2__p_subscription] = constants__c_subscriptionState_indet;
   subscription_core_2__a_subscription_session_i[subscription_core_2__p_subscription] = constants__c_session_indet;
   subscription_core_2__a_publishInterval_i[subscription_core_2__p_subscription] = constants__c_opcua_duration_indet;
   subscription_core_2__a_lifetimeExpCount_i[subscription_core_2__p_subscription] = 0;
   subscription_core_2__a_keepAliveExpCount_i[subscription_core_2__p_subscription] = 0;
   subscription_core_2__a_maxNotifsPerPublish_i[subscription_core_2__p_subscription] = 0;
   subscription_core_2__a_PublishingEnabled_i[subscription_core_2__p_subscription] = false;
   subscription_core_2__a_MoreNotifications_i[subscription_core_2__p_subscription] = false;
   subscription_core_2__a_LifetimeCounter_i[subscription_core_2__p_subscription] = 0;
   subscription_core_2__a_KeepAliveCounter_i[subscription_core_2__p_subscription] = 0;
   subscription_core_2__a_MessageSent_i[subscription_core_2__p_subscription] = false;
   subscription_core_2__a_SeqNum_i[subscription_core_2__p_subscription] = constants__c_sub_seq_num_indet;
   subscription_core_2__a_notifRepublishQueue_i[subscription_core_2__p_subscription] = constants__c_notifRepublishQueue_indet;
   subscription_core_2__a_monitoredItemQueue_i[subscription_core_2__p_subscription] = constants__c_monitoredItemQueue_indet;
   subscription_core_2__a_publishTimer_i[subscription_core_2__p_subscription] = constants__c_timer_id_indet;
   subscription_core_2__a_subscription_priority_i[subscription_core_2__p_subscription] = constants__c_priority_indet;
}

void subscription_core_2__get_subscription_priority(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_entier4 * const subscription_core_2__p_priority) {
   *subscription_core_2__p_priority = subscription_core_2__a_subscription_priority_i[subscription_core_2__p_subscription];
}

void subscription_core_2__get_session_priority_subscriptions_queue(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_prio_idx,
   constants__t_prioritySubQueue_i * const subscription_core_2__p_prioritySubQueue) {
   *subscription_core_2__p_prioritySubQueue = subscription_core_2__a_session_priority_subscriptions_queue_i[subscription_core_2__p_session][subscription_core_2__p_prio_idx];
}

void subscription_core_2__set_session_priority_subscriptions_queue(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_prio_idx,
   const constants__t_prioritySubQueue_i subscription_core_2__p_prioritySubQueue) {
   subscription_core_2__a_session_priority_subscriptions_queue_i[subscription_core_2__p_session][subscription_core_2__p_prio_idx] = subscription_core_2__p_prioritySubQueue;
}

void subscription_core_2__remove_session_priority_subscriptions_queue(
   const constants__t_session_i subscription_core_2__p_session,
   const t_entier4 subscription_core_2__p_prio_idx) {
   subscription_core_2__a_session_priority_subscriptions_queue_i[subscription_core_2__p_session][subscription_core_2__p_prio_idx] = constants__c_prioritySubQueue_indet;
}

void subscription_core_2__set_subscription_priority(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_entier4 subscription_core_2__p_priority) {
   subscription_core_2__a_subscription_priority_i[subscription_core_2__p_subscription] = subscription_core_2__p_priority;
}

void subscription_core_2__get_subscription_publishInterval(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_opcua_duration_i * const subscription_core_2__p_publishInterval) {
   *subscription_core_2__p_publishInterval = subscription_core_2__a_publishInterval_i[subscription_core_2__p_subscription];
}

void subscription_core_2__set_subscription_publishInterval(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_opcua_duration_i subscription_core_2__p_revPublishInterval) {
   subscription_core_2__a_publishInterval_i[subscription_core_2__p_subscription] = subscription_core_2__p_revPublishInterval;
}

void subscription_core_2__set_subscription_timer_id(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_timer_id_i subscription_core_2__p_timer_id) {
   subscription_core_2__a_publishTimer_i[subscription_core_2__p_subscription] = subscription_core_2__p_timer_id;
}

void subscription_core_2__get_subscription_timer_id(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_timer_id_i * const subscription_core_2__p_timer_id) {
   *subscription_core_2__p_timer_id = subscription_core_2__a_publishTimer_i[subscription_core_2__p_subscription];
}

void subscription_core_2__set_subscription_state(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_subscriptionState_i subscription_core_2__p_state,
   const t_entier4 subscription_core_2__p_transition) {
   {
      constants__t_subscriptionState_i subscription_core_2__l_old_state;
      
      subscription_core_2__l_old_state = subscription_core_2__a_subscription_state_i[subscription_core_2__p_subscription];
      subscription_core_2__local_log_subscription_transition(subscription_core_2__p_subscription,
         subscription_core_2__l_old_state,
         subscription_core_2__p_state,
         subscription_core_2__p_transition);
      subscription_core_2__a_subscription_state_i[subscription_core_2__p_subscription] = subscription_core_2__p_state;
   }
}

void subscription_core_2__get_subscription_state(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_subscriptionState_i * const subscription_core_2__p_state) {
   *subscription_core_2__p_state = subscription_core_2__a_subscription_state_i[subscription_core_2__p_subscription];
}

void subscription_core_2__set_subscription_MoreNotifications(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_bool subscription_core_2__p_moreNotifs) {
   subscription_core_2__a_MoreNotifications_i[subscription_core_2__p_subscription] = subscription_core_2__p_moreNotifs;
}

void subscription_core_2__get_subscription_MoreNotifications(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_bool * const subscription_core_2__p_moreNotifs) {
   *subscription_core_2__p_moreNotifs = subscription_core_2__a_MoreNotifications_i[subscription_core_2__p_subscription];
}

void subscription_core_2__set_subscription_MaxLifetimeAndKeepAliveCount(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_entier4 subscription_core_2__p_revLifetimeCount,
   const t_entier4 subscription_core_2__p_revMaxKeepAlive) {
   subscription_core_2__a_lifetimeExpCount_i[subscription_core_2__p_subscription] = subscription_core_2__p_revLifetimeCount;
   subscription_core_2__a_keepAliveExpCount_i[subscription_core_2__p_subscription] = subscription_core_2__p_revMaxKeepAlive;
}

void subscription_core_2__set_subscription_LifetimeAndKeepAliveCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_entier4 subscription_core_2__p_LifetimeCounter,
   const t_entier4 subscription_core_2__p_KeepAliveCounter) {
   subscription_core_2__a_LifetimeCounter_i[subscription_core_2__p_subscription] = subscription_core_2__p_LifetimeCounter;
   subscription_core_2__a_KeepAliveCounter_i[subscription_core_2__p_subscription] = subscription_core_2__p_KeepAliveCounter;
}

void subscription_core_2__decrement_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription) {
   subscription_core_2__a_LifetimeCounter_i[subscription_core_2__p_subscription] = subscription_core_2__a_LifetimeCounter_i[subscription_core_2__p_subscription] -
      1;
}

void subscription_core_2__reset_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription) {
   subscription_core_2__a_LifetimeCounter_i[subscription_core_2__p_subscription] = subscription_core_2__a_lifetimeExpCount_i[subscription_core_2__p_subscription];
}

void subscription_core_2__get_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_entier4 * const subscription_core_2__p_lifetimeCounter) {
   *subscription_core_2__p_lifetimeCounter = subscription_core_2__a_LifetimeCounter_i[subscription_core_2__p_subscription];
}

void subscription_core_2__decrement_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription) {
   subscription_core_2__a_KeepAliveCounter_i[subscription_core_2__p_subscription] = subscription_core_2__a_KeepAliveCounter_i[subscription_core_2__p_subscription] -
      1;
}

void subscription_core_2__reset_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription) {
   subscription_core_2__a_KeepAliveCounter_i[subscription_core_2__p_subscription] = subscription_core_2__a_keepAliveExpCount_i[subscription_core_2__p_subscription];
}

void subscription_core_2__get_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_entier4 * const subscription_core_2__p_keepAliveCounter) {
   *subscription_core_2__p_keepAliveCounter = subscription_core_2__a_KeepAliveCounter_i[subscription_core_2__p_subscription];
}

void subscription_core_2__set_subscription_MaxNotifsPerPublish(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_entier4 subscription_core_2__p_maxNotificationsPerPublish) {
   subscription_core_2__a_maxNotifsPerPublish_i[subscription_core_2__p_subscription] = subscription_core_2__p_maxNotificationsPerPublish;
}

void subscription_core_2__get_subscription_MaxNotifsPerPublish(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_entier4 * const subscription_core_2__p_maxNotificationsPerPublish) {
   *subscription_core_2__p_maxNotificationsPerPublish = subscription_core_2__a_maxNotifsPerPublish_i[subscription_core_2__p_subscription];
}

void subscription_core_2__set_subscription_MessageSent(
   const constants__t_subscription_i subscription_core_2__p_subscription) {
   subscription_core_2__a_MessageSent_i[subscription_core_2__p_subscription] = true;
}

void subscription_core_2__setreset_subscription_MessageSent(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_bool subscription_core_2__p_bool) {
   subscription_core_2__a_MessageSent_i[subscription_core_2__p_subscription] = subscription_core_2__p_bool;
}

void subscription_core_2__get_subscription_MessageSent(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_bool * const subscription_core_2__p_firstMsgSent) {
   *subscription_core_2__p_firstMsgSent = subscription_core_2__a_MessageSent_i[subscription_core_2__p_subscription];
}

void subscription_core_2__set_subscription_PublishingEnabled(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_bool subscription_core_2__p_pubEnabled) {
   subscription_core_2__a_PublishingEnabled_i[subscription_core_2__p_subscription] = subscription_core_2__p_pubEnabled;
   subscription_core_2__a_MoreNotifications_i[subscription_core_2__p_subscription] = false;
   subscription_core_2__a_LifetimeCounter_i[subscription_core_2__p_subscription] = subscription_core_2__a_lifetimeExpCount_i[subscription_core_2__p_subscription];
}

void subscription_core_2__get_subscription_PublishingEnabled(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   t_bool * const subscription_core_2__p_pubEnabled) {
   *subscription_core_2__p_pubEnabled = subscription_core_2__a_PublishingEnabled_i[subscription_core_2__p_subscription];
}

void subscription_core_2__set_subscription_SeqNum(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_sub_seq_num_i subscription_core_2__p_nextSeqNum) {
   subscription_core_2__a_SeqNum_i[subscription_core_2__p_subscription] = subscription_core_2__p_nextSeqNum;
}

void subscription_core_2__get_subscription_SeqNum(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_sub_seq_num_i * const subscription_core_2__p_seqNumToSend) {
   *subscription_core_2__p_seqNumToSend = subscription_core_2__a_SeqNum_i[subscription_core_2__p_subscription];
}

void subscription_core_2__reset_session_publishRequestQueue(
   const constants__t_session_i subscription_core_2__p_session) {
   subscription_core_2__a_publishRequestQueue_i[subscription_core_2__p_session] = constants__c_publishReqQueue_indet;
}

void subscription_core_2__set_session_publishRequestQueue(
   const constants__t_session_i subscription_core_2__p_session,
   const constants__t_publishReqQueue_i subscription_core_2__p_publishReqQueue) {
   subscription_core_2__a_publishRequestQueue_i[subscription_core_2__p_session] = subscription_core_2__p_publishReqQueue;
}

void subscription_core_2__get_session_publishRequestQueue(
   const constants__t_session_i subscription_core_2__p_session,
   t_bool * const subscription_core_2__p_bres,
   constants__t_publishReqQueue_i * const subscription_core_2__p_publishReqQueue) {
   *subscription_core_2__p_publishReqQueue = subscription_core_2__a_publishRequestQueue_i[subscription_core_2__p_session];
   *subscription_core_2__p_bres = constants__c_publishReqQueue_indet != *subscription_core_2__p_publishReqQueue;
}

void subscription_core_2__get_subscription_notifRepublishQueue(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_notifRepublishQueue_i * const subscription_core_2__p_republishQueue) {
   *subscription_core_2__p_republishQueue = subscription_core_2__a_notifRepublishQueue_i[subscription_core_2__p_subscription];
}

void subscription_core_2__set_subscription_notifRepublishQueue(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_notifRepublishQueue_i subscription_core_2__p_republishQueue) {
   subscription_core_2__a_notifRepublishQueue_i[subscription_core_2__p_subscription] = subscription_core_2__p_republishQueue;
}

void subscription_core_2__get_subscription_monitoredItemQueue(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   constants__t_monitoredItemQueue_i * const subscription_core_2__p_monitoredItemQueue) {
   *subscription_core_2__p_monitoredItemQueue = subscription_core_2__a_monitoredItemQueue_i[subscription_core_2__p_subscription];
}

void subscription_core_2__set_subscription_monitoredItemQueue(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_monitoredItemQueue_i subscription_core_2__p_monitoredItemQueue) {
   subscription_core_2__a_monitoredItemQueue_i[subscription_core_2__p_subscription] = subscription_core_2__p_monitoredItemQueue;
}

void subscription_core_2__log_subscription_transition(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const constants__t_subscriptionState_i subscription_core_2__p_old_state,
   const constants__t_subscriptionState_i subscription_core_2__p_new_state,
   const t_entier4 subscription_core_2__p_transition) {
   subscription_core_2__local_log_subscription_transition(subscription_core_2__p_subscription,
      subscription_core_2__p_old_state,
      subscription_core_2__p_new_state,
      subscription_core_2__p_transition);
}

void subscription_core_2__log_subscription_request_received(
   const constants__t_subscription_i subscription_core_2__p_subscription,
   const t_entier4 subscription_core_2__p_transition) {
   {
      constants__t_subscriptionState_i subscription_core_2__l_state;
      
      subscription_core_2__l_state = subscription_core_2__a_subscription_state_i[subscription_core_2__p_subscription];
      subscription_core_2__local_log_subscription_transition(subscription_core_2__p_subscription,
         subscription_core_2__l_state,
         subscription_core_2__l_state,
         subscription_core_2__p_transition);
   }
}

void subscription_core_2__log_subscription_request_received_invalid_on_session(
   const constants__t_subscription_i subscription_core_2__p_subscription) {
   subscription_core_bs__trace_subscription_transition(subscription_core_2__p_subscription,
      constants__c_subscriptionState_indet,
      constants__c_subscriptionState_indet,
      26,
      false,
      0,
      0,
      false,
      false);
}

