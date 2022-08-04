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

 Date                 : 04/08/2022 14:53:20

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
constants__t_notificationQueue_i subscription_core_1__a_pendingNotificationQueue_i[constants__t_subscription_i_max+1];
constants__t_opcua_duration_i subscription_core_1__a_publishInterval_i[constants__t_subscription_i_max+1];
constants__t_publishReqQueue_i subscription_core_1__a_publishRequestQueue_i[constants__t_subscription_i_max+1];
constants__t_timer_id_i subscription_core_1__a_publishTimer_i[constants__t_subscription_i_max+1];
constants__t_subscription_i subscription_core_1__a_session_subscription_i[constants__t_session_i_max+1];
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
      for (i = constants__t_session_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_session_subscription_i[i] = constants__c_subscription_indet;
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
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
         subscription_core_1__a_pendingNotificationQueue_i[i] = constants__c_notificationQueue_indet;
      }
   }
   {
      t_entier4 i;
      for (i = constants__t_subscription_i_max; 0 <= i; i = i - 1) {
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
void subscription_core_1__is_valid_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   t_bool * const subscription_core_1__is_valid) {
   *subscription_core_1__is_valid = subscription_core_1__s_subscription_i[subscription_core_1__p_subscription];
}

void subscription_core_1__getall_subscription(
   const constants__t_session_i subscription_core_1__p_session,
   t_bool * const subscription_core_1__p_dom,
   constants__t_subscription_i * const subscription_core_1__p_subscription) {
   *subscription_core_1__p_subscription = subscription_core_1__a_session_subscription_i[subscription_core_1__p_session];
   *subscription_core_1__p_dom = subscription_core_1__s_subscription_i[*subscription_core_1__p_subscription];
}

void subscription_core_1__getall_session(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_session_i * const subscription_core_1__p_session) {
   *subscription_core_1__p_session = subscription_core_1__a_subscription_session_i[subscription_core_1__p_subscription];
}

void subscription_core_1__add_subscription(
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
   const constants__t_timer_id_i subscription_core_1__p_timerId) {
   subscription_core_1__s_subscription_i[subscription_core_1__p_subscription] = true;
   subscription_core_1__a_subscription_state_i[subscription_core_1__p_subscription] = constants__e_subscriptionState_normal;
   subscription_core_1__a_subscription_session_i[subscription_core_1__p_subscription] = subscription_core_1__p_session;
   subscription_core_1__a_session_subscription_i[subscription_core_1__p_session] = subscription_core_1__p_subscription;
   subscription_core_1__a_publishInterval_i[subscription_core_1__p_subscription] = subscription_core_1__p_revPublishInterval;
   subscription_core_1__a_lifetimeExpCount_i[subscription_core_1__p_subscription] = subscription_core_1__p_revLifetimeCount;
   subscription_core_1__a_LifetimeCounter_i[subscription_core_1__p_subscription] = subscription_core_1__p_revLifetimeCount;
   subscription_core_1__a_keepAliveExpCount_i[subscription_core_1__p_subscription] = subscription_core_1__p_revMaxKeepAlive;
   subscription_core_1__a_KeepAliveCounter_i[subscription_core_1__p_subscription] = subscription_core_1__p_revMaxKeepAlive;
   subscription_core_1__a_maxNotifsPerPublish_i[subscription_core_1__p_subscription] = subscription_core_1__p_maxNotificationsPerPublish;
   subscription_core_1__a_MessageSent_i[subscription_core_1__p_subscription] = false;
   subscription_core_1__a_PublishingEnabled_i[subscription_core_1__p_subscription] = subscription_core_1__p_publishEnabled;
   subscription_core_1__a_SeqNum_i[subscription_core_1__p_subscription] = constants__c_sub_seq_num_init;
   subscription_core_1__a_pendingNotificationQueue_i[subscription_core_1__p_subscription] = subscription_core_1__p_notifQueue;
   subscription_core_1__a_publishRequestQueue_i[subscription_core_1__p_subscription] = subscription_core_1__p_publishQueue;
   subscription_core_1__a_notifRepublishQueue_i[subscription_core_1__p_subscription] = subscription_core_1__p_republishQueue;
   subscription_core_1__a_monitoredItemQueue_i[subscription_core_1__p_subscription] = subscription_core_1__p_monitoredItemQueue;
   subscription_core_1__a_publishTimer_i[subscription_core_1__p_subscription] = subscription_core_1__p_timerId;
   subscription_core_1__a_SeqNum_i[subscription_core_1__p_subscription] = constants__c_sub_seq_num_init;
}

void subscription_core_1__delete_subscription(
   const constants__t_subscription_i subscription_core_1__p_subscription) {
   subscription_core_1__s_subscription_i[subscription_core_1__p_subscription] = false;
   subscription_core_1__a_subscription_state_i[subscription_core_1__p_subscription] = constants__c_subscriptionState_indet;
   subscription_core_1__a_session_subscription_i[subscription_core_1__a_subscription_session_i[subscription_core_1__p_subscription]] = constants__c_subscription_indet;
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
   subscription_core_1__a_pendingNotificationQueue_i[subscription_core_1__p_subscription] = constants__c_notificationQueue_indet;
   subscription_core_1__a_publishRequestQueue_i[subscription_core_1__p_subscription] = constants__c_publishReqQueue_indet;
   subscription_core_1__a_notifRepublishQueue_i[subscription_core_1__p_subscription] = constants__c_notifRepublishQueue_indet;
   subscription_core_1__a_monitoredItemQueue_i[subscription_core_1__p_subscription] = constants__c_monitoredItemQueue_indet;
   subscription_core_1__a_publishTimer_i[subscription_core_1__p_subscription] = constants__c_timer_id_indet;
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

void subscription_core_1__get_subscription_notificationQueue(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_notificationQueue_i * const subscription_core_1__p_notificationQueue) {
   *subscription_core_1__p_notificationQueue = subscription_core_1__a_pendingNotificationQueue_i[subscription_core_1__p_subscription];
}

void subscription_core_1__get_subscription_publishRequestQueue(
   const constants__t_subscription_i subscription_core_1__p_subscription,
   constants__t_publishReqQueue_i * const subscription_core_1__p_publishReqQueue) {
   *subscription_core_1__p_publishReqQueue = subscription_core_1__a_publishRequestQueue_i[subscription_core_1__p_subscription];
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

