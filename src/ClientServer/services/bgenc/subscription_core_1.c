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

 Date                 : 27/08/2025 16:32:00

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "subscription_core_1.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 subscription_core_1__a_KeepAliveCounter_i[constants__t_subscription_i_max+1];
t_entier4 subscription_core_1__a_LifetimeCounter_i[constants__t_subscription_i_max+1];
t_bool subscription_core_1__a_MessageSent_i[constants__t_subscription_i_max+1];
t_bool subscription_core_1__a_MoreNotifications_i[constants__t_subscription_i_max+1];
t_bool subscription_core_1__a_PublishingEnabled_i[constants__t_subscription_i_max+1];
constants__t_sub_seq_num_i subscription_core_1__a_SeqNum_i[constants__t_subscription_i_max+1];
t_entier4 subscription_core_1__a_keepAliveExpCount_i[constants__t_subscription_i_max+1];
t_entier4 subscription_core_1__a_lifetimeExpCount_i[constants__t_subscription_i_max+1];
t_entier4 subscription_core_1__a_maxNotifsPerPublish_i[constants__t_subscription_i_max+1];
constants__t_monitoredItemQueue_i subscription_core_1__a_monitoredItemQueue_i[constants__t_subscription_i_max+1];
constants__t_notifRepublishQueue_i subscription_core_1__a_notifRepublishQueue_i[constants__t_subscription_i_max+1];
constants__t_opcua_duration_i subscription_core_1__a_publishInterval_i[constants__t_subscription_i_max+1];
constants__t_publishReqQueue_i subscription_core_1__a_publishRequestQueue_i[constants__t_session_i_max+1];
constants__t_timer_id_i subscription_core_1__a_publishTimer_i[constants__t_subscription_i_max+1];
t_entier4 subscription_core_1__a_session_nb_priorities_i[constants__t_session_i_max+1];
t_entier4 subscription_core_1__a_session_nb_subscriptions_i[constants__t_session_i_max+1];
t_entier4 subscription_core_1__a_session_priority_idx_i[constants__t_session_i_max+1][constants__t_priority_i_max+1];
t_entier4 subscription_core_1__a_session_priority_min_sub_idx_i[constants__t_session_i_max+1][constants__t_prio_idx_i_max+1];
t_entier4 subscription_core_1__a_session_priority_nb_subs_i[constants__t_session_i_max+1][constants__t_prio_idx_i_max+1];
t_entier4 subscription_core_1__a_session_priority_next_sub_idx_i[constants__t_session_i_max+1][constants__t_prio_idx_i_max+1];
t_entier4 subscription_core_1__a_session_seq_priority_i[constants__t_session_i_max+1][constants__t_prio_idx_i_max+1];
constants__t_subscription_i subscription_core_1__a_session_seq_subscription_i[constants__t_session_i_max+1][constants__t_sub_idx_i_max+1];
t_entier4 subscription_core_1__a_session_subscription_idx_i[constants__t_session_i_max+1][constants__t_subscription_i_max+1];
t_entier4 subscription_core_1__a_subscription_priority_i[constants__t_subscription_i_max+1];
constants__t_session_i subscription_core_1__a_subscription_session_i[constants__t_subscription_i_max+1];
constants__t_subscriptionState_i subscription_core_1__a_subscription_state_i[constants__t_subscription_i_max+1];
t_bool subscription_core_1__s_subscription_i[constants__t_subscription_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_core_1__INITIALISATION(void) {
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__s_subscription_i[i] = false;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_subscription_state_i[i] = constants__c_subscriptionState_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_subscription_session_i[i] = constants__c_session_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_publishInterval_i[i] = constants__c_opcua_duration_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_lifetimeExpCount_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_keepAliveExpCount_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_maxNotifsPerPublish_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_subscription_priority_i[i] = constants__c_priority_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_session_nb_priorities_i[i] = 0;
      }
   }
   {
      t_entier4 i, j;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         for (j = constants__t_prio_idx_i_max; 0 <= j; j = j - 1) {
            subscription_core_1__a_session_seq_priority_i[i][j] = constants__c_priority_indet;
         }
      }
   }
   {
      t_entier4 i, j;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         for (j = constants__t_priority_i_max; 0 <= j; j = j - 1) {
            subscription_core_1__a_session_priority_idx_i[i][j] = 0;
         }
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_session_nb_subscriptions_i[i] = 0;
      }
   }
   {
      t_entier4 i, j;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         for (j = constants__t_sub_idx_i_max; 0 <= j; j = j - 1) {
            subscription_core_1__a_session_seq_subscription_i[i][j] = constants__c_subscription_indet;
         }
      }
   }
   {
      t_entier4 i, j;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         for (j = constants__t_subscription_i_max; 0 <= j; j = j - 1) {
            subscription_core_1__a_session_subscription_idx_i[i][j] = 0;
         }
      }
   }
   {
      t_entier4 i, j;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         for (j = constants__t_prio_idx_i_max; 0 <= j; j = j - 1) {
            subscription_core_1__a_session_priority_nb_subs_i[i][j] = 0;
         }
      }
   }
   {
      t_entier4 i, j;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         for (j = constants__t_prio_idx_i_max; 0 <= j; j = j - 1) {
            subscription_core_1__a_session_priority_min_sub_idx_i[i][j] = 0;
         }
      }
   }
   {
      t_entier4 i, j;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         for (j = constants__t_prio_idx_i_max; 0 <= j; j = j - 1) {
            subscription_core_1__a_session_priority_next_sub_idx_i[i][j] = 0;
         }
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_PublishingEnabled_i[i] = false;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_MoreNotifications_i[i] = false;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_LifetimeCounter_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_KeepAliveCounter_i[i] = 0;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_MessageSent_i[i] = false;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_SeqNum_i[i] = constants__c_sub_seq_num_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_publishRequestQueue_i[i] = constants__c_publishReqQueue_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_notifRepublishQueue_i[i] = constants__c_notifRepublishQueue_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_monitoredItemQueue_i[i] = constants__c_monitoredItemQueue_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_publishTimer_i[i] = constants__c_timer_id_indet;
      }
   }
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_core_1__local_is_valid_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_bool * const subscription_core_1__is_valid) {
   {
      t_entier4 subscription_core_1__l_subscription_card;
      t_entier4 subscription_core_1__l_subscription_int;
      
      *subscription_core_1__is_valid = false;
      constants__get_card_t_subscription(&subscription_core_1__l_subscription_card);
      constants__get_reverse_cast_t_subscription(subscription_core_1__p_subscription,
         &subscription_core_1__l_subscription_int);
      if ((subscription_core_1__l_subscription_int != 0) &&
         (subscription_core_1__l_subscription_int <= subscription_core_1__l_subscription_card)) {
         *subscription_core_1__is_valid = subscription_core_1__s_subscription_i[subscription_core_1__p_subscription];
      }
   }
}

