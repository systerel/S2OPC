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
    subscription_core

SEES
    constants,
    constants_statuscodes_bs,
    message_out_bs,
    request_handle_bs,
    message_in_bs

DEFINITIONS
    d_variables ==
    s_subscription,
    s_monitoredItemPointer,
    s_monitoredItemId,
    a_monitoredItemId,
    a_monitoredItemSubs,
    a_monitoredItemNodeId,
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
    a_publishTimer,
    a_nodeToMonitoredItemQueue,
    /* Current valid publish response message from queue */
    publish_response_message,
    /* Notification message content */
    notification_message,
    notification_message_seq_num,
    /* Queues content */
    /* publish req queue */
    s_publishReqQueue,
    a_publish_responses,
    validPublishReqQueued,
    /* notification republish queue */
    s_republishQueue,
    a_republish_notification,
    a_notification_seq_num,
    notification_republish_iter_queue,
    s_to_iterate_sub_seq_num,
    s_iterated_sub_seq_num,

    /* notif queue */
    s_notificationQueue,
    a_monitored_item_notifications,
    /* mi queues */
    s_monitoredItemQueue,
    a_defined_monitored_items,
    /* mi queue iterator */
    mi_queue_it,
    mi_iterator_it,
    nb_monitored_items_it,
    s_mi_to_iterate_it,
    s_mi_iterated_it
    ;

    d_init ==
    s_subscription := {}        ||
    a_subscription_session := {}||
    a_publishInterval := {}     ||
    a_lifetimeExpCount := {}    ||
    a_keepAliveExpCount := {}   ||
    a_maxNotifsPerPublish := {} ||

    a_subscription_state := {}    ||

    a_MoreNotifications := {}     ||
    a_LifetimeCounter := {}       ||
    a_KeepAliveCounter := {}      ||
    a_MessageSent := {}           ||
    a_PublishingEnabled := {}     ||
    a_SeqNum            := {}     ||

    a_pendingNotificationQueue := {} ||
    a_publishRequestQueue := {} ||
    a_notifRepublishQueue := {} ||
    a_monitoredItemQueue := {} ||
    a_publishTimer := {} ||
    a_nodeToMonitoredItemQueue := {} ||

    s_monitoredItemPointer := {} ||
    s_monitoredItemId := {} ||
    a_monitoredItemId := {} ||
    a_monitoredItemSubs := {} ||
    a_monitoredItemNodeId := {} ||
    /* Publish response and notification message content */
    publish_response_message := c_msg_indet ||
    notification_message := c_notif_msg_indet ||
    notification_message_seq_num := c_sub_seq_num_indet ||

    /* Queues content */

    s_publishReqQueue := {} ||
    a_publish_responses := {} ||
    validPublishReqQueued := FALSE ||

    s_republishQueue := {} ||
    a_republish_notification := {} ||
    a_notification_seq_num := {} ||
    notification_republish_iter_queue := c_notifRepublishQueue_indet ||
    s_to_iterate_sub_seq_num :(s_to_iterate_sub_seq_num = {}) ||
    s_iterated_sub_seq_num :(s_iterated_sub_seq_num = {}) ||

    s_notificationQueue := {} ||
    a_monitored_item_notifications := {} ||
    s_monitoredItemQueue := {} ||
    a_defined_monitored_items := {} ||
    mi_queue_it := c_monitoredItemQueue_indet ||
    mi_iterator_it := c_monitoredItemQueueIterator_indet ||
    nb_monitored_items_it := 0 ||
    s_mi_to_iterate_it :(s_mi_to_iterate_it = {}) ||
    s_mi_iterated_it :(s_mi_iterated_it = {})
    ;

    d_typing ==
    s_subscription <: t_subscription &

    /* Subscription configuration record */
    a_subscription_session : s_subscription >-> t_session & // a subscription shall be associated to a session (=> 1 subscription / 1 session)
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
    /* a_SubscriptionAssignedToClient <: s_subscription */             /* Not really a state machine variable: local variable or function */

    /* Subscription Implementation variables */
    s_monitoredItemPointer <: t_monitoredItemPointer &
    s_monitoredItemId <: t_monitoredItemId &
    a_monitoredItemId : s_monitoredItemPointer >->> s_monitoredItemId &
    a_monitoredItemSubs : s_monitoredItemPointer --> t_subscription &
    a_monitoredItemNodeId : s_monitoredItemPointer --> t_NodeId &

    s_notificationQueue <: t_notificationQueue &
    a_pendingNotificationQueue : s_subscription --> s_notificationQueue &
    /* a_retransmissionQueue : s_subscription --> s_notificationMsgQueue */ /* not implemented */
    s_monitoredItemQueue <: t_monitoredItemQueue &
    a_monitoredItemQueue : s_subscription --> s_monitoredItemQueue &
    a_nodeToMonitoredItemQueue : t_NodeId --> s_monitoredItemQueue &

    a_publishTimer : s_subscription --> t_timer_id &

    /* Publish response and notification message content */
    publish_response_message : t_msg_i &
    notification_message : t_notif_msg_i &
    notification_message_seq_num : t_sub_seq_num_i &

    /* (Session) Implementation variables */

    /* Note: kept on subscription for now since 1 subs / session + no transfer. Otherwise could be cleared on session close ! */
    s_publishReqQueue <: t_publishReqQueue &
    a_publishRequestQueue :  s_subscription --> s_publishReqQueue &
    a_notifRepublishQueue : s_subscription --> t_notifRepublishQueue &
    notification_republish_iter_queue : t_notifRepublishQueue &
    s_to_iterate_sub_seq_num <: t_sub_seq_num &
    s_iterated_sub_seq_num <: t_sub_seq_num &
    s_to_iterate_sub_seq_num /\ s_iterated_sub_seq_num = {} &

    /* Queues variables */
    /* PublishRequest queue content: */
    a_publish_responses : t_msg +-> s_publishReqQueue &
    validPublishReqQueued : BOOL &

    /* Notification republish queue content */
    s_republishQueue <: t_notifRepublishQueue &
    a_republish_notification : t_notif_msg +-> t_notifRepublishQueue &
    a_notification_seq_num : t_notif_msg --> t_sub_seq_num &

    a_monitored_item_notifications : s_notificationQueue <-> s_monitoredItemPointer &
    a_defined_monitored_items : s_monitoredItemQueue <-> s_monitoredItemPointer &
    mi_queue_it : t_monitoredItemQueue_i &
    mi_iterator_it : t_monitoredItemQueueIterator_i &
    nb_monitored_items_it : NAT &
    s_mi_to_iterate_it <: t_monitoredItemPointer_i &
    s_mi_to_iterate_it <: t_monitoredItemPointer &
    s_mi_iterated_it <: t_monitoredItemPointer_i &
    s_mi_iterated_it <: t_monitoredItemPointer &
    s_mi_to_iterate_it /\ s_mi_iterated_it = {} &
    card(s_mi_to_iterate_it \/ s_mi_iterated_it) = nb_monitored_items_it
    ;

    /* Dequeuing invalid publish requests and check for a valid request still enqueued */
    d_check_for_valid_publish_req_queue(p_subscription, l_PublishingReqQueued, l_new_publish_response) ==
        l_new_publish_response : t_msg +-> t_publishReqQueue &
        l_new_publish_response <: a_publish_responses &
        l_new_publish_response |>> {a_publishRequestQueue(p_subscription)} =
        a_publish_responses |>> {a_publishRequestQueue(p_subscription)} &
        l_new_publish_response |> {a_publishRequestQueue(p_subscription)} <:
        a_publish_responses |> {a_publishRequestQueue(p_subscription)} &
        l_PublishingReqQueued = bool (l_new_publish_response~[{a_publishRequestQueue(p_subscription)}] /= {})
    ;

    /* Dequeuing a publish request context + response */
    d_dequeue_publish_req (p_subscription, p_publish_resp_msg) ==
    LET l_publishQueue BE
        l_publishQueue = a_publishRequestQueue(p_subscription)
    IN
        ANY l_resp_msg WHERE
            l_resp_msg : t_msg_i &
            l_resp_msg : t_msg &
            l_resp_msg |-> l_publishQueue : a_publish_responses
        THEN
            p_publish_resp_msg := l_resp_msg ||
            a_publish_responses, publish_response_message
            :(a_publish_responses : t_msg +-> s_publishReqQueue &
              a_publish_responses = a_publish_responses$0 - {l_resp_msg |-> l_publishQueue} &
              publish_response_message = l_resp_msg
            )
        END
    END
    ;

    d_publish_timeout_no_msg_sent ==
    p_msg_to_send := FALSE ||
    p_session := c_session_indet ||
    p_req_handle :: t_server_request_handle_i ||
    p_req_context := c_request_context_indet ||
    p_publish_resp_msg := c_msg_indet
    ;

    d_publish_timeout_no_msg_sent_setMoreNotifs ==
    d_publish_timeout_no_msg_sent ||
    p_moreNotifs := FALSE ||
    p_msg_sc := c_StatusCode_indet
    ;

    d_publish_request_notif_msg_sent_dequeuing_notifications (p_subscription, p_msg_sc, p_moreNotifs) ==
    LET l_notifQueue, l_republishQueue BE
        l_notifQueue = a_pendingNotificationQueue(p_subscription) &
        l_republishQueue = a_notifRepublishQueue(p_subscription)
    IN
    a_MessageSent := a_MessageSent \/ {p_subscription} ||
    p_msg_sc, a_SeqNum, notification_message, notification_message_seq_num,
    a_monitored_item_notifications, a_MoreNotifications, p_moreNotifs,
    a_republish_notification, a_notification_seq_num
    :(p_msg_sc : t_StatusCode_i & p_msg_sc : t_StatusCode &
      p_msg_sc : {e_sc_ok, e_sc_bad_out_of_memory} &
      a_SeqNum : s_subscription --> t_sub_seq_num &
      notification_message : t_notif_msg_i &
      notification_message_seq_num : t_sub_seq_num_i &
      a_monitored_item_notifications : s_notificationQueue <-> s_monitoredItemPointer &
      a_monitored_item_notifications <: a_monitored_item_notifications$0 &
      {l_notifQueue} <<| a_monitored_item_notifications = {l_notifQueue} <<| a_monitored_item_notifications$0 &
      p_moreNotifs : BOOL &
      a_republish_notification : t_notif_msg +-> t_notifRepublishQueue &
      a_republish_notification |>> {l_republishQueue} = a_republish_notification$0 |>> {l_republishQueue} &
      a_notification_seq_num : t_notif_msg --> t_sub_seq_num &
      // OK CASE
      (p_msg_sc = e_sc_ok =>
       {p_subscription} <<| a_SeqNum = {p_subscription} <<| a_SeqNum$0 &
       a_SeqNum (p_subscription) /= a_SeqNum$0 (p_subscription) &
       notification_message : t_notif_msg &
       notification_message_seq_num = a_SeqNum$0 (p_subscription) & // Consume precedent sequence number
       /* Add notification msg in republish queue and record seq num associated (with possibly discarded old element)
          => only indicate modified and containing the new notif here */
       (notification_message |-> l_republishQueue) : (a_republish_notification |> {l_republishQueue}) &
       a_notification_seq_num = a_notification_seq_num$0 \/ {notification_message |-> notification_message_seq_num} &
       // Update moreNotification
       ({l_notifQueue} <| a_monitored_item_notifications /= {} =>
         a_MoreNotifications = a_MoreNotifications$0 \/ {p_subscription} &
         p_moreNotifs = TRUE
       ) &
       ({l_notifQueue} <| a_monitored_item_notifications = {} =>
           a_MoreNotifications = a_MoreNotifications$0 - {p_subscription} &
           p_moreNotifs = FALSE
       )
      ) &
      // OUT OF MEMORY CASE
      (p_msg_sc = e_sc_bad_out_of_memory =>
       a_SeqNum = a_SeqNum$0 &
       a_MoreNotifications = a_MoreNotifications$0 \/ {p_subscription} &
       p_moreNotifs = TRUE &
       a_republish_notification = a_republish_notification$0 &
       a_notification_seq_num = a_notification_seq_num$0
      )
    )
    END
    ;

    d_publish_timeout_notif_msg_sent_dequeuing_notifications (p_subscription, p_msg_sc, p_moreNotifs,
                                                              p_session, p_req_handle, p_req_ctx, p_msg_to_send) ==
    p_session, p_req_handle, p_req_context
    :(p_session : t_session_i & p_session = a_subscription_session (p_subscription) &
      p_req_handle : t_server_request_handle_i &
      p_req_context : t_request_context_i & p_req_context : t_request_context) ||
    p_msg_to_send := TRUE ||
    d_publish_request_notif_msg_sent_dequeuing_notifications (p_subscription, p_msg_sc, p_moreNotifs)
    ;

    d_publish_timeout_keepAlive_msg_sent ==
    a_MessageSent := a_MessageSent \/ {p_subscription} ||
    p_msg_to_send := TRUE ||
    p_moreNotifs := FALSE ||
    p_msg_sc := e_sc_ok ||
    p_session, p_req_handle, p_req_context, notification_message, notification_message_seq_num
    :(p_session : t_session_i & p_session = a_subscription_session (p_subscription) &
      p_req_handle : t_server_request_handle_i &
      p_req_context : t_request_context_i & p_req_context : t_request_context &
      notification_message : t_notif_msg_i & notification_message : t_notif_msg &
      notification_message_seq_num = a_SeqNum (p_subscription) // Use next sequence number to be sent in keep alive
    ) /* Note: no update of republish notification queue in keepAlive case */
    ;

    d_publish_timeout_state_changed_msg_sent ==
    a_MessageSent := a_MessageSent \/ {p_subscription} ||
    p_msg_to_send := TRUE ||
    p_session, p_req_handle, p_req_context,
    notification_message, notification_message_seq_num, a_SeqNum,
    a_republish_notification, a_notification_seq_num
    :(p_session : t_session_i & p_session = a_subscription_session (p_subscription) &
        p_req_handle : t_request_handle_i & p_req_handle : s_request_handle &
        p_req_context : t_request_context_i & p_req_context : t_request_context &
        notification_message : t_notif_msg_i & notification_message : t_notif_msg &
        notification_message_seq_num = a_SeqNum$0 (p_subscription) &
        {l_subscription} <<| a_SeqNum = {l_subscription} <<| a_SeqNum$0 &
        a_SeqNum (l_subscription) /= a_SeqNum$0 (l_subscription) &
        a_republish_notification |> {l_republishQueue} =
          a_republish_notification$0 |> {l_republishQueue}  \/ {notification_message |-> l_republishQueue} &
        a_notification_seq_num = a_notification_seq_num$0 \/ {notification_message |-> notification_message_seq_num}
    )

ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typing

