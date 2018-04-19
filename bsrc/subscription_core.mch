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
    request_handle_bs

DEFINITIONS
    d_variables ==
    s_subscription,
    s_monitoredItem,
    a_session,
    a_publishInterval,
    a_lifetimeExpCount,
    a_keepAliveExpCount,
    a_maxNotifsPerPublish,
    a_subscription_state,
    a_MoreNotifications,
    a_LatePublishRequest,
    a_LifetimeCounter,
    a_KeepAliveCounter,
    a_MessageSent,
    a_NotificationAvailable,
    a_PublishingEnabled,
    a_PublishingReqQueued,
    a_SeqNum,
    a_pendingNotificationQueue,
    a_publishRequestQueue,
    a_monitoredItemQueue,
    a_nodeToMonitoredItemQueue
    ;
    
    d_typing ==
    s_subscription <: t_subscription &
    s_monitoredItem <: t_monitoredItem &
    /* Subscription configuration record */
    a_session : s_subscription >+> t_session &                  // a subscription may be associated to a session (=> 1 subscription / session)
    a_publishInterval : s_subscription --> t_opcua_duration_i & // a subscription shall have a publishing interval value
    a_lifetimeExpCount : s_subscription --> t_counter &       // a subscription shall have a lifetime expiration count value
    a_keepAliveExpCount : s_subscription --> t_counter &      // a subscription shall have a keepAlive expiration count value
    a_maxNotifsPerPublish : s_subscription --> t_counter &    // a subscription shall have a maximum notifications per publish response
    
    /* Subscription State Machine : state shall be defined (part 4 table 82) */
    a_subscription_state : s_subscription --> t_subscriptionState &
    
    /* Subscription State Machine : variables shall be defined (part 4 table 84) */
    a_MoreNotifications <: s_subscription &
    a_LatePublishRequest <: s_subscription &
    a_LifetimeCounter : s_subscription --> t_counter &
    a_KeepAliveCounter : s_subscription --> t_counter & /* missing in table 84 */
    a_MessageSent <: s_subscription &
    a_NotificationAvailable <: s_subscription &
    a_PublishingEnabled <: s_subscription &
    a_PublishingReqQueued <: s_subscription &
    /* a_RequestedMessageFound <: s_subscription */   /* <=> seqNum in a_retransmissionQueue[subscription] */
    a_SeqNum : s_subscription --> t_sub_seq_num_i &
    /* a_SubscriptionAssignedToClient <: s_subscription */             /* Not really a state machine variable: local variable or function */
    
    /* Subscription Implementation variables */
    a_pendingNotificationQueue : s_subscription --> t_notificationQueue &
    /* a_retransmissionQueue : s_subscription --> t_notificationMsgQueue */ /* not implemented */
    a_monitoredItemQueue : s_subscription --> t_monitoredItemQueue &
    a_nodeToMonitoredItemQueue : t_Node --> t_monitoredItemQueue &
    
    /* (Session) Implementation variables */
    
    /* Note: kept on subscription for now since 1 subs / session + no transfer. Otherwise could be cleared on session close ! */
    a_publishRequestQueue :  s_subscription --> t_publishReqQueue
    
ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typing

INITIALISATION
    s_subscription := {}        ||
    s_monitoredItem := {}       ||
    a_session := {}             ||
    a_publishInterval := {}     ||
    a_lifetimeExpCount := {}    ||
    a_keepAliveExpCount := {}   ||
    a_maxNotifsPerPublish := {} ||

    a_subscription_state := {}                 ||

    a_MoreNotifications := {}     ||
    a_LatePublishRequest := {}    ||
    a_LifetimeCounter := {}       ||
    a_KeepAliveCounter := {}      ||
    a_MessageSent := {}           ||
    a_NotificationAvailable := {} ||
    a_PublishingEnabled := {}     ||
    a_PublishingReqQueued := {}   ||
    a_SeqNum              := {}   ||
    
    a_pendingNotificationQueue := {} ||
    a_publishRequestQueue := {} ||
    a_monitoredItemQueue := {} ||
    a_nodeToMonitoredItemQueue :(a_nodeToMonitoredItemQueue : t_Node --> t_monitoredItemQueue) /* DEFINED ON INIT */
    