void subscription_core_1__is_valid_subscription_on_session(
   const constants__t_session_i subscription_core_1__p_session,
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_bool * const subscription_core_1__is_valid) {
   {
      t_bool subscription_core_1__l_is_valid_sub;
      constants__t_session_i subscription_core_1__l_session;
      
      subscription_core_1__l_session = constants__c_session_indet;
      subscription_core_1__local_is_valid_subscription(subscription_core_1__p_subscription,
         &subscription_core_1__l_is_valid_sub);
      if (subscription_core_1__l_is_valid_sub == true) {
         subscription_core_1__l_session = subscription_core_1__a_subscription_session_i[subscription_core_1__p_subscription];
      }
      *subscription_core_1__is_valid = ((subscription_core_1__l_is_valid_sub == true) &&
         (subscription_core_1__l_session == subscription_core_1__p_session));
   }
}

void subscription_core_1__is_valid_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_bool * const subscription_core_1__is_valid) {
   subscription_core_1__local_is_valid_subscription(subscription_core_1__p_subscription,
      subscription_core_1__is_valid);
}

void subscription_core_1__getall_session(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_session_i * const subscription_core_1__p_session) {
   *subscription_core_1__p_session = subscription_core_1__a_subscription_session_i[subscription_core_1__p_subscription];
}

void subscription_core_1__get_card_session_seq_priority(
   const constants__t_session_i subscription_core_1__p_session,
   t_entier4 * const subscription_core_1__p_priority_idx) {
   *subscription_core_1__p_priority_idx = subscription_core_1__a_session_nb_priorities_i[subscription_core_1__p_session];
}

void subscription_core_1__get_card_session_seq_subscription(
   const constants__t_session_i subscription_core_1__p_session,
   t_entier4 * const subscription_core_1__p_sub_idx) {
   *subscription_core_1__p_sub_idx = subscription_core_1__a_session_nb_subscriptions_i[subscription_core_1__p_session];
}

void subscription_core_1__get_session_seq_subscription(
   const constants__t_session_i subscription_core_1__p_session,
   const t_entier4 subscription_core_1__p_idx_sub,
   constants__t_subscription_i * const subscription_core_1__p_subscription) {
   *subscription_core_1__p_subscription = subscription_core_1__a_session_seq_subscription_i[subscription_core_1__p_session][subscription_core_1__p_idx_sub];
}

