/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

MACHINE
    subscription_core

SEES
    constants,
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
    a_monitoredItemQueue,
    a_publishTimer,
    a_nodeToMonitoredItemQueue,
    /* Current valid publish response message from queue */
    publish_response_message,
    /* Notification message content */
    notification_message,
    notification_message_seq_num,
    /* Queues content */
    s_publishReqQueue,
    a_publish_responses,
    validPublishReqQueued,
    s_notificationQueue,
    a_monitored_item_notifications,
    s_monitoredItemQueue,
    a_defined_monitored_items
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
    
    s_notificationQueue := {} ||
    a_monitored_item_notifications := {} ||
    s_monitoredItemQueue := {} ||
    a_defined_monitored_items := {}
    ;
    
    d_typing ==
    s_subscription <: t_subscription &

    /* Subscription configuration record */
    a_subscription_session : s_subscription >+> t_session &                  // a subscription may be associated to a session (=> 1 subscription / session)
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
    
    /* Queues variables */
    /* PublishRequest queue content: */
    a_publish_responses : t_msg +-> s_publishReqQueue &
    validPublishReqQueued : BOOL &
    
    a_monitored_item_notifications : s_notificationQueue <-> s_monitoredItemPointer &
    a_defined_monitored_items : s_monitoredItemQueue <-> s_monitoredItemPointer
    ;
    
    /* Dequeuing invalid publish requests and check for a valid request still enqueued */
    d_check_for_valid_publish_req_queue(p_subsciption, l_PublishingReqQueued, l_new_publish_response) ==
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
    p_req_handle :(p_req_handle : t_server_request_handle_i) ||
    p_req_context := c_request_context_indet ||
    p_publish_resp_msg := c_msg_indet
    ;
    
    d_publish_timeout_no_msg_sent_setMoreNotifs ==
    d_publish_timeout_no_msg_sent ||
    p_moreNotifs := FALSE ||
    p_msg_sc := c_StatusCode_indet
    ;
    
    d_publish_request_notif_msg_sent_dequeuing_notifications (p_subscription, p_msg_sc, p_moreNotifs) ==
    LET l_notifQueue BE
        l_notifQueue = a_pendingNotificationQueue(p_subscription)
    IN
    a_MessageSent := a_MessageSent \/ {p_subscription} ||
    p_msg_sc, a_SeqNum, notification_message, notification_message_seq_num,
    a_monitored_item_notifications, a_MoreNotifications, p_moreNotifs
    :(p_msg_sc : t_StatusCode_i & p_msg_sc : t_StatusCode &
      p_msg_sc : {e_sc_ok, e_sc_bad_out_of_memory} &
      a_SeqNum : s_subscription --> t_sub_seq_num &
      p_session : t_session_i & p_session = a_subscription_session (p_subscription) &
      p_req_handle : t_server_request_handle_i &
      notification_message : t_notif_msg_i &
      notification_message_seq_num : t_sub_seq_num_i &
      a_monitored_item_notifications : s_notificationQueue <-> s_monitoredItemPointer &
      a_monitored_item_notifications <: a_monitored_item_notifications$0 &
      {l_notifQueue} <<| a_monitored_item_notifications = {l_notifQueue} <<| a_monitored_item_notifications$0 &
      p_moreNotifs : BOOL &
      // OK CASE
      (p_msg_sc = e_sc_ok => 
       {p_subscription} <<| a_SeqNum = {p_subscription} <<| a_SeqNum$0 &
       a_SeqNum (p_subscription) /= a_SeqNum$0 (p_subscription) &
       notification_message : t_notif_msg &
       notification_message_seq_num = a_SeqNum$0 (p_subscription) & // Consume precedent sequence number
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
       p_moreNotifs = TRUE
      )
    )
    END
    ;
    
    d_publish_timeout_notif_msg_sent_dequeuing_notifications ==
    LET l_notifQueue BE
        l_notifQueue = a_pendingNotificationQueue(p_subscription)
    IN
    a_MessageSent := a_MessageSent \/ {p_subscription} ||
    p_msg_to_send := TRUE ||
    p_msg_sc, a_SeqNum, p_session, p_req_handle, p_req_context, notification_message, notification_message_seq_num,
    a_monitored_item_notifications, a_MoreNotifications, p_moreNotifs
    :(p_msg_sc : t_StatusCode_i & p_msg_sc : t_StatusCode &
      p_msg_sc : {e_sc_ok, e_sc_bad_out_of_memory} &
      a_SeqNum : s_subscription --> t_sub_seq_num &
      p_session : t_session_i & p_session = a_subscription_session (p_subscription) &
      p_req_handle : t_server_request_handle_i &
      p_req_context : t_request_context_i & p_req_context : t_request_context &
      notification_message : t_notif_msg_i &
      notification_message_seq_num : t_sub_seq_num_i &
      a_monitored_item_notifications : s_notificationQueue <-> s_monitoredItemPointer &
      a_monitored_item_notifications <: a_monitored_item_notifications$0 &
      {l_notifQueue} <<| a_monitored_item_notifications = {l_notifQueue} <<| a_monitored_item_notifications$0 &
      p_moreNotifs : BOOL &
      // OK CASE
      (p_msg_sc = e_sc_ok => 
       {p_subscription} <<| a_SeqNum = {p_subscription} <<| a_SeqNum$0 &
       a_SeqNum (p_subscription) /= a_SeqNum$0 (p_subscription) &
       notification_message : t_notif_msg &
       notification_message_seq_num = a_SeqNum$0 (p_subscription) & // Consume precedent sequence number
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
       p_moreNotifs = TRUE &
       a_MoreNotifications = a_MoreNotifications$0 \/ {p_subscription}
      )
    )
    END
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
    )
    ;
    
    d_publish_timeout_state_changed_msg_sent ==
    a_MessageSent := a_MessageSent \/ {p_subscription} ||
    p_msg_to_send := TRUE ||
    p_session, p_req_handle, p_req_context, notification_message, notification_message_seq_num, a_SeqNum
    :(p_session : t_session_i & p_session = a_subscription_session (p_subscription) &
        p_req_handle : t_request_handle_i & p_req_handle : s_request_handle & 
        p_req_context : t_request_context_i & p_req_context : t_request_context &
        notification_message : t_notif_msg_i & notification_message : t_notif_msg &
        notification_message_seq_num = a_SeqNum$0 (p_subscription) &
        {l_subscription} <<| a_SeqNum = {l_subscription} <<| a_SeqNum$0 &
        a_SeqNum (l_subscription) /= a_SeqNum$0 (l_subscription)
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
    
    /* To be used if is_session_still_activated (p_subscription) == FALSE
       since requests will not be valid anymore on another SC */
    empty_session_publish_requests (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        a_publish_responses := a_publish_responses |>> {a_publishRequestQueue (p_subscription)}
    END
    ;
    
    remove_subscription_session (p_subscription, p_session) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_session : t_session_i &
        p_session : t_session
    THEN
        a_subscription_session := a_subscription_session - {p_subscription |-> p_session}
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

    revisedPublishInterval, revisedLifetimeCount, revisedMaxKeepAlive, revisedMaxNotificationsPerPublish
      <-- compute_create_subscription_revised_params (p_session,
                                                      p_reqPublishInterval,
                                                      p_reqLifetimeCount,
                                                      p_reqMaxKeepAlive,
                                                      p_maxNotificationsPerPublish) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        a_subscription_session |> {p_session} = {} & /* session is not part of any suscription (=> 1 subscription / session) */
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
        p_session /: ran(a_subscription_session) & /* session is not part of any suscription (=> 1 subscription / session) */
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
            ANY l_subscription WHERE
                l_subscription : t_subscription - s_subscription
            THEN
                IF t_monitoredItemQueue /= s_monitoredItemQueue &
                   t_notificationQueue /= s_notificationQueue &
                   t_publishReqQueue /= s_publishReqQueue
                THEN
                    /* Transition #1 + #3 */
                    StatusCode_service := e_sc_ok ||
                    
                    subscription := l_subscription ||
                    s_subscription := s_subscription \/ {l_subscription} ||
                    
                    a_subscription_session (l_subscription) := p_session ||
                    a_publishInterval (l_subscription) := p_revPublishInterval ||
                    a_lifetimeExpCount (l_subscription) := p_revLifetimeCount  ||
                    a_keepAliveExpCount (l_subscription) := p_revMaxKeepAlive  ||
                    a_maxNotifsPerPublish (l_subscription) := p_maxNotificationsPerPublish ||
                    
                    a_subscription_state (l_subscription) := e_subscriptionState_normal   ||
                    a_LifetimeCounter (l_subscription) := p_revLifetimeCount         ||
                    a_KeepAliveCounter (l_subscription) := p_revMaxKeepAlive         ||
                    a_MessageSent := a_MessageSent \/ {l_subscription}               ||
                    a_PublishingEnabled := a_PublishingEnabled \/ {l_subscription}   ||
                    a_SeqNum (l_subscription)            := c_sub_seq_num_indet      ||
                    
                    ANY l_notifQueue WHERE
                        l_notifQueue : t_notificationQueue_i &
                        l_notifQueue : t_notificationQueue &
                        l_notifQueue /: s_notificationQueue
                    THEN
                        s_notificationQueue := s_notificationQueue \/ {l_notifQueue} ||
                        a_pendingNotificationQueue := a_pendingNotificationQueue \/ {l_subscription |-> l_notifQueue}
                    END ||
                    
                    ANY l_publishQueue WHERE
                        l_publishQueue : t_publishReqQueue_i &
                        l_publishQueue : t_publishReqQueue &
                        l_publishQueue /: s_publishReqQueue
                    THEN
                        s_publishReqQueue := s_publishReqQueue \/ {l_publishQueue} ||
                        a_publishRequestQueue := a_publishRequestQueue \/ {l_subscription |-> l_publishQueue}
                    END ||
                    
                    ANY l_monitoredItemQueue WHERE
                        l_monitoredItemQueue : t_monitoredItemQueue_i &
                        l_monitoredItemQueue : t_monitoredItemQueue &
                        l_monitoredItemQueue /: s_monitoredItemQueue
                    THEN
                        s_monitoredItemQueue := s_monitoredItemQueue \/ {l_monitoredItemQueue} ||
                        a_monitoredItemQueue := a_monitoredItemQueue \/ {l_subscription |-> l_monitoredItemQueue}
                    END
                ELSE                       
                    /* Transition #1 + #2 */
                    StatusCode_service := e_sc_bad_out_of_memory ||
                    subscription := c_subscription_indet
                END
            END                
        END
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
                    a_publish_responses := a_publish_responses \/ {p_resp_msg |-> a_publishRequestQueue (l_subscription)} ||
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
                    a_publish_responses := a_publish_responses \/ {p_resp_msg |-> a_publishRequestQueue (l_subscription)} ||
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
                subscription := l_subscription ||
                moreNotifs := FALSE
            END
        END 
    END
    ;
    
    revisedSamplingItv, revisedQueueSize <-- compute_create_montitored_item_revised_params (p_reqQueueSize) =
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
                                                                                         p_value,
                                                                                         p_valueSc,
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
        p_value : t_Variant_i &
        p_value : t_Variant &
        p_valueSc : t_StatusCode_i &
        p_valueSc : t_StatusCode &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
        p_monitoringMode : t_monitoringMode_i &
        p_monitoringMode : t_monitoringMode &
        p_clientHandle : t_client_handle_i
    THEN
        IF t_monitoredItemPointer = s_monitoredItemPointer or
            t_monitoredItemId = s_monitoredItemId
        THEN
            /* To simplify proof, this includes the case out of memory when adding queues */
            StatusCode_service := e_sc_bad_too_many_monitored_items ||
            monitoredItemPointer := c_monitoredItemPointer_indet ||
            monitoredItemId := c_monitoredItemId_indet
        ELSE
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
                    d_publish_timeout_notif_msg_sent_dequeuing_notifications
            
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
                    d_publish_timeout_notif_msg_sent_dequeuing_notifications
                    
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
        validPublishReqQueued = TRUE
    THEN               
        d_dequeue_publish_req (p_subscription, p_publish_resp_msg) ||
        // return notifications (dequeue notifications) + update MoreNotification variable
        d_publish_timeout_notif_msg_sent_dequeuing_notifications
    END
    ;
    
    server_subscription_add_notification (p_subscription, p_monitoredItemPointer, p_writeValuePointer) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_monitoredItemPointer : t_monitoredItemPointer_i &
        p_monitoredItemPointer : s_monitoredItemPointer &
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
    
    req_expiration_time <-- get_msg_header_expiration_time (p_req_header) =
    PRE
        p_req_header : t_msg_header_i &
        p_req_header = c_msg_in_header &
        c_msg_in_header : t_msg_header &
        a_msg_in_type = e_msg_subscription_publish_req
    THEN
        req_expiration_time :(req_expiration_time : t_timeref_i & req_expiration_time : t_timeref)
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

END