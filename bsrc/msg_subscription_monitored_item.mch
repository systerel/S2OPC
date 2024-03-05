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
    msg_subscription_monitored_item

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

DEFINITIONS

    d_inv_create == (
        CreateMonitoredItemReq_NodeId      : t_monitoredItemReqIndex +-> t_NodeId &
        CreateMonitoredItemReq_AttributeId : t_monitoredItemReqIndex +-> t_AttributeId_i &
        CreateMonitoredItemReq_IndexRange  : t_monitoredItemReqIndex +-> t_IndexRange_i &
        CreateMonitoredItemReq_ValidMonitoringMode : t_monitoredItemReqIndex +-> BOOL &
        CreateMonitoredItemReq_Filter : t_monitoredItemReqIndex +-> t_monitoringFilter_i &

        CreateMonitoredItemResp_Allocated : BOOL &
        CreateMonitoredItemResp_Id : t_monitoredItemReqIndex +-> t_monitoredItemId_i &
        CreateMonitoredItemResp_StatusCode : t_monitoredItemReqIndex +-> t_StatusCode
    );

    d_inv_modify == (
        ModifyMonitoredItemReq_Id : t_monitoredItemReqIndex +-> t_monitoredItemId_i &
        ModifyMonitoredItemReq_Filter : t_monitoredItemReqIndex +-> t_monitoringFilter_i &
        ModifyMonitoredItemResp_Allocated : BOOL &
        ModifyMonitoredItemResp_StatusCode : t_monitoredItemReqIndex +-> t_StatusCode
    );

    d_inv_delete == (
        DelMonitoredItemReq_Id : t_monitoredItemReqIndex +-> t_monitoredItemId_i &
        DelMonitoredItemResp_Allocated : BOOL &
        DelMonitoredItemResp_StatusCode : t_monitoredItemReqIndex +-> t_StatusCode
    );

    d_inv_set_monitoring_mode == (
        SetMonitModeMonitoredItemReq_Id : t_monitoredItemReqIndex +-> t_monitoredItemId_i &
        SetMonitModeMonitoredItemResp_Allocated : BOOL &
        SetMonitModeMonitoredItemResp_StatusCode : t_monitoredItemReqIndex +-> t_StatusCode)

ABSTRACT_VARIABLES
    nb_monitored_items,
    CreateMonitoredItemReq_NodeId,
    CreateMonitoredItemReq_AttributeId,
    CreateMonitoredItemReq_IndexRange,
    CreateMonitoredItemReq_ValidMonitoringMode,
    CreateMonitoredItemReq_Filter,
    CreateMonitoredItemResp_Allocated,
    CreateMonitoredItemResp_Id,
    CreateMonitoredItemResp_StatusCode,

    ModifyMonitoredItemReq_Id,
    ModifyMonitoredItemReq_Filter,
    ModifyMonitoredItemResp_Allocated,
    ModifyMonitoredItemResp_StatusCode,

    DelMonitoredItemReq_Id,
    DelMonitoredItemResp_Allocated,
    DelMonitoredItemResp_StatusCode,

    SetMonitModeMonitoredItemReq_Id,
    SetMonitModeMonitoredItemResp_Allocated,
    SetMonitModeMonitoredItemResp_StatusCode

INVARIANT
    nb_monitored_items : NAT &
    nb_monitored_items <= k_n_monitoredItem_max &

    d_inv_create &
    d_inv_modify &
    d_inv_delete &
    d_inv_set_monitoring_mode

