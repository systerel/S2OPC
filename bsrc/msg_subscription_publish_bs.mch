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
    msg_subscription_publish_bs
    
SEES
    constants,
    message_in_bs,
    message_out_bs,
    request_handle_bs

ABSTRACT_VARIABLES
    publish_response_message,
    notification_message,
    notification_message_seq_num,
    nb_monitored_item_notifications,
    MonitoredItemNotification_MonitoredItemId

INVARIANT
    publish_response_message : t_msg_i &
    notification_message : t_notif_msg_i &
    notification_message_seq_num : t_sub_seq_num_i &
    nb_monitored_item_notifications : NAT &
    nb_monitored_item_notifications <= k_n_monitoredItemNotif_max &
    
    MonitoredItemNotification_MonitoredItemId : t_monitoredItemNotifIndex +-> t_monitoredItemId

INITIALISATION
    publish_response_message := c_msg_indet ||
    notification_message := c_notif_msg_indet ||
    notification_message_seq_num := c_sub_seq_num_indet ||
    nb_monitored_item_notifications := 0 ||
    MonitoredItemNotification_MonitoredItemId :(MonitoredItemNotification_MonitoredItemId = {})

OPERATIONS
    
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
        ((p_resp_msg = c_msg_out & a_msg_out_type = e_msg_subscription_publish_resp)
         or publish_response_message = p_resp_msg) &
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
        ((p_resp_msg = c_msg_out & a_msg_out_type = e_msg_subscription_publish_resp)
         or publish_response_message = p_resp_msg) &
        p_moreNotifs : BOOL
    THEN
        skip
    END
    ;
    
    generate_internal_send_publish_response_event (p_session, p_publish_resp_msg, p_req_handle, p_req_context, p_statusCode) =
    PRE
        p_session : t_session_i & p_session : t_session &
        p_publish_resp_msg : t_msg_i & p_publish_resp_msg : t_msg &
        publish_response_message = p_publish_resp_msg &
        p_req_handle : t_server_request_handle_i & 
        p_req_context : t_request_context_i & p_req_context : t_request_context &
        p_statusCode : t_StatusCode_i & p_statusCode : t_StatusCode
    THEN
        skip
    END
    ;
    
    /* Define current publish response message */
    set_publish_response_msg (p_publish_resp_msg) =
    PRE
        p_publish_resp_msg : t_msg_i & p_publish_resp_msg : t_msg
    THEN
        publish_response_message := p_publish_resp_msg
    END
    ;
    
    /* Notification message treatments */
    p_notifMsg <-- get_notification_message_no_items (p_publish_resp_msg) =
    PRE
        p_publish_resp_msg : t_msg_i & p_publish_resp_msg : t_msg &
        publish_response_message = p_publish_resp_msg
    THEN
        notification_message, p_notifMsg
        :(p_notifMsg : t_notif_msg_i & p_notifMsg : t_notif_msg &
            notification_message = p_notifMsg)
    END
    ;
    
    bres, p_notifMsg <-- alloc_notification_message_items (p_publish_resp_msg, p_nb_monitored_item_notifications) =
    PRE
        p_publish_resp_msg : t_msg_i & p_publish_resp_msg : t_msg &
        publish_response_message = p_publish_resp_msg &
        p_nb_monitored_item_notifications : NAT &
        p_nb_monitored_item_notifications > 0 &
        p_nb_monitored_item_notifications <= k_n_monitoredItemNotif_max
    THEN
        CHOICE
            bres := FALSE ||
            p_notifMsg := c_notif_msg_indet
        OR
            bres := FALSE ||
            nb_monitored_item_notifications := p_nb_monitored_item_notifications ||
            notification_message, p_notifMsg
            :(p_notifMsg : t_notif_msg_i & p_notifMsg : t_notif_msg &
              notification_message = p_notifMsg)
        END
    END
    ;
    
    set_notification_message_sequence_number (p_notifMsg, p_seq_num) =
    PRE
        p_notifMsg : t_notif_msg_i & p_notifMsg : t_notif_msg &
        p_notifMsg = notification_message &
        p_seq_num : t_sub_seq_num_i & p_seq_num : t_sub_seq_num
    THEN
        notification_message_seq_num := p_seq_num
    END
    ;
    
    setall_notification_msg_monitored_item_notif (p_notifMsg, p_index, p_monitored_item_id, p_clientHandle, p_wv_pointer) =
    PRE
        p_notifMsg : t_notif_msg_i & p_notifMsg : t_notif_msg &
        p_notifMsg = notification_message &
        p_index : NAT &
        p_index : t_monitoredItemNotifIndex &
        p_index > 0 &
        p_index <= nb_monitored_item_notifications &        
        p_monitored_item_id : t_monitoredItemId_i &
        p_monitored_item_id : t_monitoredItemId &
        p_clientHandle : t_client_handle_i &
        p_wv_pointer : t_WriteValuePointer_i &
        p_wv_pointer : t_WriteValuePointer
    THEN
        MonitoredItemNotification_MonitoredItemId(p_index) := p_monitored_item_id
    END

END