void subscription_core_1__get_session_priority_nb_subs(
   const constants__t_session_i subscription_core_1__p_session,
   const t_entier4 subscription_core_1__p_prio_idx,
   t_entier4 * const subscription_core_1__p_nb_subs) {
   *subscription_core_1__p_nb_subs = subscription_core_1__a_session_priority_nb_subs_i[subscription_core_1__p_session][subscription_core_1__p_prio_idx];
}

void subscription_core_1__get_session_priority_min_sub_idx(
   const constants__t_session_i subscription_core_1__p_session,
   const t_entier4 subscription_core_1__p_prio_idx,
   t_entier4 * const subscription_core_1__p_min_idx_sub) {
   *subscription_core_1__p_min_idx_sub = subscription_core_1__a_session_priority_min_sub_idx_i[subscription_core_1__p_session][subscription_core_1__p_prio_idx];
}

void subscription_core_1__get_session_priority_next_sub_idx(
   const constants__t_session_i subscription_core_1__p_session,
   const t_entier4 subscription_core_1__p_prio_idx,
   t_entier4 * const subscription_core_1__p_next_idx_sub) {
   *subscription_core_1__p_next_idx_sub = subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__p_session][subscription_core_1__p_prio_idx];
}

void subscription_core_1__set_session_priority_next_sub_idx(
   const constants__t_session_i subscription_core_1__p_session,
   const t_entier4 subscription_core_1__p_prio_idx,
   const t_entier4 subscription_core_1__p_next_idx_sub) {
   subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__p_session][subscription_core_1__p_prio_idx] = subscription_core_1__p_next_idx_sub;
}

