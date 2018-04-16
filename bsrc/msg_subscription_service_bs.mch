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
    msg_subscription_service_bs

SEES
    constants,
    message_in_bs,
    message_out_bs
    
ABSTRACT_VARIABLES
    nb_monitored_items
    
INVARIANT
    nb_monitored_items : NAT
    
INITIALISATION
    nb_monitored_items := 0

OPERATIONS
    
    /* TODO: move into message_in_bs: + need type for header ? */
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
    
    
    /* CreateSubscription messages */
    
    reqPublishInterval, reqLifetimeCount, reqMaxKeepAlive, maxNotificationsPerPublish, publishEnabled
      <-- get_msg_create_subscription_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_subscription_create_req
    THEN
        reqPublishInterval,
        reqLifetimeCount,
        reqMaxKeepAlive,
        maxNotificationsPerPublish,
        publishEnabled
        :(reqPublishInterval : t_opcua_duration_i &
          reqLifetimeCount : t_counter &
          reqMaxKeepAlive : t_counter &
          maxNotificationsPerPublish : t_counter &
          publishEnabled : BOOL)
    END
    ;
  
    set_msg_create_subscription_resp_params (p_resp_msg,
                                             p_subscription, 
                                             p_revisedPublishInterval,
                                             p_revisedLifetimeCount,
                                             p_revisedMaxKeepAlive) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_create_resp &
        p_subscription : t_subscription_i &
        p_subscription : t_subscription &
        p_revisedPublishInterval : t_opcua_duration_i &
        p_revisedLifetimeCount : t_counter &
        p_revisedMaxKeepAlive : t_counter &
        p_revisedLifetimeCount >= 3 * p_revisedMaxKeepAlive /* part 4 table 86 */ 
    THEN
        skip
    END;
    
    set_msg_publish_resp_subscription (p_resp_msg, p_subscription) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp &
        p_subscription : t_subscription_i &
        p_subscription : t_subscription
    THEN
        skip
    END
    ;
    
    set_msg_publish_resp_notificationMsg (p_resp_msg, p_notifMsg) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_subscription_publish_resp &
        p_notifMsg : t_notif_msg_i &
        p_notifMsg : t_notif_msg
    THEN
        skip
    END
    ;
    
    /* CreateMonitoredItem messages */
    
    p_bres, p_subscription, p_timestampToRet, p_nb_monitored_items <-- getall_msg_create_monitored_items_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_create_req
    THEN
        p_bres,
        p_subscription,
        p_timestampToRet,
        p_nb_monitored_items,
        nb_monitored_items
        :(  p_bres               : BOOL &
            p_subscription       : t_subscription_i &
            p_timestampToRet     : t_TimestampsToReturn_i &
            p_nb_monitored_items : INT &
            (p_bres = TRUE
             =>
             nb_monitored_items = p_nb_monitored_items &
             p_timestampToRet : t_TimestampsToReturn)
        )
    END
    ;
    
    /* Filters and DiscardOldest not extracted => not managed */
    p_bres, p_nid, p_aid, p_monitMode, p_clientHandle, p_samplingItv, p_queueSize <-- getall_monitored_item_req_params (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_create_req &
        p_index : INT &
        p_index <= nb_monitored_items
    THEN
        p_bres, p_nid, p_aid, p_monitMode, p_clientHandle, p_samplingItv, p_queueSize
        :(p_bres : BOOL &
            p_nid : t_NodeId_i &
            p_aid : t_AttributeId_i & 
            p_monitMode : t_monitoringMode_i &
            p_clientHandle : t_client_handle_i &
            p_samplingItv : t_opcua_duration_i &
            p_queueSize : t_counter &
            (p_bres = TRUE => 
             p_nid : t_NodeId &
             p_aid : t_AttributeId &
             p_monitMode : t_monitoringMode))
    END
    ;
    
    bres <-- alloc_msg_create_monitored_items_resp_results (p_resp_msg, p_nb_results) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_create_resp &
        p_nb_results : INT &
        p_nb_results = nb_monitored_items
    THEN
        bres :: BOOL
    END
    ;
    
    setall_msg_monitored_item_resp_params (p_resp_msg, p_index, p_sc, p_monitored_item, p_revSamplingItv, p_revQueueSize) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_create_resp &
        p_index : INT &
        p_index < nb_monitored_items &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode &
        p_monitored_item : t_monitoredItem_i &
        p_monitored_item : t_monitoredItem &
        p_revSamplingItv : t_opcua_duration_i &
        p_revQueueSize : t_counter
    THEN
        skip
    END  

END
