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
    msg_subscription_create_monitored_item_bs

SEES
    constants,
    message_in_bs,
    message_out_bs
    
/* TODO: define variable to represents the response content */

OPERATIONS
    
    p_subscription <-- get_msg_create_monitored_items_req_subscription (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_create_req
    THEN
        p_subscription :: t_subscription_i
    END
    ;
    
    p_timestampToRet <-- get_msg_create_monitored_items_req_timestamp_to_ret (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_create_req
    THEN
        p_timestampToRet :: t_TimestampsToReturn_i
    END
    ;
    
    l_monitored_item_not_null <-- check_msg_create_monitored_items_req_not_null (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_create_req
    THEN
        l_monitored_item_not_null :: BOOL
    END
    ;
        
    p_nb_monitored_items <-- get_msg_create_monitored_items_req_nb_monitored_items (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_create_req
    THEN
        p_nb_monitored_items :: INT
    END
    ;
    
    p_bres, p_sc, p_nid, p_aid, p_monitMode, p_clientHandle, p_samplingItv, p_queueSize <-- getall_monitored_item_req_params (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_create_req &
        p_index : NAT &
        p_index : t_monitoredItemReqIndex
    THEN
        p_bres, p_sc, p_nid, p_aid, p_monitMode, p_clientHandle, p_samplingItv, p_queueSize
        :(p_bres : BOOL &
          p_sc : t_StatusCode_i &
          p_sc : {e_sc_ok, e_sc_bad_attribute_id_invalid, 
                  e_sc_bad_monitoring_mode_invalid, 
                  e_sc_bad_monitored_item_filter_unsupported} &
          p_nid : t_NodeId_i &
          p_aid : t_AttributeId_i & 
          p_monitMode : t_monitoringMode_i &
          p_clientHandle : t_client_handle_i &
          p_samplingItv : t_opcua_duration_i &
          p_queueSize : t_counter &
          (p_bres = TRUE => 
           p_sc = e_sc_ok &
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
        p_nb_results : NAT &
        p_nb_results > 0
    THEN
        bres :: BOOL
    END
    ;
    
    setall_msg_monitored_item_resp_params (p_resp_msg, p_index, p_sc, p_monitored_item_id, p_revSamplingItv, p_revQueueSize) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_create_resp &
        p_index : NAT &
        p_index : t_monitoredItemReqIndex &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode &
        p_monitored_item_id : t_monitoredItemId_i &
        p_monitored_item_id : t_monitoredItemId &
        p_revSamplingItv : t_opcua_duration_i &
        p_revQueueSize : t_counter
    THEN
        skip
    END
   
END