void subscription_core_1__add_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const constants__t_session_i subscription_core_1__p_session,
   const constants__t_subscriptionState_i subscription_core_1__p_state,
   const t_bool subscription_core_1__p_firstMsgSent,
   const t_entier4 subscription_core_1__p_priority,
   const constants__t_opcua_duration_i subscription_core_1__p_revPublishInterval,
   const t_entier4 subscription_core_1__p_revLifetimeCount,
   const t_entier4 subscription_core_1__p_revMaxKeepAlive,
   const t_entier4 subscription_core_1__p_maxNotificationsPerPublish,
   const t_bool subscription_core_1__p_publishEnabled,
   const constants__t_sub_seq_num_i subscription_core_1__p_seqNumInit,
   const constants__t_notifRepublishQueue_i subscription_core_1__p_republishQueue,
   const constants__t_monitoredItemQueue_i subscription_core_1__p_monitoredItemQueue,
   const constants__t_timer_id_i subscription_core_1__p_timerId,
   constants_statuscodes_bs__t_StatusCode_i * const subscription_core_1__StatusCode_service) {
   {
      t_entier4 subscription_core_1__l_prio_idx;
      t_entier4 subscription_core_1__l_nb_prio;
      t_entier4 subscription_core_1__l_idx;
      t_entier4 subscription_core_1__l_int_continue;
      t_entier4 subscription_core_1__l_sub_idx_it;
      t_entier4 subscription_core_1__l_prio;
      t_entier4 subscription_core_1__l_sub_idx_max;
      t_bool subscription_core_1__l_reset_next_sub_idx;
      t_entier4 subscription_core_1__l_sub_idx;
      constants__t_subscription_i subscription_core_1__l_sub;
      
      subscription_core_1__l_reset_next_sub_idx = false;
      subscription_core_1__l_sub_idx_max = subscription_core_1__a_session_nb_subscriptions_i[subscription_core_1__p_session] +
         1;
      if (subscription_core_1__l_sub_idx_max > constants__k_n_subscriptionPerSession_max) {
         *subscription_core_1__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_subscriptions;
      }
      else {
         *subscription_core_1__StatusCode_service = constants_statuscodes_bs__e_sc_ok;
         subscription_core_1__s_subscription_i[subscription_core_1__p_subscription] = true;
         subscription_core_1__a_subscription_state_i[subscription_core_1__p_subscription] = subscription_core_1__p_state;
         subscription_core_1__a_subscription_session_i[subscription_core_1__p_subscription] = subscription_core_1__p_session;
         subscription_core_1__a_subscription_priority_i[subscription_core_1__p_subscription] = subscription_core_1__p_priority;
         subscription_core_1__a_publishInterval_i[subscription_core_1__p_subscription] = subscription_core_1__p_revPublishInterval;
         subscription_core_1__a_lifetimeExpCount_i[subscription_core_1__p_subscription] = subscription_core_1__p_revLifetimeCount;
         subscription_core_1__a_LifetimeCounter_i[subscription_core_1__p_subscription] = subscription_core_1__p_revLifetimeCount;
         subscription_core_1__a_keepAliveExpCount_i[subscription_core_1__p_subscription] = subscription_core_1__p_revMaxKeepAlive;
         subscription_core_1__a_KeepAliveCounter_i[subscription_core_1__p_subscription] = subscription_core_1__p_revMaxKeepAlive;
         subscription_core_1__a_maxNotifsPerPublish_i[subscription_core_1__p_subscription] = subscription_core_1__p_maxNotificationsPerPublish;
         subscription_core_1__a_MessageSent_i[subscription_core_1__p_subscription] = subscription_core_1__p_firstMsgSent;
         subscription_core_1__a_PublishingEnabled_i[subscription_core_1__p_subscription] = subscription_core_1__p_publishEnabled;
         subscription_core_1__a_SeqNum_i[subscription_core_1__p_subscription] = subscription_core_1__p_seqNumInit;
         subscription_core_1__a_notifRepublishQueue_i[subscription_core_1__p_subscription] = subscription_core_1__p_republishQueue;
         subscription_core_1__a_monitoredItemQueue_i[subscription_core_1__p_subscription] = subscription_core_1__p_monitoredItemQueue;
         subscription_core_1__a_publishTimer_i[subscription_core_1__p_subscription] = subscription_core_1__p_timerId;
         subscription_core_1__l_prio_idx = subscription_core_1__a_session_priority_idx_i[subscription_core_1__p_session][subscription_core_1__p_priority];
         if (subscription_core_1__l_prio_idx == 0) {
            subscription_core_1__l_nb_prio = subscription_core_1__a_session_nb_priorities_i[subscription_core_1__p_session];
            subscription_core_1__l_prio_idx = subscription_core_1__l_nb_prio +
               1;
            subscription_core_1__l_int_continue = 1;
            while ((0 < subscription_core_1__l_prio_idx) &&
               (subscription_core_1__l_int_continue == 1)) {
               subscription_core_1__l_prio = subscription_core_1__a_session_seq_priority_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx -
                  1];
               if ((subscription_core_1__l_prio != constants__c_priority_indet) &&
                  (subscription_core_1__p_priority < subscription_core_1__l_prio)) {
                  subscription_core_1__a_session_seq_priority_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx] = subscription_core_1__l_prio;
                  subscription_core_1__a_session_priority_idx_i[subscription_core_1__p_session][subscription_core_1__l_prio] = subscription_core_1__l_prio_idx;
                  subscription_core_1__a_session_priority_nb_subs_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx] = subscription_core_1__a_session_priority_nb_subs_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx -
                     1];
                  subscription_core_1__l_prio_idx = subscription_core_1__l_prio_idx -
                     1;
               }
               else {
                  subscription_core_1__a_session_seq_priority_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx] = subscription_core_1__p_priority;
                  subscription_core_1__a_session_priority_idx_i[subscription_core_1__p_session][subscription_core_1__p_priority] = subscription_core_1__l_prio_idx;
                  subscription_core_1__a_session_priority_nb_subs_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx] = 0;
                  if (subscription_core_1__l_prio_idx > subscription_core_1__l_nb_prio) {
                     subscription_core_1__a_session_priority_min_sub_idx_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx] = subscription_core_1__l_sub_idx_max;
                     subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx] = subscription_core_1__l_sub_idx_max;
                  }
                  else {
                     subscription_core_1__l_reset_next_sub_idx = true;
                  }
                  subscription_core_1__l_int_continue = 0;
               }
            }
            subscription_core_1__a_session_nb_priorities_i[subscription_core_1__p_session] = subscription_core_1__l_nb_prio +
               1;
         }
         subscription_core_1__l_idx = subscription_core_1__a_session_nb_priorities_i[subscription_core_1__p_session];
         subscription_core_1__l_sub_idx = subscription_core_1__l_sub_idx_max;
         while (subscription_core_1__l_prio_idx < subscription_core_1__l_idx) {
            subscription_core_1__l_prio = subscription_core_1__a_session_seq_priority_i[subscription_core_1__p_session][subscription_core_1__l_idx];
            subscription_core_1__l_sub_idx = subscription_core_1__a_session_priority_min_sub_idx_i[subscription_core_1__p_session][subscription_core_1__l_idx -
               1];
            subscription_core_1__a_session_priority_min_sub_idx_i[subscription_core_1__p_session][subscription_core_1__l_idx] = subscription_core_1__l_sub_idx +
               1;
            subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__p_session][subscription_core_1__l_idx] = subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__p_session][subscription_core_1__l_idx -
               1] +
               1;
            subscription_core_1__l_idx = subscription_core_1__l_idx -
               1;
         }
         subscription_core_1__a_session_priority_nb_subs_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx] = subscription_core_1__a_session_priority_nb_subs_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx] +
            1;
         if (subscription_core_1__l_reset_next_sub_idx == true) {
            subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx] = subscription_core_1__a_session_priority_min_sub_idx_i[subscription_core_1__p_session][subscription_core_1__l_prio_idx];
         }
         subscription_core_1__l_sub_idx_it = subscription_core_1__l_sub_idx_max;
         while (subscription_core_1__l_sub_idx < subscription_core_1__l_sub_idx_it) {
            subscription_core_1__l_sub = subscription_core_1__a_session_seq_subscription_i[subscription_core_1__p_session][subscription_core_1__l_sub_idx_it -
               1];
            subscription_core_1__a_session_seq_subscription_i[subscription_core_1__p_session][subscription_core_1__l_sub_idx_it] = subscription_core_1__l_sub;
            subscription_core_1__a_session_subscription_idx_i[subscription_core_1__p_session][subscription_core_1__l_sub] = subscription_core_1__l_sub_idx_it;
            subscription_core_1__l_sub_idx_it = subscription_core_1__l_sub_idx_it -
               1;
         }
         subscription_core_1__a_session_nb_subscriptions_i[subscription_core_1__p_session] = subscription_core_1__l_sub_idx_max;
         subscription_core_1__a_session_seq_subscription_i[subscription_core_1__p_session][subscription_core_1__l_sub_idx] = subscription_core_1__p_subscription;
         subscription_core_1__a_session_subscription_idx_i[subscription_core_1__p_session][subscription_core_1__p_subscription] = subscription_core_1__l_sub_idx;
      }
   }
}

