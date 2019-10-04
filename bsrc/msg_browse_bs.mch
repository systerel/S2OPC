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
    msg_browse_bs

SEES
    constants,
    constants_statuscodes_bs,
    message_in_bs,
    message_out_bs

DEFINITIONS
    d_variables_req ==
        RequestBrowse,
        RequestBrowse_nb_BrowseValue,
        RequestBrowse_nb_BrowseTargetMax, /* requestedMaxReferencesPerNode */
        RequestBrowse_BrowseView,
        RequestBrowse_BrowseValue_NodeId,
        RequestBrowse_BrowseValue_BrowseDirection,
        RequestBrowse_BrowseValue_ReferenceTypeId,
        RequestBrowse_BrowseValue_IncludedSubtypes,
        RequestBrowse_BrowseValue_NodeClassMask,
        RequestBrowse_BrowseValue_ResultMask
    ;
    d_variables_resp ==
        ResponseBrowse,
        ResponseBrowse_BrowseStatus,
        ResponseBrowse_ContinuationPoint,
        /* The result array of complying references starting from browsed node for each browse request item */
        ResponseBrowse_ResultReferences,
        isBrowseResponseAllocated
    ;
    d_inv_req ==
        RequestBrowse                               : t_msg_i                               &
        RequestBrowse_nb_BrowseValue                : NAT                                   &
        RequestBrowse_nb_BrowseTargetMax            : NAT                                   &
        RequestBrowse_BrowseView                    : t_NodeId_i                            &
        RequestBrowse_BrowseValue_NodeId            : t_BrowseValue +-> t_NodeId_i          &
        RequestBrowse_BrowseValue_BrowseDirection   : t_BrowseValue +-> t_BrowseDirection_i &
        RequestBrowse_BrowseValue_ReferenceTypeId   : t_BrowseValue +-> t_NodeId_i          &
        RequestBrowse_BrowseValue_IncludedSubtypes <: t_BrowseValue                         &
        RequestBrowse_BrowseValue_NodeClassMask     : t_BrowseValue +-> t_BrowseNodeClassMask_i &
        RequestBrowse_BrowseValue_ResultMask     : t_BrowseValue +-> t_BrowseResultMask_i
    ;
    d_inv_resp ==
        ResponseBrowse                     : t_msg_i &
        ResponseBrowse_BrowseStatus        : t_BrowseValue +-> t_StatusCode        &
        ResponseBrowse_ContinuationPoint   : t_BrowseValue +-> t_ContinuationPointId_i &
        ResponseBrowse_ResultReferences    : t_BrowseValue +-> t_BrowseResultReferences &
        isBrowseResponseAllocated : BOOL
    ;
    d_init_req ==
        d_variables_req
        :(
            RequestBrowse = c_msg_indet                     &
            RequestBrowse_nb_BrowseValue               = 0  &
            RequestBrowse_nb_BrowseTargetMax           = 0  &
            RequestBrowse_BrowseView                   = c_NodeId_indet &
            RequestBrowse_BrowseValue_NodeId           = {} &
            RequestBrowse_BrowseValue_BrowseDirection  = {} &
            RequestBrowse_BrowseValue_ReferenceTypeId  = {} &
            RequestBrowse_BrowseValue_IncludedSubtypes = {} &
            RequestBrowse_BrowseValue_NodeClassMask    = {} &
            RequestBrowse_BrowseValue_ResultMask    = {}
        )
    ;
    d_init_resp ==
        d_variables_resp
        :(
            ResponseBrowse                     = c_msg_indet &
            ResponseBrowse_BrowseStatus        = {} &
            ResponseBrowse_ContinuationPoint   = {} &
            ResponseBrowse_ResultReferences    = {} &
            isBrowseResponseAllocated = FALSE
         )

ABSTRACT_VARIABLES
    d_variables_req,
    d_variables_resp

INVARIANT
    d_inv_req &
    d_inv_resp

INITIALISATION
    d_init_req ||
    d_init_resp

