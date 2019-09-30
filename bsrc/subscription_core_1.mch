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

MACHINE
    subscription_core_1

SEES
    constants,
    message_out_bs,
    request_handle_bs

DEFINITIONS
    d_variables ==
    s_subscription,
    a_subscription_session,
    a_publishInterval,
    a_lifetimeExpCount,
    a_keepAliveExpCount,
    a_maxNotifsPerPublish,
    a_subscription_state,
    a_MoreNotifications,
    a_LifetimeCounter,
    a_KeepAliveCounter,
    a_MessageSent,
    a_PublishingEnabled,
    a_SeqNum,

    a_pendingNotificationQueue,
    a_publishRequestQueue,
    a_notifRepublishQueue,
    a_monitoredItemQueue,
    a_publishTimer;

    d_typing ==
    s_subscription <: t_subscription &

    /* Subscription configuration record */
    a_subscription_session : s_subscription >-> t_session &     // a subscription shall be associated to a session (=> 1 subscription / 1 session)
    a_publishInterval : s_subscription --> t_opcua_duration_i & // a subscription shall have a publishing interval value
    a_lifetimeExpCount : s_subscription --> t_counter &       // a subscription shall have a lifetime expiration count value
    a_keepAliveExpCount : s_subscription --> t_counter &      // a subscription shall have a keepAlive expiration count value
    a_maxNotifsPerPublish : s_subscription --> t_counter &    // a subscription shall have a maximum notifications per publish response

    /* Subscription State Machine : state shall be defined (part 4 table 82) */
    a_subscription_state : s_subscription --> t_subscriptionState &

    /* Subscription State Machine : variables shall be defined (part 4 table 84) */
    a_MoreNotifications <: s_subscription &
    a_LifetimeCounter : s_subscription --> t_counter &
    a_KeepAliveCounter : s_subscription --> t_counter & /* missing in table 84 */
    a_MessageSent <: s_subscription &
    /* a_NotificationAvailable <: s_subscription & */ /* <=> a_monitored_item_notifications[{queue}] /= {} */
    a_PublishingEnabled <: s_subscription &
    /* a_PublishingReqQueued <: s_subscription & */ /* <=> a_publish_responses~[{queue}] /= {} */
    /* a_RequestedMessageFound <: s_subscription */   /* <=> seqNum in a_retransmissionQueue[subscription] */
    a_SeqNum : s_subscription --> t_sub_seq_num &

    /* Subscription Implementation variables */
    a_pendingNotificationQueue : s_subscription --> t_notificationQueue &
    a_monitoredItemQueue : s_subscription --> t_monitoredItemQueue &

    a_publishTimer : s_subscription --> t_timer_id &

    /* (Session) Implementation variables */

    /* Note: kept on subscription for now since 1 subs / session + no transfer. Otherwise could be cleared on session close ! */
    a_publishRequestQueue :  s_subscription --> t_publishReqQueue &
    a_notifRepublishQueue : s_subscription --> t_notifRepublishQueue

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typing

INITIALISATION
    s_subscription := {}             ||
    a_subscription_session := {}     ||
    a_publishInterval := {}          ||
    a_lifetimeExpCount := {}         ||
    a_keepAliveExpCount := {}        ||
    a_maxNotifsPerPublish := {}      ||

    a_subscription_state := {}       ||

    a_MoreNotifications := {}        ||
    a_LifetimeCounter := {}          ||
    a_KeepAliveCounter := {}         ||
    a_MessageSent := {}              ||
    a_PublishingEnabled := {}        ||
    a_SeqNum := {}                   ||

    a_pendingNotificationQueue := {} ||
    a_publishRequestQueue := {}      ||
    a_notifRepublishQueue := {}      ||
    a_monitoredItemQueue := {} ||
    a_publishTimer := {}

