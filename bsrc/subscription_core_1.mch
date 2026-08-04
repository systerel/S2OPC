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
    a_session_priority_nb_subs,
    a_session_priority_subscriptions_queue,
    a_prio_queue_defined_subscriptions,
    /* Priority subscriptions queue iterator variables */
    a_priority_subscriptions_queue_iterator,
    prio_sub_queue_it,
    prio_sub_iterator_it,
    nb_subscriptions_it,
    s_prio_sub_to_iterate_it,
    s_prio_sub_iterated_it,

    a_subscription_state,
    a_MoreNotifications,
    a_LifetimeCounter,
    a_KeepAliveCounter,
    a_MessageSent,
    a_PublishingEnabled,
    a_SeqNum, // stores the NEXT notification SN

    a_publishRequestQueue,
    a_notifRepublishQueue,
    a_monitoredItemQueue,
    a_publishTimer;

    d_typing ==
    s_subscription <: t_subscription &

    /* Subscription configuration record */
    a_subscription_session : s_subscription --> t_session &     // a subscription shall be associated to a session
    a_publishInterval : s_subscription --> t_opcua_duration_i & // a subscription shall have a publishing interval value
    a_lifetimeExpCount : s_subscription --> t_counter &       // a subscription shall have a lifetime expiration count value
    a_keepAliveExpCount : s_subscription --> t_counter &      // a subscription shall have a keepAlive expiration count value
    a_maxNotifsPerPublish : s_subscription --> t_counter &    // a subscription shall have a maximum notifications per publish response

    a_subscription_priority : s_subscription --> t_priority &
    a_session_seq_priority : t_session --> iseq(t_priority) &
    a_session_nb_subscriptions : t_session --> t_sub_idx_i &
    a_session_priority_nb_subs : t_session --> (t_prio_idx +-> t_sub_idx) &
    /* Record of the round robin queue of subscriptions for a given priority */
    a_session_priority_subscriptions_queue : t_session --> (t_prio_idx +-> t_prioritySubQueue) &
    /* Every defined priority with subscription has a priority subscriptions queue */
    !session.(session : t_session
              =>
              dom(a_session_priority_subscriptions_queue(session)) = dom(a_session_seq_priority(session))) &

    a_prio_queue_defined_subscriptions : t_prioritySubQueue <-> t_subscription &
    /* Priority subscriptions queue iterator variables */
    a_priority_subscriptions_queue_iterator : t_prioritySubQueue >->> t_prioritySubQueueIterator  &
    prio_sub_queue_it : t_prioritySubQueue_i &
    prio_sub_iterator_it : t_prioritySubQueueIterator_i &
    nb_subscriptions_it : NAT &
    s_prio_sub_to_iterate_it <: t_subscription_i &
    s_prio_sub_to_iterate_it <: t_subscription &
    s_prio_sub_iterated_it <: t_subscription_i &
    s_prio_sub_iterated_it <: t_subscription &
    s_prio_sub_to_iterate_it /\ s_prio_sub_iterated_it = {} &
    card(s_prio_sub_to_iterate_it \/ s_prio_sub_iterated_it) = nb_subscriptions_it &

    /* Subscription State Machine : state shall be defined (part 4 table 82) */
    a_subscription_state : s_subscription --> t_subscriptionState &


    /* Subscription State Machine : state shall be defined (part 4 table 82) */
    a_subscription_state : s_subscription --> t_subscriptionState &

    /* Subscription State Machine : variables shall be defined (part 4 table 84) */
    a_MoreNotifications <: s_subscription &
    a_LifetimeCounter : s_subscription --> t_counter &
    a_KeepAliveCounter : s_subscription --> t_counter & /* missing in table 84 */
    a_MessageSent <: s_subscription &
    /* a_NotificationAvailable <: s_subscription & */ /* <=> a_notification_queue_notifications[
                                                               a_monitored_item_notification_queue[
                                                                 a_defined_monitored_items[a_monitoredItemQueue[{p_subscription}]]]] /= {} */
    a_PublishingEnabled <: s_subscription &
    /* a_PublishingReqQueued <: s_subscription & */ /* <=> a_publish_responses~[{queue}] /= {} */
    /* a_RequestedMessageFound <: s_subscription */   /* <=> seqNum in a_retransmissionQueue[subscription] */
    a_SeqNum : s_subscription --> t_sub_seq_num &

    /* Subscription Implementation variables */
    a_monitoredItemQueue : s_subscription --> t_monitoredItemQueue &

    a_publishTimer : s_subscription --> t_timer_id &

    /* (Session) Implementation variables */

    /* Note: kept on subscription for now since 1 subs / session + no transfer. Otherwise could be cleared on session close ! */
    a_publishRequestQueue :  t_session +-> t_publishReqQueue &
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

    a_subscription_priority := {} ||
    a_session_seq_priority := t_session * {{}} ||
    a_session_nb_subscriptions := t_session * {0} ||
    a_session_priority_nb_subs := t_session * {{}} ||
    a_session_priority_subscriptions_queue := t_session * {{}} ||
    a_prio_queue_defined_subscriptions := {} ||
    a_priority_subscriptions_queue_iterator :(a_priority_subscriptions_queue_iterator : t_prioritySubQueue >->> t_prioritySubQueueIterator ) ||
    prio_sub_queue_it := c_prioritySubQueue_indet ||
    prio_sub_iterator_it := c_prioritySubQueueIterator_indet ||
    nb_subscriptions_it := 0 ||
    s_prio_sub_to_iterate_it :(s_prio_sub_to_iterate_it = {}) ||
    s_prio_sub_iterated_it :(s_prio_sub_iterated_it = {}) ||

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
        p_session : t_session_i &
        p_session : t_session &
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

    p_session <-- getall_session (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        p_session :(p_session : t_session_i & p_session = a_subscription_session(p_subscription))
    END
    ;

    p_priority_idx <-- get_card_session_seq_priority (p_session) =
    PRE
        p_session : t_session_i  &
        p_session : t_session
    THEN
        p_priority_idx
        :(p_priority_idx : t_prio_idx_i &
          p_priority_idx = card(a_session_seq_priority(p_session))
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

    p_prio_sub_it <-- init_session_priority_subscriptions_iterator (p_session, p_prio_idx) =
    PRE
        p_session  : t_session_i &
        p_session  : t_session   &
        p_prio_idx : t_prio_idx_i &
        p_prio_idx : t_prio_idx &
        /* Note: it should be guaranteed by the priority index  domain equality with a_session_seq_priority
                 and priority iterator based on a_session_seq_priority */
        p_prio_idx : dom(a_session_priority_subscriptions_queue(p_session))
    THEN
        prio_sub_queue_it,
        prio_sub_iterator_it,
        nb_subscriptions_it,
        s_prio_sub_to_iterate_it,
        s_prio_sub_iterated_it,
        p_prio_sub_it :(
          prio_sub_queue_it : t_prioritySubQueue_i &
          prio_sub_iterator_it : t_prioritySubQueueIterator_i &
          nb_subscriptions_it : NAT &
          s_prio_sub_to_iterate_it <: t_subscription &
          s_prio_sub_iterated_it <: t_subscription &
          p_prio_sub_it : t_prioritySubQueueIterator_i &
          /* Note: every allocated queue contains always at least 1 element */
          prio_sub_queue_it = a_session_priority_subscriptions_queue (p_session)(p_prio_idx) &
          prio_sub_iterator_it = a_priority_subscriptions_queue_iterator(prio_sub_queue_it) &
          p_prio_sub_it = prio_sub_iterator_it &
          nb_subscriptions_it > 0 &
          s_prio_sub_to_iterate_it /= {} &
          card(s_prio_sub_to_iterate_it) = nb_subscriptions_it &
          s_prio_sub_iterated_it = {}
        )
    END
    ;

    p_continue, p_subscription, p_iterator_updated <-- continue_iter_subscription_priority_queue (p_iterator) =
    PRE
        p_iterator : t_prioritySubQueueIterator_i &
        prio_sub_iterator_it = p_iterator &
        prio_sub_queue_it = a_priority_subscriptions_queue_iterator~(p_iterator) &
        s_prio_sub_to_iterate_it /= {}
    THEN
        ANY l_subscription WHERE
            l_subscription : t_subscription_i &
            l_subscription : s_prio_sub_to_iterate_it
        THEN
            p_subscription := l_subscription ||
            s_prio_sub_to_iterate_it := s_prio_sub_to_iterate_it - {l_subscription} ||
            s_prio_sub_iterated_it := s_prio_sub_iterated_it \/ {l_subscription} ||
            p_continue,
            prio_sub_iterator_it,
            p_iterator_updated :(
                p_continue : BOOL &
                prio_sub_iterator_it : t_prioritySubQueueIterator_i &
                p_iterator_updated : t_prioritySubQueueIterator_i &
                p_continue = bool(s_prio_sub_to_iterate_it /= {l_subscription}) &
                (p_continue = TRUE =>
                        prio_sub_iterator_it = a_priority_subscriptions_queue_iterator(prio_sub_queue_it)) &
                (p_continue = FALSE => prio_sub_iterator_it = c_prioritySubQueueIterator_indet) &
                p_iterator_updated = prio_sub_iterator_it
            )
        END
    END
    ;

    /* Move the given subscription at the end of the queue for next iterations
     * IMPORTANT: it SHALL not be called during iteration !!! */
    reconfigure_session_priority_subscriptions_queue_last_sub (p_session, p_sub) =
    PRE
        p_session  : t_session_i &
        p_session  : t_session   &
        p_sub : t_subscription_i &
        p_sub : t_subscription &
        p_sub : s_subscription
    THEN
        skip /* Reordering iterator is not represented */
    END
    ;

    p_prio_subs_queue, p_StatusCode_service <-- check_add_subscription (p_session, p_subscription, p_priority, p_isModifySub) =
    PRE
        p_session      : t_session_i      &
        p_session      : t_session        &
        p_subscription : t_subscription_i &
        p_subscription : t_subscription   &
        p_priority     : t_priority       &
        p_isModifySub  : BOOL             &
        (p_isModifySub = TRUE =>
            p_subscription : dom(a_subscription_session) &
            a_subscription_session(p_subscription) = p_session)
    THEN
        p_prio_subs_queue,
        p_StatusCode_service,
        s_prio_sub_to_iterate_it,
        s_prio_sub_iterated_it,
        a_prio_queue_defined_subscriptions
        :(
            p_prio_subs_queue    : t_prioritySubQueue_i &
            p_StatusCode_service : t_StatusCode_i       &
            p_StatusCode_service : {e_sc_ok, e_sc_bad_too_many_subscriptions, e_sc_bad_out_of_memory} &

            a_prio_queue_defined_subscriptions : t_prioritySubQueue <-> t_subscription &
            s_prio_sub_to_iterate_it <: t_subscription_i &
            s_prio_sub_to_iterate_it <: t_subscription &
            s_prio_sub_iterated_it <: t_subscription_i &
            s_prio_sub_iterated_it <: t_subscription &
            s_prio_sub_to_iterate_it /\ s_prio_sub_iterated_it = {} &
            card(s_prio_sub_to_iterate_it \/ s_prio_sub_iterated_it) = nb_subscriptions_it &
            dom(a_session_priority_subscriptions_queue(p_session)) = dom(a_session_seq_priority(p_session)) &

            (p_StatusCode_service = e_sc_ok =>
                 (p_isModifySub = FALSE => a_session_nb_subscriptions(p_session) < k_n_subscriptionPerSession_max) &
                 (p_isModifySub = TRUE => a_session_nb_subscriptions(p_session) <= k_n_subscriptionPerSession_max) &
                 (p_priority /: dom(a_session_seq_priority(p_session)~)
                  =>
                  card(dom(a_session_seq_priority (p_session))) < k_n_subscriptionPerSession_max) &
                 p_prio_subs_queue : t_prioritySubQueue) &
            (p_StatusCode_service /= e_sc_ok =>
                 s_prio_sub_to_iterate_it = s_prio_sub_to_iterate_it$0 &
                 s_prio_sub_iterated_it = s_prio_sub_iterated_it$0 &
                 a_prio_queue_defined_subscriptions = a_prio_queue_defined_subscriptions$0)
        )
    END
    ;

    add_subscription
       (p_subscription,
        p_session,
        p_state,
        p_firstMsgSent,
        p_priority,
        p_revPublishInterval,
        p_revLifetimeCount,
        p_revKeepAliveCount,
        p_revMaxKeepAlive,
        p_maxNotificationsPerPublish,
        p_publishEnabled,
        p_seqNumInit,
        p_republishQueue,
        p_monitoredItemQueue,
        p_timerId,
        p_prio_subs_queue,
        p_moreNotifs) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription &
        p_subscription /: s_subscription &
        p_session : t_session_i &
        p_session : t_session &
        p_state : t_subscriptionState_i &
        p_state : t_subscriptionState & /* e_subscriptionState_normal on creation */
        p_firstMsgSent : BOOL &
        p_priority : t_priority &
        p_revPublishInterval : t_opcua_duration_i &
        p_revLifetimeCount : t_counter &
        p_revLifetimeCount > 0 &
        p_revKeepAliveCount : t_counter &
        p_revMaxKeepAlive : t_counter &
        p_revMaxKeepAlive > 0 &
        p_revLifetimeCount >= 3 * p_revMaxKeepAlive & /* part 4 table 86 */
        p_maxNotificationsPerPublish : t_counter &
        p_publishEnabled : BOOL &
        p_seqNumInit : t_sub_seq_num_i &
        p_seqNumInit : t_sub_seq_num &
        p_republishQueue : t_notifRepublishQueue_i &
        p_republishQueue : t_notifRepublishQueue &
        p_monitoredItemQueue : t_monitoredItemQueue_i &
        p_monitoredItemQueue : t_monitoredItemQueue &
        p_timerId : t_timer_id_i &
        p_timerId : t_timer_id &
        a_session_nb_subscriptions(p_session) < k_n_subscriptionPerSession_max &
        p_prio_subs_queue : t_prioritySubQueue_i &
        p_prio_subs_queue : t_prioritySubQueue &
        p_moreNotifs : BOOL &
        card(dom(a_session_seq_priority (p_session))) < k_n_subscriptionPerSession_max
    THEN
        s_subscription := s_subscription \/ {p_subscription} ||
        a_subscription_state (p_subscription) := p_state     ||
        a_subscription_session (p_subscription) := p_session ||
        a_subscription_priority (p_subscription) := p_priority ||
        a_publishInterval (p_subscription) := p_revPublishInterval ||
        a_lifetimeExpCount (p_subscription) := p_revLifetimeCount  ||
        a_LifetimeCounter (p_subscription) := p_revLifetimeCount   ||
        a_keepAliveExpCount (p_subscription) := p_revMaxKeepAlive  ||
        a_KeepAliveCounter (p_subscription) := p_revKeepAliveCount ||
        a_maxNotifsPerPublish (p_subscription) := p_maxNotificationsPerPublish ||
        a_MessageSent
        :((p_firstMsgSent = FALSE => a_MessageSent = a_MessageSent$0 -  {p_subscription}) &
          (p_firstMsgSent = TRUE  => a_MessageSent = a_MessageSent$0 \/ {p_subscription})) ||
        a_PublishingEnabled
        :((p_publishEnabled = TRUE  => a_PublishingEnabled = a_PublishingEnabled$0 \/ {p_subscription}) &
          (p_publishEnabled = FALSE => a_PublishingEnabled = a_PublishingEnabled$0 -  {p_subscription})) ||
        a_MoreNotifications
        :((p_moreNotifs = TRUE  => a_MoreNotifications = a_MoreNotifications$0 \/ {p_subscription}) &
          (p_moreNotifs = FALSE => a_MoreNotifications = a_MoreNotifications$0 -  {p_subscription})) ||
        a_SeqNum (p_subscription) := p_seqNumInit                              ||
        a_notifRepublishQueue (p_subscription) := p_republishQueue    ||
        a_monitoredItemQueue (p_subscription) := p_monitoredItemQueue ||
        a_publishTimer (p_subscription) := p_timerId ||

        a_session_nb_subscriptions,
        a_session_seq_priority,
        a_session_priority_subscriptions_queue,
        a_prio_queue_defined_subscriptions,
        s_prio_sub_to_iterate_it,
        s_prio_sub_iterated_it
        :(a_session_nb_subscriptions : t_session --> t_sub_idx_i &
            a_session_seq_priority : t_session --> iseq(t_priority) &
            a_session_priority_subscriptions_queue : t_session --> (t_prio_idx +-> t_prioritySubQueue) &
            a_prio_queue_defined_subscriptions : t_prioritySubQueue <-> t_subscription &

            s_prio_sub_to_iterate_it <: t_subscription_i &
            s_prio_sub_to_iterate_it <: t_subscription &
            s_prio_sub_iterated_it <: t_subscription_i &
            s_prio_sub_iterated_it <: t_subscription &
            s_prio_sub_to_iterate_it /\ s_prio_sub_iterated_it = {} &
            card(s_prio_sub_to_iterate_it \/ s_prio_sub_iterated_it) = nb_subscriptions_it &

            a_session_nb_subscriptions = a_session_nb_subscriptions$0 <+ {p_session |-> a_session_nb_subscriptions$0(p_session) + 1} &

            !session.(session : t_session
                =>
                dom(a_session_priority_subscriptions_queue(session)) = dom(a_session_seq_priority(session)))
        )
    END
    ;

    delete_subscription (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        s_subscription          := s_subscription - {p_subscription}            ||
        a_subscription_session  := {p_subscription} <<| a_subscription_session  ||
        a_publishInterval       := {p_subscription} <<| a_publishInterval       ||
        a_lifetimeExpCount      := {p_subscription} <<| a_lifetimeExpCount      ||
        a_keepAliveExpCount     := {p_subscription} <<| a_keepAliveExpCount     ||
        a_maxNotifsPerPublish   := {p_subscription} <<| a_maxNotifsPerPublish   ||
        a_subscription_state    := {p_subscription} <<| a_subscription_state    ||
        a_MoreNotifications     := a_MoreNotifications - {p_subscription}       ||
        a_LifetimeCounter       := {p_subscription} <<| a_LifetimeCounter       ||
        a_KeepAliveCounter      := {p_subscription} <<| a_KeepAliveCounter      ||
        a_MessageSent           := a_MessageSent - {p_subscription}             ||
        a_PublishingEnabled     := a_PublishingEnabled - {p_subscription}       ||
        a_SeqNum                := {p_subscription} <<| a_SeqNum                ||
        a_notifRepublishQueue   := {p_subscription} <<| a_notifRepublishQueue   ||
        a_monitoredItemQueue    := {p_subscription} <<| a_monitoredItemQueue    ||
        a_publishTimer          := {p_subscription} <<| a_publishTimer          ||
        a_subscription_priority := {p_subscription} <<| a_subscription_priority ||

        a_session_nb_subscriptions :(a_session_nb_subscriptions : t_session --> t_sub_idx_i &
            (a_session_nb_subscriptions$0(a_subscription_session(p_subscription)) >= 1
            =>
            a_session_nb_subscriptions = a_session_nb_subscriptions$0
                <+ {a_subscription_session(p_subscription) |-> a_session_nb_subscriptions$0(a_subscription_session(p_subscription)) - 1}) &
            (a_session_nb_subscriptions$0(a_subscription_session(p_subscription)) < 1
            =>
            a_session_nb_subscriptions = a_session_nb_subscriptions$0)) ||

        a_session_seq_priority,
        a_session_priority_subscriptions_queue,
        a_prio_queue_defined_subscriptions
        :(a_session_seq_priority : t_session --> iseq(t_priority) &
          a_session_priority_subscriptions_queue : t_session --> (t_prio_idx +-> t_prioritySubQueue) &
          a_prio_queue_defined_subscriptions : t_prioritySubQueue <-> t_subscription &
          !session.(session : t_session
                    =>
                    dom(a_session_priority_subscriptions_queue(session)) = dom(a_session_seq_priority(session)) &
                    card(dom(a_session_seq_priority(session))) <= card(dom(a_session_seq_priority$0(session))))
        )
    END
    ;

    p_priority <-- get_subscription_priority(p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        p_priority := a_subscription_priority (p_subscription)
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

    set_subscription_state (p_subscription, p_state, p_transition) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_subscription_state) &
        p_state : t_subscriptionState_i &
        p_state : t_subscriptionState &
        p_transition : NAT
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
        p_subscription : s_subscription &
        p_moreNotifs : BOOL
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

    reset_session_publishRequestQueue (p_session) =
    PRE
        p_session : t_session_i &
        p_session : ran(a_subscription_session)
    THEN
        a_publishRequestQueue := {p_session} <<| a_publishRequestQueue
    END
    ;

    set_session_publishRequestQueue (p_session, p_publishReqQueue) =
    PRE
        p_session : t_session_i &
        p_session : ran(a_subscription_session) &
        p_publishReqQueue : t_publishReqQueue_i &
        p_publishReqQueue : t_publishReqQueue
    THEN
        a_publishRequestQueue(p_session) := p_publishReqQueue
    END
    ;

    p_bres, p_publishReqQueue <-- get_session_publishRequestQueue (p_session) =
    PRE
        p_session : t_session_i &
        p_session : ran(a_subscription_session)
    THEN
        p_bres, p_publishReqQueue
        :(p_bres : BOOL &
          p_publishReqQueue : t_publishReqQueue_i &
          p_bres = bool(p_session : dom(a_publishRequestQueue)) &
          (p_bres = TRUE =>
              p_publishReqQueue = a_publishRequestQueue(p_session))
        )
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
    ;

    log_subscription_transition (p_subscription, p_old_state, p_new_state, p_transition) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_old_state : t_subscriptionState_i &
        p_new_state : t_subscriptionState_i &
        p_transition : NAT
    THEN
        skip
    END
    ;

    log_subscription_request_received (p_subscription, p_transition) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_transition : NAT
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