INITIALISATION
    d_init

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

    /* To be used if is_session_still_activated (p_subscription) == FALSE
       since requests will not be valid anymore on another SC */
    empty_session_publish_requests (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        a_publish_responses := a_publish_responses |>> {a_publishRequestQueue (p_subscription)} ||
        a_republish_notification := a_republish_notification |>> {a_notifRepublishQueue(p_subscription)} ||
        a_notification_seq_num :=
          dom(a_republish_notification |> {a_notifRepublishQueue(p_subscription)}) <<| a_notification_seq_num
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

    p_republishQueue <-- get_subscription_notifRepublishQueue (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : dom(a_notifRepublishQueue)
    THEN
        p_republishQueue :(p_republishQueue : t_notifRepublishQueue_i &
                           p_republishQueue = a_notifRepublishQueue(p_subscription))
    END
    ;

    revisedPublishInterval, revisedLifetimeCount, revisedMaxKeepAlive, revisedMaxNotificationsPerPublish
      <-- compute_create_subscription_revised_params (p_reqPublishInterval,
                                                      p_reqLifetimeCount,
                                                      p_reqMaxKeepAlive,
                                                      p_maxNotificationsPerPublish) =
    PRE
        p_reqPublishInterval : t_opcua_duration_i &
        p_reqLifetimeCount : t_counter &
        p_reqMaxKeepAlive : t_counter &
        p_maxNotificationsPerPublish : t_counter
    THEN
        revisedPublishInterval,
            revisedLifetimeCount,
            revisedMaxKeepAlive,
            revisedMaxNotificationsPerPublish
            :(revisedPublishInterval : t_opcua_duration_i & revisedPublishInterval /= c_opcua_duration_indet &
                revisedLifetimeCount : t_counter &
                revisedLifetimeCount > 0 &
                revisedMaxKeepAlive : t_counter &
                revisedMaxKeepAlive > 0 &
                revisedLifetimeCount >= 3 * revisedMaxKeepAlive & /* part 4 table 86 */
                revisedMaxNotificationsPerPublish : t_counter &
                revisedMaxNotificationsPerPublish > 0
            )
    END
    ;

    StatusCode_service, subscription <-- create_subscription (p_session,
                                                              p_revPublishInterval,
                                                              p_revLifetimeCount,
                                                              p_revMaxKeepAlive,
                                                              p_maxNotificationsPerPublish,
                                                              p_publishEnabled) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_session /: ran(a_subscription_session) & /* session is not part of any subscription (=> 1 subscription / session) */
        p_revPublishInterval : t_opcua_duration_i &
        p_revLifetimeCount : t_counter &
        p_revLifetimeCount > 0 &
        p_revMaxKeepAlive : t_counter &
        p_revMaxKeepAlive > 0 &
        p_revLifetimeCount >= 3 * p_revMaxKeepAlive & /* part 4 table 86 */
        p_maxNotificationsPerPublish : t_counter &
        p_publishEnabled = TRUE /* => no subscription modification for now => refuse publishDisabled */
    THEN
        /* State table evaluation case (CreateSubscription event: #1, #2, #3) */
        IF t_subscription = s_subscription
        THEN
            StatusCode_service := e_sc_bad_too_many_subscriptions ||
            subscription := c_subscription_indet
        ELSE
            StatusCode_service, subscription,
            s_publishReqQueue, s_notificationQueue, s_republishQueue, s_monitoredItemQueue,
            /* case of e_sc_ok */
            s_subscription, a_subscription_state, a_subscription_session,
            a_publishInterval, a_lifetimeExpCount, a_LifetimeCounter,
            a_keepAliveExpCount, a_KeepAliveCounter,
            a_maxNotifsPerPublish, a_MessageSent, a_PublishingEnabled, a_SeqNum,
            a_pendingNotificationQueue, a_publishRequestQueue, a_notifRepublishQueue,
            a_monitoredItemQueue, a_publishTimer,
            /* case of e_sc_bad_out_of_memory */
            a_monitored_item_notifications,
            a_publish_responses,
            a_republish_notification, a_notification_seq_num,
            a_defined_monitored_items:(
                d_typing &

                StatusCode_service : t_StatusCode_i &
                subscription : t_subscription_i &

                (t_monitoredItemQueue /= s_monitoredItemQueue$0 &
                 t_notificationQueue /= s_notificationQueue$0 &
                 t_publishReqQueue /= s_publishReqQueue$0 &
                 t_notifRepublishQueue /= s_republishQueue$0
                =>
                    /* Transition #1 + #3 */
                    (   StatusCode_service = e_sc_ok &
                        subscription : t_subscription - s_subscription$0 &
                        s_subscription = s_subscription$0 \/ {subscription}
                    ) or
                    /* Transition #1 + #2 */
                    /* to deal with indeterminated timer creation for a_publishTimer */
                    (   StatusCode_service = e_sc_bad_out_of_memory &
                        subscription = c_subscription_indet &
                        s_subscription = s_subscription$0
                    )
                ) &
                /* Transition #1 + #2 */
                (not(t_monitoredItemQueue /= s_monitoredItemQueue$0 &
                     t_notificationQueue /= s_notificationQueue$0 &
                     t_publishReqQueue /= s_publishReqQueue$0 &
                     t_notifRepublishQueue /= s_republishQueue$0)
                =>
                    StatusCode_service = e_sc_bad_out_of_memory &
                    subscription = c_subscription_indet &
                    s_subscription = s_subscription$0
                )
            )
        END
    END
    ;


    modify_subscription (p_subscription,
                         p_revPublishInterval,
                         p_revLifetimeCount,
                         p_revMaxKeepAlive,
                         p_revMaxNotifPerPublish) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_revPublishInterval : t_opcua_duration_i &
        p_revLifetimeCount : t_counter &
        p_revLifetimeCount > 0 &
        p_revMaxKeepAlive : t_counter &
        p_revMaxKeepAlive > 0 &
        p_revLifetimeCount >= 3 * p_revMaxKeepAlive & /* part 4 table 86 */
        p_revMaxNotifPerPublish : t_counter
    THEN
        a_publishInterval (p_subscription) := p_revPublishInterval ||
        a_lifetimeExpCount (p_subscription) := p_revLifetimeCount  ||
        a_keepAliveExpCount (p_subscription) := p_revMaxKeepAlive  ||
        a_maxNotifsPerPublish (p_subscription) := p_revMaxNotifPerPublish
    END
    ;

    close_subscription (p_subscription) =
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

        /* Queues emptied : publish responses should be sent if remaining */
        a_publish_responses := a_publish_responses |>> {a_publishRequestQueue (p_subscription)} ||
        a_monitored_item_notifications := {a_pendingNotificationQueue (p_subscription)} <<| a_monitored_item_notifications ||

        /* Queues deleted */
        s_notificationQueue := s_notificationQueue - {a_pendingNotificationQueue (p_subscription)} ||
        a_pendingNotificationQueue := {p_subscription} <<| a_pendingNotificationQueue ||

        s_publishReqQueue := s_publishReqQueue - {a_publishRequestQueue(p_subscription)} ||
        a_publishRequestQueue := {p_subscription} <<| a_publishRequestQueue ||

        /* Republish queue deleted */
        a_republish_notification := a_republish_notification |>> {a_notifRepublishQueue(p_subscription)} ||
        a_notification_seq_num :=
          dom(a_republish_notification |> {a_notifRepublishQueue(p_subscription)}) <<| a_notification_seq_num ||

        s_republishQueue := s_republishQueue - {a_notifRepublishQueue(p_subscription)} ||
        a_notifRepublishQueue := {p_subscription} <<| a_notifRepublishQueue ||

        /* Remove monitoredItems */
        LET s_removedMonitoredItemPointer BE
            s_removedMonitoredItemPointer = a_defined_monitored_items[{a_monitoredItemQueue (p_subscription)}]
        IN
            s_monitoredItemPointer := s_monitoredItemPointer - s_removedMonitoredItemPointer ||
            s_monitoredItemId := s_monitoredItemId - a_monitoredItemId[s_removedMonitoredItemPointer] ||
            a_monitoredItemId := a_monitoredItemId - (s_removedMonitoredItemPointer <| a_monitoredItemId) ||
            a_monitoredItemSubs := s_removedMonitoredItemPointer <<| a_monitoredItemSubs ||
            a_monitoredItemNodeId := s_removedMonitoredItemPointer <<| a_monitoredItemNodeId ||

            s_monitoredItemQueue := s_monitoredItemQueue - {a_monitoredItemQueue(p_subscription)} ||
            a_monitoredItemQueue := {p_subscription} <<| a_monitoredItemQueue ||

            /* Remove subscription monitoredItem queue and all removed monitored items from other queues (i.e.:nodeId to mi queues) */
            a_defined_monitored_items :=
             {a_monitoredItemQueue (p_subscription)}
              <<| a_defined_monitored_items |>>
             s_removedMonitoredItemPointer
        END ||
        a_publishTimer := {p_subscription} <<| a_publishTimer
    END
    ;

    is_valid_seq_num <-- subscription_ack_notif_msg (p_sub, p_seq_num) =
    PRE
        p_sub : t_subscription_i &
        p_sub : s_subscription &
        p_seq_num : t_sub_seq_num_i
    THEN
        LET republishQueue, notifMsgs, seqNums BE
            republishQueue = a_notifRepublishQueue (p_sub) &
            notifMsgs = a_republish_notification~[{republishQueue}] &
            seqNums = a_notification_seq_num[notifMsgs]
        IN
            IF p_seq_num : seqNums
            THEN
                is_valid_seq_num := TRUE ||
                ANY notifMsg WHERE
                    notifMsg : t_notif_msg_i & notifMsg : t_notif_msg &
                    notifMsg : a_republish_notification~[{republishQueue}] &
                    a_notification_seq_num(notifMsg) = p_seq_num
                THEN
                    a_republish_notification := a_republish_notification - {notifMsg |-> republishQueue}  ||
                    a_notification_seq_num := a_notification_seq_num - {notifMsg |-> p_seq_num}
                END
            ELSE
                is_valid_seq_num := FALSE
            END
        END
    END
    ;

    StatusCode_service, async_resp_msg, subscription, moreNotifs
      <-- receive_publish_request (p_session, p_req_exp_time, p_req_handle, p_req_ctx, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_session : ran(a_subscription_session) &
        p_req_exp_time : t_timeref_i &
        p_req_exp_time : t_timeref &
        p_req_handle : t_server_request_handle_i &
        p_req_ctx : t_request_context_i &
        p_req_ctx : t_request_context &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp
    THEN
        /* State table evaluation case (PublishRequest event: #4, #5, #10, #11, #13) */
        LET l_subscription, l_PublishingEnabled, l_MoreNotifications, l_NotificationAvailable BE
            l_subscription = a_subscription_session~(p_session) &
            l_PublishingEnabled = bool (l_subscription : a_PublishingEnabled) &
            l_MoreNotifications = bool (l_subscription : a_MoreNotifications) &
            l_NotificationAvailable = bool (a_monitored_item_notifications [{a_pendingNotificationQueue(l_subscription)}] /= {})
        IN
            /* #4 transition */
            IF a_subscription_state (l_subscription) = e_subscriptionState_normal &
               (l_PublishingEnabled = FALSE
                or
                (l_PublishingEnabled = TRUE & l_MoreNotifications = FALSE)
               )
            THEN
                CHOICE
                    /* Enqueue publish request (request data + response): */
                    IF card(a_publish_responses~[{a_publishRequestQueue (l_subscription)}]) = k_n_publishRequestPerSub_max
                    THEN
                        /* + Discard oldest publish request if maximum publish requests reached */
                        ANY l_old_resp_msg WHERE
                            l_old_resp_msg : t_msg_i &
                            (l_old_resp_msg |-> a_publishRequestQueue (l_subscription)) : a_publish_responses
                        THEN
                            a_publish_responses := a_publish_responses
                             - {l_old_resp_msg |-> a_publishRequestQueue (l_subscription)}
                             \/ {p_resp_msg |-> a_publishRequestQueue (l_subscription)}
                        END
                    ELSE
                        a_publish_responses := a_publish_responses \/ {p_resp_msg |-> a_publishRequestQueue (l_subscription)}
                    END ||
                    StatusCode_service := e_sc_ok ||
                    async_resp_msg := TRUE ||
                    subscription := c_subscription_indet ||
                    moreNotifs := FALSE
                OR
                    StatusCode_service := e_sc_bad_out_of_memory ||
                    async_resp_msg := FALSE ||
                    subscription := c_subscription_indet ||
                    moreNotifs := FALSE
                END

            /* #5 transition */
            ELSIF a_subscription_state (l_subscription) = e_subscriptionState_normal &
                  l_PublishingEnabled = TRUE & l_MoreNotifications = TRUE
            THEN
                a_LifetimeCounter (l_subscription) := a_lifetimeExpCount (l_subscription) || /* reset lifetime counter */
                subscription := l_subscription ||
                async_resp_msg := FALSE ||
                d_publish_request_notif_msg_sent_dequeuing_notifications (l_subscription, StatusCode_service, moreNotifs)

            /* #10 transition */
            ELSIF a_subscription_state (l_subscription) = e_subscriptionState_late & l_PublishingEnabled = TRUE &
                (l_NotificationAvailable = TRUE or l_MoreNotifications = TRUE)
            THEN
                a_subscription_state (l_subscription) := e_subscriptionState_normal ||
                a_LifetimeCounter (l_subscription) := a_lifetimeExpCount (l_subscription) || /* reset lifetime counter */
                subscription := l_subscription ||
                async_resp_msg := FALSE ||
                /* return notifications (dequeue notifications) + update MoreNotification variable */
                d_publish_request_notif_msg_sent_dequeuing_notifications (l_subscription, StatusCode_service, moreNotifs)

            /* #11 transition */
            ELSIF a_subscription_state (l_subscription) = e_subscriptionState_late &
                (l_PublishingEnabled = FALSE
                 or
                 (l_PublishingEnabled = TRUE &
                  l_NotificationAvailable = FALSE &
                  l_MoreNotifications = FALSE)
                )
            THEN
                a_subscription_state (l_subscription) := e_subscriptionState_keepAlive ||
                a_MessageSent := a_MessageSent \/ {l_subscription} ||
                a_LifetimeCounter (l_subscription) := a_lifetimeExpCount (l_subscription) || /* reset lifetime counter */
                StatusCode_service := e_sc_ok ||
                async_resp_msg := FALSE ||
                subscription := l_subscription ||
                notification_message :(notification_message : t_notif_msg_i & notification_message : t_notif_msg) ||
                notification_message_seq_num := a_SeqNum (l_subscription) || /* KeepAlive => no seq num update */
                moreNotifs := FALSE

            /* #13 transition */
            ELSIF a_subscription_state (l_subscription) = e_subscriptionState_keepAlive
            THEN
                CHOICE
                    /* Enqueue publish request (request data + response): */
                    IF card(a_publish_responses~[{a_publishRequestQueue (l_subscription)}]) = k_n_publishRequestPerSub_max
                    THEN
                        /* + Discard oldest publish request if maximum publish requests reached */
                        ANY l_old_resp_msg WHERE
                            l_old_resp_msg : t_msg_i &
                            (l_old_resp_msg |-> a_publishRequestQueue (l_subscription)) : a_publish_responses
                        THEN
                            a_publish_responses := a_publish_responses
                             - {l_old_resp_msg |-> a_publishRequestQueue (l_subscription)}
                             \/ {p_resp_msg |-> a_publishRequestQueue (l_subscription)}
                        END
                    ELSE
                        a_publish_responses := a_publish_responses \/ {p_resp_msg |-> a_publishRequestQueue (l_subscription)}
                    END ||
                    StatusCode_service := e_sc_ok ||
                    async_resp_msg := TRUE ||
                    subscription := c_subscription_indet ||
                    moreNotifs := FALSE
                OR
                    StatusCode_service := e_sc_bad_out_of_memory ||
                    async_resp_msg := FALSE ||
                    subscription := c_subscription_indet ||
                    moreNotifs := FALSE
                END
            ELSE
                StatusCode_service := e_sc_bad_invalid_state ||
                async_resp_msg := FALSE ||
                subscription := c_subscription_indet ||
                moreNotifs := FALSE
            END
        END
    END
    ;

    revisedSamplingItv, revisedQueueSize <-- compute_create_monitored_item_revised_params (p_reqQueueSize) =
    PRE
        p_reqQueueSize : t_counter
    THEN
        revisedSamplingItv :: t_opcua_duration_i ||
        revisedQueueSize :: t_counter
    END
    ;

    StatusCode_service, monitoredItemPointer, monitoredItemId <-- create_monitored_item (p_subscription,
                                                                                         p_nid,
                                                                                         p_aid,
                                                                                         p_indexRange,
                                                                                         p_value,
                                                                                         p_valueSc,
                                                                                         p_val_ts_src,
                                                                                         p_val_ts_srv,
                                                                                         p_timestampToReturn,
                                                                                         p_monitoringMode,
                                                                                         p_clientHandle) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_nid : t_NodeId_i &
        p_nid : t_NodeId &
        p_aid : t_AttributeId_i &
        p_aid : t_AttributeId &
        p_indexRange : t_IndexRange_i &
        p_value : t_Variant_i & /* value may be indet if status code /= Good */
        p_valueSc : t_RawStatusCode &
        p_val_ts_src : t_Timestamp &
        p_val_ts_srv : t_Timestamp &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
        p_monitoringMode : t_monitoringMode_i &
        p_monitoringMode : t_monitoringMode &
        p_clientHandle : t_client_handle_i
    THEN
        IF t_monitoredItemPointer = s_monitoredItemPointer or
            t_monitoredItemId = s_monitoredItemId
        THEN
            /* We do not limit the number of MI in C code, so it means we do not have enough memory if it fails */
            StatusCode_service := e_sc_bad_out_of_memory ||
            monitoredItemPointer := c_monitoredItemPointer_indet ||
            monitoredItemId := c_monitoredItemId_indet
        ELSIF p_indexRange : t_IndexRange /* It means it is null or it can be parsed */
        THEN
            CHOICE
                ANY l_monitoredItem, l_monitoredItemId WHERE
                    l_monitoredItem : t_monitoredItemPointer - s_monitoredItemPointer &
                    l_monitoredItemId : t_monitoredItemId - s_monitoredItemId
                THEN
                    StatusCode_service := e_sc_ok ||
                    s_monitoredItemPointer := s_monitoredItemPointer \/ {l_monitoredItem} ||
                    s_monitoredItemId := s_monitoredItemId \/ {l_monitoredItemId} ||
                    a_monitoredItemId(l_monitoredItem) := l_monitoredItemId ||
                    a_monitoredItemSubs(l_monitoredItem) := p_subscription ||
                    a_monitoredItemNodeId(l_monitoredItem) := p_nid ||
                    monitoredItemPointer := l_monitoredItem ||
                    monitoredItemId := l_monitoredItemId ||
                    /* a notification is enqueued with current value of monitored item */
                    a_monitored_item_notifications :=
                    a_monitored_item_notifications \/ {a_pendingNotificationQueue(p_subscription) |-> l_monitoredItem} ||
                    /* add monitored item to both queues */
                    a_defined_monitored_items :=
                    a_defined_monitored_items \/ {a_monitoredItemQueue(p_subscription) |-> l_monitoredItem}
                                              \/ {a_nodeToMonitoredItemQueue(p_nid) |-> l_monitoredItem}
                END
            OR
                StatusCode_service := e_sc_bad_out_of_memory ||
                monitoredItemPointer := c_monitoredItemPointer_indet ||
                monitoredItemId := c_monitoredItemId_indet
            END
        ELSE
            StatusCode_service := e_sc_bad_index_range_invalid ||
            monitoredItemPointer := c_monitoredItemPointer_indet ||
            monitoredItemId := c_monitoredItemId_indet
        END
    END
    ;

    p_validPublishingReqQueued <-- server_subscription_core_check_valid_publish_req_queue (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        ANY l_PublishingReqQueued, l_new_publish_responses WHERE
            d_check_for_valid_publish_req_queue(p_subscription, l_PublishingReqQueued, l_new_publish_responses)
        THEN
            validPublishReqQueued := l_PublishingReqQueued ||
            p_validPublishingReqQueued := l_PublishingReqQueued ||
            a_publish_responses := l_new_publish_responses
        END
    END
    ;

    /* Transition #27 shall be evaluated prior to other possible transitions since lifeTimeCounter--
       never done in other transitions
    */
    p_close_sub, p_msg_to_send, p_session, p_publish_resp_msg, p_req_handle, p_req_context, p_validPubReqQueued
      <-- server_subscription_core_publish_timeout_check_lifetime (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        /* #27 transition */
        ANY l_PublishingReqQueued, l_new_publish_responses WHERE
            d_check_for_valid_publish_req_queue(p_subscription, l_PublishingReqQueued, l_new_publish_responses)
        THEN
            a_publish_responses := l_new_publish_responses ||
            validPublishReqQueued := l_PublishingReqQueued ||
            p_validPubReqQueued := l_PublishingReqQueued ||
            IF l_PublishingReqQueued = FALSE
            THEN
                IF a_LifetimeCounter (p_subscription) > 1
                THEN
                    /* Decrements lifetime counter */
                    a_LifetimeCounter (p_subscription) := a_LifetimeCounter (p_subscription) - 1 ||
                    p_close_sub := FALSE ||
                    d_publish_timeout_no_msg_sent
                ELSE
                    /* Lifetime counter expired => send issueStateChanged and close subscription */
                    a_LifetimeCounter (p_subscription) := 0 ||
                    /* How to send a status state changed as stated in OpcUa part 4 if no publish request available ?*/
                    /*d_dequeue_publish_req (p_subscription, ...) ||
                     d_publish_timeout_state_changed_msg_sent */
                    p_close_sub := TRUE ||
                    d_publish_timeout_no_msg_sent
                END
            ELSE
                p_close_sub := FALSE ||
                d_publish_timeout_no_msg_sent
            END
        END
    END
    ;

    p_msg_to_send, p_msg_sc, p_session, p_publish_resp_msg, p_req_handle, p_req_context, p_moreNotifs
      <-- server_subscription_core_publish_timeout (p_subscription, p_validPublishReqQueued) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_validPublishReqQueued : BOOL &
        p_validPublishReqQueued = validPublishReqQueued
    THEN
        /* State table evaluation case (Publish Timer Expires: #6, #7, #8, #9, #12, #14, #15, #16, #17 (+ #27 => see previous operation)) */
        LET l_PublishingEnabled, l_NotificationAvailable, l_MessageSent BE
            l_PublishingEnabled = bool (p_subscription : a_PublishingEnabled) &
            l_NotificationAvailable = bool (a_monitored_item_notifications [{a_pendingNotificationQueue(p_subscription)}] /= {}) &
            l_MessageSent = bool (p_subscription : a_MessageSent)
        IN
            /* #6 transition */
            IF a_subscription_state (p_subscription) = e_subscriptionState_normal &
               p_validPublishReqQueued = TRUE &
               l_PublishingEnabled = TRUE &
               l_NotificationAvailable = TRUE
            THEN
                /* reset lifetime counter */
                a_LifetimeCounter (p_subscription) := a_lifetimeExpCount (p_subscription) ||
                d_dequeue_publish_req (p_subscription, p_publish_resp_msg) ||
                /* return notifications (dequeue notifications) + update MoreNotification variable */
                d_publish_timeout_notif_msg_sent_dequeuing_notifications (p_subscription, p_msg_sc, p_moreNotifs,
                                                                          p_session, p_req_handle, p_req_context,
                                                                          p_msg_to_send)

            /* #7 transition */
            ELSIF a_subscription_state (p_subscription) = e_subscriptionState_normal &
                  p_validPublishReqQueued = TRUE &
                  l_MessageSent = FALSE &
                  (l_PublishingEnabled = FALSE
                   or
                   (l_PublishingEnabled = TRUE &
                    l_NotificationAvailable = FALSE)
                  )
            THEN
                /* reset lifetime counter */
                a_LifetimeCounter (p_subscription) := a_lifetimeExpCount (p_subscription) ||
                d_dequeue_publish_req (p_subscription, p_publish_resp_msg) ||
                d_publish_timeout_keepAlive_msg_sent

            /* #8 transition */
            ELSIF a_subscription_state (p_subscription) = e_subscriptionState_normal &
                  p_validPublishReqQueued = FALSE &
                  (l_MessageSent = FALSE
                   or
                   (l_PublishingEnabled = TRUE &
                    l_NotificationAvailable = TRUE)
                  )
            THEN
                a_subscription_state (p_subscription) := e_subscriptionState_late ||
                d_publish_timeout_no_msg_sent_setMoreNotifs

            /* #9 transition */
            ELSIF a_subscription_state (p_subscription) = e_subscriptionState_normal &
                  l_MessageSent = TRUE &
                  (l_PublishingEnabled = FALSE
                   or
                   (l_PublishingEnabled = TRUE &
                    l_NotificationAvailable = FALSE)
                  )
            THEN
                a_subscription_state (p_subscription) := e_subscriptionState_keepAlive ||
                /* reset keepAlive counter */
                a_KeepAliveCounter (p_subscription) := a_keepAliveExpCount (p_subscription) ||
                d_publish_timeout_no_msg_sent_setMoreNotifs

            /* #12 transition */
            ELSIF a_subscription_state (p_subscription) = e_subscriptionState_late
            THEN
                d_publish_timeout_no_msg_sent_setMoreNotifs

            /* #14 transition */
            ELSIF a_subscription_state (p_subscription) = e_subscriptionState_keepAlive &
                  l_PublishingEnabled = TRUE &
                  l_NotificationAvailable = TRUE &
                  p_validPublishReqQueued = TRUE
            THEN
                a_subscription_state (p_subscription) := e_subscriptionState_normal ||
                /* reset lifetime counter */
                a_LifetimeCounter (p_subscription) := a_lifetimeExpCount (p_subscription) ||
                d_dequeue_publish_req (p_subscription, p_publish_resp_msg) ||
                /* return notifications (dequeue notifications) + update MoreNotification variable */
                d_publish_timeout_notif_msg_sent_dequeuing_notifications (p_subscription, p_msg_sc, p_moreNotifs,
                                                                          p_session, p_req_handle, p_req_context,
                                                                          p_msg_to_send)

            /* #15 transition */
            ELSIF a_subscription_state (p_subscription) = e_subscriptionState_keepAlive &
                  p_validPublishReqQueued = TRUE &
                  a_KeepAliveCounter (p_subscription) = 1 &
                  (l_PublishingEnabled = FALSE
                   or
                   (l_PublishingEnabled = TRUE &
                    l_NotificationAvailable = FALSE)
                  )
            THEN
                /* reset keepAlive counter */
                a_KeepAliveCounter (p_subscription) := a_keepAliveExpCount (p_subscription) ||
                d_dequeue_publish_req (p_subscription, p_publish_resp_msg) ||
                d_publish_timeout_keepAlive_msg_sent

            /* #16 transition */
            ELSIF a_subscription_state (p_subscription) = e_subscriptionState_keepAlive &
                  a_KeepAliveCounter (p_subscription) > 1 &
                  (l_PublishingEnabled = FALSE
                   or
                   (l_PublishingEnabled = TRUE &
                    l_NotificationAvailable = FALSE)
                  )
            THEN
                a_KeepAliveCounter (p_subscription) := a_KeepAliveCounter (p_subscription) - 1 ||
                d_publish_timeout_no_msg_sent_setMoreNotifs

            /* #17 transition */
            ELSIF a_subscription_state (p_subscription) = e_subscriptionState_keepAlive &
                  p_validPublishReqQueued = FALSE &
                  (a_KeepAliveCounter (p_subscription) = 1
                   or
                   (a_KeepAliveCounter (p_subscription) > 1 &
                    l_PublishingEnabled = TRUE &
                    l_NotificationAvailable = TRUE)
                  )
            THEN
                a_subscription_state (p_subscription) := e_subscriptionState_late ||
                d_publish_timeout_no_msg_sent_setMoreNotifs

            ELSE
                d_publish_timeout_no_msg_sent_setMoreNotifs
            END
        END
    END
    ;

    /* Corresponds to recursive call to ReturnNotifications() when MoreNotifications = TRUE & PublishingReqQueued = TRUE */
    p_msg_to_send, p_msg_sc, p_session, p_publish_resp_msg, p_req_handle, p_req_context, p_moreNotifs
      <-- server_subscription_core_publish_timeout_return_moreNotifs (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_subscription : a_MoreNotifications &
        a_subscription_state (p_subscription) = e_subscriptionState_normal &
        a_pendingNotificationQueue [{p_subscription}] /= {} &
        validPublishReqQueued = TRUE &
        a_notifRepublishQueue(p_subscription) : s_republishQueue
    THEN
        d_dequeue_publish_req (p_subscription, p_publish_resp_msg) ||
        // return notifications (dequeue notifications) + update MoreNotification variable
        d_publish_timeout_notif_msg_sent_dequeuing_notifications (p_subscription, p_msg_sc, p_moreNotifs,
                                                                  p_session, p_req_handle, p_req_context,
                                                                  p_msg_to_send)
    END
    ;

    server_subscription_add_notification (p_subscription, p_monitoredItemPointer, p_timestampToReturn, p_writeValuePointer) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_monitoredItemPointer : t_monitoredItemPointer_i &
        p_monitoredItemPointer : s_monitoredItemPointer &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
        p_writeValuePointer : t_WriteValuePointer_i &
        p_writeValuePointer : t_WriteValuePointer
    THEN
        /* a notification is enqueued with current value of monitored item */
        a_monitored_item_notifications :=
          a_monitored_item_notifications \/ {a_pendingNotificationQueue(p_subscription) |-> p_monitoredItemPointer}
    END
    ;

    p_bres, p_monitoredItemQueue <-- get_nodeToMonitoredItemQueue (p_nid) =
    PRE
        p_nid : t_NodeId_i &
        p_nid : t_NodeId
    THEN
        CHOICE
            p_bres := FALSE ||
            p_monitoredItemQueue := c_monitoredItemQueue_indet
        OR
            p_bres := TRUE ||
            p_monitoredItemQueue :(p_monitoredItemQueue : t_monitoredItemQueue_i &
                p_monitoredItemQueue = a_nodeToMonitoredItemQueue(p_nid))
        END
    END
    ;

    p_monitoredItemId,
    p_subscription,
    p_nid,
    p_aid,
    p_timestampToReturn,
    p_monitoringMode,
    p_clientHandle
      <--getall_monitoredItemPointer (p_monitoredItemPointer) =
    PRE
        p_monitoredItemPointer : t_monitoredItemPointer_i &
        p_monitoredItemPointer : s_monitoredItemPointer
    THEN
        p_monitoredItemId,
        p_subscription,
        p_nid,
        p_aid,
        p_timestampToReturn,
        p_monitoringMode,
        p_clientHandle
        :(p_monitoredItemId : t_monitoredItemId_i &
          p_monitoredItemId = a_monitoredItemId(p_monitoredItemPointer) &
          p_subscription : t_subscription_i &
          p_subscription : t_subscription &
          p_nid : t_NodeId_i &
          p_nid : t_NodeId &
          p_aid : t_AttributeId_i &
          p_aid : t_AttributeId &
          p_timestampToReturn : t_TimestampsToReturn_i &
          p_timestampToReturn : t_TimestampsToReturn &
          p_monitoringMode : t_monitoringMode_i &
          p_monitoringMode : t_monitoringMode &
          p_clientHandle : t_client_handle_i
        )
    END
    ;

    bres <-- is_notification_triggered (p_monitoredItemPointer, p_old_wv_pointer, p_new_wv_pointer) =
    PRE
        p_monitoredItemPointer : t_monitoredItemPointer_i &
        p_monitoredItemPointer : t_monitoredItemPointer &
        p_old_wv_pointer : t_WriteValuePointer_i &
        p_old_wv_pointer : t_WriteValuePointer &
        p_new_wv_pointer : t_WriteValuePointer_i &
        p_new_wv_pointer : t_WriteValuePointer
    THEN
        bres :: BOOL
    END
    ;

    set_msg_publish_resp_subscription (p_resp_msg, p_subscription) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        p_subscription : t_subscription_i &
        p_subscription : t_subscription
    THEN
        skip
    END
    ;

    set_msg_publish_resp_notificationMsg (p_resp_msg, p_moreNotifs) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        p_moreNotifs : BOOL
    THEN
        skip
    END
    ;

    generate_internal_send_publish_response_event (p_session, p_publish_resp_msg, p_req_handle, p_req_context, p_statusCode) =
    PRE
        p_session : t_session_i & p_session : t_session &
        p_publish_resp_msg : t_msg_i & p_publish_resp_msg : t_msg &
        p_req_handle : t_server_request_handle_i &
        p_req_context : t_request_context_i & p_req_context : t_request_context &
        p_statusCode : t_StatusCode_i & p_statusCode : t_StatusCode
    THEN
        skip
    END
    ;

    subscription_core_UNINITIALISATION =
    BEGIN
        d_init
    END
    ;

    bres, p_notif_msg <-- get_republish_notif_from_queue (p_queue, p_seq_num) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue &
        p_seq_num : t_sub_seq_num_i &
        p_seq_num : t_sub_seq_num
    THEN
        IF p_seq_num : a_notification_seq_num[a_republish_notification~[{p_queue}]]
        THEN
            bres := TRUE ||
            p_notif_msg
            :(p_notif_msg : t_notif_msg_i & p_notif_msg : t_notif_msg &
              p_notif_msg : a_republish_notification~[{p_queue}] &
              a_notification_seq_num(p_notif_msg) = p_seq_num)
        ELSE
            p_notif_msg := c_notif_msg_indet ||
            bres := FALSE
        END
    END
    ;

    /* EXTENDS */
    nb_seq_nums <-- get_available_republish (p_queue) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue
    THEN
        nb_seq_nums := card(a_republish_notification~[{p_queue}])
    END
    ;

    continue, iterator <-- init_iter_notif_republish (p_queue) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue
    THEN
        continue := bool(a_republish_notification~[{p_queue}] /= {}) ||
        notification_republish_iter_queue := p_queue ||
        s_iterated_sub_seq_num := {} ||
        s_to_iterate_sub_seq_num := a_notification_seq_num[a_republish_notification~[{p_queue}]] ||
        iterator :(iterator : t_notifRepublishQueueIterator_i &
                   (a_republish_notification~[{p_queue}] /= {} =>
                     iterator : t_notifRepublishQueueIterator))
    END
    ;

    continue, seq_num <-- continue_iter_notif_republish (p_queue, p_iterator) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue &
        p_queue = notification_republish_iter_queue &
        p_iterator : t_notifRepublishQueueIterator_i &
        p_iterator : t_notifRepublishQueueIterator &
        s_to_iterate_sub_seq_num /= {}
    THEN
        ANY l_seq_num WHERE
            l_seq_num : t_sub_seq_num_i &
            l_seq_num : s_to_iterate_sub_seq_num
        THEN
            seq_num := l_seq_num ||
            continue := bool(s_to_iterate_sub_seq_num - {l_seq_num} /= {}) ||
            s_to_iterate_sub_seq_num := s_to_iterate_sub_seq_num - {l_seq_num} ||
            s_iterated_sub_seq_num := s_iterated_sub_seq_num \/ {l_seq_num}
        END
    END
    ;

    clear_notif_republish_iterator (p_queue, p_iterator) =
    PRE
        p_queue : t_notifRepublishQueue_i &
        p_queue : s_republishQueue &
        p_queue = notification_republish_iter_queue &
        p_iterator : t_notifRepublishQueueIterator_i &
        p_iterator : t_notifRepublishQueueIterator
    THEN
        notification_republish_iter_queue := c_notifRepublishQueue_indet ||
        s_to_iterate_sub_seq_num := {} ||
        s_iterated_sub_seq_num := {}
    END;

    continue, iterator <-- init_iter_monitored_item (p_queue) =
    PRE
        p_queue : t_monitoredItemQueue_i &
        p_queue : t_monitoredItemQueue
    THEN
        mi_queue_it,
        mi_iterator_it,
        nb_monitored_items_it,
        s_mi_to_iterate_it,
        s_mi_iterated_it,
        continue,
        iterator
        :(mi_queue_it : t_monitoredItemQueue_i &
          mi_iterator_it : t_monitoredItemQueueIterator_i &
          s_mi_to_iterate_it <: t_monitoredItemPointer &
          s_mi_iterated_it <: t_monitoredItemPointer &
          continue : BOOL &
          iterator : t_monitoredItemQueueIterator_i &
          (continue = TRUE =>
              mi_queue_it = p_queue &
              iterator : t_monitoredItemQueueIterator &
              mi_iterator_it = iterator &
              nb_monitored_items_it > 0 &
              s_mi_to_iterate_it /= {} &
              card(s_mi_to_iterate_it) = nb_monitored_items_it &
              s_mi_iterated_it = {}
          )
        )
    END
    ;

    clear_iter_monitored_item (p_iterator) =
    PRE
        p_iterator : t_monitoredItemQueueIterator_i &
        p_iterator : t_monitoredItemQueueIterator &
        mi_iterator_it = p_iterator
    THEN
        mi_queue_it := c_monitoredItemQueue_indet ||
        mi_iterator_it := c_monitoredItemQueueIterator_indet ||
        nb_monitored_items_it := 0 ||
        s_mi_to_iterate_it :(s_mi_to_iterate_it = {}) ||
        s_mi_iterated_it :(s_mi_iterated_it = {})
    END
    ;

    continue,
    p_monitoredItem <-- continue_iter_monitored_item (p_iterator, p_queue) =
    PRE
        p_iterator : t_monitoredItemQueueIterator_i &
        mi_iterator_it = p_iterator &
        p_queue : t_monitoredItemQueue_i &
        p_queue : t_monitoredItemQueue &
        mi_queue_it = p_queue &
        s_mi_to_iterate_it /= {}
    THEN
        ANY l_monitoredItem WHERE
            l_monitoredItem : t_monitoredItemPointer_i &
            l_monitoredItem : s_mi_to_iterate_it
        THEN
            continue := bool(s_mi_to_iterate_it /= {l_monitoredItem}) ||
            p_monitoredItem := l_monitoredItem ||
            s_mi_to_iterate_it := s_mi_to_iterate_it - {p_monitoredItem} ||
            s_mi_iterated_it := s_mi_iterated_it \/ {p_monitoredItem}
        END
    END
END