OPERATIONS

    /* Browse Request */
    p_nid_view, p_nb_BrowseTargetMax, p_nb_browse_value <-- get_browse_request_params(p_req_msg) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_browse_req
    THEN
        p_nid_view,
        p_nb_BrowseTargetMax,
        p_nb_browse_value,
        d_variables_req
        :(
            p_nid_view : t_NodeId_i &
            p_nb_BrowseTargetMax : NAT &
            p_nb_browse_value : NAT &
            d_inv_req &
            RequestBrowse = p_req_msg &
            RequestBrowse_nb_BrowseTargetMax >= 0 & /* 0 means no limit */
            RequestBrowse_BrowseValue_NodeId           : cast_t_BrowseValue[1..RequestBrowse_nb_BrowseValue] --> t_NodeId_i          &
            RequestBrowse_BrowseValue_BrowseDirection  : cast_t_BrowseValue[1..RequestBrowse_nb_BrowseValue] --> t_BrowseDirection_i &
            RequestBrowse_BrowseValue_ReferenceTypeId  : cast_t_BrowseValue[1..RequestBrowse_nb_BrowseValue] --> t_NodeId_i          &
            RequestBrowse_BrowseValue_IncludedSubtypes <: cast_t_BrowseValue[1..RequestBrowse_nb_BrowseValue]                        &
            RequestBrowse_BrowseValue_NodeClassMask    : cast_t_BrowseValue[1..RequestBrowse_nb_BrowseValue] --> t_BrowseNodeClassMask_i &
            RequestBrowse_BrowseValue_ResultMask       : cast_t_BrowseValue[1..RequestBrowse_nb_BrowseValue] --> t_BrowseResultMask_i &
            p_nid_view = RequestBrowse_BrowseView &
            p_nb_BrowseTargetMax = RequestBrowse_nb_BrowseTargetMax &
            p_nb_browse_value = RequestBrowse_nb_BrowseValue
        )
    END
    ;

    p_NodeId, p_dir, p_reftype, p_inc_subtype, p_class_mask, p_result_mask <-- getall_BrowseValue(p_req_msg, p_bvi) =
    PRE
        p_req_msg : t_msg_i &
        p_req_msg : t_msg &
        p_req_msg = c_msg_in &
        a_msg_in_type = e_msg_view_browse_req &

        p_bvi : t_BrowseValue_i &
        p_bvi : t_BrowseValue &
        p_bvi : cast_t_BrowseValue[1..RequestBrowse_nb_BrowseValue] &
        p_bvi : dom(RequestBrowse_BrowseValue_NodeId) &
        p_bvi : dom(RequestBrowse_BrowseValue_BrowseDirection) &
        p_bvi : dom(RequestBrowse_BrowseValue_ReferenceTypeId) &
        p_bvi : dom(RequestBrowse_BrowseValue_NodeClassMask) &
        p_bvi : dom(RequestBrowse_BrowseValue_ResultMask) &

        RequestBrowse = p_req_msg &
        RequestBrowse_nb_BrowseValue > 0 &
        RequestBrowse_nb_BrowseValue <= k_n_BrowseResponse_max
    THEN
        p_NodeId,
        p_dir,
        p_reftype,
        p_inc_subtype,
        p_class_mask,
        p_result_mask
        :(  p_NodeId      = RequestBrowse_BrowseValue_NodeId(p_bvi) &
            p_dir         = RequestBrowse_BrowseValue_BrowseDirection(p_bvi) &
            p_reftype     = RequestBrowse_BrowseValue_ReferenceTypeId(p_bvi) &
            p_reftype = RequestBrowse_BrowseValue_ReferenceTypeId(p_bvi) &
            p_inc_subtype = bool(p_bvi : RequestBrowse_BrowseValue_IncludedSubtypes) &
            p_class_mask  = RequestBrowse_BrowseValue_NodeClassMask(p_bvi) &
            p_result_mask  = RequestBrowse_BrowseValue_ResultMask(p_bvi)
        )
    END
    ;

    /* Browse Response */

    p_isallocated <-- alloc_browse_response(p_resp_msg, p_nb_bvi) =
    PRE
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_view_browse_resp &
        p_nb_bvi : NAT1 &
        p_nb_bvi = RequestBrowse_nb_BrowseValue
    THEN
        p_isallocated :: BOOL ||
        d_variables_resp
        :(d_inv_resp &
          isBrowseResponseAllocated = p_isallocated &
          ResponseBrowse = p_resp_msg &
          ResponseBrowse_BrowseStatus        = {} &
          ResponseBrowse_ContinuationPoint   = {} &
          ResponseBrowse_ResultReferences    = {}
         )
    END
    ;

    set_ResponseBrowse_BrowseStatus(p_resp_msg, p_bvi, p_sc) =
    PRE
        isBrowseResponseAllocated = TRUE &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_view_browse_resp &
        p_bvi  : t_BrowseValue_i &
        p_bvi  : t_BrowseValue   &
        p_sc : t_StatusCode_i &
        p_sc : t_StatusCode &
        ResponseBrowse = p_resp_msg
    THEN
        ResponseBrowse_BrowseStatus(p_bvi) := p_sc
    END
    ;

    set_ResponseBrowse_ContinuationPoint(p_resp_msg, p_bvi, p_continuationPointId) =
    PRE
        isBrowseResponseAllocated = TRUE &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_view_browse_resp &
        p_bvi : t_BrowseValue_i &
        p_bvi : t_BrowseValue   &
        p_continuationPointId : t_ContinuationPointId_i   &
        ResponseBrowse = p_resp_msg
    THEN
        ResponseBrowse_ContinuationPoint(p_bvi) := p_continuationPointId
    END
    ;

    set_ResponseBrowse_BrowseResult(p_resp_msg, p_bvi, p_nb_targets, p_browseResultReferences) =
    PRE
        isBrowseResponseAllocated = TRUE &
        p_resp_msg : t_msg_i &
        p_resp_msg : t_msg &
        p_resp_msg = c_msg_out &
        a_msg_out_type = e_msg_view_browse_resp &
        p_bvi          : t_BrowseValue_i  &
        p_bvi          : t_BrowseValue    &
        p_nb_targets   : NAT &
        p_nb_targets <= k_n_BrowseTarget_max &
        (RequestBrowse_nb_BrowseTargetMax > 0 =>
            p_nb_targets <= RequestBrowse_nb_BrowseTargetMax) &
        p_browseResultReferences : t_BrowseResultReferences_i &
        p_browseResultReferences : t_BrowseResultReferences &
        ResponseBrowse = p_resp_msg
    THEN
        ResponseBrowse_ResultReferences (p_bvi) := p_browseResultReferences
    END

END