void subscription_core_1__delete_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription) {
   {
      t_entier4 subscription_core_1__l_prio;
      constants__t_session_i subscription_core_1__l_session;
      t_entier4 subscription_core_1__l_priority;
      t_entier4 subscription_core_1__l_nb_subs;
      t_entier4 subscription_core_1__l_sub_prio_idx;
      t_entier4 subscription_core_1__l_prio_idx_it;
      t_entier4 subscription_core_1__l_nb_prio;
      t_entier4 subscription_core_1__l_nb_sub_prio;
      t_entier4 subscription_core_1__l_sub_idx_next;
      t_entier4 subscription_core_1__l_sub_idx_del;
      t_entier4 subscription_core_1__l_sub_idx;
      constants__t_subscription_i subscription_core_1__l_sub;
      
      subscription_core_1__s_subscription_i[subscription_core_1__p_subscription] = false;
      subscription_core_1__a_subscription_state_i[subscription_core_1__p_subscription] = constants__c_subscriptionState_indet;
      subscription_core_1__l_session = subscription_core_1__a_subscription_session_i[subscription_core_1__p_subscription];
      subscription_core_1__a_subscription_session_i[subscription_core_1__p_subscription] = constants__c_session_indet;
      subscription_core_1__a_publishInterval_i[subscription_core_1__p_subscription] = constants__c_opcua_duration_indet;
      subscription_core_1__a_lifetimeExpCount_i[subscription_core_1__p_subscription] = 0;
      subscription_core_1__a_keepAliveExpCount_i[subscription_core_1__p_subscription] = 0;
      subscription_core_1__a_maxNotifsPerPublish_i[subscription_core_1__p_subscription] = 0;
      subscription_core_1__a_PublishingEnabled_i[subscription_core_1__p_subscription] = false;
      subscription_core_1__a_MoreNotifications_i[subscription_core_1__p_subscription] = false;
      subscription_core_1__a_LifetimeCounter_i[subscription_core_1__p_subscription] = 0;
      subscription_core_1__a_KeepAliveCounter_i[subscription_core_1__p_subscription] = 0;
      subscription_core_1__a_MessageSent_i[subscription_core_1__p_subscription] = false;
      subscription_core_1__a_SeqNum_i[subscription_core_1__p_subscription] = constants__c_sub_seq_num_indet;
      subscription_core_1__a_notifRepublishQueue_i[subscription_core_1__p_subscription] = constants__c_notifRepublishQueue_indet;
      subscription_core_1__a_monitoredItemQueue_i[subscription_core_1__p_subscription] = constants__c_monitoredItemQueue_indet;
      subscription_core_1__a_publishTimer_i[subscription_core_1__p_subscription] = constants__c_timer_id_indet;
      subscription_core_1__l_priority = subscription_core_1__a_subscription_priority_i[subscription_core_1__p_subscription];
      subscription_core_1__l_nb_subs = subscription_core_1__a_session_nb_subscriptions_i[subscription_core_1__l_session];
      subscription_core_1__l_sub_prio_idx = subscription_core_1__a_session_priority_idx_i[subscription_core_1__l_session][subscription_core_1__l_priority];
      subscription_core_1__l_nb_prio = subscription_core_1__a_session_nb_priorities_i[subscription_core_1__l_session];
      subscription_core_1__l_nb_sub_prio = subscription_core_1__a_session_priority_nb_subs_i[subscription_core_1__l_session][subscription_core_1__l_sub_prio_idx];
      subscription_core_1__l_sub_idx_next = subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_sub_prio_idx];
      subscription_core_1__l_sub_idx_del = subscription_core_1__a_session_subscription_idx_i[subscription_core_1__l_session][subscription_core_1__p_subscription];
      subscription_core_1__l_prio_idx_it = subscription_core_1__l_sub_prio_idx;
      while (subscription_core_1__l_prio_idx_it < subscription_core_1__l_nb_prio) {
         subscription_core_1__l_prio = subscription_core_1__a_session_seq_priority_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it +
            1];
         subscription_core_1__a_session_priority_min_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it +
            1] = subscription_core_1__a_session_priority_min_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it +
            1] -
            1;
         subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it +
            1] = subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it +
            1] -
            1;
         if (subscription_core_1__l_nb_sub_prio == 1) {
            subscription_core_1__a_session_seq_priority_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it] = subscription_core_1__l_prio;
            subscription_core_1__a_session_priority_idx_i[subscription_core_1__l_session][subscription_core_1__l_prio] = subscription_core_1__l_prio_idx_it;
            subscription_core_1__a_session_priority_min_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it] = subscription_core_1__a_session_priority_min_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it +
               1];
            subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it] = subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it +
               1];
            subscription_core_1__a_session_priority_nb_subs_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it] = subscription_core_1__a_session_priority_nb_subs_i[subscription_core_1__l_session][subscription_core_1__l_prio_idx_it +
               1];
         }
         subscription_core_1__l_prio_idx_it = subscription_core_1__l_prio_idx_it +
            1;
      }
      if (subscription_core_1__l_nb_sub_prio == 1) {
         subscription_core_1__a_session_priority_idx_i[subscription_core_1__l_session][subscription_core_1__l_priority] = 0;
         subscription_core_1__a_session_priority_min_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_nb_prio] = 0;
         subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_nb_prio] = 0;
         subscription_core_1__a_session_priority_nb_subs_i[subscription_core_1__l_session][subscription_core_1__l_nb_prio] = 0;
         subscription_core_1__a_session_seq_priority_i[subscription_core_1__l_session][subscription_core_1__l_nb_prio] = constants__c_priority_indet;
         subscription_core_1__a_session_nb_priorities_i[subscription_core_1__l_session] = subscription_core_1__l_nb_prio -
            1;
      }
      else {
         subscription_core_1__a_session_priority_nb_subs_i[subscription_core_1__l_session][subscription_core_1__l_sub_prio_idx] = subscription_core_1__l_nb_sub_prio -
            1;
         if ((subscription_core_1__l_sub_idx_del < subscription_core_1__l_sub_idx_next) ||
            (subscription_core_1__l_sub_idx_next == subscription_core_1__l_nb_sub_prio)) {
            subscription_core_1__a_session_priority_next_sub_idx_i[subscription_core_1__l_session][subscription_core_1__l_sub_prio_idx] = subscription_core_1__l_sub_idx_next -
               1;
         }
      }
      subscription_core_1__l_sub_idx = subscription_core_1__l_sub_idx_del;
      while (subscription_core_1__l_sub_idx < subscription_core_1__l_nb_subs) {
         subscription_core_1__l_sub = subscription_core_1__a_session_seq_subscription_i[subscription_core_1__l_session][subscription_core_1__l_sub_idx +
            1];
         subscription_core_1__a_session_seq_subscription_i[subscription_core_1__l_session][subscription_core_1__l_sub_idx] = subscription_core_1__l_sub;
         subscription_core_1__a_session_subscription_idx_i[subscription_core_1__l_session][subscription_core_1__l_sub] = subscription_core_1__l_sub_idx;
         subscription_core_1__l_sub_idx = subscription_core_1__l_sub_idx +
            1;
      }
      subscription_core_1__a_session_subscription_idx_i[subscription_core_1__l_session][subscription_core_1__p_subscription] = 0;
      subscription_core_1__a_session_seq_subscription_i[subscription_core_1__l_session][subscription_core_1__l_nb_subs] = constants__c_subscription_indet;
      subscription_core_1__a_session_nb_subscriptions_i[subscription_core_1__l_session] = subscription_core_1__l_nb_subs -
         1;
   }
}