OPERATIONS
    
    is_valid <-- is_valid_subscription_on_session (p_session, p_subscription) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_subscription : t_subscription_i
    THEN
        is_valid := bool(p_subscription : s_subscription & a_session(p_subscription) = p_session)
    END
    ;
    
    is_valid <-- is_valid_subscription (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        is_valid := bool(p_subscription : s_subscription)
    END
    ;
        
    l_has_subscription <-- has_subscription (p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        l_has_subscription := bool(p_session : ran(a_session))
    END
    ;
    
    revisedPublishInterval, revisedLifetimeCount, revisedMaxKeepAlive
      <-- compute_create_subscription_revised_params (p_session,
                                                      p_reqPublishInterval,
                                                      p_reqLifetimeCount,
                                                      p_reqMaxKeepAlive) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        a_session |> {p_session} = {} & /* session is not part of any suscription (=> 1 subscription / session) */
        p_reqPublishInterval : t_opcua_duration_i &
        p_reqLifetimeCount : t_counter &
        p_reqMaxKeepAlive : t_counter
    THEN
        revisedPublishInterval,
            revisedLifetimeCount,
            revisedMaxKeepAlive
            :(revisedPublishInterval : t_opcua_duration_i &
                revisedLifetimeCount : t_counter &
                revisedMaxKeepAlive : t_counter &
                revisedLifetimeCount >= 3 * revisedMaxKeepAlive  /* part 4 table 86 */
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
        a_session |> {p_session} = {} & /* session is not part of any suscription (=> 1 subscription / session) */
        p_revPublishInterval : t_opcua_duration_i &
        p_revLifetimeCount : t_counter &
        p_revMaxKeepAlive : t_counter &
        p_revLifetimeCount >= 3 * p_revMaxKeepAlive & /* part 4 table 86 */
        p_maxNotificationsPerPublish : t_counter &
        p_publishEnabled = TRUE /* => no subscription modification for now => refuse publishDisabled */
    THEN
        /* State table evaluation case (CreateSubscription event: #1, #2, #3) */
        /* TODO: CREATE PUBLISH TIMER */
        IF t_subscription = s_subscription
        THEN
            StatusCode_service := e_sc_bad_too_many_subscriptions ||
            subscription := c_subscription_indet
        ELSE
            ANY l_subscription WHERE
                l_subscription : t_subscription - s_subscription
            THEN
                CHOICE
                    /* Transition #1 + #2 */
                    StatusCode_service := e_sc_bad_out_of_memory ||
                    subscription := c_subscription_indet
                OR
                    /* Transition #1 + #3 */
                    StatusCode_service := e_sc_ok ||
                    
                    subscription := l_subscription ||
                    s_subscription := s_subscription \/ {l_subscription} ||
                    
                    a_session (l_subscription) := p_session ||
                    a_publishInterval (l_subscription) := p_revPublishInterval ||
                    a_lifetimeExpCount (l_subscription) := p_revLifetimeCount  ||
                    a_keepAliveExpCount (l_subscription) := p_revMaxKeepAlive  ||
                    a_maxNotifsPerPublish (l_subscription) := p_maxNotificationsPerPublish ||
                    
                    a_subscription_state (l_subscription) := e_subscription_normal                ||
                    a_LatePublishRequest := a_LatePublishRequest \/ {l_subscription} ||
                    a_LifetimeCounter (l_subscription) := p_revLifetimeCount         ||
                    a_KeepAliveCounter (l_subscription) := p_revMaxKeepAlive         ||
                    a_MessageSent := a_MessageSent \/ {l_subscription}               ||
                    a_PublishingEnabled := a_PublishingEnabled \/ {l_subscription}   ||
                    a_SeqNum (l_subscription)            := c_sub_seq_num_indet      ||
                    
                    a_pendingNotificationQueue
                    :(
                        a_pendingNotificationQueue : s_subscription --> t_notificationQueue &
                        {l_subscription} <<| a_pendingNotificationQueue = {l_subscription} <<| a_pendingNotificationQueue$0 &
                        l_subscription : dom(a_pendingNotificationQueue)
                    ) ||
                    a_publishRequestQueue
                    :(
                        a_publishRequestQueue : s_subscription --> t_publishReqQueue &
                        {l_subscription} <<| a_publishRequestQueue = {l_subscription} <<| a_publishRequestQueue$0 &
                        l_subscription : dom(a_publishRequestQueue)
                    ) ||
                    a_monitoredItemQueue
                    :(
                        a_monitoredItemQueue : s_subscription --> t_monitoredItemQueue &
                        {l_subscription} <<| a_monitoredItemQueue = {l_subscription} <<| a_monitoredItemQueue$0 &
                        l_subscription : dom(a_monitoredItemQueue)
                    )
                END
            END                
        END
    END
    ;
    
    StatusCode_service, async_resp_msg, subscription, notifMsg
      <-- receive_publish_request (p_session, p_req_exp_time, p_req_handle, p_req_ctx, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_session : ran(a_session) &
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
            LET l_subscription BE
                l_subscription = a_session~(p_session)
            IN
            LET l_PublishingEnabled, l_MoreNotifications, l_NotificationAvailable BE
                    l_PublishingEnabled = bool (l_subscription : a_PublishingEnabled) &
                l_MoreNotifications = bool (l_subscription : a_MoreNotifications) &
                l_NotificationAvailable = bool (l_subscription : a_NotificationAvailable)
                IN                
                    IF a_subscription_state (l_subscription) = e_subscription_normal &
                        (l_PublishingEnabled = FALSE or
                         (l_PublishingEnabled = TRUE & l_MoreNotifications = FALSE))
                    THEN
                        /* #4 transition */
                        /* TODO: enqueue publish request */
                    a_PublishingReqQueued := a_PublishingReqQueued \/ {l_subscription} ||
                        StatusCode_service := e_sc_ok ||
                        async_resp_msg := TRUE ||
                        subscription := c_subscription_indet ||
                        notifMsg := c_notif_msg_indet
                    ELSIF a_subscription_state (l_subscription) = e_subscription_normal &
                          l_PublishingEnabled = TRUE & l_MoreNotifications = TRUE
                    THEN
                        /* #5 transition */
                        a_MessageSent := a_MessageSent \/ {l_subscription} ||
                        a_LifetimeCounter (l_subscription) := a_lifetimeExpCount (l_subscription) || /* reset lifetime counter */
                        /* TODO ?: return notifications (dequeue) */
                        StatusCode_service := e_sc_ok ||
                        async_resp_msg := FALSE ||
                        subscription := l_subscription ||
                    notifMsg :(notifMsg : t_notif_msg_i & notifMsg : t_notif_msg) ||
                    /* a_NotificationAvailable & a_MoreNotifications state may change for subscription */
                    a_NotificationAvailable, 
                    a_MoreNotifications 
                    :(a_NotificationAvailable <: s_subscription &
                      a_NotificationAvailable - {l_subscription} = a_NotificationAvailable$0 - {l_subscription} &
                      a_MoreNotifications <: s_subscription &
                      a_MoreNotifications - {l_subscription} = a_MoreNotifications$0 - {l_subscription}
                    )
                ELSIF a_subscription_state (l_subscription) = e_subscription_late & l_PublishingEnabled = TRUE &
                      (l_NotificationAvailable = TRUE or l_MoreNotifications = TRUE)
                    THEN
                        /* #10 transition */
                    a_subscription_state (l_subscription) := e_subscription_normal ||
                        a_MessageSent := a_MessageSent \/ {l_subscription} ||
                        a_LifetimeCounter (l_subscription) := a_lifetimeExpCount (l_subscription) || /* reset lifetime counter */
                        /* TODO ?: return notifications (dequeue) */
                        StatusCode_service := e_sc_ok ||
                        async_resp_msg := FALSE ||
                        subscription := l_subscription ||
                    notifMsg :(notifMsg : t_notif_msg_i & notifMsg : t_notif_msg) ||
                    /* a_NotificationAvailable & a_MoreNotifications state may change for subscription */
                    a_NotificationAvailable, 
                    a_MoreNotifications 
                    :(a_NotificationAvailable <: s_subscription &
                      a_NotificationAvailable - {l_subscription} = a_NotificationAvailable$0 - {l_subscription} &
                      a_MoreNotifications <: s_subscription &
                      a_MoreNotifications - {l_subscription} = a_MoreNotifications$0 - {l_subscription}
                    )
                    ELSIF a_subscription_state (l_subscription) = e_subscription_late &
                    (l_PublishingEnabled = FALSE 
                     or
                         (l_PublishingEnabled = TRUE &
                      l_NotificationAvailable = FALSE &
                      l_MoreNotifications = FALSE)
                    )
                    THEN
                        /* #11 transition */
                    a_subscription_state (l_subscription) := e_subscription_keepAlive ||
                        a_MessageSent := a_MessageSent \/ {l_subscription} ||
                        a_LifetimeCounter (l_subscription) := a_lifetimeExpCount (l_subscription) || /* reset lifetime counter */
                        /* TODO ?: return notifications (dequeue) */
                        StatusCode_service := e_sc_ok ||
                        async_resp_msg := FALSE ||
                        subscription := l_subscription ||
                        notifMsg := c_notif_msg_indet /* KeepAlive => no notification */
                ELSE
                    StatusCode_service := e_sc_bad_invalid_state ||
                    async_resp_msg := FALSE ||
                    subscription := c_subscription_indet ||
                    notifMsg := c_notif_msg_indet
                    END
                END 
            END
    END
    ;
    
    revisedSamplingItv, revisedQueueSize <-- compute_create_montitored_item_revised_params (p_reqSamplingItv, p_reqQueueSize) =
    PRE
        p_reqSamplingItv : t_opcua_duration_i &
        p_reqQueueSize : t_counter
    THEN
        revisedSamplingItv :: t_opcua_duration_i ||
        revisedQueueSize :: t_counter
    END
    ;

    StatusCode_service, monitoredItem <-- create_monitored_item (p_subscription,
                                                                 p_node,
                                                                 p_aid,
                                                                 p_timestampToReturn,
                                                                 p_monitoringMode,
                                                                 p_clientHandle) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_node : t_Node_i &
        p_node : t_Node &
        p_aid : t_AttributeId_i &
        p_aid : t_AttributeId &
        p_timestampToReturn : t_TimestampsToReturn_i &
        p_timestampToReturn : t_TimestampsToReturn &
        p_monitoringMode : t_monitoringMode_i &
        p_monitoringMode : t_monitoringMode &
        p_clientHandle : t_client_handle_i
    THEN
        IF t_monitoredItem = s_monitoredItem
        THEN
            StatusCode_service := e_sc_bad_too_many_monitored_items ||
            monitoredItem := c_monitoredItem_indet
        ELSE
            ANY l_monitoredItem WHERE
                l_monitoredItem : t_monitoredItem - s_monitoredItem
            THEN
                /* TODO: add monitored item to both queues + generate a notification ! */
                CHOICE
                    StatusCode_service := e_sc_ok ||
                    monitoredItem :(monitoredItem : t_monitoredItem_i & monitoredItem : t_monitoredItem) ||
                    s_monitoredItem := s_monitoredItem \/ {l_monitoredItem} ||
                    /* TODO: a notification shall be generated with current value of monitored item */
                    a_NotificationAvailable := a_NotificationAvailable \/ {p_subscription}
                OR
                    /* add to queue failure (out of memory) */
                    StatusCode_service := e_sc_bad_too_many_monitored_items ||
                    monitoredItem := c_monitoredItem_indet
                END
            END
        END
    END
    ;
    
    p_msg_to_send, p_session, p_publish_resp_msg, p_req_handle, p_req_context, p_notifMsg
      <-- server_subscription_core_publish_timeout (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription
    THEN
        /* State table evaluation case (Publish Timer Expires: #6, #7, #8, #9, #12, #14, #15, #16, #17, #27) */
        
        /* CAUTION: p_session shall be activated get_session_state_or_closed(p_session) = userActivated (i.e. channel is connected)
        to could send a message => some transitions cannot be done without it !
        */
        p_msg_to_send, p_session, p_publish_resp_msg, p_req_handle, p_req_context, p_notifMsg
        :(p_msg_to_send : BOOL &
          p_session : t_session_i &
          p_req_handle : t_server_request_handle_i &      
          p_req_context : t_request_context_i &
          p_publish_resp_msg : t_msg_i &
          p_notifMsg : t_notif_msg_i &
          (p_msg_to_send = TRUE => 
            p_session : t_session &
            p_req_context : t_request_context &
            p_publish_resp_msg : t_msg &
            p_notifMsg : t_notif_msg)
        )
         /* a_publishRequestQueue, a_NotificationAvailable, a_MoreNotifications */
    END
    ;
    
    monitoredItemQueue <-- get_nodeToMonitoredItemQueue (p_node) =
    PRE
        p_node : t_Node_i &
        p_node : t_Node
    THEN
        monitoredItemQueue :( monitoredItemQueue : t_monitoredItemQueue_i & monitoredItemQueue = a_nodeToMonitoredItemQueue (p_node))
    END
    ;
    
    server_subscription_add_notification (p_subscription, p_monitoredItem, p_writeValuePointer) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : s_subscription &
        p_monitoredItem : t_monitoredItem_i &
        p_monitoredItem : t_monitoredItem &
        p_writeValuePointer : t_WriteValuePointer_i &
        p_writeValuePointer : t_WriteValuePointer
    THEN
        /* TODO: add to notification queue */
        a_NotificationAvailable := a_NotificationAvailable \/ {p_subscription}
    END
        
    
END