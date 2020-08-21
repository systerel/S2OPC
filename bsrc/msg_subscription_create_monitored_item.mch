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
    msg_subscription_create_monitored_item

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

DEFINITIONS
    d_inv == (
        nb_monitored_items : NAT &
        nb_monitored_items <= k_n_monitoredItem_max &

        MonitoredItemReq_NodeId      : t_monitoredItemReqIndex +-> t_NodeId &
        MonitoredItemReq_AttributeId : t_monitoredItemReqIndex +-> t_AttributeId_i &
        MonitoredItemReq_IndexRange  : t_monitoredItemReqIndex +-> t_IndexRange_i &
        MonitoredItemReq_ValidMonitoringMode : t_monitoredItemReqIndex +-> BOOL &
        MonitoredItemReq_FilterPresence : t_monitoredItemReqIndex +-> BOOL &

        MonitoredItemResp_Allocated : BOOL &
        MonitoredItemResp_Id : t_monitoredItemReqIndex +-> t_monitoredItemId_i &
        MonitoredItemResp_StatusCode : t_monitoredItemReqIndex +-> t_StatusCode
    )

ABSTRACT_VARIABLES
    nb_monitored_items,
    MonitoredItemReq_NodeId,
    MonitoredItemReq_AttributeId,
    MonitoredItemReq_IndexRange,
    MonitoredItemReq_ValidMonitoringMode,
    MonitoredItemReq_FilterPresence,
    MonitoredItemResp_Allocated,
    MonitoredItemResp_Id,
    MonitoredItemResp_StatusCode

INVARIANT
    d_inv

INITIALISATION
    nb_monitored_items           := 0  ||
    MonitoredItemReq_NodeId      := {} ||
    MonitoredItemReq_AttributeId := {} ||
    MonitoredItemReq_IndexRange := {} ||
    MonitoredItemReq_ValidMonitoringMode := {} ||
    MonitoredItemReq_FilterPresence := {} ||
    MonitoredItemResp_Allocated := FALSE ||
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
        MonitoredItemReq_AttributeId,
        MonitoredItemReq_IndexRange,
        MonitoredItemReq_ValidMonitoringMode,
        MonitoredItemReq_FilterPresence
        :(  p_sc                 : t_StatusCode &
            p_subscription       : t_subscription_i &
            p_timestampToRet     : t_TimestampsToReturn_i &
            p_nb_monitored_items : NAT &
            (p_sc = e_sc_ok
                =>
                p_nb_monitored_items > 0 &
                p_nb_monitored_items <= k_n_monitoredItem_max &
                nb_monitored_items = p_nb_monitored_items &
                MonitoredItemReq_NodeId : 1..nb_monitored_items +-> t_NodeId &
                MonitoredItemReq_AttributeId : 1..nb_monitored_items --> t_AttributeId_i &
                MonitoredItemReq_IndexRange : 1..nb_monitored_items --> t_IndexRange_i &
                MonitoredItemReq_ValidMonitoringMode : 1..nb_monitored_items --> BOOL &
                MonitoredItemReq_FilterPresence : 1..nb_monitored_items --> BOOL &
                p_timestampToRet : t_TimestampsToReturn) &
            (p_sc /= e_sc_ok
                =>
                p_sc : {e_sc_bad_decoding_error, e_sc_bad_nothing_to_do,
                        e_sc_bad_timestamps_to_return_invalid,
                        e_sc_bad_too_many_ops} &
                d_inv
            )
        )
    END
    ;

    /* Filters and DiscardOldest not extracted => not managed */
    p_bres, p_sc, p_nid, p_aid, p_monitMode, p_clientHandle, p_samplingItv, p_queueSize, p_indexRange <-- getall_monitored_item_req_params (p_req_msg, p_index) =
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
        p_bres, p_sc, p_nid, p_aid, p_monitMode, p_clientHandle, p_samplingItv, p_indexRange, p_queueSize
        :(p_bres : BOOL &
          p_sc : t_StatusCode_i & p_sc : t_StatusCode &
          p_sc : {e_sc_ok, e_sc_bad_attribute_id_invalid,
                  e_sc_bad_monitoring_mode_invalid,
                  e_sc_bad_monitored_item_filter_unsupported} &
          p_nid : t_NodeId_i &
          p_aid : t_AttributeId_i &
          p_monitMode : t_monitoringMode_i &
          p_clientHandle : t_client_handle_i &
          p_samplingItv : t_opcua_duration_i &
          p_queueSize : t_counter &
          p_indexRange : t_IndexRange_i &
          (p_bres = TRUE =>
           p_sc = e_sc_ok &
           p_nid = MonitoredItemReq_NodeId(p_index) &
           MonitoredItemReq_AttributeId(p_index) : t_AttributeId &
           p_indexRange = MonitoredItemReq_IndexRange(p_index) &
           p_aid = MonitoredItemReq_AttributeId(p_index) &
           p_monitMode : t_monitoringMode) &
          (p_bres = FALSE => p_sc /= e_sc_ok) &
          (MonitoredItemReq_AttributeId(p_index) /: t_AttributeId =>
           p_sc = e_sc_bad_attribute_id_invalid) &

          (MonitoredItemReq_AttributeId(p_index) : t_AttributeId &
           MonitoredItemReq_ValidMonitoringMode(p_index) = FALSE =>
           p_sc = e_sc_bad_monitoring_mode_invalid) &

          (MonitoredItemReq_AttributeId(p_index) : t_AttributeId &
           MonitoredItemReq_ValidMonitoringMode(p_index) = TRUE &
           MonitoredItemReq_FilterPresence(p_index) = TRUE =>
           p_sc = e_sc_bad_monitored_item_filter_unsupported)
         )
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
        bres, MonitoredItemResp_Allocated
        :(bres : BOOL &
          MonitoredItemResp_Allocated : BOOL &
          MonitoredItemResp_Allocated = bres)
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
        (p_sc = e_sc_ok => p_monitored_item_id : t_monitoredItemId) &
        (p_sc /= e_sc_ok => p_monitored_item_id = c_monitoredItemId_indet) &
        p_revSamplingItv : t_opcua_duration_i &
        p_revQueueSize : t_counter &
        MonitoredItemResp_Allocated = TRUE
    THEN
        MonitoredItemResp_Id(p_index) := p_monitored_item_id ||
        MonitoredItemResp_StatusCode(p_index) := p_sc
    END

END