INITIALISATION
    nb_monitored_items           := 0  ||
    CreateMonitoredItemReq_NodeId      := {} ||
    CreateMonitoredItemReq_AttributeId := {} ||
    CreateMonitoredItemReq_IndexRange := {} ||
    CreateMonitoredItemReq_ValidMonitoringMode := {} ||
    CreateMonitoredItemReq_Filter := {} ||
    CreateMonitoredItemResp_Allocated := FALSE ||
    CreateMonitoredItemResp_Id := {}         ||
    CreateMonitoredItemResp_StatusCode := {} ||

    ModifyMonitoredItemReq_Id := {} ||
    ModifyMonitoredItemReq_Filter := {} ||
    ModifyMonitoredItemResp_Allocated := FALSE ||
    ModifyMonitoredItemResp_StatusCode := {} ||

    DelMonitoredItemReq_Id := {} ||
    DelMonitoredItemResp_Allocated := FALSE ||
    DelMonitoredItemResp_StatusCode := {} ||

    SetMonitModeMonitoredItemReq_Id := {} ||
    SetMonitModeMonitoredItemResp_Allocated := FALSE ||
    SetMonitModeMonitoredItemResp_StatusCode := {}

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
        CreateMonitoredItemReq_NodeId,
        CreateMonitoredItemReq_AttributeId,
        CreateMonitoredItemReq_IndexRange,
        CreateMonitoredItemReq_ValidMonitoringMode,
        CreateMonitoredItemReq_Filter
        :(  p_sc                 : t_StatusCode &
            p_subscription       : t_subscription_i &
            p_timestampToRet     : t_TimestampsToReturn_i &
            p_nb_monitored_items : NAT &
            nb_monitored_items : NAT &
            nb_monitored_items <= k_n_monitoredItem_max &
            (p_sc = e_sc_ok
                =>
                p_nb_monitored_items > 0 &
                p_nb_monitored_items <= k_n_monitoredItem_max &
                nb_monitored_items = p_nb_monitored_items &
                CreateMonitoredItemReq_NodeId : 1..nb_monitored_items --> t_NodeId &
                CreateMonitoredItemReq_AttributeId : 1..nb_monitored_items --> t_AttributeId_i &
                CreateMonitoredItemReq_IndexRange : 1..nb_monitored_items --> t_IndexRange_i &
                CreateMonitoredItemReq_ValidMonitoringMode : 1..nb_monitored_items --> BOOL &
                CreateMonitoredItemReq_Filter : 1..nb_monitored_items --> t_monitoringFilter_i &
                p_timestampToRet : t_TimestampsToReturn) &
            (p_sc /= e_sc_ok
                =>
                p_sc : {e_sc_bad_nothing_to_do,
                        e_sc_bad_timestamps_to_return_invalid,
                        e_sc_bad_too_many_ops} &
                d_inv_create
            )
        )
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
        p_index : t_monitoredItemReqIndex &
        p_index > 0 &
        p_index <= nb_monitored_items
    THEN
        p_bres, p_sc, p_nid, p_aid, p_monitMode, p_clientHandle, p_samplingItv, p_indexRange, p_discardOldest, p_queueSize, p_filter
        :(p_bres : BOOL &
          p_sc : t_StatusCode_i & p_sc : t_StatusCode &
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
          p_filter = CreateMonitoredItemReq_Filter(p_index) &
          (p_bres = TRUE =>
           p_sc = e_sc_ok &
           p_nid = CreateMonitoredItemReq_NodeId(p_index) &
           CreateMonitoredItemReq_AttributeId(p_index) : t_AttributeId &
           p_indexRange = CreateMonitoredItemReq_IndexRange(p_index) &
           p_aid = CreateMonitoredItemReq_AttributeId(p_index) &
           (p_filter = c_monitoringFilter_indet or p_filter : t_monitoringFilter) & // No filter or filter valid
           p_monitMode : t_monitoringMode) &
          (p_bres = FALSE => p_sc /= e_sc_ok) &
          (CreateMonitoredItemReq_AttributeId(p_index) /: t_AttributeId =>
           p_sc = e_sc_bad_attribute_id_invalid) &

          (CreateMonitoredItemReq_AttributeId(p_index) : t_AttributeId &
           CreateMonitoredItemReq_ValidMonitoringMode(p_index) = FALSE =>
           p_sc = e_sc_bad_monitoring_mode_invalid) &

          (CreateMonitoredItemReq_AttributeId(p_index) : t_AttributeId &
           CreateMonitoredItemReq_ValidMonitoringMode(p_index) = TRUE &
           (p_filter /= c_monitoringFilter_indet & p_filter /: t_monitoringFilter =>
               p_sc = e_sc_bad_monitored_item_filter_invalid or p_sc = e_sc_bad_monitored_item_filter_unsupported) &
           /* Note: attribute validity is also checked later but it is usefull to keep
                    this earlier verification for UACTT tests (UACTT does not expect unsupported attributes) */
           (p_filter /= c_monitoringFilter_indet &
            p_filter : t_monitoringFilter &
            CreateMonitoredItemReq_AttributeId(p_index) /= e_aid_Value =>
               p_sc = e_sc_bad_filter_not_allowed
           )
          )
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
        bres, CreateMonitoredItemResp_Allocated
        :(bres : BOOL &
          CreateMonitoredItemResp_Allocated : BOOL &
          CreateMonitoredItemResp_Allocated = bres)
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
        p_index > 0 &
        p_index <= nb_monitored_items &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode &
        p_monitored_item_id : t_monitoredItemId_i &
        (p_sc = e_sc_ok => p_monitored_item_id : t_monitoredItemId) &
        (p_sc /= e_sc_ok => p_monitored_item_id = c_monitoredItemId_indet) &
        p_revSamplingItv : t_opcua_duration_i &
        p_revQueueSize : t_counter &
        p_filterResult : t_filterResult_i &
        CreateMonitoredItemResp_Allocated = TRUE
    THEN
        CreateMonitoredItemResp_Id(p_index) := p_monitored_item_id ||
        CreateMonitoredItemResp_StatusCode(p_index) := p_sc
    END
    ;

    p_sc, p_subscription, p_timestampToRet, p_nb_monitored_items <-- getall_msg_modify_monitored_items_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_modify_req
    THEN
        p_sc,
        p_subscription,
        p_timestampToRet,
        p_nb_monitored_items,
        nb_monitored_items,
        ModifyMonitoredItemReq_Id,
        ModifyMonitoredItemReq_Filter
        :(  p_sc                 : t_StatusCode &
            p_subscription       : t_subscription_i &
            p_timestampToRet     : t_TimestampsToReturn_i &
            p_nb_monitored_items : NAT &
            nb_monitored_items : NAT &
            nb_monitored_items <= k_n_monitoredItem_max &
            (p_sc = e_sc_ok
                =>
                p_nb_monitored_items > 0 &
                p_nb_monitored_items <= k_n_monitoredItem_max &
                nb_monitored_items = p_nb_monitored_items &
                ModifyMonitoredItemReq_Id : 1..nb_monitored_items --> t_monitoredItemId_i &
                ModifyMonitoredItemReq_Filter : 1..nb_monitored_items --> t_monitoringFilter_i &
                p_timestampToRet : t_TimestampsToReturn) &
            (p_sc /= e_sc_ok
                =>
                p_sc : {e_sc_bad_nothing_to_do,
                        e_sc_bad_timestamps_to_return_invalid,
                        e_sc_bad_too_many_ops} &
                d_inv_modify
            )
        )
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
        p_index : t_monitoredItemReqIndex &
        p_index > 0 &
        p_index <= nb_monitored_items
    THEN
        p_bres, p_sc, p_monitored_item_id, p_clientHandle, p_samplingItv, p_discardOldest, p_queueSize, p_filter
        :(p_bres : BOOL &
          p_sc : t_StatusCode_i & p_sc : t_StatusCode &
          p_sc : {e_sc_ok,
                  e_sc_bad_monitored_item_filter_unsupported,
                  e_sc_bad_monitored_item_filter_invalid} &
          p_monitored_item_id : t_monitoredItemId_i &
          p_clientHandle : t_client_handle_i &
          p_samplingItv : t_opcua_duration_i &
          p_discardOldest : BOOL &
          p_queueSize : t_counter &
          p_filter : t_monitoringFilter_i &
          p_filter = ModifyMonitoredItemReq_Filter(p_index) &
          (p_bres = TRUE =>
           p_sc = e_sc_ok &
           (p_filter = c_monitoringFilter_indet or p_filter : t_monitoringFilter)) & // No filter or filter valid
          (p_bres = FALSE => p_sc /= e_sc_ok) &
          (p_filter /= c_monitoringFilter_indet & p_filter /: t_monitoringFilter => // Filter is invalid (unknown) or unsupported (Aggregate / Event)
            p_sc = e_sc_bad_monitored_item_filter_invalid or p_sc = e_sc_bad_monitored_item_filter_unsupported)
       )
    END
    ;

    bres <-- alloc_msg_modify_monitored_items_resp_results (p_resp_msg, p_nb_results) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_modify_resp &
        p_nb_results : NAT &
        p_nb_results > 0 &
        p_nb_results = nb_monitored_items
    THEN
        bres, ModifyMonitoredItemResp_Allocated
        :(bres : BOOL &
          ModifyMonitoredItemResp_Allocated : BOOL &
          ModifyMonitoredItemResp_Allocated = bres)
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
        p_index > 0 &
        p_index <= nb_monitored_items &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode &
        p_revSamplingItv : t_opcua_duration_i &
        p_revQueueSize : t_counter &
        p_filterResult : t_filterResult_i &
        ModifyMonitoredItemResp_Allocated = TRUE
    THEN
        ModifyMonitoredItemResp_StatusCode(p_index) := p_sc
    END
    ;

    p_sc, p_subscription, p_nb_monitored_items <-- getall_msg_delete_monitored_items_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_delete_req
    THEN
        p_sc,
        p_subscription,
        p_nb_monitored_items,
        nb_monitored_items,
        DelMonitoredItemReq_Id
        :(  p_sc                 : t_StatusCode &
            p_subscription       : t_subscription_i &
            p_nb_monitored_items : NAT &
            nb_monitored_items : NAT &
            nb_monitored_items <= k_n_monitoredItem_max &
            (p_sc = e_sc_ok
                =>
                p_nb_monitored_items > 0 &
                p_nb_monitored_items <= k_n_monitoredItem_max &
                nb_monitored_items = p_nb_monitored_items &
                DelMonitoredItemReq_Id : 1..nb_monitored_items --> t_monitoredItemId_i) &
            (p_sc /= e_sc_ok
                =>
                p_sc : {e_sc_bad_nothing_to_do, e_sc_bad_too_many_ops} &
                d_inv_delete
            )
        )
    END
    ;

    bres <-- alloc_msg_delete_monitored_items_resp_results (p_resp_msg, p_nb_results) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_delete_resp &
        p_nb_results : NAT &
        p_nb_results > 0 &
        p_nb_results = nb_monitored_items
    THEN
        bres, DelMonitoredItemResp_Allocated
        :(bres : BOOL &
          DelMonitoredItemResp_Allocated : BOOL &
          DelMonitoredItemResp_Allocated = bres)
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
        p_index > 0 &
        p_index <= nb_monitored_items &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode &
        DelMonitoredItemResp_Allocated = TRUE
    THEN
        DelMonitoredItemResp_StatusCode(p_index) := p_sc
    END
    ;

    p_monitored_item_id <-- getall_delete_monitored_item_req_params (p_req_msg, p_index) =
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
        p_monitored_item_id := DelMonitoredItemReq_Id(p_index)
    END
    ;

    p_sc, p_subscription, p_monitoring_mode, p_nb_monitored_items <-- getall_msg_set_monit_mode_monitored_items_req_params (p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_monitored_items_set_monitoring_mode_req
    THEN
        p_sc,
        p_subscription,
        p_monitoring_mode,
        p_nb_monitored_items,
        nb_monitored_items,
        SetMonitModeMonitoredItemReq_Id
        :(  p_sc                 : t_StatusCode &
            p_subscription       : t_subscription_i &
            p_monitoring_mode : t_monitoringMode_i &
            p_nb_monitored_items : NAT &
            nb_monitored_items : NAT &
            nb_monitored_items <= k_n_monitoredItem_max &
            (p_sc = e_sc_ok
                =>
                p_monitoring_mode : t_monitoringMode &
                p_nb_monitored_items > 0 &
                p_nb_monitored_items <= k_n_monitoredItem_max &
                nb_monitored_items = p_nb_monitored_items &
                SetMonitModeMonitoredItemReq_Id : 1..nb_monitored_items --> t_monitoredItemId_i) &
            (p_sc /= e_sc_ok
                =>
                p_sc : {e_sc_bad_nothing_to_do, e_sc_bad_too_many_ops, e_sc_bad_monitoring_mode_invalid} &
                d_inv_delete
            )
        )
    END
    ;

    bres <-- alloc_msg_set_monit_mode_monitored_items_resp_results (p_resp_msg, p_nb_results) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_monitored_items_set_monitoring_mode_resp &
        p_nb_results : NAT &
        p_nb_results > 0 &
        p_nb_results = nb_monitored_items
    THEN
        bres, SetMonitModeMonitoredItemResp_Allocated
        :(bres : BOOL &
          SetMonitModeMonitoredItemResp_Allocated : BOOL &
          SetMonitModeMonitoredItemResp_Allocated = bres)
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
        p_index > 0 &
        p_index <= nb_monitored_items &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode &
        SetMonitModeMonitoredItemResp_Allocated = TRUE
    THEN
        SetMonitModeMonitoredItemResp_StatusCode(p_index) := p_sc
    END
    ;

    p_monitored_item_id <-- getall_set_monit_mode_monitored_item_req_params (p_req_msg, p_index) =
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
        p_monitored_item_id := SetMonitModeMonitoredItemReq_Id(p_index)
    END

END
