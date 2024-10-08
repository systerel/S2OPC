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
    msg_subscription_monitored_item_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

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

    p_bres, p_sc, p_nid, p_aid, p_monitMode, p_clientHandle, p_samplingItv, p_filter, p_discardOldest, p_queueSize, p_indexRange
      <-- getall_create_monitored_item_req_params (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_create_req &
        p_index : NAT &
        p_index : t_monitoredItemReqIndex
    THEN
        p_bres, p_sc, p_nid, p_aid, p_monitMode, p_clientHandle, p_samplingItv,
        p_filter, p_indexRange, p_discardOldest, p_queueSize
        :(p_bres : BOOL &
          p_sc : t_StatusCode_i &
          p_sc : {e_sc_ok, e_sc_bad_attribute_id_invalid,
                  e_sc_bad_monitoring_mode_invalid,
                  e_sc_bad_monitored_item_filter_unsupported,
                  e_sc_bad_monitored_item_filter_invalid,
                  e_sc_bad_deadband_filter_invalid,
                  e_sc_bad_filter_not_allowed} &
          p_nid : t_NodeId_i &
          p_aid : t_AttributeId_i &
          p_monitMode : t_monitoringMode_i &
          p_clientHandle : t_client_handle_i &
          p_samplingItv : t_opcua_duration_i &
          p_discardOldest : BOOL &
          p_queueSize : t_counter &
          p_indexRange : t_IndexRange_i &
          p_filter : t_monitoringFilter_i &
          (p_bres = TRUE =>
           p_sc = e_sc_ok &
           p_nid : t_NodeId &
           p_aid : t_AttributeId &
           p_monitMode : t_monitoringMode &
           (p_filter = c_monitoringFilter_indet or p_filter : t_monitoringFilter)) &
          (p_bres = FALSE => p_sc /= e_sc_ok))
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

    setall_msg_create_monitored_item_resp_params (p_resp_msg, p_index, p_sc, p_monitored_item_id, p_revSamplingItv, p_revQueueSize, p_filterResult) =
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
        p_revQueueSize : t_counter &
        p_filterResult : t_filterResult_i
    THEN
        skip
    END
    ;

    p_bres, p_sc, p_monitored_item_id, p_clientHandle, p_samplingItv, p_filter, p_discardOldest, p_queueSize
    <-- getall_modify_monitored_item_req_params (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_modify_req &
        p_index : NAT &
        p_index : t_monitoredItemReqIndex
    THEN
        p_bres, p_sc, p_monitored_item_id, p_clientHandle, p_samplingItv, p_filter, p_discardOldest, p_queueSize
        :(p_bres : BOOL &
          p_sc : t_StatusCode_i &
          p_sc : {e_sc_ok,
                  e_sc_bad_monitored_item_filter_unsupported,
                  e_sc_bad_monitored_item_filter_invalid} &
          p_monitored_item_id : t_monitoredItemId_i &
          p_clientHandle : t_client_handle_i &
          p_samplingItv : t_opcua_duration_i &
          p_discardOldest : BOOL &
          p_queueSize : t_counter  &
          p_filter : t_monitoringFilter_i &
          (p_bres = TRUE =>
           p_sc = e_sc_ok &
           (p_filter = c_monitoringFilter_indet or p_filter : t_monitoringFilter)) &
          (p_bres = FALSE => p_sc /= e_sc_ok)
        )
    END
    ;

    p_subscription <-- get_msg_modify_monitored_items_req_subscription (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_modify_req
    THEN
        p_subscription :: t_subscription_i
    END
    ;

    p_timestampToRet <-- get_msg_modify_monitored_items_req_timestamp_to_ret (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_modify_req
    THEN
        p_timestampToRet :: t_TimestampsToReturn_i
    END
    ;

    p_nb_monitored_items <-- get_msg_modify_monitored_items_req_nb_monitored_items (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_modify_req
    THEN
        p_nb_monitored_items :: INT
    END
    ;

    bres <-- alloc_msg_modify_monitored_items_resp_results (p_resp_msg, p_nb_results) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_modify_resp &
        p_nb_results : NAT &
        p_nb_results > 0
    THEN
        bres :: BOOL
    END
    ;

    setall_msg_modify_monitored_item_resp_params (p_resp_msg, p_index, p_sc, p_revSamplingItv, p_revQueueSize, p_filterResult) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_modify_resp &
        p_index : NAT &
        p_index : t_monitoredItemReqIndex &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode &
        p_revSamplingItv : t_opcua_duration_i &
        p_revQueueSize : t_counter &
        p_filterResult : t_filterResult_i
    THEN
        skip
    END
    ;

    p_subscription, p_nb_monitored_items <-- get_msg_delete_monitored_items_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_delete_req
    THEN
        p_subscription :: t_subscription_i ||
        p_nb_monitored_items :: INT
    END
    ;

    p_monitored_item_id <-- getall_delete_monitored_item_req_params (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_delete_req &
        p_index : NAT &
        p_index : t_monitoredItemReqIndex
    THEN
        p_monitored_item_id :(p_monitored_item_id : t_monitoredItemId_i)
    END
    ;

    bres <-- alloc_msg_delete_monitored_items_resp_results (p_resp_msg, p_nb_results) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_delete_resp &
        p_nb_results : NAT &
        p_nb_results > 0
    THEN
        bres :: BOOL
    END
    ;

    setall_msg_delete_monitored_item_resp_params (p_resp_msg, p_index, p_sc) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_delete_resp &
        p_index : NAT &
        p_index : t_monitoredItemReqIndex &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode
    THEN
        skip
    END
    ;

    p_subscription, p_monitoring_mode, p_nb_monitored_items <-- get_msg_set_monit_mode_monitored_items_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_set_monitoring_mode_req
    THEN
        p_subscription :: t_subscription_i ||
        p_monitoring_mode :: t_monitoringMode_i ||
        p_nb_monitored_items :: INT
    END
    ;

    p_monitored_item_id <-- getall_set_monit_mode_monitored_item_req_params (p_req_msg, p_index) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_set_monitoring_mode_req &
        p_index : NAT &
        p_index : t_monitoredItemReqIndex
    THEN
        p_monitored_item_id :: t_monitoredItemId_i
    END
    ;

    bres <-- alloc_msg_set_monit_mode_monitored_items_resp_results (p_resp_msg, p_nb_results) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_set_monitoring_mode_resp &
        p_nb_results : NAT &
        p_nb_results > 0
    THEN
        bres :: BOOL
    END
    ;

    setall_msg_set_monit_mode_monitored_item_resp_params (p_resp_msg, p_index, p_sc) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_set_monitoring_mode_resp &
        p_index : NAT &
        p_index : t_monitoredItemReqIndex &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode
    THEN
        skip
    END

END
