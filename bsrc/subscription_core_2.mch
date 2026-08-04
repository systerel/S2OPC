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
    subscription_core_2

SEES
    constants,
    message_out_bs,
    request_handle_bs,
    constants_statuscodes_bs

DEFINITIONS
    d_variables ==
    s_subscription,
    a_subscription_session,
    a_publishInterval,
    a_lifetimeExpCount,
    a_keepAliveExpCount,
    a_maxNotifsPerPublish,

    a_subscription_priority,
    a_session_seq_priority,
    a_session_nb_subscriptions,
    a_session_priority_subscriptions_queue,

    a_subscription_state,
    a_MoreNotifications,
    a_LifetimeCounter,
    a_KeepAliveCounter,
    a_MessageSent,
    a_PublishingEnabled,
    a_SeqNum,

    a_publishRequestQueue,
    a_notifRepublishQueue,
    a_monitoredItemQueue,
    a_publishTimer;

    d_typing ==
    s_subscription <: t_subscription &

    a_subscription_session : t_subscription +-> t_session          &
    a_publishInterval      : t_subscription +-> t_opcua_duration_i &
    a_lifetimeExpCount     : t_subscription +-> t_counter          &
    a_keepAliveExpCount    : t_subscription +-> t_counter          &
    a_maxNotifsPerPublish  : t_subscription +-> t_counter          &

    a_subscription_priority : t_subscription +-> t_priority             &
    a_session_seq_priority  : t_session --> (t_prio_idx >+> t_priority) &
    a_session_nb_subscriptions : t_session --> t_sub_idx_i &
    a_session_priority_subscriptions_queue : t_session --> (t_prio_idx +-> t_prioritySubQueue) &

    a_subscription_state : t_subscription +-> t_subscriptionState &

    a_MoreNotifications <: t_subscription &
    a_LifetimeCounter : t_subscription +-> t_counter &
    a_KeepAliveCounter : t_subscription +-> t_counter &
    a_MessageSent <: t_subscription &
    a_PublishingEnabled <: t_subscription &
    a_SeqNum : t_subscription +-> t_sub_seq_num &

    a_monitoredItemQueue : t_subscription +-> t_monitoredItemQueue &

    a_publishTimer : t_subscription +-> t_timer_id &

    a_publishRequestQueue : t_session +-> t_publishReqQueue &
    a_notifRepublishQueue : t_subscription +-> t_notifRepublishQueue

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

    a_subscription_priority := {} ||
    a_session_seq_priority := t_session * {{}} ||
    a_session_priority_subscriptions_queue := t_session * {{}} ||
    a_session_nb_subscriptions := t_session * {0} ||

    a_subscription_state := {}       ||

    a_MoreNotifications := {}        ||
    a_LifetimeCounter := {}          ||
    a_KeepAliveCounter := {}         ||
    a_MessageSent := {}              ||
    a_PublishingEnabled := {}        ||
    a_SeqNum := {}                   ||

    a_publishRequestQueue := {}      ||
    a_notifRepublishQueue := {}      ||
    a_monitoredItemQueue := {} ||
    a_publishTimer := {}