void subscription_core_1__get_subscription_priority(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_entier4 * const subscription_core_1__p_priority) {
   *subscription_core_1__p_priority = subscription_core_1__a_subscription_priority_i[subscription_core_1__p_subscription];
}

void subscription_core_1__get_subscription_publishInterval(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_opcua_duration_i * const subscription_core_1__p_publishInterval) {
   *subscription_core_1__p_publishInterval = subscription_core_1__a_publishInterval_i[subscription_core_1__p_subscription];
}

void subscription_core_1__set_subscription_publishInterval(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const constants__t_opcua_duration_i subscription_core_1__p_revPublishInterval) {
   subscription_core_1__a_publishInterval_i[subscription_core_1__p_subscription] = subscription_core_1__p_revPublishInterval;
}

void subscription_core_1__set_subscription_timer_id(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const constants__t_timer_id_i subscription_core_1__p_timer_id) {
   subscription_core_1__a_publishTimer_i[subscription_core_1__p_subscription] = subscription_core_1__p_timer_id;
}

void subscription_core_1__get_subscription_timer_id(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_timer_id_i * const subscription_core_1__p_timer_id) {
   *subscription_core_1__p_timer_id = subscription_core_1__a_publishTimer_i[subscription_core_1__p_subscription];
}

void subscription_core_1__set_subscription_state(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const constants__t_subscriptionState_i subscription_core_1__p_state) {
   subscription_core_1__a_subscription_state_i[subscription_core_1__p_subscription] = subscription_core_1__p_state;
}