OPERATIONS

    is_valid <-- is_valid_subscription (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        is_valid := bool(p_subscription : s_subscription)
    END
    ;

    p_dom, p_subscription <-- getall_subscription (p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_session : ran(a_subscription_session)
    THEN
        p_dom := bool(p_session : ran(a_subscription_session)) ||
        IF p_session /: ran(a_subscription_session)
        THEN
            p_subscription := c_subscription_indet
        ELSE
            p_subscription := a_subscription_session~(p_session)
        END
    END
    ;

    p_session <-- getall_session (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        p_session :(p_session : t_session_i & p_session = a_subscription_session(p_subscription))
    END
    ;

    add_subscription (p_subscription,
                      p_session,
                      p_revPublishInterval,
                      p_revLifetimeCount,
                      p_revMaxKeepAlive,
                      p_maxNotificationsPerPublish,
                      p_publishEnabled,
                      p_notifQueue,
                      p_publishQueue,
                      p_republishQueue,
                      p_monitoredItemQueue,
                      p_timerId) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription &
        p_subscription /: s_subscription &
        p_session : t_session_i &
        p_session : t_session &
        p_session /: ran(a_subscription_session) & /* (=> 1 subscription / session) */
        p_revPublishInterval : t_opcua_duration_i &
        p_revLifetimeCount : t_counter &
        p_revLifetimeCount > 0 &
        p_revMaxKeepAlive : t_counter &
        p_revMaxKeepAlive > 0 &
        p_revLifetimeCount >= 3 * p_revMaxKeepAlive & /* part 4 table 86 */
        p_maxNotificationsPerPublish : t_counter &
        p_publishEnabled = TRUE & /* => no subscription modification for now => refuse publishDisabled */
        p_notifQueue : t_notificationQueue_i &
        p_notifQueue : t_notificationQueue &
        p_publishQueue : t_publishReqQueue_i &
        p_publishQueue : t_publishReqQueue &
        p_republishQueue : t_notifRepublishQueue_i &
        p_republishQueue : t_notifRepublishQueue &
        p_monitoredItemQueue : t_monitoredItemQueue_i &
        p_monitoredItemQueue : t_monitoredItemQueue &
        p_timerId : t_timer_id_i &
        p_timerId : t_timer_id
    THEN
        s_subscription := s_subscription \/ {p_subscription} ||
        a_subscription_state (p_subscription) := e_subscriptionState_normal   ||

        a_subscription_session (p_subscription) := p_session ||
        a_publishInterval (p_subscription) := p_revPublishInterval ||
        a_lifetimeExpCount (p_subscription) := p_revLifetimeCount  ||
        a_LifetimeCounter (p_subscription) := p_revLifetimeCount   ||
        a_keepAliveExpCount (p_subscription) := p_revMaxKeepAlive  ||
        a_KeepAliveCounter (p_subscription) := p_revMaxKeepAlive   ||
        a_maxNotifsPerPublish (p_subscription) := p_maxNotificationsPerPublish ||
        a_MessageSent := a_MessageSent - {p_subscription}                      ||
        a_PublishingEnabled := a_PublishingEnabled \/ {p_subscription}         ||
        a_SeqNum (p_subscription) := c_sub_seq_num_init                        || /* => next SeqNum == 1 */

        a_pendingNotificationQueue (p_subscription) := p_notifQueue ||
        a_publishRequestQueue (p_subscription) := p_publishQueue ||
        a_notifRepublishQueue (p_subscription) := p_republishQueue ||
        a_monitoredItemQueue (p_subscription) := p_monitoredItemQueue ||
        a_publishTimer (p_subscription) := p_timerId
    END
    ;

    delete_subscription (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        s_subscription := s_subscription - {p_subscription} ||

        a_subscription_session := {p_subscription} <<| a_subscription_session ||
        a_publishInterval := {p_subscription} <<| a_publishInterval ||
        a_lifetimeExpCount := {p_subscription} <<| a_lifetimeExpCount  ||
        a_keepAliveExpCount := {p_subscription} <<| a_keepAliveExpCount ||
        a_maxNotifsPerPublish := {p_subscription} <<| a_maxNotifsPerPublish ||

        a_subscription_state := {p_subscription} <<| a_subscription_state ||
        a_MoreNotifications := a_MoreNotifications - {p_subscription} ||
        a_LifetimeCounter := {p_subscription} <<| a_LifetimeCounter ||
        a_KeepAliveCounter := {p_subscription} <<| a_KeepAliveCounter ||
        a_MessageSent := a_MessageSent - {p_subscription}               ||
        a_PublishingEnabled := a_PublishingEnabled - {p_subscription}   ||
        a_SeqNum := {p_subscription} <<| a_SeqNum ||

        a_pendingNotificationQueue := {p_subscription} <<| a_pendingNotificationQueue ||

        a_publishRequestQueue := {p_subscription} <<| a_publishRequestQueue ||
        a_notifRepublishQueue := {p_subscription} <<| a_notifRepublishQueue ||

        a_monitoredItemQueue := {p_subscription} <<| a_monitoredItemQueue ||

        a_publishTimer := {p_subscription} <<| a_publishTimer

    END
    ;

    p_publishInterval <-- get_subscription_publishInterval (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        p_publishInterval := a_publishInterval (p_subscription)
    END
    ;

    set_subscription_publishInterval (p_subscription, p_revPublishInterval) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_revPublishInterval : t_opcua_duration_i
    THEN
        a_publishInterval (p_subscription) := p_revPublishInterval
    END
    ;

    set_subscription_timer_id (p_subscription, p_timer_id) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_timer_id : t_timer_id_i  &
        p_timer_id : t_timer_id
    THEN
        a_publishTimer (p_subscription) := p_timer_id
    END
    ;

    p_timer_id <-- get_subscription_timer_id (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        p_timer_id :(p_timer_id : t_timer_id_i &  p_timer_id = a_publishTimer (p_subscription))
    END
    ;

    set_subscription_state (p_subscription, p_state) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_subscription_state) &
        p_state : t_subscriptionState_i &
        p_state : t_subscriptionState
    THEN
        a_subscription_state (p_subscription) := p_state
    END
    ;

    p_state <-- get_subscription_state (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_subscription_state)
    THEN
        p_state := a_subscription_state (p_subscription)
    END
    ;

    set_subscription_MoreNotifications (p_subscription, p_moreNotifs) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_moreNotifs : BOOL
    THEN
        IF p_moreNotifs = TRUE
        THEN
            a_MoreNotifications := a_MoreNotifications \/ {p_subscription}
        ELSE
            a_MoreNotifications := a_MoreNotifications - {p_subscription}
        END
    END
    ;

    p_moreNotifs <-- get_subscription_MoreNotifications (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        p_moreNotifs := bool(p_subscription : a_MoreNotifications)
    END
    ;

    set_subscription_MaxLifetimeAndKeepAliveCount (p_subscription, p_revLifetimeCount, p_revMaxKeepAlive) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_keepAliveExpCount) &
        p_revMaxKeepAlive : t_counter &
        p_revMaxKeepAlive > 0 &
        p_subscription : dom(a_lifetimeExpCount) &
        p_revLifetimeCount : t_counter &
        p_revLifetimeCount > 0 &
        p_revLifetimeCount >= 3 * p_revMaxKeepAlive /* part 4 table 86 */
    THEN
        a_keepAliveExpCount (p_subscription) := p_revMaxKeepAlive ||
        a_lifetimeExpCount(p_subscription) := p_revLifetimeCount
    END
    ;

    decrement_subscription_LifetimeCounter (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_LifetimeCounter) &
        a_LifetimeCounter(p_subscription) > 1 /* when == 1 => expiration */
    THEN
        a_LifetimeCounter(p_subscription) := a_LifetimeCounter(p_subscription) - 1
    END
    ;

    reset_subscription_LifetimeCounter (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_LifetimeCounter)
    THEN
        a_LifetimeCounter(p_subscription) := a_lifetimeExpCount(p_subscription)
    END
    ;

    p_lifetimeCounter <-- get_subscription_LifetimeCounter (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_LifetimeCounter)
    THEN
        p_lifetimeCounter := a_LifetimeCounter(p_subscription)
    END
    ;

    decrement_subscription_KeepAliveCounter (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_KeepAliveCounter) &
        a_KeepAliveCounter(p_subscription) > 1 /* when == 1 => expiration */
    THEN
        a_KeepAliveCounter(p_subscription) := a_KeepAliveCounter(p_subscription) - 1
    END
    ;

    reset_subscription_KeepAliveCounter (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_KeepAliveCounter)
    THEN
        a_KeepAliveCounter(p_subscription) := a_keepAliveExpCount(p_subscription)
    END
    ;

    p_keepAliveCounter <-- get_subscription_KeepAliveCounter (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_KeepAliveCounter)
    THEN
        p_keepAliveCounter := a_KeepAliveCounter(p_subscription)
    END
    ;

    set_subscription_MaxNotifsPerPublish (p_subscription, p_maxNotificationsPerPublish) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_maxNotifsPerPublish) &
        p_maxNotificationsPerPublish : t_counter
    THEN
        a_maxNotifsPerPublish (p_subscription) := p_maxNotificationsPerPublish
    END
    ;

    p_maxNotificationsPerPublish <-- get_subscription_MaxNotifsPerPublish (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_maxNotifsPerPublish)
    THEN
        p_maxNotificationsPerPublish := a_maxNotifsPerPublish (p_subscription)
    END
    ;

    set_subscription_MessageSent (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        a_MessageSent := a_MessageSent \/ {p_subscription}
    END
    ;

    p_firstMsgSent <-- get_subscription_MessageSent (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        p_firstMsgSent := bool(p_subscription : a_MessageSent)
    END
    ;

    set_subscription_PublishingEnabled (p_subscription, p_pubEnabled) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_pubEnabled : BOOL
    THEN
        IF p_pubEnabled = TRUE
        THEN
            a_PublishingEnabled := a_PublishingEnabled \/ {p_subscription}
        ELSE
            a_PublishingEnabled := a_PublishingEnabled - {p_subscription}
        END
    END
    ;

    p_pubEnabled <-- get_subscription_PublishingEnabled (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        p_pubEnabled := bool(p_subscription : a_PublishingEnabled)
    END
    ;

    set_subscription_SeqNum (p_subscription, p_nextSeqNum) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_SeqNum) &
        p_nextSeqNum : t_sub_seq_num_i &
        p_nextSeqNum : t_sub_seq_num
    THEN
        a_SeqNum(p_subscription) := p_nextSeqNum
    END
    ;

    p_seqNumToSend <-- get_subscription_SeqNum (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_SeqNum)
    THEN
        p_seqNumToSend :(p_seqNumToSend : t_sub_seq_num_i & p_seqNumToSend = a_SeqNum(p_subscription))
    END
    ;

    p_notificationQueue <-- get_subscription_notificationQueue (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_pendingNotificationQueue)
    THEN
        p_notificationQueue :(p_notificationQueue : t_notificationQueue_i &
                              p_notificationQueue = a_pendingNotificationQueue(p_subscription))
    END
    ;

    p_publishReqQueue <-- get_subscription_publishRequestQueue (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_publishRequestQueue)
    THEN
        p_publishReqQueue :(p_publishReqQueue : t_publishReqQueue_i &
                            p_publishReqQueue = a_publishRequestQueue(p_subscription))
    END
    ;

    p_republishQueue <-- get_subscription_notifRepublishQueue (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_notifRepublishQueue)
    THEN
        p_republishQueue :(p_republishQueue : t_notifRepublishQueue_i &
                           p_republishQueue = a_notifRepublishQueue(p_subscription))
    END
    ;

    p_monitoredItemQueue <-- get_subscription_monitoredItemQueue (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_monitoredItemQueue)
    THEN
        p_monitoredItemQueue :(p_monitoredItemQueue : t_monitoredItemQueue_i &
                               p_monitoredItemQueue = a_monitoredItemQueue(p_subscription))
    END

END