OPERATIONS

    is_valid <-- is_valid_subscription_on_session (p_session, p_subscription) =
    PRE
        p_session      : t_session_i      &
        p_session      : t_session        &
        p_subscription : t_subscription_i
    THEN
        is_valid := bool(p_subscription : s_subscription & a_subscription_session(p_subscription) = p_session)
    END
    ;

    is_valid <-- is_valid_subscription (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        is_valid := bool(p_subscription : s_subscription)
    END
    ;

    set_subscription (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription
    THEN
        s_subscription := s_subscription \/ {p_subscription}
    END
    ;

    del_subscription (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        s_subscription          := s_subscription - {p_subscription}           ||
        a_subscription_state    := {p_subscription} <<| a_subscription_state   ||
        a_subscription_session  := {p_subscription} <<| a_subscription_session ||
        a_publishInterval       := {p_subscription} <<| a_publishInterval      ||
        a_lifetimeExpCount      := {p_subscription} <<| a_lifetimeExpCount     ||
        a_keepAliveExpCount     := {p_subscription} <<| a_keepAliveExpCount    ||
        a_maxNotifsPerPublish   := {p_subscription} <<| a_maxNotifsPerPublish  ||
        a_PublishingEnabled     := a_PublishingEnabled - {p_subscription}      ||
        a_MoreNotifications     := a_MoreNotifications - {p_subscription}      ||
        a_LifetimeCounter       := {p_subscription} <<| a_LifetimeCounter      ||
        a_KeepAliveCounter      := {p_subscription} <<| a_KeepAliveCounter     ||
        a_MessageSent           := a_MessageSent - {p_subscription}            ||
        a_SeqNum                := {p_subscription} <<| a_SeqNum               ||
        a_notifRepublishQueue   := {p_subscription} <<| a_notifRepublishQueue  ||
        a_monitoredItemQueue    := {p_subscription} <<| a_monitoredItemQueue   ||
        a_publishTimer          := {p_subscription} <<| a_publishTimer         ||
        a_subscription_priority := {p_subscription} <<| a_subscription_priority
    END
    ;

    p_session <-- getall_session (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_subscription_session)
    THEN
        p_session
        :(p_session : t_session_i &
          p_session = a_subscription_session(p_subscription))
    END
    ;

    p_priority_idx <-- get_card_session_seq_priority (p_session) =
    PRE
        p_session : t_session_i  &
        p_session : t_session
    THEN
        p_priority_idx
        :(p_priority_idx : t_prio_idx_i &
          p_priority_idx = card(dom(a_session_seq_priority (p_session)))
         )
    END
    ;

    p_sub_idx <-- get_card_session_seq_subscription (p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        p_sub_idx
        :(p_sub_idx : t_sub_idx_i &
          p_sub_idx = a_session_nb_subscriptions(p_session)
         )
    END
    ;

    p_session <-- get_subscription_session (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_subscription_session)
    THEN
        p_session
        :(p_session : t_session_i &
          p_session = a_subscription_session(p_subscription))
    END
    ;

    set_subscription_session (p_subscription, p_session) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription   &
        p_session      : t_session_i      &
        p_session      : t_session
    THEN
        a_subscription_session(p_subscription) := p_session
    END
    ;

    remove_session_seq_priority (p_session, p_priority_idx) =
    PRE
        p_session      : t_session_i  &
        p_session      : t_session    &
        p_priority_idx : t_prio_idx_i &
        p_priority_idx : t_prio_idx   &
        p_priority_idx : dom(a_session_seq_priority(p_session)) &
        a_session_seq_priority : t_session --> (t_prio_idx >+> t_priority)
    THEN
        a_session_seq_priority (p_session) := {p_priority_idx} <<| a_session_seq_priority (p_session)
    END
    ;

    p_priority <-- getall_session_seq_priority (p_session, p_priority_idx) =
    PRE
        p_session      : t_session_i  &
        p_session      : t_session    &
        p_priority_idx : t_prio_idx_i
    THEN
        p_priority
        :(p_priority : t_priority_i &
          (p_priority_idx : dom(a_session_seq_priority(p_session))
           =>
           p_priority = a_session_seq_priority(p_session)(p_priority_idx)) &
          (p_priority_idx /: dom(a_session_seq_priority(p_session))
           =>
           p_priority = c_priority_indet)
         )
    END
    ;

    p_priority <-- get_session_seq_priority (p_session, p_priority_idx) =
    PRE
        p_session      : t_session_i  &
        p_session      : t_session    &
        p_priority_idx : t_prio_idx_i &
        p_priority_idx : dom(a_session_seq_priority(p_session))
    THEN
        p_priority
        :(p_priority : t_priority_i &
          p_priority = a_session_seq_priority(p_session)(p_priority_idx)
         )
    END
    ;

    p_priority_idx <-- get_session_priority_idx (p_session, p_priority) =
    PRE
        p_session  : t_session_i  &
        p_session  : t_session    &
        p_priority : t_priority_i
    THEN
        p_priority_idx
        :(p_priority_idx : t_prio_idx_i &
          (p_priority : dom(a_session_seq_priority(p_session)~)
           =>
           p_priority_idx = (a_session_seq_priority(p_session))~(p_priority)) &
          (p_priority /: dom(a_session_seq_priority(p_session)~)
           =>
           p_priority_idx = 0)
         )
    END
    ;

    insert_session_seq_priority (p_session, p_priority_idx, p_priority) =
    PRE
        p_session       : t_session_i  &
        p_session       : t_session    &
        p_priority_idx  : t_prio_idx_i &
        p_priority_idx  : t_prio_idx   &
        p_priority_idx /: dom(a_session_seq_priority(p_session)) &
        p_priority      : t_priority_i &
        p_priority      : t_priority   &
        p_priority     /: ran(a_session_seq_priority(p_session)) &
        a_session_seq_priority : t_session --> (t_prio_idx >+> t_priority) &
        card(dom(a_session_seq_priority(p_session))) /= k_n_subscriptionPerSession_max
    THEN
        a_session_seq_priority (p_session)(p_priority_idx) := p_priority
    END
    ;

    swap_session_seq_priority (p_session, p_priority_idx0, p_priority_idx1) =
    PRE
        p_session        : t_session_i  &
        p_session        : t_session    &
        p_priority_idx0  : t_prio_idx_i &
        p_priority_idx0  : t_prio_idx   &
        p_priority_idx1  : t_prio_idx_i &
        p_priority_idx1  : t_prio_idx   &
        p_priority_idx0 /: dom(a_session_seq_priority(p_session)) &
        p_priority_idx1  : dom(a_session_seq_priority(p_session)) &
        a_session_seq_priority : t_session --> (t_prio_idx >+> t_priority)
    THEN
        a_session_seq_priority (p_session) := {p_priority_idx1} <<| a_session_seq_priority (p_session)
                                              <+ {p_priority_idx0 |-> a_session_seq_priority (p_session)(p_priority_idx1)}
    END
    ;

    set_card_session_seq_subscription (p_session, p_sub_idx) =
    PRE
        p_session : t_session_i &
        p_session : t_session   &
        p_sub_idx : t_sub_idx_i
    THEN
        a_session_nb_subscriptions(p_session) := p_sub_idx
    END
    ;

    decrease_card_session_seq_subscription (p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        a_session_nb_subscriptions
        :(a_session_nb_subscriptions : t_session --> t_sub_idx_i &
          (a_session_nb_subscriptions$0 (p_session) >= 1
           =>
           a_session_nb_subscriptions = a_session_nb_subscriptions$0 <+
              {p_session |-> a_session_nb_subscriptions$0 (p_session) - 1}) &
          (not(a_session_nb_subscriptions$0 (p_session) >= 1)
           =>
           a_session_nb_subscriptions = a_session_nb_subscriptions$0)
        )
    END
    ;

    p_priority <-- get_subscription_priority(p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_subscription_priority)
    THEN
        p_priority := a_subscription_priority (p_subscription)
    END
    ;

    set_subscription_priority(p_subscription, p_priority) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription   &
        p_priority     : t_priority_i     &
        p_priority     : t_priority
    THEN
        a_subscription_priority (p_subscription) := p_priority
    END
    ;

    p_prioritySubQueue <-- get_session_priority_subscriptions_queue (p_session, p_prio_idx) =
    PRE
        p_session  : t_session_i  &
        p_session  : t_session    &
        p_prio_idx : t_prio_idx_i &
        p_prio_idx : dom(a_session_priority_subscriptions_queue (p_session))
    THEN
        p_prioritySubQueue
        :(p_prioritySubQueue : t_prioritySubQueue_i &
          p_prioritySubQueue = a_session_priority_subscriptions_queue (p_session)(p_prio_idx))
    END
    ;

    set_session_priority_subscriptions_queue (p_session, p_prio_idx, p_prioritySubQueue) =
    PRE
        p_session          : t_session_i          &
        p_session          : t_session            &
        p_prio_idx         : t_prio_idx_i         &
        p_prio_idx         : t_prio_idx           &
        p_prioritySubQueue : t_prioritySubQueue_i &
        p_prioritySubQueue : t_prioritySubQueue
    THEN
        a_session_priority_subscriptions_queue (p_session)(p_prio_idx) := p_prioritySubQueue
    END
    ;

    remove_session_priority_subscriptions_queue (p_session, p_prio_idx) =
    PRE
        p_session  : t_session_i  &
        p_session  : t_session    &
        p_prio_idx : t_prio_idx_i &
        p_prio_idx : t_prio_idx
    THEN
        a_session_priority_subscriptions_queue (p_session) :=
            {p_prio_idx} <<| a_session_priority_subscriptions_queue (p_session)
    END
    ;

    p_publishInterval <-- get_subscription_publishInterval (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_publishInterval)
    THEN
        p_publishInterval := a_publishInterval (p_subscription)
    END
    ;

    set_subscription_publishInterval (p_subscription, p_revPublishInterval) =
    PRE
        p_subscription       : t_subscription_i &
        p_subscription       : t_subscription   &
        p_revPublishInterval : t_opcua_duration_i
    THEN
        a_publishInterval (p_subscription) := p_revPublishInterval
    END
    ;

    set_subscription_timer_id (p_subscription, p_timer_id) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription   &
        p_timer_id     : t_timer_id_i     &
        p_timer_id     : t_timer_id
    THEN
        a_publishTimer (p_subscription) := p_timer_id
    END
    ;

    p_timer_id <-- get_subscription_timer_id (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_publishTimer)
    THEN
        p_timer_id
        :(p_timer_id : t_timer_id_i &
          p_timer_id = a_publishTimer (p_subscription))
    END
    ;

    set_subscription_state (p_subscription, p_state, p_transition) =
    PRE
        p_subscription : t_subscription_i      &
        p_subscription : t_subscription        &
        p_state        : t_subscriptionState_i &
        p_state        : t_subscriptionState   &
        p_transition   : NAT
    THEN
        a_subscription_state (p_subscription) := p_state
    END
    ;

    p_state <-- get_subscription_state (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_subscription_state)
    THEN
        p_state :(p_state : t_subscriptionState_i &
                  p_state = a_subscription_state (p_subscription))
    END
    ;

    set_subscription_MoreNotifications (p_subscription, p_moreNotifs) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription   &
        p_moreNotifs   : BOOL
    THEN
        a_MoreNotifications
        :(  a_MoreNotifications <: t_subscription &
            (p_moreNotifs = TRUE
             =>
             a_MoreNotifications = a_MoreNotifications$0 \/ {p_subscription}) &
            (p_moreNotifs = FALSE
             =>
             a_MoreNotifications = a_MoreNotifications$0 - {p_subscription})
        )
    END
    ;

    p_moreNotifs <-- get_subscription_MoreNotifications (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        p_moreNotifs := bool(p_subscription : a_MoreNotifications)
    END
    ;

    set_subscription_MaxLifetimeAndKeepAliveCount (p_subscription, p_revLifetimeCount, p_revMaxKeepAlive) =
    PRE
        p_subscription     : t_subscription_i &
        p_subscription     : t_subscription   &
        p_revMaxKeepAlive  : t_counter        &
        p_revMaxKeepAlive  > 0                &
        p_revLifetimeCount : t_counter        &
        p_revLifetimeCount > 0                &
        p_revLifetimeCount >= 3 * p_revMaxKeepAlive /* part 4 table 86 */
    THEN
        a_keepAliveExpCount (p_subscription) := p_revMaxKeepAlive ||
        a_lifetimeExpCount  (p_subscription) := p_revLifetimeCount
    END
    ;

    set_subscription_LifetimeAndKeepAliveCounter (p_subscription, p_LifetimeCounter, p_KeepAliveCounter) =
    PRE
        p_subscription      : t_subscription_i &
        p_subscription      : t_subscription &
        p_LifetimeCounter   : t_counter &
        p_KeepAliveCounter  : t_counter
    THEN
        a_LifetimeCounter(p_subscription) := p_LifetimeCounter ||
        a_KeepAliveCounter(p_subscription) := p_KeepAliveCounter
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
        p_subscription : dom(a_lifetimeExpCount)
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
        p_subscription : dom(a_keepAliveExpCount)
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
        p_subscription               : t_subscription_i &
        p_subscription               : t_subscription   &
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
        p_subscription : t_subscription
    THEN
        a_MessageSent := a_MessageSent \/ {p_subscription}
    END
    ;

    setreset_subscription_MessageSent (p_subscription, p_bool) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription   &
        p_bool         : BOOL
    THEN
        a_MessageSent
        :(a_MessageSent <: t_subscription &
          (p_bool = TRUE  => a_MessageSent = a_MessageSent$0 \/ {p_subscription}) &
          (p_bool = FALSE => a_MessageSent = a_MessageSent$0 -  {p_subscription})
        )
    END
    ;

    p_firstMsgSent <-- get_subscription_MessageSent (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        p_firstMsgSent := bool(p_subscription : a_MessageSent)
    END
    ;

    set_subscription_PublishingEnabled (p_subscription, p_pubEnabled) =
    PRE
        p_subscription : t_subscription_i        &
        p_subscription : t_subscription          &
        p_subscription : dom(a_lifetimeExpCount) &
        p_pubEnabled   : BOOL
    THEN
        /* Transition #19 */
        a_PublishingEnabled,
        a_MoreNotifications,
        a_LifetimeCounter
        :(a_PublishingEnabled <: t_subscription &
          a_MoreNotifications <: t_subscription &
          a_LifetimeCounter : t_subscription +-> t_counter &

          (p_pubEnabled = TRUE  => a_PublishingEnabled = a_PublishingEnabled$0 \/ {p_subscription}) &
          (p_pubEnabled = FALSE => a_PublishingEnabled = a_PublishingEnabled$0 - {p_subscription}) &
          a_MoreNotifications = a_MoreNotifications$0 - {p_subscription} &
          a_LifetimeCounter = a_LifetimeCounter$0 <+ {p_subscription |-> a_lifetimeExpCount(p_subscription)}
        )
    END
    ;

    p_pubEnabled <-- get_subscription_PublishingEnabled (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        p_pubEnabled := bool(p_subscription : a_PublishingEnabled)
    END
    ;

    set_subscription_SeqNum (p_subscription, p_nextSeqNum) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription   &
        p_nextSeqNum   : t_sub_seq_num_i  &
        p_nextSeqNum   : t_sub_seq_num
    THEN
        a_SeqNum(p_subscription) := p_nextSeqNum
    END
    ;

    p_seqNumToSend <-- get_subscription_SeqNum (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_SeqNum)
    THEN
        p_seqNumToSend
        :(p_seqNumToSend : t_sub_seq_num_i &
          p_seqNumToSend = a_SeqNum(p_subscription))
    END
    ;

    reset_session_publishRequestQueue (p_session) =
    PRE
        p_session : t_session_i
    THEN
        a_publishRequestQueue := {p_session} <<| a_publishRequestQueue
    END
    ;

    set_session_publishRequestQueue (p_session, p_publishReqQueue) =
    PRE
        p_session         : t_session_i         &
        p_session         : t_session           &
        p_publishReqQueue : t_publishReqQueue_i &
        p_publishReqQueue : t_publishReqQueue
    THEN
        a_publishRequestQueue(p_session) := p_publishReqQueue
    END
    ;

    p_bres, p_publishReqQueue <-- get_session_publishRequestQueue (p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        p_bres, p_publishReqQueue
        :(p_bres : BOOL &
          p_publishReqQueue : t_publishReqQueue_i &
          p_bres = bool(p_session : dom(a_publishRequestQueue)) &
          (p_bres = TRUE
           =>
           p_publishReqQueue = a_publishRequestQueue(p_session))
        )
    END
    ;

    p_republishQueue <-- get_subscription_notifRepublishQueue (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_notifRepublishQueue)
    THEN
        p_republishQueue
        :(p_republishQueue : t_notifRepublishQueue_i &
          p_republishQueue = a_notifRepublishQueue(p_subscription))
    END
    ;

    set_subscription_notifRepublishQueue (p_subscription, p_republishQueue) =
    PRE
        p_subscription   : t_subscription_i        &
        p_subscription   : t_subscription          &
        p_republishQueue : t_notifRepublishQueue_i &
        p_republishQueue : t_notifRepublishQueue
    THEN
        a_notifRepublishQueue(p_subscription) := p_republishQueue
    END
    ;

    p_monitoredItemQueue <-- get_subscription_monitoredItemQueue (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_monitoredItemQueue)
    THEN
        p_monitoredItemQueue
        :(p_monitoredItemQueue : t_monitoredItemQueue_i &
          p_monitoredItemQueue = a_monitoredItemQueue(p_subscription))
    END
    ;

    set_subscription_monitoredItemQueue (p_subscription, p_monitoredItemQueue) =
    PRE
        p_subscription       : t_subscription_i       &
        p_subscription       : t_subscription         &
        p_monitoredItemQueue : t_monitoredItemQueue_i &
        p_monitoredItemQueue : t_monitoredItemQueue
    THEN
        a_monitoredItemQueue(p_subscription) := p_monitoredItemQueue
    END
    ;

    log_subscription_transition (p_subscription, p_old_state, p_new_state, p_transition) =
    PRE
        p_subscription : t_subscription_i      &
        p_subscription : t_subscription        &
        p_old_state    : t_subscriptionState_i &
        p_new_state    : t_subscriptionState_i &
        p_transition   : NAT
    THEN
        skip
    END
    ;

    log_subscription_request_received (p_subscription, p_transition) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription   &
        p_transition   : NAT
    THEN
        skip
    END
    ;

    log_subscription_request_received_invalid_on_session (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        skip
    END

END