void subscription_core_1__get_subscription_state(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_subscriptionState_i * const subscription_core_1__p_state) {
   *subscription_core_1__p_state = subscription_core_1__a_subscription_state_i[subscription_core_1__p_subscription];
}

void subscription_core_1__set_subscription_MoreNotifications(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const t_bool subscription_core_1__p_moreNotifs) {
   subscription_core_1__a_MoreNotifications_i[subscription_core_1__p_subscription] = subscription_core_1__p_moreNotifs;
}

void subscription_core_1__get_subscription_MoreNotifications(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_bool * const subscription_core_1__p_moreNotifs) {
   *subscription_core_1__p_moreNotifs = subscription_core_1__a_MoreNotifications_i[subscription_core_1__p_subscription];
}

void subscription_core_1__set_subscription_MaxLifetimeAndKeepAliveCount(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const t_entier4 subscription_core_1__p_revLifetimeCount,
   const t_entier4 subscription_core_1__p_revMaxKeepAlive) {
   subscription_core_1__a_lifetimeExpCount_i[subscription_core_1__p_subscription] = subscription_core_1__p_revLifetimeCount;
   subscription_core_1__a_keepAliveExpCount_i[subscription_core_1__p_subscription] = subscription_core_1__p_revMaxKeepAlive;
}

void subscription_core_1__decrement_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription) {
   subscription_core_1__a_LifetimeCounter_i[subscription_core_1__p_subscription] = subscription_core_1__a_LifetimeCounter_i[subscription_core_1__p_subscription] -
      1;
}

void subscription_core_1__reset_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription) {
   subscription_core_1__a_LifetimeCounter_i[subscription_core_1__p_subscription] = subscription_core_1__a_lifetimeExpCount_i[subscription_core_1__p_subscription];
}

void subscription_core_1__get_subscription_LifetimeCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_entier4 * const subscription_core_1__p_lifetimeCounter) {
   *subscription_core_1__p_lifetimeCounter = subscription_core_1__a_LifetimeCounter_i[subscription_core_1__p_subscription];
}

void subscription_core_1__decrement_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription) {
   subscription_core_1__a_KeepAliveCounter_i[subscription_core_1__p_subscription] = subscription_core_1__a_KeepAliveCounter_i[subscription_core_1__p_subscription] -
      1;
}

