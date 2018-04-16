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
    message_out_bs

DEFINITIONS
    d_variables ==
    s_subscription,
    a_session,
    a_publishInterval,
    a_lifetimeExpCount,
    a_keepAliveExpCount,
    a_maxNotifsPerPublish,
    a_state,
    a_MoreNotifications,
    a_LatePublishRequest,
    a_LifetimeCounter,
    a_KeepAliveCounter,
    a_MessageSent,
    a_PublishingEnabled,
    a_SeqNum,
    a_pendingNotificationQueue,
    a_publishRequestQueue,
    a_monitoredItemQueue,
    a_nodeToMonitoredItemQueue
    ;
    
    d_typing ==
    s_subscription <: t_subscription &
    /* Subscription configuration record */
    a_session : s_subscription +-> t_session &                  // a subscription may be associated to a session
    a_publishInterval : s_subscription --> t_opcua_duration_i & // a subscription shall have a publishing interval value
    a_lifetimeExpCount : s_subscription --> t_counter &       // a subscription shall have a lifetime expiration count value
    a_keepAliveExpCount : s_subscription --> t_counter &      // a subscription shall have a keepAlive expiration count value
    a_maxNotifsPerPublish : s_subscription --> t_counter &    // a subscription shall have a maximum notifications per publish response
    
    /* Subscription State Machine : state shall be defined (part 4 table 82) */
    a_state : s_subscription --> t_subscriptionState &
    
    /* Subscription State Machine : variables shall be defined (part 4 table 84) */
    a_MoreNotifications <: s_subscription &
    a_LatePublishRequest <: s_subscription &
    a_LifetimeCounter : s_subscription --> t_counter &
    a_KeepAliveCounter : s_subscription --> t_counter & /* missing in table 84 */
    a_MessageSent <: s_subscription &
    /* a_NotificationAvailable <: s_subscription */ /* <=> a_pendingNotificationQueue[subscription] empty */
    a_PublishingEnabled <: s_subscription &
    /* a_PublishingReqQueued <: s_subscription */   /* <=> a_publishRequestQueue[subscription] empty */
    /* a_RequestedMessageFound <: s_subscription */   /* <=> seqNum in a_retransmissionQueue[subscription] */
    a_SeqNum : s_subscription --> t_sub_seq_num_i &
    /* a_SubscriptionAssignedToClient <: s_subscription */             /* Not really a state machine variable: local variable or function */
    
    /* Subscription Implementation variables */
    a_pendingNotificationQueue : s_subscription --> t_notificationQueue &
    a_publishRequestQueue :  s_subscription --> t_publishReqQueue &
    /* a_retransmissionQueue : s_subscription --> t_notificationMsgQueue */ /* not implemented */
    a_monitoredItemQueue : s_subscription --> t_monitoredItemQueue &
    a_nodeToMonitoredItemQueue : t_Node --> t_monitoredItemQueue
    
ABSTRACT_VARIABLES
    d_variables

INVARIANT
    d_typing

INITIALISATION
    s_subscription := {}        ||
    a_session := {}             ||
    a_publishInterval := {}     ||
    a_lifetimeExpCount := {}    ||
    a_keepAliveExpCount := {}   ||
    a_maxNotifsPerPublish := {} ||

    a_state := {}                 ||

    a_MoreNotifications := {}     ||
    a_LatePublishRequest := {}    ||
    a_LifetimeCounter := {}       ||
    a_KeepAliveCounter := {}      ||
    a_MessageSent := {}           ||
    a_PublishingEnabled := {}     ||
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
        is_valid := bool(p_subscription : t_subscription & a_session(p_subscription) = p_session)
    END
    ;
    
    is_valid <-- is_valid_subscription (p_subscription) =
    PRE
        p_subscription : t_subscription_i
    THEN
        is_valid := bool(p_subscription : t_subscription)
    END
    ;
        
    l_has_subscription <-- has_subscription (p_session) =
    PRE
        p_session : t_session_i &
        p_session : t_session
    THEN
        l_has_subscription := bool(a_session |> {p_session} /= {})
    END
    ;
    
    acknowledge_publish_response (p_sub, p_seq_num) = 
    PRE
        p_sub : t_subscription_i &
        p_sub : t_subscription &
        p_seq_num : t_counter
    THEN
        skip
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
        p_publishEnabled : BOOL
    THEN
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
                    StatusCode_service := e_sc_bad_out_of_memory ||
                    subscription := c_subscription_indet
                OR
                    StatusCode_service := e_sc_ok ||
                    
                    subscription := l_subscription ||
                    s_subscription := s_subscription \/ {l_subscription} ||
                    
                    a_session (l_subscription) := p_session ||
                    a_publishInterval (l_subscription) := p_revPublishInterval ||
                    a_lifetimeExpCount (l_subscription) := p_revLifetimeCount  ||
                    a_keepAliveExpCount (l_subscription) := p_revMaxKeepAlive  ||
                    a_maxNotifsPerPublish (l_subscription) := p_maxNotificationsPerPublish ||
                    
                    a_state (l_subscription) := e_subscription_normal                ||
                    a_MoreNotifications := a_MoreNotifications \/ {l_subscription}   ||
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
      <-- receive_publish_request (p_session, p_req_exp_time, p_req_ctx, p_resp_msg) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_req_exp_time : t_timeref_i &
        p_req_exp_time : t_timeref &
        p_req_ctx : t_request_context_i &
        p_req_ctx : t_request_context &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp
    THEN
        CHOICE
            /* if service failed, response is synchronous */
            StatusCode_service,
            async_resp_msg,
            subscription,
            notifMsg
            :(  StatusCode_service : t_StatusCode_i &
                StatusCode_service : t_StatusCode &
                StatusCode_service /= e_sc_ok &
                async_resp_msg = FALSE &
                subscription : t_subscription_i &
                notifMsg = c_notif_msg_indet
            )
        OR
            /* if service succeeded, response is synchronous or asynchronous */
            StatusCode_service,
            async_resp_msg,
            subscription,
            notifMsg
            :(  StatusCode_service : t_StatusCode_i &
                StatusCode_service = e_sc_ok &
                async_resp_msg : BOOL &
                subscription : t_subscription_i & subscription : t_subscription &
                notifMsg : t_notif_msg_i & notifMsg : t_notif_msg
            )            
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
        p_subscription : t_subscription &
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
        /* TODO: add monitored item to both queues + generate a notification ! */
        CHOICE
            StatusCode_service := e_sc_ok ||
            monitoredItem :(monitoredItem : t_monitoredItem_i & monitoredItem : t_monitoredItem)
        OR
            StatusCode_service := e_sc_bad_too_many_monitored_items ||
            monitoredItem :(monitoredItem : t_monitoredItem_i)
        END
    END
    ;
    
    server_subscription_publish_timeout (p_subscription) =
    PRE
        p_subscription : t_subscription_i &
        p_subscription : t_subscription
    THEN
        /* State table evaluation case (Publish Timer Expires) */
        skip
    END
    
END