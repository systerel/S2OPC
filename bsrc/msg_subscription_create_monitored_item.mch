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
    msg_subscription_create_monitored_item

SEES
    constants,
    message_in_bs,
    message_out_bs
    
DEFINITIONS
    d_inv == (
        nb_monitored_items : NAT &
        nb_monitored_items : t_monitoredItemReqIndex &
        nb_monitored_items <= k_n_monitoredItem_max &
        
        MonitoredItemReq_NodeId      : t_monitoredItemReqIndex +-> t_NodeId &
        MonitoredItemReq_AttributeId : t_monitoredItemReqIndex +-> t_AttributeId &
        
        MonitoredItemResp_Id : t_monitoredItemReqIndex +-> t_monitoredItemId &
        MonitoredItemResp_StatusCode : t_monitoredItemReqIndex +-> t_StatusCode
    )

ABSTRACT_VARIABLES
    nb_monitored_items,
    MonitoredItemReq_NodeId,
    MonitoredItemReq_AttributeId,
    MonitoredItemResp_Id,
    MonitoredItemResp_StatusCode

INVARIANT
    d_inv

INITIALISATION
    nb_monitored_items           := 0  ||
    MonitoredItemReq_NodeId      := {} ||
    MonitoredItemReq_AttributeId := {} ||
    MonitoredItemResp_Id := {}         ||
    MonitoredItemResp_StatusCode := {}
    
OPERATIONS  
    
    p_sc, p_subscription, p_timestampToRet, p_nb_monitored_items <-- getall_msg_create_monitored_items_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_create_req
    THEN
        p_sc,
        p_subscription,
        p_timestampToRet,
        p_nb_monitored_items,
        nb_monitored_items,
        MonitoredItemReq_NodeId,
        MonitoredItemReq_AttributeId
        :(  p_sc                 : t_StatusCode &
            p_subscription       : t_subscription_i &
            p_timestampToRet     : t_TimestampsToReturn_i &
            p_nb_monitored_items : NAT &
            (p_sc = e_sc_ok
                =>
                p_nb_monitored_items > 0 &
                p_nb_monitored_items <= k_n_monitoredItem_max &
                nb_monitored_items = p_nb_monitored_items &
                MonitoredItemReq_NodeId : t_monitoredItemReqIndex +-> t_NodeId &
                dom(MonitoredItemReq_NodeId) = 1..nb_monitored_items &
                MonitoredItemReq_AttributeId : t_monitoredItemReqIndex +-> t_AttributeId &
                dom(MonitoredItemReq_AttributeId) = 1..nb_monitored_items &
                p_timestampToRet : t_TimestampsToReturn) &
            (p_sc /= e_sc_ok
                => p_sc : {e_sc_bad_decoding_error, e_sc_bad_nothing_to_do, e_sc_bad_timestamps_to_return_invalid, e_sc_bad_too_many_ops})
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
        p_index : NAT &
        p_index : t_monitoredItemReqIndex &
        p_index > 0 &
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
             p_nid = MonitoredItemReq_NodeId(p_index) &
             p_aid = MonitoredItemReq_AttributeId(p_index) &
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
        p_nb_results > 0 &
        p_nb_results = nb_monitored_items
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
        p_index > 0 &
        p_index <= nb_monitored_items &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode &
        p_monitored_item_id : t_monitoredItemId_i &
        p_monitored_item_id : t_monitoredItemId &
        p_revSamplingItv : t_opcua_duration_i &
        p_revQueueSize : t_counter
    THEN
        MonitoredItemResp_Id(p_index) := p_monitored_item_id ||
        MonitoredItemResp_StatusCode(p_index) := p_sc
    END

END