void subscription_core_1__reset_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription) {
   subscription_core_1__a_KeepAliveCounter_i[subscription_core_1__p_subscription] = subscription_core_1__a_keepAliveExpCount_i[subscription_core_1__p_subscription];
}

void subscription_core_1__get_subscription_KeepAliveCounter(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_entier4 * const subscription_core_1__p_keepAliveCounter) {
   *subscription_core_1__p_keepAliveCounter = subscription_core_1__a_KeepAliveCounter_i[subscription_core_1__p_subscription];
}

void subscription_core_1__set_subscription_MaxNotifsPerPublish(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const t_entier4 subscription_core_1__p_maxNotificationsPerPublish) {
   subscription_core_1__a_maxNotifsPerPublish_i[subscription_core_1__p_subscription] = subscription_core_1__p_maxNotificationsPerPublish;
}

void subscription_core_1__get_subscription_MaxNotifsPerPublish(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_entier4 * const subscription_core_1__p_maxNotificationsPerPublish) {
   *subscription_core_1__p_maxNotificationsPerPublish = subscription_core_1__a_maxNotifsPerPublish_i[subscription_core_1__p_subscription];
}

void subscription_core_1__set_subscription_MessageSent(
   const constants__t_subscription_i subscription_core_1__p_subscription) {
   subscription_core_1__a_MessageSent_i[subscription_core_1__p_subscription] = true;
}

void subscription_core_1__get_subscription_MessageSent(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_bool * const subscription_core_1__p_firstMsgSent) {
   *subscription_core_1__p_firstMsgSent = subscription_core_1__a_MessageSent_i[subscription_core_1__p_subscription];
}

void subscription_core_1__set_subscription_PublishingEnabled(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const t_bool subscription_core_1__p_pubEnabled) {
   subscription_core_1__a_PublishingEnabled_i[subscription_core_1__p_subscription] = subscription_core_1__p_pubEnabled;
   subscription_core_1__a_MoreNotifications_i[subscription_core_1__p_subscription] = false;
   subscription_core_1__a_LifetimeCounter_i[subscription_core_1__p_subscription] = subscription_core_1__a_lifetimeExpCount_i[subscription_core_1__p_subscription];
}

void subscription_core_1__get_subscription_PublishingEnabled(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_bool * const subscription_core_1__p_pubEnabled) {
   *subscription_core_1__p_pubEnabled = subscription_core_1__a_PublishingEnabled_i[subscription_core_1__p_subscription];
}

void subscription_core_1__set_subscription_SeqNum(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   const constants__t_sub_seq_num_i subscription_core_1__p_nextSeqNum) {
   subscription_core_1__a_SeqNum_i[subscription_core_1__p_subscription] = subscription_core_1__p_nextSeqNum;
}

void subscription_core_1__get_subscription_SeqNum(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_sub_seq_num_i * const subscription_core_1__p_seqNumToSend) {
   *subscription_core_1__p_seqNumToSend = subscription_core_1__a_SeqNum_i[subscription_core_1__p_subscription];
}

void subscription_core_1__reset_session_publishRequestQueue(
   const constants__t_session_i subscription_core_1__p_session) {
   subscription_core_1__a_publishRequestQueue_i[subscription_core_1__p_session] = constants__c_publishReqQueue_indet;
}

void subscription_core_1__set_session_publishRequestQueue(
   const constants__t_session_i subscription_core_1__p_session,
   const constants__t_publishReqQueue_i subscription_core_1__p_publishReqQueue) {
   subscription_core_1__a_publishRequestQueue_i[subscription_core_1__p_session] = subscription_core_1__p_publishReqQueue;
}

void subscription_core_1__get_session_publishRequestQueue(
   const constants__t_session_i subscription_core_1__p_session,
   constants__t_publishReqQueue_i * const subscription_core_1__p_publishReqQueue) {
   *subscription_core_1__p_publishReqQueue = subscription_core_1__a_publishRequestQueue_i[subscription_core_1__p_session];
}

void subscription_core_1__get_subscription_notifRepublishQueue(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_notifRepublishQueue_i * const subscription_core_1__p_republishQueue) {
   *subscription_core_1__p_republishQueue = subscription_core_1__a_notifRepublishQueue_i[subscription_core_1__p_subscription];
}

void subscription_core_1__get_subscription_monitoredItemQueue(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_monitoredItemQueue_i * const subscription_core_1__p_monitoredItemQueue) {
   *subscription_core_1__p_monitoredItemQueue = subscription_core_1__a_monitoredItemQueue_i[subscription_core_1__p_subscription];
}

